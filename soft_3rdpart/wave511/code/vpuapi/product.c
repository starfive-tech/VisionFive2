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

#include "product.h"
#include "coda9/coda9.h"
#include "wave/wave5.h"

VpuAttr g_VpuCoreAttributes[MAX_NUM_VPU_CORE];

static Int32 s_ProductIds[MAX_NUM_VPU_CORE] = {
    PRODUCT_ID_NONE,
};

typedef struct FrameBufInfoStruct {
    Uint32 unitSizeHorLuma;
    Uint32 sizeLuma;
    Uint32 sizeChroma;
    BOOL   fieldMap;
} FrameBufInfo;


Uint32 ProductVpuScan(Uint32 coreIdx)
{
    Uint32  i, productId;
    Uint32 foundProducts = 0;

    /* Already scanned */
    if (s_ProductIds[coreIdx] != PRODUCT_ID_NONE) 
        return 1;

    for (i=0; i<MAX_NUM_VPU_CORE; i++) {
        productId = Coda9VpuGetProductId(i);
        if (productId == PRODUCT_ID_NONE) 
            productId = WaveVpuGetProductId(i);
        if (productId != PRODUCT_ID_NONE) {
            s_ProductIds[i] = productId;
            foundProducts++;
        }
    }

    return (foundProducts >= 1);
}


Int32 ProductVpuGetId(Uint32 coreIdx)
{
    return s_ProductIds[coreIdx];
}

RetCode ProductVpuGetVersion(
    Uint32  coreIdx, 
    Uint32* versionInfo, 
    Uint32* revision 
    )
{
    Int32   productId = s_ProductIds[coreIdx];
    RetCode ret = RETCODE_SUCCESS;

    switch (productId) {
    case PRODUCT_ID_960:
    case PRODUCT_ID_980:
        ret = Coda9VpuGetVersion(coreIdx, versionInfo, revision);
        break;
    case PRODUCT_ID_512:
    case PRODUCT_ID_515:
    case PRODUCT_ID_521:
    case PRODUCT_ID_511:
    case PRODUCT_ID_517:
        ret = Wave5VpuGetVersion(coreIdx, versionInfo, revision);
        break;
    default:
        ret = RETCODE_NOT_FOUND_VPU_DEVICE;
    }

    return ret;
}

RetCode ProductVpuGetProductInfo(
    Uint32  coreIdx, 
    VpuAttr* attr 
    )
{
    Int32   productId = s_ProductIds[coreIdx];
    RetCode ret = RETCODE_SUCCESS;

    switch (productId) {
    case PRODUCT_ID_960:
    case PRODUCT_ID_980:
        ret = RETCODE_NOT_FOUND_VPU_DEVICE;
        break;
    case PRODUCT_ID_512:
    case PRODUCT_ID_515:
    case PRODUCT_ID_521:
    case PRODUCT_ID_511:
    case PRODUCT_ID_517:
        osal_memcpy((void*)attr, (void*)&g_VpuCoreAttributes[coreIdx], sizeof(VpuAttr));
        break;
    default:
        ret = RETCODE_NOT_FOUND_VPU_DEVICE;
    }

    return ret;
}

RetCode ProductVpuInit(Uint32 coreIdx, void* firmware, Uint32 size)
{
    RetCode ret = RETCODE_SUCCESS; 
    int     productId;

    productId  = s_ProductIds[coreIdx];

    switch (productId) {
    case PRODUCT_ID_960:
    case PRODUCT_ID_980:
        ret = Coda9VpuInit(coreIdx, firmware, size);
        break;
    case PRODUCT_ID_512:
    case PRODUCT_ID_515:
    case PRODUCT_ID_521:
    case PRODUCT_ID_511:
    case PRODUCT_ID_517:
        ret = Wave5VpuInit(coreIdx, firmware, size);
        break;
    default:
        ret = RETCODE_NOT_FOUND_VPU_DEVICE;
    }

    return ret;
}

RetCode ProductVpuReInit(Uint32 coreIdx, void* firmware, Uint32 size)
{
    RetCode ret = RETCODE_SUCCESS; 
    int     productId;

    productId  = s_ProductIds[coreIdx];

    switch (productId) {
    case PRODUCT_ID_960:
    case PRODUCT_ID_980:
        ret = Coda9VpuReInit(coreIdx, firmware, size);
        break;
    case PRODUCT_ID_512:
    case PRODUCT_ID_515:
    case PRODUCT_ID_521:
    case PRODUCT_ID_511:
    case PRODUCT_ID_517:
        ret = Wave5VpuReInit(coreIdx, firmware, size);
        break;
    default:
        ret = RETCODE_NOT_FOUND_VPU_DEVICE;
    }

    return ret;
}

Uint32 ProductVpuIsInit(Uint32 coreIdx)
{
    Uint32  pc = 0;
    int     productId;

    productId  = s_ProductIds[coreIdx];

    if (productId == PRODUCT_ID_NONE) {
        ProductVpuScan(coreIdx);
        productId  = s_ProductIds[coreIdx];
    }

    switch (productId) {
    case PRODUCT_ID_960:
    case PRODUCT_ID_980:
        pc = Coda9VpuIsInit(coreIdx);
        break;
    case PRODUCT_ID_512:
    case PRODUCT_ID_515:
    case PRODUCT_ID_521:
    case PRODUCT_ID_511:
    case PRODUCT_ID_517:
        pc = Wave5VpuIsInit(coreIdx);
        break;
    }

    return pc;
}

Int32 ProductVpuIsBusy(Uint32 coreIdx)
{
    Int32  busy;
    int    productId;

    productId = s_ProductIds[coreIdx];

    switch (productId) {
    case PRODUCT_ID_960:
    case PRODUCT_ID_980:
        busy = Coda9VpuIsBusy(coreIdx);
        break;
    case PRODUCT_ID_512:
    case PRODUCT_ID_515:
    case PRODUCT_ID_521:
    case PRODUCT_ID_511:
    case PRODUCT_ID_517:
        busy = Wave5VpuIsBusy(coreIdx);
        break;
    default:
        busy = 0;
        break;
    }

    return busy;
}

Int32 ProductVpuWaitInterrupt(CodecInst *instance, Int32 timeout)
{
    int     productId;
    int     flag = -1;

    productId = s_ProductIds[instance->coreIdx];

    switch (productId) {
    case PRODUCT_ID_960:
    case PRODUCT_ID_980:
        flag = Coda9VpuWaitInterrupt(instance, timeout);
        break;
    case PRODUCT_ID_512:
    case PRODUCT_ID_515:
    case PRODUCT_ID_521:
    case PRODUCT_ID_511:
    case PRODUCT_ID_517:
        flag = Wave5VpuWaitInterrupt(instance, timeout, FALSE);
        break;
    default:
        flag = -1;
        break;
    }

    return flag;
}

RetCode ProductVpuReset(Uint32 coreIdx, SWResetMode resetMode)
{
    int     productId;
    RetCode ret = RETCODE_SUCCESS;

    productId = s_ProductIds[coreIdx];

    switch (productId) {
    case PRODUCT_ID_960:
    case PRODUCT_ID_980:
        ret = Coda9VpuReset(coreIdx, resetMode);
        break;
    case PRODUCT_ID_512:
    case PRODUCT_ID_515:
    case PRODUCT_ID_521:
    case PRODUCT_ID_511:
    case PRODUCT_ID_517:
        ret = Wave5VpuReset(coreIdx, resetMode);
        break;
    default:
        ret = RETCODE_NOT_FOUND_VPU_DEVICE;
        break;
    }

    return ret;
}

RetCode ProductVpuSleepWake(Uint32 coreIdx, int iSleepWake, const Uint16* code, Uint32 size)
{
    int     productId;
    RetCode ret = RETCODE_NOT_FOUND_VPU_DEVICE;

    productId = s_ProductIds[coreIdx];

    switch (productId) {
    case PRODUCT_ID_960:
    case PRODUCT_ID_980:
        ret = Coda9VpuSleepWake(coreIdx, iSleepWake, (void*)code, size);
        break;
    case PRODUCT_ID_512:
    case PRODUCT_ID_515:
    case PRODUCT_ID_521:
    case PRODUCT_ID_511:
    case PRODUCT_ID_517:
        ret = Wave5VpuSleepWake(coreIdx, iSleepWake, (void*)code, size, FALSE);
        break;
    }

    return ret;
}
RetCode ProductVpuClearInterrupt(Uint32 coreIdx, Uint32 flags)
{
    int     productId;
    RetCode ret = RETCODE_NOT_FOUND_VPU_DEVICE;

    productId = s_ProductIds[coreIdx];

    switch (productId) {
    case PRODUCT_ID_960:
    case PRODUCT_ID_980:
        ret = Coda9VpuClearInterrupt(coreIdx);
        break;
    case PRODUCT_ID_512:
    case PRODUCT_ID_515:
    case PRODUCT_ID_521:
    case PRODUCT_ID_511:
    case PRODUCT_ID_517:
        ret = Wave5VpuClearInterrupt(coreIdx, flags);
        break;
    }

    return ret;
}

