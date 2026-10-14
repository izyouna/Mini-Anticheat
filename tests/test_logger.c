#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include "logger.h"

static void test_logger_init_and_close(void) {
    printf("[TEST] Running test_logger_init_and_close...\n");
    char temp_log[] = "/tmp/test_log_XXXXXX";
    int fd = mkstemp(temp_log);
    assert(fd >= 0);
    close(fd);

    int rc = logger_init(temp_log);
    assert(rc == 0);
    assert(logger_is_open());

    logger_close();
    assert(!logger_is_open());

    unlink(temp_log);
    printf("  -> PASS\n");
}

static void test_log_event_writing(void) {
    printf("[TEST] Running test_log_event_writing...\n");
    char temp_log[] = "/tmp/test_log_events_XXXXXX";
    int fd = mkstemp(temp_log);
    assert(fd >= 0);
    close(fd);

    int rc = logger_init(temp_log);
    assert(rc == 0);

    /* Write events */
    rc = log_event("INFO", 1, "systemd", "System process scanned");
    assert(rc == 0);
    rc = log_event("SUSPICIOUS", 200, "gdb", "Debugger tool detected");
    assert(rc == 0);
    rc = log_event("ALERT", 300, "cheat_test", "Controlled cheat detected");
    assert(rc == 0);
    rc = log_event("PROTECTION", 50, "game", "Terminated game to protect session");
    assert(rc == 0);

    logger_close();

    /* Read back and verify content */
    FILE *f = fopen(temp_log, "r");
    assert(f != NULL);

    char content[2048];
    size_t bytes = fread(content, 1, sizeof(content) - 1, f);
    content[bytes] = '\0';
    fclose(f);

    assert(strstr(content, "[INFO] PID: 1 (systemd) - System process scanned") != NULL);
    assert(strstr(content, "[SUSPICIOUS] PID: 200 (gdb) - Debugger tool detected") != NULL);
    assert(strstr(content, "[ALERT] PID: 300 (cheat_test) - Controlled cheat detected") != NULL);
    assert(strstr(content, "[PROTECTION] PID: 50 (game) - Terminated game to protect session") != NULL);

    unlink(temp_log);
    printf("  -> PASS\n");
}

static void test_logger_invalid_path(void) {
    printf("[TEST] Running test_logger_invalid_path...\n");
    int rc = logger_init("/path/that/cannot/possibly/exist/99999/log.txt");
    assert(rc == -1);
    assert(!logger_is_open());
    printf("  -> PASS\n");
}

int main(void) {
    printf("========================================\n");
    printf("        RUNNING LOGGER TESTS            \n");
    printf("========================================\n");

    test_logger_init_and_close();
    test_log_event_writing();
    test_logger_invalid_path();

    printf("\nAll logger tests passed!\n");
    return 0;
}
