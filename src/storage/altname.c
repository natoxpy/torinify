#include "storage/altname.h"
#include "db/helpers.h"
#include "db/sql_macros.h"
#include <stdlib.h>

AlternativeName *s_altname_alloc() {
    AlternativeName *altname = malloc(sizeof(AlternativeName));

    if (altname == NULL)
        return NULL;

    altname->id = -1;
    altname->title = NULL;
    altname->language = NULL;

    return altname;
}

void s_altname_free(AlternativeName *music) {
    if (music == NULL)
        return;

    if (music->title)
        free(music->title);
    if (music->language)
        free(music->language);

    free(music);
}
void s_altname_vec_free(Vec *altnames) {
    if (altnames == NULL)
        return;

    for (int i = 0; i < altnames->length; i++) {
        AlternativeName *altname = vec_get_ref(altnames, i);
        s_altname_free(altname);
    }

    vec_free(altnames);
}

// 0 = id, 1 = title, 2 = language
void *s_altname_collect(sqlite3_stmt *stmt) {
    AlternativeName *altname = s_altname_alloc();

    altname->id = dbh_get_column_int(stmt, 0);
    altname->title = dbh_get_column_text(stmt, 1);
    altname->language = dbh_get_column_text(stmt, 2);

    return altname;
}

TDB_CODE s_altname_add(sqlite3 *db, AlternativeName *altname) {
    SQL_GENERIC_ADD_W_LAST_INSERT(
        SQL_INSERT(ALTERNATIVE_NAME_TABLE, "title,language", "?,?"),
        SQL_BINDS(BIND_STR(altname->title), BIND_STR(altname->language)),
        &altname->id);
}
