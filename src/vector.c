#include <stdlib.h>
#include <string.h>
#include "vector.h"

#define GROWTH_FACTOR   (2UL)

/* ======================================================================================== */
/*                                     PRIVATE METHODS                                      */
/* ======================================================================================== */

inline static char *v_elem_at(Vec *v, size_t pos)
{
    return v->ptr + (pos * v->szof);
}

inline static void v_memset(Vec *v, size_t pos, size_t nelem)
{
    memset(v_elem_at(v, pos), 0, nelem * v->szof);
}

inline static void v_memcpy(Vec *v, size_t pos, void *source, size_t nelem)
{
    memcpy(v_elem_at(v, pos), source, nelem * v->szof);
}

inline static void v_memcpy_to(void *dest, Vec *v, size_t pos, size_t nelem)
{
    memcpy(dest, v_elem_at(v, pos), nelem * v->szof);
}

inline static void v_memmove(Vec *v, size_t pos, void *source, size_t nelem)
{
    memmove(v_elem_at(v, pos), source, nelem * v->szof);
}

inline static int v_memcmp(Vec *v, size_t pos, void *ptr2, size_t nelem)
{
    return memcmp(v_elem_at(v, pos), ptr2, nelem * v->szof);
}

inline static void v_alloc(Vec *v, size_t nelem)
{
    v->ptr = malloc(nelem * v->szof);
    v->cap = nelem;
}

inline static void v_realloc(Vec *v, size_t nelem)
{
    v->ptr = realloc(v->ptr, nelem * v->szof);
    v->cap = nelem;
}

/**
 * @brief resize Vec.
 * 
 * if shrink, realloc by exact number
 * if grow  , realloc by GROWTH_FACTOR when possible, otherwise exact number
 * 
 * @param s Vstr
 * @param nelem number of elements requested
 */
static void v_resize(Vec *v, size_t nelem) 
{
    if (v->cap) {
        if (nelem < v->cap || nelem > v->cap * GROWTH_FACTOR)
            v_realloc(v, nelem);
        else if (nelem > v->cap)
            v_realloc(v, v->cap * GROWTH_FACTOR);
    }
    else {
        v_alloc(v, nelem > GROWTH_FACTOR ? nelem : GROWTH_FACTOR);
    }
}

/* ======================================================================================== */
/*                                      PUBLIC METHODS                                      */
/* ======================================================================================== */

void vec_new(Vec *v, size_t szof) 
{
    v->cap = 0;
    v->len = 0;
    v->szof = szof;
}

void vec_new_with(Vec *v, size_t szof, size_t nelem)
{
    vec_new(v, szof);
    vec_reserve(v, nelem);
}

void vec_new_with_zeroed(Vec *v, size_t szof, size_t nelem)
{
    vec_new_with(v, nelem, szof);
    v_memset(v, 0, nelem);
    v->len = nelem;
}

void vec_from(Vec *v, size_t szof, void *arr, size_t nelem)
{
    vec_new(v, szof);
    vec_insert_n(v, arr, 0, nelem);
}

void vec_clear(Vec *v) 
{
    if (v->cap)
        free(v->ptr);
    v->cap = 0;
    v->len = 0;
}

void vec_reserve(Vec *v, size_t nelem)
{
    if (nelem > v->cap)
        v_resize(v, nelem);
}

void vec_shrink_to_fit(Vec *v)
{
    if (v->cap > v->len)
        v_resize(v, v->len);
}

void *vec_elem_at(Vec *v, size_t pos)
{
    if (pos < v->len)
        return v_elem_at(v, pos);
    return NULL;
}

void vec_get(Vec *v, size_t pos, void *elem)
{
    if (pos < v->len)
        v_memcpy_to(elem, v, pos, 1);
}

void vec_set(Vec *v, void *elem, size_t pos) 
{
    if (pos < v->len)
        v_memcpy(v, pos, elem, 1);
}

void vec_push(Vec *v, void *elem) 
{
    vec_insert_n(v, elem, 1, v->len);
}

void vec_insert(Vec *v, void *elem, size_t pos)
{
    vec_insert_n(v, elem, 1, pos);
}

void vec_insert_n(Vec *v, void *elems, size_t nelem, size_t pos)
{
    if (pos <= v->len) {
        vec_reserve(v, v->len + nelem);
        v_memmove(v, pos + nelem, v_elem_at(v, pos), v->len - pos);
        v_memcpy(v, pos, elems, nelem);
        v->len += nelem;
    }
}

void vec_pop(Vec *v, void *elem) 
{
    vec_remove_n(v, v->len - 1, 1, elem);
}

void vec_remove(Vec *v, size_t pos, void *elem) 
{
    vec_remove_n(v, pos, elem, 1);
}

void vec_remove_n(Vec *v, size_t pos, void *elems, size_t nelem)
{
    if (pos + nelem - 1 < v->len) {
        if (elems)
            v_memcpy_to(elems, v, pos, nelem);
        if (pos + nelem < v->len)
            v_memmove(v, pos, v_elem_at(v, pos + nelem), v->len - (pos + nelem));
        v->len -= nelem;
    }
}

void vec_swap(Vec *v, size_t pos1, size_t pos2, void *tmp)
{
    if (pos1 < v->len && pos2 < v->len) {
        v_memcpy_to(tmp, v, pos1, 1);
        v_memcpy(v, pos1, v_elem_at(v, pos2), 1);
        v_memcpy(v, pos2, tmp, 1);
    }
}

// TODO --- proper sorting algorithm
void vec_sort(Vec *v, int order)
{
    int i, j;
    char val[256];
    void *pval;
    bool alloc;

    if (v->szof <= sizeof(val)) {
        pval = val;
        alloc = false;
    }
    else {
        pval = malloc(v->szof);
        alloc = true;
    }

    for (i = 0; i < v->len; i++) {
        for (j = i + 1; j < v->len; j++) {
            if (v_memcmp(v, i, v_elem_at(v, j), 1) * order > 0)
                vec_swap(v, i, j, pval);
        }
    }

    if (alloc)
        free(pval);
}

bool vec_iter(Vec *v, void *elem) 
{
    static size_t i = 0;

    if (v) {
        if (i < v->len) {
            vec_get(v, i++, elem);
            return true;
        }
    } else {
        i = 0;
    }

    return false;
}