#ifndef _DB_TABLES_REPR_H
#define _DB_TABLES_REPR_H
#include <db/tables.h>
#include <stdlib.h>

typedef struct {
    int id;
    char *title;
    char *language;
} MusicAltTitle;

typedef struct {
    MusicAltTitle *titles;
    size_t size;
} MusicAltTitles;

typedef struct {
    int id;
    char *title;
    char *fullpath;
    MusicAltTitles *altTitles;
    MediaSourceRow *source;
    AlbumRow *album;
    MetadataRow *metadata;
} MusicRepr;

#endif
