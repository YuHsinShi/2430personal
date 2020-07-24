/*
 * BriefLZ - small fast Lempel-Ziv
 *
 * C packer
 *
 * Copyright (c) 2002-2015 Joergen Ibsen
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 *   1. The origin of this software must not be misrepresented; you must
 *      not claim that you wrote the original software. If you use this
 *      software in a product, an acknowledgment in the product
 *      documentation would be appreciated but is not required.
 *
 *   2. Altered source versions must be plainly marked as such, and must
 *      not be misrepresented as being the original software.
 *
 *   3. This notice may not be removed or altered from any source
 *      distribution.
 */

#include <stdio.h> 
#include "myLZ_comp.h"

extern int en_debug;
extern FILE *debug_file;

static void myLZ_putbit(struct myLZ_state *bs, unsigned int bit) {
  /* Check if tag is full */
  if (!bs->bits_left--) {
    /* store tag */
    bs->tagpos[0] = (bs->tag >> 8) & 0x00FF;
    bs->tagpos[1] = bs->tag & 0x00FF;

    /* init next tag */
    bs->tagpos = bs->dst;
    bs->dst += 2;
    bs->bits_left = 15;
  }

  /* Shift bit into tag */
  bs->tag = (bs->tag << 1) + bit;
}

static unsigned long myLZ_hash4(const unsigned char *s, int hashTableSizeBits) {
  unsigned long val = (unsigned long) s[0]
        | ((unsigned long) s[1] << 8)
        | ((unsigned long) s[2] << 16)
        | ((unsigned long) s[3] << 24);

  return ((val * 2654435761UL) & 0xFFFFFFFFUL) >> (32 - hashTableSizeBits);
}

unsigned long myLZ_workmem_size(unsigned long hashTableSizeBits) {
  int hashTableSize = 1 << hashTableSizeBits;

  return hashTableSize * sizeof(unsigned long);
}

unsigned long myLZ_max_packed_size(unsigned long src_size) {
  return src_size + src_size / 8 + 8;
}

unsigned long tableExpGolomb[2][14] = { // [k = 0/1][...]
  // k = 0
  { 0, 2, 6, 14, 30, 62, 126, 254, 510, 1022, 2046, 4094, 8190, 16382},
  // k = 1
  { 1, 5, 13, 29, 61, 125, 253, 509, 1021, 2045, 4093, 8189, 16381, 32765}
};

static void myLZ_putExpGolomb(struct myLZ_state *bs, unsigned long val, int k) {
  int i1, len0, len1;

  len0 = 0;

  while (val > tableExpGolomb[k][len0]) len0++;
  len1 = len0 + k;

  for (i1 = 0; i1 < len0; i1++) myLZ_putbit(bs, 0);
  myLZ_putbit(bs, 1);
  if (len0 > 0) val = val - (tableExpGolomb[k][len0 - 1] + 1);
  for (i1 = len1 - 1; i1 >= 0; i1--)
    myLZ_putbit(bs, (val >> i1) & 1);
}


