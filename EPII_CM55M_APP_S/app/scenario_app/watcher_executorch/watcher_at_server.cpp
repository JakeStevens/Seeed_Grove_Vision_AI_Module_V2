/*
 * SSCMA-compatible AT command server. See watcher_at_server.h for the
 * contract. JSON fragments for image/classes reuse the vendored
 * send_result builders (img_2_json_str, results_2_json_str,
 * algo_tick_2_json_str) so the wire encoding matches the other WE2
 * apps in this SDK tree.
 */

#include "watcher_at_server.h"

#include <cstdlib>
#include <cstring>
#include <string>

#include "common_config.h"
#include "send_result.h"
#include "sscma_spi_slave.h"
#include "watcher_result.h"

extern "C" {
#include "xprintf.h"
}

/* Vendored helpers defined in send_result.cpp (C++ linkage). */
void el_base64_encode(const unsigned char *in, int in_len, char *out);
uint16_t el_crc16_maxim(const uint8_t *data, size_t length);

namespace {

const char *kCifar10Labels[10] = {
    "airplane", "automobile", "bird", "cat", "deer",
    "dog",      "frog",       "horse", "ship", "truck",
};
const char *kModelUuid = "watcher-executorch-mbv2-cifar10";
const char *kModelName = "MobileNetV2 CIFAR10 (ExecuTorch/Ethos-U55)";
const char *kModelVersion = "1.0.0";

watcher_stream_mode_t g_mode = WATCHER_STREAM_IDLE;
int g_remaining = 0; /* -1 = continuous */
uint32_t g_event_count = 0;
int g_opt_id = 1; /* SSCMA sensor opt: 1 = 416x416 preview */
int g_tscore = 0;
int g_tiou = 0;
int g_model_id = 0;

bool starts_with(const char *s, const char *prefix)
{
    return strncmp(s, prefix, strlen(prefix)) == 0;
}

/* Pointer just past the first '=' in cmd, or NULL. */
const char *after_eq(const char *cmd)
{
    const char *eq = strchr(cmd, '=');
    return (eq == NULL) ? NULL : eq + 1;
}

int parse_int_list(const char *args, int *out, int max_out)
{
    int n = 0;
    const char *p = args;
    if (args == NULL) {
        return 0;
    }
    while (*p != '\0' && n < max_out) {
        char *end = NULL;
        long v = strtol(p, &end, 10);
        if (end == p) {
            break;
        }
        out[n++] = (int)v;
        p = end;
        if (*p == ',') {
            p++;
        } else {
            break;
        }
    }
    return n;
}

std::string json_escape(const std::string &s)
{
    std::string out;
    out.reserve(s.size() + 2);
    for (char c : s) {
        if (c == '"' || c == '\\') {
            out += '\\';
        }
        if (c != '\n' && c != '\r') {
            out += c;
        }
    }
    return out;
}

std::string quoted(const std::string &s)
{
    return std::string("\"") + json_escape(s) + "\"";
}

void enqueue_frame(const std::string &frame)
{
    if (!watcher_spi_slave_enqueue_reply(
            reinterpret_cast<const uint8_t *>(frame.data()), frame.size())) {
        xprintf("watcher AT: reply queue full, dropped %u bytes\r\n",
                (unsigned)frame.size());
    }
}

void send_response(const char *name, const std::string &data_fragment)
{
    std::string frame = "\r{\"type\": 0, \"name\": \"";
    frame += name;
    frame += "\", \"code\": 0, \"data\": ";
    frame += data_fragment;
    frame += "}\n";
    enqueue_frame(frame);
}

void send_log_error(const char *cmd)
{
    /* Unknown command: SSCMA reports it as a type-2 log with code
     * CMD_EINVAL (5) and the command text in "data". */
    std::string frame = "\r{\"type\": 2, \"name\": \"LOG\", \"code\": 5, \"data\": ";
    frame += quoted(cmd);
    frame += "}\n";
    enqueue_frame(frame);
}

std::string model_info_data(void)
{
    std::string s = "{\"id\": ";
    s += std::to_string(g_model_id);
    s += ", \"type\": 4, \"address\": ";
    s += std::to_string((unsigned long)MOBILENET_CLASSIFICATION_FLASH_ADDR);
    s += ", \"size\": ";
    s += std::to_string((unsigned long)MOBILENET_CLASSIFICATION_MODEL_SIZE);
    s += "}";
    return s;
}

std::string sensor_data(void)
{
    std::string s = "{\"sensor\": {\"id\": 1, \"type\": 0, \"state\": 1, \"opt_id\": ";
    s += std::to_string(g_opt_id);
    s += ", \"opt_detail\": \"416x416\"}}";
    return s;
}

std::string model_info_json(void)
{
    std::string s = "{\"uuid\": \"";
    s += kModelUuid;
    s += "\", \"name\": \"";
    s += kModelName;
    s += "\", \"version\": \"";
    s += kModelVersion;
    s += "\", \"url\": \"\", \"checksum\": \"\", \"classes\": [";
    for (int i = 0; i < 10; i++) {
        if (i > 0) {
            s += ", ";
        }
        s += "\"";
        s += kCifar10Labels[i];
        s += "\"";
    }
    s += "]}";
    return s;
}

void send_info_reply(void)
{
    std::string info = model_info_json();
    size_t b64_len = ((info.size() + 2) / 3) * 4 + 1;
    std::string b64(b64_len, '\0');
    el_base64_encode(reinterpret_cast<const unsigned char *>(info.data()),
                     (int)info.size(), &b64[0]);
    /* el_base64_encode NUL-terminates; drop the padding byte. */
    b64.resize(strlen(b64.data()));
    uint16_t crc = el_crc16_maxim(reinterpret_cast<const uint8_t *>(info.data()),
                                  info.size());
    std::string data = "{\"crc16_maxim\": ";
    data += std::to_string(crc);
    data += ", \"info\": ";
    data += quoted(b64);
    data += "}";
    send_response("INFO?", data);
}

/* Scores stored in g_watcher_last_scores are already 0..100
 * percents from the linear quantized-logit map in cvapp_mb_cls.cpp
 * ((raw + 128) * 100 / 255, not softmax). They are emitted unchanged;
 * there is no logit mapping left to do here. */

/* "classes": [[score, target], ...], best-first. The vendored
 * template builder for this fragment lives only in send_result.cpp,
 * so the fragment is built locally to keep linkage simple. */
std::string classes_fragment(void)
{
    int count = g_watcher_last_count;
    std::string s = "\"classes\": [";
    if (count > WATCHER_MAX_TOP_CLASSES) {
        count = WATCHER_MAX_TOP_CLASSES;
    }
    for (int i = 0; i < count; i++) {
        if (i > 0) {
            s += ", ";
        }
        s += "[";
        s += std::to_string(g_watcher_last_scores[i]);
        s += ", ";
        s += std::to_string(g_watcher_last_targets[i]);
        s += "]";
    }
    s += "]";
    return s;
}

std::string image_fragment(const uint8_t *jpeg, uint32_t jpeg_len)
{
    el_img_t img;
    img.data = const_cast<uint8_t *>(jpeg);
    img.size = jpeg_len;
    img.width = 320;
    img.height = 240;
    img.format = EL_PIXEL_FORMAT_JPEG;
    img.rotate = EL_PIXEL_ROTATE_0;
    return img_2_json_str(&img);
}

std::string event_prefix(const char *name, uint32_t count)
{
    std::string s = "\r{\"type\": 1, \"name\": \"";
    s += name;
    s += "\", \"code\": 0, \"data\": {\"count\": ";
    s += std::to_string(count);
    return s;
}

} /* namespace */

