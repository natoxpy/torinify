#include "errors/errors.h"
#include "storage/artist.h"
#include <sqlite3.h>
#include <stdbool.h>
#include <string.h>

int _verify_artist_exists(sqlite3 *db, int id, char **log) {
    sqlite3_stmt *stmt;
    int ret = sqlite3_prepare_v2(db, "SELECT id,name FROM Artist WHERE id = ?;",
                                 -1, &stmt, NULL);
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
        *log = "Artist was not found";
        ret = -1;
    } else {
        *log = "sqlite step return unexpected value";
        ret = -2;
    }

clean:
    sqlite3_finalize(stmt);

    return ret;
}

void test_artist_get_1(sqlite3 *db, bool *passed, char **name, char **log) {
    *name = "Artist get (doesn't exist case)";

    Artist *artist = NULL;
    int ret = s_artist_get(db, 1, &artist);
    if (ret == TDB_NOT_FOUND) {
        *passed = true;
        return;
    } else {
        *passed = false;
        *log = "Unexpected return value from s_artist_get";
    }
}

void test_artist_add(sqlite3 *db, bool *passed, char **name, char **log) {
    *name = "Artist Add";

    Artist artist = {.id = -1, "Artist id 1"};
    int ret = s_artist_add(db, &artist);

    Artist artist2 = {.id = -1, "Artist id 2"};
    int ret2 = s_artist_add(db, &artist2);

    if (ret != TDB_SUCCESS || ret2 != TDB_SUCCESS) {
        *passed = false;
        *log = "s_artist_add failed to return TDB_SUCCESS";
        return;
    }

    if (artist.id == -1 || artist2.id == -1) {
        *passed = false;
        *log = "artist->id was not set to id in database";
        return;
    }

    char *verify_log = NULL;
    if (_verify_artist_exists(db, artist.id, &verify_log) != 0 ||
        _verify_artist_exists(db, artist2.id, &verify_log) != 0) {
        *passed = false;
        if (verify_log != NULL)
            *log = verify_log;
        else
            *log = "unable to verify if artist exists";

        return;
    }

    *passed = true;
}

void test_artist_get_2(sqlite3 *db, bool *passed, char **name, char **log) {
    *name = "Artist get";

    Artist *artist = NULL;
    int ret = s_artist_get(db, 1, &artist);
    if (ret != TDB_SUCCESS) {
        *passed = false;
        *log = "s_artist_add failed to return TDB_SUCCESS";
        return;
    }

    if (artist == NULL) {
        *passed = false;
        *log = "artist is null";
        return;
    }

    if (strcmp(artist->name, "Artist id 1") != 0) {
        *log = "artist->name did not match expected values";
        *passed = false;

        s_artist_free(artist);
        return;
    }

    s_artist_free(artist);

    *passed = true;
}

void test_artist_get_3(sqlite3 *db, bool *passed, char **name, char **log) {
    *name = "Artist get all";
    Vec *artists = NULL;
    if (s_artist_get_all(db, &artists) != TDB_SUCCESS) {
        *passed = false;
        *log = "s_artist_get_all failed to return TDB_SUCCESS";
    }

    if (artists->length != 2) {
        *passed = false;
        *log = "Expected to get 2 artist";
        s_artist_vec_free(artists);
        return;
    }

    s_artist_vec_free(artists);

    *passed = true;
}

void test_artist_update(sqlite3 *db, bool *passed, char **name, char **log) {
    *name = "Artist update fields";

    if (s_artist_update_name(db, 1, "Updated Name")) {
        *passed = false;
        *log = "s_artist_update_name did not return TDB_SUCCESS";
        return;
    }

    Artist *artist;

    if (s_artist_get(db, 1, &artist) != TDB_SUCCESS) {
        *passed = false;
        *log = "s_artist_get did not return TDB_SUCCESS";
        goto clean;
    }

    if (strcmp(artist->name, "Updated Name") != 0) {
        *passed = false;
        *log = "Artist->name did not match expected updated value";
        goto clean;
    }

    *passed = true;

clean:
    s_artist_free(artist);
}

void test_artist_delete(sqlite3 *db, bool *passed, char **name, char **log) {
    *name = "Artist delete";

    if (s_artist_delete(db, 1) != TDB_SUCCESS ||
        s_artist_delete(db, 2) != TDB_SUCCESS) {
        *passed = false;
        *log = "s_artist_delete did not return TDB_SUCCESS";
        return;
    }

    Artist *artist;
    if (s_artist_get(db, 1, &artist) == TDB_NOT_FOUND) {
        *passed = true;
    } else {
        *passed = false;
        *log = "s_artist_get did not return expected TDB_NOT_FOUND";
    }

    s_artist_free(artist);
}
