#ifndef ITP_STRING_H
#define ITP_STRING_H

#include_next <string.h>

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef CFG_DBG_RMALLOC

#define FUNCTIONIZE(a,b)  a(b)
#define STRINGIZE(a)      #a
#define INT2STRING(i)     FUNCTIONIZE(STRINGIZE,i)
#define RM_FILE_POS       __FILE__ ":" INT2STRING(__LINE__)

char *Rstrdup(const char *str, const char *file);
char *Rstrndup(const char *str, size_t n, const char *file);

#  ifdef strdup
#    undef strdup
#  endif
#  define strdup(s)       Rstrdup((s), RM_FILE_POS)

#  ifdef strndup
#    undef strndup
#  endif
#  define strndup(s, n)   Rstrndup((s), (n), RM_FILE_POS)

#endif // CFG_DBG_RMALLOC

#ifdef __cplusplus
}
#endif

#endif // ITP_STRING_H
