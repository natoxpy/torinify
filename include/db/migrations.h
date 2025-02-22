#ifndef _DB_MIGRATION_H
#define _DB_MIGRATION_H
#include <errors/errors.h>
#include <sqlite3.h>

#define TABLE_MIGRATIONS_NAME ".migrations"

// return `0` on `success`
T_CODE m_migrations(sqlite3 *db);
#endif
