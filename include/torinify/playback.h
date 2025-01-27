#ifndef _TORINIFY_PLAYBACK_H
#define _TORINIFY_PLAYBACK_H

#include <stdint.h>
typedef struct PlaybackContext PlaybackContext;

/// Playback Context (`pbc`)
struct PlaybackContext {};

/// return less than zero indicate errors
int pb_init(PlaybackContext **pbc);

void pb_free(PlaybackContext *pbc);

/// return less than zero indicate errors
int pb_set_buffer(PlaybackContext *pbc, uint8_t *buf, int size);

/// return less than zero indicate errors
int pb_play_file(PlaybackContext *pbc, char *filename);

int pb_get_duration(PlaybackContext *pbc);

int pb_get_current_time(PlaybackContext *pbc);

/// return less than zero indicate errors
int pb_set_current_time(PlaybackContext *pbc, int milisecs);

int pb_get_paused(PlaybackContext *pbc);

/// return less than zero indicate errors
int pb_pause(PlaybackContext *pbc);
/// return less than zero indicate errors
int pb_play(PlaybackContext *pbc);

int pb_get_volume(PlaybackContext *pbc);
int pb_set_volume(PlaybackContext *pbc);

#endif
