#include "sstring.h"

#include <stdlib.h>
#include <string.h>

#define GROWTH_FACTOR (2UL)

/********************************************************************************************
 *                                     PRIVATE METHODS                                      *
 ********************************************************************************************/

inline static void s_alloc(SString *s, size_t nbytes) {
    s->ptr = malloc(nbytes);
    s->cap = nbytes;
}

inline static void s_realloc(SString *s, size_t nbytes) {
    s->ptr = realloc(s->ptr, nbytes);
    s->cap = nbytes;
}

/**
 * @brief resize SString.
 * 
 * if shrink, realloc by exact number
 * if grow  , realloc by GROWTH_FACTOR when possible, otherwise exact number
 * 
 * @param s SString
 * @param nbytes number of bytes required
 */
static void SString_resize(SString *s, size_t nbytes) {
    if (s->cap) {
        if (nbytes < s->cap || nbytes > s->cap * GROWTH_FACTOR)
            s_realloc(s, nbytes);
        else if (nbytes > s->cap)
            s_realloc(s, s->cap * GROWTH_FACTOR);
    } else {
        s_alloc(s, nbytes > GROWTH_FACTOR ? nbytes : GROWTH_FACTOR);
    }
}

/********************************************************************************************
 *                                      PUBLIC METHODS                                      *
 ********************************************************************************************/

void SString_new(SString *s) {
    s->cap = 0;
    s->len = 0;
}

void SString_new_with(SString *s, size_t len) {
    SString_new(s);
    SString_reserve(s, len);
    SString_cpy(s, "");
}

void SString_from(SString *s, const char *source) {
    SString_new(s);
    SString_cpy(s, source);
}

void SString_free(SString *s) {
    if (s->cap)
        free(s->ptr);
    s->cap = 0;
    s->len = 0;
}

void SString_reserve(SString *s, size_t len) {
    if (len + 1 > s->cap)
        SString_resize(s, len + 1);
}

void SString_shrink_to_fit(SString *s) {
    if (s->cap > s->len + 1)
        SString_resize(s, s->len + 1);
}

char *SString_cpy(SString *dest, const char *source) {
    dest->len = strlen(source);
    SString_reserve(dest, dest->len);
    return strcpy(dest->ptr, source);
}

char *SString_ncpy(SString *dest, const char *source, size_t num) {
    dest->len = strlen(source);
    if (dest->len > num)
        dest->len = num;
    SString_reserve(dest, dest->len);
    dest->ptr[dest->len] = '\0';
    return strncpy(dest->ptr, source, dest->len);
}

char *SString_cat(SString *dest, const char *source) {
    dest->len += strlen(source);
    SString_reserve(dest, dest->len);
    return strcat(dest->ptr, source);
}

char *SString_ncat(SString *dest, const char *source, size_t num) {
    size_t len_src;

    len_src = strlen(source);
    if (len_src < num)
        num = len_src;
    dest->len += num;
    SString_reserve(dest, dest->len);
    dest->ptr[dest->len] = '\0';
    return strncat(dest->ptr, source, num);
}

char *SString_merge(SString *dest, SString *source, const char *sep) {
    if (source->len) {
        dest->len += source->len + strlen(sep);
        SString_reserve(dest, dest->len);
        strcat(strcat(dest->ptr, sep), source->ptr);
    }
    SString_free(source);
    return dest->ptr;
}