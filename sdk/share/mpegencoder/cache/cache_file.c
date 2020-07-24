#include "cache_file.h"
#include "../libavformat/avio.h"

#if 1
static void
_PrintFileName(
    MMP_CHAR* filename)
{
    printf("filename: %s\n", filename);
}
#else
#define _PrintFileName(f)
#endif

static void
_file_open_callback(
    PAL_FILE*   file,
    MMP_ULONG   result,
    void*       arg)
{
    *((int*)arg) = (int)file;
}

cache_t* new_cache(MMP_CHAR* url, int sync)
{
    int i;
    cache_t* c = (cache_t*)calloc(1, sizeof(cache_t));
    if (!c)
        goto error;

    c->url = PalStrdup(url);
    if (!c->url)
        goto error;

    _PrintFileName(c->url);
    if (sync)
    {
        c->fd = (int)PalFileOpen(c->url, PAL_FILE_WBP, MMP_NULL, MMP_NULL);
#ifdef DEBUG_WRITE_FAIL
        printf("fd(%X)\n", c->fd);
#endif
        if (!c->fd)
            goto error;
    }
    else
        PalFileOpen(c->url, PAL_FILE_WBP, _file_open_callback, &c->fd);

    for (i = 0; i < CACHE_MAX_BUFFER_ENTRY_COUNT; ++i)
    {
        c->cbc.cache_buffer[i].buffer =  &c->cbc.ocache_buffer[i * CACHE_MAX_BUFFER_ENTRY_SIZE];
    }
    c->cbc.b = c->cbc.cache_buffer;

    return c;

error:
    if (c)
    {
        if (c->url) free(c->url);
        free(c);
    }
    return MMP_NULL;
}

static void
_file_callback(
    PAL_FILE*   file,
    MMP_ULONG   result,
    void*       arg)
{
    *((int*)arg) = 1;
}

void free_cache(cache_t *c)
{
    if (c)
    {
        if (c->fd)
        {
            int done = 0;

            PalFileClose((PAL_FILE*)c->fd, _file_callback, &done);
            while (!done)
            {
#ifdef DEBUG_WRITE_FAIL
                trac("");
#endif
                PalSleep(1);
            }
#ifdef DEBUG_WRITE_FAIL
            trac("");
#endif

            done = 0;
            _PrintFileName(c->url);
            PalFileDelete(c->url, _file_callback, &done);
            while (!done)
            {
#ifdef DEBUG_WRITE_FAIL
                trac("");
#endif
                PalSleep(1);
            }
#ifdef DEBUG_WRITE_FAIL
            trac("");
#endif
        }

        if (c->url) free(c->url);
        free(c);
    }
}

void cache_write(cache_t *c, unsigned char *buffer, int len)
{
    do
    {
        int r;
        int wr;

        if ((!c) || (!buffer) || (len <= 0))
            break;
        wr = len;
        while (wr > 0) {
            r = CACHE_MAX_BUFFER_ENTRY_SIZE - c->cbc.b->buf_pos;
            if (r > wr)
                r = wr;
            memcpy(c->cbc.b->buffer + c->cbc.b->buf_pos, buffer, r);
            //printf("w(%d)\n", len);
            //if (r <= 0)
            //    return -1;
            wr -= r;
            buffer += r;
            c->cbc.b->buf_pos += r;
            if (c->cbc.b->buf_len < c->cbc.b->buf_pos)
                c->cbc.b->buf_len = c->cbc.b->buf_pos;

            if (CACHE_MAX_BUFFER_ENTRY_SIZE == c->cbc.b->buf_len)
            {
                cache_flush(c, MMP_FALSE);
            }
        }
    } while (0);
}

static void
_file_write_callback(
    PAL_FILE*   file,
    MMP_ULONG   result,
    void*       arg)
{
    cache_buffer_ctrl_t* cbc = (cache_buffer_ctrl_t*)arg;
    if (cbc)
    {
        cbc->bfi++;
        if (cbc->bfi >= CACHE_MAX_BUFFER_ENTRY_COUNT)
            cbc->bfi -= CACHE_MAX_BUFFER_ENTRY_COUNT;
    }
}

