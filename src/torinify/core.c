#include <libavutil/samplefmt.h>
#include <stdio.h>
#include <stdlib.h>
#include <torinify/core.h>
#include <torinify/playback.h>

TorinifyContext *tgc = NULL;

/// Initiates Torinify Global Context (`tgc`)
int torinify_init() {
    int ret = 0;
    tgc = malloc(sizeof(TorinifyContext));

    if (!tgc) {
        fprintf(stderr, "Memory allocation failed for Torinigy Global Context");
        return -1;
    }

    if ((ret = pb_init(&tgc->playback)) < 0)
        return ret;

    return 0;
}

/// Cleans Torinify Global Context (`tgc`)
void torinify_cleanup() {
    if (tgc == NULL)
        return;

    if (tgc->playback != NULL)
        pb_free(tgc->playback);

    free(tgc);
}

// Global playback Context
// PlaybackContext *g_pb_ctx;
