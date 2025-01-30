#include "errors/errors.h"
#include <audio/audio.h>
#include <dirent.h>
#include <migrations/migration.h>
#include <sqlite3.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int migration_table_exist(sqlite3 *db) {
    int exists = 0;

    sqlite3_stmt *stmt;

    char sql[256];
    snprintf(sql, sizeof(sql),
             "SELECT name FROM sqlite_master WHERE type='table' AND name='%s';",
             TABLE_MIGRATIONS_NAME);

    int ret = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    if (ret != SQLITE_OK) {
        error_log("Could not query SQLITE to check if \"%s\" table exists",
                  TABLE_MIGRATIONS_NAME);

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

struct MigrationFiles {
    char **names;
    int *indeces;
    int count;
};

void migration_files_free(struct MigrationFiles *mfiles) {
    if (!mfiles)
        return;

    if (mfiles->names) {
        for (int i = 0; i < mfiles->count; i++) {
            free(mfiles->names[i]);
        }
        free(mfiles->names); // Then free the array of char pointers
    }
    if (mfiles->indeces)
        free(mfiles->indeces);

    free(mfiles);
}

/// Always returns everything inside of `MigrationFiles` ordered,
/// and only returns files with indexes larger than `current_latest`
T_CODE get_migration_files(char *migrations_dir,
                           struct MigrationFiles **out_mfiles,
                           int current_latest) {
    struct MigrationFiles *mfiles = malloc(sizeof(struct MigrationFiles));

    mfiles->count = 0;
    mfiles->names = malloc(sizeof(char *) * mfiles->count);
    mfiles->indeces = malloc(sizeof(int) * mfiles->count);

    DIR *FD;
    struct dirent *in_file;

    if (NULL == (FD = opendir(migrations_dir))) {
        error_log("Could not open migrations directory in path \"%s\"",
                  migrations_dir);

        return T_FAIL;
    }

    while ((in_file = readdir(FD))) {
        if (!strcmp(in_file->d_name, ".") || !strcmp(in_file->d_name, ".."))
            continue;

        char c[256];
        int c_i = 0;

        for (int i = 0; i < 30; i++) {
            if (in_file->d_name[i] == '_')
                break;

            c[c_i] = in_file->d_name[i];
            c_i++;
        }

        c[c_i] = '\0';

        int index = atoi(c);

        if (index <= current_latest)
            continue;

        mfiles->count++;

        char **names = realloc(mfiles->names, sizeof(char *) * mfiles->count);
        int *indeces = realloc(mfiles->indeces, sizeof(int) * mfiles->count);

        if (!mfiles->names || !mfiles->indeces) {
            migration_files_free(mfiles);
            if (!mfiles->names)
                error_log("Failed to allocate enough memory for Migration File "
                          "Names");
            if (mfiles->indeces)
                error_log("Failed to allocate enough memory for Migration File "
                          "indeces");
            goto cleanup;
        }

        mfiles->names = names;
        mfiles->indeces = indeces;

        char *name = strdup(in_file->d_name);

        if (!name) {
            migration_files_free(mfiles);
            error_log(
                "Failed to allocate enough memory for Migration File Name");

            goto cleanup;
        }

        mfiles->names[mfiles->count - 1] = name;
        mfiles->indeces[mfiles->count - 1] = index;
    }

    int swapped = 1;

    while (swapped == 1) {
        swapped = 0;

        for (int i = 1; i <= mfiles->count - 1; i++) {
            int pv = mfiles->indeces[i - 1];
            int cr = mfiles->indeces[i];

            if (pv > cr) {
                swapped = 1;

                mfiles->indeces[i] = pv;
                mfiles->indeces[i - 1] = cr;

                char *pv_name = mfiles->names[i - 1];
                char *cr_name = mfiles->names[i];

                mfiles->names[i] = pv_name;
                mfiles->names[i - 1] = cr_name;
            }
        }
    }

cleanup:
    if (FD)
        closedir(FD);

    *out_mfiles = mfiles;

    return T_SUCCESS;
}

T_CODE m_migrations(sqlite3 *db, char *migrations_dir) {
    if (!migration_table_exist(db) && create_migration_table(db) != T_SUCCESS) {
        error_log("Unable to continue due to missing table \"%s\" even after "
                  "creation attempt",
                  TABLE_MIGRATIONS_NAME);
        return T_FAIL;
    }

    int current_latest_index = get_migration_index(db);

    struct MigrationFiles *mfiles;
    int ret =
        get_migration_files(migrations_dir, &mfiles, current_latest_index);

    if (ret != T_SUCCESS) {
        error_log("Could not get Migration Files");
        goto cleanup;
    }

    if (mfiles->count == 0)
        goto cleanup;

    ret = sqlite3_exec(db, "BEGIN TRANSACTION;", NULL, NULL, NULL);

    int latest_index = 0;

    for (int i = 0; i < mfiles->count; i++) {
        char *filename = mfiles->names[i];
        int index = mfiles->indeces[i];

        char path[256];
        snprintf(path, sizeof(path), "%s/%s", migrations_dir, filename);

        uint8_t *filedata;
        size_t size = f_read_file(path, &filedata);

        char *sql = (char *)filedata;

        ret = sqlite3_exec(db, sql, NULL, NULL, NULL);

        if (ret != SQLITE_OK) {
            error_log(
                "Migration file \"%s\" cause \"%s\", Attempting Rollback ",
                filename, sqlite3_errmsg(db));

            ret = sqlite3_exec(db, "ROLLBACK;", NULL, NULL, NULL);

            if (ret != SQLITE_OK)
                error_log("Rollback failed cause \"%s\"", sqlite3_errmsg(db));

            free(filedata);

            ret = T_FAIL;
            goto cleanup;
        }

        free(filedata);
        latest_index = index;
    }

    ret = update_migration_index(db, latest_index);

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
    if (mfiles)
        migration_files_free(mfiles);

    if (ret != T_SUCCESS)
        return T_FAIL;

    return T_SUCCESS;
}
