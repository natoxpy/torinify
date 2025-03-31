#include "db/sql.h"
#include "torinify/search_engine.h"
#include "utils/file.h"
#include <db/migrations.h>
#include <errors/errors.h>
#include <libavutil/samplefmt.h>
#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <torinify/core.h>
#include <torinify/playback.h>

/// Torinify Global Context
TorinifyContext *tgc = NULL;

Queue *get_active_queue() {
    if (tgc->playback->active_queue == UINT32_MAX)
        return NULL;

    return vec_get(tgc->playback->queues, tgc->playback->active_queue);
}

T_CODE tf_init_db(char *filename) {
    int ret;

    if ((ret = tf_sqlite3_init(filename)) != T_SUCCESS)
        return ret;

    if ((ret = tf_sqlite3_migrations()) != T_SUCCESS)
        return ret;

    return T_SUCCESS;
}

T_CODE tf_sqlite3_init(char *filename) {
    sqlite3 *db;
    int ret = sqlite3_open(filename, &db);

    if (ret != SQLITE_OK) {
        error_log("Could not open SQLITE file \"%s\"", filename);
        return T_FAIL;
    }

    tgc->sqlite3 = db;

    return T_SUCCESS;
}

T_CODE tf_sqlite3_migrations() {
    if (migrate_database(tgc->sqlite3) == TDB_SUCCESS)
        return T_SUCCESS;
    else
        return T_FAIL;
}

void thread_handle_queue(void *args) {}

/// Initiates Torinify Global Context (`tgc`)
T_CODE tf_init() {
    int ret = 0;
    tgc = malloc(sizeof(TorinifyContext));

    if (!tgc) {
        error_log("Could not allocate enough memory for TorinifyContext");
        return T_FAIL;
    }

    tgc->playback = NULL;
    tgc->sqlite3 = NULL;

    if ((ret = pb_init(&tgc->playback)) != T_SUCCESS)
        return ret;

    av_log_set_level(AV_LOG_QUIET);

    return T_SUCCESS;
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

T_CODE tf_scan_sources() {
    // M_scan(tgc->sqlite3);
    return T_SUCCESS;
}

T_CODE tf_set_src(char *filename) {
    uint8_t *data;
    int size = f_read_file(filename, &data);

    AAudioContext *audio_ctx;
    if (a_audio_context_init(data, size, &audio_ctx) != 0)
        fprintf(stderr, "audio context init");

    int sample_rate = audio_ctx->codec_ctx->sample_rate;
    int nb_channels = audio_ctx->codec_ctx->ch_layout.nb_channels;

    AAudioVector *au_vec;
    if (a_audio_decode(audio_ctx, &au_vec, NULL) != 0)
        fprintf(stderr, "audio could not be decoded");
    a_audio_free_context(audio_ctx);

    APlaybackFeed *pbfeed;
    a_playback_feed_init(&pbfeed, au_vec, sample_rate, nb_channels);
    a_playback(pbfeed);

    Queue *q = get_active_queue();

    if (q == NULL)
        return T_FAIL;

    q->feed = pbfeed;

    return T_SUCCESS;
}

T_CODE tf_play() {
    Queue *q = get_active_queue();

    if (q == NULL)
        return T_FAIL;

    a_play(q->feed);
    return T_SUCCESS;
}

T_CODE tf_pause() {
    Queue *q = get_active_queue();

    if (q == NULL)
        return T_FAIL;

    a_pause(q->feed);
    return T_SUCCESS;
}

int tf_get_paused() {
    Queue *q = get_active_queue();

    if (q == NULL)
        return 0;

    return q->feed->paused;
}

void tf_set_current_time(long miliseconds) {
    Queue *q = get_active_queue();

    if (q == NULL)
        return;

    a_set_current_time(q->feed, miliseconds);
}

long tf_get_current_time() {
    Queue *q = get_active_queue();

    if (q == NULL)
        return 0;

    return a_get_current_time(q->feed);
}

void tf_search(char *query, double threshold, Vec **results) {
    Vec *search_ctx = NULL;
    s_vec_search_context_init(tgc->sqlite3, &search_ctx);
    s_process_search(search_ctx, query, results, threshold);
    s_vec_search_context_free(search_ctx);
}
