#ifndef __VECTOR_H__
#define __VECTOR_H__

#include <stdbool.h>

typedef struct {
    char *ptr;      // underlying data (if needed, access through vec_ptr())
    size_t cap;     // number of elements for which there is space allocated
    size_t len;     // number of usable elements
    size_t szof;    // sizeof() of the data type to be held
} Vec;

void  vec_new            (Vec *v, size_t szof);
void  vec_new_with       (Vec *v, size_t szof, size_t nelem);
void  vec_new_with_zeroed(Vec *v, size_t szof, size_t nelem);
void  vec_from           (Vec *v, size_t szof, void *arr, size_t nelem);
void  vec_clear          (Vec *v);
void  vec_reserve        (Vec *v, size_t nelem);
void  vec_shrink_to_fit  (Vec *v);
void *vec_elem_at        (Vec *v, size_t pos);
void  vec_push           (Vec *v, void *elem);
void  vec_insert         (Vec *v, void *elem, size_t pos);
void  vec_insert_n       (Vec *v, void *elems, size_t nelem, size_t pos);
void  vec_pop            (Vec *v, void *elem);
void  vec_remove         (Vec *v, size_t pos, void *elem);
void  vec_remove_n       (Vec *v, size_t pos, void *elems, size_t nelem);
void  vec_get            (Vec *v, size_t pos, void *elem);
void  vec_set            (Vec *v, void *elem, size_t pos);
void  vec_swap           (Vec *v, size_t pos1, size_t pos2, void *tmp);
void  vec_sort           (Vec *v, int order);
bool  vec_iter           (Vec *v, void *elem);

/* returns pointer to the underlying array data, or NULL if empty
 * if changes to the Vec are made, this pointer can become invalid */
inline void *vec_ptr(Vec *v)
{
    if (v->len)
        return (void *)v->ptr;
    return NULL;
}

inline bool vec_is_empty(Vec *v) 
{
    return v->len == 0;
}

#define VEC_ORDER_ASC       1   // sort in ascending order
#define VEC_ORDER_DESC     -1   // sort in descending order

#define vec_iter_reset()    vec_iter(NULL, NULL)

#endif  /* __VECTOR_H__ */