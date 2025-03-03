#include "db/exec/music_table.h"
#include "db/helpers.h"
#include "errors/errors.h"
#include <sqlite3.h>
#include <stdlib.h>

void *collect_music(sqlite3_stmt *stmt) {
    MusicRow *row = dbt_music_row_alloc();

    row->id = dbh_get_column_int(stmt, 0);
    row->title = dbh_get_column_text(stmt, 1);
    row->metadata = dbh_get_column_int(stmt, 2);
    row->fullpath = dbh_get_column_text(stmt, 3);
    row->album = dbh_get_column_int(stmt, 4);
    row->source = dbh_get_column_int(stmt, 5);

    return row;
}

TDB_CODE DB_insert_music_row(sqlite3 *db, char *title, int metadata,
                             char *fullpath, int source, int album,
                             int *out_row_id) {
    int ret = TDB_SUCCESS;
    sqlite3_stmt *stmt = NULL;

    if ((ret = dbh_prepare(db, DB_SQL_MUSIC_INSERT, &stmt)))
        goto clean;

    BindValue binds[] = {BIND_STR(title), BIND_INT(metadata),
                         BIND_STR(fullpath), BIND_INT(album), BIND_INT(source)};

    if ((ret = dbh_bind_array(db, stmt, binds, SIZEOF_BINDS(binds))) !=
        TDB_SUCCESS)
        goto clean;

    if ((ret = dbh_sql_execute(db, stmt, NULL, NULL)) != TDB_SUCCESS)
        goto clean;

    if (out_row_id)
        *out_row_id = sqlite3_last_insert_rowid(db);

clean:
    if (stmt)
        sqlite3_finalize(stmt);

    return ret;
}

/// @todo implement
TDB_CODE DB_query_music_single(sqlite3 *db, int id, MusicRow **out_music_row) {
    int ret;
    sqlite3_stmt *stmt = NULL;

    if ((ret = dbh_prepare(db, DB_SQL_MUSIC_SELECT_WHERE_ID, &stmt)))
        goto clean;

    if ((ret = dbh_bind_array(db, stmt, (BindValue[]){BIND_INT(id)}, 1)) !=
        TDB_SUCCESS)
        goto clean;

    MusicRow *row = NULL;
    if ((ret = dbh_sql_execute_single(db, stmt, (void *)&row, collect_music)) !=
        TDB_SUCCESS)
        goto clean;

    *out_music_row = row;
clean:
    if (stmt)
        sqlite3_finalize(stmt);

    return ret;
}

TDB_CODE DB_query_music_all(sqlite3 *db, Vec **out_vec_music_row) {
    int ret;
    sqlite3_stmt *stmt = NULL;

    if ((ret = dbh_prepare(db, DB_SQL_MUSIC_SELECT, &stmt)) != TDB_SUCCESS)
        goto clean;

    Vec *out = vec_init(sizeof(MusicRow *));
    ret = dbh_sql_execute(db, stmt, out, &collect_music);
    if (ret != TDB_SUCCESS)
        goto clean;

    *out_vec_music_row = out;

clean:
    if (stmt)
        sqlite3_finalize(stmt);

    return ret;
}

TDB_CODE DB_remove_music_row(sqlite3 *db, int id) {
    int ret;
    sqlite3_stmt *stmt = NULL;
    if ((ret = dbh_prepare(db, DB_SQL_MUSIC_DELETE_WHERE_ID, &stmt)) !=
        TDB_SUCCESS)
        goto clean;

    if ((ret = dbh_bind_array(db, stmt, (BindValue[]){BIND_INT(id)}, 1)) !=
        TDB_SUCCESS)
        goto clean;

    if ((ret = dbh_sql_execute(db, stmt, NULL, NULL)) != TDB_SUCCESS)
        goto clean;

clean:
    if (stmt)
        sqlite3_finalize(stmt);

    return ret;
}

