#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "process_scanner.h"

static void print_banner(void) {
    printf("====================================\n");
    printf("       MINI ANTI-CHEAT v0.1         \n");
    printf("         PROCESS SCANNER            \n");
    printf("====================================\n\n");
}

static void print_usage(const char *prog_name) {
    printf("Usage: %s [OPTIONS]\n\n", prog_name);
    printf("Options:\n");
    printf("  --zombies           Filter and display only zombie processes\n");
    printf("  --proc-dir <dir>    Custom /proc directory path (for testing)\n");
    printf("  -h, --help          Show this help message\n");
}

static void print_process_table(const ProcessList *list) {
    printf("%-8s %-8s %-20s %-5s\n", "PID", "PPID", "NAME", "STATE");
    printf("----------------------------------------\n");
    for (size_t i = 0; i < list->count; i++) {
        printf("%-8d %-8d %-20.20s %-5c\n",
               (int)list->processes[i].pid,
               (int)list->processes[i].ppid,
               list->processes[i].name,
               list->processes[i].state);
    }
    printf("\n");
}

int main(int argc, char *argv[]) {
    bool filter_zombies = false;
    const char *custom_proc_dir = NULL;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--zombies") == 0) {
            filter_zombies = true;
        } else if (strcmp(argv[i], "--proc-dir") == 0) {
            if (i + 1 < argc) {
                custom_proc_dir = argv[++i];
            } else {
                fprintf(stderr, "Error: --proc-dir requires a directory path argument.\n");
                return 1;
            }
        } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return 0;
        } else {
            fprintf(stderr, "Unknown option: %s\n", argv[i]);
            print_usage(argv[0]);
            return 1;
        }
    }

    print_banner();

    ProcessList all_procs;
    process_list_init(&all_procs);

    const char *target_proc_dir = custom_proc_dir ? custom_proc_dir : DEFAULT_PROC_DIR;
    if (scan_processes(&all_procs, target_proc_dir) != 0) {
        fprintf(stderr, "Error: Unable to access %s.\n", target_proc_dir);
        fprintf(stderr, "Note: Mini Anti-Cheat requires the Linux /proc filesystem.\n");
        fprintf(stderr, "Please run on Ubuntu Linux, Ubuntu VM (UTM), or WSL2.\n");
        process_list_free(&all_procs);
        return 1;
    }

    process_list_sort_by_pid(&all_procs);

    if (filter_zombies) {
        ProcessList zombie_procs;
        process_list_init(&zombie_procs);
        if (filter_processes_by_state(&all_procs, &zombie_procs, 'Z') == 0) {
            print_process_table(&zombie_procs);
            printf("Processes scanned: %zu\n", all_procs.count);
            printf("Zombie processes displayed: %zu\n", zombie_procs.count);
        } else {
            fprintf(stderr, "Error filtering processes.\n");
        }
        process_list_free(&zombie_procs);
    } else {
        print_process_table(&all_procs);
        printf("Processes scanned: %zu\n", all_procs.count);
        printf("Processes displayed: %zu\n", all_procs.count);
    }

    process_list_free(&all_procs);
    return 0;
}
