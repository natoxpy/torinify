
#ifndef _TORINIFY_CORE_H
#define _TORINIFY_CORE_H
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
void tf_sqlite3_init(char *filename);

// Music
void tf_register(char *filename);
// END - MUSIC

int tf_init(void);
void tf_cleanup(void);

#endif // _TORINIFY_CORE_H
