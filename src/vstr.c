#include <stdlib.h>
#include <string.h>

#include "vstr.h"

/**
 * @brief amount to grow underlying memory to
 */
#define GROWTH_FACTOR 2UL

/**
 * @brief allocate Vstr.
 * 
 * @param[in,out] vs Vstr
 * @param[in] nbytes number of bytes required
 */
INLINE static void vstr_alloc(Vstr *vs, size_t nbytes)
{
   vs->ptr = malloc(nbytes);
   vs->cap = nbytes;
}

/**
 * @brief reallocate Vstr.
 * 
 * @param[in,out] vs Vstr
 * @param[in] nbytes number of bytes required
 */
INLINE static void vstr_realloc(Vstr *vs, size_t nbytes)
{
   vs->ptr = realloc(vs->ptr, nbytes);
   vs->cap = nbytes;
}

/**
 * @brief resize Vstr.
 * 
 * if shrink, realloc by exact number
 * if grow  , realloc by GROWTH_FACTOR when possible, otherwise exact number
 * 
 * @param[in,out] vs Vstr
 * @param[in] nbytes number of bytes required
 */
static void vstr_resize(Vstr *vs, size_t nbytes)
{
   if (vs->cap) {
      if (nbytes < vs->cap || nbytes > vs->cap * GROWTH_FACTOR)
         vstr_realloc(vs, nbytes);
      else if (nbytes > vs->cap)
         vstr_realloc(vs, vs->cap * GROWTH_FACTOR);
   }
   else
      vstr_alloc(vs, nbytes > GROWTH_FACTOR ? nbytes : GROWTH_FACTOR);
}

/**
 * @brief append @p src from @p pos onward without bound-checking
 * 
 * @param[in,out] vs Vstr
 * @param[in] pos start position
 * @param[in] src source string
 * @param[in] nbyte number of bytes of @p src
 */
INLINE static void vstr_append_unchecked(Vstr *dst, size_t pos, const char *src, size_t nbyte)
{
   size_t new_len = pos + nbyte;

   vstr_reserve(dst, new_len);
   memcpy(&dst->ptr[pos], src, nbyte);
   dst->ptr[pos + nbyte] = '\0';
   dst->len = new_len;
}

void vstr_new(Vstr *vs)
{
   vs->cap = vs->len = 0;
}

void vstr_new_with(Vstr *vs, size_t len)
{
   vstr_new(vs);
   vstr_reserve(vs, len);
   vstr_cpy(vs, "");
}

void vstr_from(Vstr *vs, const char *src)
{
   vstr_new(vs);
   vstr_cpy(vs, src);
}

void vstr_free(Vstr *vs)
{
   if (vs->cap)
      free(vs->ptr);
   vs->cap = vs->len = 0;
}

void vstr_truncate(Vstr *vs, size_t new_len)
{
   if (new_len < vs->len) {
      vs->len = new_len;
      vs->ptr[new_len] = '\0';
   }
}

void vstr_reserve(Vstr *vs, size_t len)
{
   if (len + 1 > vs->cap)
      vstr_resize(vs, len + 1);
}

void vstr_shrink_to_fit(Vstr *vs)
{
   if (vs->cap > vs->len + 1)
      vstr_resize(vs, vs->len + 1);
}

bool vstr_insert(Vstr *dst, size_t pos, const char *src, size_t nbyte)
{
   if (pos > dst->len)
      return false;

   size_t new_len = dst->len + nbyte;

   vstr_reserve(dst, new_len);
   memmove(&dst->ptr[pos + nbyte], &dst->ptr[pos], dst->len - pos);
   memcpy(&dst->ptr[pos], src, nbyte);
   dst->ptr[new_len] = '\0';
   dst->len = new_len;

   return true;
}

char *vstr_cpy(Vstr *dst, const char *src)
{
   vstr_append_unchecked(dst, 0, src, strlen(src));

   return dst->ptr;
}

char *vstr_ncpy(Vstr *dst, const char *src, size_t nbyte)
{
   size_t src_len;

   src_len = strlen(src);
   if (src_len < nbyte)
      nbyte = src_len;
   vstr_append_unchecked(dst, 0, src, nbyte);

   return dst->ptr;
}

char *vstr_cat(Vstr *dst, const char *src)
{
   vstr_append_unchecked(dst, dst->len, src, strlen(src));

   return dst->ptr;
}

char *vstr_ncat(Vstr *dst, const char *src, size_t nbyte)
{
   size_t src_len;

   src_len = strlen(src);
   if (src_len < nbyte)
      nbyte = src_len;
   vstr_append_unchecked(dst, dst->len, src, nbyte);

   return dst->ptr;
}