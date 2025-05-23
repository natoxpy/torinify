#ifndef _TORINIFY_CORE_H
#define _TORINIFY_CORE_H
#include <errors/errors.h>
#include <sqlite3.h>
#include <threads.h>
#include <torinify/playback.h>
#include <torinify/scanner.h>
#include <utils/generic_vec.h>

typedef struct {
    ScannerContext *scanner;
    PlaybackContext *playback;
    sqlite3 *sqlite3;
} TorinifyContext;

/// Torinigy Global Context
extern TorinifyContext *tgc;

/// This will be cleaned up with `tf_cleanup`
T_CODE tf_sqlite3_init(char *filename);

/// Will call `m_migrations` with `tgc->sqlite3`
T_CODE tf_sqlite3_migrations();

T_CODE tf_init_db(char *filename);

// Playback
T_CODE tf_set_src(char *filename);
T_CODE tf_play();
T_CODE tf_pause();
int tf_get_paused();
void tf_set_current_time(long miliseconds);
long tf_get_current_time();

// Music
T_CODE tf_register_source(char *dirpath);
T_CODE tf_scan_sources();

/// results contain `Struct SearchResult`
void tf_search(char *query, double threshold, Vec **results);
/// search context is cache `tgc`

// END - MUSIC

T_CODE
tf_init(void);
void tf_cleanup(void);

#endif // _TORINIFY_CORE_H
