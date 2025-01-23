#include "audio.c"
#include <libavutil/audio_fifo.h>
#include <libavutil/samplefmt.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <threads.h>
#include <unistd.h>

typedef struct PlaybackContext PlaybackContext;

#define CLAMP(x, low, high)                                                    \
    ((x) < (low) ? (low) : ((x) > (high) ? (high) : (x)))

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

PlaybackContext *cl_playback_alloc() {
    PlaybackContext *pb_ctx = malloc(sizeof(PlaybackContext));

    if (!pb_ctx) {
        fprintf(stderr, "Memory allocation failed for PlaybackContext\n");
        return NULL;
    }

    pb_ctx->ma_device = malloc(sizeof(ma_device));

    pb_ctx->duration = 0;
    pb_ctx->paused = true;
    pb_ctx->samples_consumed = 0;
    pb_ctx->volume = 1.0f;

    return pb_ctx;
}

void cl_data_callback(ma_device *pDevice, void *pOutput, const void *pInput,
                      ma_uint32 frameCount) {

    PlaybackContext *pb_ctx = (PlaybackContext *)pDevice->pUserData;

    AVAudioFifo *fifo = pb_ctx->ctx->fifo;

    int samples_consumed = av_audio_fifo_read(fifo, &pOutput, frameCount);

    float *output = (float *)pOutput;

    if (samples_consumed >= 0)
        pb_ctx->samples_consumed += samples_consumed;

    for (ma_uint32 i = 0; i < frameCount * pb_ctx->ctx->channels; ++i) {
        output[i] = output[i] * pb_ctx->volume;
    }
}

int cl_init_device(PlaybackContext *pb_ctx) {
    ma_device_config deviceConfig;

    deviceConfig = ma_device_config_init(ma_device_type_playback);
    deviceConfig.playback.format = pb_ctx->ctx->ma_format;
    deviceConfig.playback.channels = pb_ctx->ctx->channels;
    deviceConfig.sampleRate = pb_ctx->ctx->sample_rate;
    deviceConfig.dataCallback = cl_data_callback;
    deviceConfig.pUserData = pb_ctx;

    if (ma_device_init(NULL, &deviceConfig, pb_ctx->ma_device) != MA_SUCCESS) {
        fprintf(stderr, "Failed to open playback device. \n");

        return -3;
    }

    return 0;
}

int cl_start_device(ma_device *device) {
    if (ma_device_start(device) != MA_SUCCESS) {
        fprintf(stderr, "Failed to start playback device. \n");
        ma_device_uninit(device);
        return -4;
    }

    return 0;
}

int cl_stop_device(ma_device *device) {
    if (ma_device_stop(device) != MA_SUCCESS) {
        fprintf(stderr, "Failed to start playback device. \n");
        ma_device_uninit(device);
        return -4;
    }

    return 0;
}

int cl_playback(PlaybackContext *pb_ctx) {

    int ret = cl_init_device(pb_ctx);

    if (ret < 0)
        return ret;

    cl_start_device(pb_ctx->ma_device);

    return 0;
}

int cl_set_audio_current_time(PlaybackContext *pb_ctx,
                              unsigned int miliseconds) {

    av_audio_fifo_reset(pb_ctx->ctx->fifo);

    int mili_sample_rate = pb_ctx->ctx->sample_rate / 1000;
    int consume = miliseconds * mili_sample_rate;

    int skip = consume * pb_ctx->ctx->channels *
               av_get_bytes_per_sample(AV_SAMPLE_FMT_FLT);

    void *data = pb_ctx->ctx->data + skip;

    av_audio_fifo_write(pb_ctx->ctx->fifo, (void **)&data,
                        pb_ctx->ctx->nb_samples - consume);

    pb_ctx->samples_consumed = consume;

    return 0;
}
