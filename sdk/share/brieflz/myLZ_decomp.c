/*
 * BriefLZ - small fast Lempel-Ziv
 *
 * C depacker
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

static unsigned int myLZ_getbit(struct myLZ_state *bs) {
  unsigned int bit;

  /* Check if tag is empty */
  if (!bs->bits_left--) {
    /* Load next tag */
    bs->tag = ((unsigned int)bs->src[0] << 8)
        | (unsigned int)bs->src[1];
    bs->src += 2;
    bs->bits_left = 15;
  }

  /* Shift bit out of tag */
  bit = (bs->tag & 0x8000) ? 1 : 0;
  bs->tag <<= 1;

  return bit;
}

#if 0
static unsigned long blz_getgamma(struct myLZ_state *bs) {
	unsigned long result = 1;

	/* Input gamma2-encoded bits */
	do {
		result = (result << 1) + blz_getbit(bs);
	} while (blz_getbit(bs));

	return result;
}
#endif

extern unsigned long tableExpGolomb[2][14];

static unsigned long myLZ_getExpGolomb(struct myLZ_state *bs, int k) {
  int i1, len0, len1;
  unsigned long val;

  len0 = 0;

  while (myLZ_getbit(bs) == 0)
    len0++;
  len1 = len0 + k;
  val = 0;
  for (i1 = 0; i1 < len1; i1++)
    val = (val << 1) | myLZ_getbit(bs);
  if (len0 > 0) val += tableExpGolomb[k][len0 - 1] + 1;

  return val;
}


