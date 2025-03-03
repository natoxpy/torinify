#ifndef _DB_TABLES_H
#define _DB_TABLES_H

#include <errors/errors.h>
#include <utils/generic_vec.h>

typedef struct {
    int id;
    int migration_index;
} DotMigrationRow;

typedef struct {
    int id;
    char *path;
} MediaSourceRow;

typedef struct {
    int id;
    char *title;
    char *artist;
    int year;
} AlbumRow;

typedef struct {
    int id;
    char *artist;
    char *genre;
    int year;
} MetadataRow;

typedef struct {
    int id;
    char *title;
    char *fullpath;
    int source;
    int album;
    int metadata;
} MusicRow;

typedef struct {
    int id;
    char *title;
    char *language;
    int music_id;
} MusicAltTitleRow;

MusicRow *dbt_music_row_alloc();
MediaSourceRow *dbt_source_row_alloc();

void dbt_music_row_free(MusicRow *row);
void dbt_music_vec_rows_free(Vec *vec);

void dbt_source_row_free(MediaSourceRow *row);

///
/// Allocate Vec with \ref vec_init(uint32_t)
/// "vec_init(sizeof(MediaSourceRow*))."
void dbt_source_vec_rows_free(Vec *vec);

#endif
