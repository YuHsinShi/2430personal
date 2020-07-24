
/*
 * This is a simple example packer, which can compress and decompress a
 * single file using BriefLZ.
 *
 * It processes the data in blocks of 1024k. Adjust BLOCK_SIZE to 56k or less
 * to compile for 16-bit.
 *
 * Each compressed block starts with a 24 byte header with the following
 * format:
 *
 *   - 32-bit signature (string "blz",0x1A)
 *   - 32-bit format version (1 in current version)
 *   - 32-bit size of compressed data following header
 *   - 32-bit CRC32 value of compressed data, or 0
 *   - 32-bit size of original uncompressed data
 *   - 32-bit CRC32 value of original uncompressed data, or 0
 *
 * All values in the header are stored in network order (big endian, most
 * significant byte first), and are read and written using the read_be32()
 * and write_be32() functions.
 */

#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include <math.h>

#include "myLZ_comp.h"

/*
 * The block-size used to process data.
 */
//#define BLOCK_SIZE (1024 * 1024UL)
//#define BLOCK_SIZE (2 * 1024UL)
//#define BLOCK_SIZE (8192UL)

/*
 * The size of the block header.
 */
#define HEADER_SIZE (14)

/*
 * Unsigned char type.
 */
typedef unsigned char byte;

int en_debug;
FILE *debug_file = NULL;
#if 0
/*
 * Get the low-order 8 bits of a value.
 */
#if CHAR_BIT == 8
	#define octet(v) ((byte) (v))
#else
	#define octet(v) ((v) & 0x00FF)
#endif

/*
 * Store a 32-bit unsigned value in network order.
 */
static void write_be32(byte *p, unsigned long val) {
  p[0] = octet(val >> 24);
  p[1] = octet(val >> 16);
  p[2] = octet(val >> 8);
  p[3] = octet(val);
}

/*
 * Read a 32-bit unsigned value in network order.
 */
static unsigned long read_be32(const byte *p) {
  return  ((unsigned long) octet(p[0]) << 24)
        | ((unsigned long) octet(p[1]) << 16)
        | ((unsigned long) octet(p[2]) << 8)
        | ((unsigned long) octet(p[3]));
}

static unsigned int ratio(unsigned long x, unsigned long y) {
  if (x <= ULONG_MAX / 100) {
    x *= 100;
  } else {
    y /= 100;
  }

  if (y == 0) {
    y = 1;
  }

  return (unsigned int) (x / y);
}

static int compress_file(myLZParaType *para) {
  byte header[HEADER_SIZE] = {'I', 'T', 'E', 1};
  FILE *src_file = NULL;
  FILE *dest_file = NULL;
  byte *data = NULL;
  byte *packed = NULL;
  byte *workmem = NULL;
  byte *totalpack = NULL;
  unsigned int srcfilesize;
  unsigned long insize = 0, outsize = 0;
  size_t n_read;
  clock_t clocks;
  int res = 0;
  int block_num = 0;
  int i;

  /* Allocate memory */
  if ((data = (byte *) malloc(para->blockSize)) == NULL
        || (packed = (byte *) malloc(myLZ_max_packed_size(para->blockSize))) == NULL
        || (workmem = (byte *) malloc(myLZ_workmem_size(para->hashTableSizeBits))) == NULL) {
    printf("ERR: not enough memory\n");
    res = 1;
    goto out;
  }

  /* Open input file */
  if ((src_file = fopen(para->src_filename, "rb")) == NULL) {
    printf("ERR: unable to open input file\n");
    res = 1;
    goto out;
  }

  fseek(src_file,0,SEEK_END);
  srcfilesize = ftell(src_file);
  fseek(src_file,0,SEEK_SET);
  totalpack = (byte *) malloc(srcfilesize);

  /* Create output file */
  if ((dest_file = fopen(para->dest_filename, "wb")) == NULL) {
    printf("ERR: unable to open output file\n");
    res = 1;
    goto out;
  }

  clocks = clock();

  if (en_debug) {
    fprintf(debug_file, "source file: %s\n", para->src_filename);
    fprintf(debug_file, "destination file: %s\n", para->dest_filename);
    fprintf(debug_file, "hash table size: %d\n", 1 << para->hashTableSizeBits);
    fprintf(debug_file, "block size: %d\n", para->blockSize);
    fprintf(debug_file, "byte per unit: %d\n", para->bytePerUnit);
    fprintf(debug_file, "coding mode: %d\n", para->codingMode);
  }

  /* While we are able to read data from input file .. */
  while ((n_read = fread(data, 1, para->blockSize, src_file)) > 0) {
    size_t packedsize;

    if (en_debug) {
      fprintf(debug_file, "--------------------\nblock number: %d\n", block_num);
      fprintf(debug_file, "source size: %d\n", n_read);
    }

    /* Compress data block */
    packedsize = myLZ_pack(data, packed, (unsigned long) n_read, workmem, para);

    if (en_debug) {
      fprintf(debug_file, "packed size: %d\n", packedsize);
    }

    /* Check for compression error */
    if (packedsize == 0) {
      printf("ERR: an error occured while compressing\n");
      res = 1;
      goto out;
    }

   for (i=0;i < packedsize;i++) {
      totalpack[i+outsize + HEADER_SIZE] = packed[i];
   }
   

    /* Write header and compressed data */
   // fwrite(header, 1, sizeof(header), dest_file);
   // fwrite(packed, 1, packedsize, dest_file);

    /* Sum input and output size */
    insize += (unsigned long) n_read;
    outsize += (unsigned long) packedsize;
    block_num++;
  }

  totalpack[0]  = 'I';
  totalpack[1]  = 'T';
  totalpack[2]  = 'E';
  totalpack[3]  = (para->codingMode << 4) | (para->bytePerUnit);
  totalpack[4]  = (para->blockSize);
  totalpack[5]  = (para->blockSize) >> 8;
  totalpack[6]  = insize ;
  totalpack[7]  = insize >> 8;
  totalpack[8]  = insize >> 16;
  totalpack[9]  = insize >> 24;
  totalpack[10] = outsize;
  totalpack[11] = outsize >> 8;
  totalpack[12] = outsize >> 16;
  totalpack[13] = outsize >> 24;

  // add end mark
  fwrite(totalpack, 1,(outsize + HEADER_SIZE), dest_file);

  //write_be32(header + 4, 0);
  //fwrite(header, 1, sizeof(header), dest_file);
  //outsize += sizeof(header);
  if (en_debug) {
    fprintf(debug_file, "--------------------\nblock number: %d\n", block_num);
    fprintf(debug_file, "end mark\n");
  }

  clocks = clock() - clocks;

  /* Show result */
  printf("compressed %lu -> %lu bytes (%u%%) in %.2f seconds\n",
        insize, outsize, ratio(outsize, insize),
        (double) clocks / (double) CLOCKS_PER_SEC);

out:
  /* Close files */
  if (dest_file != NULL) fclose(dest_file);
  if (src_file != NULL) fclose(src_file);

  /* Free memory */
  if (workmem != NULL) free(workmem);
  if (packed != NULL) free(packed);
  if (data != NULL) free(data);

  return res;
}

