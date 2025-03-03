#include "storage/music.h"
#include "db/exec/music_table.h"
#include "db/sql.h"
#include "db/tables.h"
#include "errors/errors.h"
#include "storage/album.h"
#include "utils/generic_vec.h"
#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>

Music *s_music_alloc() {
    Music *music = malloc(sizeof(Music));

    if (music == NULL) {
        error_log("Could not allocate enough memory for Music");
        return NULL;
    }

    music->id = 0;
    music->path = "";
    music->title = "";
    music->album = NULL;

    return music;
}

void s_music_free(Music *music) {
    if (music == NULL)
        return;

    if (music->album)
        s_album_free(music->album);

    if (music->path)
        free(music->path);

    if (music->title)
        free(music->title);

    free(music);
}

T_CODE s_music_get(sqlite3 *db, Music **music_out, MusicQuery query) {
    MusicRow *music_row;

    if (query.by == S_MUSIC_QUERY_BY_ID)
        DB_query_music_single(db, query.value.id, &music_row);
    // else if (query.by == S_MUSIC_QUERY_BY_TITLE)
    //     DB_query_music_single(db, &music_row,
    //                           (SQLQuery){.by = DB_QUERY_BY_FTS5_TITLE,
    //                                      {.title = query.value.title}});
    else
        return T_FAIL;

    if (music_row == NULL)
        goto end;

    Music *music = s_music_alloc();

    music->id = music_row->id;
    music->path = music_row->fullpath;
    music->title = music_row->title;

    music_row->fullpath = NULL;
    music_row->title = NULL;
    dbt_music_row_free(music_row);

    *music_out = music;

end:
    return T_SUCCESS;
}

T_CODE s_music_get_all(sqlite3 *db, Vec **out_musics) {
    int ret = T_SUCCESS;
    Vec *musics = vec_init(sizeof(Music *));

    Vec *music_rows = NULL;
    if (DB_query_music_all(db, &music_rows) != TDB_SUCCESS) {
        ret = T_FAIL;
        goto end;
    }

    if (music_rows == NULL)
        goto end;

    for (int i = 0; i < music_rows->length; i++) {
        MusicRow *row = vec_get_ref(music_rows, i);
        Music *music = s_music_alloc();

        music->id = row->id;
        music->title = row->title;
        music->path = row->fullpath;

        vec_push(musics, &music);

        music->title = NULL;
        music->path = NULL;
        row->title = NULL;
        row->fullpath = NULL;

        s_music_free(music);
    }

    *out_musics = musics;
end:
    vec_free(music_rows);
    return ret;
}

void s_vec_music_free(Vec *musics) {
    if (musics == NULL)
        return;

    if (musics->data) {
        for (int i = 0; i < musics->length; i++) {
            Music *music = vec_get(musics, i);
            free(music->path);
            free(music->title);
        }
    }

    vec_free(musics);
}
