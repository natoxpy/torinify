// Includes all methods relevant audio playback
// Abstract away specific implementation details here
// Often this will be used directly by torinify_music_player
// but it can be called externally directly as well

#include "utils/file.h"
#include <errors/errors.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <threads.h>
#include <torinify/playback.h>

atomic_int playback_handle_active = 1;

void deactivate_playback_handle() { atomic_store(&playback_handle_active, 0); }

int thread_playback_handle(void *args) {
    PlaybackContext *ctx = args;
    Queue *q = NULL;

    while (atomic_load(&playback_handle_active)) {
        if (ctx->active_queue == -1 || ctx->active_queue >= ctx->queues->length)
            continue;

        q = vec_get_ref(ctx->queues, ctx->active_queue);

        thrd_sleep(&(struct timespec){.tv_nsec = 100000000}, NULL);

        if (q->feed) {
            long duration =
                q->feed->data->length / (sizeof(float) * q->feed->channels);
            long current_time = q->feed->samples_played;

            if (!pb_q_is_paused(q) && pb_q_is_finished(q)) {
                switch (q->loopstyle) {
                case P_LOOP_NONE:
                    pb_q_pause(q);
                    break;
                case P_LOOP_SINGLE:
                    pb_q_set_current_time(q, 0);
                    break;
                case P_LOOP_QUEUE:
                    pb_q_next(q);
                    break;
                }
            }

            if (pb_q_is_finished(q) && pb_q_is_last(q)) {
                if (q->loopstyle == P_LOOP_QUEUE && q->songs->length > 0) {
                    pb_q_set_active(q, 0);
                    pb_q_play(q);
                } else {
                    pb_q_pause(q);
                }
            }
        }
    }

    return 0;
}

T_CODE pb_init(PlaybackContext **playback_ctx) {
    PlaybackContext *pbc = malloc(sizeof(PlaybackContext));

    if (!pbc) {
        error_log("Memory allocation failed for Playback Context");
        return T_FAIL;
    }
    pbc->queues = vec_init(sizeof(Queue *));
    pbc->active_queue = -1;
    pbc->thread_running = true;

    thrd_create(&pbc->thread, thread_playback_handle, pbc);

    *playback_ctx = pbc;

    return T_SUCCESS;
}

void pb_free(PlaybackContext *pbc) {
    if (pbc == NULL)
        return;

    deactivate_playback_handle();

    thrd_join(pbc->thread, NULL);

    for (int i = 0; i < pbc->queues->length; i++) {
        Queue *q = vec_get_ref(pbc->queues, i);
        pb_q_free(q);
    }

    vec_free(pbc->queues);

    free(pbc);
}

MusicQueue *pb_musicq_alloc() {
    MusicQueue *mq = malloc(sizeof(MusicQueue));
    if (mq == NULL)
        return NULL;

    mq->fullpath = NULL;
    mq->id = 0;
    mq->title = NULL;

    return mq;
}

void pb_musicq_free(MusicQueue *mq) {
    if (mq == NULL)
        return;

    if (mq->title != NULL)
        free(mq->title);

    if (mq->fullpath != NULL)
        free(mq->fullpath);

    free(mq);
}

Queue *pb_q_alloc() {
    Queue *q = malloc(sizeof(Queue));
    if (q == NULL)
        return NULL;

    q->active = -1;
    q->feed = NULL;
    q->songs = vec_init(sizeof(MusicQueue *));
    q->volume = 1;
    q->loopstyle = P_LOOP_NONE;

    return q;
}

void pb_q_free(Queue *q) {
    if (q == NULL)
        return;

    for (int i = 0; i < q->songs->length; i++) {
        MusicQueue *musicq = vec_get_ref(q->songs, i);
        pb_musicq_free(musicq);
    }

    vec_free(q->songs);
    free(q);
}

Vec *pb_q_all(Queue *q) { return q->songs; }
MusicQueue *pb_q_get(Queue *q, int index) {
    if (index == -1)
        return NULL;

    return vec_get_ref(q->songs, index);
}

MusicQueue *pb_q_get_active(Queue *q) { return pb_q_get(q, q->active); }

T_CODE pb_q_add(Queue *q, MusicQueue *m) {
    if (q->songs->length == 0) {
        q->active = 0;
        int ret = pb_q_set_src(q, m->fullpath);
        if (ret != T_SUCCESS)
            return ret;
    }

    vec_push(q->songs, &m);
    return T_SUCCESS;
}

