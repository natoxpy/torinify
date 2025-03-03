#ifndef _DB_EXEC_H
#define _DB_EXEC_H

#include <db/sql.h>
#include <db/tables.h>
#include <errors/errors.h>
#include <sqlite3.h>

TDB_CODE DB_insert_source_row(sqlite3 *db, char *path);

TDB_CODE DB_query_source_all(sqlite3 *db, Vec **sources);

TDB_CODE DB_delete_source(sqlite3 *db, int id);

TDB_CODE DB_insert_album_row(sqlite3 *db, char *title, char *artist, int year);

#endif
