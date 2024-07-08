#include <stdlib.h>
#include <string.h>

#include "vector.h"

#define GROWTH_FACTOR (2UL)

/********************************************************************************************
 *                                     PRIVATE METHODS                                      *
 ********************************************************************************************/

static inline char *Vector_ptr(Vector *v, size_t pos) {
    return ((char *)v->ptr) + (pos * v->szof);
}

static inline void Vector_alloc(Vector *v, size_t nelem) {
    v->ptr = malloc(nelem * v->szof);
    v->cap = nelem;
}

static inline void Vector_realloc(Vector *v, size_t nelem) {
    v->ptr = realloc(v->ptr, nelem * v->szof);
    v->cap = nelem;
}

/**
 * @brief resize Vector.
 * 
 * if shrink, realloc by exact number
 * if grow  , realloc by GROWTH_FACTOR when possible, otherwise exact number
 * 
 * @param s Vstr
 * @param nelem number of elements requested
 */
static void Vector_resize(Vector *v, size_t nelem) {
    if (v->cap) {
        if (nelem < v->cap || nelem > v->cap * GROWTH_FACTOR)
            Vector_realloc(v, nelem);
        else if (nelem > v->cap)
            Vector_realloc(v, v->cap * GROWTH_FACTOR);
    } else {
        Vector_alloc(v, nelem > GROWTH_FACTOR ? nelem : GROWTH_FACTOR);
    }
}

/********************************************************************************************
 *                                      PUBLIC METHODS                                      *
 ********************************************************************************************/

void Vector_new(Vector *v, size_t szof) {
    v->cap = 0;
    v->len = 0;
    v->szof = szof;
}

void Vector_new_with(Vector *v, size_t szof, size_t nelem) {
    Vector_new(v, szof);
    Vector_reserve(v, nelem);
}

void Vector_new_with_zeroed(Vector *v, size_t szof, size_t nelem) {
    Vector_new_with(v, nelem, szof);
    Vector_memset(v, v->ptr, 0, nelem);
    v->len = nelem;
}

void Vector_from(Vector *v, size_t szof, void *arr, size_t nelem) {
    Vector_new(v, szof);
    Vector_insert_n(v, arr, 0, nelem);
}

void Vector_free(Vector *v) {
    if (v->cap)
        free(v->ptr);
    v->cap = 0;
    v->len = 0;
}

void Vector_reserve(Vector *v, size_t nelem) {
    if (nelem > v->cap)
        Vector_resize(v, nelem);
}

void Vector_shrink_to_fit(Vector *v) {
    if (v->cap > v->len) {
        if (v->len)
            Vector_resize(v, v->len);
        else
            Vector_free(v);
    }
}

void Vector_get(Vector *v, size_t pos, void *elem) {
    if (pos < v->len)
        Vector_memcpy(v, elem, Vector_ptr(v, pos), 1);
}

void Vector_set(Vector *v, void *elem, size_t pos) {
    if (pos < v->len)
        Vector_memcpy(v, Vector_ptr(v, pos), elem, 1);
}

void Vector_insert_n(Vector *v, void *elems, size_t nelem, size_t pos) {
    if (pos <= v->len) {
        Vector_reserve(v, v->len + nelem);
        Vector_memmove(v, Vector_ptr(v, pos + nelem), Vector_ptr(v, pos), v->len - pos);
        Vector_memcpy(v, Vector_ptr(v, pos), elems, nelem);
        v->len += nelem;
    }
}

void Vector_remove_n(Vector *v, size_t pos, void *elems, size_t nelem) {
    if (pos + nelem - 1 < v->len) {
        if (elems)
            Vector_memcpy(v, elems, Vector_ptr(v, pos), nelem);
        if (pos + nelem < v->len)
            Vector_memmove(
                v,
                Vector_ptr(v, pos),
                Vector_ptr(v, pos + nelem),
                v->len - (pos + nelem)
            );
        v->len -= nelem;
    }
}

void Vector_swap(Vector *v, size_t pos1, size_t pos2, void *tmp) {
    if (pos1 < v->len && pos2 < v->len) {
        Vector_memcpy(v, tmp                , Vector_ptr(v, pos1), 1);
        Vector_memcpy(v, Vector_ptr(v, pos1), Vector_ptr(v, pos2), 1);
        Vector_memcpy(v, Vector_ptr(v, pos2), tmp                , 1);
    }
}