#include <audio/audio.h>
#include <libavcodec/packet.h>
#include <libswresample/swresample.h>

// static void print_frame(const AVFrame *frame) {
//     const int n = frame->nb_samples * frame->ch_layout.nb_channels;
//     const uint16_t *p = (uint16_t *)frame->data[0];
//     const uint16_t *p_end = p + n;
//
//     while (p < p_end) {
//         fputc(*p & 0xff, stdout);
//         fputc(*p >> 8 & 0xff, stdout);
//         p++;
//     }
//     fflush(stdout);
// }

static void print_frame(const uint8_t *p, const int n) {
    // const int n = frame->nb_samples * frame->ch_layout.nb_channels;
    const uint8_t *p_end = p + n;

    while (p < p_end) {
        // fputc(*p, stdout);
        fputc(*p & 0xff, stdout);
        // fputc(*p >> 8 & 0xff, stdout);
        p++;
    }
    fflush(stdout);
}

int quick_swr_init(SwrContext **out_swr_ctx, const AVChannelLayout *in_layout,
                   int in_sample, enum AVSampleFormat in_format) {
    SwrContext *swr_ctx = swr_alloc();

    if (!swr_ctx)
        return AVERROR(ENOMEM);

    int ret =
        swr_alloc_set_opts2(&swr_ctx, in_layout, AV_SAMPLE_FMT_FLT, in_sample,
                            in_layout, in_format, in_sample, 0, NULL);

    if (ret != 0)
        return ret;

    if ((ret = swr_init(swr_ctx)) < 0)
        return ret;

    *out_swr_ctx = swr_ctx;

    return 0;
}

int a_audio_decode(AAudioContext *au_ctx, AAudioVector **out_au_vec) {
    int ret = 0;
    AAudioVector *audio_vec;

    if (!(audio_vec = a_audio_vector_alloc(1))) {
        ret = -1;
        goto end;
    }

    AVPacket *packet = av_packet_alloc();
    AVFrame *frame = av_frame_alloc();

    if (!packet || !frame) {
        ret = -1;
        goto end;
    }

    SwrContext *swr_ctx;

    ret = quick_swr_init(
        &swr_ctx, &au_ctx->codec_ctx->ch_layout, au_ctx->codec_ctx->sample_rate,
        au_ctx->fmt_ctx->streams[au_ctx->stream_index]->codecpar->format);

    if (ret < 0)
        goto end;

    while (1) {
        if ((ret = av_read_frame(au_ctx->fmt_ctx, packet)) < 0)
            break;

        if (packet->stream_index != au_ctx->stream_index) {
            av_packet_unref(packet);
            continue;
        }

        if ((ret = avcodec_send_packet(au_ctx->codec_ctx, packet)) < 0)
            goto end;

        while (ret >= 0) {
            ret = avcodec_receive_frame(au_ctx->codec_ctx, frame);
            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                break;
            } else if (ret < 0)
                goto end;

            if (ret > 0)
                continue;

            uint8_t *output_buffer = NULL;
            int output_nb_samples = frame->nb_samples;

            av_samples_alloc(&output_buffer, NULL,
                             au_ctx->codec_ctx->ch_layout.nb_channels,
                             output_nb_samples, AV_SAMPLE_FMT_FLT, 0);

            swr_convert(swr_ctx, &output_buffer, output_nb_samples,
                        (const uint8_t **)frame->data, frame->nb_samples);

            int size = output_nb_samples *
                       av_get_bytes_per_sample(AV_SAMPLE_FMT_FLT) *
                       frame->ch_layout.nb_channels;

            audio_vec->samples += output_nb_samples;

            a_audio_vector_push(audio_vec, output_buffer,
                                output_nb_samples * sizeof(float) *
                                    au_ctx->codec_ctx->ch_layout.nb_channels);

            av_freep(&output_buffer);
            av_frame_unref(frame);
        }

        av_packet_unref(packet);
    }

    *out_au_vec = audio_vec;
    ret = 0;

end:
    av_packet_free(&packet);
    av_frame_free(&frame);
    swr_free(&swr_ctx);

    if (ret < 0) {
        a_audio_vector_free(audio_vec);
        return -1;
    }

    return 0;
}
