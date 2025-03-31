#ifndef _TORINIFY_PLAYBACK_H
#define _TORINIFY_PLAYBACK_H

#include <audio/audio.h>
#include <errors/errors.h>
#include <stdbool.h>
#include <stdint.h>
#include <threads.h>
#include <utils/generic_vec.h>
// typedef struct PlaybackContext PlaybackContext;

#define P_LOOP_NONE 0
#define P_LOOP_QUEUE 1
#define P_LOOP_SINGLE 2

/// Playback Context (`pbc`)
typedef struct {
    int active_queue;
    Vec *queues;
    thrd_t thread;
    bool thread_running;
} PlaybackContext;

typedef struct {
    int id;
    char *title;
    char *fullpath;
} MusicQueue;

/*
 * Vec<MusicQueue>
 */
typedef struct {
    int active;
    APlaybackFeed *feed;
    Vec *songs;
    float volume;
    int8_t loopstyle;
} Queue;

T_CODE pb_init(PlaybackContext **pbc);
void pb_free(PlaybackContext *pbc);

Queue *pb_q_alloc();
void pb_q_free(Queue *q);

MusicQueue *pb_musicq_alloc();
void pb_musicq_free(MusicQueue *mq);

// ===================
// ======= NEW =======
// ===================

Vec *pb_q_all(Queue *q);
MusicQueue *pb_q_get(Queue *q, int index);
MusicQueue *pb_q_get_active(Queue *q);
T_CODE pb_q_add(Queue *q, MusicQueue *m);
void pb_q_remove(Queue *q, int index);

T_CODE pb_q_set_active(Queue *q, int index);
void pb_q_set_volume(Queue *q, float volume);

/// wrapper over `a_set_current_time`
void pb_q_set_current_time(Queue *q, float seconds);

/// wrapper over `a_get_current_time`
float pb_q_get_current_time(Queue *q);

/// wrapper over `a_get_duration`
float pb_q_get_duration(Queue *q);

T_CODE pb_q_next(Queue *q);
T_CODE pb_q_previous(Queue *q);

bool pb_q_is_last(Queue *q);
bool pb_q_is_finished(Queue *q);
bool pb_q_is_paused(Queue *q);
bool pb_q_paused(Queue *q);
T_CODE pb_q_set_src(Queue *q, char *filename);
T_CODE pb_q_play(Queue *q);
T_CODE pb_q_pause(Queue *q);

// void pb_q_clean(Queue *q, MusicQueue *m);

void pb_all_q(PlaybackContext *pbc, Vec *qs);
void pb_get_q(PlaybackContext *pbc, int index);
void pb_add_q(PlaybackContext *pbc, Queue *q);

// void pb_remove_q(PlaybackContext *pbc, Queue *q);
// void pb_clean_q(PlaybackContext *pbc);

/// ===================
/// ======= OLD =======
/// ===================

/// return less than zero indicate errors
// T_CODE pb_q_set_src(Queue *q, char *filename);
// bool pb_q_paused(Queue *q);
// T_CODE pb_q_play(Queue *q);
// T_CODE pb_q_pause(Queue *q);

/// return less than zero indicate errors
// T_CODE pb_play_file(PlaybackContext *pbc, char *filename);
//
// long pb_get_duration(PlaybackContext *pbc);
//
// long pb_get_current_time(PlaybackContext *pbc);
//
// void pb_set_current_time(PlaybackContext *pbc, long milisecs);
//
// int pb_get_paused(PlaybackContext *pbc);
//
// void pb_pause(PlaybackContext *pbc);
// void pb_play(PlaybackContext *pbc);

// int pb_get_volume(PlaybackContext *pbc);
// void pb_set_volume(PlaybackContext *pbc);

#endif
