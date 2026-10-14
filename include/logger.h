#ifndef LOGGER_H
#define LOGGER_H

#include <sys/types.h>
#include <stdbool.h>

#define DEFAULT_LOG_FILE "logs/anticheat.log"

/*
 * Initialize the logging system.
 * Opens or creates the specified log file in append mode using POSIX open().
 * Returns 0 on success, or -1 on failure.
 */
int logger_init(const char *log_filepath);

/*
 * Close the logger and release file descriptors using POSIX close().
 */
void logger_close(void);

/*
 * Record a detection or protection event to the log file using POSIX write().
 * Format: [YYYY-MM-DD HH:MM:SS] [EVENT_TYPE] PID: <pid> (<process_name>) - <reason>
 * Returns 0 on success, or -1 on failure.
 */
int log_event(const char *event_type, pid_t pid, const char *process_name, const char *reason);

/*
 * Check if the logger is currently initialized and ready for writing.
 */
bool logger_is_open(void);

#endif /* LOGGER_H */