int cache_flush(cache_t *c, int sync)
{
    int len = 0;
    do
    {
        int w = 0;
        offset_t newpos;
        if (!c)
            break;

        while (!c->fd)
        {
            PalSleep(1);
        }

        len = c->cbc.b->buf_pos;
        if (len <= 0)
            break;

        newpos = c->cbc.b->pos + len;
        c->cbc.bwi++;
        if (c->cbc.bwi >= CACHE_MAX_BUFFER_ENTRY_COUNT)
            c->cbc.bwi -= CACHE_MAX_BUFFER_ENTRY_COUNT;
#ifdef DEBUG_WRITE_FAIL
        printf("cache_flush. bwi(%d), bfi(%d)\n", c->cbc.bwi, c->cbc.bfi);
#endif
//#ifdef ASYNC_FILE_HANDLE
        while (c->cbc.bwi == c->cbc.bfi)
        {
#ifdef DEBUG_WRITE_FAIL
            printf("x");
#endif
            PalSleep(1);
            // time out mechanism
        }
//#endif

//#ifdef ASYNC_FILE_HANDLE
        //printf("w+\n");
        w = PalFileWrite(c->cbc.b->buffer, 1, c->cbc.b->buf_len, (PAL_FILE*)c->fd, _file_write_callback, &c->cbc);
        if (sync)
            while (c->cbc.bwi != c->cbc.bfi)
            {
#ifdef DEBUG_WRITE_FAIL
                printf("o");
#endif
                PalSleep(1);
            }
        //printf("w(%d)-\n", c->b->buf_len);
        //c->cbc.bfi++;
        //if (c->cbc.bfi >= CACHE_MAX_BUFFER_ENTRY_COUNT)
        //    c->cbc.bfi -= CACHE_MAX_BUFFER_ENTRY_COUNT;
//#else
//        _seek((PAL_FILE*)c->fd, c->cbc.b->pos, PAL_SEEK_SET, MMP_NULL);
//        //printf("w+\n");
//        w = PalFileWrite(c->cbc.b->buffer, 1, c->cbc.b->buf_len, (PAL_FILE*)c->fd, MMP_NULL);
//#endif
        c->cbc.b            = c->cbc.cache_buffer + c->cbc.bwi;
        c->cbc.b->pos       = newpos;
        c->cbc.b->buf_pos   = c->cbc.b->buf_len = 0;
    } while(0);

    return len;
}

static inline int cache_write_size(cache_t *c)
{
    //return s->pos+s->buf_pos-s->buf_len;
    return (int)(c->cbc.b->pos + c->cbc.b->buf_pos);
}

//#define _LOG_
void cache_dump_to_avio_write(
    cache_t *c,
    AVIO_WRITE avio_write,
    AVIOContext *pb)
{
    do
    {
#ifdef _LOG_
        PAL_FILE* f;
        MMP_CHAR filename[255];
        int filename_len;
#endif
        int done = 0;
        int remain_size;
        const unsigned char *buf;
        if ((!c) || (!avio_write) || (!pb))
            break;

        remain_size = cache_write_size(c);
        if (remain_size == 0)
            break;
        buf = c->cbc.ocache_buffer;
        printf("seek cache\n");
        PalFileSeek((PAL_FILE*)c->fd, 0, PAL_SEEK_SET, _file_callback, &done);
        while (!done)
        {
#ifdef DEBUG_WRITE_FAIL
            trac("");
#endif
            PalSleep(1);
        }

#ifdef _LOG_
        PalStrcpy(filename, c->url);
        filename_len = PalStrlen(filename);
        filename[filename_len] = '_';
        filename[filename_len+1] = 0;
        f = PalFileOpen(filename, PAL_FILE_WB, MMP_NULL, MMP_NULL);
#endif
        for (;remain_size > 0; remain_size -= CACHE_MAX_BUFFER_SIZE)
        {
            int s = (remain_size > CACHE_MAX_BUFFER_SIZE) ? CACHE_MAX_BUFFER_SIZE : remain_size;
            done = 0;
            PalFileRead((void*)buf, s, 1, (PAL_FILE*)c->fd, _file_callback, &done);
            while (!done)
            {
#ifdef DEBUG_WRITE_FAIL
                trac("");
#endif
                PalSleep(1);
            }
#ifdef _LOG_
            PalFileWrite((void*)buf, s, 1, f, MMP_NULL, MMP_NULL);
#endif
            avio_write(pb, buf, s);
        }
#ifdef _LOG_
        PalFileClose(f, MMP_NULL, MMP_NULL);
#endif
    } while(0);
}