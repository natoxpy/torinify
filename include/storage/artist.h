#ifndef _STORAGE_ARTIST_H
#define _STORAGE_ARTIST_H

#include "storage/altname.h"
#include "utils/generic_vec.h"
#include <sqlite3.h>
#define ARTIST_TABLE "Artist"
#define ARTIST_ALBUM_MTM_TABLE "AlbumArtists"
#define ARTIST_METADATA_MTM_TABLE "MetadataArtists"

typedef struct {
    int id;
    char *name;
} Artist;

Artist *s_artist_alloc();
void s_artist_free(Artist *artist);
void s_artist_vec_free(Vec *artists);

// 0 = id, 1 = title, 2 = year
void *s_artist_collect(sqlite3_stmt *stmt);

TDB_CODE s_artist_add(sqlite3 *db, Artist *artist);
TDB_CODE s_artist_get(sqlite3 *db, int artist_id, Artist **artist);
TDB_CODE s_artist_get_all(sqlite3 *db, Vec **artists);
TDB_CODE s_artist_delete(sqlite3 *db, int artist_id);

TDB_CODE s_artist_update_name(sqlite3 *db, int artist_id, char *name);

TDB_CODE s_artist_add_alt_name(sqlite3 *db, int artist_id,
                               AlternativeName *altname);
TDB_CODE s_artist_get_alt_name_by_language(sqlite3 *db, int artist_id,
                                           char *language,
                                           AlternativeName **altname);
TDB_CODE s_artist_get_all_alt_names(sqlite3 *db, int artist_id,
                                    Vec **alt_names);
TDB_CODE s_artist_delete_alt_name(sqlite3 *db, int artist_id, int altname_id);
#endif
