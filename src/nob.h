#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <direct.h>

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

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
#define STR_SIZE(str)   (sizeof(str) - 1)

/**
 * MARK: SVec
 */

SVec *svec_from(SVec *svec, const char *s, ...)
{
   memset(svec, 0, sizeof(*svec));

   va_list args;
   va_start(args, s);
   for (; s; s = va_arg(args, const char *)) {
      svec_push(svec, s);
   }
   va_end(args);

   return svec;
}

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

void svec_insert_n(SVec *svec, size_t pos, const char **elems, size_t nelem)
{
   if (pos > svec->len)
      return;

   svec_reserve(svec, svec->len + nelem);
   memmove(&svec->ptr[pos + nelem], &svec->ptr[pos], (svec->len - pos) * sizeof(char *));

   for (size_t i = 0; i < nelem; i++) {
      svec->ptr[pos] = nob_strdup(elems[i]);
   }

   svec->len += nelem;
}

void svec_insert(SVec *svec, size_t pos, const char *elem)
{
   svec_insert_n(svec, pos, &elem, 1);
}

void svec_push(SVec *svec, const char *elem)
{
   svec_insert_n(svec, svec->len, &elem, 1);
}

void svec_remove_n(SVec *svec, size_t pos, char **elems, size_t nelem, size_t max_len)
{
   if (pos + nelem - 1 >= svec->len)
      return;

   for (size_t i = 0; i < nelem; i++) {
      if (elems)
         nob_strncpy(elems[i], svec->ptr[pos + i], max_len);
      free(svec->ptr[pos + i]);
   }

   if (pos + nelem < svec->len)
      memmove(&svec->ptr[pos], &svec->ptr[pos + nelem], (svec->len - (pos + nelem)) * sizeof(char *));
   svec->len -= nelem;
}

void svec_remove(SVec *svec, size_t pos, char *elem, size_t max_arg)
{
   svec_remove_n(svec, pos, &elem, 1, max_arg);
}

void svec_pop(SVec *svec, char *elem, size_t max_arg)
{
   svec_remove_n(svec, svec->len - 1, &elem, 1, max_arg);
}

char *svec_get(SVec *svec, size_t pos)
{
   if (pos < svec->len)
      return svec->ptr[pos];
   return NULL;
}

void svec_set(SVec *svec, size_t pos, const char *elem)
{
   if (pos < svec->len) {
      free(svec->ptr[pos]);
      svec->ptr[pos] = nob_strdup(elem);
   }
}

void svec_merge(SVec *svec, SVec *other)
{
   svec_reserve(svec, svec->len + other->len);
   memcpy(&svec->ptr[svec->len], other->ptr, other->len * sizeof(char *));
   other->len = 0;
}

void svec_split(SVec *svec, SVec *other, size_t at)
{
   memset(other, 0, sizeof(*other));

   if (at < svec->len) {
      svec_reserve(other, svec->len - at);
      memcpy(other->ptr, &svec->ptr[at], (svec->len - at) * sizeof(char *));
      svec->len = at;
   }
}

bool svec_find(SVec *svec, const char *needle, size_t *ppos)
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

/**
 * MARK: Logging
 */
static int _nob_disable_log = 0;

void nob_log_off()
{
   _nob_disable_log++;
}

void nob_log_on()
{
   if (_nob_disable_log)
      _nob_disable_log--;
}

#define nob_log(lvl, msg)       _nob_log(__FILE__, __LINE__, __func__, (lvl), (msg))
#define nob_logf(lvl, fmt, ...) _nob_logf(__FILE__, __LINE__, __func__, (lvl), (fmt), __VA_ARGS__)

