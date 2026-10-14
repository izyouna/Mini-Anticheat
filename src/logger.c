#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include "logger.h"

static int g_log_fd = -1;

int logger_init(const char *log_filepath) {
    if (!log_filepath) return -1;

    if (g_log_fd >= 0) {
        close(g_log_fd);
        g_log_fd = -1;
    }

    int fd = open(log_filepath, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd < 0) {
        return -1;
    }

    g_log_fd = fd;
    return 0;
}

void logger_close(void) {
    if (g_log_fd >= 0) {
        close(g_log_fd);
        g_log_fd = -1;
    }
}

bool logger_is_open(void) {
    return (g_log_fd >= 0);
}

int log_event(const char *event_type, pid_t pid, const char *process_name, const char *reason) {
    if (g_log_fd < 0) return -1;

    time_t now = time(NULL);
    struct tm tm_info;
    localtime_r(&now, &tm_info);

    char time_str[32];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", &tm_info);

    char log_buf[1024];
    int len = snprintf(log_buf, sizeof(log_buf),
                       "[%s] [%s] PID: %d (%s) - %s\n",
                       time_str,
                       event_type ? event_type : "EVENT",
                       (int)pid,
                       process_name ? process_name : "unknown",
                       reason ? reason : "No reason specified");

    if (len <= 0) return -1;

    ssize_t written = write(g_log_fd, log_buf, (size_t)len);
    return (written == len) ? 0 : -1;
}
