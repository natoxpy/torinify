#include <audio/audio.h>
#include <libavcodec/packet.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libswresample/swresample.h>
#include <stdint.h>
#include <stdio.h>

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

int a_audio_decode(AAudioContext *au_ctx, AAudioVector **out_au_vec,
                   DecoderStats *dstats) {
    int ret = 0;
    AAudioVector *audio_vec;

    if (!(audio_vec = a_audio_vector_alloc(1))) {
        ret = -1;
        goto clean;
    }

    AVPacket *packet = av_packet_alloc();
    AVFrame *frame = av_frame_alloc();

    if (!packet || !frame) {
        ret = -1;
        error_log("Packet or Frames not alloced");
        goto clean;
    }

    SwrContext *swr_ctx;

    ret = quick_swr_init(
        &swr_ctx, &au_ctx->codec_ctx->ch_layout, au_ctx->codec_ctx->sample_rate,
        au_ctx->fmt_ctx->streams[au_ctx->stream_index]->codecpar->format);

    if (ret < 0) {
        error_log("SWR failed to init");
        goto clean;
    }

    int seek_seconds = 10;

    int64_t timestamp =
        av_rescale_q(seek_seconds * AV_TIME_BASE, AV_TIME_BASE_Q,
                     au_ctx->fmt_ctx->streams[au_ctx->stream_index]->time_base);

    av_seek_frame(au_ctx->fmt_ctx, au_ctx->stream_index, timestamp,
                  AVSEEK_FLAG_BACKWARD);

    avcodec_flush_buffers(au_ctx->codec_ctx);

    while (1) {
        if ((ret = av_read_frame(au_ctx->fmt_ctx, packet)) < 0)
            break;

        if (packet->stream_index != au_ctx->stream_index) {
            av_packet_unref(packet);
            continue;
        }

        if (dstats)
            dstats->total_packets++;

        if ((ret = avcodec_send_packet(au_ctx->codec_ctx, packet)) < 0) {
            error_log("AVCoded Packet failed to send");
            av_packet_unref(packet);
            if (dstats)
                dstats->lost_packets++;
            continue;
        }

        while (ret >= 0) {
            ret = avcodec_receive_frame(au_ctx->codec_ctx, frame);
            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                break;
            } else if (ret < 0) {
                error_log("AVCoded Invalid packet");
                goto clean;
            }

            if (ret > 0)
                continue;

            if (dstats)
                dstats->packets++;

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

clean:
    av_packet_free(&packet);
    av_frame_free(&frame);
    swr_free(&swr_ctx);

    if (ret < 0) {
        a_audio_vector_free(audio_vec);
        return -1;
    }

    return 0;
}