void watcher_at_init(void)
{
    g_mode = WATCHER_STREAM_IDLE;
    g_remaining = 0;
    g_event_count = 0;
    g_opt_id = 1;
}

void watcher_at_reset_stream(void)
{
    g_mode = WATCHER_STREAM_IDLE;
    g_remaining = 0;
}

bool watcher_at_stream_active(void)
{
    return g_mode != WATCHER_STREAM_IDLE;
}

watcher_stream_mode_t watcher_at_stream_mode(void)
{
    return g_mode;
}

int watcher_at_stream_remaining(void)
{
    return g_remaining;
}

void watcher_at_consume_event(void)
{
    if (g_remaining > 0) {
        g_remaining--;
        if (g_remaining == 0) {
            g_mode = WATCHER_STREAM_IDLE;
        }
    }
}

void watcher_at_build_invoke_event(const uint8_t *jpeg, uint32_t jpeg_len,
                                   uint32_t algo_tick)
{
    std::string frame = event_prefix("INVOKE", g_event_count++);
    frame += ", \"resolution\": [416, 416], ";
    frame += classes_fragment();
    frame += ", ";
    frame += image_fragment(jpeg, jpeg_len);
    frame += ", ";
    frame += algo_tick_2_json_str(algo_tick);
    frame += "}}\n";
    enqueue_frame(frame);
}

void watcher_at_send_init_event(void)
{
    enqueue_frame("\r{\"type\": 1, \"name\": \"INIT@STAT\", \"code\": 0, "
                  "\"data\": {}}\n");
}

void watcher_at_build_sample_event(const uint8_t *jpeg, uint32_t jpeg_len)
{
    std::string frame = event_prefix("SAMPLE", g_event_count++);
    frame += ", \"resolution\": [416, 416], ";
    frame += image_fragment(jpeg, jpeg_len);
    frame += "}}\n";
    enqueue_frame(frame);
}

