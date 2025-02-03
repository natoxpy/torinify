#ifndef _DB_SQL_H_WHERES
#define _DB_SQL_H_WHERES

const static char *DB_SQL_WHERE_ID = "WHERE id = ?";
const static char *DB_SQL_WHERE_FULLPATH = "WHERE fullpath = ?";

#endif

#ifndef _DB_SQL_H_ALBUM
#define _DB_SQL_H_ALBUM

const static char *DB_SQL_ALBUM_INSERT =
    "INSERT INTO Album (title, artist, year) "
    "values (?,?,?);";

#endif

#ifndef _DB_SQL_H_MUSIC
#define _DB_SQL_H_MUSIC

const static char *DB_SQL_MUSIC_INSERT =
    "INSERT INTO Music (title, metadata, fullpath, album, source) "
    "VALUES (?,?,?,?,?);";

const static char *DB_SQL_MUSIC_SELECT =
    "SELECT id, title, metadata, album, source, fullpath FROM Music %s;";

const static char *DB_SQL_MUSIC_SEARCH =
    "SELECT * FROM Music"
    "WHERE id IN (SELECT rowid FROM Music_FTS WHERE title MATCH '?')";

#endif

#ifndef _DB_SQL_H_METADATA
#define _DB_SQL_H_METADATA

const static char *DB_SQL_METADATA_INSERT =
    "INSERT INTO Metadata (artist, year, genre) "
    "VALUES (?,?,?);";

#endif
