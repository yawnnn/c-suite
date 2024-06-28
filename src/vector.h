/**
 * @file vector.h
 */

#ifndef __VECTOR_H__
#define __VECTOR_H__

#include <stdbool.h>

/**
 * @brief dynamic array
 */
typedef struct Vector {
    char *ptr; /**< underlying data (if needed, access through Vector_data()) */
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
 * @brief clear variables, release memory
 *
 * if the single elements own memory, that needs to be release separately
 *
 * @param v Vector
 */
void Vector_clear(Vector *v);

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
 * @brief return pointer to element at pos
 *
 * if changes to the Vector are made, this pointer can become invalid
 *
 * @param v Vector
 * @param pos index of the element
 * @return pointer to element
 */
void *Vector_elem_at(Vector *v, size_t pos);

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
 * @brief insert element at the end of the vector through shallow-copy
 *
 * @param v Vector
 * @param elem element to insert
 */
void Vector_push(Vector *v, void *elem);

/**
 * @brief insert element at pos through shallow-copy
 *
 * @param v Vector
 * @param elem element to insert
 * @param pos index of the element
 */
void Vector_insert(Vector *v, void *elem, size_t pos);

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
 * @brief remove element from the end of the vector
 *
 * if the element owns memory, that needs to be freed through @p elem
 * 
 * @param v Vector
 * @param elem element removed, can be NULL
 */
void Vector_pop(Vector *v, void *elem);

/**
 * @brief remove element from pos, shifting the ones after
 *
 * if the element owns memory, that needs to be freed through @p elem
 * 
 * @param v Vector
 * @param pos index of the element
 * @param elem element removed, can be NULL
 */
void Vector_remove(Vector *v, size_t pos, void *elem);

/**
 * @brief bulk remove elements starting at pos, shifting the ones after
 *
 * if the elements own memory, that needs to be freed through @p elems
 * 
 * @param v Vector
 * @param pos index of the elements
 * @param elems elements removed, can be NULL
 * @param nelem number of elements to be removed
 */
void Vector_remove_n(Vector *v, size_t pos, void *elems, size_t nelem);

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
 * @brief sort the vector through a simple memcmp of the elements
 *
 * @param v Vector
 * @param order order in which to sort VEC_SORT_[ASC|DESC]
 */
void Vector_sort(Vector *v, int order);

/**
 * @brief iterate over the elements of the vector
 *
 * before starting the iteration, call Vector_iter_reset()
 * then, call in a loop until the return value is false
 * the element of the iteration is shallow-copied in @p elem
 * 
 * @param v Vector
 * @param elem contains the element of this iteration
 * @return true until end of Vector is reached
 */
bool Vector_iter(Vector *v, void *elem);

/**
 * @brief pointer to the underlying data, or NULL
 *
 * @param v Vector
 * @return pointer to beginning of data
 */
inline void *Vector_data(Vector *v) {
    if (v->len)
        return (void *)v->ptr;
    return (void *)0;
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
 * @brief reset iterator
 */
#define Vector_iter_reset() Vector_iter(NULL, NULL)

/**
 * @brief sort in ascending order
 */
#define VEC_ORDER_ASC 1

/**
 * @brief sort in descending order
 */
#define VEC_ORDER_DESC -1

#endif /* __VECTOR_H__ */