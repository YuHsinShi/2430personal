/*
 * This file is part of MPlayer.
 *
 * MPlayer is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * MPlayer is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with MPlayer; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef MPLAYER_STREAM_H
#define MPLAYER_STREAM_H

#include "../config.h"
//#include "m_option.h"
//#include "mp_msg.h"
//#include "url.h"
//#include <stdio.h>
//#include <string.h>
//#include <inttypes.h>
//#include <sys/types.h>
//#include <fcntl.h>

#define STREAMTYPE_DUMMY -1    // for placeholders, when the actual reading is handled in the demuxer
#define STREAMTYPE_FILE 0      // read from seekable file
//#define STREAMTYPE_VCD  1      // raw mode-2 CDROM reading, 2324 bytes/sector
//#define STREAMTYPE_STREAM 2    // same as FILE but no seeking (for net/stdin)
//#define STREAMTYPE_DVD  3      // libdvdread
//#define STREAMTYPE_MEMORY  4   // read data from memory area
//#define STREAMTYPE_PLAYLIST 6  // FIXME!!! same as STREAMTYPE_FILE now
//#define STREAMTYPE_DS   8      // read from a demuxer stream
//#define STREAMTYPE_DVDNAV 9    // we cannot safely "seek" in this...
//#define STREAMTYPE_CDDA 10     // raw audio CD reader
//#define STREAMTYPE_SMB 11      // smb:// url, using libsmbclient (samba)
//#define STREAMTYPE_VCDBINCUE 12      // vcd directly from bin/cue files
//#define STREAMTYPE_DVB 13
//#define STREAMTYPE_VSTREAM 14
//#define STREAMTYPE_SDP 15
//#define STREAMTYPE_PVR 16
//#define STREAMTYPE_TV 17
//#define STREAMTYPE_MF 18
//#define STREAMTYPE_RADIO 19
//#define STREAMTYPE_BLURAY 20
//#define STREAMTYPE_BD 21

//#define STREAM_BUFFER_SIZE     2048
//#define STREAM_MAX_SECTOR_SIZE (8*1024)

#define STREAM_MAX_BUFFER_ENTRY_SIZE    (128 * 1024)
#define STREAM_MAX_BUFFER_ENTRY_COUNT   (80)
#define STREAM_MAX_BUFFER_SIZE          (STREAM_MAX_BUFFER_ENTRY_SIZE * STREAM_MAX_BUFFER_ENTRY_COUNT)

//#define VCD_SECTOR_SIZE 2352
//#define VCD_SECTOR_OFFS 24
//#define VCD_SECTOR_DATA 2324

/// atm it will always use mode == STREAM_READ
/// streams that use the new api should check the mode at open
//#define STREAM_READ  0
//#define STREAM_WRITE 1
/// Seek flags, if not mannualy set and s->seek isn't NULL
/// MP_STREAM_SEEK is automaticly set
#define MP_STREAM_SEEK_BW  2   // seek backward
#define MP_STREAM_SEEK_FW  4   // seek forward
#define MP_STREAM_SEEK  (MP_STREAM_SEEK_BW|MP_STREAM_SEEK_FW)
/** This is a HACK for live555 that does not respect the
    separation between stream an demuxer and thus is not
    actually a stream cache can not be used */
//#define STREAM_NON_CACHEABLE 8

//////////// Open return code
#define STREAM_REDIRECTED -2
/// This can't open the requested protocol (used by stream wich have a
/// * protocol when they don't know the requested protocol)
#define STREAM_UNSUPPORTED -1
#define STREAM_ERROR 0
#define STREAM_OK    1

#define MAX_STREAM_PROTOCOLS 10

// stream control command. It will be sent to the stream_control() function.
#define STREAM_CTRL_RESET 0
#define STREAM_CTRL_GET_TIME_LENGTH 1
#define STREAM_CTRL_SEEK_TO_CHAPTER 2
#define STREAM_CTRL_GET_CURRENT_CHAPTER 3
#define STREAM_CTRL_GET_NUM_CHAPTERS 4
#define STREAM_CTRL_GET_CURRENT_TIME 5
#define STREAM_CTRL_SEEK_TO_TIME 6
//#define STREAM_CTRL_GET_SIZE 7
//#define STREAM_CTRL_GET_ASPECT_RATIO 8
//#define STREAM_CTRL_GET_NUM_ANGLES 9
//#define STREAM_CTRL_GET_ANGLE 10
//#define STREAM_CTRL_SET_ANGLE 11
//#define STREAM_CTRL_GET_NUM_TITLES 12
//#define STREAM_CTRL_GET_LANG 13
//#define STREAM_CTRL_GET_CURRENT_TITLE 14

