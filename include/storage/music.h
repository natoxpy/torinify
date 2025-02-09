#ifndef _STORAGE_MUSIC_H
#define _STORAGE_MUSIC_H

#include <errors/errors.h>
#include <sqlite3.h>
#include <stdint.h>
#include <storage/album.h>
#include <utils/generic_vec.h>

#define S_MUSIC_QUERY_BY_ID 0
#define S_MUSIC_QUERY_BY_TITLE 1

typedef struct {
    uint8_t by;
    union {
        uint32_t id;
        char *title;
    } value;
} MusicQuery;

typedef struct {
    int id;
    char *title;
    char *path;
    Album *album;
} Music;

Music *s_music_alloc();
void s_music_free(Music *music);

T_CODE s_music_get(sqlite3 *db, Music **music_out, MusicQuery query);
T_CODE s_music_get_many(sqlite3 *db, Vec **musics, MusicQuery query);
T_CODE s_music_get_all(sqlite3 *db, Vec **musics);
void s_vec_music_free(Vec *musics);

#endif // _STORAGE_MUSIC_H
