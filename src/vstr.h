/**
 * @file vstr.h
 */
#ifndef __VSTR_H__
#define __VSTR_H__

#include <stdbool.h>
#include <stddef.h>

#ifdef _MSC_VER
   #define INLINE __inline
#elif defined(__STDC__) && __STDC_VERSION__ >= 199901L
   #define INLINE inline
#else
   #define INLINE
#endif

/**
 * @brief variable length string
 */
typedef struct Vstr {
   char  *ptr; /**< underlying c-style string (access through vstr_data()) */
   size_t len; /**< length of the Vstr */
   size_t cap; /**< capacity allocated */
} Vstr;

/**
 * @brief new Vstr
 *
 * the Vstr is not allocated, therefore vstr_data() returns NULL
 *
 * @param[out] vs Vstr
 */
void vstr_new(Vstr *vs);

/**
 * @brief new Vstr with reserved space
 *
 * the Vstr has 0 length but is allocated, therefore vstr_data() is valid
 *
 * @param[out] vs Vstr
 * @param[in] len minimum number of characters to reserve memory for
 */
void vstr_new_with(Vstr *vs, size_t len);

/**
 * @brief new Vstr from c-style string
 *
 * @param[out] vs Vstr
 * @param[in] source source c-style string
 */
void vstr_from(Vstr *vs, const char *source);

/**
 * @brief release memory
 *
 * @param[in,out] vs Vstr
 */
void vstr_free(Vstr *vs);

/**
 * @brief shorten the Vstr to the specified length
 *
 * if @p new_len is >= that the current length, this has no effect
 * 
 * @param[in,out] vs Vstr
 * @param[in] new_len new length
 */
void vstr_truncate(Vstr *vs, size_t new_len);

/**
 * @brief reserve memory ahead of time
 *
 * @param[in,out] vs Vstr
 * @param[in] len minimum number of characters to reserve memory for
 */
void vstr_reserve(Vstr *vs, size_t len);

/**
 * @brief shrink allocated memory to what is exactly needed for length
 *
 * @param[in,out] vs Vstr
 */
void vstr_shrink_to_fit(Vstr *vs);

/**
 * @brief return the underlying c-style string, or NULL
 *
 * @param[in] vs Vstr
 * @return c-style string or NULL
 */
static INLINE char *vstr_data(Vstr *vs)
{
   /* len can be 0, but still allocated because of the null-terminating character */
   if (vs->cap)
      return vs->ptr;
   return NULL;
}

/**
 * @brief return the underlying c-style string starting at @p pos, or NULL
 *
 * @param[in] vs Vstr
 * @param[in] pos start position
 * @return c-style string or NULL
 */
static INLINE char *vstr_at(Vstr *vs, size_t pos)
{
   /* len can be 0, but still allocated because of the null-terminating character */
   if (vs->cap && pos <= vs->len)
      return vs->ptr + pos;
   return NULL;
}

/**
 * @brief insert characters from c-style string in Vstr at position, shifting the remaining ones
 * 
 * if @p pos is > than the current length, this has no effect
 *
 * @param[in,out] dest Vstr
 * @param[in] pos start position inside @p dest
 * @param[in] source source c-style string
 * @param[in] num max numbers of characters to copy
 */
void vstr_insert(Vstr *dest, size_t pos, const char *source, size_t num);

/**
 * @brief strcpy for Vstr
 *
 * @param[in,out] dest Vstr
 * @param[in] source source c-style string
 * @return the underlying c-style string of @p dest
 */
char *vstr_cpy(Vstr *dest, const char *source);

/**
 * @brief strncpy for Vstr
 *
 * @param[in,out] dest Vstr
 * @param[in] source source c-style string
 * @param[in] num max number of characters to copy
 * @return the underlying c-style string of @p dest
 */
char *vstr_ncpy(Vstr *dest, const char *source, size_t num);

/**
 * @brief strcat for Vstr
 *
 * @param[in,out] dest Vstr
 * @param[in] source source c-style string
 * @return the underlying c-style string of @p dest
 */
char *vstr_cat(Vstr *dest, const char *source);

/**
 * @brief strncat for Vstr
 *
 * @param[in,out] dest Vstr
 * @param[in] source source c-style string
 * @param[in] num max number of characters to concat
 * @return the underlying c-style string of @p dest
 */
char *vstr_ncat(Vstr *dest, const char *source, size_t num);

/**
 * @brief if Vstr is empty
 *
 * @param[in] vs Vstr
 * @return boolean
 */
static INLINE bool vstr_is_empty(Vstr *vs)
{
   return vs->len == 0;
}

#endif /* __VSTR_H__ */