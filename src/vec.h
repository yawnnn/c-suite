/**
 * @file vec.h
 */

#ifndef __VEC_H__
#define __VEC_H__

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief variable length array
 */
typedef struct Vec {
   void  *ptr; /**< underlying data (if needed, access through vec_data()) */
   size_t len; /**< number of usable elements */
   size_t szof; /**< sizeof() of the data type to be held */
   size_t cap; /**< number of elements for which there is space allocated */
} Vec;

/**
 * @brief new Vec
 *
 * the Vec is not allocated, therefore vec_data() returns NULL
 *
 * @param[out] v Vec
 * @param[in] szof size of the single elements it's going to contain
 */
void vec_new(Vec *v, size_t szof);

/**
 * @brief new Vec with reserved space
 *
 * the Vec has 0 length, therefore vec_data() returns NULL
 *
 * @param[out] v Vec
 * @param[in] szof size of the single elements it's going to contain
 * @param[in] nelem number of elements to reserve memory for
 */
void vec_new_with(Vec *v, size_t szof, size_t nelem);

/**
 * @brief new Vec with elements initialized to zero
 *
 * the Vec has @p nelem length therefore vec_data() is valid
 *
 * @param[out] v Vec
 * @param[in] szof size of the single elements it's going to contain
 * @param[in] nelem number of elements to initialize to zero
 */
void vec_new_with_zeroed(Vec *v, size_t szof, size_t nelem);

/**
 * @brief new Vec copied from existing array
 *
 * @p arr is shallow-copied
 *
 * @param[out] v Vec
 * @param[in] szof size of the single elements it's going to contain
 * @param[in] arr source array
 * @param[in] nelem number of elements of array
 */
void vec_from(Vec *v, size_t szof, void *arr, size_t nelem);

/**
 * @brief release memory
 *
 * doesn't reset szof.
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
 * @brief pointer to the underlying data, or NULL
 *
 * @param[in] v Vec
 * @return pointer to beginning of data
 */
inline void *vec_data(Vec *v) {
   if (v->len)
      return v->ptr;
   return NULL;
}

/**
 * @brief return pointer to element at pos
 *
 * if changes to the Vec are made, this pointer can become invalid
 *
 * @param[in] v Vec
 * @param[in] pos index of the element
 * @return pointer to element
 */
inline void *vec_elem_at(Vec *v, size_t pos) {
   if (pos < v->len)
      return ((char *)v->ptr) + (pos * v->szof);
   return NULL;
}

/**
 * @brief get a shallow-copy of the element at pos
 *
 * @param[in] v Vec
 * @param[in] pos index of the element
 * @param[out] elem destination of the copy
 */
void vec_get(Vec *v, size_t pos, void *elem);

/**
 * @brief set element at pos through shallow-copy
 *
 * @param[in,out] v Vec
 * @param[in] elem source of the copy
 * @param[in] pos index of the element
 */
void vec_set(Vec *v, void *elem, size_t pos);

/**
 * @brief bulk insert of elements starting at pos through shallow-copy
 * 
 * @p elems can't be part of the Vec
 * 
 * @param[in,out] v Vec
 * @param[in] elems array of elements
 * @param[in] nelem number of elements of the array
 * @param[in] pos starting index of the elements
 */
void vec_insert_n(Vec *v, void *elems, size_t nelem, size_t pos);

/**
 * @brief insert element at pos through shallow-copy
 * 
 * @p elem can't be part of the Vec
 * 
 * @param[in,out] v Vec
 * @param[in] elem element to insert
 * @param[in] pos index of the element
 */
inline void vec_insert(Vec *v, void *elem, size_t pos) {
   vec_insert_n(v, elem, 1, pos);
}

/**
 * @brief insert element at the end of the vector through shallow-copy
 * 
 * @p elem can't be part of the Vec
 * 
 * @param[in,out] v Vec
 * @param[in] elem element to insert
 */
inline void vec_push(Vec *v, void *elem) {
   vec_insert_n(v, elem, 1, v->len);
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
 */
void vec_remove_n(Vec *v, size_t pos, void *elems, size_t nelem);

/**
 * @brief remove element from pos, shifting the ones after
 *
 * doesn't deallocate memory
 * if the element owns memory, that can be freed through @p elem
 * 
 * @param[in,out] v Vec
 * @param[in] pos index of the element
 * @param[out] elem element removed, can be NULL
 */
inline void vec_remove(Vec *v, size_t pos, void *elem) {
   vec_remove_n(v, pos, elem, 1);
}

/**
 * @brief remove element from the end of the vector
 *
 * doesn't deallocate memory
 * if the element owns memory, that can be freed through @p elem
 * 
 * @param[in,out] v Vec
 * @param[out] elem element removed, can be NULL
 */
inline void vec_pop(Vec *v, void *elem) {
   vec_remove_n(v, v->len - 1, elem, 1);
}

/**
 * @brief if Vec is empty
 *
 * @param[in] v Vec
 * @return boolean
 */
inline bool vec_is_empty(Vec *v) {
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
 */
void vec_swap(Vec *v, size_t pos1, size_t pos2, void *tmp);

/**
 * @brief memset for @p v 's elements
 * 
 * @param[in,out] v Vec
 * @param[out] dst destination
 * @param[in] val value to set
 * @param[in] nelem number of @p v 's elements
 */
inline void vec_memset(Vec *v, void *dst, int val, size_t nelem) {
   memset(dst, val, nelem * v->szof);
}

/**
 * @brief memcpy for @p v 's elements
 * 
 * @param[in,out] v Vec
 * @param[out] dst destination
 * @param[in] src source
 * @param[in] nelem number of @p v 's elements
 */
inline void vec_memcpy(Vec *v, void *dst, void *src, size_t nelem) {
   memcpy(dst, src, nelem * v->szof);
}

/**
 * @brief memmove for @p v 's elements
 * 
 * @param[in,out] v Vec
 * @param[out] dst destination
 * @param[in] src source
 * @param[in] nelem number of @p v 's elements
 */
inline void vec_memmove(Vec *v, void *dst, void *src, size_t nelem) {
   memmove(dst, src, nelem * v->szof);
}

/**
 * @brief memcmp for @p v 's elements
 * 
 * @param[in] v Vec
 * @param[in] ptr1 pointer 1
 * @param[in] ptr2 pointer 2
 * @param[in] nelem number of @p v 's elements
 */
inline int vec_memcmp(Vec *v, void *ptr1, void *ptr2, size_t nelem) {
   return memcmp(ptr1, ptr2, nelem * v->szof);
}

#endif /* __VEC_H__ */