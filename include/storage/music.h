#ifndef _STORAGE_MUSIC_H
#define _STORAGE_MUSIC_H

#include "storage/album.h"
#include "storage/altname.h"
#include "storage/metadata.h"
#include "utils/generic_vec.h"
#include <sqlite3.h>

#define MUSIC_TABLE "Music"
#define MUSIC_ALTNAME_MTM_TABLE "MusicAltTitle"

typedef struct {
    int id;
    char *title;
    char *fullpath;
} Music;

Music *s_music_alloc();
void s_music_free(Music *music);
void s_music_vec_free(Vec *musics);

// 0 = id, 1 = title, 2 = fullpath
void *s_music_collect(sqlite3_stmt *stmt);

TDB_CODE s_music_add(sqlite3 *db, Music *music);
TDB_CODE s_music_get(sqlite3 *db, int music_id, Music **music);
TDB_CODE s_music_get_all(sqlite3 *db, Vec **musics);
TDB_CODE s_music_delete(sqlite3 *db, int music_id);

TDB_CODE s_music_update_title(sqlite3 *db, int music_id, char *title);
TDB_CODE s_music_update_fullpath(sqlite3 *db, int music_id, char *fullpath);
TDB_CODE s_music_update_source(sqlite3 *db, int music_id, int source_id);

TDB_CODE s_music_get_metadata(sqlite3 *db, int id, Metadata **metadata);
TDB_CODE s_music_get_album(sqlite3 *db, int id, Album **album);
TDB_CODE s_music_get_artists(sqlite3 *db, int id, Vec **artists);

TDB_CODE s_music_add_alt_name(sqlite3 *db, int music_id,
                              AlternativeName *altname);
TDB_CODE s_music_get_alt_name_by_language(sqlite3 *db, int music_id,
                                          char *language,
                                          AlternativeName **altname);
TDB_CODE s_music_get_all_alt_names(sqlite3 *db, int music_id, Vec **alt_names);
TDB_CODE s_music_delete_alt_name(sqlite3 *db, int music_id, int altname_id);

#endif
