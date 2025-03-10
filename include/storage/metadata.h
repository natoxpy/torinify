#ifndef _STORAGE_METADATA_H
#define _STORAGE_METADATA_H

#include "errors/errors.h"
#include <sqlite3.h>

#define METADATA_TABLE "Metadata"

typedef struct {
    int id;
    char *year;
} Metadata;

TDB_CODE s_metadata_add(sqlite3 *db, Metadata *metadata);

#endif
