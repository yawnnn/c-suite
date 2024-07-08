/**
 * @file vector.h
 */

#ifndef __VECTOR_H__
#define __VECTOR_H__

#include <stdbool.h>
#include <stdlib.h>

/**
 * @brief dynamic array
 */
typedef struct Vector {
    void *ptr; /**< underlying data (if needed, access through Vector_data()) */
    size_t cap; /**< number of elements for which there is space allocated */
    size_t len; /**< number of usable elements */
    size_t szof; /**< sizeof() of the data type to be held */
} Vector;

/**
 * @brief new Vector
 *
 * the Vector is not allocated, therefore Vector_data() returns NULL
 *
 * @param v Vector
 * @param szof size of the single elements it's going to contain
 */
void Vector_new(Vector *v, size_t szof);

/**
 * @brief new Vector with reserved space
 *
 * the Vector has 0 length, therefore Vector_data() returns NULL
 *
 * @param v Vector
 * @param szof size of the single elements it's going to contain
 * @param nelem number of elements to reserve memory for
 */
void Vector_new_with(Vector *v, size_t szof, size_t nelem);

/**
 * @brief new Vector with elements initialized to zero
 *
 * the Vector has @p nelem length therefore Vector_data() is valid
 *
 * @param v Vector
 * @param szof size of the single elements it's going to contain
 * @param nelem number of elements to initialize to zero
 */
void Vector_new_with_zeroed(Vector *v, size_t szof, size_t nelem);

/**
 * @brief new Vector copied from existing array
 *
 * @p arr is shallow-copied
 *
 * @param v Vector
 * @param szof size of the single elements it's going to contain
 * @param arr source array
 * @param nelem number of elements of array
 */
void Vector_from(Vector *v, size_t szof, void *arr, size_t nelem);

/**
 * @brief release memory
 *
 * doesn't reset szof.
 * if the single elements own memory, that needs to be release before by the caller
 *
 * @param v Vector
 */
void Vector_free(Vector *v);

/**
 * @brief empty the Vector but don't free the memory, so it can be reused
 *
 * @param v Vector
 */
inline void Vector_truncate(Vector *v) {
    v->len = 0;
}

/**
 * @brief reserve memory ahead of time
 *
 * @param v Vector
 * @param nelem number of elements to reserve memory for
 */
void Vector_reserve(Vector *v, size_t nelem);

/**
 * @brief shrink allocated memory to what is exactly needed for length
 *
 * @param v Vector
 */
void Vector_shrink_to_fit(Vector *v);

/**
 * @brief pointer to the underlying data, or NULL
 *
 * @param v Vector
 * @return pointer to beginning of data
 */
inline void *Vector_data(Vector *v) {
    if (v->len)
        return v->ptr;
    return NULL;
}

/**
 * @brief return pointer to element at pos
 *
 * if changes to the Vector are made, this pointer can become invalid
 *
 * @param v Vector
 * @param pos index of the element
 * @return pointer to element
 */
inline void *Vector_elem_at(Vector *v, size_t pos) {
    if (pos < v->len)
        return (void *)(((char *)v->ptr) + (pos * v->szof));
    return NULL;
}

/**
 * @brief get a shallow-copy of the element at pos
 *
 * @param v Vector
 * @param pos index of the element
 * @param elem destination of the copy
 */
void Vector_get(Vector *v, size_t pos, void *elem);

/**
 * @brief set element at pos through shallow-copy
 *
 * @param v Vector
 * @param elem source of the copy
 * @param pos index of the element
 */
void Vector_set(Vector *v, void *elem, size_t pos);

/**
 * @brief bulk insert of elements starting at pos through shallow-copy
 *
 * @param v Vector
 * @param elems array of elements
 * @param nelem number of elements of the array
 * @param pos starting index of the elements
 */
void Vector_insert_n(Vector *v, void *elems, size_t nelem, size_t pos);

/**
 * @brief insert element at the end of the vector through shallow-copy
 *
 * @param v Vector
 * @param elem element to insert
 */
inline void Vector_push(Vector *v, void *elem) {
    Vector_insert_n(v, elem, 1, v->len);
}

/**
 * @brief insert element at pos through shallow-copy
 *
 * @param v Vector
 * @param elem element to insert
 * @param pos index of the element
 */
inline void Vector_insert(Vector *v, void *elem, size_t pos) {
    Vector_insert_n(v, elem, 1, pos);
}

/**
 * @brief bulk remove elements starting at pos, shifting the ones after
 *
 * doesn't deallocate memory
 * if the elements own memory, that needs to be freed through @p elems
 * 
 * @param v Vector
 * @param pos index of the elements
 * @param elems elements removed, can be NULL
 * @param nelem number of elements to be removed
 */
void Vector_remove_n(Vector *v, size_t pos, void *elems, size_t nelem);

/**
 * @brief remove element from the end of the vector
 *
 * doesn't deallocate memory
 * if the element owns memory, that needs to be freed through @p elem
 * 
 * @param v Vector
 * @param elem element removed, can be NULL
 */
inline void Vector_pop(Vector *v, void *elem) {
    Vector_remove_n(v, v->len - 1, elem, 1);
}

/**
 * @brief remove element from pos, shifting the ones after
 *
 * doesn't deallocate memory
 * if the element owns memory, that needs to be freed through @p elem
 * 
 * @param v Vector
 * @param pos index of the element
 * @param elem element removed, can be NULL
 */
inline void Vector_remove(Vector *v, size_t pos, void *elem) {
    Vector_remove_n(v, pos, elem, 1);
}

/**
 * @brief if Vector is empty
 *
 * @param v Vector
 * @return boolean
 */
inline bool Vector_is_empty(Vector *v) {
    return v->len == 0;
}

/**
 * @brief swap two elements of the Vector
 *
 * @p tmp is required for simplicity of implementation. 
 * its value after the call is undefined and shouldn't be used
 * 
 * @param v Vector
 * @param pos1 index of the element
 * @param pos2 index of the element
 * @param tmp pointer to memory big enough to contain an element of the vector
 */
void Vector_swap(Vector *v, size_t pos1, size_t pos2, void *tmp);

/**
 * @brief memset for @p v 's elements
 * 
 * @param v Vector
 * @param dst destination
 * @param val value to set
 * @param nelem number of @p v 's elements
 */
inline void Vector_memset(Vector *v, void *dst, int val, size_t nelem) {
    memset(dst, val, nelem * v->szof);
}

/**
 * @brief memcpy for @p v 's elements
 * 
 * @param v Vector
 * @param dst destination
 * @param src source
 * @param nelem number of @p v 's elements
 */
inline void Vector_memcpy(Vector *v, void *dst, void *src, size_t nelem) {
    memcpy(dst, src, nelem * v->szof);
}

/**
 * @brief memmove for @p v 's elements
 * 
 * @param v Vector
 * @param dst destination
 * @param src source
 * @param nelem number of @p v 's elements
 */
inline void Vector_memmove(Vector *v, void *dst, void *src, size_t nelem) {
    memmove(dst, src, nelem * v->szof);
}

/**
 * @brief memcmp for @p v 's elements
 * 
 * @param v Vector
 * @param ptr1 pointer 1
 * @param ptr2 pointer 2
 * @param nelem number of @p v 's elements
 */
inline int Vector_memcmp(Vector *v, void *ptr1, void *ptr2, size_t nelem) {
    return memcmp(ptr1, ptr2, nelem * v->szof);
}

#endif /* __VECTOR_H__ */