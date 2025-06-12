#include "storage/album.h"
#include "storage/artist.h"
#include <sqlite3.h>
#include <stdbool.h>
#include <string.h>

void test_album_artist_add(sqlite3 *db, bool *passed, char **name, char **log) {
    *name = "Album <-> artist Add";

    int ret = s_album_add_artist(db, 1, 1, ARTIST_TYPE_ARTIST);
    if (ret != TDB_SUCCESS) {
        *log = "s_album_add_artist did not return TDB_SUCCESS";
        return;
    }

    int ret2 = s_album_add_artist(db, 2, 1, ARTIST_TYPE_ARTIST);
    if (ret2 != TDB_SUCCESS) {
        *log = "s_album_add_artist did not return TDB_SUCCESS";
        return;
    }

    *passed = true;
}

void test_get_artist_albums(sqlite3 *db, bool *passed, char **name,
                            char **log) {
    *name = "Album <-> artist, get artist albums";

    Vec *albums = NULL;
    int ret = s_artist_get_all_albums(db, 1, &albums);

    if (ret != TDB_SUCCESS) {
        *log = "s_artist_get_all_albums did not return TDB_SUCCESS";
        return;
    }

    if (albums->length != 2) {
        *log = "s_artist_get_all_albums expected to return 2 in vector albums";
        goto cleanup;
    }

    Album *album = vec_get_ref(albums, 1);

    if (strcmp(album->title, "Album id 2") != 0) {
        *log = "s_artist_get_all_albums expected album 2 with a certain name, "
               "did not match";
        goto cleanup;
    }

    *passed = true;

cleanup:
    s_album_vec_free(albums);
}