// TDB_CODE DB_insert_music_row(sqlite3 *db, char *title, int metadata,
//                              char *fullpath, int source, int album) {
//     int ret = TDB_SUCCESS;
//     sqlite3_stmt *stmt;
//
//     if ((ret = dbh_prepare(db, DB_SQL_MUSIC_INSERT, &stmt)) != TDB_SUCCESS)
//         goto cleanup;
//
//     sqlite3_bind_text(stmt, 1, title, -1, SQLITE_STATIC);
//     sqlite3_bind_int(stmt, 2, metadata);
//     sqlite3_bind_text(stmt, 3, fullpath, -1, SQLITE_STATIC);
//
//     if (album != -1)
//         sqlite3_bind_int(stmt, 4, album);
//     else
//         sqlite3_bind_null(stmt, 4);
//
//     if (source != 1)
//         sqlite3_bind_int(stmt, 5, source);
//     else
//         sqlite3_bind_null(stmt, 5);
//
//     if (sqlite3_step(stmt) != SQLITE_DONE) {
//         error_log("Could not complete sql, possible cause \"%s\"",
//                   sqlite3_errmsg(db));
//         ret = TDB_FAIL;
//         goto cleanup;
//     }
//
// cleanup:
//     if (stmt)
//         sqlite3_finalize(stmt);
//
//     return ret;
// }
//
// TDB_CODE get_one_music_collect(sqlite3_stmt *stmt, MusicRow **music_row_out)
// {
//     MusicRow *music_row;
//
//     if (*music_row_out != NULL)
//         music_row = *music_row_out;
//     else if ((music_row = dbt_music_row_alloc()) == NULL)
//         return TDB_FAIL;
//
//     music_row->id = dbh_get_column_int(stmt, 0);
//     music_row->title = dbh_get_column_text(stmt, 1);
//     music_row->fullpath = dbh_get_column_text(stmt, 2);
//     music_row->source = dbh_get_column_int(stmt, 3);
//     music_row->album = dbh_get_column_int(stmt, 4);
//     music_row->metadata = dbh_get_column_int(stmt, 5);
//
//     *music_row_out = music_row;
//
//     return TDB_SUCCESS;
// }
//
// TDB_CODE DB_query_music_single(sqlite3 *db, MusicRow **out_music_row,
//                                SQLQuery query) {
//     int ret;
//     sqlite3_stmt *stmt;
//     MusicRow *music_row;
//
//     char sql[255];
//
//     switch (query.by) {
//     case DB_QUERY_BY_ID:
//         dbh_join_sql(sql, sizeof(sql), DB_SQL_MUSIC_SELECT, DB_SQL_WHERE_ID);
//         break;
//     case DB_QUERY_BY_TITLE:
//         dbh_join_sql(sql, sizeof(sql), DB_SQL_MUSIC_SELECT,
//         DB_SQL_WHERE_TITLE); break;
//     case DB_QUERY_BY_FTS5_TITLE:
//         dbh_join_sql(sql, sizeof(sql), DB_SQL_MUSIC_SEARCH,
//                      DB_SQL_WHERE_TITLE_MATCH);
//         break;
//     case DB_QUERY_BY_FULLPATH:
//         dbh_join_sql(sql, sizeof(sql), DB_SQL_MUSIC_SELECT,
//                      DB_SQL_WHERE_FULLPATH);
//         break;
//     default:
//         goto cleanup;
//     };
//
//     if ((ret = dbh_prepare(db, sql, &stmt)) != TDB_SUCCESS)
//         goto cleanup;
//
//     switch (query.by) {
//     case DB_QUERY_BY_ID:
//         ret = dbh_bind_int(db, stmt, 1, query.value.id);
//         break;
//     case DB_QUERY_BY_TITLE:
//     case DB_QUERY_BY_FTS5_TITLE:
//         ret = dbh_bind_text(db, stmt, 1, query.value.title);
//         break;
//     case DB_QUERY_BY_FULLPATH:
//         ret = dbh_bind_text(db, stmt, 1, query.value.fullpath);
//         break;
//     default:
//         goto cleanup;
//     }
//
//     if (ret != TDB_SUCCESS)
//         goto cleanup;
//
//     ret = sqlite3_step(stmt);
//
//     if (ret == SQLITE_ROW) {
//         ret = get_one_music_collect(stmt, &music_row);
//         *out_music_row = music_row;
//     } else if (ret == SQLITE_DONE) {
//         dbt_music_row_free(music_row);
//         *out_music_row = NULL;
//         ret = TDB_SUCCESS;
//     } else {
//         error_log("Sqlite return value was unnexpected, error message
//         \"%s\"",
//                   sqlite3_errmsg(db));
//         ret = TDB_FAIL;
//     }
//
// cleanup:
//     if (stmt)
//         sqlite3_finalize(stmt);
//
//     if (ret != TDB_SUCCESS && music_row != NULL)
//         dbt_music_row_free(music_row);
//
//     return ret;
// }
//
// TDB_CODE DB_query_music_all(sqlite3 *db, Vec **out_vec_music_row) {
//     int ret = TDB_SUCCESS;
//     sqlite3_stmt *stmt;
//     Vec *vec_music = vec_init(sizeof(MusicRow *));
//
//     if (vec_music == NULL) {
//         ret = TDB_FAIL;
//         goto cleanup;
//     }
//
//     char sql[255];
//     dbh_join_sql(sql, sizeof(sql), DB_SQL_MUSIC_SELECT, "");
//
//     if ((ret = dbh_prepare(db, sql, &stmt)) != TDB_SUCCESS)
//         goto cleanup;
//
//     MusicRow *music_row = dbt_music_row_alloc();
//
//     while (sqlite3_step(stmt) != SQLITE_DONE) {
//         ret = get_one_music_collect(stmt, &music_row);
//         vec_push(vec_music, &music_row);
//     }
//
//     // free(music_row);
//     *out_vec_music_row = vec_music;
//
// cleanup:
//     if (stmt)
//         sqlite3_finalize(stmt);
//
//     return ret;
// }
