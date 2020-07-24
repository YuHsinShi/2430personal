#ifndef ITP_MALLOC_H
#define ITP_MALLOC_H

#ifdef malloc
#undef malloc
#endif

#ifdef realloc
#undef realloc
#endif

#ifdef calloc
#undef calloc
#endif

#ifdef free
#undef free
#endif

#ifdef memalign
#undef memalign
#endif

#include_next <malloc.h>

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef CFG_DBG_RMALLOC

#define FUNCTIONIZE(a,b)  a(b)
#define STRINGIZE(a)      #a
#define INT2STRING(i)     FUNCTIONIZE(STRINGIZE,i)
#define RM_FILE_POS       __FILE__ ":" INT2STRING(__LINE__)

void *Rmalloc(size_t size, const char *file);
void *Rcalloc(size_t nelem, size_t size, const char *file);
void *Rrealloc(void *p, size_t size, const char *file);
void  Rfree(void *p, const char *file);
void *Rmemalign(size_t align, size_t size, const char *file);

#  ifdef malloc
#    undef malloc
#  endif
#  define malloc(s)       Rmalloc((s), RM_FILE_POS)

#  ifdef calloc
#    undef calloc
#  endif
#  define calloc(n,s)     Rcalloc((n), (s), RM_FILE_POS)

#  ifdef realloc
#    undef realloc
#  endif
#  define realloc(p,s)    Rrealloc((p), (s), RM_FILE_POS)

#  ifdef free
#    undef free
#  endif
#  define free(p)         Rfree((p), RM_FILE_POS)

#  ifdef memalign
#    undef memalign
#  endif
#  define memalign(a, s)  Rmemalign((a), (s), RM_FILE_POS)

#endif // CFG_DBG_RMALLOC

#ifdef __cplusplus
}
#endif

#endif // ITP_MALLOC_H
