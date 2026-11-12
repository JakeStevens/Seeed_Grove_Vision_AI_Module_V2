/*
 * SPI-slave transport for the SenseCAP Watcher HX6538 firmware.
 *
 * Implements the ESP32-S3 sscma_client SPI framing (see
 * sscma_spi_framing.h) as an SPI slave on the WE2 DW SPI slave
 * peripheral (PB2=DO, PB3=DI, PB4=CLK, PB5=CS, mode 0, up to 12MHz
 * from the ESP32 master). The SYNC line is HX6538 GPIO2 (PB8),
 * driven high while reply bytes are queued.
 *
 * The transport is driven by watcher_spi_slave_poll(), which must be
 * called regularly from the datapath event context. DMA completion
 * callbacks only set flags; all parsing and queueing happens in the
 * poll call, never in interrupt context.
 */

#ifndef WATCHER_SSCMA_SPI_SLAVE_H
#define WATCHER_SSCMA_SPI_SLAVE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Initialise pinmux, SYNC GPIO and the SPI slave peripheral and arm
 * the first 256-byte command-phase receive. Returns 0 on success. */
int watcher_spi_slave_init(void);

/* Progress the transport state machine: parse completed command-phase
 * packets, arm reply transfers, and refresh SYNC. Call on every
 * datapath frame event (and after any long operation) so AT commands
 * are answered well inside the ESP32 client's 2 s request timeout. */
void watcher_spi_slave_poll(void);

/* Enqueue reply bytes for delivery to the ESP32 host. The bytes are
 * streamed out by later READ commands. Returns false when the queue
 * is full (the caller should count and drop the reply). */
bool watcher_spi_slave_enqueue_reply(const uint8_t *data, size_t len);

/* Drop all queued reply bytes (used by AT+BREAK before its own
 * response is enqueued). */
void watcher_spi_slave_drop_replies(void);

#ifdef __cplusplus
}
#endif

#endif /* WATCHER_SSCMA_SPI_SLAVE_H */
