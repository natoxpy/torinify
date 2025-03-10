
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

#define SQL_INSERT(table, rows, values)                                        \
    "INSERT INTO " table " ( " rows " ) VALUES (" values ");"

#define SQL_SELECT(table, rows, mod, query)                                    \
    "SELECT " rows " FROM " table " " mod " " query ""

#define SQL_DELETE(table, mod, query) "DELETE FROM " table " " mod " " query " "

#define SQL_UPDATE(table, setter, mod, query)                                  \
    "UPDATE " table " SET " setter " " mod " " query " ;"

#define SQL_INNER_SELECT(table, rows, mod, query)                              \
    "(" SQL_SELECT(table, rows, mod, query) ")"

#endif
