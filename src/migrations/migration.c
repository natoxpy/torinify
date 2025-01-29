#include <migrations/migration.h>
#include <sqlite3.h>
#include <stdio.h>

int migration_table_exist(sqlite3 *db) {

    int exists = 0;

    sqlite3_stmt *stmt;

    char sql[256];
    snprintf(sql, sizeof(sql),
             "SELECT name FROM sqlite_master WHERE type='table' AND name='%s';",
             TABLE_MIGRATIONS_NAME);

    int ret = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    if (ret != SQLITE_OK) {
        goto end;
    }

    ret = sqlite3_step(stmt);
    exists = ret == SQLITE_ROW;

end:
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

    printf("c %d\n", ret);

    if (ret != SQLITE_OK)
        return -1;

    snprintf(sql, sizeof(sql),
             "INSERT INTO \"%s\" (id, migration_index) VALUES (1, 0);",
             TABLE_MIGRATIONS_NAME);

    ret = sqlite3_exec(db, sql, NULL, NULL, NULL);

    printf("a %d\n", ret);

    if (ret != SQLITE_OK)
        return -1;

    return 0;
}

int get_migration_index(sqlite3 *db) {
    sqlite3_stmt *stmt;

    char sql[256];
    snprintf(sql, sizeof(sql), "select migration_index from \"%s\"",
             TABLE_MIGRATIONS_NAME);

    int ret = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    if (ret != SQLITE_OK)
        goto cleanup;

    ret = sqlite3_step(stmt);

    if (ret == SQLITE_ROW) {
        ret = sqlite3_column_int(stmt, 0);
        sqlite3_finalize(stmt);
        return ret;
    }

cleanup:
    if (!stmt)
        sqlite3_finalize(stmt);

    if (ret != SQLITE_OK)
        return -1;

    return 0;
}

int update_migration_index(sqlite3 *db, int migration_index) {

    char sql[256];
    snprintf(sql, sizeof(sql),
             "UPDATE \"%s\" SET migration_index = %d WHERE id = 1",
             TABLE_MIGRATIONS_NAME, migration_index);

    int ret = sqlite3_exec(db, sql, NULL, NULL, NULL);

    if (ret != SQLITE_OK)
        return -1;

    return 0;
}

int m_migrations(sqlite3 *db) {
    if (!migration_table_exist(db) && create_migration_table(db) != 0)
        return -1;

    int index = get_migration_index(db);
    update_migration_index(db, index + 1);

    return 0;
}
