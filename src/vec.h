/*
 * Copyright (c) 2026 Alessandro Martone
 *
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
 */

/**
 * @file vec.h
 */

#ifndef __VEC_H__
#define __VEC_H__

#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#ifdef _MSC_VER
   #define INLINE __inline
#elif defined(__STDC__) && __STDC_VERSION__ >= 199901L
   #define INLINE inline
#else
   #define INLINE
#endif

typedef void (*FreeFn)(void *); /**< free function for elements */

/**
 * @brief dynamic heap-allocated array
 */
typedef struct Vec {
   void  *ptr; /**< underlying data */
   size_t len; /**< number of usable elements */
   size_t cap; /**< number of elements for which there is space allocated */
   size_t size; /**< size of the data type to be held */
   FreeFn free_fn; /**< if != NULL, free function for elements */
} Vec;

/**
 * @brief initialize empty struct
 *
 * @param[out] v Vec
 * @param[in] size size of the single elements it's going to contain
 * @param[in] free_fn free function for elements, or NULL
 */
void vec_new(Vec *v, size_t size, FreeFn free_fn);

/**
 * @brief initialize struct and reserve space (without initializing them or the length)
 *
 * @param[out] v Vec
 * @param[in] size size of the single elements it's going to contain
 * @param[in] nelem number of elements to reserve memory for
 * @param[in] free_fn free function for elements, or NULL
 */
void vec_new_with(Vec *v, size_t size, size_t nelem, FreeFn free_fn);

/**
 * @brief initialize struct with the content of an array
 *
 * @p arr is shallow-copied
 *
 * @param[out] v Vec
 * @param[in] size size of the single elements it's going to contain
 * @param[in] arr source array
 * @param[in] nelem number of elements of array
 * @param[in] free_fn free function for elements, or NULL
 */
void vec_from(Vec *v, size_t size, const void *arr, size_t nelem, FreeFn free_fn);

/**
 * @brief release memory
 *
 * doesn't reset size.
 * if the single elements own memory, that needs to be release before by the caller
 *
 * @param[in,out] v Vec
 */
void vec_free(Vec *v);

/**
 * @brief shorten the Vec to the specified length
 *
 * if @p new_len is >= that the current length, this has no effect
 * 
 * @param[in,out] v Vec
 * @param[in] new_len new length
 */
void vec_truncate(Vec *v, size_t new_len);

/**
 * @brief reserve memory ahead of time
 *
 * @param[in,out] v Vec
 * @param[in] nelem number of elements to reserve memory for
 */
void vec_reserve(Vec *v, size_t nelem);

/**
 * @brief shrink allocated memory to what is exactly needed for length
 *
 * @param[in,out] v Vec
 */
void vec_shrink_to_fit(Vec *v);

/**
 * @brief return slice to the underlying data
 * 
 * WARNING: this can be invalidated by insertions into the Vec
 *
 * @param[in] v Vec
 * @param[out] len length of the returned array
 * 
 * @return pointer to beginning of data, or NULL
 */
INLINE static void *vec_slice(const Vec *v, size_t *len)
{
   *len = v->len;
   if (v->len)
      return v->ptr;
   return NULL;
}

/**
 * @brief return pointer to element at pos
 *
 * WARNING: this can be invalidated by insertions into the Vec
 *
 * @param[in] v Vec
 * @param[in] pos index of the element
 * 
 * @return pointer to element, or NULL
 */
INLINE static void *vec_at(const Vec *v, size_t pos)
{
   if (pos < v->len)
      return &((char *)v->ptr)[pos * v->size];
   return NULL;
}

/**
 * @brief get a shallow-copy of the element at pos
 *
 * @param[in] v Vec
 * @param[in] pos index of the element
 * @param[out] elem destination of the copy
 * 
 * @return false in case of failure
 */
bool vec_get(const Vec *v, size_t pos, void *elem);

/**
 * @brief set element at pos through shallow-copy
 *
 * @param[in,out] v Vec
 * @param[in] elem source of the copy
 * @param[in] pos index of the element
 * 
 * @return false in case of failure
 */
bool vec_set(Vec *v, const void *elem, size_t pos);

/**
 * @brief bulk insert of elements starting at pos through shallow-copy
 * 
 * @p elems can't be part of the Vec
 * 
 * @param[in,out] v Vec
 * @param[in] pos starting index of the elements
 * @param[in] elems array of elements. if NULL, the new elements are zeroed
 * @param[in] nelem number of elements of the array
 * 
 * @return pointer to the first inserted element, or NULL in case of failure
 */
