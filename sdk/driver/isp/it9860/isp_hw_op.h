#ifndef __ISP_HW_OP_H__
#define __ISP_HW_OP_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "isp_types.h"

//=============================================================================
//                        Macro Definition
//=============================================================================

//=============================================================================
//                        Structure Definition
//=============================================================================
typedef struct ISP_HW_OPERATION {
    void        (*ReadReg)(MMP_UINT32 addr, MMP_UINT32 *data);
    void        (*WriteReg)(const ISP_CONTEXT *ISPctxt, MMP_UINT32 addr, MMP_UINT32 data);
    void        (*WriteRegMask)(const ISP_CONTEXT *ISPctxt, MMP_UINT32 addr, MMP_UINT32 data, MMP_UINT32 mask);
    void        (*WaitQueue)(MMP_UINT32 sizeInByte);
    void        (*FireQueue)(void);
    void        (*FireIspQueue)(void);
    ISP_RESULT  (*WaitIspIdle)(const ISP_CONTEXT *ISPctxt);
} ISP_HW_OPERATION;

//=============================================================================
//				          Global Data Definition
//=============================================================================
extern ISP_HW_OPERATION gtIspHwOpIspQ;
extern ISP_HW_OPERATION gtIspHwOpCmdQ;
extern ISP_HW_OPERATION gtIspHwOpMMIO;
extern ISP_HW_OPERATION *gptIspHwOp;

//=============================================================================
//				          Public Function Definition
//=============================================================================
extern void
ISP_CmdSelect(
    ISP_CONTEXT *ISPctxt,
    ISP_HW_OPERATION   *ptIspHwOp);

extern void
ISP_ReadRegister(
    const ISP_CONTEXT *ISPctxt,
    MMP_UINT32         addr,
    MMP_UINT32         *data);

extern void
ISP_WriteRegister(
    const ISP_CONTEXT *ISPctxt,
    MMP_UINT32         addr,
    MMP_UINT32         data);

extern void
ISP_WriteRegisterMask(
    const ISP_CONTEXT *ISPctxt,
    MMP_UINT32         addr,
    MMP_UINT32         data,
    MMP_UINT32         mask);

extern void
ISP_CMD_QUEUE_WAIT(
    const ISP_CONTEXT *ISPctxt,
    MMP_UINT32         sizeInByte);

extern void
ISP_CMD_QUEUE_FIRE(
    const ISP_CONTEXT *ISPctxt);

extern void
ISP_FireIspQueue(
    const ISP_CONTEXT *ISPctxt);

extern ISP_RESULT
ISP_WaitIspIdle(
    const ISP_CONTEXT *ISPctxt);

#ifdef __cplusplus
}
#endif

#endif