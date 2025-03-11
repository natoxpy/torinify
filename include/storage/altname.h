#ifndef _STORAGE_ALT_NAME_H
#define _STORAGE_ALT_NAME_H

#include "utils/generic_vec.h"
#include <sqlite3.h>

#define ALTERNATIVE_NAME_TABLE "AlternativeName"
#define ALTERNATIVE_NAME_COLLECT_FIELDS "id,title,language"

typedef struct {
    int id;
    char *title;
    char *language;
} AlternativeName;

AlternativeName *s_altname_alloc();
void s_altname_free(AlternativeName *music);
void s_altname_vec_free(Vec *musics);

// 0 = id, 1 = title, 2 = language
void *s_altname_collect(sqlite3_stmt *stmt);

TDB_CODE s_altname_add(sqlite3 *db, AlternativeName *altname);

#endif
