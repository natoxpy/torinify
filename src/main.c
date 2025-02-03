/// Example: how to use Torinify
#include <audio/audio.h>
#include <db/exec.h>
#include <errors/errors.h>
#include <libavutil/dict.h>
#include <media/media.h>
#include <sqlite3.h>
#include <stdbool.h>
#include <stdio.h>
#include <taglib/tag_c.h>
#include <torinify/core.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    int ret = T_SUCCESS;

    if ((ret = tf_init()) != T_SUCCESS ||
        (ret = tf_init_db("../sqlite.db", "../migrations")) != T_SUCCESS)
        goto end;

    uint8_t *data;
    int size = f_read_file("m/IronLotus.wav", &data);

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

    a_set_current_time(pbfeed, 13000);

    scanf("main");
end:
    a_playback_feed_free(pbfeed);
    tf_cleanup();

    if (ret != T_SUCCESS) {
        error_print_all();
        return -1;
    }

    return 0;
}
