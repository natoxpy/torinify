#ifndef _ERRORS_ERRORS_H
#define _ERRORS_ERRORS_H
#define SUCCESS T_SUCCESS

#include <stdarg.h>

// Torinify Errors
typedef enum { T_FAIL = -1, T_SUCCESS = 0 } T_CODE;

#define MAXLOG_LENGTH 256
#define MAX_LOG_ENTRIES 16

static char log_buffer[MAX_LOG_ENTRIES][MAXLOG_LENGTH];
static int log_index = 0;

void error_log(const char *fmt, ...);
char *error_get_log();

void error_print_log();
void error_print_all();

#endif
