#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "process_scanner.h"
#include "cheat_detector.h"

static void print_banner(void) {
    printf("====================================\n");
    printf("       MINI ANTI-CHEAT v0.2         \n");
    printf("   PROCESS SCANNER & CHEAT DETECTOR \n");
    printf("====================================\n\n");
}

static void print_usage(const char *prog_name) {
    printf("Usage: %s [OPTIONS]\n\n", prog_name);
    printf("Options:\n");
    printf("  --detect            Run cheat detection engine on scanned processes\n");
    printf("  --alerts-only       Display only SUSPICIOUS and CONFIRMED_CHEAT processes\n");
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

static void print_detection_table(const DetectionResultList *list) {
    printf("%-8s %-20s %-16s %s\n", "PID", "NAME", "STATUS", "REASON");
    printf("--------------------------------------------------------------------------------\n");
    for (size_t i = 0; i < list->count; i++) {
        printf("%-8d %-20.20s %-16s %s\n",
               (int)list->items[i].pid,
               list->items[i].name,
               detection_status_to_string(list->items[i].status),
               list->items[i].reason);
    }
    printf("\n");
}

int main(int argc, char *argv[]) {
    bool filter_zombies = false;
    bool run_detection = false;
    bool alerts_only = false;
    const char *custom_proc_dir = NULL;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--zombies") == 0) {
            filter_zombies = true;
        } else if (strcmp(argv[i], "--detect") == 0) {
            run_detection = true;
        } else if (strcmp(argv[i], "--alerts-only") == 0) {
            run_detection = true;
            alerts_only = true;
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

    if (run_detection) {
        DetectionResultList detection_results;
        detection_result_list_init(&detection_results);

        if (detect_cheats(&all_procs, &detection_results) != 0) {
            fprintf(stderr, "Error running detection engine.\n");
            detection_result_list_free(&detection_results);
            process_list_free(&all_procs);
            return 1;
        }

        size_t count_normal = 0;
        size_t count_suspicious = 0;
        size_t count_confirmed = 0;

        for (size_t i = 0; i < detection_results.count; i++) {
            if (detection_results.items[i].status == DETECTION_NORMAL) count_normal++;
            else if (detection_results.items[i].status == DETECTION_SUSPICIOUS) count_suspicious++;
            else if (detection_results.items[i].status == DETECTION_CONFIRMED_CHEAT) count_confirmed++;
        }

        if (alerts_only) {
            DetectionResultList alerts;
            detection_result_list_init(&alerts);
            filter_suspicious_or_cheats(&detection_results, &alerts);
            print_detection_table(&alerts);
            printf("Total scanned: %zu\n", all_procs.count);
            printf("Alerts displayed: %zu (Suspicious: %zu, Confirmed Cheat: %zu)\n",
                   alerts.count, count_suspicious, count_confirmed);
            detection_result_list_free(&alerts);
        } else {
            print_detection_table(&detection_results);
            printf("Total scanned: %zu\n", all_procs.count);
            printf("Summary: Normal: %zu | Suspicious: %zu | Confirmed Cheat: %zu\n",
                   count_normal, count_suspicious, count_confirmed);
        }

        detection_result_list_free(&detection_results);
    } else if (filter_zombies) {
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
