#include <libavcodec/avcodec.h>
#include <libavcodec/codec.h>
#include <libavformat/avformat.h>
#include <libavutil/audio_fifo.h>
#include <libavutil/frame.h>
#include <libavutil/samplefmt.h>
#include <libswresample/swresample.h>
#include <priv/audio.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

FileContext *cl_file_context_alloc() {

    FileContext *file_ctx = malloc(sizeof(FileContext));

    if (!file_ctx) {
        fprintf(stderr, "Memory allocation failed for FileContext\n");
        return NULL;
    }

    file_ctx->fmt_ctx = NULL;
    file_ctx->dec_ctx = NULL;
    file_ctx->stream_index = -1;

    return file_ctx;
}

void cl_file_context_free(FileContext *file_ctx) {
    if (file_ctx == NULL)
        return;

    avformat_close_input(&file_ctx->fmt_ctx);
    avcodec_free_context(&file_ctx->dec_ctx);
    free(file_ctx);
}

int cl_open_input_file(FileContext **in_file_ctx, const char *filename) {
    const AVCodec *dec;
    int ret;

    FileContext *file_ctx = cl_file_context_alloc();

    if ((ret = avformat_open_input(&file_ctx->fmt_ctx, filename, NULL, NULL)) <
        0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot open input file\n");
        return ret;
    }

    if ((ret = avformat_find_stream_info(file_ctx->fmt_ctx, NULL)) < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot find stream information\n");
        return ret;
    }

    ret = av_find_best_stream(file_ctx->fmt_ctx, AVMEDIA_TYPE_AUDIO, -1, -1,
                              &dec, 0);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR,
               "Cannot find an audio stream in the input file\n");
        return ret;
    }

    file_ctx->stream_index = ret;

    AVCodecContext *dec_ctx = avcodec_alloc_context3(dec);
    if (!dec_ctx) {
        return AVERROR(ENOMEM);
    }

    avcodec_parameters_to_context(
        dec_ctx, file_ctx->fmt_ctx->streams[file_ctx->stream_index]->codecpar);

    if ((ret = avcodec_open2(dec_ctx, dec, NULL)) < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannon open audio decoder \n");
        return ret;
    }

    file_ctx->dec_ctx = dec_ctx;

    *in_file_ctx = file_ctx;

    return 0;
}

void cl_playback_ready_context_free(PlaybackReadyContext *pb_ready_ctx) {

    if (!pb_ready_ctx)
        return;

    av_audio_fifo_free(pb_ready_ctx->fifo);
    free(pb_ready_ctx->data);
    free(pb_ready_ctx);
}

RawData *cl_rawdata_alloc() {
    RawData *data_ptr = malloc(sizeof(RawData));

    data_ptr->capacity = 1;
    data_ptr->length = 0;
    data_ptr->data = calloc(data_ptr->capacity, sizeof(uint8_t));

    return data_ptr;
}

DecodedFileContext *cl_decoded_file_context_alloc() {
    DecodedFileContext *decd_ctx = malloc(sizeof(DecodedFileContext));

    if (!decd_ctx) {
        fprintf(stderr, "Cannon allocate memory for DecodedFileContext\n");
        return NULL;
    }

    decd_ctx->data = cl_rawdata_alloc();
    decd_ctx->nb_samples = 0;

    return decd_ctx;
}

int cl_rawdata_append(RawData *data_ptr, uint8_t *data, int size) {
    while (data_ptr->length + size > data_ptr->capacity) {
        data_ptr->capacity = data_ptr->capacity * 2;
        data_ptr->data = realloc(data_ptr->data, data_ptr->capacity);
    }

    memcpy(data_ptr->data + data_ptr->length, data, size);
    data_ptr->length += size;
    return 0;
}

void cl_rawdata_free(RawData *data_ptr) {
    if (!data_ptr)
        return;

    free(data_ptr->data);
    free(data_ptr);
}

void cl_decoded_file_free(DecodedFileContext *decd_ctx) {
    if (!decd_ctx)
        return;

    cl_rawdata_free(decd_ctx->data);
    free(decd_ctx);
}

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