RetCode ProductVpuDecBuildUpOpenParam(CodecInst* pCodec, DecOpenParam* param)
{
    Int32   productId;
    Uint32  coreIdx;
    RetCode ret = RETCODE_NOT_FOUND_VPU_DEVICE;

    coreIdx   = pCodec->coreIdx;
    productId = s_ProductIds[coreIdx];

    switch (productId) {
    case PRODUCT_ID_960:
    case PRODUCT_ID_980:
        ret = Coda9VpuBuildUpDecParam(pCodec, param);
        break;
    case PRODUCT_ID_512:
    case PRODUCT_ID_515:
    case PRODUCT_ID_521:
    case PRODUCT_ID_511:
    case PRODUCT_ID_517:
        ret = Wave5VpuBuildUpDecParam(pCodec, param);
        break;
    }

    return ret;
}

PhysicalAddress ProductVpuDecGetRdPtr(CodecInst* instance)
{
    Int32   productId;
    Uint32  coreIdx;
    PhysicalAddress retRdPtr; 
    DecInfo*    pDecInfo;
    RetCode ret = RETCODE_SUCCESS;

    pDecInfo = VPU_HANDLE_TO_DECINFO(instance);

    coreIdx   = instance->coreIdx;
    productId = s_ProductIds[coreIdx];

    switch (productId) {
    case PRODUCT_ID_512:
    case PRODUCT_ID_515:
    case PRODUCT_ID_521:
    case PRODUCT_ID_511:
    case PRODUCT_ID_517:
        ret = Wave5VpuDecGetRdPtr(instance, &retRdPtr);
        if (ret != RETCODE_SUCCESS)
            retRdPtr = pDecInfo->streamRdPtr;
        else {
            pDecInfo->streamRdPtr = retRdPtr;
		}
        break;
    default:
        retRdPtr = VpuReadReg(coreIdx, pDecInfo->streamRdPtrRegAddr);
        break;
    }

    return retRdPtr;

}


RetCode ProductVpuEncUpdateBitstreamBuffer(CodecInst* instance, Int32 size)
{
    Int32   productId;
    Uint32  coreIdx;
    RetCode ret = RETCODE_SUCCESS;
    BOOL    updateNewbsBuf = (BOOL)(size == 0);
    coreIdx   = instance->coreIdx;
    productId = s_ProductIds[coreIdx];

    switch (productId) {
    case PRODUCT_ID_521:
        ret = Wave5VpuEncUpdateBS(instance, updateNewbsBuf);
        break;
    default:
        ret = RETCODE_NOT_FOUND_VPU_DEVICE;
    }

    return ret;
}
RetCode ProductVpuEncGetRdWrPtr(CodecInst* instance, PhysicalAddress* rdPtr, PhysicalAddress* wrPtr)
{
    Int32   productId;
    Uint32  coreIdx;
    EncInfo*    pEncInfo;
    RetCode ret = RETCODE_SUCCESS;
    pEncInfo = VPU_HANDLE_TO_ENCINFO(instance);

    coreIdx   = instance->coreIdx;
    productId = s_ProductIds[coreIdx];

    switch (productId) {
    case PRODUCT_ID_521:
        ret = Wave5VpuEncGetRdWrPtr(instance, rdPtr, wrPtr);
        if (ret != RETCODE_SUCCESS) {
            *rdPtr = pEncInfo->streamRdPtr;
            *wrPtr = pEncInfo->streamWrPtr;
        }
        else {
            pEncInfo->streamRdPtr = *rdPtr;
            pEncInfo->streamWrPtr = *wrPtr;
        }
        break;
    default:
        *wrPtr = pEncInfo->streamWrPtr;
        *rdPtr = pEncInfo->streamRdPtr;
        break;
    }

    return ret;

}

RetCode ProductVpuEncBuildUpOpenParam(CodecInst* pCodec, EncOpenParam* param)
{
    Int32   productId;
    Uint32  coreIdx;
    RetCode ret = RETCODE_NOT_SUPPORTED_FEATURE;

    coreIdx   = pCodec->coreIdx;
    productId = s_ProductIds[coreIdx];

    switch (productId) {
    case PRODUCT_ID_960:
    case PRODUCT_ID_980:
        ret = Coda9VpuBuildUpEncParam(pCodec, param);
        break;
    case PRODUCT_ID_521:
        ret = Wave5VpuBuildUpEncParam(pCodec, param);
        break;
    default:
        ret = RETCODE_NOT_SUPPORTED_FEATURE;
    }

    return ret;
}

RetCode ProductCheckDecOpenParam(DecOpenParam* param)
{
    Int32       productId; 
    Uint32      coreIdx;
    VpuAttr*    pAttr;

    if (param == 0) 
        return RETCODE_INVALID_PARAM;

    if (param->coreIdx > MAX_NUM_VPU_CORE) 
        return RETCODE_INVALID_PARAM;

    coreIdx   = param->coreIdx;
    productId = s_ProductIds[coreIdx];
    pAttr     = &g_VpuCoreAttributes[coreIdx];

    if (param->bitstreamBuffer % 8) 
        return RETCODE_INVALID_PARAM;

    if (param->bitstreamMode == BS_MODE_INTERRUPT) {
        if (param->bitstreamBufferSize % 1024 || param->bitstreamBufferSize < 1024) 
            return RETCODE_INVALID_PARAM;
    }

    if (PRODUCT_ID_W_SERIES(productId)) {
        if (param->virtAxiID > 16) {
            // Maximum number of AXI channels is 15
            return RETCODE_INVALID_PARAM;
        }
    }

    // Check bitstream mode
    if ((pAttr->supportBitstreamMode&(1<<param->bitstreamMode)) == 0)
        return RETCODE_INVALID_PARAM;

    if ((pAttr->supportDecoders&(1<<param->bitstreamFormat)) == 0) 
        return RETCODE_INVALID_PARAM;

    /* check framebuffer endian */
    if ((pAttr->supportEndianMask&(1<<param->frameEndian)) == 0) {
        APIDPRINT("%s:%d Invalid frame endian(%d)\n", __FUNCTION__, __LINE__, (Int32)param->frameEndian);
        return RETCODE_INVALID_PARAM;
    }

    /* check streambuffer endian */
    if ((pAttr->supportEndianMask&(1<<param->streamEndian)) == 0) {
        APIDPRINT("%s:%d Invalid stream endian(%d)\n", __FUNCTION__, __LINE__, (Int32)param->streamEndian);
        return RETCODE_INVALID_PARAM;
    }

    /* check WTL */
    if (param->wtlEnable) {
        if (pAttr->supportWTL == 0) 
            return RETCODE_NOT_SUPPORTED_FEATURE;
        switch (productId) {
        case PRODUCT_ID_960:
        case PRODUCT_ID_980:
            if (param->wtlMode != FF_FRAME && param->wtlMode != FF_FIELD ) 
                return RETCODE_INVALID_PARAM;
            break;
        default:
            break;
        }
    }

    /* Tiled2Linear */
    if (param->tiled2LinearEnable) {
        if (pAttr->supportTiled2Linear == 0) 
            return RETCODE_NOT_SUPPORTED_FEATURE;

        if (productId == PRODUCT_ID_960 || productId == PRODUCT_ID_980) {
            if (param->tiled2LinearMode != FF_FRAME && param->tiled2LinearMode != FF_FIELD ) {
                APIDPRINT("%s:%d Invalid Tiled2LinearMode(%d)\n", __FUNCTION__, __LINE__, (Int32)param->tiled2LinearMode);
                return RETCODE_INVALID_PARAM;
            }
        }
    }
    if (productId == PRODUCT_ID_960 || productId == PRODUCT_ID_980) {
        if( param->mp4DeblkEnable == 1 && !(param->bitstreamFormat == STD_MPEG4 || param->bitstreamFormat == STD_H263 || param->bitstreamFormat == STD_MPEG2 || param->bitstreamFormat == STD_DIV3)) 
            return RETCODE_INVALID_PARAM;
        if (param->wtlEnable && param->tiled2LinearEnable) 
            return RETCODE_INVALID_PARAM;
    } 
    else {
        if (param->mp4DeblkEnable || param->mp4Class)
            return RETCODE_INVALID_PARAM;
        if (param->avcExtension)
            return RETCODE_INVALID_PARAM;
        if (param->tiled2LinearMode != FF_NONE)
            return RETCODE_INVALID_PARAM;
    }

    return RETCODE_SUCCESS;
}

