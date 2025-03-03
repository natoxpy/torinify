#ifndef _DB_MIGRATION_H
#define _DB_MIGRATION_H
#include <errors/errors.h>
#include <sqlite3.h>

TDB_CODE migrate_database(sqlite3 *db);

#endif
