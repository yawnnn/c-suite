#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "vector.h"

#if __DBG__
#define __VEC_DEBUG_MODE 1
#else
#define __VEC_DEBUG_MODE 0
#endif

#define CRED     "\x1b[31m"
#define CGREEN   "\x1b[32m"
#define CYELLOW  "\x1b[33m"
#define CBLUE    "\x1b[34m"
#define CMAGENTA "\x1b[35m"
#define CCYAN    "\x1b[36m"
#define CRESET   "\x1b[0m"

const char max_depth_tabs[20] = "\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t";
short int __dbg_depth = 0;

#define __DBG_PRINT_BEFORE(v)                                   \
if (__VEC_DEBUG_MODE) {                                         \
    printf("%.*s", __dbg_depth++, max_depth_tabs);              \
    printf(CRED "DBG(" CGREEN "before" CRESET CRED ")" CRESET ": In " CYELLOW "%s" CRESET "(). ", __func__); \
    vec_dbg(v);                                                 \
}                                                               \

#define __DBG_PRINT_AFTER(v)                                    \
if (__VEC_DEBUG_MODE) {                                         \
    printf("%.*s", --__dbg_depth, max_depth_tabs);              \
    printf(CRED "DBG(" CMAGENTA "after" CRESET CRED ")" CRESET ": In " CYELLOW "%s" CRESET "(). ", __func__); \
    vec_dbg(v);                                                 \
}    

#define GROWTH_FACTOR   (2UL)

//#define v_set(v, n)         memset((v)->ptr, 0, (n) * (v)->size)                /* memset */
//#define v_cpy(v, e)         memcpy((v)->ptr, (e), (v)->size)                    /* memcpy */
//#define v_alloc(v, n)       (v)->ptr = malloc((n) * (v)->size)             /* malloc */
//#define v_realloc(v, n)     (v)->ptr = realloc((v)->ptr, (n) * (v)->size)  /* realloc */

static char *v_at(Vec *v, unsigned int index)
{
    return v->ptr + (index * v->size);
}

static void v_at_r(Vec *v, unsigned int index, void *elem)
{
    memcpy(elem, v->ptr + (index * v->size), v->size);
}

static void v_set(Vec *v, unsigned int index, unsigned int n)
{
    memset(v_at(v, index), 0, n * v->size);
}

static void v_cpy(Vec *v, unsigned int index, void *elems, unsigned int n)
{
    memcpy(v_at(v, index), elems, n * v->size);
}

static void v_move(Vec *v, unsigned int index, void *elems, unsigned int n)
{
    memmove(v_at(v, index), elems, n * v->size);
}

static void v_alloc(Vec *v, unsigned int n)
{
    v->ptr = malloc(n * v->size);
}

static void v_realloc(Vec *v, unsigned int n)
{
    v->ptr = realloc(v->ptr, n * v->size);
}

static void vec_set_capacity(Vec *v, unsigned int cap) 
{
    __DBG_PRINT_BEFORE(v);
    if (v->cap == 0)
        v_alloc(v, cap);
    else
        v_realloc(v, cap);

    v->cap = cap;
    __DBG_PRINT_AFTER(v);
}

static void vec_grow(Vec *v) 
{
    __DBG_PRINT_BEFORE(v);
    if (v->cap == 0) {
        v_alloc(v, 1);
        v->cap = 1;
    } else {
        v_realloc(v, v->cap * GROWTH_FACTOR);
        v->cap *= GROWTH_FACTOR;
    }
    __DBG_PRINT_AFTER(v);
}

void vec_new(Vec *v, unsigned int size) 
{
    v->ptr = NULL;
    v->cap = 0;
    v->len = 0;
    v->size = size;
}

void vec_drop(Vec *v) 
{
    v->cap = 0;
    v->len = 0;
    if (v->cap)
        free(v->ptr);
}

void vec_new_with(Vec *v, unsigned int cap, unsigned int size)
{
    v->cap = 0;
    v->len = 0;
    v->size = size;
    vec_set_capacity(v, cap);
}

