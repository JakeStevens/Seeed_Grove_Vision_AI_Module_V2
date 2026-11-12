/*
 * SPI-slave transport for the SenseCAP Watcher HX6538 firmware.
 *
 * Wire format: see sscma_spi_framing.h. The ESP32-S3 master clocks a
 * 256-byte command phase, then (for READ/AVAILABLE) clocks the data
 * phase only ~2 ms later, so the reply transfer must be armed from
 * the receive-DMA callback, not from frame polling: the callback
 * parses the 4-byte header and arms the transmit DMA immediately.
 * Only WRITE payload processing (AT command dispatch) is deferred
 * to watcher_spi_slave_poll().
 *
 * TX-queue access happens from both interrupt context (drain, in the
 * receive callback) and thread context (enqueue, in poll/AT handling).
 * All queue operations in this file run with IRQs disabled; the
 * framing-queue primitives themselves stay allocation-free and
 * host-testable (see sscma_spi_framing.c).
 */

#ifdef __cplusplus
extern "C" {
#include "hx_drv_spi.h"
#include "hx_drv_scu.h"
#include "hx_drv_gpio.h"
#include "WE2_core.h"
#include "xprintf.h"
}
#endif
#include "cmsis_compiler.h"

#include "sscma_spi_framing.h"
#include "sscma_spi_slave.h"
#include "watcher_at_server.h"

#include <string.h>

/* Reply queue storage: one JSON event (320x240 JPEG, base64) is
 * typically 15-40 kB. 96 kB leaves headroom for a response plus a
 * queued event. */
#define WATCHER_SPI_TX_QUEUE_CAP (96u * 1024u)
#define WATCHER_SPI_CMD_BUF_CAP 4096u

/* Large buffers live in the SRAM .bss.NoInit section (ahead of the
 * mm-heap start symbol), not in the 256 kB DATA region: the DATA
 * region also holds the 128 kB heap and 64 kB stack and is already
 * nearly full in the baseline torch build. All state is explicitly
 * reset by watcher_spi_slave_init(), so no-init storage is safe. */
static uint8_t s_tx_queue_storage[WATCHER_SPI_TX_QUEUE_CAP]
    __attribute__((section(".bss.NoInit.watcher_tx"), aligned(32)));
static watcher_tx_queue_t s_tx_queue;
static char s_cmd_storage[WATCHER_SPI_CMD_BUF_CAP]
    __attribute__((section(".bss.NoInit.watcher_cmd"), aligned(4)));
static watcher_cmd_asm_t s_cmd_asm;

static uint8_t s_rx_buf[WATCHER_SPI_RX_PACKET_LEN]
    __attribute__((section(".bss.NoInit.watcher_dma"), aligned(32)));
static uint8_t s_tx_buf[WATCHER_SPI_MAX_READ_PAYLOAD]
    __attribute__((section(".bss.NoInit.watcher_dma"), aligned(32)));
static uint8_t s_pkt_payload[WATCHER_SPI_MAX_WRITE_PAYLOAD]
    __attribute__((section(".bss.NoInit.watcher_dma"), aligned(4)));

static void watcher_spi_rx_cb(void *arg);
static void watcher_spi_tx_cb(void *arg);

static DEV_SPI_PTR s_spi_dev = NULL;
static volatile bool s_rx_armed = false;
static volatile bool s_rx_needs_arm = false;
static volatile bool s_pkt_ready = false;
static volatile uint8_t s_pkt_cmd = 0;
static volatile uint16_t s_pkt_len = 0;
static volatile bool s_stop_requested = false;

/* Diagnostics (ISR-safe counters; printed throttled from poll()). */
static volatile uint32_t s_diag_rx_packets = 0;
static volatile uint32_t s_diag_rx_parse_errors = 0;
static volatile uint32_t s_diag_tx_done = 0;
static volatile uint32_t s_diag_arm_failures = 0;

static void sync_update(void)
{
    GPIO_OUT_LEVEL_E level = (watcher_tx_queue_available(&s_tx_queue) > 0)
                                 ? GPIO_OUT_HIGH
                                 : GPIO_OUT_LOW;
    (void)hx_drv_gpio_set_out_value(GPIO2, level);
}

static void sync_update_irq_guarded(void)
{
    __disable_irq();
    sync_update();
    __enable_irq();
}

