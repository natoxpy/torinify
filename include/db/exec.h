#ifndef _DB_EXEC_H
#define _DB_EXEC_H

#include <db/sql.h>
#include <db/tables.h>
#include <errors/errors.h>
#include <sqlite3.h>

TDB_CODE DB_insert_source_row(sqlite3 *db, char *path);

TDB_CODE DB_query_source_all(sqlite3 *db, Vec **sources);

TDB_CODE DB_insert_metadata_row(sqlite3 *db, char *artist, int year,
                                char *genre);

TDB_CODE DB_insert_album_row(sqlite3 *db, char *title, char *artist, int year);

TDB_CODE DB_insert_music_row(sqlite3 *db, char *title, int metadata,
                             char *fullpath, int source, int album);

TDB_CODE DB_query_music_single(sqlite3 *db, MusicRow **out_music_row,
                               SQLQuery query);

TDB_CODE DB_query_music_all(sqlite3 *db, Vec **out_vec_music_row);

#endif
