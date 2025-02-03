#ifndef _DB_TABLES_H
#define _DB_TABLES_H

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

#endif
