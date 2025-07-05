#include "vstr.h"

#include <stdlib.h>
#include <string.h>

/**
 * @brief amount to grow underlying memory to
 */
#define GROWTH_FACTOR (2UL)

/**
 * @brief allocate Vstr.
 * 
 * @param[in,out] vs Vstr
 * @param[in] nbytes number of bytes required
 */
inline static void vstr_alloc(Vstr *vs, size_t nbytes)
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
inline static void vstr_realloc(Vstr *vs, size_t nbytes)
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

void vstr_new(Vstr *vs)
{
   vs->cap = 0;
   vs->len = 0;
}

void vstr_new_with(Vstr *vs, size_t len)
{
   vstr_new(vs);
   vstr_reserve(vs, len);
   vstr_cpy(vs, "");
}

void vstr_from(Vstr *vs, const char *source)
{
   vstr_new(vs);
   vstr_cpy(vs, source);
}

void vstr_free(Vstr *vs)
{
   if (vs->cap)
      free(vs->ptr);
   vs->cap = 0;
   vs->len = 0;
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

void vstr_insert(Vstr *dest, size_t pos, const char *source, size_t num)
{
   if (pos <= dest->len) {
      size_t new_len = dest->len + num;

      vstr_reserve(dest, new_len);
      memmove(&dest->ptr[pos + num], &dest->ptr[pos], dest->len - pos);
      memcpy(&dest->ptr[pos], source, num);
      dest->ptr[new_len] = '\0';
      dest->len = new_len;
   }
}

char *vstr_cpy(Vstr *dest, const char *source)
{
   vstr_truncate(dest, 0);
   vstr_insert(dest, 0, source, strlen(source));

   return dest->ptr;
}

char *vstr_ncpy(Vstr *dest, const char *source, size_t num)
{
   size_t src_len;

   src_len = strlen(source);
   if (src_len < num)
      num = src_len;
   vstr_truncate(dest, 0);
   vstr_insert(dest, 0, source, num);

   return dest->ptr;
}

char *vstr_cat(Vstr *dest, const char *source)
{
   vstr_insert(dest, dest->len, source, strlen(source));

   return dest->ptr;
}

char *vstr_ncat(Vstr *dest, const char *source, size_t num)
{
   size_t src_len;

   src_len = strlen(source);
   if (src_len < num)
      num = src_len;
   vstr_insert(dest, dest->len, source, num);

   return dest->ptr;
}