unsigned long myLZ_depack(const void *src, void *dst, unsigned long depacked_size,unsigned int packblocksize ,myLZParaType *para) {
  struct myLZ_state bs;
  unsigned long dst_size;
  unsigned long outsize =0;
  unsigned int bloclsize;
  unsigned int error = 0;

  /* Check for empty input */
  if (depacked_size == 0) return 0;

  bs.src = (const unsigned char *) src;
  bs.dst = (unsigned char *) dst;
  
  /* Main decompression loop */
  while (outsize < depacked_size) {
    bs.bits_left = 0;
   
    if((depacked_size - outsize) > packblocksize){
        bloclsize = packblocksize;
    } else {
        bloclsize = (depacked_size - outsize);
    }
   if (para->bytePerUnit == 1) {
      /* First byte verbatim */
      *bs.dst++ = *bs.src++;
      dst_size = 1;
      if (en_debug) {
        fprintf(debug_file, "f: %02X, dst_size = %4d\n", *(bs.dst - 1), dst_size);
      }
    } else if (para->bytePerUnit == 2) {
      /* First 16 bit */
      *bs.dst++ = *bs.src++;
      *bs.dst++ = *bs.src++;
      dst_size = 2;
      if (en_debug) {
        fprintf(debug_file, "f: %02X %02X, dst_size = %4d\n", *(bs.dst - 2), *(bs.dst - 1), dst_size);
      }
    } else { // para->bytePerUnit == 4
      /* First 32 bit */
      *bs.dst++ = *bs.src++;
      *bs.dst++ = *bs.src++;
      *bs.dst++ = *bs.src++;
      *bs.dst++ = *bs.src++;
      dst_size = 4;
      if (en_debug) {
        fprintf(debug_file, "f: %02X %02X %02X %02X, dst_size = %4d\n", *(bs.dst - 4), *(bs.dst - 3), *(bs.dst - 2), *(bs.dst - 1), dst_size);
      }
    }

   while (dst_size < bloclsize) {
       
      if (myLZ_getbit(&bs) == 0) {
        /* Input match length and offset */
        unsigned long coded_length, len, coded_offset, off, comb;

        // [SmallY20160308] change the order of offset & length
        switch (para->codingMode) {
          case 0: // GG8
            coded_offset = myLZ_getExpGolomb(&bs, 1);
            coded_offset = (coded_offset << 8) + (unsigned long) *bs.src++;
            coded_length = myLZ_getExpGolomb(&bs, 1);
            break;

          case 1: // EE8
            coded_offset = myLZ_getExpGolomb(&bs, 0);
            coded_offset = (coded_offset << 8) + (unsigned long) *bs.src++;
            coded_length = myLZ_getExpGolomb(&bs, 0);
            break;

          case 2: // G1G7
            coded_offset = myLZ_getExpGolomb(&bs, 1);
            comb = (unsigned long) *bs.src++;
            coded_length = myLZ_getExpGolomb(&bs, 1);

            coded_length = (coded_length << 1) | (comb >> 7);
            coded_offset = (coded_offset << 7) | (comb & 0x07F);
            break;

          case 3: // E1E7
            coded_offset = myLZ_getExpGolomb(&bs, 0);
            comb = (unsigned long) *bs.src++;
            coded_length = myLZ_getExpGolomb(&bs, 0);

            coded_length = (coded_length << 1) | (comb >> 7);
            coded_offset = (coded_offset << 7) | (comb & 0x07F);
            break;
          case 4: // G2G6
            coded_offset = myLZ_getExpGolomb(&bs, 1);
            comb = (unsigned long) *bs.src++;
            coded_length = myLZ_getExpGolomb(&bs, 1);

            coded_length = (coded_length << 2) | (comb >> 6);
            coded_offset = (coded_offset << 6) | (comb & 0x03F);
            break;
          case 5: // E2E6
            coded_offset = myLZ_getExpGolomb(&bs, 0);
            comb = (unsigned long) *bs.src++;
            coded_length = myLZ_getExpGolomb(&bs, 0);

            coded_length = (coded_length << 2) | (comb >> 6);
            coded_offset = (coded_offset << 6) | (comb & 0x03F);
            break;

        case 6: // length < 8 G1G6 no length Golomb,length >= 8 G1G6 
            coded_offset = myLZ_getExpGolomb(&bs, 1);
            comb = (unsigned long) *bs.src++;
            if (comb & 0x80) {
                coded_offset = (coded_offset << 6) + (comb & 0x03F);
                coded_length = (comb >> 6) & 0x001;
                break; 
            } else {
                coded_length = myLZ_getExpGolomb(&bs, 1);
                coded_offset = (coded_offset << 6) + (comb & 0x03F);
                coded_length = (coded_length << 1) + ((comb >> 6) & 0x001);
                break; 
            }
        }

      if (para->bytePerUnit == 1) {
          len = coded_length + 4;
          off = coded_offset + 1;
      } else if (para->bytePerUnit == 2) {
        len = (coded_length + 2) << 1;
        off = (coded_offset + 1) << 1;
      } else { // para->bytePerUnit == 4
        len = (coded_length + 1) << 2;
        off = (coded_offset + 1) << 2;
      }

          if (off > dst_size) {
              error = 1;
              printf("CPD ERR");
              break;
          }
        if (en_debug) {
          fprintf(debug_file, "c: coded_length = %4d, extra_byte = %02X, coded_offset = %4d, len = %4d, off = %4d\n ", coded_length, *(bs.src - 1), coded_offset, len, off);
//        fprintf(debug_file, "  output unit:");
      }

        /* Copy match */
        {
          const unsigned char *p = bs.dst - off;
          unsigned long i;

          for (i = len; i > 0; --i) {
            *bs.dst++ = *p++;
            if (en_debug) {
              fprintf(debug_file, " %02X", *(bs.dst -1));
              if (len > 32)
                len += 0;
              if (((len - i + 1) & 0x1F) == 0)
                fprintf(debug_file, "\n ");
            }
          }
        }

        dst_size += len;
        if (en_debug) {
          fprintf(debug_file, ", dst_size = %4d\n", dst_size);
        }
      } else {
         if ((dst_size + para->bytePerUnit) > bloclsize) {
              while (dst_size < bloclsize) {
                *bs.dst++ = *bs.src++;
                dst_size++;
                if (en_debug) {
                  fprintf(debug_file, "n: %02X, dst_size = %4d\n", *(bs.dst - 1), dst_size);
                }
              }
          } else {
            if (para->bytePerUnit == 1) {
              /* Copy literal */
              *bs.dst++ = *bs.src++;
              dst_size++;
              if (en_debug) {
                fprintf(debug_file, "n: %02X, dst_size = %4d\n", *(bs.dst - 1), dst_size);
              }
            } else if (para->bytePerUnit == 2) {
              /* Copy literal */
              *bs.dst++ = *bs.src++;
              *bs.dst++ = *bs.src++;
              dst_size += 2;
              if (en_debug) {
                fprintf(debug_file, "n: %02X %02X, dst_size = %4d\n", *(bs.dst - 2), *(bs.dst - 1), dst_size);
              }
            } else { // para->bytePerUnit == 4
              *bs.dst++ = *bs.src++;
              *bs.dst++ = *bs.src++;
              *bs.dst++ = *bs.src++;
              *bs.dst++ = *bs.src++;
              dst_size += 4;
              if (en_debug) {
                fprintf(debug_file, "n: %02X %02X %02X %02X, dst_size = %4d\n",  *(bs.dst - 4), *(bs.dst - 3),*(bs.dst - 2), *(bs.dst - 1), dst_size);
              }
            }
         }
      }
    }
   //printf("dst_size = %d \n",dst_size);
    outsize += dst_size;
    if (error) {
        break;
    }
  }
  /* Return decompressed size */
  return outsize;
}
