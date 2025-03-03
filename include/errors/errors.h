#ifndef _ERRORS_ERRORS_H
#define _ERRORS_ERRORS_H
#define SUCCESS T_SUCCESS

#include <stdarg.h>

// Torinify Errors
//
// `T_FAIL` can be used for any failure which does not require any further
// specification, most commontly use purely for mistakes that cannot be
// recovered from, or should not normally happen. For all intents and purposes,
// it is a critical error.
typedef enum {
    T_SUCCESS = 0,
    T_FAIL,
    T_DB_SQL_CANNOT_EXECUTE,
    T_DB_SQL_CANNOT_PREPARE,
    T_DB_NOT_FOUND,
} T_CODE;

#define MAXLOG_LENGTH 256
#define MAX_LOG_ENTRIES 16

static char log_buffer[MAX_LOG_ENTRIES][MAXLOG_LENGTH];
static int log_index = 0;

void error_log(const char *fmt, ...);
char *error_get_log();

void error_print_log();
void error_print_all();

#endif

#ifndef _ERRORS_DB_ERRORS_H
#define _ERRORS_DB_ERRORS_H

typedef enum { TDB_NOT_FOUND = -2, TDB_FAIL = -1, TDB_SUCCESS = 0 } TDB_CODE;

#endif
