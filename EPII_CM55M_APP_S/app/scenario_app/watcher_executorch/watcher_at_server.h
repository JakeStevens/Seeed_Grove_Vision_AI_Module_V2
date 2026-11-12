/*
 * SSCMA-compatible AT command server for the SenseCAP Watcher HX6538
 * firmware. Commands arrive over the SPI-slave transport (see
 * sscma_spi_slave.h); replies and INVOKE/SAMPLE events are queued
 * there as \r{JSON}\n frames. The reply names and payload shapes
 * match what the ESP32-S3 sscma_client expects (reply-name matching
 * in sscma_client_request, algorithm/class/image parsing in the
 * factory tf_module_ai_camera task).
 *
 * Single datapath note: this firmware always runs the proven
 * 320x240 datapath (INP subsample 2x) and labels events with
 * resolution [416, 416], the preview resolution the ESP32 inference
 * path routes on. The JPEG payload itself carries its true 320x240
 * dimensions in its header, which the ESP32 JPEG decoder reads.
 */

#ifndef WATCHER_AT_SERVER_H
#define WATCHER_AT_SERVER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    WATCHER_STREAM_IDLE = 0,
    WATCHER_STREAM_INVOKE,
    WATCHER_STREAM_SAMPLE
} watcher_stream_mode_t;

void watcher_at_init(void);

/* Handle one complete AT command line (CRLF already stripped).
 * Enqueues the type-0 response and, for INVOKE/SAMPLE, arms the
 * event stream. Called from watcher_spi_slave_poll() context. */
void watcher_at_handle_command(const char *cmd);

/* Stop any running INVOKE/SAMPLE stream (AT+BREAK / SPI RESET/STOP). */
void watcher_at_reset_stream(void);

bool watcher_at_stream_active(void);
watcher_stream_mode_t watcher_at_stream_mode(void);
/* Events remaining in the armed stream, or -1 for continuous (-1 times). */
int watcher_at_stream_remaining(void);

/* Build and queue one INVOKE event from the just-captured JPEG frame
 * and the last completed inference result (watcher_result.h).
 * Call after cv_mb_cls_run() on a frame while streaming INVOKE. */
void watcher_at_build_invoke_event(const uint8_t *jpeg, uint32_t jpeg_len,
                                   uint32_t algo_tick);

/* Build and queue one SAMPLE (image-only) event. */
void watcher_at_build_sample_event(const uint8_t *jpeg, uint32_t jpeg_len);

/* Queue the INIT@STAT connect event once per boot. The ESP32-S3
 * client only starts its camera flow (EVENT_START) when it receives
 * a reply whose name contains INIT@STAT; call this once the datapath
 * is running and the SPI slave is armed. The payload is ignored by
 * the client beyond the name. */
void watcher_at_send_init_event(void);

/* Account for one emitted event; stops the stream at the requested
 * count. Call after building an event. */
void watcher_at_consume_event(void);

#ifdef __cplusplus
}
#endif

#endif /* WATCHER_AT_SERVER_H */
