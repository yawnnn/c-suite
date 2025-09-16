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

/**
 * @brief dynamic heap-allocated array
 */
typedef struct Vec {
   void  *ptr; /**< underlying data (if needed, access through vec_data()) */
   size_t len; /**< number of usable elements */
   size_t cap; /**< number of elements for which there is space allocated */
   size_t sizeof_t; /**< size of the data type to be held */
} Vec;

/**
 * @brief new Vec
 *
 * the Vec is not allocated, therefore vec_data() returns NULL
 *
 * @param[out] v Vec
 * @param[in] sizeof_t size of the single elements it's going to contain
 */
void vec_new(Vec *v, size_t sizeof_t);

/**
 * @brief new Vec with reserved space
 *
 * the Vec has 0 length, therefore vec_data() returns NULL
 *
 * @param[out] v Vec
 * @param[in] sizeof_t size of the single elements it's going to contain
 * @param[in] nelem number of elements to reserve memory for
 */
void vec_new_with(Vec *v, size_t sizeof_t, size_t nelem);

/**
 * @brief new Vec with elements initialized to zero
 *
 * the Vec has @p nelem length therefore vec_data() is valid
 *
 * @param[out] v Vec
 * @param[in] sizeof_t size of the single elements it's going to contain
 * @param[in] nelem number of elements to initialize to zero
 */
void vec_new_with_zeroed(Vec *v, size_t sizeof_t, size_t nelem);

/**
 * @brief new Vec copied from existing array
 *
 * @p arr is shallow-copied
 *
 * @param[out] v Vec
 * @param[in] sizeof_t size of the single elements it's going to contain
 * @param[in] arr source array
 * @param[in] nelem number of elements of array
 */
void vec_from(Vec *v, size_t sizeof_t, const void *arr, size_t nelem);

/**
 * @brief release memory
 *
 * doesn't reset sizeof_t.
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
 * @brief pointer to the underlying data
 *
 * @param[in] v Vec
 * 
 * @return pointer to beginning of data, or NULL
 */
INLINE static void *vec_data(Vec *v)
{
   if (v->len)
      return v->ptr;
   return NULL;
}

/**
 * @brief const counterpart of @p vec_data
 */
INLINE static const void *vec_data_const(const Vec *v)
{
   return vec_data((Vec *)v);
}

/**
 * @brief return pointer to element at pos
 *
 * if changes to the Vec are made, this pointer can become invalid
 *
 * @param[in] v Vec
 * @param[in] pos index of the element
 * 
 * @return pointer to element, or NULL
 */
INLINE static void *vec_at(Vec *v, size_t pos)
{
   if (pos < v->len)
      return &((char *)v->ptr)[pos * v->sizeof_t];
   return NULL;
}

/**
 * @brief const counterpart of @p vec_at
 */
INLINE static const void *vec_at_const(const Vec *v, size_t pos)
{
   return vec_at((Vec *)v, pos);
}

/**
 * @brief get a shallow-copy of the element at pos
 *
 * @param[in] v Vec
 * @param[in] pos index of the element
 * @param[out] elem destination of the copy
 * 
 * @return false of failure
 */
bool vec_get(const Vec *v, size_t pos, void *elem);

/**
 * @brief set element at pos through shallow-copy
 *
 * @param[in,out] v Vec
 * @param[in] elem source of the copy
 * @param[in] pos index of the element
 * 
 * @return false of failure
 */
bool vec_set(Vec *v, const void *elem, size_t pos);

/**
 * @brief bulk insert of elements starting at pos through shallow-copy
 * 
 * @p elems can't be part of the Vec
 * 
 * @param[in,out] v Vec
 * @param[in] pos starting index of the elements
 * @param[in] elems array of elements
 * @param[in] nelem number of elements of the array
 * 
 * @return false of failure
 */
bool vec_insert_n(Vec *v, size_t pos, const void *elems, size_t nelem);

/**
 * @brief insert element at pos through shallow-copy
 * 
 * @p elem can't be part of the Vec
 * 
 * @param[in,out] v Vec
 * @param[in] pos index of the element
 * @param[in] elem element to insert
 * 
 * @return false of failure
 */
INLINE static bool vec_insert(Vec *v, size_t pos, const void *elem)
{
   return vec_insert_n(v, pos, elem, 1);
}

/**
 * @brief insert element at the end of the vector through shallow-copy
 * 
 * @p elem can't be part of the Vec
 * 
 * @param[in,out] v Vec
 * @param[in] elem element to insert
 * 
 * @return false of failure
 */
INLINE static bool vec_push(Vec *v, const void *elem)
{
   return vec_insert_n(v, v->len, elem, 1);
}

/**
 * @brief bulk remove elements starting at pos, shifting the ones after
 *
 * doesn't deallocate memory
 * if the elements own memory, that can be freed through @p elems
 * 
 * @param[in,out] v Vec
 * @param[in] pos index of the elements
 * @param[out] elems the elements removed, can be NULL
 * @param[in] nelem number of elements to be removed
 * 
 * @return false of failure
 */
bool vec_remove_n(Vec *v, size_t pos, void *elems, size_t nelem);

/**
 * @brief remove element from pos, shifting the ones after
 *
 * doesn't deallocate memory
 * if the element owns memory, that can be freed through @p elem
 * 
 * @param[in,out] v Vec
 * @param[in] pos index of the element
 * @param[out] elem element removed, can be NULL
 * 
 * @return false of failure
 */
INLINE static bool vec_remove(Vec *v, size_t pos, void *elem)
{
   return vec_remove_n(v, pos, elem, 1);
}

/**
 * @brief remove element from the end of the vector
 *
 * doesn't deallocate memory
 * if the element owns memory, that can be freed through @p elem
 * 
 * @param[in,out] v Vec
 * @param[out] elem element removed, can be NULL
 * 
 * @return false of failure
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
 * @brief swap two elements of the Vec
 *
 * @p tmp is required for simplicity of implementation. its value after the call is undefined
 * 
 * @param[in,out] v Vec
 * @param[in] pos1 index of the element
 * @param[in] pos2 index of the element
 * @param[out] tmp memory big enough to contain an element of the vector
 * 
 * @return false of failure
 */
bool vec_swap(Vec *v, size_t pos1, size_t pos2, void *tmp);

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
   memset(dst, val, nelem * v->sizeof_t);
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
   memcpy(dst, src, nelem * v->sizeof_t);
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
   memmove(dst, src, nelem * v->sizeof_t);
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
   return memcmp(ptr1, ptr2, nelem * v->sizeof_t);
}

#endif /* __VEC_H__ */