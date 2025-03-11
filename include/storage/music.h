#ifndef _STORAGE_MUSIC_H
#define _STORAGE_MUSIC_H

#include "storage/altname.h"
#include "storage/metadata.h"
#include "utils/generic_vec.h"
#include <sqlite3.h>

#define MUSIC_TABLE "Music"
#define MUSIC_ALTNAME_MTM_TABLE "MusicAltTitle"
#define MUSIC_COLLECT_FIELDS "id,title,fullpath"

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
TDB_CODE s_music_get_by_title(sqlite3 *db, char *title, Vec **musics);
TDB_CODE s_music_get_by_any_title(sqlite3 *db, char *any_title, Vec **musics);
TDB_CODE s_music_get_all(sqlite3 *db, Vec **musics);
TDB_CODE s_music_delete(sqlite3 *db, int music_id);

TDB_CODE s_music_update_title(sqlite3 *db, int music_id, char *title);
TDB_CODE s_music_update_fullpath(sqlite3 *db, int music_id, char *fullpath);
TDB_CODE s_music_update_source(sqlite3 *db, int music_id, int source_id);

TDB_CODE s_music_get_metadata(sqlite3 *db, int music_id, Metadata **metadata);

TDB_CODE s_music_add_album(sqlite3 *db, int music_id, int album_id);
TDB_CODE s_music_get_all_albums(sqlite3 *db, int music_id, Vec **albums);
TDB_CODE s_music_delete_album(sqlite3 *db, int music_id, int album_id);

/// `prefered` should be ISO_639-2
TDB_CODE s_music_get_prefered_title(sqlite3 *db, Music *music, char *prefered,
                                    char **name, AlternativeName **altname);

TDB_CODE s_music_add_title(sqlite3 *db, int music_id, int altname_id);
TDB_CODE s_music_get_title_by_language(sqlite3 *db, int music_id,
                                       char *language,
                                       AlternativeName **altname);
TDB_CODE s_music_get_all_titles(sqlite3 *db, int music_id, Vec **alt_names);
TDB_CODE s_music_delete_title(sqlite3 *db, int music_id, int altname_id);

#endif