static int arm_rx(void)
{
    int32_t ret;
    if (s_spi_dev == NULL) {
        return -1;
    }
    ret = s_spi_dev->spi_read_dma(s_rx_buf, WATCHER_SPI_RX_PACKET_LEN,
                                  (void *)watcher_spi_rx_cb);
    if (ret == 0) {
        s_rx_armed = true;
        return 0;
    }
    s_diag_arm_failures++;
    s_rx_needs_arm = true;
    return (int)ret;
}

static void watcher_spi_rx_cb(void *arg)
{
    uint8_t cmd = 0;
    uint16_t len = 0;
    int prc;
    (void)arg;

    s_rx_armed = false;
    hx_InvalidateDCache_by_Addr((volatile void *)s_rx_buf,
                                (int32_t)sizeof(s_rx_buf));
    prc = watcher_spi_parse_packet(s_rx_buf, &cmd, &len);
    if (prc != 0) {
        s_diag_rx_parse_errors++;
        (void)arm_rx();
        return;
    }
    s_diag_rx_packets++;

    switch (cmd) {
    case WATCHER_SPI_CMD_WRITE:
        /* Defer AT dispatch to poll(); copy the payload now because
         * the RX buffer is reused by the next armed receive. */
        __disable_irq();
        memcpy(s_pkt_payload, &s_rx_buf[WATCHER_SPI_HEADER_LEN], len);
        s_pkt_cmd = cmd;
        s_pkt_len = len;
        s_pkt_ready = true;
        __enable_irq();
        (void)arm_rx();
        break;

    case WATCHER_SPI_CMD_READ: {
        size_t drained;
        __disable_irq();
        drained = watcher_tx_queue_drain(&s_tx_queue, s_tx_buf, len);
        __enable_irq();
        if (drained < len) {
            /* The master only asks for the length AVAILABLE reported,
             * so this should not happen; pad so the phase clocks out
             * exactly the requested number of bytes. */
            memset(&s_tx_buf[drained], 0, len - drained);
        }
        sync_update_irq_guarded();
        hx_CleanDCache_by_Addr((volatile void *)s_tx_buf, (int32_t)len);
        if (s_spi_dev->spi_write_dma(s_tx_buf, len, (void *)watcher_spi_tx_cb) != 0) {
            s_rx_needs_arm = true;
        }
        break;
    }

    case WATCHER_SPI_CMD_AVAILABLE: {
        uint8_t avail[2];
        __disable_irq();
        watcher_spi_encode_available(avail,
                                     (uint16_t)watcher_tx_queue_available(&s_tx_queue));
        __enable_irq();
        s_tx_buf[0] = avail[0];
        s_tx_buf[1] = avail[1];
        hx_CleanDCache_by_Addr((volatile void *)s_tx_buf, 2);
        if (s_spi_dev->spi_write_dma(s_tx_buf, 2, (void *)watcher_spi_tx_cb) != 0) {
            s_rx_needs_arm = true;
        }
        break;
    }

    case WATCHER_SPI_CMD_RESET:
        __disable_irq();
        watcher_tx_queue_reset(&s_tx_queue);
        watcher_cmd_asm_reset(&s_cmd_asm);
        __enable_irq();
        s_stop_requested = true;
        sync_update_irq_guarded();
        (void)arm_rx();
        break;

    case WATCHER_SPI_CMD_STOP:
        s_stop_requested = true;
        (void)arm_rx();
        break;

    case WATCHER_SPI_CMD_START:
    default:
        (void)arm_rx();
        break;
    }
}

static void watcher_spi_tx_cb(void *arg)
{
    (void)arg;
    s_diag_tx_done++;
    /* Re-arm immediately so the next command phase is not missed. */
    (void)arm_rx();
}

