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

#ifndef __VPUAPI_PRODUCT_ABSTRACT_H__
#define __VPUAPI_PRODUCT_ABSTRACT_H__

#include "vpuapi.h"
#include "vpuapifunc.h"

#define IS_CODA_DECODER_HANDLE(_inst)      (_inst->codecMode < AVC_ENC)

#define IS_WAVE_DECODER_HANDLE(_inst)      (_inst->codecMode == W_HEVC_DEC || _inst->codecMode == W_SVAC_DEC || _inst->codecMode == W_AVC_DEC || _inst->codecMode == W_VP9_DEC || _inst->codecMode == W_AVS2_DEC || _inst->codecMode == W_AV1_DEC)

enum {
    FramebufCacheNone,
    FramebufCacheMaverickI,
    FramebufCacheMaverickII,
};



typedef struct _tag_FramebufferIndex {
    Int16 tiledIndex;
    Int16 linearIndex;
} FramebufferIndex;

extern VpuAttr g_VpuCoreAttributes[MAX_NUM_VPU_CORE];

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*
 * Returns 0 - not found product id
 *         1 - found product id
 */
extern Uint32 ProductVpuScan(
    Uint32 coreIdx
    );


extern Int32 ProductVpuGetId(
    Uint32 coreIdx
    );

extern RetCode ProductVpuGetVersion(
    Uint32  coreIdx, 
    Uint32* versionInfo, 
    Uint32* revision 
    );

extern RetCode ProductVpuGetProductInfo(
    Uint32  coreIdx, 
    VpuAttr* productInfo 
    );

extern RetCode ProductVpuInit(
    Uint32 coreIdx,
    void*  firmware,
    Uint32 size
    );

extern RetCode ProductVpuReInit(
    Uint32 coreIdx,
    void*  firmware,
    Uint32 size
    );

extern Uint32 ProductVpuIsInit(
    Uint32 coreIdx
    );

extern Int32 ProductVpuIsBusy(
    Uint32 coreIdx
    );

extern Int32 ProductVpuWaitInterrupt(
    CodecInst*  instance,
    Int32       timeout
    );

extern RetCode ProductVpuReset(
    Uint32      coreIdx,
    SWResetMode resetMode
    );

extern RetCode ProductVpuSleepWake(
    Uint32 coreIdx,
    int iSleepWake,
     const Uint16* code, 
     Uint32 size
    );

extern RetCode ProductVpuClearInterrupt(
    Uint32      coreIdx,
    Uint32      flags
    );

extern RetCode ProductVpuDecBuildUpOpenParam(
    CodecInst*    instance,
    DecOpenParam* param
    );

extern RetCode ProductCheckDecOpenParam(
    DecOpenParam* param
    );

extern RetCode ProductVpuDecInitSeq(
    CodecInst*  instance
    );

extern RetCode ProductVpuDecFiniSeq(
    CodecInst*  instance
    );

extern RetCode ProductVpuDecSetBitstreamFlag(
    CodecInst*  instance,
    BOOL        running,
    Int32       size
    );

/*
 * FINI_SEQ
 */
extern RetCode ProductVpuDecEndSequence(
    CodecInst*  instance
    );

/**
 *  @brief      Abstract function for SEQ_INIT.
 */
extern RetCode ProductVpuDecGetSeqInfo(
    CodecInst*      instance, 
    DecInitialInfo* info
    );

/**
 *  \brief      Check parameters for product specific decoder.
 */
extern RetCode ProductVpuDecCheckCapability(
    CodecInst*  instance
    );

/**
 * \brief       Decode a coded picture.  
 */
extern RetCode ProductVpuDecode(
    CodecInst*  instance,
    DecParam*   option
    );

/**
 *
 */
extern RetCode ProductVpuDecGetResult(
    CodecInst*      instance,
    DecOutputInfo*  result
    );

/**
 * \brief                   Flush framebuffers to prepare decoding new sequence
 * \param   instance        decoder handle
 * \param   retIndexes      Storing framebuffer indexes in display order. 
 *                          If retIndexes[i] is -1 then there is no display index from i-th.
 */