void _nob_logf(const char *file, int line, const char *func, LogLvl lvl, const char *fmt, ...)
{
   if (_nob_disable_log)
      return;

   switch (lvl) {
      case LOG_INFO:
         fprintf(stderr, "[I] ");
         break;
      case LOG_WARNING:
         fprintf(stderr, "[W] ");
         break;
      case LOG_ERROR:
         fprintf(stderr, "[E] ");
         break;
      case LOG_DONT_LOG:
      default:
         return;
   }

   fprintf(stderr, "%s::%s::%d: ", file, func, line);

   va_list args;
   va_start(args, fmt);
   vfprintf(stderr, fmt, args);
   va_end(args);
   fprintf(stderr, "\n");
}

void _nob_log(const char *file, int line, const char *func, LogLvl lvl, const char *msg)
{
   _nob_logf(file, line, func, lvl, msg);
}

/**
 * MARK: Generic utils
 */

char *nob_strdup(const char *str)
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

char *nob_msgerr()
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
 * MARK: Filesystem API
 */
int nob_get_mtime(const char *path, FILETIME *mtime)
{
   HANDLE fd =
      CreateFileA(path, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, NULL);
   if (fd == INVALID_HANDLE_VALUE) {
      nob_log(LOG_ERROR, nob_msgerr());
      return -1;
   }
   WINBOOL okay = GetFileTime(fd, NULL, NULL, mtime);
   CloseHandle(fd);
   if (!okay) {
      nob_log(LOG_ERROR, nob_msgerr());
      return -1;
   }
   return 0;
}

int nob_cmp_time(FILETIME *t1, FILETIME *t2)
{
   return (int)CompareFileTime(t1, t2);
}

bool nob_isdir(const char *path)
{
   DWORD attr = GetFileAttributesA(path);
   return attr != INVALID_FILE_ATTRIBUTES && (attr & FILE_ATTRIBUTE_DIRECTORY);
}

bool nob_isfile(const char *path)
{
   DWORD attr = GetFileAttributesA(path);
   return attr != INVALID_FILE_ATTRIBUTES && !(attr & FILE_ATTRIBUTE_DIRECTORY);
   //struct stat s;
   //if( stat(path,&s) == 0 )
   //if( s.st_mode & S_IFDIR )
   //else if( s.st_mode & S_IFREG )
}

bool nob_mkdir(const char *path)
{
   int res = _mkdir(path);
   if (res < 0 && (errno != EEXIST || !nob_isdir(path))) {
      nob_logf(LOG_ERROR, "Couldn't create directory %s: %s", path, strerror(errno));
      return false;
   }
   return true;
}

#define LS_ONLY_DIRS  0x01
#define LS_ONLY_FILES 0x02

int nob_ls(const char *parent, SVec *children, int flags)
{
   int  err = 0;
   char pattern[MAX_PATH];

   snprintf(pattern, MAX_PATH, "%s\\*", parent);

   WIN32_FIND_DATAA find_data;
   HANDLE           find_hnd = FindFirstFileA(pattern, &find_data);
   if (find_hnd == INVALID_HANDLE_VALUE) {
      nob_log(LOG_ERROR, nob_msgerr());
      return -1;
   }

   svec_reset(children);
   do {
      if (!flags
          || ((flags & LS_ONLY_DIRS) && (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
          || ((flags & LS_ONLY_FILES) && !(find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)))
      {
         svec_push(children, find_data.cFileName);
      }
   } while (FindNextFileA(find_hnd, &find_data));

   if (GetLastError() != ERROR_NO_MORE_FILES) {
      nob_log(LOG_ERROR, nob_msgerr());
      err = -1;
   }

   if (!FindClose(find_hnd)) {
      nob_log(LOG_WARNING, nob_msgerr());
      err = -1;
   }

   return err;
}

int nob_cp(const char *src, const char *dst)
{
   int err = 0;

   if (!CopyFileA(src, dst, FALSE)) {
      nob_log(LOG_ERROR, nob_msgerr());
      err = -1;
   }

   return err;
}

int nob_del(const char *path)
{
   if (!DeleteFileA(path)) {
      nob_log(LOG_ERROR, nob_msgerr());
      return -1;
   }

   return 0;
}