#include "db/exec/metadata_table.h"
#include "db/helpers.h"
#include "errors/errors.h"
#include <stdio.h>
#include <stdlib.h>

TDB_CODE DB_insert_metadata_row(sqlite3 *db, char *artist, int year,
                                char *genre, int *out_row_id) {
    int ret;
    sqlite3_stmt *stmt = NULL;

    if ((ret = dbh_prepare(db, DB_SQL_METADATA_INSERT, &stmt)) != TDB_SUCCESS)
        goto clean;

    BindValue binds[] = {BIND_STR(artist), BIND_INT(year), BIND_STR(genre)};
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
