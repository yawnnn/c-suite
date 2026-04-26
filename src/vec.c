/*
 * Copyright (c) 2026 Alessandro Martone
 *
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
 */

#include <stdlib.h>

#include "vec.h"

/**
 * @brief amount to grow underlying memory to
 */
#define GROWTH_FACTOR 2

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
   return &((char *)v->ptr)[pos * v->size];
}

/**
 * @brief const counterpart of @p vec_at_unchecked
 */
INLINE static const char *vec_at_unchecked_const(const Vec *v, size_t pos)
{
   return vec_at_unchecked((Vec *)v, pos);
}

void vec_new(Vec *v, size_t size, FreeFn free_fn)
{
   v->ptr = NULL;
   v->len = v->cap = 0;
   v->size = size;
   v->free_fn = free_fn;
}

void vec_new_with(Vec *v, size_t size, size_t nelem, FreeFn free_fn)
{
   vec_new(v, size, free_fn);
   vec_reserve(v, nelem);
}

void vec_from(Vec *v, size_t size, const void *arr, size_t nelem, FreeFn free_fn)
{
   vec_new(v, size, free_fn);
   vec_insert_n(v, 0, arr, nelem);
}

void vec_free(Vec *v)
{
   if (v->cap) {
      if (v->free_fn) {
         size_t i;
         for (i = 0; i < v->len; i++) {
            v->free_fn(vec_at_unchecked(v, i));
         }
      }
      free(v->ptr);
   }
   v->ptr = NULL;
   v->len = v->cap = 0;
}

void vec_truncate(Vec *v, size_t new_len)
{
   if (new_len < v->len) {
      if (v->free_fn) {
         size_t i;
         for (i = new_len; i < v->len; i++) {
            v->free_fn(vec_at_unchecked(v, i));
         }
      }
      v->len = new_len;
   }
}

void vec_reserve(Vec *v, size_t nelem)
{
   if (nelem > v->cap) {
      v->cap *= GROWTH_FACTOR;
      if (v->cap < nelem)
         v->cap = nelem;

      if (v->ptr)
         v->ptr = realloc(v->ptr, v->cap * v->size);
      else
         v->ptr = malloc(v->cap * v->size);
   }
}

void vec_shrink_to_fit(Vec *v)
{
   if (v->cap > v->len) {
      if (v->len) {
         v->cap = v->len;
         v->ptr = realloc(v->ptr, v->cap * v->size);
      }
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

   if (v->free_fn)
      v->free_fn(vec_at_unchecked(v, pos));
   vec_memcpy(v, vec_at_unchecked(v, pos), elem, 1);

   return true;
}

void *vec_insert_n(Vec *v, size_t pos, const void *elems, size_t nelem)
{
   if (pos > v->len)
      return NULL;

   vec_reserve(v, v->len + nelem);
   vec_memmove(v, vec_at_unchecked(v, pos + nelem), vec_at_unchecked(v, pos), v->len - pos);
   if (elems)
      vec_memcpy(v, vec_at_unchecked(v, pos), elems, nelem);
   else
      vec_memset(v, vec_at_unchecked(v, pos), 0, nelem);
   v->len += nelem;

   return vec_at_unchecked(v, pos);
}

bool vec_remove_n(Vec *v, size_t pos, void *elems, size_t nelem)
{
   if (pos + nelem - 1 >= v->len)
      return false;

   if (elems)
      vec_memcpy(v, elems, vec_at_unchecked(v, pos), nelem);
   else if (v->free_fn) {
      size_t i;
      for (i = pos; i < pos + nelem; i++) {
         v->free_fn(vec_at_unchecked(v, i));
      }
   }
   
   if (pos + nelem < v->len) {
      vec_memmove(
         v,
         vec_at_unchecked(v, pos),
         vec_at_unchecked(v, pos + nelem),
         v->len - (pos + nelem)
      );
   }
   v->len -= nelem;

   return true;
}

bool vec_swap(Vec *v, size_t pos1, size_t pos2, size_t nelem)
{
   char tmp[256];
   char *p1, *p2;
   size_t leftover;

   if (pos1 == pos2)
      return true;

   if (pos1 + nelem > v->len 
      || pos2 + nelem > v->len 
      || (pos1 > pos2 && pos1 < pos2 + nelem)
      || (pos2 > pos1 && pos2 < pos1 + nelem))
      return false;

   p1 = vec_at_unchecked(v, pos1);
   p2 = vec_at_unchecked(v, pos2);
   leftover = v->size * nelem;

   while (leftover) {
      size_t chunk = leftover < sizeof(tmp) ? leftover : sizeof(tmp);

      memcpy(tmp, p1, chunk);
      memcpy(p1, p2, chunk);
      memcpy(p2, tmp, chunk);
      p1 += chunk;
      p2 += chunk;
      leftover -= chunk;
   }

   return true;
}