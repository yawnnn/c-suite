#ifndef __VSTR_H__
#define __VSTR_H__

#include <stdbool.h>

typedef struct {
    char *ptr;      /**< underlying c-style string (access through vstr_c_str()) */
    size_t cap;     /**< capacity allocated */
    size_t len;     /**< length of the string */
} Vstr;

/**
 * @brief new string
 * 
 * the string is not allocated and therefore vstr_c_str() & others are unusable
 * 
 * @param s Vstr
 */
void   vstr_new(Vstr *s);

/**
 * @brief new string with reserved space
 * 
 * the string has 0 length but is allocated, therefore vstr_c_str() & others are usable
 * 
 * @param s Vstr
 * @param len minimum number of characters to reserve memory for
 */
void   vstr_new_with(Vstr *s, size_t len);

/**
 * @brief new string from c-style string
 * 
 * @param s Vstr
 * @param source source c-style string
 */
void   vstr_from(Vstr *s, const char *source);

/**
 * @brief clear variables, release memory
 * 
 * @param s Vstr
 */
void   vstr_clear(Vstr *s);

/**
 * @brief reserve memory ahead of time
 * 
 * @param s Vstr
 * @param len minimum number of characters to reserve memory for
 */
void   vstr_reserve(Vstr *s, size_t len);

/**
 * @brief shrink allocated memory to what is exactly needed for length
 * 
 * @param s Vstr
 */
void   vstr_shrink_to_fit(Vstr *s);

/**
 * @brief strcpy for Vstr
 * 
 * @param dest Vstr
 * @param source source c-style string
 * @return the underlying c-style string of <dest>
 */
char  *vstr_cpy(Vstr *dest, const char *source);

/**
 * @brief strncpy for Vstr
 * 
 * @param dest Vstr
 * @param source source c-style string
 * @param num max number of characters to copy
 * @return the underlying c-style string of <dest>
 */
char  *vstr_ncpy(Vstr *dest, const char *source, size_t num);

/**
 * @brief strcat for Vstr
 * 
 * @param dest Vstr
 * @param source source c-style string
 * @return the underlying c-style string of <dest>
 */
char  *vstr_cat(Vstr *dest, const char *source);

/**
 * @brief strncat for Vstr
 * 
 * @param dest Vstr
 * @param source source c-style string
 * @param num max number of characters to concat
 * @return the underlying c-style string of <dest>
 */
char  *vstr_ncat(Vstr *dest, const char *source, size_t num);

/**
 * @brief merge two Vstr
 * 
 * merge <source> into <dest> with c-string <sep> in between. consumes <source>
 * 
 * @param dest Vstr dest
 * @param source Vstr source
 * @return the underlying c-style string of <dest>
 */
char  *vstr_merge(Vstr *dest, Vstr *source, const char *sep);

/**
 * @brief return the underlying c-style string
 * 
 * @param s Vstr
 * @return c-style string
 */
inline char *vstr_c_str(Vstr *s)
{
    if (s->cap)         // len could be 0, but still allocated because of the null-terminating character
        return s->ptr;
    return NULL;
}

/**
 * @brief wether Vstr is empty
 * 
 * @param s Vstr
 * @return wether <s> is empty
 */
inline bool vstr_is_empty(Vstr *s)
{
    return s->len == 0;
}

/**
 * @brief reset iterator
 */
#define vstr_iter_reset()   vstr_iter(NULL, NULL)

#endif  /* __VSTR_H__ */