void *vec_insert_n(Vec *v, size_t pos, const void *elems, size_t nelem);

/**
 * @brief insert element at pos through shallow-copy
 * 
 * see @p vec_insert_n for details
 * 
 * @param[in,out] v Vec
 * @param[in] pos index of the element
 * @param[in] elem element to insert
 * 
 * @return pointer to the inserted element, or NULL in case of failure
 */
INLINE static void *vec_insert(Vec *v, size_t pos, const void *elem)
{
   return vec_insert_n(v, pos, elem, 1);
}

/**
 * @brief insert element at the end of the vector through shallow-copy
 * 
 * see @p vec_insert_n for details
 * 
 * @param[in,out] v Vec
 * @param[in] elem element to insert
 * 
 * @return pointer to the inserted element, or NULL in case of failure
 */
INLINE static void *vec_push(Vec *v, const void *elem)
{
   return vec_insert_n(v, v->len, elem, 1);
}

/**
 * @brief bulk remove elements starting at pos, shifting the ones after
 *
 * doesn't deallocate the array's memory. see @p vec_shrink_to_fit
 * if the element owns memory, that will be freed automatically through @p free_fn
 * or through @p elem if it's not NULL
 * 
 * @param[in,out] v Vec
 * @param[in] pos index of the elements
 * @param[out] elems if != NULL, on exit it's set with the elements removed
 * @param[in] nelem number of elements to be removed
 * 
 * @return false in case of failure
 */
bool vec_remove_n(Vec *v, size_t pos, void *elems, size_t nelem);

/**
 * @brief remove element from pos, shifting the ones after
 *
 * see @p vec_remove_n for details
 * 
 * @param[in,out] v Vec
 * @param[in] pos index of the element
 * @param[out] elem if != NULL, on exit it's set with the element removed
 * 
 * @return false in case of failure
 */
INLINE static bool vec_remove(Vec *v, size_t pos, void *elem)
{
   return vec_remove_n(v, pos, elem, 1);
}

/**
 * @brief remove element from the end of the vector
 *
 * see @p vec_remove_n for details
 * 
 * @param[in,out] v Vec
 * @param[out] elem if != NULL, on exit it's set with the element removed
 * 
 * @return false in case of failure
 */
INLINE static bool vec_pop(Vec *v, void *elem)
{
   return vec_remove_n(v, v->len - 1, elem, 1);
}

/**
 * @brief if Vec is empty
 *
 * @param[in] v Vec
 * 
 * @return result
 */
INLINE static bool vec_is_empty(const Vec *v)
{
   return v->len == 0;
}

/**
 * @brief swap two non-overlapping ranges of elements in the Vec
 * 
 * @param[in,out] v Vec
 * @param[in] pos1 start of the first range
 * @param[in] pos2 start of the second range
 * @param[in] nelem number of elements per range
 * 
 * @return false in case of failure
 */
bool vec_swap(Vec *v, size_t pos1, size_t pos2, size_t nelem);

/**
 * @brief memset for @p v 's elements
 * 
 * @param[in,out] v Vec
 * @param[out] dst destination
 * @param[in] val value to set
 * @param[in] nelem number of @p v 's elements
 */
INLINE static void vec_memset(const Vec *v, void *dst, int val, size_t nelem)
{
   memset(dst, val, nelem * v->size);
}

/**
 * @brief memcpy for @p v 's elements
 * 
 * @param[in,out] v Vec
 * @param[out] dst destination
 * @param[in] src source
 * @param[in] nelem number of @p v 's elements
 */
INLINE static void vec_memcpy(const Vec *v, void *dst, const void *src, size_t nelem)
{
   memcpy(dst, src, nelem * v->size);
}

/**
 * @brief memmove for @p v 's elements
 * 
 * @param[in,out] v Vec
 * @param[out] dst destination
 * @param[in] src source
 * @param[in] nelem number of @p v 's elements
 */
INLINE static void vec_memmove(const Vec *v, void *dst, const void *src, size_t nelem)
{
   memmove(dst, src, nelem * v->size);
}

/**
 * @brief memcmp for @p v 's elements
 * 
 * @param[in] v Vec
 * @param[in] ptr1 pointer 1
 * @param[in] ptr2 pointer 2
 * @param[in] nelem number of @p v 's elements
 */
INLINE static int vec_memcmp(const Vec *v, const void *ptr1, const void *ptr2, size_t nelem)
{
   return memcmp(ptr1, ptr2, nelem * v->size);
}

#endif /* __VEC_H__ */