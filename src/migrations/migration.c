#include <dirent.h>
#include <migrations/migration.h>
#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

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

struct MigrationFiles {
    char **names;
    int *indexes;
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
    if (mfiles->indexes)
        free(mfiles->indexes);

    free(mfiles);
}

int get_migration_files(char *migrations_dir,
                        struct MigrationFiles **out_mfiles,
                        int current_latest) {
    struct MigrationFiles *mfiles = malloc(sizeof(struct MigrationFiles));

    mfiles->count = 0;
    mfiles->names = malloc(sizeof(char *) * mfiles->count);
    mfiles->indexes = malloc(sizeof(int) * mfiles->count);

    DIR *FD;
    struct dirent *in_file;

    if (NULL == (FD = opendir(migrations_dir)))
        return 0;

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

        int index = atoi(c);

        if (index <= current_latest)
            continue;

        mfiles->count++;

        mfiles->names = realloc(mfiles->names, sizeof(char *) * mfiles->count);
        mfiles->indexes = realloc(mfiles->indexes, sizeof(int) * mfiles->count);

        mfiles->names[mfiles->count - 1] = strdup(in_file->d_name);
        mfiles->indexes[mfiles->count - 1] = index;
    }

    closedir(FD);

    *out_mfiles = mfiles;

    return 0;
}

int m_migrations(sqlite3 *db, char *migrations_dir) {
    if (!migration_table_exist(db) && create_migration_table(db) != 0)
        return -1;

    update_migration_index(db, 4);
    int current_latest_index = get_migration_index(db);

    struct MigrationFiles *mfiles;
    get_migration_files(migrations_dir, &mfiles, current_latest_index);

    for (int i = 0; i < mfiles->count; i++) {
        printf("name %s\n", mfiles->names[i]);
        printf("index %d\n\n", mfiles->indexes[i]);
    }

    migration_files_free(mfiles);

    return 0;
}
