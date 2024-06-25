#include <stdlib.h>
#include <string.h>
#include "vstr.h"

#define GROWTH_FACTOR   (2UL)

/* ======================================================================================== */
/*                                     PRIVATE METHODS                                      */
/* ======================================================================================== */

inline static void vs_alloc(Vstr *s, size_t nbytes)
{
    s->ptr = malloc(nbytes);
    s->cap = nbytes;
}

inline static void vs_realloc(Vstr *s, size_t nbytes)
{
    s->ptr = realloc(s->ptr, nbytes);
    s->cap = nbytes;
}

/**
 * @brief resize Vstr.
 * 
 * if shrink, realloc by exact number
 * if grow  , realloc by GROWTH_FACTOR when possible, otherwise exact number
 * 
 * @param s Vstr
 * @param nbytes number of bytes required
 */
static void vs_resize(Vstr *s, size_t nbytes) 
{
    if (s->cap) {
        if (nbytes < s->cap || nbytes > s->cap * GROWTH_FACTOR)
            vs_realloc(s, nbytes);
        else if (nbytes > s->cap)
            vs_realloc(s, s->cap * GROWTH_FACTOR);
    } else {
        vs_alloc(s, nbytes > GROWTH_FACTOR ? nbytes : GROWTH_FACTOR);
    }
}

/* ======================================================================================== */
/*                                      PUBLIC METHODS                                      */
/* ======================================================================================== */

void vstr_new(Vstr *s) 
{
    s->cap = 0;
    s->len = 0;
}

void vstr_new_with(Vstr *s, size_t len)
{
    vstr_new(s);
    vstr_reserve(s, len);
    vstr_cpy(s, "");
}

void vstr_from(Vstr *s, const char *source)
{
    vstr_new(s);
    vstr_cpy(s, source);
}

void vstr_clear(Vstr *s) 
{
    if (s->cap)
        free(s->ptr);
    s->cap = 0;
    s->len = 0;
}

void vstr_reserve(Vstr *s, size_t len)
{
    if (len + 1 > s->cap)
        vs_resize(s, len + 1);
}

void vstr_shrink_to_fit(Vstr *s)
{
    if (s->cap > s->len + 1)
        vs_resize(s, s->len + 1);
}

char *vstr_cpy(Vstr *dest, const char *source) 
{
    dest->len = strlen(source);
    vstr_reserve(dest, dest->len);
    return strcpy(dest->ptr, source);
}

char *vstr_ncpy(Vstr *dest, const char *source, size_t num) 
{
    dest->len = strlen(source);
    if (dest->len > num)
        dest->len = num;
    vstr_reserve(dest, dest->len);
    dest->ptr[dest->len] = '\0';
    return strncpy(dest->ptr, source, dest->len);
}

char *vstr_cat(Vstr *dest, const char *source) 
{
    dest->len += strlen(source);
    vstr_reserve(dest, dest->len);
    return strcat(dest->ptr, source);
}

char *vstr_ncat(Vstr *dest, const char *source, size_t num) 
{
    size_t len_src;
    
    len_src = strlen(source);
    if (len_src < num)
        num = len_src;
    dest->len += num;
    vstr_reserve(dest, dest->len);
    dest->ptr[dest->len] = '\0';
    return strncat(dest->ptr, source, num);
}

char *vstr_merge(Vstr *dest, Vstr *source, const char *sep) 
{
    if (source->len) {
        dest->len += source->len + strlen(sep);
        vstr_reserve(dest, dest->len);
        strcat(strcat(dest->ptr, sep), source->ptr);
    }
    vstr_clear(source);
    return vstr_c_str(dest);
}