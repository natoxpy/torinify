#include "db/migrations.h"
#include "storage/altname.h"
#include "storage/music.h"
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

    int ret = s_music_add_alt_name(db, 1, &altname);

    AlternativeName altname2 = {
        .id = -1, .title = "Nombre Traducido", .language = "spa"};

    int ret2 = s_music_add_alt_name(db, 1, &altname2);

    if (ret != TDB_SUCCESS || ret2 != TDB_SUCCESS) {
        *passed = false;
        *log = "s_music_add_alt_name did not return TDB_SUCCESS";
        return;
    }

    *passed = true;
}

void test_music_get_altname(sqlite3 *db, bool *passed, char **name,
                            char **log) {
    *name = "Music get alternative names";

    AlternativeName *altname;
    int ret = s_music_get_alt_name_by_language(db, 1, "eng", &altname);
    AlternativeName *altname2;
    int ret2 = s_music_get_alt_name_by_language(db, 1, "spa", &altname2);

    if (ret == TDB_NOT_FOUND) {
        *passed = false;
        *log = "Altname not found";
        goto clean;
    }

    if (ret2 == TDB_NOT_FOUND) {
        *passed = false;
        *log = "Altname2 not found";
        goto clean;
    }

    if (ret != TDB_SUCCESS || ret2 != TDB_SUCCESS) {
        *passed = false;
        *log = "s_music_get_alt_name_by_language failed to return TDB_SUCCESS";
        goto clean;
    }

    if (altname == NULL) {
        *passed = false;
        *log = "altname was not found";
        goto clean;
    }

    if (altname2 == NULL) {
        *passed = false;
        *log = "altname2 was not found";
        goto clean;
    }

    if (strcmp(altname->language, "eng") != 0 ||
        strcmp(altname->title, "Translated name") != 0) {
        *passed = false;
        *log = "Expected eng";
        goto clean;
    }

    if (strcmp(altname2->language, "spa") != 0 ||
        strcmp(altname2->title, "Nombre Traducido") != 0) {
        *passed = false;
        *log = "Expected spa";
        goto clean;
    }

    *passed = true;

clean:
    s_altname_free(altname);
    s_altname_free(altname2);
}

void test_music_get_all_altname(sqlite3 *db, bool *passed, char **name,
                                char **log) {
    *name = "Music get all alternative names";
    Vec *altnames;
    if (s_music_get_all_alt_names(db, 1, &altnames) != TDB_SUCCESS) {
        *passed = false;
        *log = "s_music_get_all_alt_names did not return TDB_SUCCESS";
        goto clean;
    }

    if (altnames->length != 2) {
        *passed = false;
        *log = "Expected 2 altername names";
        goto clean;
    }

    *passed = true;

clean:
    s_altname_vec_free(altnames);
}

void test_music_delete_altname(sqlite3 *db, bool *passed, char **name,
                               char **log) {
    *name = "Music delete alternative names";

    int ret = s_music_delete_alt_name(db, 1, 1);
    int ret2 = s_music_delete_alt_name(db, 1, 2);

    if (ret != TDB_SUCCESS || ret2 != TDB_SUCCESS) {
        *passed = false;
        *log = "s_music_delete_alt_name did not return TDB_SUCCESS";
        return;
    }

    AlternativeName *altname;
    AlternativeName *altname2;

    ret = s_music_get_alt_name_by_language(db, 1, "end", &altname);
    ret2 = s_music_get_alt_name_by_language(db, 1, "spa", &altname2);

    if (ret != TDB_NOT_FOUND || ret2 != TDB_NOT_FOUND) {
        *passed = false;
        *log = "s_music_get_alt_name_by_language did not return TDB_NOT_FOUND";
        goto clean;
    }

    *passed = true;
clean:
    s_altname_free(altname);
    s_altname_free(altname2);
}
