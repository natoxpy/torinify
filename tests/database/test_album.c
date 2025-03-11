#include "errors/errors.h"
#include "storage/album.h"
#include <sqlite3.h>
#include <stdbool.h>
#include <string.h>

int _verify_album_exists(sqlite3 *db, int id, char **log) {
    sqlite3_stmt *stmt;
    int ret = sqlite3_prepare_v2(
        db, "SELECT id,title,year FROM Album WHERE id = ?;", -1, &stmt, NULL);
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
        *log = "Album was not found";
        ret = -1;
    } else {
        *log = "sqlite step return unexpected value";
        ret = -2;
    }

clean:
    sqlite3_finalize(stmt);

    return ret;
}

void test_album_get_preadd(sqlite3 *db, bool *passed, char **name, char **log) {
    *name = "Album get (doesn't exist case)";

    Album *album = NULL;
    int ret = s_album_get(db, 1, &album);
    if (ret == TDB_NOT_FOUND) {
        *passed = true;
        return;
    } else {
        *passed = false;
        *log = "Unexpected return value from s_album_get";
    }
}

void test_album_add(sqlite3 *db, bool *passed, char **name, char **log) {
    *name = "Album Add";

    Album album = {.id = -1, "Album id 1", "2024-00-00"};
    int ret = s_album_add(db, &album);

    Album album2 = {.id = -1, "Album id 2", "2025-00-00"};
    int ret2 = s_album_add(db, &album2);

    if (ret != TDB_SUCCESS || ret2 != TDB_SUCCESS) {
        *passed = false;
        *log = "s_album_add failed to return TDB_SUCCESS";
        return;
    }

    if (album.id == -1 || album2.id == -1) {
        *passed = false;
        *log = "album->id was not set to id in database";
        return;
    }

    char *verify_log = NULL;
    if (_verify_album_exists(db, album.id, &verify_log) != 0 ||
        _verify_album_exists(db, album2.id, &verify_log) != 0) {
        *passed = false;
        if (verify_log != NULL)
            *log = verify_log;
        else
            *log = "unable to verify if album exists";

        return;
    }

    *passed = true;
}

void test_album_get(sqlite3 *db, bool *passed, char **name, char **log) {
    *name = "Album get";

    Album *album = NULL;
    int ret = s_album_get(db, 1, &album);
    if (ret != TDB_SUCCESS) {
        *passed = false;
        *log = "s_album_add failed to return TDB_SUCCESS";
        return;
    }

    if (album == NULL) {
        *passed = false;
        *log = "album is null";
        return;
    }

    if (!(strcmp(album->title, "Album id 1") == 0 &&
          strcmp(album->year, "2024-00-00") == 0)) {
        *log = "Album fields did not match expected values";
        *passed = false;

        s_album_free(album);
        return;
    }

    s_album_free(album);

    *passed = true;
}

void test_album_get_all(sqlite3 *db, bool *passed, char **name, char **log) {
    *name = "Album get all";
    Vec *albums = NULL;
    if (s_album_get_all(db, &albums) != TDB_SUCCESS) {
        *passed = false;
        *log = "s_album_get_all failed to return TDB_SUCCESS";
    }

    if (albums->length != 2) {
        *passed = false;
        *log = "Expected to get 2 albums";
        s_album_vec_free(albums);
        return;
    }

    s_album_vec_free(albums);

    *passed = true;
}

void test_album_update(sqlite3 *db, bool *passed, char **name, char **log) {
    *name = "Album update fields";

    if (s_album_update_title(db, 1, "Updated Title") != TDB_SUCCESS ||
        s_album_update_year(db, 1, "2025-03-30") != TDB_SUCCESS) {
        *passed = false;
        *log = "update did not return TDB_SUCCESS";
        return;
    }

    Album *album;

    if (s_album_get(db, 1, &album) != TDB_SUCCESS) {
        *passed = false;
        *log = "s_album_get did not return TDB_SUCCESS";
        goto clean;
    }

    if (strcmp(album->title, "Updated Title") != 0) {
        *passed = false;
        *log = "Album->title did not match expected updated value";
        goto clean;
    }

    if (strcmp(album->year, "2025-03-30") != 0) {
        *passed = false;
        *log = "Album->year did not match expected updated value";
        goto clean;
    }

    *passed = true;

clean:
    s_album_free(album);
}

void test_album_delete(sqlite3 *db, bool *passed, char **name, char **log) {
    *name = "Album delete";

    if (s_album_delete(db, 1) != TDB_SUCCESS ||
        s_album_delete(db, 2) != TDB_SUCCESS) {
        *passed = false;
        *log = "s_album_delete did not return TDB_SUCCESS";
        return;
    }

    Album *album;
    if (s_album_get(db, 1, &album) == TDB_NOT_FOUND) {
        *passed = true;
    } else {
        *passed = false;
        *log = "s_album_get did not return expected TDB_NOT_FOUND";
    }

    s_album_free(album);
}
