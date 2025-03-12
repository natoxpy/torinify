#ifndef _STORAGE_METADATA_H
#define _STORAGE_METADATA_H

#include "errors/errors.h"
#include <sqlite3.h>

#define METADATA_TABLE "Metadata"
#define METADATA_COLLECT_FIELDS "id,year"

typedef struct {
    int id;
    // ISO8601 (year)-(mo)-(da)T
    char *year;
} Metadata;

Metadata *s_metadata_alloc();
void s_metadata_free(Metadata *metadata);

TDB_CODE s_metadata_add(sqlite3 *db, Metadata *metadata);

// 0 = id, 1 = year
void *s_metadata_collect(sqlite3_stmt *stmt);

#endif
