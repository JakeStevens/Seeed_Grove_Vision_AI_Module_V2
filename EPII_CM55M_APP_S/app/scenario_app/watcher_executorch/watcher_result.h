/*
 * Last completed inference result, shared between the ExecuTorch
 * classification path (cvapp_mb_cls.cpp) and the AT event builder
 * (watcher_at_server.cpp). Both run in the datapath event context on
 * bare metal (no RTOS preemption between them), so no locking is used.
 * Up to five entries for classification: targets are class indices
 * best-first, scores are 0..100 percents from the linear
 * quantized-logit map (raw + 128) * 100 / 255, not softmax.
 */

#ifndef WATCHER_RESULT_H
#define WATCHER_RESULT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define WATCHER_MAX_TOP_CLASSES 5

extern volatile int g_watcher_last_count;
extern volatile int g_watcher_last_targets[WATCHER_MAX_TOP_CLASSES];
extern volatile int g_watcher_last_scores[WATCHER_MAX_TOP_CLASSES];
/* Increments after every completed inference. */
extern volatile uint32_t g_watcher_last_seq;

#ifdef __cplusplus
}
#endif

#endif /* WATCHER_RESULT_H */
