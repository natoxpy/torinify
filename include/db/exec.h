#ifndef _DB_EXEC_H
#define _DB_EXEC_H
#include <db/table_repr.h>
#include <db/tables.h>
#include <errors/errors.h>
#include <sqlite3.h>

typedef enum { TDB_FAIL = -1, TDB_SUCCESS = 0 } TDB_CODE;
// Allocators
TDB_CODE DB_music_row_alloc(MusicRow **mrow);
void DB_music_row_free(MusicRow *mrow);

// Inserts

TDB_CODE DB_insert_source_row(sqlite3 *db, char *path);

TDB_CODE DB_insert_metadata_row(sqlite3 *db, char *artist, int year,
                                char *genre);

TDB_CODE DB_insert_album_row(sqlite3 *db, char *title, char *artist, int year);

TDB_CODE DB_insert_music_row(sqlite3 *db, char *title, int metadata,
                             char *fullpath, int source, int album);

// Get

TDB_CODE DB_get_one_music_where_id(sqlite3 *db, MusicRow **out_music_row,
                                   int id);

TDB_CODE DB_get_one_music_where_fullpath(sqlite3 *db, MusicRow **music_row,
                                         char *fullpath);

// High Level repr

TDB_CODE DB_insert_music_repl(sqlite3 *db, char *title, MetadataRow *metadata,
                              char *fullpath, int source, int album);

#endif
