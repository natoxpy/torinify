// Includes all methods relevant audio playback
// Abstract away specific implementation details here
// Often this will be used directly by torinify_music_player
// but it can be called externally directly as well

#include <errors/errors.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <torinify/playback.h>

T_CODE pb_init(PlaybackContext **playback_ctx) {
    PlaybackContext *pbc = malloc(sizeof(PlaybackContext));

    if (!pbc) {
        error_log("Memory allocation failed for Playback Context");
        return T_FAIL;
    }

    pbc->feed = NULL;

    *playback_ctx = pbc;

    return T_SUCCESS;
}

void pb_free(PlaybackContext *pbc) {
    if (pbc == NULL)
        return;

    if (pbc->feed)
        a_playback_feed_free(pbc->feed);

    free(pbc);
}