RetCode ProductVpuDecInitSeq(CodecInst* instance)
{
    int         productId;
    RetCode     ret = RETCODE_NOT_FOUND_VPU_DEVICE;

    productId   = instance->productId;

    switch (productId) {
    case PRODUCT_ID_960:
    case PRODUCT_ID_980:
        ret = Coda9VpuDecInitSeq(instance);
        break;
    case PRODUCT_ID_512:
    case PRODUCT_ID_515:
    case PRODUCT_ID_521:
    case PRODUCT_ID_511:
    case PRODUCT_ID_517:
        ret = Wave5VpuDecInitSeq(instance);
        break;
    }

    return ret;
}

RetCode ProductVpuDecFiniSeq(CodecInst* instance)
{
    int         productId;
    RetCode     ret = RETCODE_NOT_FOUND_VPU_DEVICE;

    productId   = instance->productId;

    switch (productId) {
    case PRODUCT_ID_960:
    case PRODUCT_ID_980:
        ret = Coda9VpuFiniSeq(instance);
        break;
    case PRODUCT_ID_512:
    case PRODUCT_ID_515:
    case PRODUCT_ID_521:
    case PRODUCT_ID_511:
    case PRODUCT_ID_517:
        ret = Wave5VpuDecFiniSeq(instance);
        break;
    }

    return ret;
}

RetCode ProductVpuDecGetSeqInfo(CodecInst* instance, DecInitialInfo* info)
{
    int         productId;
    RetCode     ret = RETCODE_NOT_FOUND_VPU_DEVICE;

    productId   = instance->productId;

    switch (productId) {
    case PRODUCT_ID_960:
    case PRODUCT_ID_980:
        ret = Coda9VpuDecGetSeqInfo(instance, info);
        break;
    case PRODUCT_ID_512:
    case PRODUCT_ID_515:
    case PRODUCT_ID_521:
    case PRODUCT_ID_511:
    case PRODUCT_ID_517:
        ret = Wave5VpuDecGetSeqInfo(instance, info);
        break;
    }

    return ret;
}

RetCode ProductVpuDecCheckCapability(CodecInst* instance)
{
    DecInfo* pDecInfo;
    VpuAttr* pAttr     = &g_VpuCoreAttributes[instance->coreIdx];

    pDecInfo = &instance->CodecInfo->decInfo;

    if ((pAttr->supportDecoders&(1<<pDecInfo->openParam.bitstreamFormat)) == 0)
        return RETCODE_NOT_SUPPORTED_FEATURE;

    switch (instance->productId) {
    case PRODUCT_ID_960:
        if (pDecInfo->mapType >= TILED_FRAME_NO_BANK_MAP) 
            return RETCODE_NOT_SUPPORTED_FEATURE;
        if (pDecInfo->tiled2LinearMode == FF_FIELD) 
            return RETCODE_NOT_SUPPORTED_FEATURE;
        break;
    case PRODUCT_ID_980:
        if (pDecInfo->mapType >= COMPRESSED_FRAME_MAP) 
            return RETCODE_NOT_SUPPORTED_FEATURE;
        break;
    case PRODUCT_ID_512:
    case PRODUCT_ID_515:
    case PRODUCT_ID_521:
    case PRODUCT_ID_511:
    case PRODUCT_ID_517:
        if (pDecInfo->mapType != LINEAR_FRAME_MAP && pDecInfo->mapType != COMPRESSED_FRAME_MAP && pDecInfo->mapType != COMPRESSED_FRAME_MAP_DUAL_CORE_8BIT && pDecInfo->mapType != COMPRESSED_FRAME_MAP_DUAL_CORE_10BIT)
            return RETCODE_NOT_SUPPORTED_FEATURE;
        break;
    }

    return RETCODE_SUCCESS;
}

RetCode ProductVpuDecode(CodecInst* instance, DecParam* option)
{
    int         productId;
    RetCode     ret = RETCODE_NOT_FOUND_VPU_DEVICE;

    productId = instance->productId;

    switch (productId) {
    case PRODUCT_ID_960:
    case PRODUCT_ID_980:
        ret = Coda9VpuDecode(instance, option);
        break;
    case PRODUCT_ID_512:
    case PRODUCT_ID_515:
    case PRODUCT_ID_521:
    case PRODUCT_ID_511:
    case PRODUCT_ID_517:
        ret = Wave5VpuDecode(instance, option);
        break;
    }

    return ret;
}

RetCode ProductVpuDecGetResult(CodecInst*  instance, DecOutputInfo* result)
{
    int         productId;
    RetCode     ret = RETCODE_NOT_FOUND_VPU_DEVICE;

    productId = instance->productId;

    switch (productId) {
    case PRODUCT_ID_960:
    case PRODUCT_ID_980:
        ret = Coda9VpuDecGetResult(instance, result);
        break;
    case PRODUCT_ID_512:
    case PRODUCT_ID_515:
    case PRODUCT_ID_521:
    case PRODUCT_ID_511:
    case PRODUCT_ID_517:
        ret = Wave5VpuDecGetResult(instance, result);
        break;
    }

    return ret;
}

RetCode ProductVpuDecFlush(CodecInst* instance, FramebufferIndex* retIndexes, Uint32 size)
{
    RetCode ret = RETCODE_SUCCESS;

    switch (instance->productId) {
    case PRODUCT_ID_512:
    case PRODUCT_ID_515:
    case PRODUCT_ID_521:
    case PRODUCT_ID_511:
    case PRODUCT_ID_517:
        ret = Wave5VpuDecFlush(instance, retIndexes, size);
        break;
    default:
        ret = Coda9VpuDecFlush(instance, retIndexes, size);
        break;
    }

    return ret;
}

/************************************************************************/
/* Decoder & Encoder                                                    */
/************************************************************************/

RetCode ProductVpuDecSetBitstreamFlag(
    CodecInst*  instance,
    BOOL        running,
    Int32       size
    )
{
    int         productId;
    RetCode     ret = RETCODE_NOT_FOUND_VPU_DEVICE;
    BOOL        eos; 
    BOOL        checkEos;
    BOOL        explicitEnd;
    DecInfo*    pDecInfo = &instance->CodecInfo->decInfo;

    productId = instance->productId;

    eos      = (BOOL)(size == 0);
    checkEos = (BOOL)(size > 0);
    explicitEnd = (BOOL)(size == -2);

    switch (productId) {
    case PRODUCT_ID_960:
    case PRODUCT_ID_980:
        if (checkEos || explicitEnd) eos = (BOOL)((pDecInfo->streamEndflag&0x04) == 0x04);
        ret = Coda9VpuDecSetBitstreamFlag(instance, running, eos);
        break;
    case PRODUCT_ID_512:
    case PRODUCT_ID_515:
    case PRODUCT_ID_521:
    case PRODUCT_ID_511:
    case PRODUCT_ID_517:
        if (checkEos || explicitEnd) eos = (BOOL)pDecInfo->streamEndflag;
        ret = Wave5VpuDecSetBitstreamFlag(instance, running, eos, explicitEnd);
        break;
    }

    return ret;
}

/**
 * \param   stride          stride of framebuffer in pixel.
 */
