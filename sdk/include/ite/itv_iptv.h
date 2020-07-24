/*
 * Copyright (c) 2015 ITE technology Corp. All Rights Reserved.
 */
/** @file itv.h
 * Used to do H/W video overlay
 *
 * @author I-Chun Lai
 * @version 0.1
 */
#ifndef ITV_H
#define ITV_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ite/ith.h"
#include "ite/itu.h"
#include "isp/mmp_isp.h"
#include "capture/capture_9860/mmp_capture.h"

//=============================================================================
//                              Constant Definition
//=============================================================================
#define ITV_MAX_NDBUF    3
#define ITV_MAX_DISP_BUF 3
#define ITV_MAX_RMI_NINS        1
#define ITV_MAX_RMI_DBUF_ENTRY  288

#define MTAL_MAX_CSTRING_LEN    1024

/* FRAME FUNCTION */
#define ITV_FF_NSET 2
#define ITV_FF_NBUF 2

#define ITV_MAX_OOS_DEFERRED 6

///////////////////////////////////////////////////////////
#define MTAL_EVERYTHING_OK                      0
#define MTAL_ERR_VIDEO_RESOLUTION_NOT_SUPPORT   1
#define MTAL_ERR_VIDEO_CODEC_NOT_SUPPORT        2

//=============================================================================
//                              Structure Definition
//=============================================================================
typedef enum
{
    ITV_640x480p60 = 0,     //0
    ITV_480i59,             //1
    ITV_480p59,             //2
    ITV_480i60,             //3
    ITV_480p60,             //4
    ITV_576i50,             //5
    ITV_576p50,             //6
    ITV_720p50,             //7
    ITV_720p59,             //8
    ITV_720p60,             //9
    ITV_1080p23,            //10
    ITV_1080p24,            //11
    ITV_1080p25,            //12
    ITV_1080p29,            //13
    ITV_1080p30,            //14
    ITV_1080i50,            //15
    ITV_1080p50,            //16
    ITV_1080i59,            //17
    ITV_1080p59,            //18
    ITV_1080i60,            //19
    ITV_1080p60,            //20
    ITV_800x600p60,         //21
    ITV_1024x768p60,        //22
    ITV_1280x768p60,        //23
    ITV_1280x800p60,        //24
    ITV_1280x960p60,        //25
    ITV_1280x1024p60,       //26
    ITV_1360x768p60,        //27
    ITV_1366x768p60,        //28
    ITV_1440x900p60,        //29
    ITV_1400x1050p60,       //30
    ITV_1440x1050p60,       //31
    ITV_1600x900p60,        //32
    ITV_1600x1200p60,       //33
    ITV_1680x1050p60,       //34
    ITV_RESOLUTION_UNKNOWN, //35
} ITV_ENUM_DISPLAY_FORMAT;

enum {
    MCTL_CMD_FLUSH,
}; // MCTL_CMD

typedef enum {
    MSG_SYS_CMD,
    MSG_CHANGE_DISPLAY_FORMAT,
    MSG_NO_SIGNAL,
    MSG_RESOLUTION_NOT_SUPPORTED,
    MSG_WAIT_CONNECTION,
    MSG_CONNECT_SUCCESS,
	MSG_CLEAN_OSD,
    MSG_MCTL_CMD,
} ITV_EVENT_MESSAGE;

typedef struct
{
    char srcname[MTAL_MAX_CSTRING_LEN];
    unsigned pos_x;
    unsigned pos_y;
    unsigned src_w;
    unsigned src_h;
} MTAL_STREAM_SPEC;

typedef struct
{
    int n_strm;
    MTAL_STREAM_SPEC strm_spec[ITV_MAX_RMI_NINS];
} MTAL_SPEC;

/*
 * These function pointers should be implemented in ffmpeg
 * filter components, if doing so, then itv can central
 * control behavior of filters in media controller.
 */
#define __USR_CALLBACK__
typedef struct
{
    // member
    void *component_prop;

    // behavior
    /**
     * flush - callback method
     *  Flush source stream buffer
     *
     * @caller: Media Controller
     * @maintain: URLProtocol implementation
     *
     * @param: priv_data
     *  User defined structure in URLProtocol
     */
    __USR_CALLBACK__ int (*flush)(void *component_property);
} FF_COMPONENT;

typedef enum {
    ITV_SRC_COMPONENT = 0,
    ITV_DEMUX_COMPONENT,
    ITV_DECODER_COMPONENT,
} ITV_COMPONENT_TYPE;