PlaybackReadyContext *
cl_playback_ready_context_init(FileContext *file_ctx,
                               DecodedFileContext *decoded_ctx) {
    PlaybackReadyContext *ptr = malloc(sizeof(PlaybackReadyContext));

    AVAudioFifo *fifo = av_audio_fifo_alloc(
        AV_SAMPLE_FMT_FLT, file_ctx->dec_ctx->ch_layout.nb_channels,
        decoded_ctx->nb_samples);

    if (!ptr) {
        fprintf(stderr, "Could not allocate memory for PlaybackReadyContext\n");
        return NULL;
    }

    uint8_t *data = decoded_ctx->data->data;

    av_audio_fifo_write(fifo, (void **)&data, decoded_ctx->nb_samples);

    ptr->fifo = fifo;
    ptr->data = decoded_ctx->data->data;
    ptr->channels = file_ctx->dec_ctx->ch_layout.nb_channels;
    ptr->sample_rate = file_ctx->dec_ctx->sample_rate;
    ptr->ma_format = ma_format_f32;
    ptr->nb_samples = decoded_ctx->nb_samples;

    cl_file_context_free(file_ctx);
    free(decoded_ctx->data);
    free(decoded_ctx);

    return ptr;
}

int cl_decoded_file_ctx(FileContext *file_ctx,
                        DecodedFileContext **decoded_ctx) {
    int ret;
    AVPacket *packet = av_packet_alloc();
    AVFrame *frame = av_frame_alloc();

    AVStream *stream = file_ctx->fmt_ctx->streams[file_ctx->stream_index];

    DecodedFileContext *decoded_file = cl_decoded_file_context_alloc();

    //

    SwrContext *swr_ctx = swr_alloc();

    if (!swr_ctx) {
        fprintf(stderr, "Could not allocate resampler context\n");
        return AVERROR(ENOMEM);
    }

    printf("sample %d\n", file_ctx->dec_ctx->sample_rate);
    swr_alloc_set_opts2(&swr_ctx, &file_ctx->dec_ctx->ch_layout,
                        AV_SAMPLE_FMT_FLT, file_ctx->dec_ctx->sample_rate,
                        &file_ctx->dec_ctx->ch_layout, stream->codecpar->format,
                        file_ctx->dec_ctx->sample_rate, 0, NULL);

    if ((ret = swr_init(swr_ctx)) < 0) {
        fprintf(stderr, "Failed to initialize the resampling context\n");
        return ret;
    }

    //

    while (1) {
        if ((ret = av_read_frame(file_ctx->fmt_ctx, packet)) < 0)
            break;

        if (packet->stream_index == file_ctx->stream_index) {
            ret = avcodec_send_packet(file_ctx->dec_ctx, packet);
            if (ret < 0) {
                av_log(NULL, AV_LOG_ERROR,
                       "Error while sending a frame from the decoder\n");
                break;
            }

            while (ret >= 0) {
                ret = avcodec_receive_frame(file_ctx->dec_ctx, frame);
                if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                    break;
                } else if (ret < 0) {
                    av_log(NULL, AV_LOG_ERROR,
                           "Error while receiving a frame from the decoder\n");
                    return ret;
                }

                if (ret >= 0) {

                    uint8_t *output_buffer = NULL;

                    int output_samples = av_rescale_rnd(
                        frame->nb_samples, file_ctx->dec_ctx->sample_rate,
                        stream->codecpar->sample_rate, AV_ROUND_UP);

                    av_samples_alloc(&output_buffer, NULL,
                                     stream->codecpar->ch_layout.nb_channels,
                                     output_samples, AV_SAMPLE_FMT_FLT, 0);

                    swr_convert(swr_ctx, &output_buffer, output_samples,
                                (const uint8_t **)frame->data,
                                frame->nb_samples);

                    decoded_file->nb_samples += output_samples;

                    int size = output_samples *
                               av_get_bytes_per_sample(AV_SAMPLE_FMT_FLT) *
                               frame->ch_layout.nb_channels;

                    // print_frame(output_buffer, size);

                    cl_rawdata_append(decoded_file->data, output_buffer, size);

                    av_freep(&output_buffer);
                    av_frame_unref(frame);
                }
            }
        }

        av_packet_unref(packet);
    }

    // print_frame(decoded_file->data->data, decoded_file->nb_samples * 2);

    av_frame_free(&frame);
    av_packet_free(&packet);

    swr_free(&swr_ctx);

    // RESAMPLING END

    *decoded_ctx = decoded_file;

    return 0;
}
