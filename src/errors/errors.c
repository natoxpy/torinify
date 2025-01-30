#include <errors/errors.h>
#include <stdio.h>

/// Saves log to the global buffer, can get the current log with
/// `error_get_log()`
///
/// After move than MAX_LOG_ENTRIES (16) logs are made, the log_index returns to
/// 0 and all new entries override previous entries
void error_log(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vsnprintf(log_buffer[log_index], MAXLOG_LENGTH, fmt, args);
    va_end(args);

    log_index = (log_index + 1) % MAX_LOG_ENTRIES;
}

/// Returns Latest Error Log
char *error_get_log() {
    int index = (log_index - 1 + MAX_LOG_ENTRIES) % MAX_LOG_ENTRIES;
    return log_buffer[index];
}

/// Prints `log_index`
void error_print_log() { fprintf(stderr, "%s\n", error_get_log()); };

/// Prints all logs to from start index to `log_index`
void error_print_all() {
    for (int i = 0; i < log_index; i++) {
        fprintf(stderr, "%d: %s\n", i + 1, log_buffer[i]);
    }
};
