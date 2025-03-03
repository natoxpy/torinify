#include <audio/audio.h>
#include <db/migrations.h>
#include <db/sql.h>
#include <errors/errors.h>
#include <sqlite3.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <utils/generic_vec.h>

/*
int migration_table_exist(sqlite3 *db) {
    int exists = 0;

    sqlite3_stmt *stmt;

    char sql[256];
    snprintf(sql, sizeof(sql),
             "SELECT name FROM sqlite_master WHERE type='table' AND name='%s';",
             TABLE_MIGRATIONS_NAME);

    int ret = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    if (ret != SQLITE_OK) {
        error_log("Could not query SQLITE to check if \"%s\" table exists, "
                  "possible error \"%s\"",
                  TABLE_MIGRATIONS_NAME, sqlite3_errmsg(db));

        goto end;
    }

    exists = sqlite3_step(stmt) == SQLITE_ROW;
end:
    if (stmt)
        sqlite3_finalize(stmt);

    return exists;
}

// return `0` on `success`
int create_migration_table(sqlite3 *db) {
    char sql[256];
    snprintf(sql, sizeof(sql),
             "CREATE TABLE \"%s\" (id INTEGER PRIMARY KEY, "
             "migration_index INTEGER);",
             TABLE_MIGRATIONS_NAME);

    int ret = sqlite3_exec(db, sql, NULL, NULL, NULL);

    if (ret != SQLITE_OK) {
        error_log("Could not execute SQLITE query to create table \"%s\", due "
                  "to sqlite3 error: %s",
                  TABLE_MIGRATIONS_NAME, sqlite3_errmsg(db));
        return T_FAIL;
    }

    snprintf(sql, sizeof(sql),
             "INSERT INTO \"%s\" (id, migration_index) VALUES (1, 0);",
             TABLE_MIGRATIONS_NAME);

    ret = sqlite3_exec(db, sql, NULL, NULL, NULL);

    if (ret != SQLITE_OK) {
        error_log("Could not execute SQLITE add initial migration data into "
                  "table \"%s\"",
                  TABLE_MIGRATIONS_NAME);

        return T_FAIL;
    }

    return T_SUCCESS;
}

int get_migration_index(sqlite3 *db) {
    int sql_result = -1;
    sqlite3_stmt *stmt;

    char sql[256];
    snprintf(sql, sizeof(sql), "select migration_index from \"%s\"",
             TABLE_MIGRATIONS_NAME);

    int ret = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    if (ret != SQLITE_OK) {
        error_log("Could not prepare statement to get migration index from "
                  "table \"%s\"",
                  TABLE_MIGRATIONS_NAME);
        goto cleanup;
    }

    sql_result = sqlite3_step(stmt);

    if (sql_result == SQLITE_ROW) {
        sql_result = sqlite3_column_int(stmt, 0);
    } else {
        error_log(
            "Coult not find latest migration_index information in table \"%s\"",
            TABLE_MIGRATIONS_NAME);

        sql_result = -1;
    }
cleanup:
    if (stmt)
        sqlite3_finalize(stmt);

    if (sql_result != -1)
        return sql_result;

    return T_SUCCESS;
}

T_CODE update_migration_index(sqlite3 *db, int migration_index) {

    char sql[256];
    snprintf(sql, sizeof(sql),
             "UPDATE \"%s\" SET migration_index = %d WHERE id = 1",
             TABLE_MIGRATIONS_NAME, migration_index);

    int ret = sqlite3_exec(db, sql, NULL, NULL, NULL);

    if (ret != SQLITE_OK) {
        error_log("Could not query to get latest migration index");
        return T_FAIL;
    }

    return T_SUCCESS;
}

Vec *get_migrations(sqlite3 *db) {
    Vec *migrations = vec_init(sizeof(char **));
    int start = get_migration_index(db);

    char *static_migrations[1] = {(char *)_sql_one_init};
    const unsigned int static_migrations_len = 1;

    for (int i = start; i < static_migrations_len; i++) {
        vec_push(migrations, (void *)&static_migrations[i]);
    }

    return migrations;
}

T_CODE m_migrations(sqlite3 *db) {
    if (!migration_table_exist(db) && create_migration_table(db) != T_SUCCESS) {
        error_log("Unable to continue due to missing table \"%s\" even after "
                  "creation attempt",
                  TABLE_MIGRATIONS_NAME);
        return T_FAIL;
    }

    int ret = 0;

    int current_latest_index = get_migration_index(db);

    printf("current: %d\n", current_latest_index);

    Vec *migrations = get_migrations(db);

    ret = sqlite3_exec(db, "BEGIN TRANSACTION;", NULL, NULL, NULL);

    int latest_index = 0; // << BRUHHHH

    for (int index = 1; index <= migrations->length; index++) {

        char *sql = vec_get_ref(migrations, index - 1);

        ret = sqlite3_exec(db, sql, NULL, NULL, NULL);

        if (ret != SQLITE_OK) {
            error_log("Migration cause \"%s\", Attempting Rollback ",
                      sqlite3_errmsg(db));

            ret = sqlite3_exec(db, "ROLLBACK;", NULL, NULL, NULL);

            if (ret != SQLITE_OK)
                error_log("Rollback failed cause \"%s\"", sqlite3_errmsg(db));

            ret = T_FAIL;
            goto cleanup;
        }

        latest_index = index;
    }

    ret = update_migration_index(db, latest_index); // << BRUHHHH

    if (ret != T_SUCCESS) {
        error_log(
            "Migration failed to commit bause \"%s\", Attempting Rollback",
            sqlite3_errmsg(db));

        ret = sqlite3_exec(db, "ROLLBACK;", NULL, NULL, NULL);

        if (ret != SQLITE_OK)
            error_log("Rollback failed cause \"%s\"", sqlite3_errmsg(db));

        ret = T_FAIL;
        goto cleanup;
    }

    ret = sqlite3_exec(db, "COMMIT;", NULL, NULL, NULL);

    if (ret != SQLITE_OK) {
        error_log("Migration failed to commit bause \"%s\"",
                  sqlite3_errmsg(db));
        ret = T_FAIL;
    }

cleanup:
    vec_free(migrations);

    if (ret != T_SUCCESS)
        return T_FAIL;

    return T_SUCCESS;
}
*/
