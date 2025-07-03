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

void svec_insert_n(SVec *svec, size_t nelem, const char **elems, size_t pos)
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

void svec_insert(SVec *svec, const char *elem, size_t pos)
{
   svec_insert_n(svec, 1, &elem, pos);
}

void svec_push(SVec *svec, const char *elem)
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

void svec_set(SVec *svec, size_t pos, const char *elem)
{
   if (pos < svec->len) {
      free(svec->ptr[pos]);
      svec->ptr[pos] = nob_strdup(elem);
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

SVec *svec_from(SVec *svec, const char *s, ...)
{
   svec_reset(svec);

   va_list args;
   va_start(args, s);
   for (; s; s = va_arg(args, const char *)) {
      svec_push(svec, s);
   }
   va_end(args);

   return svec;
}

SVec *svec_tmp()
{
   static SVec svec = {0};
   return &svec;
}

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

void _nob_logf(LogLvl lvl, int line, const char *func, const char *fmt, ...)
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

void _nob_log(LogLvl lvl, int line, const char *func, const char *msg)
{
   _nob_logf(lvl, line, func, msg);
}

int nob_get_mtime(Priority priority, const char *path, FILETIME *mtime)
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

typedef struct OpenDir {
   HANDLE           find_hnd;
   WIN32_FIND_DATAA find_data;
   char             name[MAX_PATH + 1];
} OpenDir;

OpenDir *opendir(const char *dirpath)
{
   char buffer[MAX_PATH];
   snprintf(buffer, MAX_PATH, "%s\\*", dirpath);

   OpenDir *dir = (OpenDir *)calloc(1, sizeof(OpenDir));

   dir->find_hnd = FindFirstFileA(buffer, &dir->find_data);
   if (dir->find_hnd == INVALID_HANDLE_VALUE) {
      nob_log(LOG_ERROR, nob_win_errmsg());
      free(dir);
      dir = NULL;
   }

   return dir;
}

char *readdir(OpenDir *dirp)
{
   if (!FindNextFileA(dirp->find_hnd, &dirp->find_data)) {
      if (GetLastError() != ERROR_NO_MORE_FILES)
         nob_log(LOG_ERROR, nob_win_errmsg());
      return NULL;
   }

   nob_strncpy(dirp->name, dirp->find_data.cFileName, STR_SIZE(dirp->name));

   return dirp->name;
}

int closedir(OpenDir *dirp)
{
   int err = 0;

   if (!FindClose(dirp->find_hnd)) {
      nob_log(LOG_WARNING, nob_win_errmsg());
      err = -1;
   }

   free(dirp);

   return err;
}

#define LS_ONLY_DIRS  0x01
#define LS_ONLY_FILES 0x02

bool nob_ls(const char *parent, SVec *children, int flags)
{
   bool     result = true;
   OpenDir *dir = NULL;

   dir = opendir(parent);
   if (!dir)
      return false;

   errno = 0;
   DirEntry *ent = readdir(dir);
   while (ent != NULL) {
      nob_da_append(children, nob_temp_strdup(ent->name));
      ent = readdir(dir);
   }

   if (errno != 0) {
#ifdef _WIN32
      nob_log(
         NOB_ERROR,
         "Could not read directory %s: %s",
         parent,
         nob_win32_error_message(GetLastError())
      );
#else
      nob_log(NOB_ERROR, "Could not read directory %s: %s", parent, strerror(errno));
#endif  // _WIN32
      nob_return_defer(false);
   }

defer:
   if (dir)
      closedir(dir);
   return result;
}

bool nob_cp(const char *src, const char *dst)
{
   char *ps;
   if ((ps = strstr(src, "*")) != NULL) {
   }
   else {
      if (!CopyFileA(src, dst, FALSE))
         nob_win_errmsg();
   }
}

void nob_compile(SVec *prereqs, SVec *recipe)
{
   svec_reset(prereqs);
   svec_reset(recipe);
}