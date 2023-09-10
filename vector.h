#ifndef __VECTOR_H__
#define __VECTOR_H__

#include "gendef.h"

typedef struct vec {
    char *ptr;
    unsigned int cap;
    unsigned int len;
    unsigned int size;
} Vec;

typedef char*(*PFN_TO_STRING)(void *, char *);

void vec_new(Vec *v, unsigned int size);
void vec_drop(Vec *v);
void vec_new_with(Vec *v, unsigned int cap, unsigned int size);
void vec_init(Vec *v, unsigned int len, unsigned int size);
void vec_resize(Vec *v, unsigned int min);
void vec_push(Vec *v, void *elem);
void vec_push_n(Vec *v, void *elems, unsigned int n);
void vec_merge(Vec *v1, Vec *v2) ;
void *vec_pop(Vec *v);
void *vec_get(Vec *v, unsigned int index);
void vec_set(Vec *v, void *elem, unsigned int index);
void vec_remove(Vec *v, unsigned int index);
void vec_pop_r(Vec *v, void *elem);
void vec_get_r(Vec *v, void *elem, unsigned int index);
unsigned int vec_len(Vec *v);
bool vec_iter(Vec *v, void *elem);
void vec_print(Vec *v, PFN_TO_STRING to_string);
void vec_dbg(Vec *v);

#define vec_iter_reset()        vec_iter(NULL, NULL)

#endif  /* __VECTOR_H__ */