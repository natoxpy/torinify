#include "db/exec/music_table.h"
#include "db/exec/metadata_table.h"
#include "db/helpers.h"
#include "db/tables.h"
#include "errors/errors.h"
#include <sqlite3.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *get_music(sqlite3 *db, char **out) {
    char *ret = NULL;
    sqlite3_stmt *stmt = NULL;

    if (sqlite3_prepare_v2(
            db, "SELECT title FROM Music WHERE fullpath = '/path/to/song';", -1,
            &stmt, NULL)) {
        ret = "Unable to prepare sqlite3";
        goto cleanup;
    }

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        *out = strdup((char *)sqlite3_column_text(stmt, 0));
    } else {
        ret = "Sqlite3 did not found SQLITE_ROW";
        goto cleanup;
    }

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        *out = NULL;
        ret = "Sqlite3 final step wasn't SQLITE_DONE";
    }

cleanup:
    if (stmt)
        sqlite3_finalize(stmt);

    return ret;
}

void test_add_music(sqlite3 *db, bool *passed, char **name, char **log) {
    *name = "Add Music";

    int metadata_row_id;
    int ret =
        DB_insert_metadata_row(db, "Artist", 2020, "Music", &metadata_row_id);

    if (ret != TDB_SUCCESS) {
        *passed = false;
        *log =
            "DB_insert_metadata_row returned something other than TDB_SUCCESS";
        return;
    }

    ret = DB_insert_music_row(db, "Music added", metadata_row_id,
                              "/path/to/song", -1, -1, NULL);

    if (ret != TDB_SUCCESS) {
        *passed = false;
        *log = "DB_insert_music_row returned something other than TDB_SUCCESS";
        return;
    }

    char *row_name = NULL;
    char *msg = get_music(db, &row_name);

    if (msg != NULL) {
        *log = msg;
        *passed = false;
        return;
    }

    if (strcmp(row_name, "Music added") != 0) {
        *log = "Row name did not match expected name";
        *passed = false;
        free(row_name);
        return;
    }

    free(row_name);

    *passed = true;
}

void test_query_music(sqlite3 *db, bool *passed, char **name, char **log) {
    *name = "Query Music";

    MusicRow *row;
    int ret = DB_query_music_single(db, 1, &row);

    if (ret != TDB_SUCCESS) {
        *passed = false;
        *log =
            "DB_query_music_single returned something other than TDB_SUCCESS";
        return;
    }

    if (strcmp(row->fullpath, "/path/to/song") != 0) {
        *passed = false;
        *log = "Row did not match expected fullpath value";
        return;
    }

    dbt_music_row_free(row);

    *passed = true;
}

void test_remove_music(sqlite3 *db, bool *passed, char **name, char **log) {
    *name = "Remove Music";

    int ret = DB_remove_music_row(db, 1);

    if (!(ret == TDB_SUCCESS || ret == TDB_NOT_FOUND)) {
        *passed = false;
        *log = "DB_remove_music_row returned something other than TDB_SUCCESS "
               "or TDB_NOT_FOUND";
        return;
    }

    char *row_name = NULL;
    char *msg = get_music(db, &row_name);

    if (row_name != NULL) {
        *log = "Row name did not match expected name";
        *passed = false;
        free(row_name);
        return;
    }

    *passed = true;
}

TDB_CODE add_music_row(sqlite3 *db, char *name, char *ppath, char **plog) {
    int metadata_row_id;
    int ret =
        DB_insert_metadata_row(db, "Artist", 2020, "Music", &metadata_row_id);

    if (ret != TDB_SUCCESS) {
        *plog =
            "DB_insert_metadata_row returned something other than TDB_SUCCESS";
        return TDB_FAIL;
    }

    ret = DB_insert_music_row(db, name, metadata_row_id, ppath, -1, -1, NULL);
    return TDB_SUCCESS;
}

// Bulk insert method could be considered later
void test_insert_many_music_with_transaction(sqlite3 *db, bool *passed,
                                             char **name, char **log) {
    *name = "Insert Many Music";
    int ret;

    ret = dbh_start_transaction(db);
    if (ret != TDB_SUCCESS) {
        *passed = false;
        *log = "Unable to start transaction";
        return;
    }

    char *songs[] = {"Song 1", "Song 2", "Song 3"};
    char *paths[] = {"path/to/one", "path/to/two", "path/to/three"};

    for (int i = 0; i < sizeof(songs) / sizeof(char *); i++) {
        char *song_title = songs[i];
        char *song_path = paths[i];

        ret = add_music_row(db, song_title, song_path, log);
        if (ret != TDB_SUCCESS) {
            *passed = false;
            return;
        }
    }

    ret = dbh_commit_transaction(db);

    if (ret != TDB_SUCCESS) {
        *passed = false;
        *log = "Unable to end transaction";
        return;
    }

    Vec *musics;
    ret = DB_query_music_all(db, &musics);

    int music_len = musics->length;

    dbt_music_vec_rows_free(musics);

    if (music_len != 3) {
        *passed = false;
        *log = "Not all expected songs where added";
        return;
    }

    *passed = true;
}

// This test could make more sense later when used with a query function for
// more specific things
void test_query_all_music(sqlite3 *db, bool *passed, char **name, char **log) {
    *name = "Query All Music";

    Vec *musics;
    int ret = DB_query_music_all(db, &musics);

    char *expected_arr[] = {"Song 1", "Song 2", "Song 3"};

    bool found_all_expected = true;

    for (int i = 0; i < sizeof(expected_arr) / sizeof(char *); i++) {
        char *expected = expected_arr[i];
        bool expected_found = false;

        for (int j = 0; j < musics->length; j++) {
            MusicRow *row = vec_get_ref(musics, j);

            if (strcmp(row->title, expected)) {
                expected_found = true;
                break;
            }
        }

        if (expected_found == false) {
            found_all_expected = false;
            break;
        }
    }

    dbt_music_vec_rows_free(musics);

    if (!found_all_expected) {
        *passed = false;
        *log = "Did not found all the songs that were expected";
        return;
    }

    *passed = true;
}

// Bulk Deletion method could be considsered later
void test_remove_all_music(sqlite3 *db, bool *passed, char **name, char **log) {
    *name = "Remove All Music";

    Vec *musics;
    int ret = DB_query_music_all(db, &musics);
    if (ret != TDB_SUCCESS) {
        *passed = false;
        return;
    }

    for (int i = 0; i < musics->length; i++) {
        MusicRow *row = vec_get_ref(musics, i);

        ret = DB_remove_music_row(db, row->id);
        if (ret != TDB_SUCCESS) {
            *passed = false;
            *log =
                "DB_remove_music_row returned something other than TDB_SUCCESS";
            return;
        }
    }

    dbt_music_vec_rows_free(musics);

    ret = DB_query_music_all(db, &musics);

    int music_len = musics->length;

    dbt_music_vec_rows_free(musics);

    if (music_len != 0) {
        *passed = false;
        *log = "Not all musics were deleted";
        return;
    }

    *passed = true;
}
