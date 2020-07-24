#ifndef	COMPILER_H
#define	COMPILER_H


#if defined(WIN32)


#ifndef	__packed
#define	__packed
#endif
#ifndef	__aligned
#define	__aligned(x)
#endif
#ifndef	__iomem
#define	__iomem		volatile
#endif

#define	__cacheline_aligned
#define	____cacheline_aligned

#else // #if defined(WIN32)


#ifndef	__packed
#define	__packed		__attribute__((packed))
#endif
#ifndef	__aligned
#define	__aligned(x)		__attribute__((aligned(x)))
#endif
#ifndef	__iomem
#define	__iomem
#endif


/* Indirect	macros required	for	expanded argument pasting, eg. __LINE__. */
#define	___PASTE(a,b)	    	a##b
#define	__PASTE(a,b)		___PASTE(a,b)

/* Not-quite-unique	ID.	*/
#ifndef	__UNIQUE_ID
# define __UNIQUE_ID(prefix)	__PASTE(__PASTE(__UNIQUE_ID_, prefix),	__LINE__)
#endif

#define	CACHE_LINE_BYTES	(32)

#ifndef	__cacheline_aligned
#define	__cacheline_aligned	__aligned(CACHE_LINE_BYTES)
#endif
#ifndef	____cacheline_aligned
#define	____cacheline_aligned	__aligned(CACHE_LINE_BYTES)
#endif

#define	____cacheline_aligned_in_smp

#define BUILD_BUG_ON(x)		do {} while(0)


#endif // #if defined(WIN32)

#endif // COMPILER_H