unsigned long myLZ_pack(const void *src, void *dst, unsigned long src_size, void *workmem, myLZParaType *para) {
  int i1;
  struct myLZ_state bs;
  const unsigned char **lookup = (const unsigned char **) workmem;
  const unsigned char *prevsrc = (const unsigned char *) src;
  unsigned long src_avail = src_size;
  unsigned long hash4, hash_match;

  /* Check for empty input */
  if (src_avail == 0) {
    return 0;
  }

  /* Initialize lookup[] */
  for (i1 = 0; i1 < (1 << para->hashTableSizeBits); i1++) lookup[i1] = 0;

  bs.src = (const unsigned char *) src;
  bs.dst = (unsigned char *) dst;

  if (para->bytePerUnit == 1) {
    /* First byte verbatim */
    *bs.dst++ = *bs.src++;
    if (en_debug) {
      fprintf(debug_file, "f: pos =    0, %02X, src_avail = %d\n", *(bs.dst - 1), src_avail - 1);
    }
    /* Check for 1 byte input */
    if (--src_avail == 0) return 1;
  } else if (para->bytePerUnit == 2) {
    /* First 16 bit */
    *bs.dst++ = *bs.src++;
    *bs.dst++ = *bs.src++;
    if (en_debug) {
      fprintf(debug_file, "f: pos =    0, %02X %02X, src_avail = %d\n", *(bs.dst - 2), *(bs.dst - 1), src_avail - 2);
    }
    /* Check for 1 unit input */
	if(src_avail < 2)	printf("	### err2: src_avail < 2(%d)\n", src_avail);
    src_avail -= 2;
    if (src_avail == 0)	return 2;
  } else { // para->bytePerUnit == 4
    /* First 32 bit */
    *bs.dst++ = *bs.src++;
    *bs.dst++ = *bs.src++;
    *bs.dst++ = *bs.src++;
    *bs.dst++ = *bs.src++;
    if (en_debug) {
      fprintf(debug_file, "f: pos =    0, %02X %02X %02X %02X, src_avail = %d\n", *(bs.dst - 4), *(bs.dst - 3), *(bs.dst - 2), *(bs.dst - 1), src_avail - 4);
    }
    /* Check for 1 unit input */
	if(src_avail < 4)	printf("	### err3: src_avail < 4(%d)\n", src_avail);
    src_avail -= 4;
    if (src_avail == 0) return 4;
  }

  /* Initialize first tag */
  bs.tagpos = bs.dst;
  bs.dst += 2;
  bs.tag = 0;
  bs.bits_left = 16;

  /* Main compression loop */
  while (src_avail >= 4) {
    const unsigned char *p;
    unsigned long len = 0;

    /* Update lookup[] up to current position */
    while (prevsrc < bs.src) {
      lookup[myLZ_hash4(prevsrc, para->hashTableSizeBits)] = prevsrc;

      prevsrc += para->bytePerUnit;
    }

    /* Look up current position */
    p = lookup[hash4 = myLZ_hash4(bs.src, para->hashTableSizeBits)];
    hash_match = (unsigned long)p - (unsigned long)src;

    /* Check match */
    if (p) {
      if (para->bytePerUnit == 1) {
        while (len < src_avail && p[len] == bs.src[len]) ++len;
      } else if (para->bytePerUnit == 2) {
        while (len < (src_avail-1) && *((unsigned short *)&(p[len])) == *((unsigned short *)&(bs.src[len]))) len += 2;
      } else { // para->bytePerUnit == 4
        while (len < (src_avail-3) && *((unsigned int *)&(p[len])) == *((unsigned int *)&(bs.src[len]))) len +=4;
      }
    }


    /* Output match or literal */
    if (len > 3) {
      unsigned long off;
      int coded_length, comb;
      if (para->bytePerUnit == 1) {
        off = (unsigned long) (bs.src - p - 1);
        coded_length = len - 4;
      } else if (para->bytePerUnit == 2) {
        off = (unsigned long)(((bs.src - p) >> 1) - 1);
        coded_length = (len >> 1) - 2;
      } else { // para->bytePerUnit == 4
        off = (unsigned long)(((bs.src - p) >> 2) - 1);
        coded_length = (len >> 2) - 1;
      }

      if (en_debug) {
        fprintf(debug_file, "m: pos = %4d, hash4 = %04X, match_pos = %04X, off = %4d, coded_offset = %4d, len = %4d, coded_length = %4d, src_avail =%4d\n", bs.src - (unsigned long)src, hash4, hash_match, bs.src - p, off, len, coded_length, src_avail - len);
      }

      /* Output match tag */
      myLZ_putbit(&bs, 0);

      // [SmallY20160308] change the order of offset & length
      switch(para->codingMode) {
        case 0: // GG8
          // Output match offset
          myLZ_putExpGolomb(&bs, off >> 8, 1);
          *bs.dst++ = off & 0x00FF;

          // Output match length
          myLZ_putExpGolomb(&bs, coded_length, 1);
          break;

        case 1: // EE8
          // Output match offset
          myLZ_putExpGolomb(&bs, off >> 8, 0);
          *bs.dst++ = off & 0x00FF;

          // Output match length2
          myLZ_putExpGolomb(&bs, coded_length, 0);
          break;

        case 2: // G1G7
          comb = ((coded_length & 1) << 7) | (off & 0x07F);

          // Output match offset
          myLZ_putExpGolomb(&bs, off >> 7, 1);
          *bs.dst++ = comb & 0x00FF;

          // Output match length
          myLZ_putExpGolomb(&bs, coded_length >> 1, 1);
          break;

        case 3: // E1E7
          comb = ((coded_length & 1) << 7) | (off & 0x07F);

          // Output match offset
          myLZ_putExpGolomb(&bs, off >> 7, 0);
          *bs.dst++ = comb & 0x00FF;

          // Output match length
          myLZ_putExpGolomb(&bs, coded_length >> 1, 0);
          break;
        case 4: // G2G6
          comb = ((coded_length & 3) << 6) | (off & 0x03F);

          // Output match offset
          myLZ_putExpGolomb(&bs, (off >> 6), 1);
          *bs.dst++ = comb & 0x00FF;

          // Output match length
          myLZ_putExpGolomb(&bs, coded_length >> 2, 1);
          break;
        case 5: // E2E6
          comb = ((coded_length & 3) << 6) | (off & 0x03F);

          // Output match offset
          myLZ_putExpGolomb(&bs, (off >> 6), 0);
          *bs.dst++ = comb & 0x00FF;

          // Output match length
          myLZ_putExpGolomb(&bs, coded_length >> 2, 0);
          break;

        case 6: // length < 8 G2G5 no length Golomb,length >= 8 G1G6 
            if (len <= (4+para->bytePerUnit)){
              comb = ((coded_length &0x01) << 6) | (off & 0x03F) | 0x80;
              
              // Output match offset
              myLZ_putExpGolomb(&bs, (off >> 6), 1);
              *bs.dst++ = comb & 0x00FF;

              break;
            } else{
              // Output match offset

              comb = ((coded_length & 0x01) << 6) | (off & 0x03F);

              // Output match offset
              myLZ_putExpGolomb(&bs, (off >> 6), 1);
              *bs.dst++ = comb & 0x007F;

              // Output match length
              myLZ_putExpGolomb(&bs, (coded_length >> 1), 1);
              break;
            }

         }

      bs.src += len;
	  if(src_avail < len)	printf("	### err1:(%d,%d) src_avail < len\n", src_avail, len);
      src_avail -= len;
//compCount++;
    } else {
      /* Output literal tag */
      myLZ_putbit(&bs, 1);

      /* Copy literal */
      if (para->bytePerUnit == 1) {
        *bs.dst++ = *bs.src++;
        if (en_debug) {
          fprintf(debug_file, "n: pos = %4d, %02X, src_avail = %d\n", bs.src - (unsigned long)src - 1, *(bs.dst - 1), src_avail - 1);
        }
      } else if (para->bytePerUnit == 2) {
        *bs.dst++ = *bs.src++;
        *bs.dst++ = *bs.src++;
        if (en_debug) {
          fprintf(debug_file, "n: pos = %4d, %02X %02X, src_avail = %d\n", bs.src - (unsigned long)src - 1, *(bs.dst - 2), *(bs.dst - 1), src_avail - 2);
        }
      } else { // para->bytePerUnit == 4
        *bs.dst++ = *bs.src++;
        *bs.dst++ = *bs.src++;
        *bs.dst++ = *bs.src++;
        *bs.dst++ = *bs.src++;
        if (en_debug) {
          fprintf(debug_file, "n: pos = %4d, %02X %02X %02X %02X, src_avail = %d\n", bs.src - (unsigned long)src - 1, *(bs.dst - 4), *(bs.dst - 3), *(bs.dst - 2), *(bs.dst - 1), src_avail - 4);
        }
      }
	  if(src_avail < para->bytePerUnit)	printf("	### err0: src_avail < bytePerUnit(%d,%d)\n", src_avail, para->bytePerUnit);
      src_avail -= para->bytePerUnit;
    }
//totalCount++;
  }

  /* Output any remaining literals */
  while (src_avail > 0) {
    /* Output literal tag */
    myLZ_putbit(&bs, 1);
    if (src_avail >= para->bytePerUnit ) {
        /* Copy literal */
        if (para->bytePerUnit == 1) {
          *bs.dst++ = *bs.src++;
          if (en_debug) {
            fprintf(debug_file, "n: pos = %4d, %02X, src_avail = %d\n", bs.src - (unsigned long)src - 1, *(bs.dst - 1), src_avail - 1);
          }
        } else if (para->bytePerUnit == 2) {
          *bs.dst++ = *bs.src++;
          *bs.dst++ = *bs.src++;
          if (en_debug) {
            fprintf(debug_file, "n: pos = %4d, %02X %02X, src_avail = %d\n", bs.src - (unsigned long)src - 1, *(bs.dst - 2), *(bs.dst - 1), src_avail - 2);
          }
        } else { // para->bytePerUnit == 4
          *bs.dst++ = *bs.src++;
          *bs.dst++ = *bs.src++;
          *bs.dst++ = *bs.src++;
          *bs.dst++ = *bs.src++;
          if (en_debug) {
            fprintf(debug_file, "n: pos = %4d, %02X %02X %02X %02X, src_avail = %d\n", bs.src - (unsigned long)src - 1, *(bs.dst - 4), *(bs.dst - 3), *(bs.dst - 2), *(bs.dst - 1), src_avail - 4);
          }
        }
		if(src_avail < para->bytePerUnit)	printf("	### err: src_avail < bytePerUnit(%d,%d)\n", src_avail, para->bytePerUnit);
        src_avail -= para->bytePerUnit; 
    } else {
        /* Copy literal */
        if (src_avail == 1) {
          *bs.dst++ = *bs.src++;
          if (en_debug) {
            fprintf(debug_file, "n: pos = %4d, %02X, src_avail = %d\n", bs.src - (unsigned long)src - 1, *(bs.dst - 1), src_avail - 1);
          }
        } else if (src_avail == 2) {
          *bs.dst++ = *bs.src++;
          *bs.dst++ = *bs.src++;
          if (en_debug) {
            fprintf(debug_file, "n: pos = %4d, %02X %02X, src_avail = %d\n", bs.src - (unsigned long)src - 1, *(bs.dst - 2), *(bs.dst - 1), src_avail - 2);
          }
        } else { // para->bytePerUnit == 4
		
		  //if(src_avail==3)	printf("	### err: src_avail==3\n");
          *bs.dst++ = *bs.src++;
          *bs.dst++ = *bs.src++;
          *bs.dst++ = *bs.src++;
          if (en_debug) {
            fprintf(debug_file, "n: pos = %4d, %02X %02X %02X, src_avail = %d\n", bs.src - (unsigned long)src - 1, *(bs.dst - 3), *(bs.dst - 2), *(bs.dst - 1), src_avail - 3);
          }
        }
        src_avail = 0; 
    }
  }
  /* Shift last tag into position and store */
  bs.tag <<= bs.bits_left;
  bs.tagpos[0] = (bs.tag >> 8) & 0x00FF;
  bs.tagpos[1] = bs.tag & 0x00FF;

  /* Return compressed size */
  return (unsigned long) (bs.dst - (unsigned char *) dst);
}