typedef struct
{
	signed long long target_pts;

    uint8_t  *ya;       /// address of Y decoded video buffer
    uint8_t  *ua;       /// address of U decoded video buffer
    uint8_t  *va;       /// address of V decoded video buffer
    uint32_t src_w;     /// width of decoded video buffer
    uint32_t src_h;     /// height of decoded video buffer
    int      bidx;
    uint32_t pitch_y;   /// pitch of Y
    uint32_t pitch_uv;  /// pitch of UV
    uint32_t format;    /// YUV format. see MMP_ISP_INFORMAT.
} ITV_DBUF_PROPERTY;

typedef struct
{
    uint32_t startX;
    uint32_t startY;
    uint32_t width;
    uint32_t height;
    uint32_t pitch;

    /// color key value ([A]lpha, [R]ed, [G]reen, [B]lue)
    uint32_t colorKeyR;
    uint32_t colorKeyG;
    uint32_t colorKeyB;
#if (CFG_CHIP_FAMILY == 970 || CFG_CHIP_FAMILY == 9860)
    uint32_t EnAlphaBlend;
#endif
    uint32_t constAlpha;
} ITV_UI_PROPERTY;

//=============================================================================
//                Public Function Definition
//=============================================================================
int      itv_init(void);
int      itv_deinit(void);
int      itv_set_pb_mode(int pb_mode);

/* FRAME FUNCTION */
void 	itv_cleanup_screen();

int      itv_ff_setup_base(int id, int bid, uint8_t *base);
int      itv_ff_enable(int id, int enable);

uint8_t *itv_get_uibuf_anchor(int id);
int      itv_update_uibuf_anchor(int id, ITV_UI_PROPERTY *uiprop);
void     itv_flush_uibuf(int id);

uint8_t *itv_get_dbuf_anchor(void);
int      itv_update_dbuf_anchor(ITV_DBUF_PROPERTY *prop);
void     itv_flush_dbuf(void);

void     itv_set_rotation(ITURotation rot);
void
itv_set_video_window(
    uint32_t  startX,
    uint32_t  startY,
    uint32_t  width,
    uint32_t  height);

ISP_RESULT
itv_enable_isp_feature(
    MMP_ISP_CAPS cap);

void itv_set_vidSurf_buf(uint8_t* addr, uint8_t index);
int itv_get_vidSurf_index(void);
int itv_update_vidSurf_anchor(void);
void itv_stop_vidSurf_anchor(void);
ITURotation itv_get_rotation(void);
bool itv_get_new_video(void);
int itv_set_output_format(MMP_CAP_INPUT_INFO df, uint32_t audio_sample_rate);
/* MTAL */
int mtal_pb_open(MTAL_SPEC *spec);
int mtal_pb_close(void);

int mtal_pb_play(void);
int mtal_pb_pause(int pause_flag);
int mtal_pb_stop(void);

int mtal_pb_set_livesrc(int livesrc);
int mtal_pb_is_livesrc(void);

unsigned mtal_get_pb_mode(void);
void mtal_set_pb_mode(int pb_mode);

// FIXME: it's quite dangerous, shouldn't public
int mtal_set_component(ITV_COMPONENT_TYPE type, FF_COMPONENT *comp);
int mtal_set_dataInputStatus(bool bHasData);
bool mtal_get_dataInputStatus();
void mtal_get_source_path(char* pOutputPath);
void mtal_set_source_path(char* pSrcPath);

int mtal_ev_handler_init(void);
int mtal_ev_handler_deinit(void);
int mtal_put_message(ITV_EVENT_MESSAGE event, int data);

void mtal_set_switch_flag(int flag);
int mtal_get_switch_flag(void);
void mtal_set_delay_time(short delayTime);
short mtal_get_delay_time();
short mtal_get_delay_time();
bool mtal_get_screen_mode_flag(void);
void mtal_cal_display_area(int *width, int *height);
int mtal_message_handler(void);
void mtal_get_display_setting(int *width, int *height, int* framerate);
void mtal_set_security_session_key(uint8_t* pSessionKey);
void mtal_get_security_session_key(uint8_t* pSessionKey);
void mtal_decrypt_data(uint8_t* pData);
int mtal_decrypt_data_by_size(uint8_t* pData, int size);
bool mtal_get_source_lock_flag(void);
int mtal_set_source_lock_flag(bool bSourceLock);

#ifdef __cplusplus
}
#endif

#endif // ITV_H