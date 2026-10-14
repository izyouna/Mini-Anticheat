#ifndef CHEAT_DETECTOR_H
#define CHEAT_DETECTOR_H

#include <stdbool.h>
#include <stddef.h>
#include "process_scanner.h"

#define MAX_REASON_LEN 256

typedef enum {
    DETECTION_NORMAL = 0,
    DETECTION_SUSPICIOUS = 1,
    DETECTION_CONFIRMED_CHEAT = 2
} DetectionStatus;

typedef struct {
    pid_t pid;
    char name[MAX_PROC_NAME_LEN];
    DetectionStatus status;
    char reason[MAX_REASON_LEN];
} DetectionResult;

typedef struct {
    DetectionResult *items;
    size_t count;
    size_t capacity;
} DetectionResultList;

/*
 * Initialize an empty DetectionResultList.
 */
void detection_result_list_init(DetectionResultList *list);

/*
 * Free memory allocated within a DetectionResultList.
 */
void detection_result_list_free(DetectionResultList *list);

/*
 * Convert DetectionStatus enum to a readable string ("NORMAL", "SUSPICIOUS", "CONFIRMED_CHEAT").
 */
const char *detection_status_to_string(DetectionStatus status);

/*
 * Evaluates a single process against rule-based detection policies.
 */
DetectionResult evaluate_process(const ProcessInfo *proc);

/*
 * Evaluates all processes in the given ProcessList.
 * Populates results. Returns 0 on success, -1 on allocation failure.
 */
int detect_cheats(const ProcessList *procs, DetectionResultList *results);

/*
 * Filters detection results to include only SUSPICIOUS or CONFIRMED_CHEAT items.
 * dst must be initialized before calling. Returns 0 on success, -1 on error.
 */
int filter_suspicious_or_cheats(const DetectionResultList *src, DetectionResultList *dst);

#endif /* CHEAT_DETECTOR_H */
