#include "db/helpers.h"
#include "db/sql_macros.h"
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
    SQL_GENERIC_GET(SQL_SELECT(ALBUM_TABLE, "id,title,year", "WHERE", "id = ?"),
                    SQL_BINDS(BIND_INT(album_id)), album, s_album_collect);
}

TDB_CODE s_album_get_all(sqlite3 *db, Vec **albums) {
    SQL_GENERIC_GET_ALL(SQL_SELECT(ALBUM_TABLE, "id,title,year", "", ""),
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
