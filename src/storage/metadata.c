#include "storage/metadata.h"
#include "db/sql_macros.h"

TDB_CODE s_metadata_add(sqlite3 *db, Metadata *metadata) {
    SQL_GENERIC_ADD_W_LAST_INSERT(SQL_INSERT(METADATA_TABLE, "year", "?"),
                                  SQL_BINDS(BIND_STR(metadata->year)),
                                  &metadata->id);
}
