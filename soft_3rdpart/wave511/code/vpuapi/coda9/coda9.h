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
 
