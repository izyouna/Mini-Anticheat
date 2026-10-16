#ifndef _DEFAULT_SOURCE
#define _DEFAULT_SOURCE
#endif
#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "process_scanner.h"

static void test_process_list_init_and_free(void) {
    printf("[TEST] Running test_process_list_init_and_free...\n");
    ProcessList list;
    process_list_init(&list);
    assert(list.processes == NULL);
    assert(list.count == 0);
    assert(list.capacity == 0);
    process_list_free(&list);
    assert(list.processes == NULL);
    assert(list.count == 0);
    assert(list.capacity == 0);
    printf("  -> PASS\n");
}

static void test_parse_proc_status_missing(void) {
    printf("[TEST] Running test_parse_proc_status_missing...\n");
    ProcessInfo info;
    memset(&info, 0, sizeof(info));
    /* PID 999999 should not exist in /proc or non-existent path */
    bool ok = parse_proc_status("/tmp/nonexistent_proc_dir_12345", 999999, &info);
    assert(!ok);
    printf("  -> PASS\n");
}

static void create_mock_proc_entry(const char *base_dir, int pid, const char *name, char state, int ppid) {
    char dir_path[512];
    snprintf(dir_path, sizeof(dir_path), "%s/%d", base_dir, pid);
    mkdir(dir_path, 0755);

    char status_path[1024];
    snprintf(status_path, sizeof(status_path), "%s/%d/status", base_dir, pid);
    FILE *f = fopen(status_path, "w");
    assert(f != NULL);
    fprintf(f, "Name:\t%s\n", name);
    fprintf(f, "Umask:\t0022\n");
    fprintf(f, "State:\t%c (sleeping)\n", state);
    fprintf(f, "Tgid:\t%d\n", pid);
    fprintf(f, "Ngid:\t0\n");
    fprintf(f, "Pid:\t%d\n", pid);
    fprintf(f, "PPid:\t%d\n", ppid);
    fprintf(f, "TracerPid:\t0\n");
    fclose(f);
}

static void test_scan_and_filter_mock(void) {
    printf("[TEST] Running test_scan_and_filter_mock...\n");
    char template[] = "/tmp/mock_proc_XXXXXX";
    char *mock_dir = mkdtemp(template);
    assert(mock_dir != NULL);

    /* Create entries */
    create_mock_proc_entry(mock_dir, 1, "systemd", 'S', 0);
    create_mock_proc_entry(mock_dir, 42, "game", 'R', 1);
    create_mock_proc_entry(mock_dir, 99, "zombie_proc", 'Z', 42);

    /* Also create non-pid directory and file */
    char sys_dir[512];
    snprintf(sys_dir, sizeof(sys_dir), "%s/sys", mock_dir);
    mkdir(sys_dir, 0755);

    char cpuinfo[512];
    snprintf(cpuinfo, sizeof(cpuinfo), "%s/cpuinfo", mock_dir);
    FILE *fcpu = fopen(cpuinfo, "w");
    if (fcpu) {
        fprintf(fcpu, "processor : 0\n");
        fclose(fcpu);
    }

    /* Test scanning */
    ProcessList list;
    process_list_init(&list);
    int res = scan_processes(&list, mock_dir);
    assert(res == 0);
    assert(list.count == 3);

    bool found_1 = false, found_42 = false, found_99 = false;
    for (size_t i = 0; i < list.count; i++) {
        if (list.processes[i].pid == 1) {
            found_1 = true;
            assert(strcmp(list.processes[i].name, "systemd") == 0);
            assert(list.processes[i].state == 'S');
            assert(list.processes[i].ppid == 0);
        } else if (list.processes[i].pid == 42) {
            found_42 = true;
            assert(strcmp(list.processes[i].name, "game") == 0);
            assert(list.processes[i].state == 'R');
            assert(list.processes[i].ppid == 1);
        } else if (list.processes[i].pid == 99) {
            found_99 = true;
            assert(strcmp(list.processes[i].name, "zombie_proc") == 0);
            assert(list.processes[i].state == 'Z');
            assert(list.processes[i].ppid == 42);
        }
    }
    assert(found_1 && found_42 && found_99);

    /* Test filter by state 'Z' */
    ProcessList zombies;
    process_list_init(&zombies);
    res = filter_processes_by_state(&list, &zombies, 'Z');
    assert(res == 0);
    assert(zombies.count == 1);
    assert(zombies.processes[0].pid == 99);
    assert(zombies.processes[0].state == 'Z');

    /* Cleanup mock directory */
    char cleanup_cmd[512];
    snprintf(cleanup_cmd, sizeof(cleanup_cmd), "rm -rf %s", mock_dir);
    int ret = system(cleanup_cmd);
    (void)ret;

    process_list_free(&zombies);
    process_list_free(&list);
    printf("  -> PASS\n");
}

static void test_process_list_sort(void) {
    printf("[TEST] Running test_process_list_sort...\n");
    ProcessList list;
    process_list_init(&list);

    ProcessInfo p3 = { .pid = 300, .ppid = 1, .name = "p3", .state = 'S' };
    ProcessInfo p1 = { .pid = 10, .ppid = 1, .name = "p1", .state = 'S' };
    ProcessInfo p2 = { .pid = 50, .ppid = 1, .name = "p2", .state = 'S' };

    /* Use filter_processes_by_state to insert into list, or scan */
    ProcessList mock_src;
    process_list_init(&mock_src);
    filter_processes_by_state(&(ProcessList){ .processes = (ProcessInfo[]){ p3, p1, p2 }, .count = 3 }, &list, 'S');

    assert(list.count == 3);
    assert(list.processes[0].pid == 300);

    process_list_sort_by_pid(&list);
    assert(list.processes[0].pid == 10);
    assert(list.processes[1].pid == 50);
    assert(list.processes[2].pid == 300);

    process_list_free(&list);
    printf("  -> PASS\n");
}

int main(void) {
    printf("========================================\n");
    printf("      RUNNING PROCESS SCANNER TESTS     \n");
    printf("========================================\n");

    test_process_list_init_and_free();
    test_parse_proc_status_missing();
    test_scan_and_filter_mock();
    test_process_list_sort();

    printf("\nAll process scanner tests passed!\n");
    return 0;
}