RetCode ProductVpuAllocateFramebuffer(
    CodecInst* inst, FrameBuffer* fbArr, TiledMapType mapType, Int32 num, 
    Int32 stride, Int32 height, FrameBufferFormat format, 
    BOOL cbcrInterleave, BOOL nv21, Int32 endian, 
    vpu_buffer_t* vb, Int32 gdiIndex,
    FramebufferAllocType fbType)
{
    Int32           i;
    Uint32          coreIdx;
    vpu_buffer_t    vbFrame;
    FrameBufInfo    fbInfo;
    DecInfo*        pDecInfo = &inst->CodecInfo->decInfo;
    EncInfo*        pEncInfo = &inst->CodecInfo->encInfo;
    // Variables for TILED_FRAME/FILED_MB_RASTER
    Uint32          sizeLuma;
    Uint32          sizeChroma;
    ProductId       productId     = (ProductId)inst->productId;
    RetCode         ret           = RETCODE_SUCCESS;
    
    osal_memset((void*)&vbFrame, 0x00, sizeof(vpu_buffer_t));
    osal_memset((void*)&fbInfo,  0x00, sizeof(FrameBufInfo));

    coreIdx = inst->coreIdx;

    if (inst->codecMode == W_VP9_DEC) {
        Uint32 framebufHeight;
            framebufHeight = VPU_ALIGN64(height);
        sizeLuma   = CalcLumaSize(inst, inst->productId, stride, framebufHeight, format, cbcrInterleave, mapType, NULL);
        sizeChroma = CalcChromaSize(inst, inst->productId, stride, framebufHeight, format, cbcrInterleave, mapType, NULL);
    }
    else {
        DRAMConfig* bufferConfig = NULL;
        if (productId == PRODUCT_ID_960) {
            bufferConfig = &pDecInfo->dramCfg;
            bufferConfig = (inst->isDecoder == TRUE) ? &pDecInfo->dramCfg : &pEncInfo->dramCfg;
        }
        sizeLuma   = CalcLumaSize(inst, inst->productId, stride, height, format, cbcrInterleave, mapType, bufferConfig);
        sizeChroma = CalcChromaSize(inst, inst->productId, stride, height, format, cbcrInterleave, mapType, bufferConfig);
    }

    // Framebuffer common informations
    for (i=0; i<num; i++) {
        if (fbArr[i].updateFbInfo == TRUE ) {
            fbArr[i].updateFbInfo = FALSE;
            fbArr[i].myIndex        = i+gdiIndex;
            fbArr[i].stride         = stride;
            fbArr[i].height         = height;
            fbArr[i].mapType        = mapType;
            fbArr[i].format         = format;
            fbArr[i].cbcrInterleave = (mapType >= COMPRESSED_FRAME_MAP ? TRUE : cbcrInterleave);
            fbArr[i].nv21           = nv21;
            fbArr[i].endian         = (mapType >= COMPRESSED_FRAME_MAP ? VDI_128BIT_LITTLE_ENDIAN : endian);
            fbArr[i].lumaBitDepth   = pDecInfo->initialInfo.lumaBitdepth;
            fbArr[i].chromaBitDepth = pDecInfo->initialInfo.chromaBitdepth;
            fbArr[i].sourceLBurstEn = FALSE;
            if(inst->codecMode == W_HEVC_ENC || inst->codecMode == W_SVAC_ENC || inst->codecMode == W_AVC_ENC) {
                fbArr[i].endian         = (mapType >= COMPRESSED_FRAME_MAP ? VDI_128BIT_LITTLE_ENDIAN : endian);
                fbArr[i].lumaBitDepth   = pEncInfo->openParam.EncStdParam.waveParam.internalBitDepth;
                fbArr[i].chromaBitDepth = pEncInfo->openParam.EncStdParam.waveParam.internalBitDepth;
            }
        }
    }

    //********* START : framebuffers for SVAC spatial SVC **********/
    // Decoder
    if (inst->codecMode == W_SVAC_DEC && pDecInfo->initialInfo.spatialSvcEnable == TRUE && mapType >= COMPRESSED_FRAME_MAP) {
        stride = (pDecInfo->initialInfo.lumaBitdepth > 8) ? VPU_ALIGN32(VPU_ALIGN32(VPU_ALIGN128(pDecInfo->initialInfo.picWidth>>1)*5)/4) : VPU_ALIGN128(pDecInfo->initialInfo.picWidth>>1);
        for (i=num; i<num*2; i++) {
            if (fbArr[i].updateFbInfo == TRUE ) {
                fbArr[i].updateFbInfo = FALSE;
                fbArr[i].myIndex        = i+gdiIndex;
                fbArr[i].stride         = stride;
                fbArr[i].height         = VPU_ALIGN128(pDecInfo->initialInfo.picHeight>>1);
                fbArr[i].mapType        = COMPRESSED_FRAME_MAP_SVAC_SVC_BL;
                fbArr[i].format         = format;
                fbArr[i].cbcrInterleave = (mapType >= COMPRESSED_FRAME_MAP ? TRUE : cbcrInterleave);
                fbArr[i].nv21           = nv21;
                fbArr[i].endian         = endian;
                fbArr[i].lumaBitDepth   = pDecInfo->initialInfo.lumaBitdepth;
                fbArr[i].chromaBitDepth = pDecInfo->initialInfo.chromaBitdepth;
                fbArr[i].sourceLBurstEn = FALSE;
            }
        }
    }

    // Encoder
    if (inst->codecMode == W_SVAC_ENC && pEncInfo->openParam.EncStdParam.waveParam.svcEnable == TRUE && mapType >= COMPRESSED_FRAME_MAP) { // only set when allocating recon buffer. (do not set when allocating source buffer)
        stride = (pEncInfo->openParam.EncStdParam.waveParam.internalBitDepth > 8) ? VPU_ALIGN32(VPU_ALIGN32(VPU_ALIGN16(pEncInfo->openParam.picWidthBL)*5)/4) : VPU_ALIGN32(pEncInfo->openParam.picWidthBL);
        for (i=num; i<num*2; i++) {
            if (fbArr[i].updateFbInfo == TRUE ) {
                fbArr[i].updateFbInfo = FALSE;
                fbArr[i].myIndex        = i+gdiIndex;
                fbArr[i].stride         = stride;
                fbArr[i].height         = pEncInfo->openParam.picHeightBL;
                fbArr[i].mapType        = COMPRESSED_FRAME_MAP_SVAC_SVC_BL;
                fbArr[i].format         = format;
                fbArr[i].cbcrInterleave = (mapType >= COMPRESSED_FRAME_MAP ? TRUE : cbcrInterleave);
                fbArr[i].nv21           = nv21;
                fbArr[i].endian         = endian;
                fbArr[i].lumaBitDepth   = pEncInfo->openParam.EncStdParam.waveParam.internalBitDepth;
                fbArr[i].chromaBitDepth = pEncInfo->openParam.EncStdParam.waveParam.internalBitDepth;
                fbArr[i].sourceLBurstEn = FALSE;
                if(inst->codecMode == W_SVAC_ENC) {
                    fbArr[i].endian         = (mapType >= COMPRESSED_FRAME_MAP ? VDI_128BIT_LITTLE_ENDIAN : endian);
                    fbArr[i].lumaBitDepth   = pEncInfo->openParam.EncStdParam.waveParam.internalBitDepth;
                    fbArr[i].chromaBitDepth = pEncInfo->openParam.EncStdParam.waveParam.internalBitDepth;
                }
            }
        }
    }
    //********* END : framebuffers for SVAC spatial SVC **********/


    switch (mapType) {
    case LINEAR_FRAME_MAP:
    case LINEAR_FIELD_MAP:
    case COMPRESSED_FRAME_MAP:
    case COMPRESSED_FRAME_MAP_DUAL_CORE_8BIT:
    case COMPRESSED_FRAME_MAP_DUAL_CORE_10BIT:
        ret = UpdateFrameBufferAddr(mapType, fbArr, num, sizeLuma, sizeChroma);
        if (ret != RETCODE_SUCCESS)
            break;

        if (inst->codecMode == W_SVAC_DEC && pDecInfo->initialInfo.spatialSvcEnable == TRUE && mapType >= COMPRESSED_FRAME_MAP) {
            stride = (pDecInfo->initialInfo.lumaBitdepth > 8) ? VPU_ALIGN32(VPU_ALIGN32(VPU_ALIGN128(pDecInfo->initialInfo.picWidth>>1)*5)/4) : VPU_ALIGN128(pDecInfo->initialInfo.picWidth>>1);
            // Update FrameBufferAddr for SVC BL
            sizeLuma   = CalcLumaSize(inst, inst->productId, stride, VPU_ALIGN128(pDecInfo->initialInfo.picHeight>>1), format, cbcrInterleave, mapType, NULL);
            sizeChroma = CalcChromaSize(inst, inst->productId, stride, VPU_ALIGN128(pDecInfo->initialInfo.picHeight>>1), format, cbcrInterleave, mapType, NULL);
            ret = UpdateFrameBufferAddr(COMPRESSED_FRAME_MAP_SVAC_SVC_BL, fbArr+num, num, sizeLuma, sizeChroma);
        }
        if (inst->codecMode == W_SVAC_ENC && pEncInfo->openParam.EncStdParam.waveParam.svcEnable == TRUE  && mapType >= COMPRESSED_FRAME_MAP) { // only set when allocating recon buffer. (do not set when allocating source buffer)
            stride = (pEncInfo->openParam.EncStdParam.waveParam.internalBitDepth > 8) ? VPU_ALIGN32(VPU_ALIGN32(VPU_ALIGN16(pEncInfo->openParam.picWidthBL)*5)/4) : VPU_ALIGN32(pEncInfo->openParam.picWidthBL);
            sizeLuma   = CalcLumaSize(inst, inst->productId, stride, pEncInfo->openParam.picHeightBL, format, cbcrInterleave, mapType, NULL);
            sizeChroma = CalcChromaSize(inst, inst->productId, stride, pEncInfo->openParam.picHeightBL, format, cbcrInterleave, mapType, NULL);
            ret = UpdateFrameBufferAddr(COMPRESSED_FRAME_MAP_SVAC_SVC_BL, fbArr+num, num, sizeLuma, sizeChroma);
        }
        break;

    default:
        /* Tiled map */
        if (productId == PRODUCT_ID_960) {
            DRAMConfig*     pDramCfg;
            PhysicalAddress tiledBaseAddr = 0;
            TiledMapConfig* pMapCfg;

            pDramCfg = (inst->isDecoder == TRUE) ? &pDecInfo->dramCfg : &pEncInfo->dramCfg;
            pMapCfg  = (inst->isDecoder == TRUE) ? &pDecInfo->mapCfg  : &pEncInfo->mapCfg;
            vbFrame.phys_addr = GetTiledFrameBase(coreIdx, fbArr, num); 
            if (fbType == FB_TYPE_PPU) {
                tiledBaseAddr = pMapCfg->tiledBaseAddr;
            }
            else {
                pMapCfg->tiledBaseAddr = vbFrame.phys_addr;
                tiledBaseAddr = vbFrame.phys_addr;
            }
            *vb = vbFrame;
            ret = AllocateTiledFrameBufferGdiV1(mapType, tiledBaseAddr, fbArr, num, sizeLuma, sizeChroma, pDramCfg);
        }
        else {
            // PRODUCT_ID_980
            ret = AllocateTiledFrameBufferGdiV2(mapType, fbArr, num, sizeLuma, sizeChroma);
        }
        break;
    }
    for (i=0; i<num; i++) {
        if(inst->codecMode == W_HEVC_ENC || inst->codecMode == W_SVAC_ENC || inst->codecMode == W_AVC_ENC) {
            if (gdiIndex != 0) {        // FB_TYPE_PPU
                APIDPRINT("SOURCE FB[%02d] Y(0x%08x), Cb(0x%08x), Cr(0x%08x)\n", i, fbArr[i].bufY, fbArr[i].bufCb, fbArr[i].bufCr);
            }
        }
    }
    return ret;
}

