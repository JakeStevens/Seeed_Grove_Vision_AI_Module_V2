/*
 * SSCMA SPI transport framing core. See sscma_spi_framing.h for the wire
 * format contract. No WE2 SDK dependencies: host-testable.
 */

#include "sscma_spi_framing.h"

#include <string.h>

int watcher_spi_parse_packet(const uint8_t *pkt, uint8_t *cmd_out, uint16_t *len_out)
{
    uint8_t cmd;
    uint16_t len;
    size_t trailer_at;

    if (pkt == NULL || cmd_out == NULL || len_out == NULL) {
        return -1;
    }
    if (pkt[0] != WATCHER_SPI_FEATURE_TRANSPORT) {
        return -1;
    }
    cmd = pkt[1];
    switch (cmd) {
    case WATCHER_SPI_CMD_READ:
    case WATCHER_SPI_CMD_WRITE:
    case WATCHER_SPI_CMD_AVAILABLE:
    case WATCHER_SPI_CMD_START:
    case WATCHER_SPI_CMD_STOP:
    case WATCHER_SPI_CMD_RESET:
        break;
    default:
        return -2;
    }
    len = (uint16_t)(((uint16_t)pkt[2] << 8) | pkt[3]);
    switch (cmd) {
    case WATCHER_SPI_CMD_WRITE:
        if (len > WATCHER_SPI_MAX_WRITE_PAYLOAD) {
            return -3;
        }
        trailer_at = WATCHER_SPI_HEADER_LEN + len;
        break;
    case WATCHER_SPI_CMD_READ:
        if (len == 0 || len > WATCHER_SPI_MAX_READ_PAYLOAD) {
            return -3;
        }
        trailer_at = WATCHER_SPI_HEADER_LEN;
        break;
    default:
        if (len != 0) {
            return -3;
        }
        trailer_at = WATCHER_SPI_HEADER_LEN;
        break;
    }
    if (trailer_at + 1 >= WATCHER_SPI_RX_PACKET_LEN) {
        return -4;
    }
    if (pkt[trailer_at] != WATCHER_SPI_TRAILER_BYTE ||
        pkt[trailer_at + 1] != WATCHER_SPI_TRAILER_BYTE) {
        return -4;
    }
    *cmd_out = cmd;
    *len_out = len;
    return 0;
}

void watcher_spi_encode_available(uint8_t out[2], uint16_t pending_len)
{
    uint16_t v = pending_len;
    if (v > WATCHER_SPI_AVAILABLE_MAX) {
        v = WATCHER_SPI_AVAILABLE_MAX;
    }
    out[0] = (uint8_t)(v >> 8);
    out[1] = (uint8_t)(v & 0xFFu);
}

void watcher_tx_queue_init(watcher_tx_queue_t *q, uint8_t *buf, size_t cap)
{
    q->buf = buf;
    q->cap = cap;
    q->head = 0;
    q->len = 0;
}

bool watcher_tx_queue_enqueue(watcher_tx_queue_t *q, const uint8_t *data, size_t len)
{
    size_t i;
    size_t tail;
    if (q == NULL || q->buf == NULL || (len > 0 && data == NULL)) {
        return false;
    }
    if (len > q->cap - q->len) {
        return false;
    }
    tail = (q->head + q->len) % q->cap;
    for (i = 0; i < len; i++) {
        q->buf[tail] = data[i];
        tail++;
        if (tail == q->cap) {
            tail = 0;
        }
    }
    q->len += len;
    return true;
}

size_t watcher_tx_queue_available(const watcher_tx_queue_t *q)
{
    return (q == NULL) ? 0 : q->len;
}

size_t watcher_tx_queue_drain(watcher_tx_queue_t *q, uint8_t *out, size_t max_len)
{
    size_t n;
    size_t i;
    if (q == NULL || q->buf == NULL || out == NULL) {
        return 0;
    }
    n = (max_len < q->len) ? max_len : q->len;
    for (i = 0; i < n; i++) {
        out[i] = q->buf[q->head];
        q->head++;
        if (q->head == q->cap) {
            q->head = 0;
        }
    }
    q->len -= n;
    return n;
}

void watcher_tx_queue_reset(watcher_tx_queue_t *q)
{
    if (q != NULL) {
        q->head = 0;
        q->len = 0;
    }
}

void watcher_cmd_asm_init(watcher_cmd_asm_t *a, char *buf, size_t cap)
{
    a->buf = buf;
    a->cap = cap;
    a->len = 0;
    a->discarding = false;
    if (cap > 0) {
        buf[0] = '\0';
    }
}

void watcher_cmd_asm_reset(watcher_cmd_asm_t *a)
{
    a->len = 0;
    a->discarding = false;
    if (a->cap > 0) {
        a->buf[0] = '\0';
    }
}

bool watcher_cmd_asm_feed(watcher_cmd_asm_t *a, const uint8_t *data, size_t len,
                          char **cmd_out)
{
    size_t i;
    if (a == NULL || a->buf == NULL || a->cap == 0 || (len > 0 && data == NULL)) {
        return false;
    }
    for (i = 0; i < len; i++) {
        char c = (char)data[i];
        if (a->discarding) {
            if (c == '\n') {
                a->discarding = false;
            }
            continue;
        }
        if (c == '\n') {
            /* Strip a trailing CR; terminate; report the command. */
            if (a->len > 0 && a->buf[a->len - 1] == '\r') {
                a->len--;
            }
            a->buf[a->len] = '\0';
            if (cmd_out != NULL) {
                *cmd_out = a->buf;
            }
            a->len = 0;
            return true;
        }
        if (a->len + 1 >= a->cap) {
            /* No room for this byte plus NUL: discard the rest of
             * this line, through its terminating newline. */
            a->len = 0;
            a->buf[0] = '\0';
            a->discarding = true;
            continue;
        }
        a->buf[a->len++] = c;
        a->buf[a->len] = '\0';
    }
    return false;
}
