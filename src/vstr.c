/*
 * Copyright (c) 2026 Alessandro Martone
 *
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
 */

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

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
INLINE static void vstr_alloc(VStr *vs, size_t nbytes)
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
INLINE static void vstr_realloc(VStr *vs, size_t nbytes)
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
static void vstr_resize(VStr *vs, size_t nbytes)
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
INLINE static void vstr_append_unchecked(VStr *dst, size_t pos, const char *src, size_t nbyte)
{
   size_t new_len = pos + nbyte;

   vstr_reserve(dst, new_len);
   memcpy(&dst->ptr[pos], src, nbyte);
   dst->ptr[pos + nbyte] = '\0';
   dst->len = new_len;
}

/**
 * @brief sprintf on Vstr, internal impl that uses va_list
 */
INLINE static int vstr_vsprintf(VStr *dst, size_t pos, const char *format, va_list args)
{
   if (pos > dst->len)
      return -2;

   va_list args_copy;
   va_copy(args_copy, args);
   int n = vsnprintf(NULL, 0, format, args_copy);
   va_end(args_copy);

   if (n > 0) {
      vstr_reserve(dst, pos + n);
      vsnprintf(&dst->ptr[pos], n + 1, format, args);
      dst->len = pos + n;
   }

   return n;
}

void vstr_new(VStr *vs)
{
   vs->cap = vs->len = 0;
}

void vstr_new_with(VStr *vs, size_t len)
{
   vstr_new(vs);
   vstr_reserve(vs, len);
   vstr_cpy(vs, "");
}

void vstr_from(VStr *vs, const char *src)
{
   vstr_new(vs);
   vstr_cpy(vs, src);
}

void vstr_free(VStr *vs)
{
   if (vs->cap)
      free(vs->ptr);
   vs->cap = vs->len = 0;
}

void vstr_truncate(VStr *vs, size_t new_len)
{
   if (new_len < vs->len) {
      vs->len = new_len;
      vs->ptr[new_len] = '\0';
   }
}

void vstr_reserve(VStr *vs, size_t len)
{
   if (len + 1 > vs->cap)
      vstr_resize(vs, len + 1);
}

void vstr_shrink_to_fit(VStr *vs)
{
   if (vs->cap > vs->len + 1)
      vstr_resize(vs, vs->len + 1);
}

bool vstr_insert(VStr *dst, size_t pos, const char *src, size_t nbyte)
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

char *vstr_cpy(VStr *dst, const char *src)
{
   vstr_append_unchecked(dst, 0, src, strlen(src));

   return dst->ptr;
}

char *vstr_ncpy(VStr *dst, const char *src, size_t nbyte)
{
   size_t src_len;

   src_len = strlen(src);
   if (src_len < nbyte)
      nbyte = src_len;
   vstr_append_unchecked(dst, 0, src, nbyte);

   return dst->ptr;
}

char *vstr_cat(VStr *dst, const char *src)
{
   vstr_append_unchecked(dst, dst->len, src, strlen(src));

   return dst->ptr;
}

char *vstr_ncat(VStr *dst, const char *src, size_t nbyte)
{
   size_t src_len;

   src_len = strlen(src);
   if (src_len < nbyte)
      nbyte = src_len;
   vstr_append_unchecked(dst, dst->len, src, nbyte);

   return dst->ptr;
}

int vstr_sprintf(VStr *dst, size_t pos, const char *format, ...)
{
   va_list args;
   va_start(args, format);
   int n = vstr_vsprintf(dst, pos, format, args);
   va_end(args);

   return n;
}