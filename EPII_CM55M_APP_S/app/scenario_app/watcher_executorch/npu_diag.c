/*
 * Bounded Ethos-U semaphore wait with NPU status dump.
 *
 * The upstream Ethos-U core driver (0.16.0) ships a weak
 * ethosu_semaphore_take() that spins on __WFE() forever: if the NPU
 * never raises its completion interrupt, Method::execute() never
 * returns and the whole event loop freezes with no diagnostic. This
 * strong override keeps the same wait semantics but bounds it to
 * ~2 s (the compiled model runs in ~20 ms) and, on expiry, dumps
 * the NPU STATUS/QREAD/QSIZE/BASEP registers so the stall cause
 * (bus fault, parse error, watchdog, mid-stream stall, or a
 * completed stream whose IRQ never arrived) is visible on the
 * console. ethosu_wait() then treats the -1 as an inference
 * timeout, resets the NPU, and returns an error instead of
 * hanging the device.
 *
 * The semaphore layout (one count byte) matches the driver's
 * private struct ethosu_semaphore_t. The DWT cycle counter is
 * enabled by et_pal_init() at startup; an iteration cap covers
 * the case where it is not running yet.
 */
#include <stdint.h>
#include <stddef.h>

#include "WE2_device.h"
#include "xprintf.h"

/* Same selection as cvapp_mb_cls.cpp: the secure alias in the
 * non-secure build, the direct base in the TrustZone build. */
#ifdef TRUSTZONE_SEC
#define WATCHER_U55_BASE BASE_ADDR_APB_U55_CTRL_ALIAS
#else
#ifndef TRUSTZONE
#define WATCHER_U55_BASE BASE_ADDR_APB_U55_CTRL_ALIAS
#else
#define WATCHER_U55_BASE BASE_ADDR_APB_U55_CTRL
#endif
#endif

struct watcher_ethosu_semaphore_t {
    uint8_t count;
};

#define WATCHER_NPU_STATUS_WORD (*(volatile uint32_t *)(WATCHER_U55_BASE + 0x0004UL))
#define WATCHER_NPU_CMD_WORD (*(volatile uint32_t *)(WATCHER_U55_BASE + 0x0008UL))
#define WATCHER_NPU_QREAD_WORD (*(volatile uint32_t *)(WATCHER_U55_BASE + 0x0018UL))
#define WATCHER_NPU_QSIZE_WORD (*(volatile uint32_t *)(WATCHER_U55_BASE + 0x0020UL))
#define WATCHER_NPU_BASEP_WORD(i) (*(volatile uint32_t *)(WATCHER_U55_BASE + 0x0080UL + (uint32_t)(i) * 8UL))
#define WATCHER_NPU_WD_STATUS_WORD (*(volatile uint32_t *)(WATCHER_U55_BASE + 0x0100UL))
#define WATCHER_NPU_MAC_STATUS_WORD (*(volatile uint32_t *)(WATCHER_U55_BASE + 0x0104UL))
#define WATCHER_NPU_AO_STATUS_WORD (*(volatile uint32_t *)(WATCHER_U55_BASE + 0x0108UL))
#define WATCHER_NPU_DMA_STATUS0_WORD (*(volatile uint32_t *)(WATCHER_U55_BASE + 0x0110UL))
#define WATCHER_NPU_DMA_STATUS1_WORD (*(volatile uint32_t *)(WATCHER_U55_BASE + 0x0114UL))
#define WATCHER_NPU_QBASE_WORD (*(volatile uint32_t *)(WATCHER_U55_BASE + 0x0010UL))
#define WATCHER_NPU_REGIONCFG_WORD (*(volatile uint32_t *)(WATCHER_U55_BASE + 0x003CUL))
#define WATCHER_NPU_QCONFIG_WORD (*(volatile uint32_t *)(WATCHER_U55_BASE + 0x001CUL))
#define WATCHER_DWT_CYCCNT (*(volatile uint32_t *)0xE0001004UL)

/* 400 MHz core clock: 2 s budget. */
#define WATCHER_NPU_WAIT_CYCLES (800000000UL)
#define WATCHER_NPU_WAIT_ITERS (20000000UL)

int ethosu_semaphore_take(void *sem, uint64_t timeout)
{
    uint32_t start;
    uint32_t iters = 0;
    struct watcher_ethosu_semaphore_t *s =
        (struct watcher_ethosu_semaphore_t *)sem;

    (void)timeout;
    if (s == NULL) {
        return -1;
    }
    start = WATCHER_DWT_CYCCNT;
    while (s->count == 0) {
        __asm volatile("wfe" ::: "memory");
        ++iters;
        if (iters > WATCHER_NPU_WAIT_ITERS ||
            (uint32_t)(WATCHER_DWT_CYCCNT - start) > WATCHER_NPU_WAIT_CYCLES) {
            uint32_t status = WATCHER_NPU_STATUS_WORD;
            xprintf("watcher NPU wait timeout: STATUS=0x%08x "
                    "(state=%u irq=%u bus=%u parse=%u end=%u "
                    "wd=%u ecc=%u fault_ch=%u) "
                    "CMD=0x%08x QREAD=%u QSIZE=%u "
                    "BASEP0=0x%08x BASEP1=0x%08x BASEP2=0x%08x "
                    "WD_STATUS=0x%08x MAC=0x%08x AO=0x%08x "
                    "DMA0=0x%08x DMA1=0x%08x "
                    "QBASE=0x%08x REGIONCFG=0x%08x QCONFIG=0x%08x\r\n",
                    (unsigned int)status,
                    (unsigned int)(status & 0x1U),
                    (unsigned int)((status >> 1) & 0x1U),
                    (unsigned int)((status >> 2) & 0x1U),
                    (unsigned int)((status >> 4) & 0x1U),
                    (unsigned int)((status >> 5) & 0x1U),
                    (unsigned int)((status >> 7) & 0x1U),
                    (unsigned int)((status >> 8) & 0x1U),
                    (unsigned int)((status >> 12) & 0xFU),
                    (unsigned int)WATCHER_NPU_CMD_WORD,
                    (unsigned int)WATCHER_NPU_QREAD_WORD,
                    (unsigned int)WATCHER_NPU_QSIZE_WORD,
                    (unsigned int)WATCHER_NPU_BASEP_WORD(0),
                    (unsigned int)WATCHER_NPU_BASEP_WORD(1),
                    (unsigned int)WATCHER_NPU_BASEP_WORD(2),
                    (unsigned int)WATCHER_NPU_WD_STATUS_WORD,
                    (unsigned int)WATCHER_NPU_MAC_STATUS_WORD,
                    (unsigned int)WATCHER_NPU_AO_STATUS_WORD,
                    (unsigned int)WATCHER_NPU_DMA_STATUS0_WORD,
                    (unsigned int)WATCHER_NPU_DMA_STATUS1_WORD,
                    (unsigned int)WATCHER_NPU_QBASE_WORD,
                    (unsigned int)WATCHER_NPU_REGIONCFG_WORD,
                    (unsigned int)WATCHER_NPU_QCONFIG_WORD);
            return -1;
        }
    }
    s->count--;
    return 0;
}
