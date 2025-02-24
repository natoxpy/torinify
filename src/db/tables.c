#include <db/tables.h>
#include <errors/errors.h>
#include <stdlib.h>

MusicRow *dbt_music_row_alloc() {
    MusicRow *music_row = malloc(sizeof(MusicRow));

    if (!music_row) {
        error_log("Failed to allocate memory for music_row");
        return NULL;
    }

    music_row->id = -1;
    music_row->title = NULL;
    music_row->fullpath = NULL;
    music_row->source = -1;
    music_row->album = -1;
    music_row->metadata = -1;

    return music_row;
}

MediaSourceRow *dbt_source_row_alloc() {
    MediaSourceRow *row = malloc(sizeof(MediaSourceRow));
    if (!row) {
        error_log("Failed to allocate memory for MediaSourceRow");
        return NULL;
    }

    row->id = 0;
    row->path = NULL;

    return row;
}

void dbt_music_row_free(MusicRow *row) {
    if (!row)
        return;

    if (row->title)
        free(row->title);

    if (row->fullpath)
        free(row->fullpath);

    free(row);
}

void dbt_music_vec_rows_free(Vec *vec) {
    if (vec == NULL)
        return;

    MusicRow *row;

    for (int i = 0; i < vec->length; i++) {
        row = (MusicRow *)(vec->data + (i * vec->element_size));
        free(row->title);
        free(row->fullpath);
    }

    vec_free(vec);
}

void dbt_source_row_free(MediaSourceRow *row) {
    if (row == NULL)
        return;

    if (row->path)
        free(row->path);

    free(row);
}

void dbt_source_vec_rows_free(Vec *vec) {
    if (vec == NULL)
        return;

    for (int i = 0; i < vec->length; i++) {
        MediaSourceRow *row = vec_get_ref(vec, i);
        dbt_source_row_free(row);
    }

    vec_free(vec);
}
