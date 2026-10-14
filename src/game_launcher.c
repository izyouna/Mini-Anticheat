#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <sys/wait.h>
#include "game_launcher.h"

void game_process_init(GameProcess *game) {
    if (!game) return;
    game->pid = -1;
    game->pipe_read_fd = -1;
    game->is_running = false;
    game->exit_code = 0;
    game->terminated_by_signal = false;
    game->term_signal = 0;
}

void game_process_cleanup(GameProcess *game) {
    if (!game) return;
    if (game->pipe_read_fd >= 0) {
        close(game->pipe_read_fd);
        game->pipe_read_fd = -1;
    }
    game->pid = -1;
    game->is_running = false;
}

int launch_game(GameProcess *game, const char *game_bin, char *const argv[]) {
    if (!game || !game_bin) return -1;

    game_process_init(game);

    int pipe_fds[2];
    if (pipe(pipe_fds) != 0) {
        perror("pipe");
        return -1;
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        close(pipe_fds[0]);
        close(pipe_fds[1]);
        return -1;
    }

    if (pid == 0) {
        /* Child Process */
        close(pipe_fds[0]); /* Close read end in child */

        /* Count existing argv arguments */
        int arg_count = 0;
        if (argv) {
            while (argv[arg_count] != NULL) {
                arg_count++;
            }
        }

        /* Build new argv with --ipc-fd <pipe_write_fd> */
        char fd_str[16];
        snprintf(fd_str, sizeof(fd_str), "%d", pipe_fds[1]);

        char **new_argv = malloc((size_t)(arg_count + 3) * sizeof(char *));
        if (!new_argv) {
            perror("malloc new_argv");
            exit(1);
        }

        for (int i = 0; i < arg_count; i++) {
            new_argv[i] = argv[i];
        }
        new_argv[arg_count] = "--ipc-fd";
        new_argv[arg_count + 1] = fd_str;
        new_argv[arg_count + 2] = NULL;

        execvp(game_bin, new_argv);
        perror("execvp");
        free(new_argv);
        exit(127);
    }

    /* Parent Process */
    close(pipe_fds[1]); /* Close write end in parent */

    /* Set non-blocking on pipe_read_fd */
    int flags = fcntl(pipe_fds[0], F_GETFL, 0);
    if (flags >= 0) {
        fcntl(pipe_fds[0], F_SETFL, flags | O_NONBLOCK);
    }

    game->pid = pid;
    game->pipe_read_fd = pipe_fds[0];
    game->is_running = true;

    return 0;
}

bool check_game_status(GameProcess *game) {
    if (!game || !game->is_running || game->pid <= 0) {
        return false;
    }

    int status = 0;
    pid_t res = waitpid(game->pid, &status, WNOHANG);
    if (res == 0) {
        /* Child is still running */
        return true;
    } else if (res == game->pid) {
        /* Child has terminated */
        game->is_running = false;
        if (WIFEXITED(status)) {
            game->exit_code = WEXITSTATUS(status);
        } else if (WIFSIGNALED(status)) {
            game->terminated_by_signal = true;
            game->term_signal = WTERMSIG(status);
        }
        return false;
    } else {
        /* Process no longer exists */
        game->is_running = false;
        return false;
    }
}

int read_game_ipc_events(GameProcess *game, char *buffer, size_t max_len) {
    if (!game || game->pipe_read_fd < 0 || !buffer || max_len == 0) {
        return -1;
    }

    ssize_t n = read(game->pipe_read_fd, buffer, max_len);
    if (n > 0) {
        return (int)n;
    } else if (n == 0) {
        return 0; /* EOF */
    } else {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return 0; /* No data currently available */
        }
        return -1;
    }
}

int terminate_game(GameProcess *game, int sig) {
    if (!game || !game->is_running || game->pid <= 0) {
        return -1;
    }

    /* Security check: only signal the child process created and tracked by this launcher */
    if (kill(game->pid, sig) != 0) {
        perror("kill child process");
        return -1;
    }

    /* Wait synchronously to prevent zombie process */
    int status = 0;
    waitpid(game->pid, &status, 0);

    game->is_running = false;
    if (WIFEXITED(status)) {
        game->exit_code = WEXITSTATUS(status);
    } else if (WIFSIGNALED(status)) {
        game->terminated_by_signal = true;
        game->term_signal = WTERMSIG(status);
    }

    return 0;
}

int monitor_game_session(GameProcess *game, int scan_interval_ms, int max_ticks, const char *proc_dir) {
    if (!game) return -1;

    printf("[MONITOR] Starting real-time monitoring session for game (PID: %d)...\n", (int)game->pid);
    char ipc_buffer[512];
    int ticks = 0;

    while (check_game_status(game)) {
        ticks++;

        /* 1. Drain IPC messages from the game */
        int bytes = read_game_ipc_events(game, ipc_buffer, sizeof(ipc_buffer) - 1);
        if (bytes > 0) {
            ipc_buffer[bytes] = '\0';
            /* Strip trailing newline for clean log output */
            size_t l = strlen(ipc_buffer);
            while (l > 0 && (ipc_buffer[l - 1] == '\n' || ipc_buffer[l - 1] == '\r')) {
                ipc_buffer[--l] = '\0';
            }
            printf("[MONITOR] Game IPC Event: %s\n", ipc_buffer);
        }

        /* 2. Scan active processes */
        ProcessList procs;
        process_list_init(&procs);
        if (scan_processes(&procs, proc_dir) == 0) {
            DetectionResultList detections;
            detection_result_list_init(&detections);
            if (detect_cheats(&procs, &detections) == 0) {
                for (size_t i = 0; i < detections.count; i++) {
                    if (detections.items[i].status == DETECTION_CONFIRMED_CHEAT) {
                        printf("\033[1;31m[ALERT] CONFIRMED CHEAT DETECTED!\033[0m Process '%s' (PID: %d) - %s\n",
                               detections.items[i].name, (int)detections.items[i].pid, detections.items[i].reason);
                    } else if (detections.items[i].status == DETECTION_SUSPICIOUS) {
                        printf("\033[1;33m[ALERT] SUSPICIOUS PROCESS:\033[0m '%s' (PID: %d) - %s\n",
                               detections.items[i].name, (int)detections.items[i].pid, detections.items[i].reason);
                    }
                }
            }
            detection_result_list_free(&detections);
        }
        process_list_free(&procs);

        if (max_ticks > 0 && ticks >= max_ticks) {
            printf("[MONITOR] Max monitoring ticks (%d) reached.\n", max_ticks);
            break;
        }

        usleep((useconds_t)scan_interval_ms * 1000);
    }

    printf("[MONITOR] Game process finished (Exit code: %d).\n", game->exit_code);
    return 0;
}
