#include "storage/music.h"
#include "db/helpers.h"
#include "db/sql_macros.h"
#include "errors/errors.h"
#include <sqlite3.h>

Music *s_music_alloc() {
    Music *music = malloc(sizeof(Music));
    if (music == NULL)
        return music;

    music->id = -1;
    music->title = NULL;
    music->fullpath = NULL;

    return music;
}

void s_music_free(Music *music) {
    if (music == NULL)
        return;

    if (music->title)
        free(music->title);

    if (music->fullpath)
        free(music->fullpath);

    free(music);
}

void s_music_vec_free(Vec *musics) {
    if (musics == NULL)
        return;

    for (int i = 0; i < musics->length; i++) {
        s_music_free(vec_get_ref(musics, i));
    }

    vec_free(musics);
}

void *s_music_collect(sqlite3_stmt *stmt) {
    Music *row = s_music_alloc();

    row->id = dbh_get_column_int(stmt, 0);
    row->title = dbh_get_column_text(stmt, 1);
    row->fullpath = dbh_get_column_text(stmt, 2);

    return row;
}

TDB_CODE s_music_add(sqlite3 *db, Music *music) {
    Metadata mt = {.id = -1, .year = "0000-00-00"};
    int _ret = s_metadata_add(db, &mt);

    if (mt.id == -1 || _ret != TDB_SUCCESS) {
        error_log("Failed to add metadata row");
        return TDB_FAIL;
    }

    SQL_GENERIC_ADD_W_LAST_INSERT(
        SQL_INSERT(MUSIC_TABLE, "title,fullpath,metadata_id", "?,?,?"),
        SQL_BINDS(BIND_STR(music->title), BIND_STR(music->fullpath),
                  BIND_INT(mt.id)),
        &music->id);
}

TDB_CODE s_music_get(sqlite3 *db, int music_id, Music **music) {
    SQL_GENERIC_GET(
        SQL_SELECT(MUSIC_TABLE, "id,title,fullpath", "WHERE", "id = ?"),
        SQL_BINDS(BIND_INT(music_id)), music, s_music_collect);
}

TDB_CODE s_music_get_all(sqlite3 *db, Vec **musics){
    SQL_GENERIC_GET_ALL(SQL_SELECT(MUSIC_TABLE, "id,title,fullpath", "", ""),
                        musics, s_music_collect, Music)}

TDB_CODE s_music_delete(sqlite3 *db, int id){
    SQL_GENERIC_DELETE(SQL_DELETE(MUSIC_TABLE, "WHERE", "id = ?"),
                       SQL_BINDS(BIND_INT(id)))}

TDB_CODE s_music_update_title(sqlite3 *db, int id, char *title) {
    char *sql = SQL_UPDATE(MUSIC_TABLE, "title = ?", "WHERE", "id = ?");
    SQL_GENERIC_UPDATE(sql, SQL_BINDS(BIND_STR(title), BIND_INT(id)))
}

TDB_CODE s_music_update_fullpath(sqlite3 *db, int id, char *fullpath) {
    char *sql = SQL_UPDATE(MUSIC_TABLE, "fullpath = ?", "WHERE", "id = ?");
    SQL_GENERIC_UPDATE(sql, SQL_BINDS(BIND_STR(fullpath), BIND_INT(id)))
}

TDB_CODE s_music_update_source(sqlite3 *db, int music_id, int source_id) {
    char *sql = SQL_UPDATE(MUSIC_TABLE, "source_id = ?", "WHERE", "id = ?");
    SQL_GENERIC_UPDATE(sql, SQL_BINDS(BIND_INT(source_id), BIND_INT(music_id)))
}

// ======
// ALTERNATIVE_NAME RELATIONSHIP MANY TO MANY
// ======

TDB_CODE s_music_add_alt_name(sqlite3 *db, int music_id,
                              AlternativeName *altname) {
    int _ret;
    if ((_ret = s_altname_add(db, altname)) != TDB_SUCCESS)
        return _ret;

    char *sql = SQL_INSERT(MUSIC_ALTNAME_MTM_TABLE, "music_id,title_id", "?,?");

    SQL_GENERIC_ADD(sql, SQL_BINDS(BIND_INT(music_id), BIND_INT(altname->id)));
}

TDB_CODE s_music_get_alt_name_by_language(sqlite3 *db, int music_id,
                                          char *language,
                                          AlternativeName **altname) {
    char *s = SQL_SELECT(ALTERNATIVE_NAME_TABLE, "id,title,language",
                         "WHERE language = ? AND id in",
                         SQL_INNER_SELECT(MUSIC_ALTNAME_MTM_TABLE, "id",
                                          "WHERE", "music_id = ?"));

    SQL_GENERIC_GET(s, SQL_BINDS(BIND_STR(language), BIND_INT(music_id)),
                    altname, s_altname_collect)
}

TDB_CODE s_music_get_all_alt_names(sqlite3 *db, int music_id, Vec **altnames) {
    char *s =
        SQL_SELECT(ALTERNATIVE_NAME_TABLE, "id,title,language", "WHERE id IN",
                   SQL_INNER_SELECT(MUSIC_ALTNAME_MTM_TABLE, "title_id",
                                    "WHERE", "music_id = ?"));

    SQL_GENERIC_GET_ALL_WBINDS(s, SQL_BINDS(BIND_INT(music_id)), altnames,
                               s_altname_collect, AlternativeName)
}

TDB_CODE s_music_delete_alt_name(sqlite3 *db, int music_id, int altname_id) {
    char *s = SQL_DELETE(ALTERNATIVE_NAME_TABLE, "WHERE id = ? AND id IN",
                         SQL_INNER_SELECT(MUSIC_ALTNAME_MTM_TABLE, "id",
                                          "WHERE", "music_id = ?"));

    SQL_GENERIC_DELETE(s, SQL_BINDS(BIND_INT(altname_id), BIND_INT(music_id)));
}
