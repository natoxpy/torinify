#include "db/migrations.h"
#include "db/exec.h"
#include "db/helpers.h"
#include <sqlite3.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

TDB_CODE check_migration_database_exists(sqlite3 *db, bool *found) {
    int ret = TDB_SUCCESS;
    sqlite3_stmt *stmt = NULL;
    if ((ret = dbh_prepare(db,
                           "SELECT name FROM sqlite_master WHERE type='table' "
                           "and name='.migrations'",
                           &stmt)) != TDB_SUCCESS)
        goto clean;

    int sql_ret = sqlite3_step(stmt);

    // if the step instantly ends no table with name '.migrations' was found
    if (sql_ret == SQLITE_DONE)
        *found = false;
    else if (sql_ret == SQLITE_ROW)
        *found = true;
    else {
        error_log("sqlite3 step returned an unexpected value");
        ret = TDB_FAIL;
    }

clean:
    if (stmt)
        sqlite3_finalize(stmt);

    return ret;
}

TDB_CODE insert_first_migration_value(sqlite3 *db) {
    int ret = TDB_SUCCESS;
    sqlite3_stmt *stmt = NULL;

    if ((ret = dbh_prepare(
             db, "INSERT INTO '.migrations'(id, migration_index) VALUES (1,0)",
             &stmt)) != TDB_SUCCESS)
        goto clean;

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        error_log("sqlite3 step did not finish executing, possible sqlite3 "
                  "error '%s'",
                  sqlite3_errmsg(db));
        ret = TDB_FAIL;
    }

clean:
    if (stmt)
        sqlite3_finalize(stmt);

    return ret;
}

TDB_CODE create_migration_table(sqlite3 *db) {
    int ret = TDB_SUCCESS;
    sqlite3_stmt *stmt = NULL;

    if ((ret = dbh_prepare(
             db,
             "CREATE TABLE '.migrations' ('id' INTEGER PRIMARY KEY, "
             "'migration_index' INT);",
             &stmt)) != TDB_SUCCESS) {
        goto clean;
    }

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        error_log("sqlite3 step did not finish executing, possible sqlite3 "
                  "error '%s'",
                  sqlite3_errmsg(db));
        ret = TDB_FAIL;
        goto clean;
    }

    ret = insert_first_migration_value(db);

clean:
    if (stmt)
        sqlite3_finalize(stmt);

    return ret;
}

TDB_CODE get_migration_index(sqlite3 *db, int *index) {
    int ret = TDB_SUCCESS;
    sqlite3_stmt *stmt = NULL;

    if ((ret = dbh_prepare(
             db, "SELECT migration_index FROM '.migrations' WHERE id = 1;",
             &stmt)))
        goto clean;

    if (sqlite3_step(stmt) != SQLITE_ROW) {
        error_log("sqlite3 step did not contain a row");
        ret = TDB_FAIL;
        goto clean;
    }

    ret = TDB_SUCCESS;

    *index = dbh_get_column_int(stmt, 0);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        error_log(
            "statement did not finish executing as expected, sql error '%s'",
            sqlite3_errmsg(db));
        ret = TDB_FAIL;
    }

clean:
    if (stmt)
        sqlite3_finalize(stmt);

    return ret;
}

TDB_CODE update_migration_index(sqlite3 *db, int index) {
    int ret = TDB_SUCCESS;
    sqlite3_stmt *stmt = NULL;

    if ((ret = dbh_prepare(
             db, "UPDATE '.migrations' SET migration_index = ? WHERE id = 1;",
             &stmt)))
        goto clean;

    if ((ret = dbh_bind_int(db, stmt, 1, index)) != TDB_SUCCESS)
        goto clean;

    ret = dbh_sql_execute(db, stmt, NULL, NULL);
clean:
    if (stmt)
        sqlite3_finalize(stmt);

    return ret;
}

TDB_CODE migrate_database(sqlite3 *db) {
    int ret = TDB_SUCCESS;

    bool does_migration_table_exist;
    if ((ret = check_migration_database_exists(
             db, &does_migration_table_exist)) != TDB_SUCCESS) {
        error_log("Failed to check if migration database exists");
        goto clean;
    }

    if (does_migration_table_exist == false &&
        (ret = create_migration_table(db)) != TDB_SUCCESS) {
        error_log("Failed to create migration table");
        goto clean;
    }

    char *sqls[] = {(char *)_sql_one_init};
    int sqls_len[] = {_sql_one_init_len};

    int current_index = 0;
    if ((ret = get_migration_index(db, &current_index)) != TDB_SUCCESS) {
        error_log("Failed to get migration index");
        goto clean;
    }

    for (int i = current_index; i < sizeof(sqls) / sizeof(char *); i++) {
        char *sql = sqls[i];
        int sql_len = sqls_len[i];
        sql[sql_len - 1] = '\0';

        char *err;
        if (sqlite3_exec(db, sql, NULL, NULL, &err) != SQLITE_OK) {
            error_log(
                "Failed to execute migration sql #%d, possible cause \"%s\"", i,
                err);

            free(err);
            goto clean;
        }

        free(err);

        if ((ret = update_migration_index(db, i + 1)) != TDB_SUCCESS) {
            error_log("Failed to update migration index");
            goto clean;
        }
    }

clean:
    return ret;
}
