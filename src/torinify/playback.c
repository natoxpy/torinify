// Includes all methods relevant audio playback
// Abstract away specific implementation details here
// Often this will be used directly by torinify_music_player
// but it can be called externally directly as well

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <torinify/playback.h>

int pb_init(PlaybackContext **pbc) {
    *pbc = malloc(sizeof(PlaybackContext));

    if (!pbc) {
        fprintf(stderr, "Memory allocation failed for Playback Context");
        return -1;
    }

    return 0;
}

void pb_free(PlaybackContext *pbc) {
    if (pbc == NULL)
        return;

    free(pbc);
}

int pb_set_buffer(PlaybackContext *pbc, uint8_t *buf, int size) { return 0; }

int pb_play_file(PlaybackContext *pbc, char *filename) { return 0; }
