#include <audio/audio.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavformat/avio.h>
#include <libavutil/avutil.h>
#include <libavutil/error.h>
#include <stdint.h>

static int read_packet(void *opaque, uint8_t *buf, int buf_size) {
    AAudioContextBuffer *bd = (AAudioContextBuffer *)opaque;
    buf_size = FFMIN(buf_size, bd->length);

    if (!buf_size)
        return AVERROR_EOF;

    /* copy internal buffer data to buf */
    memcpy(buf, bd->ptr, buf_size);
    bd->ptr += buf_size;
    bd->length -= buf_size;

    return buf_size;
}

AAudioContext *a_audio_alloc_context() {
    AAudioContext *audio_ctx = malloc(sizeof(AAudioContext));

    if (!audio_ctx)
        return NULL;

    audio_ctx->au_buf = NULL;
    audio_ctx->au_vec = NULL;
    audio_ctx->fmt_ctx = NULL;
    audio_ctx->codec_ctx = NULL;
    audio_ctx->stream_index = -1;

    return audio_ctx;
}

void a_audio_free_context(AAudioContext *audio_ctx) {
    if (audio_ctx->au_buf != NULL) {
        free(audio_ctx->au_buf);
    }

    if (audio_ctx->au_vec != NULL) {
        a_audio_vector_free(audio_ctx->au_vec);
    }

    if (audio_ctx->fmt_ctx != NULL) {
        if (audio_ctx->fmt_ctx->pb)
            av_freep(&audio_ctx->fmt_ctx->pb->buffer);

        avio_context_free(&audio_ctx->fmt_ctx->pb);
        avformat_close_input(&audio_ctx->fmt_ctx);
    }

    if (audio_ctx->codec_ctx != NULL) {
        avcodec_free_context(&audio_ctx->codec_ctx);
    }

    free(audio_ctx);
}

int fmt_ctx_audio_ctx(AAudioContext *audio_ctx, AAudioContextBuffer *au_buf) {
    int ret = 0;
    audio_ctx->fmt_ctx = avformat_alloc_context();
    AVFormatContext *fmt_ctx = audio_ctx->fmt_ctx;

    if (!fmt_ctx) {
        ret = -1;
        goto end;
    }

    int avio_ctx_buffer_size = 4096;
    uint8_t *avio_ctx_buffer = av_malloc(avio_ctx_buffer_size);

    if (avio_ctx_buffer == NULL) {
        ret = -1;
        goto end;
    }

    AVIOContext *avio_ctx =
        avio_alloc_context(avio_ctx_buffer, avio_ctx_buffer_size, 0, au_buf,
                           &read_packet, NULL, NULL);

    if (avio_ctx == NULL) {
        ret = -1;
        goto end;
    }

    fmt_ctx->pb = avio_ctx;

    if (avformat_open_input(&fmt_ctx, NULL, NULL, NULL) != 0) {

        ret = -1;
        goto end;
    }

    if (avformat_find_stream_info(fmt_ctx, NULL) < 0) {
        ret = -1;
        goto end;
    }

end:
    if (ret < 0)
        return -1;

    return 0;
}
int codec_ctx_audio_ctx(AAudioContext *audio_ctx) {

    int ret = 0;
    const AVCodec *codec;
    int stream_index = av_find_best_stream(
        audio_ctx->fmt_ctx, AVMEDIA_TYPE_AUDIO, -1, -1, &codec, 0);

    if (stream_index < 0) {
        ret = -1;
        goto end;
    }

    audio_ctx->stream_index = stream_index;
    audio_ctx->codec_ctx = avcodec_alloc_context3(codec);

    if (!audio_ctx->codec_ctx) {
        ret = 0;
        goto end;
    }

    avcodec_parameters_to_context(
        audio_ctx->codec_ctx,
        audio_ctx->fmt_ctx->streams[audio_ctx->stream_index]->codecpar);

    ret = avcodec_open2(audio_ctx->codec_ctx, codec, NULL);

    if (ret < 0) {
        goto end;
    }

end:
    if (ret < 0)
        return -1;

    return 0;
}

int a_audio_context_init(uint8_t *in_data, size_t in_size,
                         AAudioContext **audio_ctx) {
    int ret = 0;

    AAudioContext *au_ctx = a_audio_alloc_context();

    AAudioVector *au_vec;
    ret = a_audio_vector_init(&au_vec, in_data, in_size);

    if (ret < 0)
        goto end;

    AAudioContextBuffer *au_buf = a_audio_vector_as_buffer(au_vec);

    if (!au_buf) {
        ret = -1;
        goto end;
    }

    ret = fmt_ctx_audio_ctx(au_ctx, au_buf);
    if (ret < 0)
        goto end;

    ret = codec_ctx_audio_ctx(au_ctx);
    if (ret < 0)
        goto end;

    au_ctx->au_buf = au_buf;
    au_ctx->au_vec = au_vec;
    *audio_ctx = au_ctx;

end:
    return 0;
}
