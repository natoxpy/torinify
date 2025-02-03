#include <db/exec.h>
#include <db/sql.h>
#include <db/tables.h>
#include <sqlite3.h>
#include <stdio.h>
#include <string.h>

// Allocators
TDB_CODE DB_music_row_alloc(MusicRow **mrow) {
    MusicRow *music_row = malloc(sizeof(MusicRow));

    if (!music_row) {
        error_log("Failed to allocate memory for music_row");
        free(music_row);
        return TDB_FAIL;
    }

    music_row->id = -1;
    music_row->title = NULL;
    music_row->fullpath = NULL;
    music_row->source = -1;
    music_row->album = -1;
    music_row->metadata = -1;

    *mrow = music_row;

    return TDB_SUCCESS;
}

void DB_music_row_free(MusicRow *mrow) {
    if (!mrow)
        return;

    if (mrow->title)
        free(mrow->title);

    if (mrow->fullpath)
        free(mrow->fullpath);

    free(mrow);
}

// Helpers

void join_sql(char *buffer, size_t buffer_size, const char *sql_root,
              const char *sql_arg) {
    snprintf(buffer, buffer_size, sql_root, sql_arg);
};

TDB_CODE start_transaction(sqlite3 *db) {
    int ret = sqlite3_exec(db, "BEGIN TRANSACTION;", 0, 0, 0);

    if (ret != SQLITE_OK) {
        error_log("Could not begin transaction, possible cause \"%s\"",
                  sqlite3_errmsg(db));
        return TDB_FAIL;
    }

    return TDB_SUCCESS;
}

TDB_CODE rollback_transaction(sqlite3 *db) {
    int ret = sqlite3_exec(db, "ROLLBACK;", 0, 0, 0);

    if (ret != SQLITE_OK) {
        error_log("Could not rollback, possible cause \"%s\"",
                  sqlite3_errmsg(db));
        return TDB_FAIL;
    }

    return TDB_SUCCESS;
}

TDB_CODE commit_transaction(sqlite3 *db) {
    int ret = sqlite3_exec(db, "COMMIT;", 0, 0, 0);

    if (ret != SQLITE_OK) {
        error_log("Could not commit changes, possible cause \"%s\"",
                  sqlite3_errmsg(db));
        return TDB_FAIL;
    }

    return TDB_SUCCESS;
}

TDB_CODE prepare(sqlite3 *db, const char *sql, sqlite3_stmt **stmt) {
    int ret = sqlite3_prepare_v2(db, sql, -1, stmt, NULL);

    if (ret != SQLITE_OK) {
        error_log("Could not prepare sql string \"%s\", possible cause \"%s\"",
                  sql, sqlite3_errmsg(db));
        return TDB_FAIL;
    }

    return TDB_SUCCESS;
}

