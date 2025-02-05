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

T_CODE tf_init_db(char *filename, char *migrations_dir) {
    int ret;

    if ((ret = tf_sqlite3_init(filename)) != T_SUCCESS)
        return ret;

    if ((ret = tf_sqlite3_migrations(migrations_dir)) != T_SUCCESS)
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

T_CODE tf_sqlite3_migrations(char *migrations_dir) {
    return m_migrations(tgc->sqlite3, migrations_dir);
}

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

T_CODE tf_set_src(char *filename) {
    uint8_t *data;
    int size = f_read_file(filename, &data);

    AAudioContext *audio_ctx;
    if (a_audio_context_init(data, size, &audio_ctx) != 0)
        fprintf(stderr, "audio context init");

    int sample_rate = audio_ctx->codec_ctx->sample_rate;
    int nb_channels = audio_ctx->codec_ctx->ch_layout.nb_channels;

    AAudioVector *au_vec;
    if (a_audio_decode(audio_ctx, &au_vec) != 0)
        fprintf(stderr, "audio could not be decoded");
    a_audio_free_context(audio_ctx);

    APlaybackFeed *pbfeed;
    a_playback_feed_init(&pbfeed, au_vec, sample_rate, nb_channels);
    a_playback(pbfeed);

    tgc->playback->feed = pbfeed;

    return T_SUCCESS;
}

T_CODE tf_play() {
    a_play(tgc->playback->feed);
    return T_SUCCESS;
}

T_CODE tf_pause() {
    a_pause(tgc->playback->feed);
    return T_SUCCESS;
}

int tf_get_paused() { return tgc->playback->feed->paused; }

void tf_set_current_time(long miliseconds) {
    a_set_current_time(tgc->playback->feed, miliseconds);
}

long tf_get_current_time() { return a_get_current_time(tgc->playback->feed); }
