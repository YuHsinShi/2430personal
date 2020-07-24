#ifndef ITC_IPCAM_H
#define ITC_IPCAM_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef CFG_BUILD_FFMPEG
typedef struct AVPacket {
    int64_t pts;
    int64_t dts;
    uint8_t *data;
    int   size;
    int   stream_index;

    int   duration;
    void  (*destruct)(struct AVPacket *);
    void  *priv;
    int64_t pos;                            ///< byte position in stream, -1 if unknown

    double timestamp;
} AVPacket;

typedef struct AVPacketList {
    AVPacket pkt;
    struct AVPacketList *next;
} AVPacketList;

typedef struct
{
    int width;
    int height;
    int max_framerate;
} RTSP_MEDIA_INFO;

typedef enum
{
    STREAM_EOF,
    STREAM_ERROR,
    STREAM_CONNECT_FAIL,
	STREAM_START_PLAYING,
    STREAM_GET_INFO,
    STREAM_SET_VOLUME
} RTSPCLIENT_EVENT;
typedef void (*cb_RtspHandler_t)(RTSPCLIENT_EVENT event_id, void *arg);
#endif

int PutIntoPacketQueue(unsigned char* inputbuf, int inputbuf_size, double timestamp);

#ifdef __cplusplus
}
#endif

#endif
