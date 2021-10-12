//--=========================================================================--
//  This file is a part of VPU Reference API project
//-----------------------------------------------------------------------------
//
//       This confidential and proprietary software may be used only
//     as authorized by a licensing agreement from Chips&Media Inc.
//     In the event of publication, the following notice is applicable:
//
//            (C) COPYRIGHT 2006 - 2011  CHIPS&MEDIA INC.
//                      ALL RIGHTS RESERVED
//
//       The entire notice above must be reproduced on all authorized
//       copies.
//
//--=========================================================================--
#ifndef __CODA9_FUNCTION_H__
#define __CODA9_FUNCTION_H__

#include "vpuapi.h"
#include "../product.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

extern void 
Coda9BitIssueCommand(
    Uint32      coreIdx, 
    CodecInst*  inst, 
    int         cmd
    );

extern Uint32 Coda9VpuGetProductId(
    Uint32 coreIdx
    );

extern RetCode Coda9VpuGetVersion(
    Uint32  coreIdx, 
    Uint32* versionInfo, 
    Uint32* revision
    );

extern RetCode Coda9VpuInit(
    Uint32   coreIdx,
    void*    firmware,
    Uint32   size
    );

extern RetCode Coda9VpuReInit(
	Uint32   coreIdx,
	void*    firmware,
	Uint32   size
	);

extern Uint32 Coda9VpuIsInit(
    Uint32 coreIdx
    );

extern Int32 Coda9VpuIsBusy(
    Uint32 coreIdx
    );

extern Int32 Coda9VpuWaitInterrupt(
    CodecInst*  handle,
    Int32       timeout
    );

extern RetCode Coda9VpuReset(
    Uint32      coreIdx, 
    SWResetMode resetMode
    );

extern RetCode Coda9VpuSleepWake(
    Uint32 coreIdx, 
    int iSleepWake,
    const Uint16* code,
    Uint32 size
    );

extern RetCode Coda9VpuClearInterrupt(
    Uint32  coreIdx
    );

extern RetCode Coda9VpuFiniSeq(
    CodecInst*  instance
    );

extern RetCode Coda9VpuBuildUpDecParam(
    CodecInst*      instance, 
    DecOpenParam*   param
    );

extern RetCode Coda9VpuDecInitSeq(
    DecHandle handle
    );

extern RetCode Coda9VpuDecRegisterFramebuffer(
    CodecInst* instance
    );

extern RetCode Coda9VpuDecSetBitstreamFlag(
    CodecInst*  instance, 
    BOOL        running, 
    BOOL        eos
    );

extern RetCode Coda9VpuDecGetSeqInfo(
    CodecInst*      instance, 
    DecInitialInfo* info
    );

extern RetCode Coda9VpuDecode(
    CodecInst* instance,
    DecParam*  option
    );

extern RetCode Coda9VpuDecGetResult(
    CodecInst*      instance,
    DecOutputInfo*  result
    );

extern RetCode Coda9VpuDecFlush(
    CodecInst*          instance, 
    FramebufferIndex*   framebufferIndexes,
    Uint32              size
    );

/************************************************************************/
/* Encoder                                                              */
/************************************************************************/
extern RetCode Coda9VpuEncRegisterFramebuffer(
    CodecInst*      instance
    );

extern RetCode Coda9VpuBuildUpEncParam(
    CodecInst*      pCodec, 
    EncOpenParam*   param
    );

extern RetCode Coda9VpuEncSetup(
    CodecInst*      instance
    );

extern RetCode Coda9VpuEncode(
    CodecInst*      pCodecInst, 
    EncParam*       param
    );

extern RetCode Coda9VpuEncGetResult(
    CodecInst*      pCodecInst, 
    EncOutputInfo*  info
    );

extern RetCode Coda9VpuEncGiveCommand(
    CodecInst*      pCodecInst, 
    CodecCommand    cmd, 
    void*           param
    );
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __CODA9_FUNCTION_H__ */
 
