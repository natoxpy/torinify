#include "miniaudio.h"
#include <audio/audio.h>
#include <errors/errors.h>
#include <stdint.h>

APlaybackFeed *a_playback_feed_alloc() {
    APlaybackFeed *pb = malloc(sizeof(APlaybackFeed));

    if (pb == NULL) {
        error_log("Could not allocate enough memory for playback feed");
        return NULL;
    }

    pb->data = NULL;
    pb->device = NULL;
    pb->paused = 1;
    pb->sample_rate = 0;
    pb->samples_played = 0;
    pb->volume = 1.0f;
    pb->channels = 2;

    return pb;
}

void a_playback_feed_free(APlaybackFeed *playback_feed) {
    if (playback_feed == NULL)
        return;

    if (playback_feed->device) {
        ma_device_uninit(playback_feed->device);
        free(playback_feed->device);
    }

    if (playback_feed->data)
        a_audio_vector_free(playback_feed->data);

    free(playback_feed);
}

T_CODE a_playback_feed_init(APlaybackFeed **pb, AAudioVector *au_vec,
                            int sample_rate, int channels) {
    int ret = T_SUCCESS;
    APlaybackFeed *playback_feed = a_playback_feed_alloc();
    if (playback_feed == NULL) {
        ret = T_FAIL;
        goto cleanup;
    }

    playback_feed->data = au_vec;
    playback_feed->sample_rate = sample_rate;
    playback_feed->channels = channels;

    *pb = playback_feed;
cleanup:

    if (ret != T_SUCCESS)
        a_playback_feed_free(playback_feed);

    return ret;
}

void data_callback(ma_device *pDevice, void *pOutput, const void *pInput,
                   ma_uint32 frameCount) {
    APlaybackFeed *playback_feed = (APlaybackFeed *)pDevice->pUserData;

    if (playback_feed == NULL || playback_feed->data == NULL ||
        playback_feed->data->ptr == NULL) {
        memset(pOutput, 0,
               frameCount * pDevice->playback.channels * sizeof(float));
        return;
    }

    float *audio_data = (float *)playback_feed->data->ptr;

    // Calculate the starting sample index
    size_t startSample =
        playback_feed->samples_played * pDevice->playback.channels;
    size_t samplesToCopy = frameCount * pDevice->playback.channels;

    // Ensure we don't read past the buffer size
    size_t availableSamples = playback_feed->data->length / sizeof(float);
    if (startSample >= availableSamples) {
        // No more data to play, fill buffer with silence
        memset(pOutput, 0,
               frameCount * pDevice->playback.channels * sizeof(float));
        return;
    }

    // Adjust samplesToCopy if necessary
    if (startSample + samplesToCopy > availableSamples) {
        samplesToCopy = availableSamples - startSample;
    }

    // Copy audio data from playback buffer to output
    memcpy(pOutput, &audio_data[startSample], samplesToCopy * sizeof(float));

    // Fill remaining samples with silence if there wasn't enough data
    if (samplesToCopy < frameCount * pDevice->playback.channels) {
        a_pause(playback_feed);

        memset((float *)pOutput + samplesToCopy, 0,
               (frameCount * pDevice->playback.channels - samplesToCopy) *
                   sizeof(float));
    }

    float *data = pOutput;

    for (int i = 0; i < frameCount * pDevice->playback.channels; i++) {
        data[i] *= playback_feed->volume;

        // Optional: Clipping to prevent distortion
        if (data[i] > 1.0f)
            data[i] = 1.0f;
        if (data[i] < -1.0f)
            data[i] = -1.0f;
    }

    // Update playback position
    playback_feed->samples_played += frameCount;
}

T_CODE a_playback(APlaybackFeed *playback_feed) {
    ma_device_config config = ma_device_config_init(ma_device_type_playback);
    config.playback.format = ma_format_f32;

    config.playback.channels = playback_feed->channels;
    config.sampleRate = playback_feed->sample_rate;

    config.dataCallback = data_callback;
    config.pUserData = playback_feed;

    ma_device *device = malloc(sizeof(ma_device));

    if (ma_device_init(NULL, &config, device) != MA_SUCCESS) {
        return -1; // Failed to initialize the device.
    }

    playback_feed->device = device;

    return T_SUCCESS;
}

void a_pause(APlaybackFeed *playback_feed) {
    if (!playback_feed->device)
        return;

    playback_feed->paused = 1;
    ma_device_stop(playback_feed->device);
}

void a_play(APlaybackFeed *playback_feed) {
    if (!playback_feed->device)
        return;

    playback_feed->paused = 0;
    ma_device_start(playback_feed->device);
}

void a_set_current_time(APlaybackFeed *playback_feed,
                        unsigned long miliseconds) {
    if (!playback_feed->device)
        return;

    long samples_ms = playback_feed->sample_rate / 1000;
    playback_feed->samples_played = samples_ms * miliseconds;
}

long a_get_current_time(APlaybackFeed *playback_feed) {
    float samples_ms = (float)playback_feed->samples_played /
                       (float)playback_feed->sample_rate;

    return (long)(samples_ms * 1000);
}

long a_get_duration(APlaybackFeed *playback_feed) {
    float samples_ms =
        (float)playback_feed->data->samples / (float)playback_feed->sample_rate;

    return (long)(samples_ms * 1000);
}