RetCode ProductVpuRegisterFramebuffer(CodecInst* instance)
{
    RetCode         ret = RETCODE_FAILURE;
    FrameBuffer*    fb;
    DecInfo*        pDecInfo = &instance->CodecInfo->decInfo;
    Int32           gdiIndex = 0;
    EncInfo*        pEncInfo = &instance->CodecInfo->encInfo;
    switch (instance->productId) {
    case PRODUCT_ID_960:
    case PRODUCT_ID_980:
        if (IS_CODA_DECODER_HANDLE(instance))
            ret = Coda9VpuDecRegisterFramebuffer(instance);
        else 
            ret = Coda9VpuEncRegisterFramebuffer(instance);
        break;
    default:
        /************************************************************************/
        /*        for WAVE5 series (512/515/520/525/511/521...)                 */
        /************************************************************************/
        if (IS_WAVE_DECODER_HANDLE(instance)) {
            if (pDecInfo->mapType < COMPRESSED_FRAME_MAP)
                return RETCODE_NOT_SUPPORTED_FEATURE;

            fb = pDecInfo->frameBufPool;

            gdiIndex = 0;
            if (pDecInfo->wtlEnable == TRUE) {
                if (fb[0].mapType >= COMPRESSED_FRAME_MAP) 
                    gdiIndex = pDecInfo->numFbsForDecoding;

                if (instance->codecMode == W_SVAC_DEC && pDecInfo->initialInfo.spatialSvcEnable == TRUE)
                    gdiIndex = pDecInfo->numFbsForDecoding*2;

                ret = Wave5VpuDecRegisterFramebuffer(instance, &fb[gdiIndex], LINEAR_FRAME_MAP, pDecInfo->numFbsForWTL);
                if (ret != RETCODE_SUCCESS)
                    return ret;
                gdiIndex = gdiIndex == 0 ? pDecInfo->numFbsForDecoding: 0;
            }

            ret = Wave5VpuDecRegisterFramebuffer(instance, &fb[gdiIndex], COMPRESSED_FRAME_MAP, pDecInfo->numFbsForDecoding);
            if (ret != RETCODE_SUCCESS)
                return ret;

            if (instance->codecMode == W_SVAC_DEC && pDecInfo->initialInfo.spatialSvcEnable == TRUE) { // BL for SVC
                gdiIndex = pDecInfo->numFbsForDecoding;
                ret = Wave5VpuDecRegisterFramebuffer(instance, &fb[gdiIndex], COMPRESSED_FRAME_MAP_SVAC_SVC_BL, pDecInfo->numFbsForDecoding);
                if (ret != RETCODE_SUCCESS)
                    return ret;
            }
        }
        else {
            // ENCODER
            if (pEncInfo->mapType < COMPRESSED_FRAME_MAP)
                return RETCODE_NOT_SUPPORTED_FEATURE;

            fb = pEncInfo->frameBufPool;

            if (instance->codecMode == W_SVAC_ENC && pEncInfo->openParam.EncStdParam.waveParam.svcEnable == TRUE) {  // for BL
                gdiIndex = pEncInfo->numFrameBuffers;
                ret = Wave5VpuEncRegisterFramebuffer(instance, &fb[gdiIndex], COMPRESSED_FRAME_MAP_SVAC_SVC_BL, pEncInfo->numFrameBuffers);
            }
            gdiIndex = 0;   // for EL
            ret = Wave5VpuEncRegisterFramebuffer(instance, &fb[gdiIndex], COMPRESSED_FRAME_MAP, pEncInfo->numFrameBuffers);

            if (ret != RETCODE_SUCCESS)
                return ret;
        }
        break;
    }
    return ret;
}

RetCode ProductVpuDecUpdateFrameBuffer(CodecInst* instance, FrameBuffer* fbcFb, FrameBuffer* linearFb, Uint32 mvColIndex, Uint32 picWidth, Uint32 picHeight)
{
    RetCode ret = RETCODE_NOT_SUPPORTED_FEATURE;

    if (PRODUCT_ID_W_SERIES(instance->productId)) {
        EnterLock(instance->coreIdx);
        ret = Wave5VpuDecUpdateFramebuffer(instance, fbcFb, linearFb, mvColIndex, picWidth, picHeight);
        LeaveLock(instance->coreIdx);
    }

    return ret;
}

Int32 ProductCalculateFrameBufSize(CodecInst* inst, Int32 productId, Int32 stride, Int32 height, TiledMapType mapType, FrameBufferFormat format, BOOL interleave, DRAMConfig* pDramCfg)
{
    Int32 size_dpb_lum, size_dpb_chr, size_dpb_all;

    size_dpb_lum = CalcLumaSize(inst, productId, stride, height, format, interleave, mapType, pDramCfg);
    size_dpb_chr = CalcChromaSize(inst, productId, stride, height, format, interleave, mapType, pDramCfg);

    if (mapType < COMPRESSED_FRAME_MAP)
        size_dpb_all = size_dpb_lum + size_dpb_chr*2;
    else
        size_dpb_all = size_dpb_lum + size_dpb_chr;

    return size_dpb_all;
}


Int32 ProductCalculateAuxBufferSize(AUX_BUF_TYPE type, CodStd codStd, Int32 width, Int32 height)
{
    Int32 size = 0;

    switch (type) {
    case AUX_BUF_TYPE_MVCOL:
        if (codStd == STD_AVC || codStd == STD_VC1 || codStd == STD_MPEG4 || codStd == STD_H263 || codStd == STD_RV || codStd == STD_AVS ) {
            size =  VPU_ALIGN32(width)*VPU_ALIGN32(height);
            size = (size*3)/2;
            size = (size+4)/5;
            size = ((size+7)/8)*8;
        } 
        else if (codStd == STD_HEVC) {
            size = WAVE5_DEC_HEVC_MVCOL_BUF_SIZE(width, height);
        }
        else if (codStd == STD_VP9) {
            size = WAVE5_DEC_VP9_MVCOL_BUF_SIZE(width, height);
        }
        else if (codStd == STD_AVS2) {
            size = WAVE5_DEC_AVS2_MVCOL_BUF_SIZE(width, height);
        }
        else if (codStd == STD_AV1) {
            size = WAVE5_DEC_AV1_MVCOL_BUF_SIZE(width, height);
        }
        else {
            size = 0;
        }
        break;
    case AUX_BUF_TYPE_FBC_Y_OFFSET:
        size  = WAVE5_FBC_LUMA_TABLE_SIZE(width, height);
        break;
    case AUX_BUF_TYPE_FBC_C_OFFSET:
        size  = WAVE5_FBC_CHROMA_TABLE_SIZE(width, height);
        break;
    }

    return size;
}

