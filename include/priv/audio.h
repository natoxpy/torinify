#ifndef MINIAUDIO_IMPLEMENTATION
#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio.h>
#endif

#ifndef _AUDIO_H
#define _AUDIO_H
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/audio_fifo.h>
#include <libavutil/samplefmt.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <threads.h>
#include <unistd.h>

typedef struct FileContext FileContext;

struct FileContext {
    AVFormatContext *fmt_ctx;
    AVCodecContext *dec_ctx;
    int stream_index;
};

typedef struct DecodedFileContext DecodedFileContext;
typedef struct RawData RawData;
typedef struct PlaybackReadyContext PlaybackReadyContext;

struct DecodedFileContext {
    RawData *data;
    int nb_samples;
};

struct RawData {
    uint8_t *data;
    int capacity;
    int length;
};

struct PlaybackReadyContext {
    AVAudioFifo *fifo;
    uint8_t *data;
    int channels;
    int sample_rate;
    ma_format ma_format;
    int nb_samples;
};

FileContext *cl_file_context_alloc();

void cl_file_context_free(FileContext *file_ctx);

int cl_open_input_file(FileContext **in_file_ctx, const char *filename);

void cl_playback_ready_context_free(PlaybackReadyContext *pb_ready_ctx);

RawData *cl_rawdata_alloc();

DecodedFileContext *cl_decoded_file_context_alloc();

int cl_rawdata_append(RawData *data_ptr, uint8_t *data, int size);

void cl_rawdata_free(RawData *data_ptr);

void cl_decoded_file_free(DecodedFileContext *decd_ctx);

static void print_frame(const uint8_t *p, const int n);

PlaybackReadyContext *
cl_playback_ready_context_init(FileContext *file_ctx,
                               DecodedFileContext *decoded_ctx);

int cl_decoded_file_ctx(FileContext *file_ctx,
                        DecodedFileContext **decoded_ctx);
#endif
