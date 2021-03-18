/*
 * Copyright (c) 2019, Chips&Media
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef __WAVE5_FUNCTION_H__
#define __WAVE5_FUNCTION_H__

#include "vpuapi.h"
#include "product.h"

#define WAVE5_TEMPBUF_OFFSET                (1024*1024)
#define WAVE5_TEMPBUF_SIZE                  (1024*1024)
#define WAVE5_TASK_BUF_OFFSET               (2*1024*1024)   // common mem = | codebuf(1M) | tempBuf(1M) | taskbuf0x0 ~ 0xF |

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define WAVE5_SUBSAMPLED_ONE_SIZE(_w, _h)       ((((_w/4)+15)&~15)*(((_h/4)+7)&~7))
#define WAVE5_AVC_SUBSAMPLED_ONE_SIZE(_w, _h)   ((((_w/4)+31)&~31)*(((_h/4)+3)&~3))

#define BSOPTION_ENABLE_EXPLICIT_END        (1<<0)

#define WTL_RIGHT_JUSTIFIED          0
#define WTL_LEFT_JUSTIFIED           1
#define WTL_PIXEL_8BIT               0
#define WTL_PIXEL_16BIT              1
#define WTL_PIXEL_32BIT              2

extern Uint32 Wave5VpuIsInit(
    Uint32      coreIdx
    );

extern Int32 Wave5VpuIsBusy(
    Uint32 coreIdx
    );

extern Int32 WaveVpuGetProductId(
    Uint32      coreIdx
    );

extern RetCode Wave5VpuEncGiveCommand(
    CodecInst *pCodecInst, 
    CodecCommand cmd, 
    void *param
    );

extern void Wave5BitIssueCommand(
    CodecInst* instance, 
    Uint32 cmd
    );

extern RetCode Wave5VpuGetVersion(
    Uint32  coreIdx, 
    Uint32* versionInfo, 
    Uint32* revision
    );

extern RetCode Wave5VpuInit(
    Uint32      coreIdx,
    void*       firmware,
    Uint32      size
    );

extern RetCode Wave5VpuSleepWake(
    Uint32 coreIdx, 
    int iSleepWake, 
    const Uint16* code, 
    Uint32 size,
    BOOL reset
    );

extern RetCode Wave5VpuReset(
    Uint32 coreIdx, 
    SWResetMode resetMode
    );

extern RetCode Wave5VpuBuildUpDecParam(
    CodecInst* instance, 
    DecOpenParam* param
    );

extern RetCode Wave5VpuDecSetBitstreamFlag(
    CodecInst* instance, 
    BOOL running, 
    BOOL eos,
    BOOL explictEnd
    );

extern RetCode Wave5VpuDecRegisterFramebuffer(
    CodecInst* inst, 
    FrameBuffer* fbArr, 
    TiledMapType mapType, 
    Uint32 count
    );

extern RetCode Wave5VpuDecUpdateFramebuffer(
    CodecInst*      inst, 
    FrameBuffer*    fbcFb, 
    FrameBuffer*    linearFb, 
    Int32           mvIndex,
    Int32           picWidth,
    Int32           picHeight
    );

extern RetCode Wave5VpuDecFlush(
    CodecInst* instance, 
    FramebufferIndex* framebufferIndexes, 
    Uint32 size
    );

extern RetCode Wave5VpuReInit(
    Uint32 coreIdx, 
    void* firmware, 
    Uint32 size
    );

extern RetCode Wave5VpuDecInitSeq(
    CodecInst* instance
    );

extern RetCode Wave5VpuDecGetSeqInfo(
    CodecInst* instance, 
    DecInitialInfo* info
    );

extern RetCode Wave5VpuDecode(
    CodecInst* instance, 
    DecParam* option
    );

extern RetCode Wave5VpuDecGetResult(
    CodecInst* instance, 
    DecOutputInfo* result
    );

extern RetCode Wave5VpuDecFiniSeq(
    CodecInst* instance
    );

extern RetCode Wave5DecWriteProtect(
    CodecInst* instance
    );

extern RetCode Wave5DecClrDispFlag(
    CodecInst* instance, 
    Uint32 index
    );

extern RetCode Wave5DecSetDispFlag(
    CodecInst* instance, 
    Uint32 index
    );

extern Int32 Wave5VpuWaitInterrupt(
    CodecInst*  instance, 
    Int32       timeout,
    BOOL        pending
    );

extern RetCode Wave5VpuClearInterrupt(
    Uint32 coreIdx, 
    Uint32 flags
    );

extern RetCode Wave5VpuDecGetRdPtr(
    CodecInst* instance, 
    PhysicalAddress *rdPtr
    );

extern RetCode Wave5VpuGetBwReport(
    CodecInst*  instance,
    VPUBWData*  bwMon 
    );


extern RetCode Wave5VpuGetDebugInfo(
    CodecInst* instance,
    VPUDebugInfo* info
    );


/***< WAVE5 Encoder >******/
RetCode Wave5VpuEncUpdateBS(
    CodecInst* instance,
    BOOL updateNewBsbuf
    );

RetCode Wave5VpuEncGetRdWrPtr(CodecInst* instance, 
    PhysicalAddress *rdPtr, 
    PhysicalAddress *wrPtr
    );

extern RetCode Wave5VpuBuildUpEncParam(
    CodecInst* instance, 
    EncOpenParam* param
    );

extern RetCode Wave5VpuEncInitSeq(
    CodecInst*instance
    );

extern RetCode Wave5VpuEncGetSeqInfo(
    CodecInst* instance, 
    EncInitialInfo* info
    );

#ifdef SUPPORT_LOOK_AHEAD_RC
extern RetCode Wave5VpuEncSetLarcData(
    CodecInst* instance,
    EncLarcInfo larcInfo
    );
#endif

extern RetCode Wave5VpuEncRegisterFramebuffer(
    CodecInst* inst, 
    FrameBuffer* fbArr, 
    TiledMapType mapType, 
    Uint32 count
    );

extern RetCode Wave5EncWriteProtect(
    CodecInst* instance
    );

extern RetCode Wave5VpuEncode(
    CodecInst* instance,
    EncParam* option
    );

extern RetCode Wave5VpuEncGetResult(
    CodecInst* instance,
    EncOutputInfo* result
    );

extern RetCode Wave5VpuEncGetHeader(
    EncHandle instance, 
    EncHeaderParam * encHeaderParam
    );

extern RetCode Wave5VpuEncFiniSeq(
    CodecInst*  instance 
    );

extern RetCode Wave5VpuEncParaChange(
    EncHandle instance, 
    EncChangeParam* param
    );

extern RetCode CheckEncCommonParamValid(
    EncOpenParam* pop
    );

extern RetCode CheckEncRcParamValid(
    EncOpenParam* pop
    );

extern RetCode CheckEncCustomGopParamValid(
    EncOpenParam* pop
    );

extern RetCode Wave5VpuGetSrcBufFlag(
    CodecInst* instance, 
    Uint32* flag
    );


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __WAVE5_FUNCTION_H__ */

