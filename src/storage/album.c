#include "db/helpers.h"
#include "db/sql_macros.h"
#include "storage/artist.h"
#include "storage/music.h"
#include <stdlib.h>
#include <storage/album.h>

Album *s_album_alloc() {
    Album *album = malloc(sizeof(Album));
    if (album == NULL)
        return NULL;

    album->id = 0;
    album->title = NULL;
    album->year = NULL;

    return album;
}

void s_album_free(Album *album) {
    if (album == NULL)
        return;

    if (album->title)
        free(album->title);

    if (album->year)
        free(album->year);

    free(album);
}

void s_album_vec_free(Vec *musics) {
    if (musics == NULL)
        return;

    for (int i = 0; i < musics->length; i++) {
        s_album_free(vec_get_ref(musics, i));
    }

    vec_free(musics);
}

void *s_album_collect(sqlite3_stmt *stmt) {
    Album *album = s_album_alloc();

    album->id = dbh_get_column_int(stmt, 0);
    album->title = dbh_get_column_text(stmt, 1);
    album->year = dbh_get_column_text(stmt, 2);

    return album;
}

TDB_CODE s_album_add(sqlite3 *db, Album *album) {
    SQL_GENERIC_ADD_W_LAST_INSERT(
        SQL_INSERT(ALBUM_TABLE, "title,year", "?,?"),
        SQL_BINDS(BIND_STR(album->title), BIND_STR(album->year)), &album->id);
}

TDB_CODE s_album_get(sqlite3 *db, int album_id, Album **album) {
    SQL_GENERIC_GET(
        SQL_SELECT(ALBUM_TABLE, ALBUM_COLLECT_FIELDS, "WHERE", "id = ?"),
        SQL_BINDS(BIND_INT(album_id)), album, s_album_collect);
}

TDB_CODE s_album_get_by_title(sqlite3 *db, char *title, Vec **albums) {
    SQL_GENERIC_GET_ALL_WBINDS(
        SQL_SELECT(ALBUM_TABLE, ALBUM_COLLECT_FIELDS, "WHERE", "title = ?"),
        SQL_BINDS(BIND_STR(title)), albums, s_album_collect, Album);
}

TDB_CODE s_album_get_by_any_title(sqlite3 *db, char *any_title, Vec **albums) {
    char *sql = SQL_SELECT(
        ALBUM_TABLE, ALBUM_COLLECT_FIELDS, "WHERE title = ? OR id IN ",
        SQL_INNER_SELECT(ALBUM_ALTNAME_MTM_TABLE, "album_id",
                         "WHERE title_id IN",
                         SQL_INNER_SELECT(ALTERNATIVE_NAME_TABLE, "id", "WHERE",
                                          "title = ?")));

    SQL_GENERIC_GET_ALL_WBINDS(
        sql, SQL_BINDS(BIND_STR(any_title), BIND_STR(any_title)), albums,
        s_album_collect, Album);
}

TDB_CODE s_album_get_all(sqlite3 *db, Vec **albums) {
    SQL_GENERIC_GET_ALL(SQL_SELECT(ALBUM_TABLE, ALBUM_COLLECT_FIELDS, "", ""),
                        albums, s_album_collect, Album);
}

TDB_CODE s_album_delete(sqlite3 *db, int album_id) {
    SQL_GENERIC_DELETE(SQL_DELETE(ALBUM_TABLE, "WHERE", "id = ?"),
                       SQL_BINDS(BIND_INT(album_id)));
}

TDB_CODE s_album_update_title(sqlite3 *db, int album_id, char *title) {
    char *sql = SQL_UPDATE(ALBUM_TABLE, "title = ?", "WHERE", "id = ?");
    SQL_GENERIC_UPDATE(sql, SQL_BINDS(BIND_STR(title), BIND_INT(album_id)))
}

TDB_CODE s_album_update_year(sqlite3 *db, int album_id, char *year) {
    char *sql = SQL_UPDATE(ALBUM_TABLE, "year = ?", "WHERE", "id = ?");
    SQL_GENERIC_UPDATE(sql, SQL_BINDS(BIND_STR(year), BIND_INT(album_id)))
}

// ======
// MUSIC RELATIONSHIP MANY TO MANY
TDB_CODE s_album_add_music(sqlite3 *db, int album_id, int music_id) {
    return s_music_add_album(db, music_id, album_id);
}

TDB_CODE s_album_delete_music(sqlite3 *db, int album_id, int music_id) {
    return s_music_delete_album(db, music_id, album_id);
}

TDB_CODE s_album_get_all_musics(sqlite3 *db, int album_id, Vec **musics) {
    char *s = SQL_SELECT(MUSIC_TABLE, MUSIC_COLLECT_FIELDS, "WHERE id IN",
                         SQL_INNER_SELECT(ALBUM_MUSIC_MTM_TABLE, "music_id",
                                          "WHERE", "album_id = ?"));

    SQL_GENERIC_GET_ALL_WBINDS(s, SQL_BINDS(BIND_INT(album_id)), musics,
                               s_music_collect, Music)
}

// ======
// ARTIST RELATIONSHIP MANY TO MANY
// ======

TDB_CODE s_album_add_artist(sqlite3 *db, int album_id, int artist_id,
                            char *artist_type) {
    char *sql = SQL_INSERT(ALBUM_ARTIST_MTM_TABLE,
                           "album_id,artist_id,artist_type", "?,?,?");
    SQL_GENERIC_ADD(sql, SQL_BINDS(BIND_INT(album_id), BIND_INT(artist_id),
                                   BIND_STR(artist_type)));
}

TDB_CODE s_album_get_all_artists(sqlite3 *db, int album_id, Vec **artists) {
    char *s = SQL_SELECT(ARTIST_TABLE, ARTIST_COLLECT_FIELDS, "WHERE id IN",
                         SQL_INNER_SELECT(ALBUM_ARTIST_MTM_TABLE, "artist_id",
                                          "WHERE", "album_id = ?"));

    SQL_GENERIC_GET_ALL_WBINDS(s, SQL_BINDS(BIND_INT(album_id)), artists,
                               s_artist_collect, Artist)
}

TDB_CODE s_album_delete_artist(sqlite3 *db, int album_id, int artist_id) {
    char *s = SQL_DELETE(ALBUM_ARTIST_MTM_TABLE, "WHERE",
                         "album_id = ? AND artist_id = ?");

    SQL_GENERIC_DELETE(s, SQL_BINDS(BIND_INT(album_id), BIND_INT(artist_id)));
}
