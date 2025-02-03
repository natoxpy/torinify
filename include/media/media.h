
#ifndef _MEDIA_MEDIA_H
#define _MEDIA_MEDIA_H

#include "errors/errors.h"
#include <sqlite3.h>
#include <stdlib.h>

typedef struct {
    char *source;
    int id;
} MediaSource;

typedef struct {
    MediaSource *sources;
    size_t size;
} MediaSources;

typedef struct {
} MediaAlbum;

typedef struct {
    char *title;
    MediaAlbum *album;
} MediaMusic;

/// Adds realpath of `dirname` to database
///
/// Returns `T_FAIL` on any error, outputs to `error_print_log()` for the fail
/// details, If no problems arise returns `T_SUCCESS`
T_CODE M_register_source(sqlite3 *db, char *dirname);
T_CODE M_get_sources(sqlite3 *db, MediaSources **media_srcs);
/// Returns `T_DB_NOT_FOUND` if the id didn't return any rows
T_CODE M_get_source(sqlite3 *db, MediaSource **media_src, int id);

void M_free_sources(MediaSources *media_srcs);
void M_free_source(MediaSource *media_src);

T_CODE M_remove_source(sqlite3 *db, int id);

/// Perform an algorithm that looks at all the sources inside the database,
/// reads all the metadata and adds it to the database, it goes to each file one
/// by one for every source registered by the database
T_CODE M_scan(sqlite3 *db);

T_CODE M_scan_dir(sqlite3 *db, char *dirpath, int source_id);

T_CODE M_scan_file(sqlite3 *db, char *filepath, int source);

int M_supported_music_file(char *fullpath);

#endif
