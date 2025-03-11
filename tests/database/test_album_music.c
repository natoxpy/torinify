#include "errors/errors.h"
#include "storage/music.h"
#include <sqlite3.h>
#include <stdbool.h>

void test_album_music_add(sqlite3 *db, bool *passed, char **name, char **log) {
    *name = "Album <-> Music Add";

    int ret = s_music_add_album(db, 1, 1);
    if (ret != TDB_SUCCESS) {
        *log = "s_music_add_album did not return TDB_SUCCESS";
        return;
    }

    *passed = true;
}