void vec_init(Vec *v, unsigned int len, unsigned int size)
{
    vec_new_with(v, len, size);
    v_set(v, 0, len);
    v->len = len;
}

void vec_resize(Vec *v, unsigned int min) 
{
    __DBG_PRINT_BEFORE(v);
    if (v->cap > min || v->cap * GROWTH_FACTOR < min)
        vec_set_capacity(v, min);
    else if (v->cap < min)
        vec_grow(v);
    __DBG_PRINT_AFTER(v);
}

void vec_push(Vec *v, void *elem) 
{
    __DBG_PRINT_BEFORE(v);
    if (v->len == v->cap)
        vec_grow(v);

    v_cpy(v, v->len++, elem, 1);
    __DBG_PRINT_AFTER(v);
}

void vec_push_n(Vec *v, void *elems, unsigned int n) 
{
    __DBG_PRINT_BEFORE(v);
    if (v->len + n > v->cap)
        vec_resize(v, v->len + n);

    v_cpy(v, v->len, elems, n);
    v->len += n;
    __DBG_PRINT_AFTER(v);
}

void vec_merge(Vec *v1, Vec *v2) 
{
    __DBG_PRINT_BEFORE(v1);
    vec_push_n(v1, v2->ptr, v2->len);
    vec_drop(v2);
    __DBG_PRINT_AFTER(v1);
}

void *vec_pop(Vec *v) 
{
    __DBG_PRINT_BEFORE(v);
    if (v->len != 0)
        return v_at(v, --(v->len));
    return NULL;
    __DBG_PRINT_AFTER(v);
}

void *vec_get(Vec *v, unsigned int index)
{
    if (index < v->len)
        return (void *)v_at(v, index);
    return NULL;
}

void vec_set(Vec *v, void *elem, unsigned int index) 
{
    __DBG_PRINT_BEFORE(v);
    if (index < v->len)
        v_cpy(v, index, elem, 1);
    __DBG_PRINT_AFTER(v);
}

void vec_remove(Vec *v, unsigned int index) 
{
    __DBG_PRINT_BEFORE(v);
    if (v->len != 0) {
        if (index + 1 < v->len)
            v_move(v, index, v_at(v, index + 1), v->len - (index + 1));
        v->len--;
    }
    __DBG_PRINT_AFTER(v);
}

void vec_pop_r(Vec *v, void *elem) 
{
    __DBG_PRINT_BEFORE(v);
    if (v->len != 0)
        v_at_r(v, v->len--, elem);
    __DBG_PRINT_AFTER(v);
}

void vec_get_r(Vec *v, void *elem, unsigned int index) 
{
    if (index < v->len)
        v_at_r(v, index, elem);
}

unsigned int vec_len(Vec *v) 
{
    return v->len;
}

bool vec_iter(Vec *v, void *elem) 
{
    static unsigned int i = 0;

    if (v) {
        if (i < v->len) {
            vec_get_r(v, elem, i++);
            return YES;
        }
    } else {
        i = 0;
    }

    return NO;
}

void vec_print(Vec *v, PFN_TO_STRING pfn_to_string)
{
    void *pval;
    char val[32];
    char *buf = NULL;
    bool alloc;

    if (v->len == 0)
        return;

    if (v->size > sizeof(val)) {
        pval = malloc(v->size);
        alloc = YES;
    }
    else {
        pval = val;
        alloc = NO;
    }

    vec_iter_reset();
    while (vec_iter(v, pval))
        printf("%s ", pfn_to_string ? pfn_to_string(pval, buf) : *((char **)pval));
    printf("\n");
    
    if (alloc)
        free(pval);
    if (pfn_to_string)
        free(buf);
}

void vec_dbg(Vec *v)
{
    printf("Vec => ptr: " CCYAN "%p" CRESET ", cap: %u, len: %u, size: %u\n", v->ptr, v->cap, v->len, v->size);
}