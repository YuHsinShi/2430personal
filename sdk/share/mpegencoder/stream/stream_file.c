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

#include "../config.h"
#include "../mencoder.h"
#include "stream.h"

//[TODO]@@
#if 0
static void
dmacpy(
    void*   pdst,
    void*   psrc,
    int     size)
{

    MMP_UINT32 dmaAttribList[] =
    {
        MMP_DMA_ATTRIB_DMA_TYPE,            (MMP_UINT32)MMP_DMA_TYPE_MEM_TO_MEM,
        MMP_DMA_ATTRIB_SRC_ADDR,            (MMP_UINT32)psrc,
        MMP_DMA_ATTRIB_DST_ADDR,            (MMP_UINT32)pdst,
        MMP_DMA_ATTRIB_TOTAL_SIZE_IN_BYTE,  (MMP_UINT32)size,
        MMP_DMA_ATTRIB_NONE
    };

    MMP_DMA_CONTEXT tDmaContext = { 0 };
    mmpDmaCreateContext(&tDmaContext);

    mmpDmaWaitIdle(tDmaContext);
    mmpDmaSetAttrib(tDmaContext, dmaAttribList);
    mmpDmaFire(tDmaContext);

    mmpDmaWaitIdle(tDmaContext);
    mmpDmaDestroyContext(tDmaContext);

}
#endif

static int write_buffer(stream_t *s, char* buffer, int len) {
    int r;
    int wr = len;
    while (wr > 0) {
        //r = fwrite(buffer,1,len,(FILE*)s->fd);
        r = STREAM_MAX_BUFFER_ENTRY_SIZE - s->sbc->b->buf_pos;
        if (r > wr)
            r = wr;
		//[TODO]@@
        //if (r > 1024)
            //dmacpy(s->sbc->b->buffer + s->sbc->b->buf_pos, buffer, r);
        //else
            memcpy(s->sbc->b->buffer + s->sbc->b->buf_pos, buffer, r);
        //printf("w(%d)\n", len);
        //if (r <= 0)
        //    return -1;
        wr -= r;
        buffer += r;
        s->sbc->b->buf_pos += r;
        if (s->sbc->b->buf_len < s->sbc->b->buf_pos)
            s->sbc->b->buf_len = s->sbc->b->buf_pos;

        if (STREAM_MAX_BUFFER_ENTRY_SIZE == s->sbc->b->buf_len)
        {
            stream_flush_buffer(s);
        }
    }
    return len;
}

static void
_file_callback(
    PAL_FILE* file,
    MMP_ULONG   result,
    void*       arg)
{
    if(arg)
        *((int*)arg) = 1;
}

static void
_file_write_callback(
    PAL_FILE*   file,
    MMP_ULONG   result,
    void*       arg)
{
    stream_buffer_ctrl_t* sbc = mencoder_GetStreamBufferCtrl();
    if (sbc)
    {
        sbc->bfi++;
        if (sbc->bfi >= STREAM_MAX_BUFFER_ENTRY_COUNT)
            sbc->bfi -= STREAM_MAX_BUFFER_ENTRY_COUNT;
    }
}

static int
_seek(
    PAL_FILE*           stream,
    offset_t            offset,
    MMP_INT             origin,
    PAL_FILE_CALLBACK   callback)
{
    #define _2G  (0x7FFFFFFFL)

    if ((origin == PAL_SEEK_SET) && (offset >= _2G))
    {
        PalFileSeek(stream, 0, PAL_SEEK_SET, callback, MMP_NULL);//seek ¨ì begin
        
        while (offset > _2G){
            PalFileSeek(stream, _2G, PAL_SEEK_CUR, callback, MMP_NULL);
            offset -= _2G;
        }
        PalFileSeek(stream, (long)offset, PAL_SEEK_CUR, callback, MMP_NULL);
    }
    else
        PalFileSeek(stream, offset, origin, callback, MMP_NULL);
}

