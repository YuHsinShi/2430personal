/*
 * buffered I/O
 * Copyright (c) 2000,2001 Fabrice Bellard
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

//#include "libavutil/crc.h"
#include "../libavutil/dict.h"
#include "../libavutil/intreadwrite.h"
//#include "libavutil/opt.h"
//#include "libavutil/avassert.h"
#include "avformat.h"
#include "avio.h"
#include "avio_internal.h"
#include "internal.h"
//#include "url.h"
//#include <stdarg.h>

/**
 * Do seeks within this distance ahead of the current buffer by skipping
 * data instead of calling the protocol seek function, for seekable
 * protocols.
 */
#define SHORT_SEEK_THRESHOLD 4096

static int url_resetbuf(AVIOContext *s);

int ffio_init_context(
    AVIOContext *s,
    unsigned char *buffer,
    int buffer_size,
    void *opaque,
    int (*write_packet)(void *opaque, uint8_t *buf, int buf_size),
    int64_t (*seek)(void *opaque, int64_t offset, int whence),
    uint64_t (*write_size)(void *opaque))
{
    s->buffer      = buffer;
    s->buffer_size = buffer_size;
    s->buf_ptr     = buffer;
    s->opaque      = opaque;

    url_resetbuf(s);

    s->write_packet     = write_packet;
    s->seek             = seek;
    s->write_size       = write_size;
    s->pos              = 0;
    s->must_flush       = 0;
    s->error            = 0;

    return 0;
}

AVIOContext *avio_alloc_context(
    unsigned char *buffer,
    int buffer_size,
    void *opaque,
    int (*write_packet)(void *opaque, uint8_t *buf, int buf_size),
    int64_t (*seek)(void *opaque, int64_t offset, int whence),
    uint64_t (*write_size)(void *opaque))
{
    AVIOContext *s = av_mallocz(sizeof(AVIOContext));
    if (!s)
    {
        trac("");
        return NULL;
    }
    ffio_init_context(s, buffer, buffer_size, opaque,
                  write_packet, seek, write_size);
    return s;
}

static void writeout(AVIOContext *s, const uint8_t *data, int len)
{
    if (s->write_packet && !s->error) {
        int ret = s->write_packet(s->opaque, (uint8_t *)data, len);
        if (ret < 0) {
            s->error = ret;
        }
    }
    s->pos += len;
}

static void flush_buffer(AVIOContext *s)
{
    if (s->buf_ptr > s->buffer) {
        writeout(s, s->buffer, s->buf_ptr - s->buffer);
    }
    s->buf_ptr = s->buffer;
}

void avio_w8(AVIOContext *s, int b)
{
    av_assert2(b>=-128 && b<=255);
    *s->buf_ptr++ = b;
    if (s->buf_ptr >= s->buf_end)
        flush_buffer(s);
}

void avio_write(AVIOContext *s, const unsigned char *buf, int size)
{
    avio_flush(s);
    writeout(s, buf, size);
    return;
}

void avio_flush(AVIOContext *s)
{
    flush_buffer(s);
    s->must_flush = 0;
}

int64_t avio_seek(AVIOContext *s, int64_t offset, int whence)
{
    int64_t offset1;
    int64_t pos;

    if(!s)
        return AVERROR(EINVAL);

    pos = s->pos;

    if (whence != SEEK_CUR && whence != SEEK_SET)
        return AVERROR(EINVAL);

    if (whence == SEEK_CUR) {
        offset1 = pos + (s->buf_ptr - s->buffer);
        if (offset == 0)
            return offset1;
        offset += offset1;
    }
    offset1 = offset - pos;
    if (!s->must_flush && (!s->seek) &&
        offset1 >= 0 && offset1 <= (s->buf_end - s->buffer)) {
        /* can do the seek inside the buffer */
        s->buf_ptr = s->buffer + offset1;
    } else {
        int64_t res;

        flush_buffer(s);
        s->must_flush = 1;
        if (!s->seek)
            return AVERROR(EPIPE);
        if ((res = s->seek(s->opaque, offset, SEEK_SET)) < 0)
            return res;
        s->buf_ptr = s->buffer;
        s->pos = offset;
    }
    return offset;
}

uint64_t avio_write_size(AVIOContext *s)
{
    if (s->write_size)
        return s->write_size(s->opaque);
    return 0;
}

int64_t avio_skip(AVIOContext *s, int64_t offset)
{
    return avio_seek(s, offset, SEEK_CUR);
}

void avio_wl32(AVIOContext *s, unsigned int val)
{
#ifdef TEST
    avio_w8(s, (uint8_t) val       );
    avio_w8(s, (uint8_t)(val >> 8 ));
    avio_w8(s, (uint8_t)(val >> 16));
    avio_w8(s,           val >> 24 );
#else
    uint8_t data[] = {   
        (uint8_t) val,    
        (uint8_t)(val >> 8 ),    
        (uint8_t)(val >> 16),    
                  val >> 24};
    avio_write(s, data, sizeof(data));
#endif
}

void avio_wb32(AVIOContext *s, unsigned int val)
{
#ifdef TEST
    avio_w8(s,           val >> 24 );
    avio_w8(s, (uint8_t)(val >> 16));
    avio_w8(s, (uint8_t)(val >> 8 ));
    avio_w8(s, (uint8_t) val       );
#else
    uint8_t data[] = {   
                  val >> 24,  
        (uint8_t)(val >> 16),  
        (uint8_t)(val >> 8 ),   
        (uint8_t) val};
    avio_write(s, data, sizeof(data));
#endif
}

void avio_wb64(AVIOContext *s, uint64_t val)
{
    avio_wb32(s, (uint32_t)(val >> 32));
    avio_wb32(s, (uint32_t)(val & 0xffffffff));
}

void avio_wb16(AVIOContext *s, unsigned int val)
{
#ifdef TEST
    avio_w8(s, (int)val >> 8);
    avio_w8(s, (uint8_t)val);
#else
    uint8_t data[] = {   
        (uint8_t)(val >> 8 ),   
        (uint8_t) val};
    avio_write(s, data, sizeof(data));
#endif
}

void avio_wb24(AVIOContext *s, unsigned int val)
{
    avio_wb16(s, (int)val >> 8);
    avio_w8(s, (uint8_t)val);
}

/* Input stream */

static int url_resetbuf(AVIOContext *s)
{
    s->buf_end = s->buffer + s->buffer_size;
    return 0;
}
