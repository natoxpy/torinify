#ifndef MINIAUDIO_IMPLEMENTATION
#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio.h>
#endif

#ifndef _PLAYBACK_H
#define _PLAYBACK_H
#include <libavutil/audio_fifo.h>
#include <libavutil/samplefmt.h>
#include <priv/audio.h>

#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <threads.h>
#include <unistd.h>

typedef struct PlaybackContext PlaybackContext;

struct PlaybackContext {
    PlaybackReadyContext *ctx;
    ma_device *ma_device;
    unsigned duration;
    bool paused;
    unsigned int samples_consumed;
    float volume;
};

struct PlaybackLoop {
    double *target_current_time;
    bool *target_paused;
    bool ended;
};

PlaybackContext *cl_playback_alloc();

void cl_data_callback(ma_device *pDevice, void *pOutput, const void *pInput,
                      ma_uint32 frameCount);

int cl_init_device(PlaybackContext *pb_ctx);

int cl_start_device(ma_device *device);

int cl_stop_device(ma_device *device);

int cl_playback(PlaybackContext *pb_ctx);

int cl_set_audio_current_time(PlaybackContext *pb_ctx,
                              unsigned int miliseconds);
#endif
