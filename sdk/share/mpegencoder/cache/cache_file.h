#ifndef CACHE_FILE_H
#define CACHE_FILE_H

#include "../config.h"
#include "../libavformat/avio.h"

#define CACHE_MAX_BUFFER_ENTRY_SIZE    (64 * 1024)
#define CACHE_MAX_BUFFER_ENTRY_COUNT   (2)
#define CACHE_MAX_BUFFER_SIZE          (CACHE_MAX_BUFFER_ENTRY_SIZE * CACHE_MAX_BUFFER_ENTRY_COUNT)

typedef void
(*AVIO_WRITE)(
    AVIOContext *pb,
    const unsigned char *buf,
    int size);

typedef struct cache_buffer {
    offset_t pos;

    unsigned int buf_pos;   // iclai: indicate the next byte of data in the
                            // internal [buffer] is read by the demuxer. The
                            // position indicated by this variable is relative
                            // to the start of the internal [buffer].
    unsigned int buf_len;   // iclai: how many bytes of data is already filled in the internal [buffer]
    unsigned char *buffer;
} cache_buffer_t;

typedef struct cache_ctrl {
    unsigned char       ocache_buffer[CACHE_MAX_BUFFER_SIZE];

    cache_buffer_t      cache_buffer[CACHE_MAX_BUFFER_ENTRY_COUNT];
    cache_buffer_t*     b;  // pointer to active stream buffer
    unsigned int        bwi;// active buffer write index
    unsigned int        bfi;// active buffer flush index
} cache_buffer_ctrl_t;

typedef struct cache {
    // Write
    //int (*write_buffer)(struct stream *s, char* buffer, int len);
    // Flush
    //int (*flush_buffer)(struct stream *s);
    // Seek
    //int (*seek)(struct stream *s,offset_t pos);
    // Close
    //void (*close)(struct stream *s);

    int fd;                 // file descriptor, see man open(2)
    MMP_CHAR* url;         // strdup() of filename/url
                            // iclai: currently operationg filename or url.

    cache_buffer_ctrl_t     cbc;
    //uint64_t                write_size;
    int                     sync;
} cache_t;

cache_t* new_cache(MMP_CHAR* url, int sync);
void free_cache(cache_t *c);
void cache_write(cache_t *c, unsigned char *buffer, int len);
int cache_flush(cache_t *c, int sync);
void cache_dump_to_avio_write(
    cache_t *c,
    AVIO_WRITE avio_write,
    AVIOContext *pb);
#endif