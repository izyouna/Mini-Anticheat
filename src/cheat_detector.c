#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "cheat_detector.h"

#define INITIAL_CAPACITY 32

/*
 * List of known suspicious tools/debuggers/injectors.
 * Matching these flags the process as SUSPICIOUS (never auto-terminated).
 */
static const char *SUSPICIOUS_TOOLS[] = {
    "gdb",
    "lldb",
    "scanmem",
    "gameconqueror",
    "strace",
    "cheatengine",
    "radare2",
    "frida",
    NULL
};

/*
 * List of controlled cheat simulators designed for this project.
 * Matching these flags the process as CONFIRMED_CHEAT.
 */
static const char *CONFIRMED_CHEAT_TOOLS[] = {
    "cheat_test",
    NULL
};

static bool contains_case_insensitive(const char *haystack, const char *needle) {
    if (!haystack || !needle) return false;
    size_t hlen = strlen(haystack);
    size_t nlen = strlen(needle);
    if (nlen > hlen) return false;

    for (size_t i = 0; i <= hlen - nlen; i++) {
        size_t j = 0;
        while (j < nlen && tolower((unsigned char)haystack[i + j]) == tolower((unsigned char)needle[j])) {
            j++;
        }
        if (j == nlen) return true;
    }
    return false;
}

void detection_result_list_init(DetectionResultList *list) {
    if (!list) return;
    list->items = NULL;
    list->count = 0;
    list->capacity = 0;
}

void detection_result_list_free(DetectionResultList *list) {
    if (!list) return;
    if (list->items) {
        free(list->items);
        list->items = NULL;
    }
    list->count = 0;
    list->capacity = 0;
}

static bool detection_result_list_add(DetectionResultList *list, const DetectionResult *item) {
    if (!list || !item) return false;

    if (list->count >= list->capacity) {
        size_t new_cap = (list->capacity == 0) ? INITIAL_CAPACITY : list->capacity * 2;
        DetectionResult *new_items = realloc(list->items, new_cap * sizeof(DetectionResult));
        if (!new_items) {
            perror("realloc detection results");
            return false;
        }
        list->items = new_items;
        list->capacity = new_cap;
    }

    list->items[list->count] = *item;
    list->count++;
    return true;
}

const char *detection_status_to_string(DetectionStatus status) {
    switch (status) {
        case DETECTION_NORMAL:
            return "NORMAL";
        case DETECTION_SUSPICIOUS:
            return "SUSPICIOUS";
        case DETECTION_CONFIRMED_CHEAT:
            return "CONFIRMED_CHEAT";
        default:
            return "UNKNOWN";
    }
}

DetectionResult evaluate_process(const ProcessInfo *proc) {
    DetectionResult result;
    memset(&result, 0, sizeof(result));

    if (!proc) {
        result.status = DETECTION_NORMAL;
        snprintf(result.reason, sizeof(result.reason), "Invalid process info");
        return result;
    }

    result.pid = proc->pid;
    snprintf(result.name, sizeof(result.name), "%s", proc->name);

    /* Rule 1: Check controlled cheat simulators (CONFIRMED_CHEAT) */
    for (size_t i = 0; CONFIRMED_CHEAT_TOOLS[i] != NULL; i++) {
        if (contains_case_insensitive(proc->name, CONFIRMED_CHEAT_TOOLS[i])) {
            result.status = DETECTION_CONFIRMED_CHEAT;
            snprintf(result.reason, sizeof(result.reason),
                     "Controlled cheat simulator detected: %s", CONFIRMED_CHEAT_TOOLS[i]);
            return result;
        }
    }

    /* Rule 2: Check known suspicious inspection/debugging tools (SUSPICIOUS) */
    for (size_t i = 0; SUSPICIOUS_TOOLS[i] != NULL; i++) {
        if (contains_case_insensitive(proc->name, SUSPICIOUS_TOOLS[i])) {
            result.status = DETECTION_SUSPICIOUS;
            snprintf(result.reason, sizeof(result.reason),
                     "Suspicious debugging/inspection tool signature: %s", SUSPICIOUS_TOOLS[i]);
            return result;
        }
    }

    /* Rule 3: Default normal */
    result.status = DETECTION_NORMAL;
    snprintf(result.reason, sizeof(result.reason), "Normal process behavior");
    return result;
}

int detect_cheats(const ProcessList *procs, DetectionResultList *results) {
    if (!procs || !results) return -1;

    for (size_t i = 0; i < procs->count; i++) {
        DetectionResult res = evaluate_process(&procs->processes[i]);
        if (!detection_result_list_add(results, &res)) {
            return -1;
        }
    }
    return 0;
}

int filter_suspicious_or_cheats(const DetectionResultList *src, DetectionResultList *dst) {
    if (!src || !dst) return -1;

    for (size_t i = 0; i < src->count; i++) {
        if (src->items[i].status == DETECTION_SUSPICIOUS ||
            src->items[i].status == DETECTION_CONFIRMED_CHEAT) {
            if (!detection_result_list_add(dst, &src->items[i])) {
                return -1;
            }
        }
    }
    return 0;
}
