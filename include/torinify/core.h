#ifndef _TORINIFY_CORE_H
#define _TORINIFY_CORE_H
#include "errors/errors.h"
#include <sqlite3.h>
#include <torinify/playback.h>

typedef struct TorinifyContext TorinifyContext;
struct TorinifyContext {
    PlaybackContext *playback;
    sqlite3 *sqlite3;
};

/// Torinigy Global Context
extern TorinifyContext *tgc;

/// This will be cleaned up with `tf_cleanup`
T_CODE tf_sqlite3_init(char *filename);

/// Will call `m_migrations` with `tgc->sqlite3`
T_CODE tf_sqlite3_migrations(char *dirpath);

T_CODE tf_init_db(char *filename, char *migrations_dir);

// Music
T_CODE tf_register_source(char *dirpath);
T_CODE tf_scan_sources();

// END - MUSIC

T_CODE tf_init(void);
void tf_cleanup(void);

#endif // _TORINIFY_CORE_H
