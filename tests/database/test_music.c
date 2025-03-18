#include "errors/errors.h"
#include "storage/music.h"
#include <sqlite3.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

int _verify_music_exists(sqlite3 *db, int id, char **log) {
    sqlite3_stmt *stmt;
    int ret = sqlite3_prepare_v2(
        db, "SELECT id,title,fullpath FROM Music WHERE id = ?;", -1, &stmt,
        NULL);
    if (ret != SQLITE_OK) {
        error_log("Unable to prepare, cause \"%s\"", sqlite3_errmsg(db));
        ret = 1;
        goto clean;
    }

    ret = sqlite3_bind_int(stmt, 1, id);
    if (ret != SQLITE_OK) {
        error_log("Unable to bind id, cause \"%s\"", sqlite3_errmsg(db));
        ret = 1;
        goto clean;
    }

    ret = sqlite3_step(stmt);
    if (ret == SQLITE_ROW) {
        ret = 0;
    } else if (ret == SQLITE_DONE) {
        *log = "Music was not found";
        ret = -1;
    } else {
        *log = "sqlite step return unexpected value";
        ret = -2;
    }

clean:
    sqlite3_finalize(stmt);

    return ret;
}

void test_music_get_preadd(sqlite3 *db, bool *passed, char **name, char **log) {
    *name = "Music get (doesn't exist case)";

    Music *music = NULL;
    int ret = s_music_get(db, 1, &music);
    if (ret == TDB_NOT_FOUND) {
        *passed = true;
        return;
    } else {
        *passed = false;
        *log = "Unexpected return value from s_music_get";
    }
}

void test_music_add(sqlite3 *db, bool *passed, char **name, char **log) {
    *name = "Music Add";

    Music music = {.id = -1, "Song id 1", "/path/to/song"};
    int ret = s_music_add(db, &music);

    Music music2 = {.id = -1, "こんにちは世界", "への道/歌"};
    int ret2 = s_music_add(db, &music2);

    if (ret != TDB_SUCCESS || ret2 != TDB_SUCCESS) {
        *passed = false;
        *log = "s_music_add failed to return TDB_SUCCESS";
        return;
    }

    if (music.id == -1 || music2.id == -1) {
        *passed = false;
        *log = "music->id was not set to id in database";
        return;
    }

    char *verify_log = NULL;
    if (_verify_music_exists(db, music.id, &verify_log) != 0 ||
        _verify_music_exists(db, music2.id, &verify_log) != 0) {
        *passed = false;
        if (verify_log != NULL)
            *log = verify_log;
        else
            *log = "unable to verify if music exists";

        return;
    }

    *passed = true;
}

void test_music_get(sqlite3 *db, bool *passed, char **name, char **log) {
    *name = "Music get";

    Music *music = NULL;
    Music *music2 = NULL;

    int ret = s_music_get(db, 1, &music);
    int ret2 = s_music_get(db, 2, &music2);

    if (ret != TDB_SUCCESS && ret2 != TDB_SUCCESS) {
        *log = "s_music_add failed to return TDB_SUCCESS";
        return;
    }

    if (music == NULL || music2 == NULL) {
        *log = "music is null";
        return;
    }

    if (!(strcmp(music->title, "Song id 1") == 0 &&
          strcmp(music->fullpath, "/path/to/song") == 0)) {
        *log = "Music fields did not match expected values";
        goto clean;
    }

    if (!(strcmp(music2->title, "こんにちは世界") == 0 &&
          strcmp(music2->fullpath, "への道/歌") == 0)) {
        *log = "Music fields did not match expected values";
        goto clean;
    }

clean:
    s_music_free(music);

    *passed = true;
}

void test_music_get_metadata(sqlite3 *db, bool *passed, char **name,
                             char **log) {
    *name = "Music get metadata";

    Metadata *metadata;
    if (s_music_get_metadata(db, 1, &metadata) != TDB_SUCCESS) {
        *log = "s_music_get_metadata did not return TDB_SUCCESS";
        goto clean;
    }

    if (metadata->id != 1) {
        *log = "Metadata id was not as expected";
        goto clean;
    }

    if (strcmp(metadata->year, "0000-00-00") != 0) {
        *log = "Metadata year was not as expected";
        goto clean;
    }

    *passed = true;
clean:
    s_metadata_free(metadata);
}

void test_music_get_by_title(sqlite3 *db, bool *passed, char **name,
                             char **log) {
    *name = "Music get by title";

    Vec *musics;
    s_music_get_by_title(db, "Song id 1", &musics);

    if (musics->length != 1) {
        *log = "Unable to find expected song by title";
        goto clean;
    }

    Music *music = vec_get_ref(musics, 0);

    if (music->id != 1) {
        *log = "Music found did not match expected id";
        goto clean;
    }

    *passed = true;

clean:
    s_music_vec_free(musics);
}

void test_music_get_all(sqlite3 *db, bool *passed, char **name, char **log) {
    *name = "Music get all";
    Vec *musics = NULL;
    if (s_music_get_all(db, &musics) != TDB_SUCCESS) {
        *passed = false;
        *log = "s_music_get_all failed to return TDB_SUCCESS";
    }

    if (musics->length != 2) {
        *passed = false;
        *log = "Expected to get 2 musics";
        s_music_vec_free(musics);
        return;
    }

    s_music_vec_free(musics);

    *passed = true;
}

void test_music_update(sqlite3 *db, bool *passed, char **name, char **log) {
    *name = "Music update fields";

    if (s_music_update_title(db, 1, "Updated Title") != TDB_SUCCESS ||
        s_music_update_fullpath(db, 1, "/new/path") != TDB_SUCCESS) {
        *passed = false;
        *log = "update did not return TDB_SUCCESS";
        return;
    }

    Music *music;

    if (s_music_get(db, 1, &music) != TDB_SUCCESS) {
        *passed = false;
        *log = "s_music_get did not return TDB_SUCCESS";
        goto clean;
    }

    if (strcmp(music->title, "Updated Title") != 0) {
        *passed = false;
        *log = "Music->title did not match expected updated value";
        goto clean;
    }

    if (strcmp(music->fullpath, "/new/path") != 0) {
        *passed = false;
        *log = "Music->fullpath did not match expected updated value";
        goto clean;
    }

    *passed = true;

clean:
    s_music_free(music);
}

void test_music_delete(sqlite3 *db, bool *passed, char **name, char **log) {
    *name = "Music delete";

    if (s_music_delete(db, 1) != TDB_SUCCESS ||
        s_music_delete(db, 2) != TDB_SUCCESS) {
        *passed = false;
        *log = "s_music_delete did not return TDB_SUCCESS";
        return;
    }

    Music *music;
    if (s_music_get(db, 1, &music) == TDB_NOT_FOUND) {
        *passed = true;
    } else {
        *passed = false;
        *log = "s_music_get did not return expected TDB_NOT_FOUND";
    }

    s_music_free(music);
}
