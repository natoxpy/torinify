#include "storage/metadata.h"
#include "db/helpers.h"
#include "db/sql_macros.h"

Metadata *s_metadata_alloc() {
    Metadata *metadata = malloc(sizeof(Metadata));
    if (metadata == NULL)
        return NULL;

    metadata->id = -1;
    metadata->year = NULL;

    return metadata;
}

void s_metadata_free(Metadata *metadata) {
    if (metadata == NULL)
        return;

    if (metadata->year)
        free(metadata->year);

    free(metadata);
}

TDB_CODE s_metadata_add(sqlite3 *db, Metadata *metadata) {
    SQL_GENERIC_ADD_W_LAST_INSERT(SQL_INSERT(METADATA_TABLE, "year", "?"),
                                  SQL_BINDS(BIND_STR(metadata->year)),
                                  &metadata->id);
}

void *s_metadata_collect(sqlite3_stmt *stmt) {
    Metadata *metadata = s_metadata_alloc();

    metadata->id = dbh_get_column_int(stmt, 0);
    metadata->year = dbh_get_column_text(stmt, 1);

    return metadata;
}
