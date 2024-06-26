#ifndef __VECTOR_H__
#define __VECTOR_H__

#include <stdbool.h>

typedef struct __Vec {
    char *ptr;   /**< underlying data (if needed, access through vec_data()) */
    size_t cap;  /**< number of elements for which there is space allocated */
    size_t len;  /**< number of usable elements */
    size_t szof; /**< sizeof() of the data type to be held */
} Vec;

/**
 * @brief new Vec
 *
 * the Vec is not allocated, therefore vec_data() returns NULL
 *
 * @param v Vec
 * @param szof size of the single elements it's going to contain
 */
void vec_new(Vec *v, size_t szof);

/**
 * @brief new Vec with reserved space
 *
 * the Vec has 0 length, therefore vec_data() returns NULL
 *
 * @param v Vec
 * @param szof size of the single elements it's going to contain
 * @param nelem number of elements to reserve memory for
 */
void vec_new_with(Vec *v, size_t szof, size_t nelem);

/**
 * @brief new Vec with elements initialized to zero
 *
 * the Vec has <nelem> length therefore vec_data() is valid
 *
 * @param v Vec
 * @param szof size of the single elements it's going to contain
 * @param nelem number of elements to initialize to zero
 */
void vec_new_with_zeroed(Vec *v, size_t szof, size_t nelem);

/**
 * @brief new Vec copied from existing array
 *
 * <arr> is shallow-copied
 *
 * @param v Vec
 * @param szof size of the single elements it's going to contain
 * @param arr source array
 * @param nelem number of elements of array
 */
void vec_from(Vec *v, size_t szof, void *arr, size_t nelem);

/**
 * @brief clear variables, release memory
 *
 * if the single elements own memory, that needs to be release separately
 *
 * @param v Vec
 */
void vec_clear(Vec *v);

/**
 * @brief reserve memory ahead of time
 *
 * @param v Vec
 * @param nelem number of elements to reserve memory for
 */
void vec_reserve(Vec *v, size_t nelem);

/**
 * @brief shrink allocated memory to what is exactly needed for length
 *
 * @param v Vec
 */
void vec_shrink_to_fit(Vec *v);

/**
 * @brief return pointer to element at pos
 *
 * if changes to the Vec are made, this pointer can become invalid
 *
 * @param v Vec
 * @param pos index of the element
 * @return pointer to element
 */
void *vec_elem_at(Vec *v, size_t pos);

/**
 * @brief get a shallow-copy of the element at pos
 *
 * @param v Vec
 * @param pos index of the element
 * @param elem destination of the copy
 */
void vec_get(Vec *v, size_t pos, void *elem);

/**
 * @brief set element at pos through shallow-copy
 *
 * @param v Vec
 * @param elem source of the copy
 * @param pos index of the element
 */
void vec_set(Vec *v, void *elem, size_t pos);

/**
 * @brief insert element at the end of the vector through shallow-copy
 *
 * @param v Vec
 * @param elem element to insert
 */
void vec_push(Vec *v, void *elem);

/**
 * @brief insert element at pos through shallow-copy
 *
 * @param v Vec
 * @param elem element to insert
 * @param pos index of the element
 */
void vec_insert(Vec *v, void *elem, size_t pos);

/**
 * @brief bulk insert of elements starting at pos through shallow-copy
 *
 * @param v Vec
 * @param elems array of elements
 * @param nelem number of elements of the array
 * @param pos starting index of the elements
 */
void vec_insert_n(Vec *v, void *elems, size_t nelem, size_t pos);

/**
 * @brief remove element from the end of the vector
 *
 * if the element owns memory, that needs to be freed through <elem>
 * 
 * @param v Vec
 * @param elem element removed, can be NULL
 */
void vec_pop(Vec *v, void *elem);

/**
 * @brief remove element from pos, shifting the ones after
 *
 * if the element owns memory, that needs to be freed through <elem>
 * 
 * @param v Vec
 * @param pos index of the element
 * @param elem element removed, can be NULL
 */
void vec_remove(Vec *v, size_t pos, void *elem);

/**
 * @brief bulk remove elements starting at pos, shifting the ones after
 *
 * if the elements own memory, that needs to be freed through <elems>
 * 
 * @param v Vec
 * @param pos index of the elements
 * @param elems elements removed, can be NULL
 * @param nelem number of elements to be removed
 */
void vec_remove_n(Vec *v, size_t pos, void *elems, size_t nelem);

/**
 * @brief swap two elements of the Vec
 *
 * <tmp> is required for simplicity of implementation. 
 * its value after the call is undefined and shouldn't be used
 * 
 * @param v Vec
 * @param pos1 index of the element
 * @param pos2 index of the element
 * @param tmp pointer to memory big enough to contain an element of the vector
 */
void vec_swap(Vec *v, size_t pos1, size_t pos2, void *tmp);

/**
 * @brief sort the vector through a simple memcmp of the elements
 *
 * @param v Vec
 * @param order order in which to sort VEC_SORT_[ASC|DESC]
 */
void vec_sort(Vec *v, int order);

/**
 * @brief iterate over the elements of the vector
 *
 * before starting the iteration, call vec_iter_reset()
 * then, call in a loop until the return value is false
 * the element of the iteration is shallow-copied in <elem>
 * 
 * @param v Vec
 * @param elem contains the element of this iteration
 * @return true until end of Vec is reached
 */
bool vec_iter(Vec *v, void *elem);

/* returns pointer to the underlying array data, or NULL if empty
 * if changes to the Vec are made, this pointer can become invalid */
/**
 * @brief pointer to the underlying data, or NULL
 *
 * @param v Vec
 * @return pointer to beginning of data
 */
inline void *vec_data(Vec *v)
{
    if (v->len)
        return (void *)v->ptr;
    return NULL;
}

/**
 * @brief wether Vec is empty
 *
 * @param v Vec
 * @return boolean
 */
inline bool vec_is_empty(Vec *v)
{
    return v->len == 0;
}

/**
 * @brief reset iterator
 */
#define vec_iter_reset()    vec_iter(NULL, NULL)

#define VEC_ORDER_ASC        1 // sort in ascending order
#define VEC_ORDER_DESC      -1 // sort in descending order

#endif /* __VECTOR_H__ */