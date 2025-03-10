#ifndef _DB_EXEC_ALBUM_TABLE_H
#define _DB_EXEC_ALBUM_TABLE_H

const static char *DB_SQL_ALBUM_INSERT =
    "INSERT INTO Album (title, artist, year) "
    "values (?,?,?);";

#endif
