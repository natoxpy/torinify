#ifndef _STORAGE_ALBUM_H
#define _STORAGE_ALBUM_H

#include <stdint.h>
typedef struct {
    int id;
    char *title;
    char *artist;
    uint32_t year;
} Album;

typedef unsigned long AlbumID;

Album *s_album_alloc();
void s_album_free(Album *album);

#endif // _STORAGE_ALBUM_H
