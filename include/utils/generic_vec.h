#ifndef _UTILS_GENERIC_VEC_H
#define _UTILS_GENERIC_VEC_H
#include <errors/errors.h>
#include <stdint.h>

/**
 * Represents a growable vector.
 *
 * Can be initiated with @ref vec_init and it can be freed with @ref vec_free.
 *
 * Any allocated memory pushed with @ref vec_push will not be freed by @ref
 * vec_free.
 *
 * @class Vec
 */
typedef struct {
    uint8_t *data;
    uint32_t element_size;
    uint32_t capacity;
    uint32_t length;
} Vec;

/// @returns `NULL` if `element_size` equals ZERO
Vec *vec_init(uint32_t element_size);
Vec *vec_init_with_capacity(uint32_t element_size, int capacity);

/**
 * @memberof Vec
 */
void vec_free(Vec *vec);

/**
 * @memberof Vec
 */
T_CODE vec_push(Vec *vec, void *data);

/**
 * @memberof Vec
 */
void *vec_get(Vec *vec, uint32_t index);

/**
 * Use if your vector contains pointers.
 * @memberof Vec
 */
void *vec_get_ref(Vec *vec, uint32_t index);

/**
 * @memberof Vec
 */
void vec_set(Vec *vec, void *data, int index);

/**
 * @brief `vec->element_size == vec2->element_size` must be true
 * @memberof Vec
 */
void vec_join(Vec *vec, Vec *vec2);

/**
 * @memberof Vec
 */
void vec_copy(Vec *vec, Vec **vec2);

/**
 * @memberof Vec
 */
void *vec_pop(Vec *vec);

/**
 * @memberof Vec
 */
void *vec_pop_ref(Vec *vec);

/**
 * @memberof Vec
 */
void vec_n_split_vec(Vec *vec, Vec **vecs, int n);

/**
 * @memberof Vec
 */
void vec_remove(Vec *vec, int n);

#endif
