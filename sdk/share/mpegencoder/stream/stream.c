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

#include <stdio.h>
#include <stdlib.h>

#include "../config.h"

//#if HAVE_WINSOCK2_H
//#include <winsock2.h>
//#endif

//#include "mp_msg.h"
//#include "help_mp.h"
//#include "osdep/shmem.h"
//#include "osdep/timer.h"
//#include "network.h"
#include "stream.h"
#include "../mencoder.h"
//#include "libmpdemux/demuxer.h"
//#include "libavutil/common.h"
//#include "libavutil/intreadwrite.h"

//#include "m_option.h"
//#include "m_struct.h"

//#include "cache2.h"

#ifdef DEBUG_WRITE_FAIL
static void
    _PrintFileName(
    MMP_CHAR* filename)
{

    printf("filename: %s\n", filename);
}
#else
#define _PrintFileName(f)
#endif

extern const stream_info_t stream_info_file;

static const stream_info_t* const auto_open_streams[] = {
    &stream_info_file,
    NULL
};

stream_buffer_ctrl_t*
new_stream_buffer_ctrl(
    void)
{
    stream_buffer_ctrl_t* sbc = calloc(1, sizeof(stream_buffer_ctrl_t));
    int i;
    if (!sbc)
    {
        trac("");
        return NULL;
    }

    sbc->b = sbc->stream_buffer + sbc->bwi;

    for (i = 0; i < STREAM_MAX_BUFFER_ENTRY_COUNT; ++i)
    {
        stream_buffer_t *b = &sbc->stream_buffer[i];
        b->pos = b->buf_pos = b->buf_len = 0;
        b->buffer = sbc->ocache_buffer + STREAM_MAX_BUFFER_ENTRY_SIZE * i;
    }
    return sbc;
}

void
free_stream_buffer_ctrl(
    stream_buffer_ctrl_t    *sbc)
{
    if (sbc)
        free(sbc);
}

static stream_t*
open_stream_plugin(
    const stream_info_t     *sinfo,
    const MMP_CHAR         *filename,
    int                     *ret,
    int                     sync)
{
    void* arg = NULL;
    stream_t* s;

    s = new_stream(0,-2);
    if (!s)
        return NULL;
    s->url = PalStrdup(filename);
    s->sync = sync;
    *ret = sinfo->open(s);
    if ((*ret) != STREAM_OK) {
        if (s)
        {
            if (s->url) free(s->url);
            //if (s->ocache_buffer) free(s->ocache_buffer);
            free(s);
        }
        return NULL;
    }
    if (s->type <= -2)
        mp_msg(MSGT_OPEN,MSGL_WARN, MSGTR_StreamNeedType);
    if (s->flags & MP_STREAM_SEEK && !s->seek)
        s->flags &= ~MP_STREAM_SEEK;
    if (s->seek && !(s->flags & MP_STREAM_SEEK))
        s->flags |= MP_STREAM_SEEK;

    return s;
}

stream_t* open_stream_full(const MMP_CHAR* filename, int sync) {
    int i,r;
    const stream_info_t* sinfo;
    stream_t* s;

    for (i = 0 ; auto_open_streams[i] ; i++) {
        sinfo = auto_open_streams[i];
        s = open_stream_plugin(sinfo, filename, &r, sync);
        if(s) return s;
    }

    mp_msg(MSGT_OPEN,MSGL_ERR, MSGTR_StreamCantHandleURL,filename);
    return NULL;
}

stream_t* open_output_stream(const MMP_CHAR* filename, int sync) {
    if (!filename) {
        mp_msg(MSGT_OPEN,MSGL_ERR,MSGTR_StreamNULLFilename);
        return NULL;
    }

    return open_stream_full(filename, sync);
}

//=================== STREAMER =========================
int stream_write_buffer(stream_t *s, unsigned char *buf, int len) {
    int rd;
    uint64_t write_size;
    if (!s->write_buffer)
        return -1;
    rd = s->write_buffer(s, buf, len);
    if (rd < 0)
        return -1;
    //s->pos += rd;
    //assert(rd == len && "stream_write_buffer(): unexpected short write");
    write_size =  (uint64_t)stream_tell(s);
    if (write_size > s->write_size)
    	s->write_size = write_size;
    return rd;
}

