#include "storage/artist.h"
#include "db/exec.h"
#include "db/helpers.h"
#include "db/sql_macros.h"

Artist *s_artist_alloc() {
    Artist *artist = malloc(sizeof(Artist));
    if (artist == NULL)
        return NULL;

    artist->id = -1;
    artist->name = NULL;

    return artist;
}

void s_artist_free(Artist *artist) {
    if (artist == NULL)
        return;

    if (artist->name)
        free(artist->name);

    free(artist);
}

void s_artist_vec_free(Vec *artists) {
    if (artists == NULL)
        return;

    for (int i = 0; i < artists->length; i++) {
        s_artist_free(vec_get_ref(artists, i));
    }

    vec_free(artists);
}

void *s_artist_collect(sqlite3_stmt *stmt) {
    Artist *artist = s_artist_alloc();

    artist->id = dbh_get_column_int(stmt, 0);
    artist->name = dbh_get_column_text(stmt, 1);

    return artist;
}

TDB_CODE s_artist_add(sqlite3 *db, Artist *artist) {
    SQL_GENERIC_ADD_W_LAST_INSERT(SQL_INSERT(ARTIST_TABLE, "name", "?"),
                                  SQL_BINDS(BIND_STR(artist->name)),
                                  &artist->id);
}

TDB_CODE s_artist_get(sqlite3 *db, int artist_id, Artist **artist) {
    SQL_GENERIC_GET(SQL_SELECT(ARTIST_TABLE, "id,name", "WHERE", "id = ?"),
                    SQL_BINDS(BIND_INT(artist_id)), artist, s_artist_collect);
}

TDB_CODE s_artist_get_by_name(sqlite3 *db, char *name, Vec **artists) {

    SQL_GENERIC_GET_ALL_WBINDS(
        SQL_SELECT(ARTIST_TABLE, "id,name", "WHERE", "name = ?"),
        SQL_BINDS(BIND_STR(name)), artists, s_artist_collect, Artist);
}

TDB_CODE s_artist_get_all(sqlite3 *db, Vec **artists) {
    SQL_GENERIC_GET_ALL(SQL_SELECT(ARTIST_TABLE, "id,name", "", ""), artists,
                        s_artist_collect, Artist);
}

TDB_CODE s_artist_delete(sqlite3 *db, int artist_id) {
    SQL_GENERIC_DELETE(SQL_DELETE(ARTIST_TABLE, "WHERE", "id = ?"),
                       SQL_BINDS(BIND_INT(artist_id)));
}

TDB_CODE s_artist_update_name(sqlite3 *db, int artist_id, char *name) {
    char *sql = SQL_UPDATE(ARTIST_TABLE, "name = ?", "WHERE", "id = ?");
    SQL_GENERIC_UPDATE(sql, SQL_BINDS(BIND_STR(name), BIND_INT(artist_id)))
}