RetCode ProductClrDispFlag(CodecInst* instance, Uint32 index)
{
    RetCode ret = RETCODE_SUCCESS;
    ret = Wave5DecClrDispFlag(instance, index);
    return ret;
}

RetCode ProductSetDispFlag(CodecInst* instance, Uint32 index)
{
    RetCode ret = RETCODE_SUCCESS;
    ret = Wave5DecSetDispFlag(instance, index);
    return ret;
}

RetCode ProductVpuGetBandwidth(CodecInst* instance, VPUBWData* data)
{
    if (data == 0) {
        return RETCODE_INVALID_PARAM;
    }                

    if (instance->productId < PRODUCT_ID_512)
        return RETCODE_INVALID_COMMAND;

    return Wave5VpuGetBwReport(instance, data);
}


RetCode ProductVpuGetDebugInfo(CodecInst* instance, VPUDebugInfo* info)
{
    if (info == 0) {
        return RETCODE_INVALID_PARAM;
    }               

    if (instance->productId < PRODUCT_ID_512)
        return RETCODE_INVALID_COMMAND;

    return Wave5VpuGetDebugInfo(instance, info);
}


/************************************************************************/
/* ENCODER                                                              */
/************************************************************************/
RetCode ProductVpuGetSrcBufFlag(CodecInst* instance, Uint32* flag) 
{
    if (instance->productId < PRODUCT_ID_521)
        return RETCODE_INVALID_COMMAND;

    return Wave5VpuGetSrcBufFlag(instance, flag);
}