struct stream;
typedef struct stream_info_st {
    /// mode isn't used atm (ie always READ) but it shouldn't be ignored
    /// opts is at least in it's defaults settings and may have been
    /// altered by url parsing if enabled and the options string parsing.
    int (*open)(
        struct stream   *st);
    //const char* protocols[MAX_STREAM_PROTOCOLS];
} stream_info_t;

typedef struct stream_buffer {
    offset_t pos;

    offset_t buf_pos;   // iclai: indicate the next byte of data in the
                        // internal [buffer] is read by the demuxer. The
                        // position indicated by this variable is relative
                        // to the start of the internal [buffer].
    offset_t buf_len;   // iclai: how many bytes of data is already filled in the internal [buffer]

    //unsigned char buffer[STREAM_BUFFER_SIZE>STREAM_MAX_SECTOR_SIZE?STREAM_BUFFER_SIZE:STREAM_MAX_SECTOR_SIZE];
    unsigned char *buffer;
} stream_buffer_t;

typedef struct stream_buffer_ctrl {
    unsigned char       ocache_buffer[STREAM_MAX_BUFFER_SIZE];

    stream_buffer_t     stream_buffer[STREAM_MAX_BUFFER_ENTRY_COUNT];
    stream_buffer_t*    b;  // pointer to active stream buffer
    unsigned int        bwi;// active buffer write index
    unsigned int        bfi;// active buffer flush index
} stream_buffer_ctrl_t;

typedef struct stream {
    // Write
    int (*write_buffer)(struct stream *s, char* buffer, int len);
    // Flush
    int (*flush_buffer)(struct stream *s);
    // Seek
    int (*seek)(struct stream *s,offset_t pos);
    // Close
    void (*close)(struct stream *s);

    int fd;                 // file descriptor, see man open(2)
    int type;               // see STREAMTYPE_*
    int flags;
    int sector_size;        // sector size (seek will be aligned on this size if non 0)

    int eof;
    void* priv;             // used for DVD, TV, RTSP etc
    MMP_CHAR* url;         // strdup() of filename/url
                            // iclai: currently operationg filename or url.

    stream_buffer_ctrl_t*   sbc;
    offset_t                flush_pos;
    uint64_t                write_size; // in bytes
    int                     sync;
} stream_t;

stream_buffer_ctrl_t*
new_stream_buffer_ctrl(
    void);

void
free_stream_buffer_ctrl(
    stream_buffer_ctrl_t    *sbc);

int stream_seek_long(stream_t *s, offset_t pos);

// no cache, define wrappers:
#define cache_stream_seek_long(x,y) stream_seek_long(x,y)
#define stream_enable_cache(x,y,z,w) 1
int stream_write_buffer(stream_t *s, unsigned char *buf, int len);
int stream_flush_buffer(stream_t *s);

static inline int stream_eof(stream_t *s)
{
    return s->eof;
}

static inline offset_t stream_tell(stream_t *s)
{
    //return s->pos+s->buf_pos-s->buf_len;
    return s->sbc->b->pos + s->sbc->b->buf_pos;
}

static inline int stream_seek(stream_t *s, offset_t pos)
{
    mp_dbg(MSGT_DEMUX, MSGL_DBG3, "seek to 0x%"PRIX64"\n", pos);

    if (pos < 0) {
        mp_msg(MSGT_DEMUX, MSGL_ERR,
               "Invalid seek to negative position %"PRIx64"!\n", pos);
        pos = 0;
    }
    if (pos >= s->sbc->b->pos) {
        offset_t x = (s->sbc->b->pos + s->sbc->b->buf_len) - pos;
        if(x>=0){
            s->sbc->b->buf_pos = s->sbc->b->buf_len - x;
            // putchar('*');fflush(stdout);
            return 1;
        }
    }

    return cache_stream_seek_long(s,pos);
}

static inline uint64_t stream_write_size(stream_t *s)
{
    return s->write_size;
}

void stream_reset(stream_t *s);
int stream_control(stream_t *s, int cmd, void *arg);
stream_t* new_stream(int fd,int type);
void free_stream(stream_t *s, int sync);
stream_t* open_stream(const MMP_CHAR* filename,char** options,int* file_format);
stream_t* open_stream_full(const MMP_CHAR* filename, int sync);
stream_t* open_output_stream(const MMP_CHAR* filename, int sync);
/// Internal seek function bypassing the stream buffer
//int stream_seek_internal(stream_t *s, offset_t newpos);

extern char * audio_stream;

#endif /* MPLAYER_STREAM_H */
