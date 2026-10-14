#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>

static volatile sig_atomic_t g_running = 1;

static void handle_signal(int sig) {
    (void)sig;
    g_running = 0;
}

static void print_usage(const char *prog_name) {
    printf("Usage: %s [OPTIONS]\n\n", prog_name);
    printf("Options:\n");
    printf("  --target <pid>     Specify target game process PID (simulation only)\n");
    printf("  --duration <sec>   Run for specified seconds and exit (default: infinite)\n");
    printf("  --mode <type>      Simulation mode: score_hack | speed_hack | memory_scan (default: score_hack)\n");
    printf("  -h, --help         Show this help message\n");
}

int main(int argc, char *argv[]) {
    int target_pid = -1;
    int duration_sec = -1;
    const char *mode = "score_hack";

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--target") == 0) {
            if (i + 1 < argc) {
                target_pid = atoi(argv[++i]);
            } else {
                fprintf(stderr, "Error: --target requires a PID integer\n");
                return 1;
            }
        } else if (strcmp(argv[i], "--duration") == 0) {
            if (i + 1 < argc) {
                duration_sec = atoi(argv[++i]);
            } else {
                fprintf(stderr, "Error: --duration requires a seconds integer\n");
                return 1;
            }
        } else if (strcmp(argv[i], "--mode") == 0) {
            if (i + 1 < argc) {
                mode = argv[++i];
            } else {
                fprintf(stderr, "Error: --mode requires a mode name\n");
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

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handle_signal;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    pid_t my_pid = getpid();
    printf("====================================\n");
    printf("     CONTROLLED CHEAT SIMULATOR     \n");
    printf("====================================\n");
    printf("[CHEAT_TEST] Started simulator with PID: %d\n", (int)my_pid);
    if (target_pid > 0) {
        printf("[CHEAT_TEST] Target game PID set to: %d\n", target_pid);
    } else {
        printf("[CHEAT_TEST] Running in standalone educational test mode\n");
    }
    printf("[CHEAT_TEST] Simulation Mode: %s\n", mode);
    printf("[CHEAT_TEST] NOTE: Safe educational simulation - no real processes are modified.\n\n");
    fflush(stdout);

    int elapsed = 0;
    while (g_running) {
        elapsed++;
        printf("[CHEAT_TEST] [%02ds] Simulating unauthorized event: '%s' activity...\n",
               elapsed, mode);
        fflush(stdout);

        if (duration_sec > 0 && elapsed >= duration_sec) {
            printf("[CHEAT_TEST] Test duration (%d seconds) completed. Exiting.\n", duration_sec);
            break;
        }

        sleep(1);
    }

    if (!g_running) {
        printf("\n[CHEAT_TEST] Received termination signal. Exiting safely...\n");
    }

    return 0;
}
