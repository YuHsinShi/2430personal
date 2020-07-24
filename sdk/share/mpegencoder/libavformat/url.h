/*
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

/**
 * @file
 * unbuffered private I/O API
 */

#ifndef AVFORMAT_URL_H
#define AVFORMAT_URL_H

#include "avio.h"
//#include "version.h"

#include "../libavutil/dict.h"

#define URL_PROTOCOL_FLAG_NESTED_SCHEME 1 /*< The protocol name can be the first part of a nested protocol scheme */
#define URL_PROTOCOL_FLAG_NETWORK       2 /*< The protocol uses network */

typedef struct URLContext {
    struct URLProtocol *prot;
    void *priv_data;
    char *filename;             /**< specified URL */
    int flags;
    int max_packet_size;        /**< if non zero, the stream is packetized with this max packet size */
    int is_streamed;            /**< true if streamed (no seek possible), default = false */
    int is_connected;
    int64_t rw_timeout;         /**< maximum time to wait for (network) read/write operation completion, in mcs */
} URLContext;

typedef struct URLProtocol {
    const char *name;
    int     (*url_open)( URLContext *h, const char *url, int flags);
    /**
     * This callback is to be used by protocols which open further nested
     * protocols. options are then to be passed to ffurl_open()/ffurl_connect()
     * for those nested protocols.
     */
    int     (*url_open2)(URLContext *h, const char *url, int flags);

    /**
     * Read data from the protocol.
     * If data is immediately available (even less than size), EOF is
     * reached or an error occurs (including EINTR), return immediately.
     * Otherwise:
     * In non-blocking mode, return AVERROR(EAGAIN) immediately.
     * In blocking mode, wait for data/EOF/error with a short timeout (0.1s),
     * and return AVERROR(EAGAIN) on timeout.
     * Checking interrupt_callback, looping on EINTR and EAGAIN and until
     * enough data has been read is left to the calling function; see
     * retry_transfer_wrapper in avio.c.
     */
    int     (*url_read)( URLContext *h, unsigned char *buf, int size);
    int     (*url_write)(URLContext *h, const unsigned char *buf, int size);
    int64_t (*url_seek)( URLContext *h, int64_t pos, int whence);
    int     (*url_close)(URLContext *h);
    struct URLProtocol *next;
    int (*url_read_pause)(URLContext *h, int pause);
    int64_t (*url_read_seek)(URLContext *h, int stream_index,
                             int64_t timestamp, int flags);
    int (*url_get_file_handle)(URLContext *h);
    int (*url_get_multi_file_handle)(URLContext *h, int **handles,
                                     int *numhandles);
    int (*url_shutdown)(URLContext *h, int flags);
    int priv_data_size;
    int flags;
    int (*url_check)(URLContext *h, int mask);
} URLProtocol;

/**
 * Register the URLProtocol protocol.
 *
 * @param size the size of the URLProtocol struct referenced
 */
int ffurl_register_protocol(URLProtocol *protocol, int size);

/**
 * Iterate over all available protocols.
 *
 * @param prev result of the previous call to this functions or NULL.
 */
URLProtocol *ffurl_protocol_next(URLProtocol *prev);

#endif /* AVFORMAT_URL_H */