int stream_flush_buffer(stream_t *s) {
    offset_t newpos;
    int64_t len = s->sbc->b->buf_len;
    if (len <= 0)
        return 0;

    newpos = s->sbc->b->pos + len;
    s->sbc->bwi++;
    if (s->sbc->bwi >= STREAM_MAX_BUFFER_ENTRY_COUNT)
        s->sbc->bwi -= STREAM_MAX_BUFFER_ENTRY_COUNT;
#ifdef ASYNC_FILE_HANDLE
    while (s->sbc->bwi == s->sbc->bfi)
    {
#ifdef DEBUG_WRITE_FAIL
        printf("i");
#endif
        PalSleep(1);
        // time out mechanism
    }
#endif
    s->flush_buffer(s);
    s->sbc->b            = s->sbc->stream_buffer + s->sbc->bwi;
    s->sbc->b->pos       = newpos;
    s->sbc->b->buf_pos   = s->sbc->b->buf_len = 0;

    return len;
}

int stream_seek_long(stream_t *s,offset_t pos){
    offset_t newpos=0;

    stream_flush_buffer(s);

    if (!s->seek || !s->seek(s,pos))
        return 0;
    return 1;
}

void stream_reset(stream_t *s){
    if (s->eof){
        s->sbc->b->pos       = 0;
        s->sbc->b->buf_pos   = s->sbc->b->buf_len = 0;
        s->eof=0;
    }
}

int stream_control(stream_t *s, int cmd, void *arg){
    return STREAM_UNSUPPORTED;
}

stream_t* new_stream(int fd,int type){
    stream_t *s;
    stream_buffer_ctrl_t* sbc = mencoder_GetStreamBufferCtrl();

    if (!sbc) return NULL;
    s = calloc(1, sizeof(stream_t));
    if (!s) return NULL;

    s->sbc      = sbc;

    s->fd   = fd;
    s->type = type;
    s->priv = NULL;
    s->url  = NULL;

    stream_reset(s);
    return s;
}

static void
_file_callback(
    PAL_FILE* file,
    MMP_ULONG result,
    void*     arg)
{
    if (arg)
        *((int*)arg) = 1;
}

void free_stream(stream_t *s, int sync) {
    //  printf("\n*** free_stream() called ***\n");
#ifdef CONFIG_STREAM_CACHE
    cache_uninit(s);
#endif
    stream_flush_buffer(s);
    if (s->close) s->close(s);

    if (s->fd > 0) {
#ifdef ASYNC_FILE_HANDLE
        int done = 0;
        if (sync)
        {   
            PalSleep(200);
            
            PalFileClose((PAL_FILE*)s->fd, _file_callback, &done);
            while (!done)
            {
#ifdef DEBUG_WRITE_FAIL
                trac("");
#endif
                PalSleep(1);
            }
        }
        else
            PalFileClose((PAL_FILE*)s->fd, _file_callback, MMP_NULL);
#else
        PalFileClose((PAL_FILE*)s->fd, MMP_NULL, MMP_NULL);
#endif
        s->fd = MMP_NULL;

        if (0 == s->write_size)
        {   // delete empty file
#ifdef DEBUG_WRITE_FAIL
            printf("Delete file (file size == 0):\n");
            _PrintFileName(s->url);
#endif

#ifdef ASYNC_FILE_HANDLE
            if (sync)
            {
                done = 0;
                PalFileDelete(s->url, _file_callback, &done);
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
            else
                PalFileDelete(s->url, _file_callback, MMP_NULL);
#else
            PalFileDelete(s->url, MMP_NULL, MMP_NULL);
#endif
        }
    }
    // Disabled atm, i don't like that. s->priv can be anything after all
    // streams should destroy their priv on close
    if(s->url) free(s->url);
    free(s);
}

