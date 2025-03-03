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
    pbc->queues = vec_init(sizeof(Queue *));
    pbc->active_queue = UINT32_MAX;
    *playback_ctx = pbc;

    return T_SUCCESS;
}

void pb_free(PlaybackContext *pbc) {
    if (pbc == NULL)
        return;

    for (int i = 0; i < pbc->queues->length; i++) {
        Queue *q = vec_get_ref(pbc->queues, i);
        pb_q_free(q);
    }

    vec_free(pbc->queues);

    free(pbc);
}

Queue *pb_q_alloc() {
    Queue *q = malloc(sizeof(Queue));
    if (q == NULL)
        return NULL;

    q->active = UINT32_MAX;
    q->feed = NULL;
    q->songs = vec_init(sizeof(MusicQueue *));

    return q;
}

void pb_q_free(Queue *q) {
    if (q == NULL)
        return;

    for (int i = 0; i < q->songs->length; i++) {
        MusicQueue *musicq = vec_get_ref(q->songs, i);
        free(musicq);
    }

    vec_free(q->songs);
    free(q);
}

void pb_q_all(Queue *q, Vec **ms) { *ms = q->songs; }
void pb_q_get(Queue *q, uint32_t index, MusicQueue **m) {
    *m = vec_get_ref(q->songs, index);
}
void pb_q_add(Queue *q, MusicQueue *m) { vec_push(q->songs, &m); }
void pb_q_remove(Queue *q, uint32_t index);
void pb_q_clean(Queue *q, MusicQueue *m);

void pb_all_q(PlaybackContext *pbc, Vec *qs);
void pb_get_q(PlaybackContext *pbc, uint32_t index);
void pb_add_q(PlaybackContext *pbc, Queue *q) { vec_push(pbc->queues, &q); };
void pb_remove_q(PlaybackContext *pbc, Queue *q);
void pb_clean_q(PlaybackContext *pbc);
