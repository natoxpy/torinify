#ifndef _STORAGE_ALBUM_H
#define _STORAGE_ALBUM_H

#include "storage/altname.h"
#include "utils/generic_vec.h"
#include <sqlite3.h>
#include <stdint.h>

#define ALBUM_TABLE "Album"
#define ALBUM_ALTNAME_MTM_TABLE "AlbumAltTitle"
#define ALBUM_ARTIST_MTM_TABLE "AlbumArtists"
#define ALBUM_MUSIC_MTM_TABLE "AlbumMusics"
#define ALBUM_COLLECT_FIELDS "id,title,year"

typedef struct {
    int id;
    char *title;
    char *year;
} Album;

Album *s_album_alloc();
void s_album_free(Album *album);
void s_album_vec_free(Vec *musics);

// 0 = id, 1 = title, 2 = year
void *s_album_collect(sqlite3_stmt *stmt);

TDB_CODE s_album_add(sqlite3 *db, Album *album);
TDB_CODE s_album_get(sqlite3 *db, int album_id, Album **album);
TDB_CODE s_album_get_by_title(sqlite3 *db, char *title, Vec **albums);
TDB_CODE s_album_get_by_any_title(sqlite3 *db, char *any_title, Vec **albums);
TDB_CODE s_album_get_all(sqlite3 *db, Vec **albums);
TDB_CODE s_album_delete(sqlite3 *db, int album_id);

TDB_CODE s_album_update_title(sqlite3 *db, int album_id, char *title);
TDB_CODE s_album_update_year(sqlite3 *db, int album_id, char *year);

TDB_CODE s_album_add_music(sqlite3 *db, int album_id, int music_id);
TDB_CODE s_album_delete_music(sqlite3 *db, int album_id, int music_id);
TDB_CODE s_album_get_all_musics(sqlite3 *db, int album_id, Vec **musics);

TDB_CODE s_album_add_artist(sqlite3 *db, int album_id, int artist_id,
                            char *artist_type);
TDB_CODE s_album_get_all_artists(sqlite3 *db, int album_id, Vec **artists);
TDB_CODE s_album_delete_artist(sqlite3 *db, int album_id, int artist_id);

TDB_CODE s_album_add_title(sqlite3 *db, int album_id, AlternativeName *altname);
TDB_CODE s_album_get_by_language(sqlite3 *db, int album_id, char *language,
                                 AlternativeName **altname);
TDB_CODE s_album_get_all_titles(sqlite3 *db, int album_id, Vec **alt_names);
TDB_CODE s_album_delete_title(sqlite3 *db, int album_id, int altname_id);

#endif // _STORAGE_ALBUM_H
