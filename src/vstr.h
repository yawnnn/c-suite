/**
 * @file vstr.h
 */

#ifndef __VSTR_H__
#define __VSTR_H__

#include <stdbool.h>
#include <stdlib.h>

/**
 * @brief variable length string
 */
typedef struct Vstr {
    char *ptr; /**< underlying c-style string (access through vstr_data()) */
    size_t cap; /**< capacity allocated */
    size_t len; /**< length of the Vstr */
} Vstr;

/**
 * @brief new Vstr
 *
 * the Vstr is not allocated, therefore vstr_data() returns NULL
 *
 * @param s Vstr
 */
void vstr_new(Vstr *s);

/**
 * @brief new Vstr with reserved space
 *
 * the Vstr has 0 length but is allocated, therefore vstr_data() is valid
 *
 * @param s Vstr
 * @param len minimum number of characters to reserve memory for
 */
void vstr_new_with(Vstr *s, size_t len);

/**
 * @brief new Vstr from c-style string
 *
 * @param s Vstr
 * @param source source c-style string
 */
void vstr_from(Vstr *s, const char *source);

/**
 * @brief release memory
 *
 * @param s Vstr
 */
void vstr_free(Vstr *s);

/**
 * @brief empty the string but don't free the memory, so it can be reused
 *
 * @param s Vstr
 */
inline void vstr_truncate(Vstr *s) {
    if (s->len) {
        *s->ptr = '\0';
        s->len = 0;
    }
}

/**
 * @brief reserve memory ahead of time
 *
 * @param s Vstr
 * @param len minimum number of characters to reserve memory for
 */
void vstr_reserve(Vstr *s, size_t len);

/**
 * @brief shrink allocated memory to what is exactly needed for length
 *
 * @param s Vstr
 */
void vstr_shrink_to_fit(Vstr *s);

/**
 * @brief return the underlying c-style string, or NULL
 *
 * @param s Vstr
 * @return c-style string or NULL
 */
inline char *vstr_data(Vstr *s) {
    if (s->cap)  // len could be 0, but still allocated because of the null-terminating character
        return s->ptr;
    return NULL;
}

/**
 * @brief return the underlying c-style string starting at @p pos, or NULL
 *
 * @param s Vstr
 * @param pos start position
 * @return c-style string or NULL
 */
inline char *vstr_data_from(Vstr *s, size_t pos) {
    // asking for the position from the null-terminating char is valid, so i hate to check s->cap too
    if (pos <= s->len && s->cap)
        return s->ptr + pos;
    return NULL;
}

/**
 * @brief strcpy for Vstr
 *
 * @param dest Vstr
 * @param source source c-style string
 * @return the underlying c-style string of @p dest
 */
char *vstr_cpy(Vstr *dest, const char *source);

/**
 * @brief strncpy for Vstr
 *
 * @param dest Vstr
 * @param source source c-style string
 * @param num max number of characters to copy
 * @return the underlying c-style string of @p dest
 */
char *vstr_ncpy(Vstr *dest, const char *source, size_t num);

/**
 * @brief strcat for Vstr
 *
 * @param dest Vstr
 * @param source source c-style string
 * @return the underlying c-style string of @p dest
 */
char *vstr_cat(Vstr *dest, const char *source);

/**
 * @brief strncat for Vstr
 *
 * @param dest Vstr
 * @param source source c-style string
 * @param num max number of characters to concat
 * @return the underlying c-style string of @p dest
 */
char *vstr_ncat(Vstr *dest, const char *source, size_t num);

/**
 * @brief merge two Vstr
 *
 * merge @p source into @p dest with c-style string @p sep in between. consumes @p source
 *
 * @param dest Vstr dest
 * @param source Vstr source
 * @return the underlying c-style string of @p dest
 */
char *vstr_merge(Vstr *dest, Vstr *source, const char *sep);

/**
 * @brief if Vstr is empty
 *
 * @param s Vstr
 * @return boolean
 */
inline bool vstr_is_empty(Vstr *s) {
    return s->len == 0;
}

#endif /* __VSTR_H__ */