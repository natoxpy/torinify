#ifndef _DB_EXEC_MEDIASOURCE_TABLE_H
#define _DB_EXEC_MEDIASOURCE_TABLE_H

#include "db/sql.h"
#define MEDIASOURCES_TABLE "MediaSource"

static const char *SQL_INSERT_TO_AUDIOSOURCE =
    SQL_INSERT(MEDIASOURCES_TABLE, "path", "?");

static const char *SQL_SELECT_ALL_AUDIOSOURCE =
    SQL_SELECT(MEDIASOURCES_TABLE, "id, path", "", "");

static const char *SQL_GET_ONE_AUDIOSOURCE =
    SQL_SELECT(MEDIASOURCES_TABLE, "id, path", "where", "id = ?");

static const char *SQL_REMOVE_AUDIOSOURCE =
    SQL_DELETE(MEDIASOURCES_TABLE, "where", "id = ?");

#endif
