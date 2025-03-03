#ifndef _DB_EXEC_METADATA_TABLE_H
#define _DB_EXEC_METADATA_TABLE_H

#include "db/sql.h"
#include "errors/errors.h"
#include <sqlite3.h>

#define METADATA_TABLE "Metadata"
#define METADATA_TABLE_COMMON_ROWS "artist, year, genre"
#define METADATA_TABLE_ROWS "id, artist, year, genre"

const static char *DB_SQL_METADATA_INSERT =
    SQL_INSERT(METADATA_TABLE, METADATA_TABLE_COMMON_ROWS, "?,?,?");

// const static char *DB_SQL_METADATA_INSERT =
//     "INSERT INTO Metadata (artist, year, genre) "
//     "VALUES (?,?,?);";

/*
 * `out_row_id` returns the row id of the inserted row
 */
TDB_CODE DB_insert_metadata_row(sqlite3 *db, char *artist, int year,
                                char *genre, int *out_row_id);

#endif
