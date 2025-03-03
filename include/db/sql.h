
#ifndef _SQL_QUERY_H
#define _SQL_QUERY_H

#include <stdint.h>
#define DB_QUERY_BY_ID 0
#define DB_QUERY_BY_TITLE 1
#define DB_QUERY_BY_FTS5_TITLE 2
#define DB_QUERY_BY_FULLPATH 3
#define DB_NO_QUERY 4

typedef struct {
    uint32_t by;
    union {
        uint32_t id;
        char *title;
        char *fullpath;
    } value;
} SQLQuery;
#endif

#ifndef _SQL_LINKED_H
#define _SQL_LINKED_H
extern const unsigned char _sql_one_init[];
extern unsigned int _sql_one_init_len;
#endif

#ifndef _SQL_BUILDER_H
#define _SQL_BUILDER_H

#define SQL_INSERT(tablename, rows, values)                                    \
    "INSERT INTO " tablename " (" rows ") VALUES (" values "); "

#define SQL_SELECT(tablename, rows, mod, query)                                \
    "SELECT " rows " FROM " tablename " " mod " " query ";"

#define SQL_DELETE(tablename, mod, query)                                      \
    "DELETE FROM " tablename " " mod " " query "; "

#endif

#ifndef _DB_SQL_H_WHERES
#define _DB_SQL_H_WHERES

const static char *DB_SQL_WHERE_ID = "WHERE id = ?";
const static char *DB_SQL_WHERE_FULLPATH = "WHERE fullpath = ?";
const static char *DB_SQL_WHERE_TITLE = "WHERE title = ?";
const static char *DB_SQL_WHERE_TITLE_MATCH = "WHERE title MATCH ?";

#endif

#ifndef _DB_SQL_H_ALBUM
#define _DB_SQL_H_ALBUM

const static char *DB_SQL_ALBUM_INSERT =
    "INSERT INTO Album (title, artist, year) "
    "values (?,?,?);";

#endif

#ifndef _DB_SQL_H_MUSIC
#define _DB_SQL_H_MUSIC

#endif

#ifndef _DB_SQL_H_METADATA
#define _DB_SQL_H_METADATA

#endif

#ifndef _DB_SQL_H_SOURCES
#define _DB_SQL_H_SOURCES

#define MEDIASOURCES_TABLE "MediaSource"

static const char *SQL_INSERT_TO_AUDIOSOURCE =
    SQL_INSERT(MEDIASOURCES_TABLE, "path", "?");

static const char *SQL_SELECT_ALL_AUDIOSOURCE =
    SQL_SELECT(MEDIASOURCES_TABLE, "id, path", "", "");

static const char *SQL_GET_ONE_AUDIOSOURCE =
    SQL_SELECT(MEDIASOURCES_TABLE, "id, path", "where", "id = ?");

static const char *SQL_REMOVE_AUDIOSOURCE =
    SQL_DELETE(MEDIASOURCES_TABLE, "where", "id = ?");

#endif
