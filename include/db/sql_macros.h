#ifndef _DB_SQL_MACROS_H
#define _DB_SQL_MACROS_H
#include "db/helpers.h"
#include <stdlib.h>

#define SQL_START                                                              \
    int ret = TDB_FAIL;                                                        \
    sqlite3_stmt *stmt = NULL;

#define SQL_PREPARE(SQL)                                                       \
    if ((ret = dbh_prepare(db, SQL, &stmt)) != TDB_SUCCESS)                    \
        goto clean;

#define SQL_BINDS(...)                                                         \
    BindValue binds[] = {__VA_ARGS__};                                         \
    if ((ret = dbh_bind_array(db, stmt, binds, SIZEOF_BINDS(binds))) !=        \
        TDB_SUCCESS)                                                           \
        goto clean;

#define SQL_EXECUTE                                                            \
    if ((ret = dbh_sql_execute(db, stmt, NULL, NULL)) != TDB_SUCCESS)          \
        goto clean;

#define SQL_EXECUTE_COLLECT_SINGLE(TO, COLLECT)                                \
    ret = dbh_sql_execute_single(db, stmt, (void **)TO, COLLECT);              \
    if (ret == TDB_NOT_FOUND)                                                  \
        *TO = NULL;                                                            \
    if (ret != TDB_SUCCESS)                                                    \
        goto clean;

#define SQL_EXECUTE_COLLECT_ALL(OUT, COLLECT, COLLECT_TYPE)                    \
    Vec *out = vec_init(sizeof(COLLECT_TYPE *));                               \
    ret = dbh_sql_execute(db, stmt, out, &COLLECT);                            \
    if (ret != TDB_SUCCESS) {                                                  \
        vec_free(out);                                                         \
        goto clean;                                                            \
    }                                                                          \
    *OUT = out;

#define SQL_LAST_INSERTED(TO)                                                  \
    if (TO != NULL)                                                            \
        *TO = sqlite3_last_insert_rowid(db);

#define SQL_END                                                                \
    if (stmt)                                                                  \
        sqlite3_finalize(stmt);                                                \
    return ret;

// ================
// High Abstraction
// ================

#define SQL_GENERIC_ADD_W_LAST_INSERT(SQL, BINDS, LAST_ID_TARGET)              \
    SQL_START                                                                  \
    SQL_PREPARE(SQL)                                                           \
    BINDS                                                                      \
    SQL_EXECUTE                                                                \
    SQL_LAST_INSERTED(LAST_ID_TARGET)                                          \
    clean:                                                                     \
    SQL_END

#define SQL_GENERIC_ADD(SQL, BINDS)                                            \
    SQL_START                                                                  \
    SQL_PREPARE(SQL)                                                           \
    BINDS                                                                      \
    SQL_EXECUTE                                                                \
    clean:                                                                     \
    SQL_END

#define SQL_GENERIC_GET(SQL, BINDS, RETURN_TO, COLLECT)                        \
    SQL_START                                                                  \
    SQL_PREPARE(SQL)                                                           \
    BINDS                                                                      \
    SQL_EXECUTE_COLLECT_SINGLE(RETURN_TO, COLLECT)                             \
    clean:                                                                     \
    SQL_END

#define SQL_GENERIC_GET_ALL_WBINDS(SQL, BINDS, RETURN_TO, COLLECT,             \
                                   COLLECT_TYPE)                               \
    SQL_START                                                                  \
    SQL_PREPARE(SQL)                                                           \
    BINDS                                                                      \
    SQL_EXECUTE_COLLECT_ALL(RETURN_TO, COLLECT, COLLECT_TYPE)                  \
    clean:                                                                     \
    SQL_END

#define SQL_GENERIC_GET_ALL(SQL, RETURN_TO, COLLECT, COLLECT_TYPE)             \
    SQL_START                                                                  \
    SQL_PREPARE(SQL)                                                           \
    SQL_EXECUTE_COLLECT_ALL(RETURN_TO, COLLECT, COLLECT_TYPE)                  \
    clean:                                                                     \
    SQL_END

#define SQL_GENERIC_DELETE(SQL, BINDS)                                         \
    SQL_START                                                                  \
    SQL_PREPARE(SQL)                                                           \
    BINDS                                                                      \
    SQL_EXECUTE                                                                \
    clean:                                                                     \
    SQL_END

#define SQL_GENERIC_UPDATE(SQL, BINDS)                                         \
    SQL_START                                                                  \
    SQL_PREPARE(SQL)                                                           \
    BINDS                                                                      \
    SQL_EXECUTE                                                                \
    clean:                                                                     \
    SQL_END

#endif
