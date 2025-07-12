#include <stdlib.h>
#include "vec.h"

#define GROWTH_FACTOR (2UL)

/**
 * @brief pointer to the element of @p v at @p pos, without checking the validity of the address.
 * 
 * @param[in] v Vec
 * @param[in] pos position of the element
 * 
 * @return the pointer
 */
static INLINE char *vec_ptr(Vec *v, size_t pos)
{
   return ((char *)v->ptr) + (pos * v->szof);
}

/**
 * @brief alloc Vec.
 * 
 * @param[in,out] v Vec
 * @param[in] nelem number of elements requested
 */
static INLINE void vec_alloc(Vec *v, size_t nelem)
{
   v->ptr = malloc(nelem * v->szof);
   v->cap = nelem;
}

/**
 * @brief realloc Vec.
 * 
 * @param[in,out] v Vec
 * @param[in] nelem number of elements requested
 */
static INLINE void vec_realloc(Vec *v, size_t nelem)
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
 * @param[in,out] v Vec
 * @param[in] nelem number of elements requested
 */
static void vec_resize(Vec *v, size_t nelem)
{
   if (v->cap) {
      if (nelem < v->cap || nelem > v->cap * GROWTH_FACTOR)
         vec_realloc(v, nelem);
      else if (nelem > v->cap)
         vec_realloc(v, v->cap * GROWTH_FACTOR);
   }
   else
      vec_alloc(v, nelem > GROWTH_FACTOR ? nelem : GROWTH_FACTOR);
}

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
   vec_memset(v, v->ptr, 0, nelem);
   v->len = nelem;
}

void vec_from(Vec *v, size_t szof, void *arr, size_t nelem)
{
   vec_new(v, szof);
   vec_insert_n(v, 0, arr, nelem);
}

void vec_free(Vec *v)
{
   if (v->cap)
      free(v->ptr);
   v->cap = 0;
   v->len = 0;
}

void vec_truncate(Vec *v, size_t new_len)
{
   if (new_len < v->len)
      v->len = new_len;
}

void vec_reserve(Vec *v, size_t nelem)
{
   if (nelem > v->cap)
      vec_resize(v, nelem);
}

void vec_shrink_to_fit(Vec *v)
{
   if (v->cap > v->len) {
      if (v->len)
         vec_resize(v, v->len);
      else
         vec_free(v);
   }
}

void vec_get(Vec *v, size_t pos, void *elem)
{
   if (pos < v->len)
      vec_memcpy(v, elem, vec_ptr(v, pos), 1);
}

void vec_set(Vec *v, void *elem, size_t pos)
{
   if (pos < v->len)
      vec_memcpy(v, vec_ptr(v, pos), elem, 1);
}

void vec_insert_n(Vec *v, size_t pos, void *elems, size_t nelem)
{
   if (pos <= v->len) {
      vec_reserve(v, v->len + nelem);
      vec_memmove(v, vec_ptr(v, pos + nelem), vec_ptr(v, pos), v->len - pos);
      vec_memcpy(v, vec_ptr(v, pos), elems, nelem);
      v->len += nelem;
   }
}

void vec_remove_n(Vec *v, size_t pos, void *elems, size_t nelem)
{
   if (pos + nelem - 1 < v->len) {
      if (elems)
         vec_memcpy(v, elems, vec_ptr(v, pos), nelem);
      if (pos + nelem < v->len)
         vec_memmove(v, vec_ptr(v, pos), vec_ptr(v, pos + nelem), v->len - (pos + nelem));
      v->len -= nelem;
   }
}

void vec_swap(Vec *v, size_t pos1, size_t pos2, void *tmp)
{
   if (pos1 < v->len && pos2 < v->len) {
      vec_memcpy(v, tmp, vec_ptr(v, pos1), 1);
      vec_memcpy(v, vec_ptr(v, pos1), vec_ptr(v, pos2), 1);
      vec_memcpy(v, vec_ptr(v, pos2), tmp, 1);
   }
}