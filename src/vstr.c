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

/* if shrink, realloc by exact number
 * if grow  , realloc by GROWTH_FACTOR when possible, otherwise exact number */
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

/* initializes the string, the string is not allocated and therefore unusable */
void vstr_new(Vstr *s) 
{
    s->cap = 0;
    s->len = 0;
}

/* new string with space for at most a string of length <len>. the string has 0 length but is usable */
void vstr_init(Vstr *s, size_t len)
{
    vstr_new(s);
    vstr_reserve(s, len);
    vstr_cpy(s, "");
}

/* new string from c-style string */
void vstr_from(Vstr *s, const char *source)
{
    vstr_new(s);
    vstr_cpy(s, source);
}

/* clear variables, release memory */
void vstr_clear(Vstr *s) 
{
    if (s->cap)
        free(s->ptr);
    s->cap = 0;
    s->len = 0;
}

/* reserve memory for a string of at least <len> characters */
void vstr_reserve(Vstr *s, size_t len)
{
    if (len + 1 > s->cap)
        vs_resize(s, len + 1);
}

/* ensure the memory allocated is exactly as needed for the length */
void vstr_shrink_to_fit(Vstr *s)
{
    if (s->cap > s->len + 1)
        vs_resize(s, s->len + 1);
}

/* strcpy of <source>
 * allocates memory as needed */
char *vstr_cpy(Vstr *dest, const char *source) 
{
    dest->len = strlen(source);
    vstr_reserve(dest, dest->len);
    return strcpy(dest->ptr, source);
}

/* strncpy of <source> of at most <num> characters
 * allocates memory as needed */
char *vstr_ncpy(Vstr *dest, const char *source, size_t num) 
{
    dest->len = strlen(source);
    if (dest->len > num)
        dest->len = num;
    vstr_reserve(dest, dest->len);
    dest->ptr[dest->len] = '\0';
    return strncpy(dest->ptr, source, dest->len);
}

/* strcat of <source>
 * allocates memory as needed */
char *vstr_cat(Vstr *dest, const char *source) 
{
    dest->len += strlen(source);
    vstr_reserve(dest, dest->len);
    return strcat(dest->ptr, source);
}

/* strncat of <source> of at most <num> characters
 * allocates memory as needed */
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

/* merge <s2> into <s1> with c-string <sep> in between. consumes <s2> */
char *vstr_merge(Vstr *s1, Vstr *s2, const char *sep) 
{
    if (s2->len) {
        s1->len += s2->len + strlen(sep);
        vstr_reserve(s1, s1->len);
        strcat(strcat(s1->ptr, sep), s2->ptr);
    }
    vstr_clear(s2);
    return vstr_c_str(s1);
}