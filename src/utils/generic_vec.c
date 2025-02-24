#include <errors/errors.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <utils/generic_vec.h>

Vec *vec_init(uint32_t element_size) {
    return vec_init_with_capacity(element_size, 0);
}

Vec *vec_init_with_capacity(uint32_t element_size, int capacity) {
    if (element_size < 0) {
        error_log("Could not initiate a Vec with element_size of 0");
        return NULL;
    }

    Vec *vec = malloc(sizeof(Vec));

    if (vec == NULL) {
        error_log("Could not allocate enough memory for Vec");
        return NULL;
    }

    char *data = malloc(element_size * capacity);

    if (data == NULL && capacity != 0) {
        error_log(
            "Could not allocate enough memory for Vec data with capacity");
        return NULL;
    }

    vec->data = data;
    vec->element_size = element_size;
    vec->capacity = capacity;
    vec->length = 0;

    return vec;
}

void vec_free(Vec *vec) {
    if (vec == NULL)
        return;

    if (vec->data) {
        free(vec->data);
        vec->data = NULL;
    }

    free(vec);
}

T_CODE vec_push(Vec *vec, void *data) {
    if (vec->length >= vec->capacity) {
        uint32_t capacity = vec->capacity == 0 ? 2 : vec->capacity * 1.5;

        uint8_t *new_data = realloc(vec->data, capacity * vec->element_size);

        if (new_data == NULL) {
            error_log("Could not reallocate more space for vec");
            return T_FAIL;
        }

        vec->data = new_data;
        vec->capacity = capacity;
    }

    memcpy(((uint8_t *)vec->data) + (vec->length * vec->element_size),
           (const void *)data, vec->element_size);

    vec->length++;

    return T_SUCCESS;
}

void *vec_get(Vec *vec, uint32_t index) {
    if (index >= vec->length)
        return NULL;

    return vec->data + index * vec->element_size;
};

void *vec_get_ref(Vec *vec, uint32_t index) {
    return *(void **)vec_get(vec, index);
};

void vec_set(Vec *vec, void *data, int index) {
    memcpy(vec->data + index * vec->element_size, data, vec->element_size);
}

void vec_join(Vec *vec, Vec *vec2) {
    if (vec->element_size != vec2->element_size)
        return;

    if (vec->length + vec2->length > vec->capacity) {
        uint8_t *data = realloc(vec->data, (vec->capacity + vec2->length) *
                                               vec->element_size);

        if (data == NULL)
            return;

        vec->data = data;
        vec->capacity = vec->capacity + vec2->length;
    }

    memcpy(vec->data + (vec->length * vec->element_size), vec2->data,
           vec2->length * vec2->element_size);

    vec->length += vec2->length;
}

void vec_copy(Vec *vec, Vec **vec2) {
    *vec2 = vec_init_with_capacity(vec->element_size, vec->length);

    memcpy(vec->data, (*vec2)->data, vec->element_size * vec->length);
}

void *vec_pop(Vec *vec) {
    void *data = vec_get(vec, vec->length - 1);
    vec->length -= 1;
    return data;
}

void *vec_pop_ref(Vec *vec) { return *(void **)vec_pop(vec); }

void vec_n_split(Vec *vec, Vec **vecs, int n) {
    int capacity = vec->length / n;
    int offset = 0;

    for (int i = 0; i < n; i++) {
        if (i == n - 1) {
            capacity = vec->length - i * capacity;
        }

        Vec *dest_vec = vec_init_with_capacity(vec->element_size, capacity);

        memcpy(dest_vec->data, vec->data + (offset * vec->element_size),
               vec->element_size * capacity);

        dest_vec->length = capacity;
        offset += capacity;

        vecs[i] = dest_vec;
    }
}

void vec_n_split_vec(Vec *vec, Vec **vecs, int n) {
    Vec *out = vec_init_with_capacity(vec->element_size, n);

    Vec **svecs = malloc(sizeof(Vec *) * n);
    vec_n_split(vec, svecs, n);

    for (int i = 0; i < n; i++) {
        Vec *y = svecs[i];
        vec_push(out, &y);
    }

    *vecs = out;
}
