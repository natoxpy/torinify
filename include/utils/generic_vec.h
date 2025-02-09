#ifndef _UTILS_GENERIC_VEC_H
#define _UTILS_GENERIC_VEC_H
#include <errors/errors.h>
#include <stdint.h>

typedef struct {
    void *data;
    uint32_t element_size;
    uint32_t capacity;
    uint32_t length;
} Vec;

/// returns `NULL` if `element_size` equals ZERO
Vec *vec_init(uint32_t element_size);
Vec *vec_init_with_capacity(uint32_t element_size, int capacity);

void vec_free(Vec *vec);
T_CODE vec_push(Vec *vec, void *data);
void *vec_get(Vec *vec, uint32_t index);

void vec_set(Vec *vec, void *data, int index);

/// `vec->element_size == vec2->element_size` must be true
void vec_join(Vec *vec, Vec *vec2);
void vec_n_split(Vec *vec, Vec **vecs, int n);

#endif
