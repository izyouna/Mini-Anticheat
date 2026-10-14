#ifndef GAME_LAUNCHER_H
#define GAME_LAUNCHER_H

#include <sys/types.h>
#include <stdbool.h>
#include <stddef.h>
#include "cheat_detector.h"

typedef struct {
    pid_t pid;
    int pipe_read_fd;
    bool is_running;
    int exit_code;
    bool terminated_by_signal;
    int term_signal;
} GameProcess;

/*
 * Initialize GameProcess structure.
 */
void game_process_init(GameProcess *game);

/*
 * Launch the game executable as a child process using fork() and execvp(),
 * establishing an IPC pipe to receive events from the game.
 * Returns 0 on success, or -1 on failure.
 */
int launch_game(GameProcess *game, const char *game_bin, char *const argv[]);

/*
 * Check child process status using waitpid() with WNOHANG.
 * Returns true if the game is still running, false if it has terminated.
 */
bool check_game_status(GameProcess *game);

/*
 * Read available IPC messages from the game pipe (non-blocking).
 * Returns bytes read, 0 if nothing available, or -1 on error/EOF.
 */
int read_game_ipc_events(GameProcess *game, char *buffer, size_t max_len);

/*
 * Safely terminate the launched game process using kill().
 * Strictly sends signal ONLY to game->pid (owned by launcher).
 * Waits for termination with waitpid() to prevent zombie processes.
 * Returns 0 on success, or -1 on error.
 */
int terminate_game(GameProcess *game, int sig);

/*
 * Clean up file descriptors and state.
 */
void game_process_cleanup(GameProcess *game);

/*
 * Run real-time monitoring loop:
 * Monitors game status, reads IPC events, scans processes periodically,
 * and detects cheats.
 * If max_ticks > 0, stops after max_ticks iterations.
 * If proc_dir is NULL, uses DEFAULT_PROC_DIR.
 * Returns 0 on normal game exit, or -1 on error.
 */
int monitor_game_session(GameProcess *game, int scan_interval_ms, int max_ticks, const char *proc_dir);

#endif /* GAME_LAUNCHER_H */
