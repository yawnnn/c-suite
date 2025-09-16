#include <stdlib.h>

#include "vec.h"

/**
 * @brief amount to grow underlying memory to
 */
#define GROWTH_FACTOR 2UL

/**
 * @brief pointer to the element of @p v at @p pos, without bound-checking
 * 
 * @param[in] v Vec
 * @param[in] pos position of the element
 * 
 * @return the pointer, already cast to char* for convenience
 */
INLINE static char *vec_at_unchecked(Vec *v, size_t pos)
{
   return &((char *)v->ptr)[pos * v->sizeof_t];
}

/**
 * @brief const counterpart of @p vec_at_unchecked
 */
INLINE static const char *vec_at_unchecked_const(const Vec *v, size_t pos)
{
   return vec_at_unchecked((Vec *)v, pos);
}

/**
 * @brief alloc Vec.
 * 
 * @param[in,out] v Vec
 * @param[in] nelem number of elements requested
 */
INLINE static void vec_alloc(Vec *v, size_t nelem)
{
   v->ptr = malloc(nelem * v->sizeof_t);
   v->cap = nelem;
}

/**
 * @brief realloc Vec.
 * 
 * @param[in,out] v Vec
 * @param[in] nelem number of elements requested
 */
INLINE static void vec_realloc(Vec *v, size_t nelem)
{
   v->ptr = realloc(v->ptr, nelem * v->sizeof_t);
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

void vec_new(Vec *v, size_t sizeof_t)
{
   v->len = v->cap = 0;
   v->sizeof_t = sizeof_t;
}

void vec_new_with(Vec *v, size_t sizeof_t, size_t nelem)
{
   vec_new(v, sizeof_t);
   vec_reserve(v, nelem);
}

void vec_new_with_zeroed(Vec *v, size_t sizeof_t, size_t nelem)
{
   vec_new_with(v, nelem, sizeof_t);
   vec_memset(v, v->ptr, 0, nelem);
   v->len = nelem;
}

void vec_from(Vec *v, size_t sizeof_t, const void *arr, size_t nelem)
{
   vec_new(v, sizeof_t);
   vec_insert_n(v, 0, arr, nelem);
}

void vec_free(Vec *v)
{
   if (v->cap)
      free(v->ptr);
   v->len = v->cap = 0;
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

bool vec_get(const Vec *v, size_t pos, void *elem)
{
   if (pos >= v->len)
      return false;

   vec_memcpy(v, elem, vec_at_unchecked_const(v, pos), 1);
   return true;
}

bool vec_set(Vec *v, const void *elem, size_t pos)
{
   if (pos >= v->len)
      return false;

   vec_memcpy(v, vec_at_unchecked(v, pos), elem, 1);
   return true;
}

bool vec_insert_n(Vec *v, size_t pos, const void *elems, size_t nelem)
{
   if (pos > v->len)
      return false;

   vec_reserve(v, v->len + nelem);
   vec_memmove(v, vec_at_unchecked(v, pos + nelem), vec_at_unchecked(v, pos), v->len - pos);
   vec_memcpy(v, vec_at_unchecked(v, pos), elems, nelem);
   v->len += nelem;

   return true;
}

bool vec_remove_n(Vec *v, size_t pos, void *elems, size_t nelem)
{
   if (pos + nelem - 1 >= v->len)
      return false;

   if (elems)
      vec_memcpy(v, elems, vec_at_unchecked(v, pos), nelem);
   if (pos + nelem < v->len)
      vec_memmove(
         v,
         vec_at_unchecked(v, pos),
         vec_at_unchecked(v, pos + nelem),
         v->len - (pos + nelem)
      );
   v->len -= nelem;

   return true;
}

bool vec_swap(Vec *v, size_t pos1, size_t pos2, void *tmp)
{
   if (pos1 >= v->len || pos2 >= v->len)
      return false;

   vec_memcpy(v, tmp, vec_at_unchecked(v, pos1), 1);
   vec_memcpy(v, vec_at_unchecked(v, pos1), vec_at_unchecked(v, pos2), 1);
   vec_memcpy(v, vec_at_unchecked(v, pos2), tmp, 1);

   return true;
}