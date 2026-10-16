#ifndef _DEFAULT_SOURCE
#define _DEFAULT_SOURCE
#endif
#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>

typedef struct {
    int health;
    int score;
    int tick;
    pid_t pid;
    volatile sig_atomic_t running;
    int ipc_fd;
} GameState;

static GameState g_game;

static void handle_signal(int sig) {
    (void)sig;
    g_game.running = 0;
}

static void print_usage(const char *prog_name) {
    printf("Usage: %s [OPTIONS]\n\n", prog_name);
    printf("Options:\n");
    printf("  --ticks <N>        Run for N ticks and exit (default: infinite)\n");
    printf("  --interval <ms>    Tick interval in milliseconds (default: 500)\n");
    printf("  --ipc-fd <fd>      Write state events to specified file descriptor\n");
    printf("  -h, --help         Show this help message\n");
}

int main(int argc, char *argv[]) {
    int max_ticks = -1;
    int interval_ms = 500;
    int ipc_fd = -1;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--ticks") == 0) {
            if (i + 1 < argc) {
                max_ticks = atoi(argv[++i]);
            } else {
                fprintf(stderr, "Error: --ticks requires an integer\n");
                return 1;
            }
        } else if (strcmp(argv[i], "--interval") == 0) {
            if (i + 1 < argc) {
                interval_ms = atoi(argv[++i]);
            } else {
                fprintf(stderr, "Error: --interval requires an integer\n");
                return 1;
            }
        } else if (strcmp(argv[i], "--ipc-fd") == 0) {
            if (i + 1 < argc) {
                ipc_fd = atoi(argv[++i]);
            } else {
                fprintf(stderr, "Error: --ipc-fd requires a file descriptor integer\n");
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

    g_game.health = 100;
    g_game.score = 0;
    g_game.tick = 0;
    g_game.pid = getpid();
    g_game.running = 1;
    g_game.ipc_fd = ipc_fd;

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handle_signal;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    srand((unsigned int)time(NULL) ^ (unsigned int)getpid());

    printf("====================================\n");
    printf("        MINI GAME SIMULATOR         \n");
    printf("====================================\n");
    printf("[GAME] Process started with PID: %d\n", (int)g_game.pid);
    printf("[GAME] Initial State - Health: %d, Score: %d\n", g_game.health, g_game.score);
    fflush(stdout);

    while (g_game.running) {
        g_game.tick++;

        /* Simulate gameplay events */
        int roll = rand() % 100;
        if (roll < 35) {
            int gain = 10 + (rand() % 20);
            g_game.score += gain;
            printf("[GAME] Tick %3d | Scored +%d points! | Health: %3d | Score: %5d\n",
                   g_game.tick, gain, g_game.health, g_game.score);
        } else if (roll < 55) {
            int dmg = 5 + (rand() % 15);
            g_game.health -= dmg;
            if (g_game.health < 0) g_game.health = 0;
            printf("[GAME] Tick %3d | Took %d damage!    | Health: %3d | Score: %5d\n",
                   g_game.tick, dmg, g_game.health, g_game.score);
        } else {
            printf("[GAME] Tick %3d | Exploring area...    | Health: %3d | Score: %5d\n",
                   g_game.tick, g_game.health, g_game.score);
        }
        fflush(stdout);

        /* Send IPC update if pipe fd is specified */
        if (g_game.ipc_fd >= 0) {
            char ipc_buf[128];
            int len = snprintf(ipc_buf, sizeof(ipc_buf),
                               "EVENT:TICK:%d:HEALTH:%d:SCORE:%d\n",
                               g_game.tick, g_game.health, g_game.score);
            if (len > 0) {
                ssize_t written = write(g_game.ipc_fd, ipc_buf, (size_t)len);
                (void)written;
            }
        }

        if (g_game.health <= 0) {
            printf("[GAME] Health depleted! Game Over.\n");
            break;
        }

        if (max_ticks > 0 && g_game.tick >= max_ticks) {
            printf("[GAME] Target tick limit (%d) reached. Exiting normally.\n", max_ticks);
            break;
        }

        usleep((useconds_t)interval_ms * 1000);
    }

    if (!g_game.running) {
        printf("\n[GAME] Received termination signal. Exiting cleanly...\n");
    }

    printf("[GAME] Final Results - Ticks: %d, Score: %d, Health: %d\n",
           g_game.tick, g_game.score, g_game.health);
    return 0;
}
