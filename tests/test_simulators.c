#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "cheat_detector.h"

static void test_simulator_detection_rules(void) {
    printf("[TEST] Running test_simulator_detection_rules...\n");

    /* Game simulator should be evaluated as NORMAL */
    ProcessInfo game_proc = { .pid = 1234, .ppid = 1, .name = "game", .state = 'R' };
    DetectionResult res_game = evaluate_process(&game_proc);
    assert(res_game.status == DETECTION_NORMAL);
    assert(strcmp(detection_status_to_string(res_game.status), "NORMAL") == 0);

    /* Cheat test simulator should be evaluated as CONFIRMED_CHEAT */
    ProcessInfo cheat_proc = { .pid = 5678, .ppid = 1, .name = "cheat_test", .state = 'R' };
    DetectionResult res_cheat = evaluate_process(&cheat_proc);
    assert(res_cheat.status == DETECTION_CONFIRMED_CHEAT);
    assert(strcmp(detection_status_to_string(res_cheat.status), "CONFIRMED_CHEAT") == 0);

    printf("  -> PASS\n");
}

static void test_game_execution_ticks(void) {
    printf("[TEST] Running test_game_execution_ticks...\n");
    int rc = system("./game --ticks 2 --interval 10 > /dev/null 2>&1");
    assert(rc == 0);
    printf("  -> PASS\n");
}

static void test_cheat_test_execution_duration(void) {
    printf("[TEST] Running test_cheat_test_execution_duration...\n");
    int rc = system("./cheat_test --duration 1 > /dev/null 2>&1");
    assert(rc == 0);
    printf("  -> PASS\n");
}

int main(void) {
    printf("========================================\n");
    printf("      RUNNING SIMULATOR TESTS           \n");
    printf("========================================\n");

    test_simulator_detection_rules();
    test_game_execution_ticks();
    test_cheat_test_execution_duration();

    printf("\nAll simulator tests passed!\n");
    return 0;
}
