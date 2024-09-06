#include "vstr.h"

#define GROWTH_FACTOR (2UL)

/**
 * @brief allocate Vstr.
 * 
 * @param s Vstr
 * @param nbytes number of bytes required
 */
inline static void vstr_alloc(Vstr *s, size_t nbytes) {
   s->ptr = malloc(nbytes);
   s->cap = nbytes;
}

/**
 * @brief reallocate Vstr.
 * 
 * @param s Vstr
 * @param nbytes number of bytes required
 */
inline static void vstr_realloc(Vstr *s, size_t nbytes) {
   s->ptr = realloc(s->ptr, nbytes);
   s->cap = nbytes;
}

/**
 * @brief resize Vstr.
 * 
 * if shrink, realloc by exact number
 * if grow  , realloc by GROWTH_FACTOR when possible, otherwise exact number
 * 
 * @param s Vstr
 * @param nbytes number of bytes required
 */
static void vstr_resize(Vstr *s, size_t nbytes) {
   if (s->cap) {
      if (nbytes < s->cap || nbytes > s->cap * GROWTH_FACTOR)
         vstr_realloc(s, nbytes);
      else if (nbytes > s->cap)
         vstr_realloc(s, s->cap * GROWTH_FACTOR);
   } else {
      vstr_alloc(s, nbytes > GROWTH_FACTOR ? nbytes : GROWTH_FACTOR);
   }
}

void vstr_new(Vstr *s) {
   s->cap = 0;
   s->len = 0;
}

void vstr_new_with(Vstr *s, size_t len) {
   vstr_new(s);
   vstr_reserve(s, len);
   vstr_cpy(s, "");
}

void vstr_from(Vstr *s, const char *source) {
   vstr_new(s);
   vstr_cpy(s, source);
}

void vstr_free(Vstr *s) {
   if (s->cap) free(s->ptr);
   s->cap = 0;
   s->len = 0;
}

void vstr_truncate(Vstr *s, size_t new_len) {
   if (new_len < s->len) {
      s->len = new_len;
      s->ptr[new_len] = '\0';
   }
}

void vstr_reserve(Vstr *s, size_t len) {
   if (len + 1 > s->cap) vstr_resize(s, len + 1);
}

void vstr_shrink_to_fit(Vstr *s) {
   if (s->cap > s->len + 1) vstr_resize(s, s->len + 1);
}

void vstr_insert(Vstr *dest, size_t pos, const char *source, size_t num) {
   if (pos <= dest->len) {
      size_t new_len = dest->len + num;

      vstr_reserve(dest, new_len);
      memmove(&dest->ptr[pos + num], &dest->ptr[pos], dest->len - pos);
      memcpy(&dest->ptr[pos], source, num);
      dest->ptr[new_len] = '\0';
      dest->len = new_len;
   }
}

char *vstr_cpy(Vstr *dest, const char *source) {
   vstr_truncate(dest, 0);
   vstr_insert(dest, 0, source, strlen(source));

   return dest->ptr;
}

char *vstr_ncpy(Vstr *dest, const char *source, size_t num) {
   size_t src_len;

   src_len = strlen(source);
   if (src_len < num) num = src_len;
   vstr_truncate(dest, 0);
   vstr_insert(dest, 0, source, num);

   return dest->ptr;
}

char *vstr_cat(Vstr *dest, const char *source) {
   vstr_insert(dest, dest->len, source, strlen(source));

   return dest->ptr;
}

char *vstr_ncat(Vstr *dest, const char *source, size_t num) {
   size_t src_len;

   src_len = strlen(source);
   if (src_len < num) num = src_len;
   vstr_insert(dest, dest->len, source, num);

   return dest->ptr;
}