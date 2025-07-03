/**
 * @file logger.c
 * @brief Logger class/struct implementation
 */
#include "utils/logger.h"

#include <stdlib.h>
#include <string.h>

#include "global/CONSTANTS.h"

void logger_init(struct logger *log, const char *log_path, const char *username) {
    log->username = username;

    if (!log_path) {
        log->log_file = stdout;
        return;
    }
    
    log->log_file = fopen(log_path, "a");
    
    if (!log->log_file) {
        fprintf(stderr, "Failed to open log file: %s\nUsing stdout.\n", log_path);
        log->log_file = stdout;
        return;
    }
}

void logger_log(struct logger *log, const char *format, ...) {
    if (!log) {
        fprintf(stderr, "Logger not initialized!\n");
        return;
    }

    time_t now;
    time(&now);
    char timestamp[DATETIME_LEN];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", localtime(&now));

    // Print timestamp and user
    if (!log->username) {
        log->username = "{Username not set}";
    }

    fprintf(log->log_file, "[%s] User: %s | ", timestamp, log->username);

    // Variadic arguments printf style
    va_list args;
    va_start(args, format);
    int ret = vfprintf(log->log_file, format, args);
    va_end(args);

    if (ret < 0) {
        fprintf(stderr, "Failed to write to log file!");
        return;
    }

    fprintf(log->log_file, "\n"); // Add newline
    fflush(log->log_file); // Force write
}

void logger_deinit(struct logger *log) {
    if (log && log->log_file != stdout) {
        fclose(log->log_file);
        log->log_file = NULL;
    }
}
