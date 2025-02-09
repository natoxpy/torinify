#ifndef _DB_EXEC_H
#define _DB_EXEC_H
#include <db/table_repr.h>
#include <db/tables.h>
#include <errors/errors.h>
#include <sqlite3.h>
#include <stdint.h>
#include <string.h>
#include <utils/generic_vec.h>

#define DB_QUERY_BY_ID 0
#define DB_QUERY_BY_TITLE 1
#define DB_QUERY_BY_FTS5_TITLE 2
#define DB_QUERY_BY_FULLPATH 3

typedef struct {
    uint32_t by;
    union {
        uint32_t id;
        char *title;
        char *fullpath;
    } value;
} SQLQuery;

typedef enum { TDB_FAIL = -1, TDB_SUCCESS = 0 } TDB_CODE;
// Allocators
TDB_CODE DB_music_row_alloc(MusicRow **mrow);
void DB_music_row_free(MusicRow *mrow);
void DB_vec_music_row_free(Vec *vec);

// Inserts

TDB_CODE DB_insert_source_row(sqlite3 *db, char *path);

TDB_CODE DB_insert_metadata_row(sqlite3 *db, char *artist, int year,
                                char *genre);

TDB_CODE DB_insert_album_row(sqlite3 *db, char *title, char *artist, int year);

TDB_CODE DB_insert_music_row(sqlite3 *db, char *title, int metadata,
                             char *fullpath, int source, int album);

// Get

TDB_CODE DB_query_music_single(sqlite3 *db, MusicRow **out_music_row,
                               SQLQuery query);

TDB_CODE DB_query_music_all(sqlite3 *db, Vec **out_vec_music_row);

// High Level repr

TDB_CODE
DB_insert_music_repl(sqlite3 *db, char *title, MetadataRow *metadata,
                     char *fullpath, int source, int album);
// HELPERS
TDB_CODE prepare(sqlite3 *db, const char *sql, sqlite3_stmt **stmt);
void join_sql(char *buffer, size_t buffer_size, const char *sql_root,
              const char *sql_arg);

/// Return must be `free()`
char *get_column_text(sqlite3_stmt *stmt, int icol);
int get_column_int(sqlite3_stmt *stmt, int icol);
#endif
