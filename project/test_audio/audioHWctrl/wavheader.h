#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef struct _riff_t {
	char riff[4] ;	/* "RIFF" (ASCII characters) */
    unsigned long  len ;	/* Length of package (binary, little endian) */
	char wave[4] ;	/* "WAVE" (ASCII characters) */
} riff_t;

typedef struct _format_t {
	char  fmt[4];		/* "fmt_" (ASCII characters) */
	unsigned long   len ;	/* length of FORMAT chunk (always 0x10) */
	unsigned short  type;		/* codec type*/
	unsigned short  channel ;	/* Channel numbers (0x01 = mono, 0x02 = stereo) */
	unsigned long   rate ;	/* Sample rate (binary, in Hz) */
	unsigned long   bps ;	/* Average Bytes Per Second */
 	unsigned short  blockalign ;	/*number of bytes per sample */
	unsigned short  bitpspl ;	/* bits per sample */     
} format_t;

/* The DATA chunk */

typedef struct _data_t {
	char data[4] ;	/* "data" (ASCII characters) */
	int  len ;	/* length of data */
} data_t;

typedef struct _wave_header_t
{
	riff_t riff_chunk;
	format_t format_chunk;
	data_t data_chunk;
} wave_header_t;

typedef struct _Playdata {
    FILE* fd;
    int hsize;
    unsigned int playlenth;
}playdata;