RetCode ProductCheckEncOpenParam(EncOpenParam* pop)
{
    Int32       coreIdx;
    Int32       picWidth;
    Int32       picHeight;
    Int32       productId;
    VpuAttr*    pAttr;

    if (pop == 0) 
        return RETCODE_INVALID_PARAM;

    if (pop->coreIdx > MAX_NUM_VPU_CORE) 
        return RETCODE_INVALID_PARAM;

    coreIdx   = pop->coreIdx;
    picWidth  = pop->picWidth;
    picHeight = pop->picHeight;    
    productId = s_ProductIds[coreIdx];
    pAttr     = &g_VpuCoreAttributes[coreIdx];

    if ( pop->bitstreamFormat == STD_AVC && pop->srcBitDepth == 10 && pAttr->supportAVC10bitEnc != TRUE )
        return RETCODE_NOT_SUPPORTED_FEATURE;

    if ( pop->bitstreamFormat == STD_HEVC && pop->srcBitDepth == 10 && pAttr->supportHEVC10bitEnc != TRUE )
        return RETCODE_NOT_SUPPORTED_FEATURE;

    if ((pAttr->supportEncoders&(1<<pop->bitstreamFormat)) == 0) 
        return RETCODE_NOT_SUPPORTED_FEATURE;

    if (pop->ringBufferEnable == TRUE) {
        if (pop->bitstreamBuffer % 8) {
            return RETCODE_INVALID_PARAM;
        }

        if (productId == PRODUCT_ID_521) {
            if (pop->bitstreamBuffer % 16)
                return RETCODE_INVALID_PARAM;
        }

        if (productId == PRODUCT_ID_521) {
            if (pop->bitstreamBufferSize < (1024*64)) {
                return RETCODE_INVALID_PARAM;
            }
        }

        if (pop->bitstreamBufferSize % 1024 || pop->bitstreamBufferSize < 1024) 
            return RETCODE_INVALID_PARAM;
    }

    if (pop->frameRateInfo == 0) 
        return RETCODE_INVALID_PARAM;

    if (pop->bitstreamFormat == STD_AVC) {
        if (productId == PRODUCT_ID_980) {
            if (pop->bitRate > 524288 || pop->bitRate < 0) 
                return RETCODE_INVALID_PARAM;
        }
    }
    else if (pop->bitstreamFormat == STD_HEVC || pop->bitstreamFormat == STD_SVAC) {
        if ( productId == PRODUCT_ID_521) {
            if (pop->bitRate > 700000000 || pop->bitRate < 0) 
                return RETCODE_INVALID_PARAM;
        }
    }
    else {
        if (pop->bitRate > 32767 || pop->bitRate < 0) 
            return RETCODE_INVALID_PARAM;
    }

    if (pop->frameSkipDisable != 0 && pop->frameSkipDisable != 1)
        return RETCODE_INVALID_PARAM;

    if (pop->sliceMode.sliceMode != 0 && pop->sliceMode.sliceMode != 1) 
        return RETCODE_INVALID_PARAM;

    if (pop->sliceMode.sliceMode == 1) {
        if (pop->sliceMode.sliceSizeMode != 0 && pop->sliceMode.sliceSizeMode != 1) {
            return RETCODE_INVALID_PARAM;
        }
        if (pop->sliceMode.sliceSizeMode == 1 && pop->sliceMode.sliceSize == 0 ) {
            return RETCODE_INVALID_PARAM;
        }
    }

    if (pop->intraRefreshNum < 0) 
        return RETCODE_INVALID_PARAM;

    if (pop->MEUseZeroPmv != 0 && pop->MEUseZeroPmv != 1) 
        return RETCODE_INVALID_PARAM;

    if (pop->intraCostWeight < 0 || pop->intraCostWeight >= 65535) 
        return RETCODE_INVALID_PARAM;

    if (productId == PRODUCT_ID_980) {
        if (pop->MESearchRangeX < 0 || pop->MESearchRangeX > 4) {
            return RETCODE_INVALID_PARAM;
        }
        if (pop->MESearchRangeY < 0 || pop->MESearchRangeY > 3) {
            return RETCODE_INVALID_PARAM;
        }        
    }
    else {
        if (pop->MESearchRange < 0 || pop->MESearchRange >= 4) 
            return RETCODE_INVALID_PARAM;
    }

    if (pop->bitstreamFormat == STD_MPEG4) {
        EncMp4Param * param = &pop->EncStdParam.mp4Param;
        if (param->mp4DataPartitionEnable != 0 && param->mp4DataPartitionEnable != 1) {
            return RETCODE_INVALID_PARAM;
        }
        if (param->mp4DataPartitionEnable == 1) {
            if (param->mp4ReversibleVlcEnable != 0 && param->mp4ReversibleVlcEnable != 1) {
                return RETCODE_INVALID_PARAM;
            }
        }
        if (param->mp4IntraDcVlcThr < 0 || 7 < param->mp4IntraDcVlcThr) {
            return RETCODE_INVALID_PARAM;
        }

        if (picWidth < MIN_ENC_PIC_WIDTH || picWidth > MAX_ENC_PIC_WIDTH ) {
            return RETCODE_INVALID_PARAM;
        }

        if (picHeight < MIN_ENC_PIC_HEIGHT) {
            return RETCODE_INVALID_PARAM;
        }
    }
    else if (pop->bitstreamFormat == STD_H263) {
        EncH263Param * param = &pop->EncStdParam.h263Param;
        Uint32 frameRateInc, frameRateRes;

        if (param->h263AnnexJEnable != 0 && param->h263AnnexJEnable != 1) {
            return RETCODE_INVALID_PARAM;
        }
        if (param->h263AnnexKEnable != 0 && param->h263AnnexKEnable != 1) {
            return RETCODE_INVALID_PARAM;
        }
        if (param->h263AnnexTEnable != 0 && param->h263AnnexTEnable != 1) {
            return RETCODE_INVALID_PARAM;
        }

        if (picWidth < MIN_ENC_PIC_WIDTH || picWidth > MAX_ENC_PIC_WIDTH ) {
            return RETCODE_INVALID_PARAM;
        }
        if (picHeight < MIN_ENC_PIC_HEIGHT) {
            return RETCODE_INVALID_PARAM;
        }

        frameRateInc = ((pop->frameRateInfo>>16) &0xFFFF) + 1;
        frameRateRes = pop->frameRateInfo & 0xFFFF;

        if ((frameRateRes/frameRateInc) <15) {
            return RETCODE_INVALID_PARAM;
        }
    }
    else if (pop->bitstreamFormat == STD_AVC && productId != PRODUCT_ID_521) {

        EncAvcParam* param     = &pop->EncStdParam.avcParam;

        if (param->constrainedIntraPredFlag != 0 && param->constrainedIntraPredFlag != 1) 
            return RETCODE_INVALID_PARAM;
        if (param->disableDeblk != 0 && param->disableDeblk != 1 && param->disableDeblk != 2) 
            return RETCODE_INVALID_PARAM;
        if (param->deblkFilterOffsetAlpha < -6 || 6 < param->deblkFilterOffsetAlpha) 
            return RETCODE_INVALID_PARAM;
        if (param->deblkFilterOffsetBeta < -6 || 6 < param->deblkFilterOffsetBeta) 
            return RETCODE_INVALID_PARAM;
        if (param->chromaQpOffset < -12 || 12 < param->chromaQpOffset) 
            return RETCODE_INVALID_PARAM;
        if (param->audEnable != 0 && param->audEnable != 1) 
            return RETCODE_INVALID_PARAM;
        if (param->frameCroppingFlag != 0 &&param->frameCroppingFlag != 1) 
            return RETCODE_INVALID_PARAM;
        if (param->frameCropLeft & 0x01 || param->frameCropRight & 0x01 ||
            param->frameCropTop & 0x01  || param->frameCropBottom & 0x01) {
                return RETCODE_INVALID_PARAM;
        }

        if (picWidth < MIN_ENC_PIC_WIDTH || picWidth > MAX_ENC_PIC_WIDTH ) 
            return RETCODE_INVALID_PARAM;
        if (picHeight < MIN_ENC_PIC_HEIGHT) 
            return RETCODE_INVALID_PARAM;
    }
    else if (pop->bitstreamFormat == STD_HEVC || pop->bitstreamFormat == STD_SVAC || (pop->bitstreamFormat == STD_AVC && productId == PRODUCT_ID_521)) {
        EncWaveParam* param     = &pop->EncStdParam.waveParam;

        if (param->svcEnable == TRUE && pop->bitstreamFormat != STD_SVAC)
            return RETCODE_INVALID_PARAM;

        if (picWidth < W4_MIN_ENC_PIC_WIDTH || picWidth > W4_MAX_ENC_PIC_WIDTH)
            return RETCODE_INVALID_PARAM;

        if (picHeight < W4_MIN_ENC_PIC_HEIGHT || picHeight > W4_MAX_ENC_PIC_HEIGHT)
            return RETCODE_INVALID_PARAM;

        if (pop->bitstreamFormat == STD_HEVC) { // only for HEVC condition
            if (param->profile != HEVC_PROFILE_MAIN && param->profile != HEVC_PROFILE_MAIN10 && param->profile != HEVC_PROFILE_STILLPICTURE)
                return RETCODE_INVALID_PARAM;
            if (param->internalBitDepth > 8 && param->profile == HEVC_PROFILE_MAIN)
                return RETCODE_INVALID_PARAM;
        }

        if (param->internalBitDepth != 8 && param->internalBitDepth != 10)
            return RETCODE_INVALID_PARAM;

        if (param->internalBitDepth > 8 && param->profile == HEVC_PROFILE_MAIN)
            return RETCODE_INVALID_PARAM;

        if (pop->bitstreamFormat == STD_SVAC && param->svcEnable == TRUE && param->svcMode == 1) {
            // only low delay encoding supported when svc enable.
            if (param->gopPresetIdx == PRESET_IDX_IBPBP || param->gopPresetIdx == PRESET_IDX_IBBBP || param->gopPresetIdx == PRESET_IDX_RA_IB)
                return RETCODE_INVALID_PARAM;
        }

        if (param->decodingRefreshType < 0 || param->decodingRefreshType > 2)
            return RETCODE_INVALID_PARAM;

        if (param->gopPresetIdx == PRESET_IDX_CUSTOM_GOP) {
            if ( param->gopParam.customGopSize < 1 || param->gopParam.customGopSize > MAX_GOP_NUM)
                return RETCODE_INVALID_PARAM;
        }


        if (pop->bitstreamFormat == STD_AVC) {
            if (param->customLambdaEnable == 1) 
                return RETCODE_INVALID_PARAM;
        }
        if (param->constIntraPredFlag != 1 && param->constIntraPredFlag != 0)
            return RETCODE_INVALID_PARAM;

        if (param->intraRefreshMode < 0 || param->intraRefreshMode > 4)
            return RETCODE_INVALID_PARAM;
        

        if (pop->bitstreamFormat == STD_HEVC) {
            if (param->independSliceMode < 0 || param->independSliceMode > 1)
                return RETCODE_INVALID_PARAM;

            if (param->independSliceMode != 0) {
                if (param->dependSliceMode < 0 || param->dependSliceMode > 2)
                    return RETCODE_INVALID_PARAM;
            }
        }
        

        if (param->useRecommendEncParam < 0 && param->useRecommendEncParam > 3) 
            return RETCODE_INVALID_PARAM;

        if (param->useRecommendEncParam == 0 || param->useRecommendEncParam == 2 || param->useRecommendEncParam == 3) {

            if (param->intraNxNEnable != 1 && param->intraNxNEnable != 0)
                return RETCODE_INVALID_PARAM;
            
            if (param->skipIntraTrans != 1 && param->skipIntraTrans != 0)
                return RETCODE_INVALID_PARAM;

            if (param->scalingListEnable != 1 && param->scalingListEnable != 0)
                return RETCODE_INVALID_PARAM;

            if (param->tmvpEnable != 1 && param->tmvpEnable != 0)
                return RETCODE_INVALID_PARAM;

            if (param->wppEnable != 1 && param->wppEnable != 0)
                return RETCODE_INVALID_PARAM;

            if (param->useRecommendEncParam != 3) {     // in FAST mode (recommendEncParam==3), maxNumMerge value will be decided in FW
                if (param->maxNumMerge < 0 || param->maxNumMerge > 3) 
                    return RETCODE_INVALID_PARAM;
            }

            if (param->disableDeblk != 1 && param->disableDeblk != 0)
                return RETCODE_INVALID_PARAM;

            if (param->disableDeblk == 0 || param->saoEnable != 0) {
                if (param->lfCrossSliceBoundaryEnable != 1 && param->lfCrossSliceBoundaryEnable != 0)
                    return RETCODE_INVALID_PARAM;
            }

            if (param->disableDeblk == 0) {
                if (param->betaOffsetDiv2 < -6 || param->betaOffsetDiv2 > 6)
                    return RETCODE_INVALID_PARAM;

                if (param->tcOffsetDiv2 < -6 || param->tcOffsetDiv2 > 6)
                    return RETCODE_INVALID_PARAM;
            }
        }

        if (param->losslessEnable != 1 && param->losslessEnable != 0) 
            return RETCODE_INVALID_PARAM;

        if (param->intraQP < 0 || param->intraQP > 63) 
            return RETCODE_INVALID_PARAM;

        if (pop->bitstreamFormat == STD_SVAC && param->internalBitDepth != 8) {
            if (param->intraQP < 1 || param->intraQP > 63) 
                return RETCODE_INVALID_PARAM;
        }

        if (pop->rcEnable != 1 && pop->rcEnable != 0) 
            return RETCODE_INVALID_PARAM;

        if (pop->rcEnable == 1) {

            if (param->minQpI < 0 || param->minQpI > 63) 
                return RETCODE_INVALID_PARAM;
            if (param->maxQpI < 0 || param->maxQpI > 63) 
                return RETCODE_INVALID_PARAM;

            if (param->minQpP < 0 || param->minQpP > 63) 
                return RETCODE_INVALID_PARAM;
            if (param->maxQpP < 0 || param->maxQpP > 63) 
                return RETCODE_INVALID_PARAM;

            if (param->minQpB < 0 || param->minQpB > 63) 
                return RETCODE_INVALID_PARAM;
            if (param->maxQpB < 0 || param->maxQpB > 63) 
                return RETCODE_INVALID_PARAM;

            if (pop->bitstreamFormat == STD_SVAC && param->internalBitDepth != 8) {
                if (param->minQpI < 1 || param->minQpI > 63) 
                    return RETCODE_INVALID_PARAM;
                if (param->maxQpI < 1 || param->maxQpI > 63) 
                    return RETCODE_INVALID_PARAM;

                if (param->minQpP < 1 || param->minQpP > 63) 
                    return RETCODE_INVALID_PARAM;
                if (param->maxQpP < 1 || param->maxQpP > 63) 
                    return RETCODE_INVALID_PARAM;

                if (param->minQpB < 1 || param->minQpB > 63) 
                    return RETCODE_INVALID_PARAM;
                if (param->maxQpB < 1 || param->maxQpB > 63) 
                    return RETCODE_INVALID_PARAM;
            }
         
            if (param->cuLevelRCEnable != 1 && param->cuLevelRCEnable != 0) 
                return RETCODE_INVALID_PARAM;
            
            if (param->hvsQPEnable != 1 && param->hvsQPEnable != 0) 
                return RETCODE_INVALID_PARAM;

            if (param->hvsQPEnable) {
                if (param->hvsMaxDeltaQp < 0 || param->hvsMaxDeltaQp > 51) 
                    return RETCODE_INVALID_PARAM;
            }            
            
            if (param->bitAllocMode < 0 && param->bitAllocMode > 2) 
                return RETCODE_INVALID_PARAM;

            if (pop->vbvBufferSize < 10 || pop->vbvBufferSize > 3000 )
                return RETCODE_INVALID_PARAM;
        }

        // packed format & cbcrInterleave & nv12 can't be set at the same time. 
        if (pop->packedFormat == 1 && pop->cbcrInterleave == 1)
            return RETCODE_INVALID_PARAM;

        if (pop->packedFormat == 1 && pop->nv21 == 1)
            return RETCODE_INVALID_PARAM;

        // check valid for common param
        if (CheckEncCommonParamValid(pop) == RETCODE_FAILURE)
            return RETCODE_INVALID_PARAM;

        // check valid for RC param
        if (CheckEncRcParamValid(pop) == RETCODE_FAILURE)
            return RETCODE_INVALID_PARAM;

        if (param->gopPresetIdx == PRESET_IDX_CUSTOM_GOP) {
            if (CheckEncCustomGopParamValid(pop) == RETCODE_FAILURE)
                return RETCODE_INVALID_COMMAND;
        }

        if (param->chromaCbQpOffset < -12 || param->chromaCbQpOffset > 12)
            return RETCODE_INVALID_PARAM;

        if (param->chromaCrQpOffset < -12 || param->chromaCrQpOffset > 12)
            return RETCODE_INVALID_PARAM;
        
        if (param->intraRefreshMode == 3 && param-> intraRefreshArg == 0)
            return RETCODE_INVALID_PARAM;

        if (pop->bitstreamFormat == STD_HEVC) {
            if (param->nrYEnable != 1 && param->nrYEnable != 0) 
                return RETCODE_INVALID_PARAM;

            if (param->nrCbEnable != 1 && param->nrCbEnable != 0) 
                return RETCODE_INVALID_PARAM;

            if (param->nrCrEnable != 1 && param->nrCrEnable != 0) 
                return RETCODE_INVALID_PARAM;

            if (param->nrNoiseEstEnable != 1 && param->nrNoiseEstEnable != 0) 
                return RETCODE_INVALID_PARAM;

            if (param->nrNoiseSigmaY > 255)
                return RETCODE_INVALID_PARAM;

            if (param->nrNoiseSigmaCb > 255)
                return RETCODE_INVALID_PARAM;

            if (param->nrNoiseSigmaCr > 255)
                return RETCODE_INVALID_PARAM;

            if (param->nrIntraWeightY > 31)
                return RETCODE_INVALID_PARAM;

            if (param->nrIntraWeightCb > 31)
                return RETCODE_INVALID_PARAM;

            if (param->nrIntraWeightCr > 31)
                return RETCODE_INVALID_PARAM;

            if (param->nrInterWeightY > 31)
                return RETCODE_INVALID_PARAM;

            if (param->nrInterWeightCb > 31)
                return RETCODE_INVALID_PARAM;

            if (param->nrInterWeightCr > 31)
                return RETCODE_INVALID_PARAM;

            if((param->nrYEnable == 1 || param->nrCbEnable == 1 || param->nrCrEnable == 1) && (param->losslessEnable == 1))
                return RETCODE_INVALID_PARAM;
        }
        
        
        if (pop->bitstreamFormat == STD_SVAC) {
            // set parameters that not used for SVAC encoder to default value (=0)
            param->decodingRefreshType  = 0;
            param->customMDEnable       = 0;
            param->customLambdaEnable   = 0;
            param->maxNumMerge          = 0;

            if (param->chromaAcQpOffset < -3 || param->chromaAcQpOffset> 3)
                return RETCODE_INVALID_PARAM;

            if (param->chromaDcQpOffset < -3 || param->chromaDcQpOffset> 3)
                return RETCODE_INVALID_PARAM;

            if (param->lumaDcQpOffset < -3 || param->lumaDcQpOffset> 3)
                return RETCODE_INVALID_PARAM;
        }
       
    }
    if (pop->linear2TiledEnable == TRUE) {
        if (pop->linear2TiledMode != FF_FRAME && pop->linear2TiledMode != FF_FIELD ) 
            return RETCODE_INVALID_PARAM;
    }
    
    return RETCODE_SUCCESS;
}

