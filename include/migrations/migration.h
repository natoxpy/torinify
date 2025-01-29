#ifndef _MIGRATIONS_MIGRATION_H
#define _MIGRATIONS_MIGRATION_H
#include <sqlite3.h>

#define TABLE_MIGRATIONS_NAME ".migrations"

// return `0` on `success`
int m_migrations(sqlite3 *db);
#endif
