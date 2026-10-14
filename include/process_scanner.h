#ifndef PROCESS_SCANNER_H
#define PROCESS_SCANNER_H

#include <sys/types.h>
#include <stddef.h>
#include <stdbool.h>

#define MAX_PROC_NAME_LEN 256
#define MAX_PATH_LEN 1024
#define DEFAULT_PROC_DIR "/proc"

typedef struct {
    pid_t pid;
    pid_t ppid;
    char name[MAX_PROC_NAME_LEN];
    char state;             /* e.g., 'R', 'S', 'Z', 'T', 'D', 'I' */
} ProcessInfo;

typedef struct {
    ProcessInfo *processes;
    size_t count;
    size_t capacity;
} ProcessList;

/*
 * Initialize an empty ProcessList.
 */
void process_list_init(ProcessList *list);

/*
 * Free all memory allocated within a ProcessList.
 */
void process_list_free(ProcessList *list);

/*
 * Parse a single process status file under {proc_dir}/{pid}/status.
 * Returns true if successfully read and parsed, false otherwise (e.g., terminated or inaccessible).
 */
bool parse_proc_status(const char *proc_dir, pid_t pid, ProcessInfo *info);

/*
 * Scan numeric PID directories under proc_dir (defaults to DEFAULT_PROC_DIR if proc_dir is NULL).
 * Populates list. Returns 0 on success, or -1 if the directory could not be opened.
 */
int scan_processes(ProcessList *list, const char *proc_dir);

/*
 * Filter processes in src having matching state character, adding them to dst.
 * dst must be initialized before calling.
 * Returns 0 on success, or -1 on memory allocation error.
 */
int filter_processes_by_state(const ProcessList *src, ProcessList *dst, char state);

/*
 * Sort processes in list in ascending order of PID.
 */
void process_list_sort_by_pid(ProcessList *list);

#endif /* PROCESS_SCANNER_H */