static int flush_buffer(stream_t *s) {
    int w = 0;

#ifdef ASYNC_FILE_HANDLE
    if (s->flush_pos != s->sbc->b->pos)
    {
        _seek((PAL_FILE*)s->fd, s->sbc->b->pos, PAL_SEEK_SET, _file_callback);
        s->flush_pos = s->sbc->b->pos;
    }
    //printf("w+\n");
    w = PalFileWrite(s->sbc->b->buffer, 1, s->sbc->b->buf_len, (PAL_FILE*)s->fd, _file_write_callback, MMP_NULL);
    s->flush_pos += s->sbc->b->buf_len;
    //w = PalFileWrite(s->b->buffer, 1, s->b->buf_len, (PAL_FILE*)s->fd, MMP_NULL);
    //printf("w(%d)-\n", s->b->buf_len);
    //s->sbc->bfi++;
    //if (s->sbc->bfi >= STREAM_MAX_BUFFER_ENTRY_COUNT)
    //    s->sbc->bfi -= STREAM_MAX_BUFFER_ENTRY_COUNT;
#else
    _seek((PAL_FILE*)s->fd, s->sbc->b->pos, PAL_SEEK_SET, MMP_NULL);
    //printf("w+\n");
    w = PalFileWrite(s->sbc->b->buffer, 1, s->sbc->b->buf_len, (PAL_FILE*)s->fd, MMP_NULL, MMP_NULL);
#endif
    return w;
}

static int seek(stream_t *s, offset_t newpos) {
    s->sbc->b->pos = newpos;
    //printf("%s(%d)\n", __FUNCTION__, __LINE__);

#ifdef ASYNC_FILE_HANDLE
    if (_seek((PAL_FILE*)s->fd, s->sbc->b->pos, PAL_SEEK_SET, _file_callback) != 0) {
#else
    if (_seek((PAL_FILE*)s->fd, s->sbc->b->pos, PAL_SEEK_SET, MMP_NULL) != 0) {
#endif
        s->eof=1;
        return 0;
    }
    return 1;
}

static int* pfd;

static void
_file_open_callback(
    PAL_FILE* file,
    MMP_ULONG result,
    void*     arg)
{
    *((int*)arg) = (int)file;
}

static int open_f(stream_t *stream) {
    PAL_FILE* f;
    MMP_CHAR *filename;

    filename = stream->url;
    if (!filename) {
        mp_msg(MSGT_OPEN,MSGL_ERR, "[file] No filename\n");
        return STREAM_ERROR;
    }

#ifdef ASYNC_FILE_HANDLE
    if (stream->sync)
    {
        f = PalFileOpen(filename, PAL_FILE_WBP, MMP_NULL, MMP_NULL);
        if (!f) {
            mp_msg(MSGT_OPEN,MSGL_ERR,MSGTR_FileNotFound,filename);
            return STREAM_ERROR;
        }
        stream->fd = (int)f;
    }
    else
    {
        //pfd = &stream->fd;
        PalFileOpen(filename, PAL_FILE_WBP, _file_open_callback, &stream->fd);
    }
#else
    f = PalFileOpen(filename, PAL_FILE_WBP, MMP_NULL, MMP_NULL);
    if (!f) {
        mp_msg(MSGT_OPEN,MSGL_ERR,MSGTR_FileNotFound,filename);
        return STREAM_ERROR;
    }
    stream->fd = (int)f;
#endif

    stream->seek = seek;
    stream->type = STREAMTYPE_FILE;

    mp_msg(MSGT_OPEN,MSGL_V,"[file] File size is %"PRId64" bytes\n", (int64_t)len);

    stream->write_buffer = write_buffer;
    stream->flush_buffer = flush_buffer;

    return STREAM_OK;
}

const stream_info_t stream_info_file = {
    open_f,
    //{ "file", "", NULL }
};
