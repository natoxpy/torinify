#ifndef _DB_HELPERS_H
#define _DB_HELPERS_H

#include "db/tables.h"
#include <db/exec.h>
#include <sqlite3.h>
#include <stdlib.h>

void dbh_realpath(char *path, char *resolved);

void dbh_join_sql(char *buffer, size_t buffer_size, const char *sql_root,
                  const char *sql_arg);

TDB_CODE dbh_start_transaction(sqlite3 *db);
TDB_CODE dbh_rollback_transaction(sqlite3 *db);
TDB_CODE dbh_commit_transaction(sqlite3 *db);
TDB_CODE dbh_prepare(sqlite3 *db, const char *sql, sqlite3_stmt **stmt);
TDB_CODE dbh_bind_text(sqlite3 *db, sqlite3_stmt *stmt, int n, char *text);
TDB_CODE dbh_bind_int(sqlite3 *db, sqlite3_stmt *stmt, int n, int value);
char *dbh_get_column_text(sqlite3_stmt *stmt, int icol);
int dbh_get_column_int(sqlite3_stmt *stmt, int icol);

#define DBH_BIND_VALUE_TYPE_NUMBER 0
#define DBH_BIND_VALUE_TYPE_STRING 1
#define DBH_BIND_VALUE_TYPE_NULL 2

typedef struct {
    int type;
    union {
        char *str;
        int num;
    } value;
} BindValue;

#define BIND_INT(x)                                                            \
    (BindValue) {                                                              \
        .type = DBH_BIND_VALUE_TYPE_NUMBER, .value = {.num = x }               \
    }

#define BIND_STR(x)                                                            \
    (BindValue) {                                                              \
        .type = DBH_BIND_VALUE_TYPE_STRING, .value = {.str = x }               \
    }

#define BIND_NULL(x)                                                           \
    (BindValue) { .type = DBH_BIND_VALUE_TYPE_NULL }

#define SIZEOF_BINDS(x) sizeof(x) / sizeof(BindValue)

TDB_CODE dbh_bind_array(sqlite3 *db, sqlite3_stmt *stmt, BindValue *arr,
                        int bind_length);

TDB_CODE dbh_sql_execute(sqlite3 *db, sqlite3_stmt *stmt, Vec *out_vec,
                         void *(*collect)(sqlite3_stmt *));

TDB_CODE dbh_sql_execute_single(sqlite3 *db, sqlite3_stmt *stmt, void **out,
                                void *(*collect)(sqlite3_stmt *));

#endif
