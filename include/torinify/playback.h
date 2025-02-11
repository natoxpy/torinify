#ifndef _TORINIFY_PLAYBACK_H
#define _TORINIFY_PLAYBACK_H

#include <audio/audio.h>
#include <errors/errors.h>
#include <stdint.h>
typedef struct PlaybackContext PlaybackContext;

/// Playback Context (`pbc`)
struct PlaybackContext {
    APlaybackFeed *feed;
};

T_CODE pb_init(PlaybackContext **pbc);

void pb_free(PlaybackContext *pbc);

/// return less than zero indicate errors
T_CODE pb_set_buffer(PlaybackContext *pbc, uint8_t *buf, int size);

/// return less than zero indicate errors
T_CODE pb_play_file(PlaybackContext *pbc, char *filename);

long pb_get_duration(PlaybackContext *pbc);

long pb_get_current_time(PlaybackContext *pbc);

void pb_set_current_time(PlaybackContext *pbc, long milisecs);

int pb_get_paused(PlaybackContext *pbc);

void pb_pause(PlaybackContext *pbc);
void pb_play(PlaybackContext *pbc);

int pb_get_volume(PlaybackContext *pbc);
void pb_set_volume(PlaybackContext *pbc);

#endif
