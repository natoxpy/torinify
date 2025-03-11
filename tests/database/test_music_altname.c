#include "db/migrations.h"
#include "errors/errors.h"
#include "storage/altname.h"
#include "storage/music.h"
#include "utils/generic_vec.h"
#include <sqlite3.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void test_music_add_altname(sqlite3 *db, bool *passed, char **name,
                            char **log) {
    *name = "Music add altname";

    AlternativeName altname = {
        .id = -1, .title = "Translated name", .language = "eng"};
    AlternativeName altname2 = {
        .id = -1, .title = "Nombre Traducido", .language = "spa"};
    AlternativeName altname3 = {.id = -1, .title = "翻訳名", .language = "jpn"};
    AlternativeName altname4 = {
        .id = -1, .title = "Translated for ID 2", .language = "eng"};

    if (s_altname_add(db, &altname) != TDB_SUCCESS ||
        s_altname_add(db, &altname2) != TDB_SUCCESS ||
        s_altname_add(db, &altname3) != TDB_SUCCESS ||
        s_altname_add(db, &altname4) != TDB_SUCCESS) {
        *log = "s_altname_add did not return TDB_SUCCESS";
        return;
    }

    int ret = s_music_add_title(db, 1, altname.id);
    int ret2 = s_music_add_title(db, 1, altname2.id);
    int ret3 = s_music_add_title(db, 1, altname3.id);
    int ret4 = s_music_add_title(db, 2, altname4.id);

    if (ret != TDB_SUCCESS || ret2 != TDB_SUCCESS || ret3 != TDB_SUCCESS ||
        ret4 != TDB_SUCCESS) {
        *log = "s_music_add_alt_name did not return TDB_SUCCESS";
        return;
    }

    *passed = true;
}

void test_music_get_altname(sqlite3 *db, bool *passed, char **name,
                            char **log) {
    *name = "Music get alternative names";

    AlternativeName *altname;
    int ret = s_music_get_title_by_language(db, 1, "eng", &altname);
    AlternativeName *altname2;
    int ret2 = s_music_get_title_by_language(db, 1, "spa", &altname2);
    AlternativeName *altname3;
    int ret3 = s_music_get_title_by_language(db, 2, "eng", &altname3);

    if (ret == TDB_NOT_FOUND) {
        *log = "Altname not found";
        goto clean;
    }

    if (ret2 == TDB_NOT_FOUND) {
        *log = "Altname2 not found";
        goto clean;
    }

    if (ret3 == TDB_NOT_FOUND) {
        *log = "Altname3 not found";
        goto clean;
    }

    if (ret != TDB_SUCCESS || ret2 != TDB_SUCCESS || ret3 != TDB_SUCCESS) {
        *log = "s_music_get_alt_name_by_language failed to return TDB_SUCCESS";
        goto clean;
    }

    if (altname == NULL) {
        *log = "altname was not found";
        goto clean;
    }

    if (altname2 == NULL) {
        *log = "altname2 was not found";
        goto clean;
    }

    if (strcmp(altname->language, "eng") != 0 ||
        strcmp(altname->title, "Translated name") != 0) {
        *log = "Expected eng for id 1";
        goto clean;
    }

    if (strcmp(altname2->language, "spa") != 0 ||
        strcmp(altname2->title, "Nombre Traducido") != 0) {
        *log = "Expected spa for id 1";
        goto clean;
    }

    if (strcmp(altname3->language, "eng") != 0 ||
        strcmp(altname3->title, "Translated for ID 2") != 0) {
        *log = "Expected eng for id 2";
        goto clean;
    }

    *passed = true;

clean:
    s_altname_free(altname);
    s_altname_free(altname2);
    s_altname_free(altname3);
}

void test_music_get_all_altname(sqlite3 *db, bool *passed, char **name,
                                char **log) {
    *name = "Music get all alternative names";
    Vec *altnames;
    if (s_music_get_all_titles(db, 1, &altnames) != TDB_SUCCESS) {
        *passed = false;
        *log = "s_music_get_all_alt_names did not return TDB_SUCCESS";
        goto clean;
    }

    if (altnames->length != 3) {
        *passed = false;
        *log = "Expected 3 altername names";
        goto clean;
    }

    *passed = true;

clean:
    s_altname_vec_free(altnames);
}

void test_music_delete_altname(sqlite3 *db, bool *passed, char **name,
                               char **log) {
    *name = "Music delete alternative names";

    int ret = s_music_delete_title(db, 1, 1);
    int ret2 = s_music_delete_title(db, 1, 2);
    int ret3 = s_music_delete_title(db, 1, 3);

    if (ret != TDB_SUCCESS || ret2 != TDB_SUCCESS) {
        *passed = false;
        *log = "s_music_delete_alt_name did not return TDB_SUCCESS";
        return;
    }

    AlternativeName *altname;
    AlternativeName *altname2;
    AlternativeName *altname3;

    ret = s_music_get_title_by_language(db, 1, "end", &altname);
    ret2 = s_music_get_title_by_language(db, 1, "spa", &altname2);
    ret3 = s_music_get_title_by_language(db, 1, "jpn", &altname3);

    if (ret != TDB_NOT_FOUND || ret2 != TDB_NOT_FOUND ||
        ret3 != TDB_NOT_FOUND) {
        *passed = false;
        *log = "s_music_get_alt_name_by_language did not return TDB_NOT_FOUND";
        goto clean;
    }

    *passed = true;
clean:
    s_altname_free(altname);
    s_altname_free(altname2);
    s_altname_free(altname3);
}

void test_music_get_by_any_title(sqlite3 *db, bool *passed, char **name,
                                 char **log) {
    *name = "Music get by any title";
    int ret, ret2, ret3, ret4;

    Vec *musics, *musics2, *musics3, *musics4;
    ret = s_music_get_by_any_title(db, "Translated name", &musics);
    ret2 = s_music_get_by_any_title(db, "Translated for ID 2", &musics2);
    ret3 = s_music_get_by_any_title(db, "翻訳名", &musics3);
    ret4 = s_music_get_by_any_title(db, "Song id 2", &musics4);

    if (ret != TDB_SUCCESS || ret2 != TDB_SUCCESS || ret3 != TDB_SUCCESS ||
        ret4 != TDB_SUCCESS) {
        *log = "s_music_get_by_any_title did not return TDB_SUCCESS";
        goto clean;
    }

    if ((musics || musics2 || musics3 || musics4) &&
        (musics->length != 1 || musics2->length != 1 || musics3->length != 1 ||
         musics4->length != 1)) {
        *log = "Musics length did not match expected length";
        goto clean;
    }

    Music *music = vec_get_ref(musics, 0);
    Music *music2 = vec_get_ref(musics2, 0);
    Music *music3 = vec_get_ref(musics3, 0);
    Music *music4 = vec_get_ref(musics4, 0);

    if (music->id != 1 || music3->id != 1) {
        *log = "Music 1 or 3 did not match id";
        goto clean;
    }

    if (music2->id != 2 || music4->id != 2) {
        *log = "Music 2 or 4 did not match id";
        goto clean;
    }

    *passed = true;

clean:
    s_music_vec_free(musics);
    s_music_vec_free(musics2);
    s_music_vec_free(musics3);
    s_music_vec_free(musics4);
}