int watcher_spi_slave_init(void)
{
    int32_t ret;

    watcher_tx_queue_init(&s_tx_queue, s_tx_queue_storage, sizeof(s_tx_queue_storage));
    watcher_cmd_asm_init(&s_cmd_asm, s_cmd_storage, sizeof(s_cmd_storage));

    /* SPI slave pins: PB2=DO, PB3=DI, PB4=CLK, PB5=CS (see the
     * hx_drv_spi.h slave usage notes). SYNC is PB8 as GPIO2. */
    (void)hx_drv_scu_set_PB2_pinmux(SCU_PB2_PINMUX_SPI_S_DO, 1);
    (void)hx_drv_scu_set_PB3_pinmux(SCU_PB3_PINMUX_SPI_S_DI, 1);
    (void)hx_drv_scu_set_PB4_pinmux(SCU_PB4_PINMUX_SPI_S_CLK, 1);
    (void)hx_drv_scu_set_PB5_pinmux(SCU_PB5_PINMUX_SPI_S_CS, 1);
    (void)hx_drv_scu_set_PB8_pinmux(SCU_PB8_PINMUX_GPIO2_1, 0);
    (void)hx_drv_gpio_set_output(GPIO2, GPIO_OUT_LOW);
    /* SPI-slave sampling clock: the HX6538 datasheet (Table 5.10)
     * requires ssi_clk >= 10x the SCLK period for SPIS sampling
     * (spec is based on 400 MHz ssi_clk at 0.9 V). The ESP32 master
     * clocks at 12 MHz, so ssi_clk must be >= 120 MHz. Use the PLL
     * at 400 MHz (div 0). A 12 MHz ssi_clk (the earlier XTAL24M/1
     * setting) cannot sample a 12 MHz SCLK at all, and the receive
     * DMA never completed. */
    (void)hx_drv_scu_set_pdlsc_sspisclk_cfg(SCU_LSCSSPISSRC_PLL, 0);

    ret = hx_drv_spi_slv_init(USE_DW_SPI_SLV_0, DW_SSPI_S_RELBASE);
    if (ret != 0) {
        xprintf("watcher SPI slave init failed: %d\r\n", (int)ret);
        return (int)ret;
    }
    s_spi_dev = hx_drv_spi_slv_get_dev(USE_DW_SPI_SLV_0);
    if (s_spi_dev == NULL) {
        xprintf("watcher SPI slave get_dev failed\r\n");
        return -1;
    }
    ret = s_spi_dev->spi_open(DEV_SLAVE_MODE, SPI_CPOL_0_CPHA_0);
    if (ret != 0) {
        xprintf("watcher SPI slave open failed: %d\r\n", (int)ret);
        return (int)ret;
    }
    if (arm_rx() != 0) {
        xprintf("watcher SPI slave initial RX arm failed\r\n");
        return -1;
    }
    xprintf("watcher SPI slave ready (slave, mode 0)\r\n");
    return 0;
}

bool watcher_spi_slave_enqueue_reply(const uint8_t *data, size_t len)
{
    bool ok;
    __disable_irq();
    ok = watcher_tx_queue_enqueue(&s_tx_queue, data, len);
    __enable_irq();
    sync_update_irq_guarded();
    return ok;
}

void watcher_spi_slave_drop_replies(void)
{
    __disable_irq();
    watcher_tx_queue_reset(&s_tx_queue);
    __enable_irq();
    sync_update_irq_guarded();
}

void watcher_spi_slave_poll(void)
{
    static uint32_t s_poll_count = 0;
    if ((++s_poll_count % 150) == 0) {
        xprintf("watcher SPI diag: polls=%lu rx=%lu rx_err=%lu tx=%lu "
                "arm_fail=%lu armed=%d queue=%u\r\n",
                (unsigned long)s_poll_count,
                (unsigned long)s_diag_rx_packets,
                (unsigned long)s_diag_rx_parse_errors,
                (unsigned long)s_diag_tx_done,
                (unsigned long)s_diag_arm_failures,
                s_rx_armed ? 1 : 0,
                (unsigned)watcher_tx_queue_available(&s_tx_queue));
    }
    if (s_stop_requested) {
        s_stop_requested = false;
        watcher_at_reset_stream();
    }
    if (s_rx_needs_arm && !s_rx_armed) {
        s_rx_needs_arm = false;
        (void)arm_rx();
    }
    if (s_pkt_ready) {
        char *cmd_text = NULL;
        bool complete;
        uint8_t pkt_cmd;
        uint16_t pkt_len;
        __disable_irq();
        pkt_cmd = s_pkt_cmd;
        pkt_len = s_pkt_len;
        s_pkt_ready = false;
        __enable_irq();
        (void)pkt_cmd;
        complete = watcher_cmd_asm_feed(&s_cmd_asm, s_pkt_payload, pkt_len, &cmd_text);
        if (complete && cmd_text != NULL) {
            watcher_at_handle_command(cmd_text);
        }
    }
    sync_update_irq_guarded();
}