RetCode ProductVpuEncFiniSeq(CodecInst* instance)
{
    RetCode     ret = RETCODE_NOT_FOUND_VPU_DEVICE;

    switch (instance->productId) {
    case PRODUCT_ID_960:
    case PRODUCT_ID_980:
        ret = Coda9VpuFiniSeq(instance);
        break;
    case PRODUCT_ID_512:
    case PRODUCT_ID_515:
    case PRODUCT_ID_511:
    case PRODUCT_ID_517:
        ret = RETCODE_NOT_SUPPORTED_FEATURE;
        break;
    case PRODUCT_ID_521:
        ret = Wave5VpuEncFiniSeq(instance);
        break;
    }
    return ret;
}

RetCode ProductVpuEncSetup(CodecInst* instance)
{
    RetCode     ret = RETCODE_NOT_FOUND_VPU_DEVICE;

    switch (instance->productId) {
    case PRODUCT_ID_960:
    case PRODUCT_ID_980:
        ret = Coda9VpuEncSetup(instance);
        break;
    case PRODUCT_ID_512:
    case PRODUCT_ID_515:
    case PRODUCT_ID_521:
    case PRODUCT_ID_511:
    case PRODUCT_ID_517:
        ret = RETCODE_NOT_SUPPORTED_FEATURE;
        break;
    }

    return ret;
}

RetCode ProductVpuEncode(CodecInst* instance, EncParam* param)
{
    RetCode     ret = RETCODE_NOT_FOUND_VPU_DEVICE;

    switch (instance->productId) {
    case PRODUCT_ID_960:
    case PRODUCT_ID_980:
        ret = Coda9VpuEncode(instance, param);
        break;
    case PRODUCT_ID_512:
    case PRODUCT_ID_515:
    case PRODUCT_ID_511:
    case PRODUCT_ID_517:
        ret = RETCODE_NOT_SUPPORTED_FEATURE;
        break;
    case PRODUCT_ID_521:
        ret = Wave5VpuEncode(instance, param);
        break;
    default:
        break;
    }

    return ret;
}

RetCode ProductVpuEncGetResult(CodecInst* instance, EncOutputInfo* result)
{
    RetCode     ret = RETCODE_NOT_FOUND_VPU_DEVICE;

    switch (instance->productId) {
    case PRODUCT_ID_960:
    case PRODUCT_ID_980:
        ret = Coda9VpuEncGetResult(instance, result);
        break;
    case PRODUCT_ID_512:
    case PRODUCT_ID_515:
    case PRODUCT_ID_511:
    case PRODUCT_ID_517:
        ret = RETCODE_NOT_SUPPORTED_FEATURE;
        break;
    case PRODUCT_ID_521:
        ret = Wave5VpuEncGetResult(instance, result);
        break;
    }

    return ret;
}

RetCode ProductVpuEncGiveCommand(CodecInst* instance, CodecCommand cmd, void* param)
{
    RetCode     ret = RETCODE_NOT_SUPPORTED_FEATURE;

    switch (instance->productId) {
    case PRODUCT_ID_960:
    case PRODUCT_ID_980:
        ret = Coda9VpuEncGiveCommand(instance, cmd, param);
        break;
    default:
        ret = Wave5VpuEncGiveCommand(instance, cmd, param);
        break;
    }
    
    return ret;
}

RetCode ProductVpuEncInitSeq(CodecInst* instance)
{
    int         productId;
    RetCode     ret = RETCODE_NOT_FOUND_VPU_DEVICE;

    productId   = instance->productId;

    switch (productId) {
    case PRODUCT_ID_521:
        ret = Wave5VpuEncInitSeq(instance);
        break;
    default:
        break;
    }

    return ret;
}

RetCode ProductVpuEncGetSeqInfo(CodecInst* instance, EncInitialInfo* info)
{
    int         productId;
    RetCode     ret = RETCODE_NOT_FOUND_VPU_DEVICE;

    productId   = instance->productId;

    switch (productId) {
    case PRODUCT_ID_521:
        ret = Wave5VpuEncGetSeqInfo(instance, info);
        break;
    default:
        break;
    }

    return ret;
}
 
