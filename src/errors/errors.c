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
    vsnprintf(LOG_BUFFER[LOG_INDEX], MAXLOG_LENGTH, fmt, args);
    va_end(args);

    LOG_INDEX = (LOG_INDEX + 1) % MAX_LOG_ENTRIES;
}

/// Returns Latest Error Log
char *error_get_log() {
    int index = (LOG_INDEX - 1 + MAX_LOG_ENTRIES) % MAX_LOG_ENTRIES;
    return LOG_BUFFER[index];
}

/// Prints `log_index`
void error_print_log() { fprintf(stderr, "%s\n", error_get_log()); };

/// Prints all logs to from start index to `log_index`
void error_print_all() {
    for (int i = 0; i < LOG_INDEX; i++) {
        fprintf(stderr, "%d: %s\n", i + 1, LOG_BUFFER[i]);
    }

    LOG_INDEX = 0;
};

/// Prints all logs to from start index to `log_index`
void error_print_all_cb(void(cb)(char *)) {
    for (int i = 0; i < LOG_INDEX; i++) {
        cb(LOG_BUFFER[i]);
    }

    LOG_INDEX = 0;
};

int get_log_index() { return LOG_INDEX; }