TDB_CODE bind_text(sqlite3 *db, sqlite3_stmt *stmt, int n, char *text) {
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

TDB_CODE bind_int(sqlite3 *db, sqlite3_stmt *stmt, int n, int value) {
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

/// Return must be `free()`
char *get_column_text(sqlite3_stmt *stmt, int icol) {
    if (sqlite3_column_type(stmt, icol) == SQLITE_NULL)
        return NULL;
    return strdup((const char *)sqlite3_column_text(stmt, icol));
}

int get_column_int(sqlite3_stmt *stmt, int icol) {
    if (sqlite3_column_type(stmt, icol) == SQLITE_NULL)
        return -1;

    return sqlite3_column_int(stmt, icol);
}

// Inserters

TDB_CODE DB_insert_metadata_row(sqlite3 *db, char *artist, int year,
                                char *genre) {
    int ret;
    sqlite3_stmt *stmt = NULL;
    sqlite_int64 row_id = -1; // Variable to store the inserted row's ID

    if ((ret = prepare(db, DB_SQL_METADATA_INSERT, &stmt)) != TDB_SUCCESS) {
        goto cleanup;
    }

    sqlite3_bind_text(stmt, 1, artist, -1, SQLITE_STATIC);

    if (year != -1)
        sqlite3_bind_int(stmt, 2, year);
    else
        sqlite3_bind_null(stmt, 2);

    if (genre != NULL)
        sqlite3_bind_text(stmt, 3, genre, -1, SQLITE_STATIC);
    else
        sqlite3_bind_null(stmt, 3);

    ret = sqlite3_step(stmt);
    if (ret != SQLITE_DONE) {
        goto cleanup;
    }

    row_id = sqlite3_last_insert_rowid(db);
cleanup:
    if (stmt)
        sqlite3_finalize(stmt);

    return (ret == SQLITE_DONE) ? row_id : TDB_FAIL;
}

TDB_CODE DB_insert_album_row(sqlite3 *db, char *title, char *artist, int year) {
    int ret;
    sqlite3_stmt *stmt = NULL;

cleanup:
    if (stmt)
        sqlite3_finalize(stmt);

    return ret;
}

TDB_CODE DB_insert_music_row(sqlite3 *db, char *title, int metadata,
                             char *fullpath, int source, int album) {
    int ret = TDB_SUCCESS;
    sqlite3_stmt *stmt;

    if ((ret = prepare(db, DB_SQL_MUSIC_INSERT, &stmt)) != TDB_SUCCESS)
        goto cleanup;

    sqlite3_bind_text(stmt, 1, title, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, metadata);
    sqlite3_bind_text(stmt, 3, fullpath, -1, SQLITE_STATIC);

    if (album != -1)
        sqlite3_bind_int(stmt, 4, album);
    else
        sqlite3_bind_null(stmt, 4);

    if (source != 1)
        sqlite3_bind_int(stmt, 5, source);
    else
        sqlite3_bind_null(stmt, 5);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        error_log("Could not complete sql, possible cause \"%s\"",
                  sqlite3_errmsg(db));
        ret = TDB_FAIL;
        goto cleanup;
    }

cleanup:
    if (stmt)
        sqlite3_finalize(stmt);

    return ret;
}
// Get

TDB_CODE DB_get_one_music_where_id(sqlite3 *db, MusicRow **out_music_row,
                                   int id) {
    int ret;
    sqlite3_stmt *stmt;
    MusicRow *music_row;

    char sql[255];
    join_sql(sql, sizeof(sql), DB_SQL_MUSIC_SELECT, DB_SQL_WHERE_ID);

    if ((ret = prepare(db, sql, &stmt)) != TDB_SUCCESS)
        goto cleanup;

    if (bind_int(db, stmt, 1, id) != TDB_SUCCESS)
        goto cleanup;

    ret = sqlite3_step(stmt);

    if (ret == SQLITE_ROW) {
        if (DB_music_row_alloc(&music_row) != TDB_SUCCESS)
            goto cleanup;

        music_row->id = get_column_int(stmt, 0);
        music_row->title = get_column_text(stmt, 1);
        music_row->fullpath = get_column_text(stmt, 2);
        music_row->source = get_column_int(stmt, 3);
        music_row->album = get_column_int(stmt, 4);
        music_row->metadata = get_column_int(stmt, 5);

        *out_music_row = music_row;
        ret = TDB_SUCCESS;
    } else if (ret == SQLITE_DONE) {
        DB_music_row_free(music_row);
        *out_music_row = NULL;
        ret = TDB_SUCCESS;
    } else {
        error_log("Sqlite return value was unnexpected, error message \"%s\"",
                  sqlite3_errmsg(db));
        ret = TDB_FAIL;
    }

cleanup:
    if (stmt)
        sqlite3_finalize(stmt);

    if (ret != TDB_SUCCESS && music_row)
        DB_music_row_free(music_row);

    return ret;
}

TDB_CODE DB_get_one_music_where_fullpath(sqlite3 *db, MusicRow **out_music_row,
                                         char *fullpath) {
    int ret;
    sqlite3_stmt *stmt;
    MusicRow *music_row;

    char sql[255];
    join_sql(sql, sizeof(sql), DB_SQL_MUSIC_SELECT, DB_SQL_WHERE_FULLPATH);

    if ((ret = prepare(db, sql, &stmt)) != TDB_SUCCESS)
        goto cleanup;

    if (bind_text(db, stmt, 1, fullpath) != TDB_SUCCESS)
        goto cleanup;

    ret = sqlite3_step(stmt);

    if (ret == SQLITE_ROW) {
        if (DB_music_row_alloc(&music_row) != TDB_SUCCESS)
            goto cleanup;

        music_row->id = get_column_int(stmt, 0);
        music_row->title = get_column_text(stmt, 1);
        music_row->fullpath = get_column_text(stmt, 2);
        music_row->source = get_column_int(stmt, 3);
        music_row->album = get_column_int(stmt, 4);
        music_row->metadata = get_column_int(stmt, 5);

        *out_music_row = music_row;
        ret = TDB_SUCCESS;
    } else if (ret == SQLITE_DONE) {
        *out_music_row = NULL;
        ret = TDB_SUCCESS;
    } else {
        error_log("Sqlite return value was unnexpected, error message \"%s\"",
                  sqlite3_errmsg(db));
        ret = TDB_FAIL;
    }

cleanup:
    if (stmt)
        sqlite3_finalize(stmt);

    if (ret != TDB_SUCCESS && music_row)
        DB_music_row_free(music_row);

    return ret;
}

// High Level Repr

TDB_CODE DB_insert_music_repl(sqlite3 *db, char *title, MetadataRow *metadata,
                              char *fullpath, int source, int album) {
    int ret = TDB_SUCCESS;

    if ((ret = start_transaction(db)) != TDB_SUCCESS)
        goto skipRollback;

    int metadata_id = DB_insert_metadata_row(db, metadata->artist,
                                             metadata->year, metadata->genre);

    if (metadata_id == TDB_FAIL) {
        error_log("Metadata row insert failed");
        goto cleanup;
    }

    ret = DB_insert_music_row(db, title, metadata_id, fullpath, source, album);

    if (ret != T_SUCCESS) {
        error_log("Music row insert failed");
        goto cleanup;
    }

    if ((ret = commit_transaction(db)) != TDB_SUCCESS)
        goto cleanup;

cleanup:
    if (ret != TDB_SUCCESS) {
        rollback_transaction(db);
        return ret;
    }

skipRollback:
    return ret;
}
