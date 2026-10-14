#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include "process_scanner.h"

#define INITIAL_CAPACITY 64

void process_list_init(ProcessList *list) {
    if (!list) return;
    list->processes = NULL;
    list->count = 0;
    list->capacity = 0;
}

void process_list_free(ProcessList *list) {
    if (!list) return;
    if (list->processes) {
        free(list->processes);
        list->processes = NULL;
    }
    list->count = 0;
    list->capacity = 0;
}

static bool process_list_add(ProcessList *list, const ProcessInfo *info) {
    if (!list || !info) return false;

    if (list->count >= list->capacity) {
        size_t new_capacity = (list->capacity == 0) ? INITIAL_CAPACITY : list->capacity * 2;
        ProcessInfo *new_proc = realloc(list->processes, new_capacity * sizeof(ProcessInfo));
        if (!new_proc) {
            perror("realloc process list");
            return false;
        }
        list->processes = new_proc;
        list->capacity = new_capacity;
    }

    list->processes[list->count] = *info;
    list->count++;
    return true;
}

static bool is_numeric_str(const char *s) {
    if (!s || *s == '\0') return false;
    for (size_t i = 0; s[i] != '\0'; i++) {
        if (!isdigit((unsigned char)s[i])) {
            return false;
        }
    }
    return true;
}

static void trim_trailing_newline(char *s) {
    if (!s) return;
    size_t len = strlen(s);
    while (len > 0 && (s[len - 1] == '\n' || s[len - 1] == '\r')) {
        s[len - 1] = '\0';
        len--;
    }
}

bool parse_proc_status(const char *proc_dir, pid_t pid, ProcessInfo *info) {
    if (!proc_dir || !info || pid <= 0) return false;

    char status_path[MAX_PATH_LEN];
    int n = snprintf(status_path, sizeof(status_path), "%s/%d/status", proc_dir, (int)pid);
    if (n < 0 || (size_t)n >= sizeof(status_path)) {
        return false;
    }

    FILE *f = fopen(status_path, "r");
    if (!f) {
        /* Process terminated or permission denied - not an unrecoverable failure */
        return false;
    }

    memset(info, 0, sizeof(*info));
    info->pid = pid;

    char line[512];
    bool has_name = false;
    bool has_state = false;

    while (fgets(line, sizeof(line), f)) {
        if (strncmp(line, "Name:", 5) == 0) {
            char *p = line + 5;
            while (*p == ' ' || *p == '\t') p++;
            trim_trailing_newline(p);
            strncpy(info->name, p, sizeof(info->name) - 1);
            info->name[sizeof(info->name) - 1] = '\0';
            has_name = true;
        } else if (strncmp(line, "State:", 6) == 0) {
            char *p = line + 6;
            while (*p == ' ' || *p == '\t') p++;
            if (*p != '\0') {
                info->state = *p;
                has_state = true;
            }
        } else if (strncmp(line, "PPid:", 5) == 0) {
            char *p = line + 5;
            while (*p == ' ' || *p == '\t') p++;
            info->ppid = (pid_t)strtol(p, NULL, 10);
        }
    }

    fclose(f);
    return (has_name && has_state);
}

int scan_processes(ProcessList *list, const char *proc_dir) {
    if (!list) return -1;

    const char *dir_path = proc_dir ? proc_dir : DEFAULT_PROC_DIR;
    DIR *dir = opendir(dir_path);
    if (!dir) {
        /* On non-Linux systems without /proc or inaccessible directory */
        return -1;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (is_numeric_str(entry->d_name)) {
            pid_t pid = (pid_t)strtol(entry->d_name, NULL, 10);
            if (pid > 0) {
                ProcessInfo info;
                if (parse_proc_status(dir_path, pid, &info)) {
                    if (!process_list_add(list, &info)) {
                        closedir(dir);
                        return -1;
                    }
                }
            }
        }
    }

    closedir(dir);
    return 0;
}

int filter_processes_by_state(const ProcessList *src, ProcessList *dst, char state) {
    if (!src || !dst) return -1;

    for (size_t i = 0; i < src->count; i++) {
        if (src->processes[i].state == state) {
            if (!process_list_add(dst, &src->processes[i])) {
                return -1;
            }
        }
    }
    return 0;
}

static int compare_process_pid(const void *a, const void *b) {
    const ProcessInfo *pa = (const ProcessInfo *)a;
    const ProcessInfo *pb = (const ProcessInfo *)b;
    if (pa->pid < pb->pid) return -1;
    if (pa->pid > pb->pid) return 1;
    return 0;
}

void process_list_sort_by_pid(ProcessList *list) {
    if (!list || !list->processes || list->count < 2) return;
    qsort(list->processes, list->count, sizeof(ProcessInfo), compare_process_pid);
}

