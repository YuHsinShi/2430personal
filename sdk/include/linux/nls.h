#ifndef NLS_H
#define NLS_H

#include "ite/ith.h"
#include <linux/os.h>



typedef unsigned int unicode_t;

#define MAX_WCHAR_T	0xffff

/* Byte order for UTF-16 strings */
enum utf16_endian {
	UTF16_HOST_ENDIAN,
	UTF16_LITTLE_ENDIAN,
	UTF16_BIG_ENDIAN
};

extern int utf16s_to_utf8s(const u16 *pwcs, int inlen, enum utf16_endian endian,
			u8 *s, int maxout);

extern int utf8s_to_utf16s(const u8 *s, int len,
		enum utf16_endian endian, u16 *pwcs, int maxlen);


#endif // NLS_H
