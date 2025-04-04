#ifndef _AUDIO_VECTOR_H
#define _AUDIO_VECTOR_H

#include <errors/errors.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavformat/avio.h>
#include <miniaudio.h>
#include <stdint.h>

typedef struct AAudioVector AAudioVector;

struct AAudioVector {
    uint8_t *ptr;
    int capacity;
    size_t length;
    int samples;
};

typedef struct AAudioContextBuffer AAudioContextBuffer;
struct AAudioContextBuffer {
    uint8_t *ptr;
    size_t length;
    size_t len;
    size_t pos;
};

/// If no capacity is given, it will default to `one`
///
/// return less than zero indicate errors
AAudioVector *a_audio_vector_alloc(size_t capacity);
void a_audio_vector_free(AAudioVector *au_vec);

/// @todo IMPLEMENT
/// return less than zero indicate errors
int a_audio_vector_push(AAudioVector *au_vec, const uint8_t *buffer,
                        size_t size);

/// return less than zero indicate errors
int a_audio_vector_init(AAudioVector **au_vec, uint8_t *data, size_t size);

AAudioContextBuffer *a_audio_vector_as_buffer(AAudioVector *au_vec);

#endif

#ifndef _AUDIO_AUDIO_H
#define _AUDIO_AUDIO_H

typedef struct AAudioContext AAudioContext;
struct AAudioContext {
    AAudioContextBuffer *au_buf;
    AAudioVector *au_vec;
    AVFormatContext *fmt_ctx;
    AVCodecContext *codec_ctx;
    int stream_index;
};

/// a_audio_data_free() to free the struct completely
///
/// return `NULL` if allocation fails
AAudioContext *a_audio_data_alloc();
void a_audio_data_free(AAudioContext *au_data);

/// `in_data` raw file data
///
/// `in_size` size in bytes of the file data
///
/// return less than zero indicate errors
int a_audio_context_init(uint8_t *in_data, size_t in_size,
                         AAudioContext **audio_ctx);

#endif

#ifndef _AUDIO_DECODE_H
#define _AUDIO_DECODE_H

typedef struct {
    size_t packets;
    size_t total_packets;
    size_t lost_packets;
} DecoderStats;

/// a_audio_context_free() to free the struct completely
///
/// return `NULL` if allocation fails
AAudioContext *a_audio_alloc_context();
void a_audio_free_context(AAudioContext *audio_ctx);

/// return more than zero are the size of the output data,
/// return less than zero indicate errors
int a_audio_decode(AAudioContext *au_ctx, AAudioVector **au_vec,
                   DecoderStats *dstats);
/// return more than zero are the size of the output data,
/// return less than zero indicate errors
int a_resample(uint8_t *in_data, uint8_t *out_data);

#endif

#ifndef _AUDIO_PLAYBACK_H
#define _AUDIO_PLAYBACK_H

typedef struct APlaybackFeed APlaybackFeed;
struct APlaybackFeed {
    ma_device *device;
    AAudioVector *data;
    float volume;
    int channels;
    int samples_played;
    int sample_rate;
    int paused;
};

APlaybackFeed *a_playback_feed_alloc();
void a_playback_feed_free(APlaybackFeed *playback_feed);
T_CODE a_playback_feed_init(APlaybackFeed **pb, AAudioVector *au_vec,
                            int sample_rate, int channels);

T_CODE a_playback(APlaybackFeed *playback_feed);
void a_pause(APlaybackFeed *playback_feed);
void a_play(APlaybackFeed *playback_feed);

/**
 * The function also handles two common cases:
 *
 * - If `seconds` is negative, it is set to `0`.
 * - If `seconds` exceeds `a_get_duration`, it is set to `a_get_duration`.
 */
void a_set_current_time(APlaybackFeed *playback_feed, float seconds);
float a_get_current_time(APlaybackFeed *playback_feed);
float a_get_duration(APlaybackFeed *playback_feed);
#endif
