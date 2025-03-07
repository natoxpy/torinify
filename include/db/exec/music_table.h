#ifndef _DB_EXEC_MUSIC_TABLE_H
#define _DB_EXEC_MUSIC_TABLE_H

#include "db/migrations.h"
#include "db/sql.h"
#include "db/tables.h"

#define MUSIC_TABLE "Music"
#define MUSIC_TABLE_COMMON_ROWS "title, metadata, fullpath, album, source"
#define MUSIC_TABLE_ROWS "id, title, metadata, fullpath, album, source"

const static char *DB_SQL_MUSIC_INSERT =
    SQL_INSERT(MUSIC_TABLE, MUSIC_TABLE_COMMON_ROWS, "?,?,?,?,?");

const static char *DB_SQL_MUSIC_SELECT =
    SQL_SELECT(MUSIC_TABLE, MUSIC_TABLE_ROWS, "", "");

const static char *DB_SQL_MUSIC_SELECT_WHERE_ID =
    SQL_SELECT(MUSIC_TABLE, MUSIC_TABLE_ROWS, "WHERE", "id = ?");

const static char *DB_SQL_MUSIC_SELECT_WHERE_FULLPATH =
    SQL_SELECT(MUSIC_TABLE, MUSIC_TABLE_ROWS, "WHERE", "fullpath = ?");

const static char *DB_SQL_MUSIC_DELETE_WHERE_ID =
    SQL_DELETE(MUSIC_TABLE, "WHERE", "id = ?");

// const static char *DB_SQL_MUSIC_SEARCH =
//     "SELECT id, title, fullpath, source, album, metadata FROM Music "
//     "WHERE id IN (SELECT rowid FROM MusicFTS %s);";

TDB_CODE DB_insert_music_row(sqlite3 *db, char *title, int metadata,
                             char *fullpath, int source, int album,
                             int *out_row_id);
TDB_CODE DB_query_music_single(sqlite3 *db, int id, MusicRow **out_music_row);
TDB_CODE DB_query_music_single_by_fullpath(sqlite3 *db, char *path,
                                           MusicRow **out_music_row);
TDB_CODE DB_query_music_all(sqlite3 *db, Vec **out_vec_music_row);
TDB_CODE DB_remove_music_row(sqlite3 *db, int id);

#endif