static int decompress_file(myLZParaType *para) {
  byte header[HEADER_SIZE];
  FILE *newfile = NULL;
  FILE *packedfile = NULL;
  byte *data = NULL;
  byte *packed = NULL;
  byte *totalpacked = NULL;
  unsigned long insize = 0, outsize = 0;
  unsigned long srcfilesize , hdr_packedsize , hdr_depackedsize;
  unsigned int packblocksize;
  clock_t clocks;
  size_t max_packed_size;
  int res = 0;
  int block_num = 0;
  int i;


  /* Open input file */
  if ((packedfile = fopen(para->src_filename, "rb")) == NULL) {
    printf("ERR: unable to open input file\n");
    res = 1;
    goto out;
  }

  /* Create output file */
  if ((newfile = fopen(para->dest_filename, "wb")) == NULL) {
    printf("ERR: unable to open output file\n");
    res = 1;
    goto out;
  }

  clocks = clock();


  fseek(packedfile,0,SEEK_END);
  srcfilesize = ftell(packedfile);
  fseek(packedfile,0,SEEK_SET);  

  fread(header, 1, sizeof(header), packedfile);
  
  /* Verify values in header */
  if ((read_be32(header + 0 * 4) >> 8) != 0x495445 )/* "ITE" */
  {
    printf("ERR: invalid header in compressed file\n");
    res = 1;
    goto out;
  }

  para->bytePerUnit = header[3] & 0x0F;
  para->codingMode = header[3] >> 4;

  packblocksize    = ((unsigned int)header[5] << 8)  | (unsigned int)header[4];
  hdr_depackedsize = ((unsigned long)header[9] << 24)  | ((unsigned long)header[8] << 16)  | ((unsigned long)header[7] << 8)  | (unsigned long)header[6];
  hdr_packedsize   = ((unsigned long)header[13] << 24) | ((unsigned long)header[12] << 16) | ((unsigned long)header[11] << 8) | (unsigned long)header[10];

  if (en_debug) {
    fprintf(debug_file, "source file: %s\n", para->src_filename);
    fprintf(debug_file, "destination file: %s\n", para->dest_filename);
    fprintf(debug_file, "hash table size: %d\n", 1 << para->hashTableSizeBits);
    fprintf(debug_file, "block size: %d\n", packblocksize);
  }

  /* Allocate memory */
  if ((data = (byte *) malloc(hdr_depackedsize*2)) == NULL
        || (packed = (byte *) malloc(hdr_packedsize)) == NULL) {
    printf("ERR: not enough memory\n");
    res = 1;
    goto out;
  }

  fread(packed, 1, hdr_packedsize, packedfile);

  if (en_debug) {
    fprintf(debug_file, "--------------------\nblock number: %d\n", block_num);
    fprintf(debug_file, "packed size: %d\n", hdr_packedsize);
    fprintf(debug_file, "depacked size: %d\n", hdr_depackedsize);
    fprintf(debug_file, "byte per unit: %d\n", para->bytePerUnit);
    fprintf(debug_file, "coding mode: %d\n", para->codingMode);
  }

  /* While we are able to read a header from input file .. */
 // while (hdr_depackedsize != outsize) {

    unsigned long depackedsize;

    
    depackedsize = myLZ_depack(packed, data,hdr_depackedsize, packblocksize ,para);
 
    /* Check for decompression error */
    if (depackedsize != hdr_depackedsize) {
      printf("ERR: an error occured while decompressing\n");
      printf("depackedsize = %d ,hdr_depackedsize = %d \n ",depackedsize,hdr_depackedsize);
      res = 1;
      goto out;
    }
 
    /* Write decompressed data */
    fwrite(data, 1, hdr_depackedsize, newfile);
 
    /* sum input and output size */
    insize += (unsigned long) hdr_packedsize;
    outsize += (unsigned long) hdr_depackedsize;
    block_num++;
 // }

end:
  clocks = clock() - clocks;

  /* Show result */
  printf("decompressed %lu -> %lu bytes in %.2f seconds\n",
        insize, outsize,
        (double) clocks / (double) CLOCKS_PER_SEC);

out:
  /* Close files */
  if (packedfile != NULL) fclose(packedfile);
  if (newfile != NULL) fclose(newfile);

  /* Free memory */
  if (packed != NULL) free(packed);
  if (data != NULL) free(data);

  return res;
}