void watcher_at_handle_command(const char *cmd)
{
    int args[4];
    int nargs;
    const char *tail;

    if (cmd == NULL || cmd[0] == '\0') {
        return;
    }
    xprintf("watcher AT: %s\r\n", cmd);

    if (strcmp(cmd, "AT+ID?") == 0) {
        /* data must be a JSON string: the ESP32 client only accepts
         * string fields (cJSON_IsString), a bare number leaves its
         * info.id NULL. */
        send_response("ID?", "\"1\"");
        return;
    }
    if (strcmp(cmd, "AT+NAME?") == 0) {
        send_response("NAME?", quoted("SenseCAP Watcher (ExecuTorch)"));
        return;
    }
    if (strcmp(cmd, "AT+VER?") == 0) {
        send_response("VER?",
                      "{\"software\": \"watcher-executorch 1.0.0\", "
                      "\"hardware\": \"SenseCAP Watcher W1 (HX6538)\"}");
        return;
    }
    if (strcmp(cmd, "AT+INFO?") == 0) {
        send_info_reply();
        return;
    }
    if (starts_with(cmd, "AT+INFO=")) {
        /* Set-model-info payload (quoted base64); stored info is the
         * built-in reference model, so accept and acknowledge. */
        send_response("INFO", "\"\"");
        return;
    }
    if (strcmp(cmd, "AT+MODEL?") == 0) {
        send_response("MODEL?", model_info_data());
        return;
    }
    if (starts_with(cmd, "AT+MODEL=")) {
        nargs = parse_int_list(after_eq(cmd), args, 1);
        if (nargs == 1) {
            g_model_id = args[0];
        }
        send_response("MODEL", model_info_data());
        return;
    }
    if (strcmp(cmd, "AT+SENSOR?") == 0) {
        send_response("SENSOR?", sensor_data());
        return;
    }
    if (starts_with(cmd, "AT+SENSOR=")) {
        nargs = parse_int_list(after_eq(cmd), args, 3);
        if (nargs == 3) {
            g_opt_id = args[2];
        }
        send_response("SENSOR", sensor_data());
        return;
    }
    if (strcmp(cmd, "AT+BREAK") == 0) {
        watcher_spi_slave_drop_replies();
        watcher_at_reset_stream();
        send_response("BREAK", "{}");
        return;
    }
    if (strcmp(cmd, "AT+RST") == 0) {
        watcher_at_reset_stream();
        send_response("RST", "{}");
        return;
    }
    if (starts_with(cmd, "AT+INVOKE=")) {
        nargs = parse_int_list(after_eq(cmd), args, 3);
        int times = (nargs >= 1) ? args[0] : -1;
        g_mode = WATCHER_STREAM_INVOKE;
        g_remaining = (times == 0) ? 1 : times;
        send_response("INVOKE",
                      "{\"algorithm\": {\"type\": 4, \"categroy\": 3, "
                      "\"category\": 3}}");
        return;
    }
    if (starts_with(cmd, "AT+SAMPLE=")) {
        nargs = parse_int_list(after_eq(cmd), args, 1);
        int times = (nargs >= 1) ? args[0] : -1;
        g_mode = WATCHER_STREAM_SAMPLE;
        g_remaining = (times == 0) ? 1 : times;
        send_response("SAMPLE", "{}");
        return;
    }
    if (starts_with(cmd, "AT+TSCORE=")) {
        nargs = parse_int_list(after_eq(cmd), args, 1);
        if (nargs == 1) {
            g_tscore = args[0];
        }
        send_response("TSCORE", std::to_string(g_tscore));
        return;
    }
    if (strcmp(cmd, "AT+TSCORE?") == 0) {
        send_response("TSCORE?", std::to_string(g_tscore));
        return;
    }
    if (starts_with(cmd, "AT+TIOU=")) {
        nargs = parse_int_list(after_eq(cmd), args, 1);
        if (nargs == 1) {
            g_tiou = args[0];
        }
        send_response("TIOU", std::to_string(g_tiou));
        return;
    }
    if (strcmp(cmd, "AT+TIOU?") == 0) {
        send_response("TIOU?", std::to_string(g_tiou));
        return;
    }
    if (strcmp(cmd, "AT+STAT?") == 0) {
        std::string data = "{\"count\": ";
        data += std::to_string(g_event_count);
        data += "}";
        send_response("STAT?", data);
        return;
    }
    if (strcmp(cmd, "AT+ALGOS?") == 0) {
        send_response("ALGOS?", "{\"algos\": []}");
        return;
    }
    if (strcmp(cmd, "AT+MODELS?") == 0) {
        send_response("MODELS?", "{\"models\": []}");
        return;
    }
    if (strcmp(cmd, "AT+SENSORS?") == 0) {
        send_response("SENSORS?", "{\"sensors\": []}");
        return;
    }
    /* Accept-and-ignore host feature commands the factory flow may
     * send; their payloads do not change HX6538 behaviour here. */
    tail = after_eq(cmd);
    if (starts_with(cmd, "AT+LED") || starts_with(cmd, "AT+WIFI") ||
        starts_with(cmd, "AT+MQTTSERVER") || starts_with(cmd, "AT+MQTTPUBSUB") ||
        starts_with(cmd, "AT+ACTION") || starts_with(cmd, "AT+OTA")) {
        std::string name(cmd + 3, (tail != NULL) ? (size_t)(tail - 1 - (cmd + 3))
                                                : strlen(cmd + 3));
        /* Name is the token before '=' or the whole token for '?'. */
        size_t cut = name.find_first_of("=?");
        if (cut != std::string::npos) {
            name.resize(cut);
        }
        send_response(name.c_str(), "{}");
        return;
    }

    send_log_error(cmd);
}
