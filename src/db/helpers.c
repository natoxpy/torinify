#include "db/tables.h"
#include <db/exec.h>
#include <db/helpers.h>
#include <libavformat/avformat.h>
#include <sqlite3.h>
#include <stdatomic.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <utils/generic_vec.h>

#ifdef _WIN32
#include <windows.h>
void dbh_realpath(char *path, char *resolved) {
    _fullpath(resolved, path, 2048);
};
#elif __unix__
void dbh_realpath(char *path, char *resolved) { realpath(path, resolved); }
#endif

void dbh_join_sql(char *buffer, size_t buffer_size, const char *sql_root,
                  const char *sql_arg) {
    snprintf(buffer, buffer_size, sql_root, sql_arg);
};

TDB_CODE dbh_start_transaction(sqlite3 *db) {
    int ret = sqlite3_exec(db, "BEGIN TRANSACTION;", 0, 0, 0);

    if (ret != SQLITE_OK) {
        error_log("Could not begin transaction, possible cause \"%s\"",
                  sqlite3_errmsg(db));
        return TDB_FAIL;
    }

    return TDB_SUCCESS;
}

TDB_CODE dbh_rollback_transaction(sqlite3 *db) {
    int ret = sqlite3_exec(db, "ROLLBACK;", 0, 0, 0);

    if (ret != SQLITE_OK) {
        error_log("Could not rollback, possible cause \"%s\"",
                  sqlite3_errmsg(db));
        return TDB_FAIL;
    }

    return TDB_SUCCESS;
}

TDB_CODE dbh_commit_transaction(sqlite3 *db) {
    int ret = sqlite3_exec(db, "COMMIT;", 0, 0, 0);

    if (ret != SQLITE_OK) {
        error_log("Could not commit changes, possible cause \"%s\"",
                  sqlite3_errmsg(db));
        return TDB_FAIL;
    }

    return TDB_SUCCESS;
}

TDB_CODE dbh_prepare(sqlite3 *db, const char *sql, sqlite3_stmt **stmt) {
    int ret = sqlite3_prepare_v2(db, sql, -1, stmt, NULL);

    if (ret != SQLITE_OK) {
        error_log("Could not prepare sql string \"%s\", possible cause \"%s\"",
                  sql, sqlite3_errmsg(db));
        return TDB_FAIL;
    }

    return TDB_SUCCESS;
}

TDB_CODE dbh_bind_text(sqlite3 *db, sqlite3_stmt *stmt, int n, char *text) {
    if (text != NULL) {
        if (sqlite3_bind_text(stmt, n, text, -1, SQLITE_STATIC) != SQLITE_OK) {
            error_log("Could not bind text \"%s\" to param (%d), possible "
                      "cause \"%s\"",
                      n, sqlite3_errmsg(db));

            return TDB_FAIL;
        }
    } else {
        if (sqlite3_bind_null(stmt, n) != SQLITE_OK) {
            error_log("Could not bind NULL in place of TEXT to param (%d), "
                      "possible cause \"%s\"",
                      n, sqlite3_errmsg(db));
            return TDB_FAIL;
        }
    }

    return TDB_SUCCESS;
}

TDB_CODE dbh_bind_int(sqlite3 *db, sqlite3_stmt *stmt, int n, int value) {
    if (value != -1) {
        if (sqlite3_bind_int(stmt, n, value) != SQLITE_OK) {
            error_log("Could not bind text \"%s\" to param (%d), possible "
                      "cause \"%s\"",
                      n, sqlite3_errmsg(db));
            return TDB_FAIL;
        }
    } else {
        if (sqlite3_bind_null(stmt, n) != SQLITE_OK) {
            error_log("Could not bind NULL in place of int to param (%d), "
                      "possible cause \"%s\"",
                      n, sqlite3_errmsg(db));
            return TDB_FAIL;
        }
    }
    return TDB_SUCCESS;
}

TDB_CODE dbh_bind_array(sqlite3 *db, sqlite3_stmt *stmt, BindValue *arr,
                        int bind_length) {
    int ret;

    for (int i = 0; i < bind_length; i++) {
        BindValue value = arr[i];
        switch (value.type) {
        case DBH_BIND_VALUE_TYPE_NUMBER:
            ret = dbh_bind_int(db, stmt, i + 1, value.value.num);
            break;
        case DBH_BIND_VALUE_TYPE_STRING:
            ret = dbh_bind_text(db, stmt, i + 1, value.value.str);
            break;
        case DBH_BIND_VALUE_TYPE_NULL:
            ret = dbh_bind_int(db, stmt, i + 1, -1);
            break;
        default:
            error_log("Does not support bind type");
        }

        if (ret != TDB_SUCCESS)
            goto clean;
    }
clean:
    return ret;
}

/// Return must be `free()`

char *dbh_get_column_text(sqlite3_stmt *stmt, int icol) {
    if (sqlite3_column_type(stmt, icol) == SQLITE_NULL)
        return NULL;
    return strdup((const char *)sqlite3_column_text(stmt, icol));
}

int dbh_get_column_int(sqlite3_stmt *stmt, int icol) {
    if (sqlite3_column_type(stmt, icol) == SQLITE_NULL)
        return -1;

    return sqlite3_column_int(stmt, icol);
}

TDB_CODE dbh_sql_execute(sqlite3 *db, sqlite3_stmt *stmt, Vec *out_vec,
                         void *(*collect)(sqlite3_stmt *)) {
    int rc;
    void *data;
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        if ((out_vec != NULL || collect != NULL) &&
            (data = collect(stmt)) != NULL) {
            vec_push(out_vec, &data);
        }
    }

    if (rc != SQLITE_DONE) {
        error_log("sqlite3 step did not end when expected to end, possible "
                  "error \"%s\"",
                  sqlite3_errmsg(db));

        return TDB_FAIL;
    }

    return TDB_SUCCESS;
}

TDB_CODE dbh_sql_execute_single(sqlite3 *db, sqlite3_stmt *stmt, void **out,
                                void *(*collect)(sqlite3_stmt *)) {
    int rc;
    void *data;

    if (out != NULL && (rc = sqlite3_step(stmt)) != SQLITE_ROW) {
        if (rc == SQLITE_DONE)
            return TDB_NOT_FOUND;

        error_log("sqlite3 step did not found row as expected, possible "
                  "error \"%s\"",
                  sqlite3_errmsg(db));

        return TDB_FAIL;
    } else if (out == NULL && (rc = sqlite3_step(stmt)) != SQLITE_DONE) {
        error_log("sqlite3 step did not end when expected to end, possible "
                  "error \"%s\"",
                  sqlite3_errmsg(db));

        return TDB_FAIL;
    }

    if (collect != NULL && (data = collect(stmt)) != NULL) {
        *out = data;
        rc = sqlite3_step(stmt);
    }

    if (rc != SQLITE_DONE) {
        error_log("sqlite3 step did not end when expected to end, possible "
                  "error \"%s\"",
                  sqlite3_errmsg(db));

        return TDB_FAIL;
    }

    return TDB_SUCCESS;
}
