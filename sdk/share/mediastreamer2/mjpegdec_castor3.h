#ifndef MJPEGDEC_CASTOR3_H
#define MJPEGDEC_CASTOR3_H

#ifdef __cplusplus
extern "C" {
#endif

#define NUM_DATA_POINTERS 	4
typedef struct YUV_FRAME_TAG
{
	uint8_t *data[NUM_DATA_POINTERS];
	int linesize[NUM_DATA_POINTERS];
	int width;
	int height;
} YUV_FRAME;


#ifdef __cplusplus
}
#endif

#endif

