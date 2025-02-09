#include <stdlib.h>
#include <storage/album.h>

Album *s_album_alloc() {
    //
    return NULL;
}

void s_album_free(Album *album) {
    if (album == NULL)
        return;

    if (album->artist)
        free(album->artist);

    if (album->title)
        free(album->title);

    free(album);
}
