//--=========================================================================--
//  This file is a part of VPU Reference API project
//-----------------------------------------------------------------------------
//
//       This confidential and proprietary software may be used only
//     as authorized by a licensing agreement from Chips&Media Inc.
//     In the event of publication, the following notice is applicable:
//
//            (C) COPYRIGHT 2006 - 2013  CHIPS&MEDIA INC.
//                      ALL RIGHTS RESERVED
//
//       The entire notice above must be reproduced on all authorized
//       copies.
//
//--=========================================================================--

#include "main_helper.h"

#define WAVE4_FRAME                 0x09000000
#define DISP_BASE                   0x2a000000

typedef struct HDMIContext_Struct {
    Uint32   curDPBufY;
    Uint32   curDPBufU;
    Uint32   curDPBufV;
    Uint32   count;
} HDMIContext;

BOOL HDMIDisplayDevice_Open(
    RenderDevice*   device
    )
{
    HDMIContext* ctx = (HDMIContext*)osal_malloc(sizeof(HDMIContext));
    ctx->curDPBufY = 0;
    ctx->curDPBufU = 0;
    ctx->curDPBufV = 0;
    ctx->count     = 0;
    device->context = ctx;
    return TRUE;
}

BOOL HDMIDisplayDevice_Close(
    RenderDevice*   device
    )
{
    osal_free(device->context);

    return TRUE;
}

void HDMIDisplayDevice_Render(
    RenderDevice*   device,
    DecOutputInfo*  fbInfo,     //!<< fbInfo is not used
    Uint8*          pYuv,
    Uint32          width,
    Uint32          height
    )
{
    HDMIContext* ctx = device->context;
    Int32        core_idx=0;
    Int32        fry,frcb,frcr;
    Int32        DP_BUFF_Y0 = 0x00000000;
    Int32        DP_BUFF_U0 = 0x00800000;
    Int32        DP_BUFF_V0 = 0x00c00000;
    Int32        DP_BUFF_Y1 = 0x01000000;
    Int32        DP_BUFF_U1 = 0x01800000;
    Int32        DP_BUFF_V1 = 0x01c00000;
    FrameBuffer* fp = &fbInfo->dispFrame;
    BOOL         is4K; 
    //-----------------------------------------------------------------------------------------------
    //disp setting
    is4K = (BOOL)(width > 1920 || height > 1088);

    if (fp->myIndex) {
        if(ctx->curDPBufY == DP_BUFF_Y1) {
            ctx->curDPBufY = DP_BUFF_Y0;
            ctx->curDPBufU = DP_BUFF_U0;
            ctx->curDPBufV = DP_BUFF_V0;
        }
        else {
            ctx->curDPBufY = DP_BUFF_Y1;
            ctx->curDPBufU = DP_BUFF_U1;
            ctx->curDPBufV = DP_BUFF_V1;
        }
    }
    else {
        if (ctx->curDPBufY == DP_BUFF_Y1) {
            ctx->curDPBufY =DP_BUFF_Y0;
            ctx->curDPBufU =DP_BUFF_U0;
            ctx->curDPBufV =DP_BUFF_V0;
        }
        else {
            ctx->curDPBufY =DP_BUFF_Y1;
            ctx->curDPBufU =DP_BUFF_U1;
            ctx->curDPBufV =DP_BUFF_V1;
        }
    }

    fry  = fp->bufY; 
    frcb = fp->bufCb;
    frcr = fp->bufCr;

    EnterLock(0);
    //FRAME Y COPY
    VpuWriteReg(core_idx, WAVE4_FRAME+0xc,  ((frcb- fry)/256));
    VpuWriteReg(core_idx, WAVE4_FRAME+0x10, fp->bufY );
    VpuWriteReg(core_idx, DISP_BASE+0x40,   ctx->curDPBufY);
    VpuWriteReg(core_idx, WAVE4_FRAME+0x4,  0x0);

    while(VpuReadReg(core_idx,WAVE4_FRAME+0x8)) {
        MSleep(1);
    }
    //FRAME cb COPY
    VpuWriteReg(core_idx, WAVE4_FRAME+0xc,  ((frcr- frcb)/256));
    VpuWriteReg(core_idx, WAVE4_FRAME+0x10, fp->bufCb );
    VpuWriteReg(core_idx, DISP_BASE+0x40,   ctx->curDPBufU);
    VpuWriteReg(core_idx, WAVE4_FRAME+0x4,  0x0);

    while(VpuReadReg(core_idx,WAVE4_FRAME+0x8)) {
        MSleep(1);
    }

    //FRAME CR COPY
    VpuWriteReg(core_idx, WAVE4_FRAME+0xc,  ((frcr- frcb)/256));
    VpuWriteReg(core_idx, WAVE4_FRAME+0x10, fp->bufCr );
    VpuWriteReg(core_idx, DISP_BASE+0x40,   ctx->curDPBufV);
    VpuWriteReg(core_idx, WAVE4_FRAME+0x4,  0x0);

    while(VpuReadReg(core_idx,WAVE4_FRAME+0x8)) {
        MSleep(1);
    }

    if (is4K == TRUE) {
        VpuWriteReg(core_idx, DISP_BASE+0x104, 0x00000000); //CLK 
        VpuWriteReg(core_idx, DISP_BASE+0x004, 0x00000003); //
        VpuWriteReg(core_idx, DISP_BASE+0x014, 0x00000006); //format 0 : RGB888i, 4 : YUV420p
        VpuWriteReg(core_idx, DISP_BASE+0x034, 0x00000f00);
        VpuWriteReg(core_idx, DISP_BASE+0x030, 0x00000001); //YUV->RGB
        VpuWriteReg(core_idx, DISP_BASE+0x000, 0x00000001); //start
    }
    else {
        VpuWriteReg(core_idx, DISP_BASE+0x104, 0x01);
        VpuWriteReg(core_idx, DISP_BASE+0x004, 0x02);
        VpuWriteReg(core_idx, DISP_BASE+0x034, fp->stride);
        VpuWriteReg(core_idx, DISP_BASE+0x014, 0x06);
        VpuWriteReg(core_idx, DISP_BASE+0x030, 0x01);
        VpuWriteReg(core_idx, DISP_BASE+0x000, 0x01);
    }

    VpuWriteReg(core_idx, DISP_BASE+0x008, ctx->curDPBufY);
    VpuWriteReg(core_idx, DISP_BASE+0x00c, ctx->curDPBufU);
    VpuWriteReg(core_idx, DISP_BASE+0x010, ctx->curDPBufV);

    ctx->count++;

    if (ctx->count == 1) {
        /* Waiting for initializing HDMI */
        MSleep(1);
    }

    LeaveLock(0);
}

