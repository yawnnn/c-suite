#ifndef __VSTR_H__
#define __VSTR_H__

#include <stdbool.h>

typedef struct {
    char *ptr;      // underlying c-style string (access through vstr_c_str())
    size_t cap;     // capacity allocated
    size_t len;     // length of the string
} Vstr;

void   vstr_new          (Vstr *s);
void   vstr_init         (Vstr *s, size_t len);
void   vstr_from         (Vstr *s, const char *source);
void   vstr_clear        (Vstr *s);
void   vstr_reserve      (Vstr *s, size_t len);
void   vstr_shrink_to_fit(Vstr *s);
char  *vstr_cpy          (Vstr *dest, const char *source);
char  *vstr_ncpy         (Vstr *dest, const char *source, size_t num);
char  *vstr_cat          (Vstr *dest, const char *source);
char  *vstr_ncat         (Vstr *dest, const char *source, size_t num);
char  *vstr_merge        (Vstr *s1, Vstr *s2, const char *sep);

/* return the underlying c-style string */
inline char *vstr_c_str(Vstr *s)
{
    if (s->cap)         // len could be 0, but still allocated because of the null-terminating character
        return s->ptr;
    return NULL;
}

inline bool vstr_is_empty(Vstr *s)
{
    return s->len == 0;
}

#define vstr_iter_reset()   vstr_iter(NULL, NULL)

#endif  /* __VSTR_H__ */