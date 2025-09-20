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
   void  *ptr; /**< underlying data */
   size_t len; /**< number of usable elements */
   size_t cap; /**< number of elements for which there is space allocated */
   size_t sizeof_t; /**< size of the data type to be held */
} Vec;

/**
 * @brief convert @p v to a `void **`, so that it can be cast to `T **`
 * 
 * useful to get *some* type-safety
 * for exemple function that take the `Vec` as input/output can express the underlying `T` in the signature
 * see `VEC_FROM_PV`
 */
INLINE static void **vec_to_pv(Vec *v)
{
   return &v->ptr;
}

/**
 * @brief wrapper that casts to `T **`
 */
#define VEC_TO_PV(T, v) ((T **)vec_to_pv((v)))

/**
 * @brief const counterpart of `VEC_TO_PV`
 */
#define VEC_TO_PV_CONST(T, v) ((const T **)vec_to_pv((Vec *)(v)))

/**
 * @brief convert @p pv back to a `Vec *`
 * 
 * see `VEC_TO_PV`
 */
INLINE static Vec *vec_from_pv(void **pv)
{
   return (Vec *)((char *)(pv) - offsetof(Vec, ptr));
}

/**
 * @brief wrapper that casts @p pv to `void **`
 */
#define VEC_FROM_PV(pv) vec_from_pv((void **)(pv))

/**
 * @brief const counterpart of `VEC_FROM_PV`
 */
#define VEC_FROM_PV_CONST(pv) ((const Vec *)vec_from_pv((void **)(pv)))

/**
 * @brief new Vec
 *
 * the Vec is not allocated
 *
 * @param[out] v Vec
 * @param[in] sizeof_t size of the single elements it's going to contain
 * 
 * @return ambivalent pointer that can be used as `Vec *` (see `VEC_FROM_PV) or as `T **` for *some* type-safety (see `VEC_TO_PV`)
 */
void **vec_new(Vec *v, size_t sizeof_t);

/**
 * @brief new Vec with reserved space
 *
 * the Vec has 0 length
 *
 * @param[out] v Vec
 * @param[in] sizeof_t size of the single elements it's going to contain
 * @param[in] nelem number of elements to reserve memory for
 * 
 * @return ambivalent pointer that can be used as `Vec *` (see `VEC_FROM_PV) or as `T **` for *some* type-safety (see `VEC_TO_PV`)
 */
void **vec_new_with(Vec *v, size_t sizeof_t, size_t nelem);

/**
 * @brief new Vec with elements initialized to zero
 *
 * the Vec has @p nelem length
 *
 * @param[out] v Vec
 * @param[in] sizeof_t size of the single elements it's going to contain
 * @param[in] nelem number of elements to initialize to zero
 * 
 * @return ambivalent pointer that can be used as `Vec *` (see `VEC_FROM_PV) or as `T **` for *some* type-safety (see `VEC_TO_PV`)
 */
void **vec_new_with_zeroed(Vec *v, size_t sizeof_t, size_t nelem);

/**
 * @brief new Vec copied from existing array
 *
 * @p arr is shallow-copied
 *
 * @param[out] v Vec
 * @param[in] sizeof_t size of the single elements it's going to contain
 * @param[in] arr source array
 * @param[in] nelem number of elements of array
 * 
 * @return ambivalent pointer that can be used as `Vec *` (see `VEC_FROM_PV) or as `T **` for *some* type-safety (see `VEC_TO_PV`)
 */
void **vec_from(Vec *v, size_t sizeof_t, const void *arr, size_t nelem);

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
 * @brief return slice to the underlying data
 * 
 * WARNING: this can be invalidated by insertions into the Vec
 *
 * @param[in] v Vec
 * @param[out] len length of the returned array
 * 
 * @return pointer to beginning of data, or NULL
 */
INLINE static void *vec_slice(Vec *v, size_t *len)
{
   *len = v->len;
   if (v->len)
      return v->ptr;
   return NULL;
}

/**
 * @brief wrapper that casts to `T *`
 */
#define VEC_SLICE(T, v, len) ((T *)vec_slice((v), (len)))

/**
 * @brief const counterpart of `VEC_SLICE`
 */
#define VEC_SLICE_CONST(T, v, len) ((const T *)vec_slice((Vec *)(v), (len)))

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
INLINE static void *vec_at(Vec *v, size_t pos)
{
   if (pos < v->len)
      return &((char *)v->ptr)[pos * v->sizeof_t];
   return NULL;
}

/**
 * @brief wrapper that casts to `T *`
 */
#define VEC_AT(T, v, pos)  ((T *)vec_at((v), (pos)))

/**
 * @brief const counterpart of `VEC_AT`
 */
#define VEC_AT_CONST(T, v, pos)  ((const T *)vec_at((Vec *)(v), (pos)))

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
 * @param[in] elems array of elements
 * @param[in] nelem number of elements of the array
 * 
 * @return false in case of failure
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
 * @return false in case of failure
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
 * @return false in case of failure
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
 * @param[out] elems if != NULL, on exit it's set with the elements removed
 * @param[in] nelem number of elements to be removed
 * 
 * @return false in case of failure
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
 * doesn't deallocate memory
 * if the element owns memory, that can be freed through @p elem
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
 * @brief swap two elements of the Vec
 *
 * @p tmp is required to simplify things
 * 
 * @param[in,out] v Vec
 * @param[in] pos1 index of the element
 * @param[in] pos2 index of the element
 * @param[out] tmp memory big enough to contain an element of the vector
 * 
 * @return false in case of failure
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