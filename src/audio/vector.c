#include <audio/audio.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

AAudioVector *a_audio_vector_alloc(size_t capacity) {
    AAudioVector *au_vec = malloc(sizeof(AAudioVector));

    if (!au_vec) {
        return NULL;
    }

    au_vec->ptr = calloc(capacity, sizeof(uint8_t));
    au_vec->capacity = capacity;
    au_vec->length = 0;

    return au_vec;
}

void a_audio_vector_free(AAudioVector *au_vec) {
    if (!au_vec)
        return;

    if (au_vec->ptr != NULL)
        free(au_vec->ptr);

    free(au_vec);
}

int a_audio_vector_init(AAudioVector **au_vec, uint8_t *data, size_t size) {
    AAudioVector *audio_vec = malloc(sizeof(AAudioVector));

    if (!au_vec)
        return -1;

    audio_vec->ptr = data;
    audio_vec->capacity = size;
    audio_vec->length = size;

    *au_vec = audio_vec;
    return 0;
}

int a_audio_vector_push(AAudioVector *au_vec, const uint8_t *data,
                        size_t size) {
    if (!au_vec || !data || size == 0)
        return -1;

    if (au_vec->length + size > au_vec->capacity) {
        size_t nwcap = au_vec->capacity == 0 ? size : au_vec->capacity * 1.5;

        if (nwcap < au_vec->length + size)
            nwcap = au_vec->length + size;

        uint8_t *n_ptr = realloc(au_vec->ptr, nwcap * sizeof(uint8_t));
        if (!n_ptr)
            return -1;

        au_vec->ptr = n_ptr;
        au_vec->capacity = nwcap;
    }

    memcpy(au_vec->ptr + au_vec->length, data, size);
    au_vec->length += size;

    return 0;
}

AAudioContextBuffer *a_audio_vector_as_buffer(AAudioVector *au_vec) {
    AAudioContextBuffer *au_buf = malloc(sizeof(AAudioContextBuffer));
    if (!au_buf)
        return NULL;

    au_buf->ptr = au_vec->ptr;
    au_buf->length = au_vec->length;

    return au_buf;
}
