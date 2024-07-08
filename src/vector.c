#include "vector.h"

#include <stdlib.h>
#include <string.h>

#define GROWTH_FACTOR (2UL)

/********************************************************************************************
 *                                     PRIVATE METHODS                                      *
 ********************************************************************************************/

inline static char *v_elem_at(Vector *v, size_t pos) {
    return ((char *)v->ptr) + (pos * v->szof);
}

inline static void v_memset(Vector *v, size_t pos, size_t nelem) {
    memset(v_elem_at(v, pos), 0, nelem * v->szof);
}

inline static void v_memcpy(Vector *v, size_t pos, void *source, size_t nelem) {
    memcpy(v_elem_at(v, pos), source, nelem * v->szof);
}

inline static void
v_memcpy_to(void *dest, Vector *v, size_t pos, size_t nelem) {
    memcpy(dest, v_elem_at(v, pos), nelem * v->szof);
}

inline static void
v_memmove(Vector *v, size_t pos, void *source, size_t nelem) {
    memmove(v_elem_at(v, pos), source, nelem * v->szof);
}

inline static int v_memcmp(Vector *v, size_t pos, void *ptr2, size_t nelem) {
    return memcmp(v_elem_at(v, pos), ptr2, nelem * v->szof);
}

inline static void v_alloc(Vector *v, size_t nelem) {
    v->ptr = malloc(nelem * v->szof);
    v->cap = nelem;
}

inline static void v_realloc(Vector *v, size_t nelem) {
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
static void v_resize(Vector *v, size_t nelem) {
    if (v->cap) {
        if (nelem < v->cap || nelem > v->cap * GROWTH_FACTOR)
            v_realloc(v, nelem);
        else if (nelem > v->cap)
            v_realloc(v, v->cap * GROWTH_FACTOR);
    } else {
        v_alloc(v, nelem > GROWTH_FACTOR ? nelem : GROWTH_FACTOR);
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
    v_memset(v, 0, nelem);
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
        v_resize(v, nelem);
}

void Vector_shrink_to_fit(Vector *v) {
    if (v->cap > v->len)
        v_resize(v, v->len);
}

void Vector_get(Vector *v, size_t pos, void *elem) {
    if (pos < v->len)
        v_memcpy_to(elem, v, pos, 1);
}

void Vector_set(Vector *v, void *elem, size_t pos) {
    if (pos < v->len)
        v_memcpy(v, pos, elem, 1);
}

void Vector_insert_n(Vector *v, void *elems, size_t nelem, size_t pos) {
    if (pos <= v->len) {
        Vector_reserve(v, v->len + nelem);
        v_memmove(v, pos + nelem, v_elem_at(v, pos), v->len - pos);
        v_memcpy(v, pos, elems, nelem);
        v->len += nelem;
    }
}

void Vector_remove_n(Vector *v, size_t pos, void *elems, size_t nelem) {
    if (pos + nelem - 1 < v->len) {
        if (elems)
            v_memcpy_to(elems, v, pos, nelem);
        if (pos + nelem < v->len)
            v_memmove(
                v,
                pos,
                v_elem_at(v, pos + nelem),
                v->len - (pos + nelem)
            );
        v->len -= nelem;
    }
}

void Vector_swap(Vector *v, size_t pos1, size_t pos2, void *tmp) {
    if (pos1 < v->len && pos2 < v->len) {
        v_memcpy_to(tmp, v, pos1, 1);
        v_memcpy(v, pos1, v_elem_at(v, pos2), 1);
        v_memcpy(v, pos2, tmp, 1);
    }
}

bool Vector_iter(Vector *v, void *elem) {
    static size_t i = 0;

    if (v) {
        if (i < v->len) {
            Vector_get(v, i++, elem);
            return true;
        }
    } else {
        i = 0;
    }

    return false;
}