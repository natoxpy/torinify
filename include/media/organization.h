#ifndef _MEDIA_SCAN_H
#define _MEDIA_SCAN_H

#include "errors/errors.h"
#include "storage/music.h"
#include "utils/generic_vec.h"
#include <sqlite3.h>

typedef struct {
    Music *original_ref;
    /// `Vec` which contains `char *`
    Vec *path_tree;
    char *filename;
} OrganizerFile;

typedef struct {
    char *origin;
    /// `Vec` which contains `Music` from "storage/music"
    Vec data;
} OrganizerContext;

T_CODE or_initiate(sqlite3 *db, int source_id, OrganizerContext **out_or_ctx);
T_CODE or_remap_by_structure(OrganizerContext *or_ctx, char *path_structure);
T_CODE or_apply(OrganizerContext *or_ctx);

#endif
