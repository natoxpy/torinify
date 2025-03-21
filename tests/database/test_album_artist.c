#include "storage/album.h"
#include "storage/artist.h"
#include <sqlite3.h>
#include <stdbool.h>

void test_album_artist_add(sqlite3 *db, bool *passed, char **name, char **log) {
    *name = "Album <-> Music Add";

    int ret = s_album_add_artist(db, 1, 1, ARTIST_TYPE_ARTIST);
    if (ret != TDB_SUCCESS) {
        *log = "s_album_add_artist did not return TDB_SUCCESS";
        return;
    }

    *passed = true;
}