int parsePara(char *paraFile, myLZParaType *compPara) {
  int value;
  FILE *inf;
  char tmpStr[256], str[255];

  if ((inf = fopen(paraFile, "r")) == NULL) return -1;

  fgets(tmpStr, 255, inf);  sscanf(tmpStr, "%s", str);
  if (strncmp(str, "20160205", strlen("20160205"))) {
    fclose(inf);
    return -1;
  }
  fgets(tmpStr, 255, inf);  sscanf(tmpStr, "%d", &(compPara->bytePerUnit));
  fgets(tmpStr, 255, inf);  sscanf(tmpStr, "%d", &(compPara->hashTableSizeBits));
  fgets(tmpStr, 255, inf);  sscanf(tmpStr, "%d", &(compPara->blockSize));
  fgets(tmpStr, 255, inf);  sscanf(tmpStr, "%d", &(compPara->codingMode));

  fgets(tmpStr, 255, inf);  sscanf(tmpStr, "%s", str);
  if (strncmp(str, "[DEBUG]", strlen("[DEBUG]"))) {
    fclose(inf);
    return -1;
  }
  fgets(tmpStr, 255, inf);  sscanf(tmpStr, "%s", &(compPara->bytePerUnit));



  return 0;
}


int main(int argc, char *argv[]) {
  int action;
  myLZParaType compPara;

  if (argc > 1) {
    if (argv[1][0] == 'c') action = 1;
    else if (argv[1][0] == 'd') action = 2;
    else if (argv[1][0] == 'p') action = 3;
    else action = 0;
  } else action = 0;

  if (((action == 1 || action == 2) && argc < 4) || (action == 3 && argc < 3) || action == 0) {
    fprintf(stderr, "Argument error!\n");
    fprintf(stderr, "myLZ [[c | d] src_file dest_file | p para_file]\n");
    fprintf(stderr, "  c: compress with default setting\n");
    fprintf(stderr, "  d: decompress\n");
    fprintf(stderr, "  p: compress/decompress and debug with setting in para_file\n");
    exit(-1);
  }


  if (action == 1 || action == 2) {
    strcpy(compPara.src_filename, argv[2]);
    strcpy(compPara.dest_filename, argv[3]);
    strcpy(compPara.debug_filename, argv[3]);
    strcat(compPara.debug_filename, "_debug.txt");
    debug_file = fopen(compPara.debug_filename, "w");
    en_debug = 1;
    if (action == 1) {
      compPara.action = 1;
      compPara.bytePerUnit = 4;
      compPara.hashTableSizeBits = 11;
      compPara.blockSize = 4096;
      compPara.codingMode = 6;
    } else {
      compPara.action = 2;
      compPara.hashTableSizeBits = 12;
      compPara.blockSize = 4096;
    }
    compPara.debugLevel = 0;
  } else {
    if (parsePara(argv[2], &compPara) != 0) {
      fprintf(stderr, "Parameter file parsing error!\n");
      exit(-1);
    }
  }



  if (compPara.action == 1) compress_file(&compPara);
  else decompress_file(&compPara);

  if (debug_file != NULL) fclose(debug_file);

  return 0;
}
#endif // 0