void pb_q_remove(Queue *q, int index) {
    if (q->active == index) {
        a_playback_feed_free(q->feed);
        q->feed = NULL;
        q->active = -1;
    }

    if (q->active >= index) {
        q->active--;
    }

    MusicQueue *mq = vec_get_ref(q->songs, index);
    pb_musicq_free(mq);
    vec_remove(q->songs, index);
}

T_CODE pb_q_set_active(Queue *q, int index) {

    MusicQueue *mq = pb_q_get(q, index);
    if (mq == NULL)
        return T_SUCCESS;

    if (q->feed) {
        a_playback_feed_free(q->feed);
        q->feed = NULL;
    }

    q->active = index;
    return pb_q_set_src(q, mq->fullpath);
}

T_CODE pb_q_next(Queue *q) {
    if (q->active == -1 || q->songs->length - 1 == q->active)
        return T_SUCCESS;

    return pb_q_set_active(q, q->active + 1);
    pb_q_play(q);
}

T_CODE pb_q_previous(Queue *q) {
    if (q->active == -1 || q->active == 0)
        return T_SUCCESS;

    int ret = pb_q_set_active(q, q->active - 1);
    pb_q_play(q);
    return ret;
}

void pb_add_q(PlaybackContext *pbc, Queue *q) {
    if (pbc->queues->length == 0)
        pbc->active_queue = 0;

    vec_push(pbc->queues, &q);
};

T_CODE pb_q_set_src(Queue *q, char *filename) {
    uint8_t *data;
    int size = f_read_file(filename, &data);

    AAudioContext *audio_ctx;
    if (a_audio_context_init(data, size, &audio_ctx) != 0) {
        error_log("audio context init");
        q->feed = NULL;
        q->active = -1;
        return T_FAIL;
    }

    int sample_rate = audio_ctx->codec_ctx->sample_rate;
    int nb_channels = audio_ctx->codec_ctx->ch_layout.nb_channels;

    AAudioVector *au_vec;
    if (a_audio_decode(audio_ctx, &au_vec, NULL) != 0) {
        error_log("audio could not be decoded");
        q->feed = NULL;
        q->active = -1;
        a_audio_free_context(audio_ctx);
        return T_FAIL;
    }
    a_audio_free_context(audio_ctx);

    APlaybackFeed *pbfeed;
    a_playback_feed_init(&pbfeed, au_vec, sample_rate, nb_channels);
    a_playback(pbfeed);

    q->feed = pbfeed;
    q->feed->volume = q->volume;

    return T_SUCCESS;
}

bool pb_q_paused(Queue *q) {
    if (q->feed == NULL)
        return true;

    return q->feed->paused;
}

T_CODE pb_q_play(Queue *q) {
    if (q->feed == NULL) {
        return T_FAIL;
    }

    a_play(q->feed);
    return T_SUCCESS;
}

T_CODE pb_q_pause(Queue *q) {
    if (q->feed == NULL) {
        return T_FAIL;
    }

    a_pause(q->feed);
    return T_SUCCESS;
}

void pb_q_set_volume(Queue *q, float vol) {
    float final_vol = vol;

    if (final_vol < 0)
        final_vol = 0;

    if (final_vol > 1)
        final_vol = 1;

    if (q->feed)
        q->feed->volume = final_vol;
    q->volume = final_vol;
}

bool pb_q_is_last(Queue *q) { return q->active == q->songs->length - 1; }
bool pb_q_is_finished(Queue *q) {
    if (q->feed == NULL)
        return false;

    long duration = q->feed->data->length / (sizeof(float) * q->feed->channels);

    return duration == q->feed->samples_played;
}

bool pb_q_is_paused(Queue *q) {
    if (q->feed == NULL)
        true;
    return q->feed->paused;
}

void pb_q_set_current_time(Queue *q, float seconds) {
    if (q->feed == NULL)
        return;

    a_set_current_time(q->feed, seconds);
}

float pb_q_get_current_time(Queue *q) {
    if (q->feed == NULL)
        return 0;

    return a_get_current_time(q->feed);
}

float pb_q_get_duration(Queue *q) {
    if (q->feed == NULL)
        return 0;

    return a_get_duration(q->feed);
}
