/*
 * Copyright (c) 2018 ITE technology Corp. All Rights Reserved.
 */
/** @copy from file itv.c
 *   @just for testcap , test only.
 *
 * @version 0.1
 */
#ifndef TEST_ITV_H
#define TEST_ITV_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ite/ith.h"
#include "ite/itu.h"
#include "isp/mmp_isp.h"


//=============================================================================
//                              Constant Definition
//=============================================================================
#define ITV_MAX_NDBUF    3
#define ITV_MAX_DISP_BUF 3

//=============================================================================
//                              Structure Definition
//=============================================================================
typedef struct
{
    uint8_t  *ya;       /// address of Y decoded video buffer
    uint8_t  *ua;       /// address of U decoded video buffer
    uint8_t  *va;       /// address of V decoded video buffer
    uint8_t  *old_y;    /// old address of Y, for 3d interlance
    uint32_t src_w;     /// width of decoded video buffer
    uint32_t src_h;     /// height of decoded video buffer
    int      bidx;
    uint32_t pitch_y;   /// pitch of Y
    uint32_t pitch_uv;  /// pitch of UV
    uint32_t format;    /// YUV format. see MMP_ISP_INFORMAT.
} ITV_DBUF_PROPERTY;

//=============================================================================
//                Public Function Definition
//=============================================================================
int      itp_itv_init(void);
int      itp_itv_deinit(void);
int      itp_itv_set_pb_mode(int pb_mode);
uint8_t *itp_itv_get_dbuf_anchor(void);
int      itp_itv_update_dbuf_anchor(ITV_DBUF_PROPERTY *prop);
void     itp_itv_flush_dbuf(void);
void     itp_itv_set_video_window(uint32_t  startX, uint32_t  startY, uint32_t  width, uint32_t  height);
ISP_RESULT itp_itv_enable_isp_feature(MMP_ISP_CAPS cap);
ISP_RESULT itp_itv_enable_3d_deinterlance(bool flag);


#ifdef __cplusplus
}
#endif

#endif // TEST_ITV_H