/*
 * SSCMA SPI transport framing core for the SenseCAP Watcher HX6538 firmware.
 *
 * This file is intentionally free of WE2 SDK dependencies so the framing
 * rules can be unit-tested on a host (see tests/test_sscma_spi_framing.c).
 * It implements the wire format the ESP32-S3 sscma_client SPI transport
 * speaks (components/sscma_client/src/sscma_client_io_spi.c in
 * Seeed-Studio/SenseCAP-Watcher-Firmware):
 *
 *  - Every command phase clocks exactly 256 bytes: a 4-byte header
 *    [0x10, command, length_hi, length_lo], payload bytes at offset 4,
 *    and a 0xFF 0xFF trailer at offset 4+length (for WRITE, length <= 250;
 *    for READ/AVAILABLE/START/STOP/RESET the trailer sits at offset 4,5
 *    and length is the requested read length, or 0).
 *  - WRITE delivers command payload bytes (AT text), possibly split
 *    across several 250-byte packets.
 *  - READ clocks out pending reply bytes in a separate CS assertion
 *    that follows the READ header phase.
 *  - AVAILABLE is gated by the SYNC line (HX6538 GPIO output, high when
 *    reply bytes are pending) and returns the pending length as two
 *    big-endian bytes; 0xFFFF means "none pending".
 *  - RESET discards all pending reply bytes.
 */

#ifndef WATCHER_SSCMA_SPI_FRAMING_H
#define WATCHER_SSCMA_SPI_FRAMING_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define WATCHER_SPI_FEATURE_TRANSPORT 0x10u
#define WATCHER_SPI_CMD_READ 0x01u
#define WATCHER_SPI_CMD_WRITE 0x02u
#define WATCHER_SPI_CMD_AVAILABLE 0x03u
#define WATCHER_SPI_CMD_START 0x04u
#define WATCHER_SPI_CMD_STOP 0x05u
#define WATCHER_SPI_CMD_RESET 0x06u

#define WATCHER_SPI_MAX_WRITE_PAYLOAD 250u
#define WATCHER_SPI_MAX_READ_PAYLOAD 4095u
#define WATCHER_SPI_RX_PACKET_LEN 256u
#define WATCHER_SPI_HEADER_LEN 4u
#define WATCHER_SPI_TRAILER_BYTE 0xFFu
/* AVAILABLE must never report 0xFFFF as a real length: the ESP32 client
 * maps 0xFFFF back to "nothing pending". */
#define WATCHER_SPI_AVAILABLE_MAX 65534u

/* Parse one 256-byte packet received by the SPI slave.
 *  Returns 0 on success and fills cmd_out and len_out.
 *  Returns < 0 on error: -1 bad feature byte, -2 unknown command,
 *  -3 invalid length for the command, -4 bad trailer bytes. */
int watcher_spi_parse_packet(const uint8_t *pkt, uint8_t *cmd_out, uint16_t *len_out);

/* Encode the 2-byte big-endian pending-length reply for AVAILABLE.
 * pending_len is clamped to WATCHER_SPI_AVAILABLE_MAX. */
void watcher_spi_encode_available(uint8_t out[2], uint16_t pending_len);

/* Ordered byte-stream queue for reply bytes. READ commands drain the
 * stream from the front. Fixed ring buffer, no allocation. */
typedef struct {
    uint8_t *buf;
    size_t cap;
    size_t head;
    size_t len;
} watcher_tx_queue_t;

void watcher_tx_queue_init(watcher_tx_queue_t *q, uint8_t *buf, size_t cap);
/* Returns false (queue unchanged) when the data does not fit. */
bool watcher_tx_queue_enqueue(watcher_tx_queue_t *q, const uint8_t *data, size_t len);
size_t watcher_tx_queue_available(const watcher_tx_queue_t *q);
/* Removes and copies up to max_len bytes from the front. */
size_t watcher_tx_queue_drain(watcher_tx_queue_t *q, uint8_t *out, size_t max_len);
void watcher_tx_queue_reset(watcher_tx_queue_t *q);

/* AT command assembler: ESP32 WRITE payloads arrive in <= 250-byte
 * chunks; a complete AT command ends with '\n'. Feed payload bytes;
 * returns true when a complete command line is ready. On success,
 * *cmd_out points at the assembler buffer holding the NUL-terminated
 * command with the CRLF line ending stripped. The buffer is reused by
 * the next command; copy the command out before feeding more bytes.
 * Oversized commands (no '\n' within cap) are discarded through their
 * terminating newline, even across feed calls; the next command after
 * that newline is assembled normally. Each feed call holds bytes of a
 * single command (one SPI WRITE packet); bytes after the first '\n'
 * in a call are not processed. */
typedef struct {
    char *buf;
    size_t cap;
    size_t len;
    bool discarding;
} watcher_cmd_asm_t;

void watcher_cmd_asm_init(watcher_cmd_asm_t *a, char *buf, size_t cap);
bool watcher_cmd_asm_feed(watcher_cmd_asm_t *a, const uint8_t *data, size_t len,
                          char **cmd_out);
void watcher_cmd_asm_reset(watcher_cmd_asm_t *a);

#ifdef __cplusplus
}
#endif

#endif /* WATCHER_SSCMA_SPI_FRAMING_H */