extern RetCode ProductVpuDecFlush(
    CodecInst*          instance,
    FramebufferIndex*   retIndexes,
    Uint32              size
    );

/**
 *  \brief      Allocate framebuffers with given parameters 
 */
extern RetCode ProductVpuAllocateFramebuffer(
    CodecInst*          instance, 
    FrameBuffer*        fbArr, 
    TiledMapType        mapType, 
    Int32               num, 
    Int32               stride, 
    Int32               height,
    FrameBufferFormat   format,
    BOOL                cbcrInterleave,
    BOOL                nv21,
    Int32               endian,
    vpu_buffer_t*       vb,
    Int32               gdiIndex,
    FramebufferAllocType fbType
    );

/**
 *  \brief      Register framebuffers to VPU
 */
extern RetCode ProductVpuRegisterFramebuffer(
    CodecInst*      instance
    );

extern RetCode ProductVpuDecUpdateFrameBuffer(
    CodecInst*   instance,
    FrameBuffer* fbcFb,
    FrameBuffer* linearFb,
    Uint32       mvColIndex,
    Uint32       picWidth, 
    Uint32       picHeight
    );

extern Int32 ProductCalculateFrameBufSize(
    CodecInst*          inst,
    Int32               productId, 
    Int32               stride, 
    Int32               height, 
    TiledMapType        mapType, 
    FrameBufferFormat   format,
    BOOL                interleave,
    DRAMConfig*         pDramCfg
    );
extern Int32 ProductCalculateAuxBufferSize(
    AUX_BUF_TYPE    type, 
    CodStd          codStd, 
    Int32           width, 
    Int32           height
    );

extern RetCode ProductClrDispFlag(
    CodecInst* instance, 
    Uint32 index
    );

extern RetCode ProductSetDispFlag(
    CodecInst* instance, 
    Uint32 dispFlag
    );

extern PhysicalAddress ProductVpuDecGetRdPtr(
    CodecInst* instance
    );

extern RetCode ProductVpuGetBandwidth(
    CodecInst* instance, 
    VPUBWData* data
    );


extern RetCode ProductVpuGetDebugInfo(
    CodecInst* instance,
    VPUDebugInfo* info
    );

/************************************************************************/
/* ENCODER                                                              */
/************************************************************************/
extern RetCode ProductVpuEncUpdateBitstreamBuffer(
    CodecInst* instance,
    Int32 size
    );

extern RetCode ProductVpuEncGetRdWrPtr(
    CodecInst* instance, 
    PhysicalAddress* rdPtr, 
    PhysicalAddress* wrPtr
    );

extern RetCode ProductVpuEncBuildUpOpenParam(
    CodecInst*      pCodec, 
    EncOpenParam*   param
    );

extern RetCode ProductVpuEncFiniSeq(
    CodecInst*      instance
    );

extern RetCode ProductCheckEncOpenParam(
    EncOpenParam*   param
    );

extern RetCode ProductVpuEncSetup(
    CodecInst*      instance
    );

extern RetCode ProductVpuEncode(
    CodecInst*      instance, 
    EncParam*       param
    );

extern RetCode ProductVpuEncGetResult(
    CodecInst*      instance, 
    EncOutputInfo*  result
    );

extern RetCode ProductVpuEncGiveCommand(
    CodecInst* instance, 
    CodecCommand cmd, 
    void* param);

extern RetCode ProductVpuEncInitSeq(
    CodecInst*  instance
    );
extern RetCode ProductVpuEncGetSeqInfo(
    CodecInst* instance, 
    EncInitialInfo* info
    );

extern RetCode ProductVpuGetSrcBufFlag(
    CodecInst* instance, 
    Uint32* data
    );

#ifdef __cplusplus
};
#endif /* __cplusplus */

#endif /* __VPUAPI_PRODUCT_ABSTRACT_H__ */
 
