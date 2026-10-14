#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "cheat_detector.h"

static void test_detection_result_list_init_and_free(void) {
    printf("[TEST] Running test_detection_result_list_init_and_free...\n");
    DetectionResultList list;
    detection_result_list_init(&list);
    assert(list.items == NULL);
    assert(list.count == 0);
    assert(list.capacity == 0);

    detection_result_list_free(&list);
    assert(list.items == NULL);
    assert(list.count == 0);
    printf("  -> PASS\n");
}

static void test_evaluate_normal_process(void) {
    printf("[TEST] Running test_evaluate_normal_process...\n");
    ProcessInfo p1 = { .pid = 1, .ppid = 0, .name = "systemd", .state = 'S' };
    DetectionResult r1 = evaluate_process(&p1);
    assert(r1.pid == 1);
    assert(r1.status == DETECTION_NORMAL);
    assert(strcmp(detection_status_to_string(r1.status), "NORMAL") == 0);

    ProcessInfo p2 = { .pid = 100, .ppid = 1, .name = "bash", .state = 'S' };
    DetectionResult r2 = evaluate_process(&p2);
    assert(r2.status == DETECTION_NORMAL);
    printf("  -> PASS\n");
}

static void test_evaluate_suspicious_process(void) {
    printf("[TEST] Running test_evaluate_suspicious_process...\n");
    const char *suspicious_names[] = {
        "gdb", "lldb", "scanmem", "gameconqueror", "strace", "cheatengine"
    };
    size_t n = sizeof(suspicious_names) / sizeof(suspicious_names[0]);

    for (size_t i = 0; i < n; i++) {
        ProcessInfo p = { .pid = 500 + (pid_t)i, .ppid = 1, .state = 'S' };
        strncpy(p.name, suspicious_names[i], sizeof(p.name) - 1);
        DetectionResult r = evaluate_process(&p);
        assert(r.status == DETECTION_SUSPICIOUS);
        assert(strcmp(detection_status_to_string(r.status), "SUSPICIOUS") == 0);
        assert(strlen(r.reason) > 0);
    }
    printf("  -> PASS\n");
}

static void test_evaluate_confirmed_cheat_process(void) {
    printf("[TEST] Running test_evaluate_confirmed_cheat_process...\n");
    ProcessInfo p = { .pid = 777, .ppid = 1, .name = "cheat_test", .state = 'R' };
    DetectionResult r = evaluate_process(&p);
    assert(r.status == DETECTION_CONFIRMED_CHEAT);
    assert(strcmp(detection_status_to_string(r.status), "CONFIRMED_CHEAT") == 0);
    assert(strlen(r.reason) > 0);
    printf("  -> PASS\n");
}

static void test_detect_cheats_and_filter(void) {
    printf("[TEST] Running test_detect_cheats_and_filter...\n");
    ProcessInfo procs[] = {
        { .pid = 1, .ppid = 0, .name = "systemd", .state = 'S' },
        { .pid = 200, .ppid = 1, .name = "gdb", .state = 'S' },
        { .pid = 300, .ppid = 1, .name = "cheat_test", .state = 'R' },
        { .pid = 400, .ppid = 1, .name = "anticheat", .state = 'R' }
    };
    ProcessList plist = { .processes = procs, .count = 4, .capacity = 4 };

    DetectionResultList results;
    detection_result_list_init(&results);

    int rc = detect_cheats(&plist, &results);
    assert(rc == 0);
    assert(results.count == 4);

    DetectionResultList flagged;
    detection_result_list_init(&flagged);
    rc = filter_suspicious_or_cheats(&results, &flagged);
    assert(rc == 0);
    assert(flagged.count == 2);

    bool found_gdb = false;
    bool found_cheat_test = false;
    for (size_t i = 0; i < flagged.count; i++) {
        if (strcmp(flagged.items[i].name, "gdb") == 0) {
            found_gdb = true;
            assert(flagged.items[i].status == DETECTION_SUSPICIOUS);
        } else if (strcmp(flagged.items[i].name, "cheat_test") == 0) {
            found_cheat_test = true;
            assert(flagged.items[i].status == DETECTION_CONFIRMED_CHEAT);
        }
    }
    assert(found_gdb && found_cheat_test);

    detection_result_list_free(&flagged);
    detection_result_list_free(&results);
    printf("  -> PASS\n");
}

int main(void) {
    printf("========================================\n");
    printf("      RUNNING CHEAT DETECTOR TESTS      \n");
    printf("========================================\n");

    test_detection_result_list_init_and_free();
    test_evaluate_normal_process();
    test_evaluate_suspicious_process();
    test_evaluate_confirmed_cheat_process();
    test_detect_cheats_and_filter();

    printf("\nAll cheat detector tests passed!\n");
    return 0;
}
