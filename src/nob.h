#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

typedef enum Priority {
   PY_NECESSARY,
   //PY_OPTIONAL,
   PY_CHECK,
} Priority;

typedef enum LogLvl {
   LOG_INFO,
   LOG_WARNING,
   LOG_ERROR,
   LOG_DONT_LOG,
} LogLvl;

typedef struct SVec {
   char **ptr;
   size_t len;
   size_t cap;
} SVec;

/**
 * MARK: OS-specific
 */

char *nob_win_errmsg()
{
#define WIN_ERRMSG_SIZE 512
   static char errmsg[WIN_ERRMSG_SIZE];
   DWORD       errn = GetLastError();

   DWORD size = FormatMessageA(
      FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
      NULL,
      GetLastError(),
      LANG_USER_DEFAULT,
      errmsg,
      WIN_ERRMSG_SIZE,
      NULL
   );

   if (!size)
      sprintf(errmsg, "Couldn't get error message for 0x%lX", errn);

   return errmsg;
}

/**
 * MARK: Nob utility
 */
char *nob_strdup(char *str)
{
   size_t len = strlen(str);
   char  *dup = (char *)malloc(len + 1);
   return (char *)memcpy(dup, str, len + 1);
}

char nob_strncpy(char *dest, char *source, size_t num)
{
   strncpy(dest, source, num);
   dest[num] = '\0';
}

LogLvl nob_log_cv_priority(Priority priority)
{
   switch (priority) {
      case PY_NECESSARY:
         return LOG_ERROR;
      case PY_CHECK:
         return LOG_DONT_LOG;
      default:
         return LOG_INFO;
   }
}

#define nob_log(lvl, msg)       _nob_log((lvl), __LINE__, __func__, (msg))
#define nob_logf(lvl, fmt, ...) _nob_logf((lvl), __LINE__, __func__, (fmt), __VA_ARGS__)

void _nob_logf(LogLvl lvl, int line, const char *func, char *fmt, ...)
{
   switch (lvl) {
      case LOG_INFO:
         fprintf(stderr, "[INFO] ");
         break;
      case LOG_WARNING:
         fprintf(stderr, "[WARNING] ");
         break;
      case LOG_ERROR:
         fprintf(stderr, "[ERROR] ");
         break;
      case LOG_DONT_LOG:
      default:
         return;
   }

   fprintf(stderr, "%s::%d: ", func, line);

   va_list args;
   va_start(args, fmt);
   vfprintf(stderr, fmt, args);
   va_end(args);
   fprintf(stderr, "\n");
}

void _nob_log(LogLvl lvl, int line, const char *func, char *msg)
{
   _nob_logf(lvl, line, func, msg);
}

int nob_get_mtime(Priority priority, char *path, FILETIME *mtime)
{
   HANDLE fd =
      CreateFileA(path, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, NULL);
   if (fd == INVALID_HANDLE_VALUE) {
      nob_log(nob_log_cv_priority(priority), nob_win_errmsg());
      return -1;
   }
   WINBOOL okay = GetFileTime(fd, NULL, NULL, mtime);
   CloseHandle(fd);
   if (!okay) {
      nob_log(nob_log_cv_priority(priority), nob_win_errmsg());
      return -1;
   }
   return 0;
}

int nob_cmp_time(FILETIME *t1, FILETIME *t2)
{
   return (int)CompareFileTime(t1, t2);
}

void nob_run_cmd(SVec *args)
{
   // ...
   svec_reset(args);
}

/**
 * MARK: SVec
 */
void svec_free(SVec *svec)
{
   while (svec->len--)
      free(svec->ptr[svec->len]);
   free(svec->ptr);
   memset(svec, 0, sizeof(*svec));
}

void svec_reserve(SVec *svec, size_t nelem)
{
   if (nelem <= svec->cap)
      return;

   if (svec->cap) {
      svec->cap = svec->cap * 2 > nelem ? svec->cap * 2 : nelem;
      svec->ptr = (char **)realloc(svec->ptr, svec->cap * sizeof(char *));
   }
   else {
      svec->cap = nelem;
      svec->ptr = (char **)malloc(svec->cap * sizeof(char *));
   }
}

void svec_insert_n(SVec *svec, size_t nelem, char **elems, size_t pos)
{
   if (pos > svec->len)
      return;

   svec_reserve(svec, svec->len + nelem);
   memmove(&svec->ptr[pos + nelem], &svec->ptr[pos], svec->len - pos);

   for (size_t i = 0; i < nelem; i++) {
      svec->ptr[pos] = nob_strdup(elems[i]);
   }

   svec->len += nelem;
}

void svec_insert(SVec *svec, char *elem, size_t pos)
{
   svec_insert_n(svec, 1, &elem, pos);
}

void vec_push(SVec *svec, char *elem)
{
   svec_insert_n(svec, 1, &elem, svec->len);
}

void svec_remove_n(SVec *svec, size_t pos, size_t nelem, char **elems, size_t max_len)
{
   if (pos + nelem - 1 >= svec->len)
      return;

   for (size_t i = 0; i < nelem; i++) {
      if (elems)
         nob_strncpy(elems[i], svec->ptr[pos + i], max_len);
      free(svec->ptr[pos + i]);
   }

   if (pos + nelem < svec->len)
      memmove(&svec->ptr[pos], &svec->ptr[pos + nelem], svec->len - (pos + nelem));
   svec->len -= nelem;
}

void svec_remove(SVec *svec, size_t pos, char *elem, size_t max_arg)
{
   svec_remove_n(svec, pos, 1, &elem, max_arg);
}

void svec_pop(SVec *svec, char *elem, size_t max_arg)
{
   svec_remove_n(svec, svec->len - 1, 1, &elem, max_arg);
}

char *svec_get(SVec *svec, size_t pos)
{
   if (pos < svec->len)
      return svec->ptr[pos];
   return NULL;
}

void svec_set(SVec *svec, size_t pos, char *elem)
{
   if (pos < svec->len) {
      free(svec->ptr[pos]);
      svec->ptr[pos] = nob_strdup(elem);
   }
}

bool svec_find(SVec *svec, char *needle, size_t *ppos)
{
   size_t len = strlen(needle);

   for (size_t i = *ppos; i < svec->len; i++) {
      if (!strncmp(svec->ptr[i], needle, len)) {
         *ppos = i;
         return true;
      }
   }

   return false;
}

void svec_reset(SVec *svec)
{
   while (svec->len--)
      free(svec->ptr[svec->len]);
   svec->len = 0;
}