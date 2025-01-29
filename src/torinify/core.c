#include <libavutil/samplefmt.h>
#include <stdio.h>
#include <stdlib.h>
#include <torinify/core.h>
#include <torinify/playback.h>

/// Torinify Global Context
TorinifyContext *tgc = NULL;

void tf_sqlite3_init(char *filename) {
    sqlite3 *db;
    sqlite3_open(filename, &db);

    tgc->sqlite3 = db;
}

/// Initiates Torinify Global Context (`tgc`)
int tf_init() {
    int ret = 0;
    tgc = malloc(sizeof(TorinifyContext));

    if (!tgc)
        return -1;

    tgc->playback = NULL;
    tgc->sqlite3 = NULL;

    // if ((ret = pb_init(&tgc->playback)) < 0)
    //     return ret;

    return 0;
}

/// Cleans Torinify Global Context (`tgc`)
void tf_cleanup() {
    if (!tgc)
        return;

    if (tgc->sqlite3)
        sqlite3_close(tgc->sqlite3);

    if (tgc->playback)
        pb_free(tgc->playback);

    free(tgc);
}
