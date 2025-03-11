#include "db/migrations.h"
#include "db/sql.h"
#include "utils/generic_vec.h"
#include <sqlite3.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Vec *get_tables(sqlite3 *db) {
    Vec *v = vec_init(sizeof(char *));

    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, "SELECT name FROM sqlite_master WHERE type='table';",
                       -1, &stmt, NULL);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        char *txt = strdup((char *)sqlite3_column_text(stmt, 0));
        vec_push(v, &txt);
    }

    sqlite3_finalize(stmt);

    return v;
}

void free_tables(Vec *v) {
    for (int i = 0; i < v->length; i++) {
        free(vec_get_ref(v, i));
    }

    vec_free(v);
}

int contains_expected(char **expects, size_t expect_size, Vec *vec) {
    int match_expectation = 1;

    for (int j = 0; j < expect_size; j++) {
        char *expectation = expects[j];
        int found = 0;

        for (int i = 0; i < vec->length; i++) {
            char *table = vec_get_ref(vec, i);

            if (strcmp(table, expectation) == 0) {
                found = 1;
                break;
            }
        }

        if (found == 0) {
            match_expectation = 0;
            break;
        }
    }

    return match_expectation;
}

char *_migrations_expects[] = {
    ".migrations",    "Music",          "Album",         "Metadata",
    "MediaSource",    "Artist",         "AlbumArtists",  "Genre",
    "MetadataGenres", "Music",          "MusicAltTitle", "ArtistAltTitle",
    "AlbumAltTitle",  "AlternativeName"};

bool _migration_contains_expected_tables(sqlite3 *db) {
    Vec *tables = get_tables(db);

    int tables_match_expectation =
        contains_expected(_migrations_expects,
                          sizeof(_migrations_expects) / sizeof(char *), tables);

    free_tables(tables);

    return tables_match_expectation;
}

void test_run_migrations(sqlite3 *db, bool *passed, char **name, char **log) {
    *name = "Migrations";

    int ret = migrate_database(db);
    if (ret != TDB_SUCCESS) {
        *log = "migrate_database returned something other than TDB_SUCCESS";
        return;
    }

    if (_migration_contains_expected_tables(db))
        *passed = true;
    else {
        *passed = false;
        *log = "Tables expected to be created were not created";
    }
}

// makes sure that after the first migration you can migrate again without
// errors
void test_run_migration_2(sqlite3 *db, bool *passed, char **name, char **log) {
    *name = "Migrations x2";

    int ret = migrate_database(db);
    if (ret != TDB_SUCCESS) {
        *log = "migrate_database returned something other than TDB_SUCCESS";
        *passed = false;
        return;
    }

    if (_migration_contains_expected_tables(db))
        *passed = true;
    else {
        *passed = false;
        *log = "Tables expected to be created were not created";
    }
}
