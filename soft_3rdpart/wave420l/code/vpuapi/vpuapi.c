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

#include "vpuapifunc.h"
#include "product.h"
#include "wave/common/common_regdefine.h"
#include "wave/wave5/wave5.h"
            

#ifdef BIT_CODE_FILE_PATH
#include BIT_CODE_FILE_PATH
#endif

#define INVALID_CORE_INDEX_RETURN_ERROR(_coreIdx)  \
    if (_coreIdx >= MAX_NUM_VPU_CORE) \
    return -1;

static const Uint16* s_pusBitCode[MAX_NUM_VPU_CORE] = {NULL,};
static int s_bitCodeSize[MAX_NUM_VPU_CORE] = {0,};

Uint32 __VPU_BUSY_TIMEOUT = VPU_BUSY_CHECK_TIMEOUT;

static RetCode CheckInstanceValidity(CodecInst * pCodecInst)
{
    int i;
    vpu_instance_pool_t *vip;

    vip = (vpu_instance_pool_t *)vdi_get_instance_pool(pCodecInst->coreIdx);
    if (!vip)
        return RETCODE_INSUFFICIENT_RESOURCE;

    for (i = 0; i < MAX_NUM_INSTANCE; i++) {
        if ((CodecInst *)vip->codecInstPool[i] == pCodecInst)
            return RETCODE_SUCCESS;
    }

    return RETCODE_INVALID_HANDLE;
}

static RetCode CheckDecInstanceValidity(CodecInst* pCodecInst)
{
    RetCode ret;

    if (pCodecInst == NULL)
        return RETCODE_INVALID_HANDLE;

    ret = CheckInstanceValidity(pCodecInst);
    if (ret != RETCODE_SUCCESS) {
        return RETCODE_INVALID_HANDLE;
    }
    if (!pCodecInst->inUse) {
        return RETCODE_INVALID_HANDLE;
    }

    return ProductVpuDecCheckCapability(pCodecInst);
}

Int32 VPU_IsBusy(Uint32 coreIdx)
{
    Uint32 ret = 0;

    INVALID_CORE_INDEX_RETURN_ERROR(coreIdx);

    SetClockGate(coreIdx, 1);
    ret = ProductVpuIsBusy(coreIdx);
    SetClockGate(coreIdx, 0);

    return ret != 0;
}

Int32 VPU_IsInit(Uint32 coreIdx)
{
    Int32 pc;

    INVALID_CORE_INDEX_RETURN_ERROR(coreIdx);

    SetClockGate(coreIdx, 1);
    pc = ProductVpuIsInit(coreIdx);
    SetClockGate(coreIdx, 0);

    return pc;
}

Int32 VPU_WaitInterrupt(Uint32 coreIdx, int timeout)
{
    Int32 ret;
    CodecInst* instance;

    INVALID_CORE_INDEX_RETURN_ERROR(coreIdx);

    if ((instance=GetPendingInst(coreIdx)) != NULL) {
        ret = ProductVpuWaitInterrupt(instance, timeout);
    }
    else {
        ret = -1;
    }

    return ret;
}

Int32 VPU_WaitInterruptEx(VpuHandle handle, int timeout)
{
    Int32 ret;
    CodecInst *pCodecInst;

    pCodecInst  = handle;

    INVALID_CORE_INDEX_RETURN_ERROR(pCodecInst->coreIdx);

    ret = ProductVpuWaitInterrupt(pCodecInst, timeout);

    return ret;
}

void VPU_ClearInterrupt(Uint32 coreIdx)
{
    /* clear all interrupt flags */
    ProductVpuClearInterrupt(coreIdx, 0xffff);
}

void VPU_ClearInterruptEx(VpuHandle handle, Int32 intrFlag)
{
    CodecInst *pCodecInst;

    pCodecInst  = handle;

    ProductVpuClearInterrupt(pCodecInst->coreIdx, intrFlag);
}

int VPU_GetMvColBufSize(CodStd codStd, int width, int height, int num)
{
    int size_mvcolbuf = ProductCalculateAuxBufferSize(AUX_BUF_TYPE_MVCOL, codStd, width, height);

    if (codStd == STD_AVC || codStd == STD_HEVC || codStd == STD_VP9 )
        size_mvcolbuf *= num;

    return size_mvcolbuf;
}

RetCode VPU_GetFBCOffsetTableSize(CodStd codStd, int width, int height, int* ysize, int* csize)
{
    if (ysize == NULL || csize == NULL)
        return RETCODE_INVALID_PARAM;

    *ysize = ProductCalculateAuxBufferSize(AUX_BUF_TYPE_FBC_Y_OFFSET, codStd, width, height);
    *csize = ProductCalculateAuxBufferSize(AUX_BUF_TYPE_FBC_C_OFFSET, codStd, width, height);

    return RETCODE_SUCCESS;
}

int VPU_GetFrameBufSize(int coreIdx, int stride, int height, int mapType, int format, int interleave, DRAMConfig *pDramCfg)
{
    int productId;
    UNREFERENCED_PARAMETER(interleave);             /*!<< for backward compatiblity */

    if (coreIdx < 0 || coreIdx >= MAX_NUM_VPU_CORE)
        return -1;

    productId = ProductVpuGetId(coreIdx);

    return ProductCalculateFrameBufSize(productId, stride, height, (TiledMapType)mapType, (FrameBufferFormat)format, (BOOL)interleave, pDramCfg);
}


int VPU_GetProductId(int coreIdx)
{
    Int32 productId;

    INVALID_CORE_INDEX_RETURN_ERROR(coreIdx);

    if ((productId=ProductVpuGetId(coreIdx)) != PRODUCT_ID_NONE) {
        return productId;
    }

    if (vdi_init(coreIdx) < 0)
        return -1;

    EnterLock(coreIdx);
    if (ProductVpuScan(coreIdx) == FALSE) 
        productId = -1;
    else 
        productId = ProductVpuGetId(coreIdx);
    LeaveLock((coreIdx));

    vdi_release(coreIdx);

    return productId;
}

int VPU_GetOpenInstanceNum(Uint32 coreIdx)
{
    INVALID_CORE_INDEX_RETURN_ERROR(coreIdx);

    return vdi_get_instance_num(coreIdx);
}

static RetCode InitializeVPU(Uint32 coreIdx, const Uint16* code, Uint32 size)
{
    RetCode     ret;

    if (vdi_init(coreIdx) < 0)
        return RETCODE_FAILURE;

    EnterLock(coreIdx);    

    if (vdi_get_instance_num(coreIdx) > 0) {
        if (ProductVpuScan(coreIdx) == 0) {
            LeaveLock(coreIdx);
            return RETCODE_NOT_FOUND_VPU_DEVICE;
        }
    }

    if (VPU_IsInit(coreIdx) != 0) {
        SetClockGate(coreIdx, 1);
        ProductVpuReInit(coreIdx, (void *)code, size);
        LeaveLock(coreIdx);
        return RETCODE_CALLED_BEFORE;
    }

    InitCodecInstancePool(coreIdx);

    SetClockGate(coreIdx, 1);
    ret = ProductVpuReset(coreIdx, SW_RESET_ON_BOOT);
    if (ret != RETCODE_SUCCESS) {
        LeaveLock(coreIdx);
        return ret;
    }

    ret = ProductVpuInit(coreIdx, (void*)code, size);
    if (ret != RETCODE_SUCCESS) {
        LeaveLock(coreIdx);
        return ret;
    }

    LeaveLock(coreIdx);
    return RETCODE_SUCCESS;
}

RetCode VPU_Init(Uint32 coreIdx)
{
#ifdef BIT_CODE_FILE_PATH
    s_pusBitCode[coreIdx]  = bit_code;
    s_bitCodeSize[coreIdx] = sizeof(bit_code)/sizeof(bit_code[0]);
#endif

    if (coreIdx >= MAX_NUM_VPU_CORE)
        return RETCODE_INVALID_PARAM;

    if (s_bitCodeSize[coreIdx] == 0)
        return RETCODE_NOT_FOUND_BITCODE_PATH;

    return InitializeVPU(coreIdx, s_pusBitCode[coreIdx], s_bitCodeSize[coreIdx]);
}

RetCode VPU_InitWithBitcode(Uint32 coreIdx, const Uint16* code, Uint32 size)
{
    if (coreIdx >= MAX_NUM_VPU_CORE)
        return RETCODE_INVALID_PARAM;
    if (code == NULL || size == 0)
        return RETCODE_INVALID_PARAM;

    s_pusBitCode[coreIdx] = NULL;
    s_pusBitCode[coreIdx] = (Uint16 *)osal_malloc(size*sizeof(Uint16));
    if (!s_pusBitCode[coreIdx])
        return RETCODE_INSUFFICIENT_RESOURCE;
    osal_memcpy((void *)s_pusBitCode[coreIdx], (const void *)code, size*sizeof(Uint16));
    s_bitCodeSize[coreIdx] = size;

    return InitializeVPU(coreIdx, code, size);
}

RetCode VPU_DeInit(Uint32 coreIdx)
{
    int ret;

    if (coreIdx >= MAX_NUM_VPU_CORE)
        return RETCODE_INVALID_PARAM;

#ifdef BIT_CODE_FILE_PATH
#else
    if (s_pusBitCode[coreIdx]) {
        osal_free((void *)s_pusBitCode[coreIdx]);
    }
#endif

    s_pusBitCode[coreIdx] = NULL;
    s_bitCodeSize[coreIdx] = 0;

    ret = vdi_release(coreIdx);
    if (ret != 0)
        return RETCODE_FAILURE;

    return RETCODE_SUCCESS;
}

RetCode VPU_GetVersionInfo(Uint32 coreIdx, Uint32 *versionInfo, Uint32 *revision, Uint32 *productId)
{
    RetCode  ret;

    if (coreIdx >= MAX_NUM_VPU_CORE)
        return RETCODE_INVALID_PARAM;

    EnterLock(coreIdx);

    if (ProductVpuIsInit(coreIdx) == 0) {
        LeaveLock(coreIdx);
        return RETCODE_NOT_INITIALIZED;
    }

    if (GetPendingInst(coreIdx)) {
        LeaveLock(coreIdx);
        return RETCODE_FRAME_NOT_COMPLETE;
    }

    if (productId != NULL) {
        *productId = ProductVpuGetId(coreIdx);
    }
    ret = ProductVpuGetVersion(coreIdx, versionInfo, revision);

    LeaveLock(coreIdx);

    return ret;
}
RetCode VPU_DecOpen(DecHandle* pHandle, DecOpenParam* pop)
{
    CodecInst * pCodecInst;
    DecInfo*    pDecInfo;
    RetCode     ret;

    ret = ProductCheckDecOpenParam(pop);
    if (ret != RETCODE_SUCCESS) 
        return ret;

    EnterLock(pop->coreIdx);

    if (VPU_IsInit(pop->coreIdx) == 0) {
        LeaveLock(pop->coreIdx);
        return RETCODE_NOT_INITIALIZED;      
    }

    ret = GetCodecInstance(pop->coreIdx, &pCodecInst);
    if (ret != RETCODE_SUCCESS) 
    {
        *pHandle = 0;
        LeaveLock(pop->coreIdx);
        return ret;
    }

    pCodecInst->isDecoder = TRUE;
    *pHandle = pCodecInst;
    pDecInfo = &pCodecInst->CodecInfo->decInfo;
    osal_memset(pDecInfo, 0x00, sizeof(DecInfo));
    osal_memcpy((void*)&pDecInfo->openParam, pop, sizeof(DecOpenParam));

    if (pop->bitstreamFormat == STD_MPEG4) 
    {
        pCodecInst->codecMode    = (pCodecInst->productId == PRODUCT_ID_7Q ? C7_MP4_DEC : MP4_DEC);
        pCodecInst->codecModeAux = MP4_AUX_MPEG4;
    }
    else if (pop->bitstreamFormat == STD_AVC) 
    {
        pCodecInst->codecMode    = (pCodecInst->productId == PRODUCT_ID_7Q ? C7_AVC_DEC : AVC_DEC);
        pCodecInst->codecModeAux = pop->avcExtension;
    } 
    else if (pop->bitstreamFormat == STD_VC1) 
    {
        pCodecInst->codecMode = (pCodecInst->productId == PRODUCT_ID_7Q ? C7_VC1_DEC : VC1_DEC);
    }
    else if (pop->bitstreamFormat == STD_MPEG2) 
    {
        pCodecInst->codecMode = (pCodecInst->productId == PRODUCT_ID_7Q ? C7_MP2_DEC : MP2_DEC);
    }
    else if (pop->bitstreamFormat == STD_H263) 
    {
        pCodecInst->codecMode    = (pCodecInst->productId == PRODUCT_ID_7Q ? C7_MP4_DEC : MP4_DEC);
        pCodecInst->codecModeAux = MP4_AUX_MPEG4;
    }
    else if (pop->bitstreamFormat == STD_DIV3) 
    {
        pCodecInst->codecMode    = (pCodecInst->productId == PRODUCT_ID_7Q ? C7_DV3_DEC : DV3_DEC);
        pCodecInst->codecModeAux = MP4_AUX_DIVX3;
        pDecInfo->div3Width      = pop->div3Width;
        pDecInfo->div3Height     = pop->div3Height;
    }    
    else if (pop->bitstreamFormat == STD_RV) 
    {
        pCodecInst->codecMode = (pCodecInst->productId == PRODUCT_ID_7Q ? C7_RVX_DEC : RV_DEC);
    }
    else if (pop->bitstreamFormat == STD_AVS) 
    {
        pCodecInst->codecMode = (pCodecInst->productId == PRODUCT_ID_7Q ? C7_AVS_DEC : AVS_DEC);
    }
    else if (pop->bitstreamFormat == STD_THO) 
    {
        pCodecInst->codecMode    = VPX_DEC;
        pCodecInst->codecModeAux = VPX_AUX_THO;
    } 
    else if (pop->bitstreamFormat == STD_VP3) 
    {
        pCodecInst->codecMode    = VPX_DEC;
        pCodecInst->codecModeAux = VPX_AUX_THO;
    } 
    else if (pop->bitstreamFormat == STD_VP8) 
    {
        pCodecInst->codecMode    = (pCodecInst->productId == PRODUCT_ID_7Q ? C7_VP8_DEC : VPX_DEC);
        pCodecInst->codecModeAux = VPX_AUX_VP8;
    }
    else if (pop->bitstreamFormat == STD_HEVC) 
    {
        pCodecInst->codecMode    = HEVC_DEC;
    }
    else if (pop->bitstreamFormat == STD_VP9) 
    {
        pCodecInst->codecMode    = C7_VP9_DEC;
    }
    else if (pop->bitstreamFormat == STD_AVS2) 
    {
        pCodecInst->codecMode    = C7_AVS2_DEC;
    }
    else
    {
        LeaveLock(pop->coreIdx);
        return RETCODE_INVALID_PARAM;
    }

    pDecInfo->wtlEnable = pop->wtlEnable;
    pDecInfo->wtlMode   = pop->wtlMode;
    if (!pDecInfo->wtlEnable)
        pDecInfo->wtlMode = 0;

    pDecInfo->streamWrPtr        = pop->bitstreamBuffer;    
    pDecInfo->streamRdPtr        = pop->bitstreamBuffer;
    pDecInfo->frameDelay         = -1;
    pDecInfo->streamBufStartAddr = pop->bitstreamBuffer;
    pDecInfo->streamBufSize      = pop->bitstreamBufferSize;
    pDecInfo->streamBufEndAddr   = pop->bitstreamBuffer + pop->bitstreamBufferSize;    
    pDecInfo->reorderEnable      = VPU_REORDER_ENABLE;
    pDecInfo->mirrorDirection    = MIRDIR_NONE;
    pDecInfo->prevFrameEndPos    = pop->bitstreamBuffer;

    SetClockGate(pop->coreIdx, TRUE);
    if ((ret=ProductVpuDecBuildUpOpenParam(pCodecInst, pop)) != RETCODE_SUCCESS) {
        SetClockGate(pop->coreIdx, FALSE);
        *pHandle = 0;
        LeaveLock(pCodecInst->coreIdx);
        return ret;
    }
    SetClockGate(pop->coreIdx, FALSE);

    pDecInfo->tiled2LinearEnable = pop->tiled2LinearEnable;
    pDecInfo->tiled2LinearMode   = pop->tiled2LinearMode;
    if (!pDecInfo->tiled2LinearEnable)
        pDecInfo->tiled2LinearMode = 0;     //coda980 only

    if (!pDecInfo->wtlEnable)               //coda980, wave320, wave410 only
        pDecInfo->wtlMode = 0;

    osal_memset((void*)&pDecInfo->cacheConfig, 0x00, sizeof(MaverickCacheConfig));

    LeaveLock(pCodecInst->coreIdx);

    return RETCODE_SUCCESS;
}

RetCode VPU_DecClose(DecHandle handle)
{
    CodecInst * pCodecInst;
    DecInfo * pDecInfo;
    RetCode ret;
    int i;

    ret = CheckDecInstanceValidity(handle);
    if (ret != RETCODE_SUCCESS)
        return ret;

    pCodecInst = handle;
    pDecInfo   = &pCodecInst->CodecInfo->decInfo;

    EnterLock(pCodecInst->coreIdx);

    if ((ret=ProductVpuDecFiniSeq(pCodecInst)) != RETCODE_SUCCESS) {
        if (pCodecInst->loggingEnable)
            vdi_log(pCodecInst->coreIdx, DEC_SEQ_END, 0);

        if (ret == RETCODE_VPU_STILL_RUNNING) {
            LeaveLock(pCodecInst->coreIdx);
            return ret;
        }
    }

    if (pDecInfo->vbSlice.size)
        vdi_free_dma_memory(pCodecInst->coreIdx, &pDecInfo->vbSlice);

    if (pDecInfo->vbWork.size) {
        if (pDecInfo->workBufferAllocExt == 0) {
            vdi_free_dma_memory(pCodecInst->coreIdx, &pDecInfo->vbWork);
        }
        else {
            vdi_dettach_dma_memory(pCodecInst->coreIdx, &pDecInfo->vbWork);
        }
    }

    if (pDecInfo->vbFrame.size) {
        if (pDecInfo->frameAllocExt == 0) {
            vdi_free_dma_memory(pCodecInst->coreIdx, &pDecInfo->vbFrame);
        }
    }    
    for ( i=0 ; i<MAX_REG_FRAME; i++) {
        if (pDecInfo->vbMV[i].size) 
            vdi_free_dma_memory(pCodecInst->coreIdx, &pDecInfo->vbMV[i]);
        if (pDecInfo->vbFbcYTbl[i].size)
            vdi_free_dma_memory(pCodecInst->coreIdx, &pDecInfo->vbFbcYTbl[i]);
        if (pDecInfo->vbFbcCTbl[i].size)
            vdi_free_dma_memory(pCodecInst->coreIdx, &pDecInfo->vbFbcCTbl[i]);
    }

    if (pDecInfo->vbTemp.size) 
        vdi_dettach_dma_memory(pCodecInst->coreIdx, &pDecInfo->vbTemp);

    if (pDecInfo->vbPPU.size) {
        if (pDecInfo->ppuAllocExt == 0) 
            vdi_free_dma_memory(pCodecInst->coreIdx, &pDecInfo->vbPPU);
    }

    if (pDecInfo->vbWTL.size)
        vdi_free_dma_memory(pCodecInst->coreIdx, &pDecInfo->vbWTL);

    if (pDecInfo->vbUserData.size) 
        vdi_dettach_dma_memory(pCodecInst->coreIdx, &pDecInfo->vbUserData);

    if (pDecInfo->vbReport.size) 
        vdi_free_dma_memory(pCodecInst->coreIdx, &pDecInfo->vbReport);

    if (GetPendingInst(pCodecInst->coreIdx) == pCodecInst) 
        ClearPendingInst(pCodecInst->coreIdx);

    LeaveLock(pCodecInst->coreIdx);

    FreeCodecInstance(pCodecInst);

    return ret;
}

RetCode VPU_DecSetEscSeqInit(DecHandle handle, int escape)
{
    CodecInst * pCodecInst;
    DecInfo * pDecInfo;
    RetCode ret;

    ret = CheckDecInstanceValidity(handle);
    if (ret != RETCODE_SUCCESS)
        return ret;

    pCodecInst = handle;
    pDecInfo   = &pCodecInst->CodecInfo->decInfo;

    if (pDecInfo->openParam.bitstreamMode != BS_MODE_INTERRUPT)
        return RETCODE_INVALID_PARAM;

    pDecInfo->seqInitEscape = escape;

    return RETCODE_SUCCESS;
}

RetCode VPU_DecGetInitialInfo(DecHandle handle, DecInitialInfo* info)
{
    CodecInst*  pCodecInst;
    DecInfo*    pDecInfo;
    RetCode     ret;
    Int32       flags;
    Uint32      interruptBit;
    VpuAttr*    pAttr;

    if (PRODUCT_ID_W_SERIES(handle->productId)) {
        if (handle->productId == PRODUCT_ID_510 || handle->productId == PRODUCT_ID_512) {
            interruptBit = INT_WAVE5_INIT_SEQ;
        }
        else {
            interruptBit = INT_WAVE_DEC_PIC_HDR;
        }
    }
    else {
        /* CODA9xx */
        interruptBit = INT_BIT_SEQ_INIT;
    }

    ret = CheckDecInstanceValidity(handle);
    if (ret != RETCODE_SUCCESS)
        return ret;

    if (info == NULL) 
        return RETCODE_INVALID_PARAM;

    pCodecInst = handle;
    pDecInfo   = &pCodecInst->CodecInfo->decInfo;
    ret = ProductVpuDecCheckCapability(pCodecInst);
    if (ret != RETCODE_SUCCESS)
        return ret;

    EnterLock(pCodecInst->coreIdx);

    pAttr = &g_VpuCoreAttributes[pCodecInst->coreIdx];

    if (GetPendingInst(pCodecInst->coreIdx)) {
        /* The other instance is running */
        LeaveLock(pCodecInst->coreIdx);
        return RETCODE_FRAME_NOT_COMPLETE;
    }

    if (DecBitstreamBufEmpty(pDecInfo)) {
        LeaveLock(pCodecInst->coreIdx);
        return RETCODE_WRONG_CALL_SEQUENCE;
    }

    ret = ProductVpuDecInitSeq(handle);
    if (ret != RETCODE_SUCCESS) {
        LeaveLock(pCodecInst->coreIdx);
        return ret;
    } 
    
    if (pAttr->supportCommandQueue == TRUE) {
        if (pDecInfo->openParam.bitstreamFormat == STD_HEVC) {
            LeaveLock(pCodecInst->coreIdx);
        }
    }

    flags = ProductVpuWaitInterrupt(pCodecInst, __VPU_BUSY_TIMEOUT); 

    if (pAttr->supportCommandQueue == TRUE) {
        if (pDecInfo->openParam.bitstreamFormat == STD_HEVC) {
            EnterLock(pCodecInst->coreIdx);
        }
    }

    if (flags == -1) {
        info->rdPtr = VpuReadReg(pCodecInst->coreIdx, pDecInfo->streamRdPtrRegAddr);
        info->wrPtr = VpuReadReg(pCodecInst->coreIdx, pDecInfo->streamWrPtrRegAddr);
        ret = RETCODE_VPU_RESPONSE_TIMEOUT;
    } 
    else {
        if (flags&(1<<interruptBit)) 
            ProductVpuClearInterrupt(pCodecInst->coreIdx, (1<<interruptBit));

        if (flags != (1<<interruptBit))
            ret = RETCODE_FAILURE;
        else 
            ret = ProductVpuDecGetSeqInfo(handle, info);
    }

    info->rdPtr = VpuReadReg(pCodecInst->coreIdx, pDecInfo->streamRdPtrRegAddr);
    info->wrPtr = VpuReadReg(pCodecInst->coreIdx, pDecInfo->streamWrPtrRegAddr);

    pDecInfo->initialInfo = *info;
    if (ret == RETCODE_SUCCESS) {
        pDecInfo->initialInfoObtained = 1;
    }

    SetPendingInst(pCodecInst->coreIdx, 0);

    LeaveLock(pCodecInst->coreIdx);

    return ret;
}

RetCode VPU_DecIssueSeqInit(DecHandle handle)
{
    CodecInst*  pCodecInst;
    RetCode     ret;
    VpuAttr*    pAttr;

    ret = CheckDecInstanceValidity(handle);
    if (ret != RETCODE_SUCCESS)
        return ret;

    pCodecInst = handle;
   
    EnterLock(pCodecInst->coreIdx);

    pAttr = &g_VpuCoreAttributes[pCodecInst->coreIdx];

    if (GetPendingInst(pCodecInst->coreIdx)) {
        LeaveLock(pCodecInst->coreIdx);
        return RETCODE_FRAME_NOT_COMPLETE;
    }

    ret = ProductVpuDecInitSeq(handle);
    if (ret == RETCODE_SUCCESS) {
        SetPendingInst(pCodecInst->coreIdx, pCodecInst);
    }
    
    if (pAttr->supportCommandQueue == TRUE) {
        SetPendingInst(pCodecInst->coreIdx, NULL);
        LeaveLock(pCodecInst->coreIdx);
    }

    return ret;
}

RetCode VPU_DecCompleteSeqInit(DecHandle handle, DecInitialInfo * info)
{
    CodecInst*  pCodecInst;
    DecInfo*    pDecInfo;
    RetCode     ret;
    VpuAttr*    pAttr;

    ret = CheckDecInstanceValidity(handle);
    if (ret != RETCODE_SUCCESS) {
        return ret;
    }

    if (info == 0) {
        return RETCODE_INVALID_PARAM;
    }

    pCodecInst = handle;
    pDecInfo = &pCodecInst->CodecInfo->decInfo;

    pAttr = &g_VpuCoreAttributes[pCodecInst->coreIdx];
    
    if (pAttr->supportCommandQueue == TRUE) {
        EnterLock(pCodecInst->coreIdx);
    }
    else {
        if (pCodecInst != GetPendingInst(pCodecInst->coreIdx)) {
            SetPendingInst(pCodecInst->coreIdx, 0);
            LeaveLock(pCodecInst->coreIdx);    
            return RETCODE_WRONG_CALL_SEQUENCE;
        }
    }

    ret = ProductVpuDecGetSeqInfo(handle, info);
    if (ret == RETCODE_SUCCESS) {
        pDecInfo->initialInfoObtained = 1;
    }

    info->rdPtr = VpuReadReg(pCodecInst->coreIdx, pDecInfo->streamRdPtrRegAddr);
    info->wrPtr = VpuReadReg(pCodecInst->coreIdx, pDecInfo->streamWrPtrRegAddr);

    pDecInfo->prevFrameEndPos = info->rdPtr;

    pDecInfo->initialInfo = *info;

    SetPendingInst(pCodecInst->coreIdx, NULL);

    LeaveLock(pCodecInst->coreIdx);

    return ret;
}

static RetCode DecRegisterFrameBuffer(DecHandle handle, FrameBuffer *bufArray, int numFbsForDecoding, int numFbsForWTL, int stride, int height, int mapType)
{
    CodecInst*      pCodecInst;
    DecInfo*        pDecInfo;
    Int32           i;
    Uint32          size, totalAllocSize;
    RetCode         ret;
    FrameBuffer*    fb, nullFb;
    vpu_buffer_t*   vb;
    FrameBufferFormat format = FORMAT_420;
    Int32          totalNumOfFbs;

    ret = CheckDecInstanceValidity(handle);
    if (ret != RETCODE_SUCCESS)
        return ret;

    if (numFbsForDecoding > MAX_FRAMEBUFFER_COUNT || numFbsForWTL > MAX_FRAMEBUFFER_COUNT) {
        return RETCODE_INVALID_PARAM;
    }

    osal_memset(&nullFb, 0x00, sizeof(FrameBuffer));
    pCodecInst                  = handle;
    pDecInfo                    = &pCodecInst->CodecInfo->decInfo;
    pDecInfo->numFbsForDecoding = numFbsForDecoding;
    pDecInfo->numFbsForWTL      = numFbsForWTL;
    pDecInfo->numFrameBuffers   = numFbsForDecoding + numFbsForWTL;
    pDecInfo->stride            = stride;
    if (pCodecInst->codecMode == VPX_DEC || pCodecInst->codecMode == C7_VP8_DEC)
        pDecInfo->frameBufferHeight = VPU_ALIGN64(height);
    else if (pCodecInst->codecMode == C7_VP9_DEC)
        pDecInfo->frameBufferHeight = VPU_ALIGN64(height);
    else
        pDecInfo->frameBufferHeight = height;
    pDecInfo->mapType           = mapType;
    pDecInfo->mapCfg.productId  = pCodecInst->productId;

    ret = ProductVpuDecCheckCapability(pCodecInst);
    if (ret != RETCODE_SUCCESS)
        return ret;

    if (!pDecInfo->initialInfoObtained) 
        return RETCODE_WRONG_CALL_SEQUENCE;

    if ( (stride < pDecInfo->initialInfo.picWidth) || (stride % 8 != 0) || (height<pDecInfo->initialInfo.picHeight) ) {
        return RETCODE_INVALID_STRIDE;
    }

    EnterLock(pCodecInst->coreIdx);
    if (GetPendingInst(pCodecInst->coreIdx)) {
        LeaveLock(pCodecInst->coreIdx);
        return RETCODE_FRAME_NOT_COMPLETE;
    }

    /* clear frameBufPool */
    for (i=0; i<(int)(sizeof(pDecInfo->frameBufPool)/sizeof(FrameBuffer)); i++) {
        pDecInfo->frameBufPool[i] = nullFb;
    }

    /* LinearMap or TiledMap, compressed framebuffer inclusive. */
    if (pDecInfo->initialInfo.lumaBitdepth > 8 || pDecInfo->initialInfo.chromaBitdepth > 8) 
        format = FORMAT_420_P10_16BIT_LSB;

    totalNumOfFbs = numFbsForDecoding + numFbsForWTL;
    if (bufArray) {
        for(i=0; i<totalNumOfFbs; i++)
            pDecInfo->frameBufPool[i] = bufArray[i];
    } 
    else {
        vb = &pDecInfo->vbFrame;
        fb = &pDecInfo->frameBufPool[0];
        ret = ProductVpuAllocateFramebuffer(
            (CodecInst*)handle, fb, (TiledMapType)mapType, numFbsForDecoding, stride, height, format,
            pDecInfo->openParam.cbcrInterleave, 
            pDecInfo->openParam.nv21,
            pDecInfo->openParam.frameEndian, vb, 0, FB_TYPE_CODEC);
        if (ret != RETCODE_SUCCESS) {
            LeaveLock(pCodecInst->coreIdx);
            return ret;
        }
    }
    totalAllocSize = 0;
    if (pCodecInst->productId != PRODUCT_ID_960) {
        pDecInfo->mapCfg.tiledBaseAddr = pDecInfo->frameBufPool[0].bufY;
    }

    if (numFbsForDecoding == 1) {
        size = ProductCalculateFrameBufSize(handle->productId, stride, height, (TiledMapType)mapType, format, 
            pDecInfo->openParam.cbcrInterleave, &pDecInfo->dramCfg);
    }
    else {
        size = pDecInfo->frameBufPool[1].bufY - pDecInfo->frameBufPool[0].bufY;
    }
    size *= numFbsForDecoding;
    totalAllocSize += size;

    /* LinearMap */
    if (pDecInfo->wtlEnable == TRUE || numFbsForWTL != 0) {
        pDecInfo->stride = stride;
        if (bufArray) {
            format = pDecInfo->frameBufPool[0].format;
        } 
        else {
            TiledMapType map;
            map = pDecInfo->wtlMode==FF_FRAME ? LINEAR_FRAME_MAP : LINEAR_FIELD_MAP;
            format = pDecInfo->wtlFormat;
            vb = &pDecInfo->vbWTL;
            fb = &pDecInfo->frameBufPool[numFbsForDecoding];

            ret = ProductVpuAllocateFramebuffer(
                (CodecInst*)handle, fb, map, numFbsForWTL, stride, height,
                pDecInfo->wtlFormat,
                pDecInfo->openParam.cbcrInterleave, 
                pDecInfo->openParam.nv21,
                pDecInfo->openParam.frameEndian, vb, 0, FB_TYPE_PPU);

            if (ret != RETCODE_SUCCESS) {
                LeaveLock(pCodecInst->coreIdx);
                return ret;
            }
        }
        if (numFbsForWTL == 1) {
            size = ProductCalculateFrameBufSize(handle->productId, stride, height, (TiledMapType)mapType, format, 
                pDecInfo->openParam.cbcrInterleave, &pDecInfo->dramCfg);
        }
        else {
            size = pDecInfo->frameBufPool[numFbsForDecoding+1].bufY - 
                pDecInfo->frameBufPool[numFbsForDecoding].bufY;
        }
        size *= numFbsForWTL;
        totalAllocSize += size;
    }

    ret = ProductVpuRegisterFramebuffer(pCodecInst);

    LeaveLock(pCodecInst->coreIdx);


    return ret;
}

RetCode VPU_DecRegisterFrameBuffer(DecHandle handle, FrameBuffer *bufArray, int num, int stride, int height, int mapType)
{
    DecInfo* pDecInfo  = &handle->CodecInfo->decInfo;
    Uint32   numWTL    = 0;

    if (pDecInfo->wtlEnable == TRUE) numWTL = num;
    return DecRegisterFrameBuffer(handle, bufArray, num, numWTL, stride, height, mapType);
}

RetCode VPU_DecRegisterFrameBufferEx(DecHandle handle, FrameBuffer *bufArray, int numOfDecFbs, int numOfDisplayFbs, int stride, int height, int mapType)
{
    return DecRegisterFrameBuffer(handle, bufArray, numOfDecFbs, numOfDisplayFbs, stride, height, mapType);
}

RetCode VPU_DecGetFrameBuffer(DecHandle handle, int frameIdx, FrameBuffer* frameBuf)
{
    CodecInst * pCodecInst;
    DecInfo * pDecInfo;
    RetCode ret;

    ret = CheckDecInstanceValidity(handle);
    if (ret != RETCODE_SUCCESS)
        return ret;

    if (frameBuf == 0)
        return RETCODE_INVALID_PARAM;

    pCodecInst = handle;
    pDecInfo   = &pCodecInst->CodecInfo->decInfo;

    if (frameIdx < 0 || frameIdx >= pDecInfo->numFrameBuffers)
        return RETCODE_INVALID_PARAM;

    *frameBuf = pDecInfo->frameBufPool[frameIdx];

    return RETCODE_SUCCESS;
}

RetCode VPU_DecUpdateFrameBuffer(
    DecHandle     handle,
    FrameBuffer*  fbcFb,
    FrameBuffer*  linearFb,
    Int32         mvColIndex,
    Int32         picWidth,
    Int32         picHeight
    )
{
    if (handle == NULL) { 
        return RETCODE_INVALID_HANDLE;
    }

    return ProductVpuDecUpdateFrameBuffer((CodecInst*)handle, fbcFb, linearFb, mvColIndex, picWidth, picHeight);
}

RetCode VPU_DecSetBitstreamBuffer(DecHandle handle, PhysicalAddress rdPtr, PhysicalAddress wrPtr)
{
    CodecInst * pCodecInst;
    DecInfo * pDecInfo;
    RetCode ret;

    ret = CheckDecInstanceValidity(handle);
    if (ret != RETCODE_SUCCESS)
        return ret;

    pCodecInst = handle;
    pDecInfo = &pCodecInst->CodecInfo->decInfo;

    SetClockGate(pCodecInst->coreIdx, 1);

    if (GetPendingInst(pCodecInst->coreIdx) == pCodecInst)
        VpuWriteReg(pCodecInst->coreIdx, pDecInfo->streamRdPtrRegAddr, rdPtr);        
    else
        pDecInfo->streamRdPtr = rdPtr;

    SetClockGate(pCodecInst->coreIdx, 0);

    pDecInfo->streamWrPtr = wrPtr;

    return RETCODE_SUCCESS;
}

RetCode VPU_DecGetBitstreamBuffer(DecHandle handle, PhysicalAddress* prdPtr, PhysicalAddress* pwrPtr, Uint32* size)
{
    CodecInst*      pCodecInst;
    DecInfo*        pDecInfo;
    PhysicalAddress rdPtr;
    PhysicalAddress wrPtr;
    PhysicalAddress tempPtr;
    int             room;
    Int32           coreIdx;
    VpuAttr*        pAttr;

    coreIdx   = handle->coreIdx;

    pCodecInst = handle;
    pDecInfo = &pCodecInst->CodecInfo->decInfo;

    SetClockGate(coreIdx, TRUE);

    if (GetPendingInst(coreIdx) == pCodecInst) {
        if (pCodecInst->codecMode == AVC_DEC && pCodecInst->codecModeAux == AVC_AUX_MVC) {
            rdPtr = pDecInfo->streamRdPtr;
        }
        else {
            rdPtr = VpuReadReg(coreIdx, pDecInfo->streamRdPtrRegAddr);
        }
    }
    else {
        rdPtr = pDecInfo->streamRdPtr;
    }
    
    SetClockGate(coreIdx, FALSE);

    wrPtr = pDecInfo->streamWrPtr;

    pAttr = &g_VpuCoreAttributes[coreIdx];
    if (pCodecInst->productId == PRODUCT_ID_4102 || pCodecInst->productId == PRODUCT_ID_420 || pCodecInst->productId == PRODUCT_ID_412 || pCodecInst->productId == PRODUCT_ID_420L ||
        (pCodecInst->productId == PRODUCT_ID_7Q && pCodecInst->codecMode == C7_HEVC_DEC)) {
        if (pDecInfo->openParam.bitstreamMode != BS_MODE_PIC_END) {
            tempPtr = pDecInfo->prevFrameEndPos;
        }
        else {
            tempPtr = rdPtr;
        }
    }
    else {
        tempPtr = rdPtr;
    }

    if (pDecInfo->openParam.bitstreamMode != BS_MODE_PIC_END) {
		if (wrPtr < tempPtr) {
			room = tempPtr - wrPtr - pAttr->bitstreamBufferMargin*2;    
		}
		else {
			room = (pDecInfo->streamBufEndAddr - wrPtr) + (tempPtr - pDecInfo->streamBufStartAddr) - pAttr->bitstreamBufferMargin*2;
		}
		room--;
	}
	else {
		room = (pDecInfo->streamBufEndAddr - wrPtr);
	}

    if (prdPtr) *prdPtr = tempPtr;
    if (pwrPtr) *pwrPtr = wrPtr;
    if (size)   *size   = room;

    return RETCODE_SUCCESS;
}

RetCode VPU_DecUpdateBitstreamBuffer(DecHandle handle, int size)
{
    CodecInst*      pCodecInst;
    DecInfo*        pDecInfo;
    PhysicalAddress wrPtr;
    PhysicalAddress rdPtr;
    RetCode         ret;
    BOOL            running;

    ret = CheckDecInstanceValidity(handle);
    if (ret != RETCODE_SUCCESS)
        return ret;

    pCodecInst = handle;
    pDecInfo   = &pCodecInst->CodecInfo->decInfo;
    wrPtr      = pDecInfo->streamWrPtr;

    SetClockGate(pCodecInst->coreIdx, 1);

    running = (BOOL)(GetPendingInst(pCodecInst->coreIdx) == pCodecInst);

    if (size > 0) {
        Uint32      room    = 0;

        if (running == TRUE)
            rdPtr = VpuReadReg(pCodecInst->coreIdx, pDecInfo->streamRdPtrRegAddr);
        else
            rdPtr = pDecInfo->streamRdPtr;

        if (wrPtr < rdPtr) {
            if (rdPtr <= wrPtr + size) {
                SetClockGate(pCodecInst->coreIdx, 0);
                return RETCODE_INVALID_PARAM;
            }
        }

        wrPtr += size;

        if (wrPtr > pDecInfo->streamBufEndAddr) {
            room = wrPtr - pDecInfo->streamBufEndAddr;
            wrPtr = pDecInfo->streamBufStartAddr;
            wrPtr += room;
        } 
        else if (wrPtr == pDecInfo->streamBufEndAddr) {
            wrPtr = pDecInfo->streamBufStartAddr;
        }

        pDecInfo->streamWrPtr = wrPtr;
        pDecInfo->streamRdPtr = rdPtr;

        if (running == TRUE) {
            VpuAttr*    pAttr   = &g_VpuCoreAttributes[pCodecInst->coreIdx];
            if (pAttr->supportCommandQueue == FALSE) {
                VpuWriteReg(pCodecInst->coreIdx, pDecInfo->streamWrPtrRegAddr, wrPtr);
            }
        }
    }

    ret = ProductVpuDecSetBitstreamFlag(pCodecInst, running, size);

    SetClockGate(pCodecInst->coreIdx, 0);
    return ret;
}

RetCode VPU_HWReset(Uint32 coreIdx)
{
    if (vdi_hw_reset(coreIdx) < 0 )
        return RETCODE_FAILURE;

    if (GetPendingInst(coreIdx))
    {
        SetPendingInst(coreIdx, 0);
        LeaveLock(coreIdx);    //if vpu is in a lock state. release the state;        
    }
    return RETCODE_SUCCESS;    
}

/**
* VPU_SWReset
* IN
*    forcedReset : 1 if there is no need to waiting for BUS transaction, 
*                  0 for otherwise
* OUT
*    RetCode : RETCODE_FAILURE if failed to reset,
*              RETCODE_SUCCESS for otherwise
*/
RetCode VPU_SWReset(Uint32 coreIdx, SWResetMode resetMode, void *pendingInst)
{
    CodecInst *pCodecInst = (CodecInst *)pendingInst;
    RetCode    ret = RETCODE_SUCCESS;

    SetClockGate(coreIdx, 1);

    if (pCodecInst) {
        SetPendingInst(pCodecInst->coreIdx, 0);
        LeaveLock(coreIdx);
        SetClockGate(coreIdx, 1);
        if (pCodecInst->loggingEnable) {
            vdi_log(pCodecInst->coreIdx, (pCodecInst->productId == PRODUCT_ID_960 || pCodecInst->productId == PRODUCT_ID_980)?0x10:0x10000, 1);			
        }
    }

    ret = ProductVpuReset(coreIdx, resetMode);

    if (pCodecInst) {
        if (pCodecInst->loggingEnable) {
            vdi_log(pCodecInst->coreIdx, (pCodecInst->productId == PRODUCT_ID_960 || pCodecInst->productId == PRODUCT_ID_980)?0x10:0x10000, 0);			
        }
    }

    SetClockGate(coreIdx, 0);

    return ret;
}

//---- VPU_SLEEP/WAKE
RetCode VPU_SleepWake(Uint32 coreIdx, int iSleepWake)
{
    RetCode ret;

    SetClockGate(coreIdx, TRUE);
    ret = ProductVpuSleepWake(coreIdx, iSleepWake, s_pusBitCode[coreIdx], s_bitCodeSize[coreIdx]);
    SetClockGate(coreIdx, FALSE);

    return ret;
}

RetCode VPU_DecStartOneFrame(DecHandle handle, DecParam *param)
{
    CodecInst*  pCodecInst;
    DecInfo*    pDecInfo;
    Uint32      val     = 0;
    RetCode     ret     = RETCODE_SUCCESS;
    VpuAttr*    pAttr   = NULL;

    ret = CheckDecInstanceValidity(handle);
    if (ret != RETCODE_SUCCESS) {
        return ret;
    }

    pCodecInst = (CodecInst*)handle;
    pDecInfo   = &pCodecInst->CodecInfo->decInfo;

    if (pDecInfo->stride == 0) { // This means frame buffers have not been registered.
        return RETCODE_WRONG_CALL_SEQUENCE;
    }

    pAttr = &g_VpuCoreAttributes[pCodecInst->coreIdx];

    EnterLock(pCodecInst->coreIdx);
    if (GetPendingInst(pCodecInst->coreIdx)) {
        LeaveLock(pCodecInst->coreIdx);
        return RETCODE_FRAME_NOT_COMPLETE;
    }

    if (pAttr->supportCommandQueue == FALSE) {
        EnterDispFlagLock(pCodecInst->coreIdx);
        val  = pDecInfo->frameDisplayFlag;
        val |= pDecInfo->setDisplayIndexes;
        val &= ~(Uint32)(pDecInfo->clearDisplayIndexes);
        VpuWriteReg(pCodecInst->coreIdx, pDecInfo->frameDisplayFlagRegAddr, val);
        pDecInfo->clearDisplayIndexes = 0;
        pDecInfo->setDisplayIndexes = 0;
        LeaveDispFlagLock(pCodecInst->coreIdx);
    }

    pDecInfo->frameStartPos = pDecInfo->streamRdPtr;

    ret = ProductVpuDecode(pCodecInst, param);

    if (pAttr->supportCommandQueue == TRUE) {
        SetPendingInst(pCodecInst->coreIdx, NULL);
        LeaveLock(pCodecInst->coreIdx);
    }
    else {
        SetPendingInst(pCodecInst->coreIdx, pCodecInst);
    }

    return ret;
}

RetCode VPU_DecGetOutputInfo(DecHandle handle, DecOutputInfo* info)
{
    CodecInst*  pCodecInst;
    DecInfo*    pDecInfo;
    RetCode     ret;
    VpuRect     rectInfo;
    Uint32      val;
    Int32       decodedIndex;
    Int32       displayIndex;
    Uint32      maxDecIndex;
    VpuAttr*    pAttr;

    ret = CheckDecInstanceValidity(handle);
    if (ret != RETCODE_SUCCESS) {        
        return ret;
    }

    if (info == 0) {
        return RETCODE_INVALID_PARAM;
    }

    pCodecInst  = handle;
    pDecInfo    = &pCodecInst->CodecInfo->decInfo;
    pAttr       = &g_VpuCoreAttributes[pCodecInst->coreIdx];

    if (pAttr->supportCommandQueue == TRUE) {
        EnterLock(pCodecInst->coreIdx);
    }
    else {
        if (pCodecInst != GetPendingInst(pCodecInst->coreIdx)) {
            SetPendingInst(pCodecInst->coreIdx, 0);
            LeaveLock(pCodecInst->coreIdx);    
            return RETCODE_WRONG_CALL_SEQUENCE;
        }
    }

    osal_memset((void*)info, 0x00, sizeof(DecOutputInfo));

    ret = ProductVpuDecGetResult(pCodecInst, info);
    if (ret != RETCODE_SUCCESS) {
        info->rdPtr = pDecInfo->streamRdPtr;
        info->wrPtr = pDecInfo->streamWrPtr;
        SetPendingInst(pCodecInst->coreIdx, 0);
        LeaveLock(pCodecInst->coreIdx);
        return ret;
    }

    decodedIndex = info->indexFrameDecoded;


    // Calculate display frame region
    val = 0;
    if (decodedIndex >= 0 && decodedIndex < MAX_GDI_IDX) {
        //default value
        rectInfo.left   = 0;
        rectInfo.right  = info->decPicWidth;
        rectInfo.top    = 0;
        rectInfo.bottom = info->decPicHeight;

        if (pCodecInst->codecMode == HEVC_DEC || pCodecInst->codecMode == AVC_DEC || pCodecInst->codecMode == C7_AVC_DEC || pCodecInst->codecMode == AVS_DEC)
            rectInfo = pDecInfo->initialInfo.picCropRect;

        if (pCodecInst->codecMode == HEVC_DEC)
            pDecInfo->decOutInfo[decodedIndex].h265Info.decodedPOC = info->h265Info.decodedPOC;

        info->rcDecoded.left   =  pDecInfo->decOutInfo[decodedIndex].rcDecoded.left   = rectInfo.left;    
        info->rcDecoded.right  =  pDecInfo->decOutInfo[decodedIndex].rcDecoded.right  = rectInfo.right;   
        info->rcDecoded.top    =  pDecInfo->decOutInfo[decodedIndex].rcDecoded.top    = rectInfo.top;     
        info->rcDecoded.bottom =  pDecInfo->decOutInfo[decodedIndex].rcDecoded.bottom = rectInfo.bottom;
    }
    else
    {
        info->rcDecoded.left   = 0;  
        info->rcDecoded.right  = info->decPicWidth;      
        info->rcDecoded.top    = 0;  
        info->rcDecoded.bottom = info->decPicHeight;    
    }

    displayIndex = info->indexFrameDisplay;
    if (info->indexFrameDisplay >= 0 && info->indexFrameDisplay < MAX_GDI_IDX)
    {
        if (pCodecInst->codecMode == VC1_DEC || pCodecInst->codecMode == C7_VC1_DEC) // vc1 rotates decoded frame buffer region. the other std rotated whole frame buffer region.
        {
            if (pDecInfo->rotationEnable && (pDecInfo->rotationAngle==90 || pDecInfo->rotationAngle==270))
            {
                info->rcDisplay.left   = pDecInfo->decOutInfo[displayIndex].rcDecoded.top;                   
                info->rcDisplay.right  = pDecInfo->decOutInfo[displayIndex].rcDecoded.bottom;                
                info->rcDisplay.top    = pDecInfo->decOutInfo[displayIndex].rcDecoded.left;
                info->rcDisplay.bottom = pDecInfo->decOutInfo[displayIndex].rcDecoded.right;
            }
            else
            {
                info->rcDisplay.left   = pDecInfo->decOutInfo[displayIndex].rcDecoded.left;
                info->rcDisplay.right  = pDecInfo->decOutInfo[displayIndex].rcDecoded.right;
                info->rcDisplay.top    = pDecInfo->decOutInfo[displayIndex].rcDecoded.top;
                info->rcDisplay.bottom = pDecInfo->decOutInfo[displayIndex].rcDecoded.bottom;
            }
        }
        else
        {
            if (pDecInfo->rotationEnable)
            {
                switch(pDecInfo->rotationAngle)
                {
                case 90:
                    info->rcDisplay.left   = pDecInfo->decOutInfo[displayIndex].rcDecoded.top;
                    info->rcDisplay.right  = pDecInfo->decOutInfo[displayIndex].rcDecoded.bottom;
                    info->rcDisplay.top    = info->decPicWidth - pDecInfo->decOutInfo[displayIndex].rcDecoded.right;
                    info->rcDisplay.bottom = info->decPicWidth - pDecInfo->decOutInfo[displayIndex].rcDecoded.left;
                    break;
                case 270:
                    info->rcDisplay.left   = info->decPicHeight - pDecInfo->decOutInfo[displayIndex].rcDecoded.bottom;
                    info->rcDisplay.right  = info->decPicHeight - pDecInfo->decOutInfo[displayIndex].rcDecoded.top;
                    info->rcDisplay.top    = pDecInfo->decOutInfo[displayIndex].rcDecoded.left;
                    info->rcDisplay.bottom = pDecInfo->decOutInfo[displayIndex].rcDecoded.right;
                    break;
                case 180:
                    info->rcDisplay.left   = pDecInfo->decOutInfo[displayIndex].rcDecoded.left;
                    info->rcDisplay.right  = pDecInfo->decOutInfo[displayIndex].rcDecoded.right;
                    info->rcDisplay.top    = info->decPicHeight - pDecInfo->decOutInfo[displayIndex].rcDecoded.bottom;
                    info->rcDisplay.bottom = info->decPicHeight - pDecInfo->decOutInfo[displayIndex].rcDecoded.top;
                    break;
                default:
                    info->rcDisplay.left   = pDecInfo->decOutInfo[displayIndex].rcDecoded.left;
                    info->rcDisplay.right  = pDecInfo->decOutInfo[displayIndex].rcDecoded.right;
                    info->rcDisplay.top    = pDecInfo->decOutInfo[displayIndex].rcDecoded.top;
                    info->rcDisplay.bottom = pDecInfo->decOutInfo[displayIndex].rcDecoded.bottom;
                    break;
                }

            }
            else
            {
                info->rcDisplay.left   = pDecInfo->decOutInfo[displayIndex].rcDecoded.left;
                info->rcDisplay.right  = pDecInfo->decOutInfo[displayIndex].rcDecoded.right;
                info->rcDisplay.top    = pDecInfo->decOutInfo[displayIndex].rcDecoded.top;
                info->rcDisplay.bottom = pDecInfo->decOutInfo[displayIndex].rcDecoded.bottom;
            }

            if (pDecInfo->mirrorEnable) {
                Uint32 temp; 
                if (pDecInfo->mirrorDirection & MIRDIR_VER) {
                    temp = info->rcDisplay.top;
                    info->rcDisplay.top    = info->decPicHeight - info->rcDisplay.bottom;
                    info->rcDisplay.bottom = info->decPicHeight - temp;
                }
                if (pDecInfo->mirrorDirection & MIRDIR_HOR) {
                    temp = info->rcDisplay.left;
                    info->rcDisplay.left  = info->decPicWidth - info->rcDisplay.right;
                    info->rcDisplay.right = info->decPicWidth - temp;
                }
            }

            switch (pCodecInst->codecMode) {
            case HEVC_DEC: 
                info->h265Info.displayPOC = pDecInfo->decOutInfo[displayIndex].h265Info.decodedPOC;
                break;
            default:
                break;
            }
        }

        if (info->indexFrameDisplay == info->indexFrameDecoded)
        {
            info->dispPicWidth =  info->decPicWidth;
            info->dispPicHeight = info->decPicHeight;
        }
        else
        {
            /* 
                When indexFrameDecoded < 0, and indexFrameDisplay >= 0
                info->decPicWidth and info->decPicHeight are still valid
                But those of pDecInfo->decOutInfo[displayIndex] are invalid in VP9
            */
            info->dispPicWidth  = pDecInfo->decOutInfo[displayIndex].decPicWidth;
            info->dispPicHeight = pDecInfo->decOutInfo[displayIndex].decPicHeight;
        }

    }
    else
    {
        info->rcDisplay.left   = 0;  
        info->rcDisplay.right  = 0;  
        info->rcDisplay.top    = 0;  
        info->rcDisplay.bottom = 0;

        if (pDecInfo->rotationEnable || pDecInfo->mirrorEnable || pDecInfo->tiled2LinearEnable || pDecInfo->deringEnable) {
            info->dispPicWidth  = info->decPicWidth;
            info->dispPicHeight = info->decPicHeight;
        }
        else {
            info->dispPicWidth = 0;
            info->dispPicHeight = 0;
        }
    }

    if ( (pCodecInst->codecMode == VC1_DEC || pCodecInst->codecMode == C7_VC1_DEC) && info->indexFrameDisplay != -3) {
        if (pDecInfo->vc1BframeDisplayValid == 0) {
            if (info->picType == 2) 
                info->indexFrameDisplay = -3;
            else
                pDecInfo->vc1BframeDisplayValid = 1;
        }
    }

    pDecInfo->streamRdPtr      = VpuReadReg(pCodecInst->coreIdx, pDecInfo->streamRdPtrRegAddr);
    pDecInfo->frameDisplayFlag = VpuReadReg(pCodecInst->coreIdx, pDecInfo->frameDisplayFlagRegAddr);
    if (pCodecInst->codecMode == C7_VP9_DEC) {
        pDecInfo->frameDisplayFlag  &= 0xFFFF;
    }
    pDecInfo->frameEndPos      = pDecInfo->streamRdPtr;

    if (pDecInfo->frameEndPos < pDecInfo->frameStartPos) 
        info->consumedByte = pDecInfo->frameEndPos + pDecInfo->streamBufSize - pDecInfo->frameStartPos;
    else 
        info->consumedByte = pDecInfo->frameEndPos - pDecInfo->frameStartPos;

    if (pDecInfo->deringEnable || pDecInfo->mirrorEnable || pDecInfo->rotationEnable || pDecInfo->tiled2LinearEnable) {
        info->dispFrame        = pDecInfo->rotatorOutput;
        info->dispFrame.stride = pDecInfo->rotatorStride;
    } 
    else {
        if(pCodecInst->productId == PRODUCT_ID_7Q && pCodecInst->codecMode != C7_HEVC_DEC) {
            if((pCodecInst->codecMode == C7_AVC_DEC && pDecInfo->initialInfo.interlace == 0))  // [TO BE OPT] use fbc enable information to adjust bfOffset
                val = pDecInfo->numFbsForDecoding;
            else
                val = 0;
        }
        else  {
            val = (pDecInfo->openParam.wtlEnable == TRUE ? pDecInfo->numFbsForDecoding: 0); //fbOffset   
        }

        maxDecIndex = (pDecInfo->numFbsForDecoding > pDecInfo->numFbsForWTL) ? pDecInfo->numFbsForDecoding : pDecInfo->numFbsForWTL;

        if (0 <= info->indexFrameDisplay && info->indexFrameDisplay < (int)maxDecIndex) 
            info->dispFrame = pDecInfo->frameBufPool[val+info->indexFrameDisplay];
    }

    info->rdPtr            = pDecInfo->streamRdPtr;
    info->wrPtr            = pDecInfo->streamWrPtr;
    info->frameDisplayFlag = pDecInfo->frameDisplayFlag;

    info->sequenceNo = pDecInfo->initialInfo.sequenceNo;
    if (decodedIndex >= 0 && decodedIndex < MAX_GDI_IDX) {
        pDecInfo->decOutInfo[decodedIndex] = *info;
    }

    if (displayIndex >= 0 && displayIndex < MAX_GDI_IDX) {
        info->numOfTotMBs = info->numOfTotMBs;
        info->numOfErrMBs = info->numOfErrMBs;
        info->numOfTotMBsInDisplay = pDecInfo->decOutInfo[displayIndex].numOfTotMBs;
        info->numOfErrMBsInDisplay = pDecInfo->decOutInfo[displayIndex].numOfErrMBs;
        info->dispFrame.sequenceNo = info->sequenceNo;
    }
    else {
        info->numOfTotMBsInDisplay = 0;
        info->numOfErrMBsInDisplay = 0;
    }

    if (info->sequenceChanged != 0) {
        if (!(pCodecInst->productId == PRODUCT_ID_960 || pCodecInst->productId == PRODUCT_ID_980 || pCodecInst->productId == PRODUCT_ID_7Q)) {
	        /* Update new sequence information */
	        osal_memcpy((void*)&pDecInfo->initialInfo, (void*)&pDecInfo->newSeqInfo, sizeof(DecInitialInfo));
        }
        if ((info->sequenceChanged & SEQ_CHANGE_INTER_RES_CHANGE) != SEQ_CHANGE_INTER_RES_CHANGE) { 
            pDecInfo->initialInfo.sequenceNo++;
        }
    }
	
    if (pCodecInst->productId == PRODUCT_ID_412) {
        if ((info->sequenceChanged & SEQ_CHANGE_INTER_RES_CHANGE) && info->indexInterFrameDecoded == -1) {
            pDecInfo->initialInfo.sequenceNo--;
        }
    }

    SetPendingInst(pCodecInst->coreIdx, 0);

    LeaveLock(pCodecInst->coreIdx);

    return RETCODE_SUCCESS;
}

RetCode VPU_DecFrameBufferFlush(DecHandle handle, DecOutputInfo* pRemainings, Uint32* retNum)
{
    CodecInst*       pCodecInst;
    DecInfo*         pDecInfo;
    DecOutputInfo*   pOut;
    RetCode          ret;
    FramebufferIndex retIndex[MAX_GDI_IDX];
    Uint32           retRemainings = 0;
    Int32            i, index, val;
    VpuAttr*         pAttr      = NULL;

    ret = CheckDecInstanceValidity(handle);
    if (ret != RETCODE_SUCCESS)
        return ret;

    pCodecInst = handle;
    pDecInfo = &pCodecInst->CodecInfo->decInfo;

    EnterLock(pCodecInst->coreIdx);

    if (GetPendingInst(pCodecInst->coreIdx)) {
        LeaveLock(pCodecInst->coreIdx);
        return RETCODE_FRAME_NOT_COMPLETE;
    }

    osal_memset((void*)retIndex, 0xff, sizeof(retIndex));

    pAttr = &g_VpuCoreAttributes[pCodecInst->coreIdx];
    if (pAttr->supportCommandQueue == FALSE) {
        EnterDispFlagLock(pCodecInst->coreIdx);
        val  = pDecInfo->frameDisplayFlag;
        val |= pDecInfo->setDisplayIndexes;
        val &= ~(Uint32)(pDecInfo->clearDisplayIndexes);
        VpuWriteReg(pCodecInst->coreIdx, pDecInfo->frameDisplayFlagRegAddr, val);
        pDecInfo->clearDisplayIndexes = 0;
        pDecInfo->setDisplayIndexes = 0;
        LeaveDispFlagLock(pCodecInst->coreIdx);
    }

    if ((ret=ProductVpuDecFlush(pCodecInst, retIndex, MAX_GDI_IDX)) != RETCODE_SUCCESS) {
        LeaveLock(pCodecInst->coreIdx);
        return ret;
    }

    if (pRemainings != NULL) {
        for (i=0; i<MAX_GDI_IDX; i++) {
            index = (pDecInfo->wtlEnable == TRUE) ? retIndex[i].tiledIndex : retIndex[i].linearIndex;
            if (index < 0) 
                break;
            pRemainings[i] = pDecInfo->decOutInfo[index];
            pOut = &pRemainings[i];
            pOut->indexFrameDisplay         = pOut->indexFrameDecoded;
            pOut->indexFrameDisplayForTiled = pOut->indexFrameDecodedForTiled;
            if (pDecInfo->wtlEnable == TRUE) 
                pOut->dispFrame             = pDecInfo->frameBufPool[pDecInfo->numFbsForDecoding+retIndex[i].linearIndex];
            else
                pOut->dispFrame             = pDecInfo->frameBufPool[index];

            pOut->dispFrame.sequenceNo = pOut->sequenceNo;

            if (pCodecInst->codecMode == C7_VP9_DEC || pCodecInst->codecMode == C7_AVS2_DEC) {
                Uint32   regVal;
                regVal = VpuReadReg(pCodecInst->coreIdx, W4_RET_DEC_DISPLAY_SIZE);
                pOut->dispPicWidth   = regVal>>16;
                pOut->dispPicHeight  = regVal&0xffff;
            } 
            else {
                pOut->dispPicWidth  = pOut->decPicWidth;
                pOut->dispPicHeight = pOut->decPicHeight;
            }

            if (pCodecInst->codecMode == VC1_DEC || pCodecInst->codecMode == C7_VC1_DEC) // vc1 rotates decoded frame buffer region. the other std rotated whole frame buffer region.
            {
                if (pDecInfo->rotationEnable && (pDecInfo->rotationAngle==90 || pDecInfo->rotationAngle==270))
                {
                    pOut->rcDisplay.left   = pDecInfo->decOutInfo[index].rcDecoded.top;                   
                    pOut->rcDisplay.right  = pDecInfo->decOutInfo[index].rcDecoded.bottom;                
                    pOut->rcDisplay.top    = pDecInfo->decOutInfo[index].rcDecoded.left;
                    pOut->rcDisplay.bottom = pDecInfo->decOutInfo[index].rcDecoded.right;
                }
                else
                {
                    pOut->rcDisplay.left   = pDecInfo->decOutInfo[index].rcDecoded.left;
                    pOut->rcDisplay.right  = pDecInfo->decOutInfo[index].rcDecoded.right;
                    pOut->rcDisplay.top    = pDecInfo->decOutInfo[index].rcDecoded.top;
                    pOut->rcDisplay.bottom = pDecInfo->decOutInfo[index].rcDecoded.bottom;
                }
            }
            else
            {
                if (pDecInfo->rotationEnable)
                {
                    switch(pDecInfo->rotationAngle)
                    {
                    case 90:
                        pOut->rcDisplay.left   = pDecInfo->decOutInfo[index].rcDecoded.top;
                        pOut->rcDisplay.right  = pDecInfo->decOutInfo[index].rcDecoded.bottom;
                        pOut->rcDisplay.top    = pOut->decPicWidth - pDecInfo->decOutInfo[index].rcDecoded.right;
                        pOut->rcDisplay.bottom = pOut->decPicWidth - pDecInfo->decOutInfo[index].rcDecoded.left;
                        break;
                    case 270:
                        pOut->rcDisplay.left   = pOut->decPicHeight - pDecInfo->decOutInfo[index].rcDecoded.bottom;
                        pOut->rcDisplay.right  = pOut->decPicHeight - pDecInfo->decOutInfo[index].rcDecoded.top;
                        pOut->rcDisplay.top    = pDecInfo->decOutInfo[index].rcDecoded.left;
                        pOut->rcDisplay.bottom = pDecInfo->decOutInfo[index].rcDecoded.right;
                        break;
                    case 180:
                        pOut->rcDisplay.left   = pDecInfo->decOutInfo[index].rcDecoded.left;
                        pOut->rcDisplay.right  = pDecInfo->decOutInfo[index].rcDecoded.right;
                        pOut->rcDisplay.top    = pOut->decPicHeight - pDecInfo->decOutInfo[index].rcDecoded.bottom;
                        pOut->rcDisplay.bottom = pOut->decPicHeight - pDecInfo->decOutInfo[index].rcDecoded.top;
                        break;
                    default:
                        pOut->rcDisplay.left   = pDecInfo->decOutInfo[index].rcDecoded.left;
                        pOut->rcDisplay.right  = pDecInfo->decOutInfo[index].rcDecoded.right;
                        pOut->rcDisplay.top    = pDecInfo->decOutInfo[index].rcDecoded.top;
                        pOut->rcDisplay.bottom = pDecInfo->decOutInfo[index].rcDecoded.bottom;
                        break;
                    }

                }
                else
                {
                    pOut->rcDisplay.left   = pDecInfo->decOutInfo[index].rcDecoded.left;
                    pOut->rcDisplay.right  = pDecInfo->decOutInfo[index].rcDecoded.right;
                    pOut->rcDisplay.top    = pDecInfo->decOutInfo[index].rcDecoded.top;
                    pOut->rcDisplay.bottom = pDecInfo->decOutInfo[index].rcDecoded.bottom;
                }
            }
            retRemainings++;
        }
    }

    if (retNum) *retNum = retRemainings;

    if (pCodecInst->loggingEnable)
        vdi_log(pCodecInst->coreIdx, DEC_BUF_FLUSH, 0);

    LeaveLock(pCodecInst->coreIdx);     

    return ret;
}

RetCode VPU_DecSetRdPtr(DecHandle handle, PhysicalAddress addr, int updateWrPtr)
{
    CodecInst* pCodecInst;
    CodecInst* pPendingInst;
    DecInfo*   pDecInfo;
    RetCode    ret;

    ret = CheckDecInstanceValidity(handle);
    if (ret != RETCODE_SUCCESS) {
        return ret;
    }
    pCodecInst = (CodecInst*)handle;
    ret = ProductVpuDecCheckCapability(pCodecInst);
    if (ret != RETCODE_SUCCESS) {
        return ret;
    }

    pCodecInst   = handle;
    pDecInfo     = &pCodecInst->CodecInfo->decInfo;
    pPendingInst = GetPendingInst(pCodecInst->coreIdx);
    if (pCodecInst == pPendingInst) {
        VpuWriteReg(pCodecInst->coreIdx, pDecInfo->streamRdPtrRegAddr, addr);
    }
    else {
        EnterLock(pCodecInst->coreIdx);
        VpuWriteReg(pCodecInst->coreIdx, pDecInfo->streamRdPtrRegAddr, addr);
        LeaveLock(pCodecInst->coreIdx);
    }

    pDecInfo->streamRdPtr = addr;
    pDecInfo->prevFrameEndPos = addr;
    if (updateWrPtr == TRUE) {
        pDecInfo->streamWrPtr = addr;
    }
    pDecInfo->rdPtrValidFlag = 1;
    return RETCODE_SUCCESS;
}

RetCode VPU_EncSetWrPtr(EncHandle handle, PhysicalAddress addr, int updateRdPtr)
{
    CodecInst* pCodecInst;
    CodecInst* pPendingInst;
    EncInfo * pEncInfo;
    RetCode ret;

    ret = CheckEncInstanceValidity(handle);
    if (ret != RETCODE_SUCCESS)
        return ret;
    pCodecInst = (CodecInst*)handle;

    if (pCodecInst->productId == PRODUCT_ID_960 || pCodecInst->productId == PRODUCT_ID_980) {
        return RETCODE_NOT_SUPPORTED_FEATURE;
    }

    pEncInfo = &handle->CodecInfo->encInfo;
    pPendingInst = GetPendingInst(pCodecInst->coreIdx);
    if (pCodecInst == pPendingInst) {
        VpuWriteReg(pCodecInst->coreIdx, pEncInfo->streamWrPtrRegAddr, addr);
    }
    else {
        EnterLock(pCodecInst->coreIdx);
        VpuWriteReg(pCodecInst->coreIdx, pEncInfo->streamWrPtrRegAddr, addr);
        LeaveLock(pCodecInst->coreIdx);
    }
    pEncInfo->streamWrPtr = addr;
    if (updateRdPtr)
        pEncInfo->streamRdPtr = addr;

    return RETCODE_SUCCESS;
}

RetCode VPU_DecClrDispFlag(DecHandle handle, int index)
{
    CodecInst*  pCodecInst;
    DecInfo*    pDecInfo;
    RetCode     ret         = RETCODE_SUCCESS;
    Int32       endIndex;
    VpuAttr*    pAttr       = NULL;
    BOOL        supportCommandQueue;

    ret = CheckDecInstanceValidity(handle);
    if (ret != RETCODE_SUCCESS)
        return ret;

    pCodecInst = handle;
    pDecInfo   = &pCodecInst->CodecInfo->decInfo;
    pAttr      = &g_VpuCoreAttributes[pCodecInst->coreIdx];

    endIndex = (pDecInfo->openParam.wtlEnable == TRUE) ? pDecInfo->numFbsForWTL : pDecInfo->numFbsForDecoding;

    if ((index < 0) || (index > (endIndex - 1))) {
        return RETCODE_INVALID_PARAM;
    }
    
    supportCommandQueue = (pAttr->supportCommandQueue == TRUE);
    if (supportCommandQueue == TRUE) {
        EnterLock(pCodecInst->coreIdx);
        ret = ProductClrDispFlag(pCodecInst, index);    
        LeaveLock(pCodecInst->coreIdx);
    }
    else {
        EnterDispFlagLock(pCodecInst->coreIdx);
        pDecInfo->clearDisplayIndexes |= (1<<index);
        LeaveDispFlagLock(pCodecInst->coreIdx);
    }
    
    return ret;
}

RetCode VPU_DecGiveCommand(DecHandle handle, CodecCommand cmd, void* param)
{
    CodecInst*          pCodecInst;
    DecInfo*            pDecInfo;
    RetCode             ret;
    PhysicalAddress     addr;

    ret = CheckDecInstanceValidity(handle);
    if (ret != RETCODE_SUCCESS)
        return ret;

    pCodecInst = handle;
    pDecInfo = &pCodecInst->CodecInfo->decInfo;
    switch (cmd) 
    {
    case ENABLE_ROTATION :
        {
            if (pDecInfo->rotatorStride == 0) {
                return RETCODE_ROTATOR_STRIDE_NOT_SET;
            }
            pDecInfo->rotationEnable = 1;
            break;
        }

    case DISABLE_ROTATION :
        {
            pDecInfo->rotationEnable = 0;
            break;
        }

    case ENABLE_MIRRORING :
        {
            if (pDecInfo->rotatorStride == 0) {
                return RETCODE_ROTATOR_STRIDE_NOT_SET;
            }
            pDecInfo->mirrorEnable = 1;
            break;
        }
    case DISABLE_MIRRORING :
        {
            pDecInfo->mirrorEnable = 0;
            break;
        }
    case SET_MIRROR_DIRECTION :
        {

            MirrorDirection mirDir;

            if (param == 0) {
                return RETCODE_INVALID_PARAM;
            }
            mirDir = *(MirrorDirection *)param;
            if ( !(mirDir == MIRDIR_NONE) && !(mirDir==MIRDIR_HOR) && !(mirDir==MIRDIR_VER) && !(mirDir==MIRDIR_HOR_VER)) {
                return RETCODE_INVALID_PARAM;
            }
            pDecInfo->mirrorDirection = mirDir;

            break;
        }
    case SET_ROTATION_ANGLE :
        {
            int angle;

            if (param == 0) {
                return RETCODE_INVALID_PARAM;
            }
            angle = *(int *)param;
            if (angle != 0 && angle != 90 &&
                angle != 180 && angle != 270) {
                    return RETCODE_INVALID_PARAM;
            }
            if (pDecInfo->rotatorStride != 0) {                
                if (angle == 90 || angle ==270) {
                    if (pDecInfo->initialInfo.picHeight > pDecInfo->rotatorStride) {
                        return RETCODE_INVALID_PARAM;
                    }
                } else {
                    if (pDecInfo->initialInfo.picWidth > pDecInfo->rotatorStride) {
                        return RETCODE_INVALID_PARAM;
                    }
                }
            }

            pDecInfo->rotationAngle = angle;
            break;
        }
    case SET_ROTATOR_OUTPUT :
        {
            FrameBuffer *frame;
            if (param == 0) {
                return RETCODE_INVALID_PARAM;
            }
            frame = (FrameBuffer *)param;

            pDecInfo->rotatorOutput = *frame;
            pDecInfo->rotatorOutputValid = 1;
            break;
        }        

    case SET_ROTATOR_STRIDE :
        {
            int stride;

            if (param == 0) {
                return RETCODE_INVALID_PARAM;
            }
            stride = *(int *)param;
            if (stride % 8 != 0 || stride == 0) {
                return RETCODE_INVALID_STRIDE;
            }

            if (pDecInfo->rotationAngle == 90 || pDecInfo->rotationAngle == 270) {
                if (pDecInfo->initialInfo.picHeight > stride) {
                    return RETCODE_INVALID_STRIDE;
                }
            } else {
                if (pDecInfo->initialInfo.picWidth > stride) {
                    return RETCODE_INVALID_STRIDE;
                }                   
            }

            pDecInfo->rotatorStride = stride;
            break;
        }
    case DEC_SET_SPS_RBSP:
        {
            if (pCodecInst->codecMode != AVC_DEC && pCodecInst->codecMode != C7_AVC_DEC) {
                return RETCODE_INVALID_COMMAND;
            }
            if (param == 0) {
                return RETCODE_INVALID_PARAM;
            }

            return SetParaSet(handle, 0, (DecParamSet *)param);

        }

    case DEC_SET_PPS_RBSP:
        {
            if (pCodecInst->codecMode != AVC_DEC && pCodecInst->codecMode != C7_AVC_DEC) {
                return RETCODE_INVALID_COMMAND;
            }
            if (param == 0) {
                return RETCODE_INVALID_PARAM;
            }

            return SetParaSet(handle, 1, (DecParamSet *)param);            
        }
    case ENABLE_DERING :
        {
            if (pDecInfo->rotatorStride == 0) {
                return RETCODE_ROTATOR_STRIDE_NOT_SET;
            }
            pDecInfo->deringEnable = 1;
            break;
        }

    case DISABLE_DERING :
        {
            pDecInfo->deringEnable = 0;
            break;
        }
    case SET_SEC_AXI:
        {
            SecAxiUse secAxiUse;

            if (param == 0) {
                return RETCODE_INVALID_PARAM;
            }
            secAxiUse = *(SecAxiUse *)param;
            if (handle->productId == PRODUCT_ID_410 || handle->productId == PRODUCT_ID_4102 || handle->productId == PRODUCT_ID_420 ||
                handle->productId == PRODUCT_ID_412 || handle->productId == PRODUCT_ID_420L || handle->productId == PRODUCT_ID_510 || 
                handle->productId == PRODUCT_ID_512 || handle->productId == PRODUCT_ID_515 || handle->productId == PRODUCT_ID_520 ) {
                pDecInfo->secAxiInfo.u.wave4.useIpEnable    = secAxiUse.u.wave4.useIpEnable;
                pDecInfo->secAxiInfo.u.wave4.useLfRowEnable = secAxiUse.u.wave4.useLfRowEnable;
                pDecInfo->secAxiInfo.u.wave4.useBitEnable   = secAxiUse.u.wave4.useBitEnable;
            } 
            else if (handle->productId == PRODUCT_ID_7Q) {
                if (handle->codecMode == C7_HEVC_DEC) {
                    pDecInfo->secAxiInfo.u.wave4.useIpEnable    = secAxiUse.u.wave4.useIpEnable;
                    pDecInfo->secAxiInfo.u.wave4.useLfRowEnable = secAxiUse.u.wave4.useLfRowEnable;
                    pDecInfo->secAxiInfo.u.wave4.useBitEnable   = secAxiUse.u.wave4.useBitEnable;
                }
                else {
                    pDecInfo->secAxiInfo.u.coda9.useBitEnable  = secAxiUse.u.coda9.useBitEnable;
                    pDecInfo->secAxiInfo.u.coda9.useIpEnable   = secAxiUse.u.coda9.useIpEnable;
                    pDecInfo->secAxiInfo.u.coda9.useDbkYEnable = secAxiUse.u.coda9.useDbkYEnable;
                    pDecInfo->secAxiInfo.u.coda9.useDbkCEnable = secAxiUse.u.coda9.useDbkCEnable;
                    pDecInfo->secAxiInfo.u.coda9.useOvlEnable  = secAxiUse.u.coda9.useOvlEnable;
                }
            }
            else {
                pDecInfo->secAxiInfo.u.coda9.useBitEnable  = secAxiUse.u.coda9.useBitEnable;
                pDecInfo->secAxiInfo.u.coda9.useIpEnable   = secAxiUse.u.coda9.useIpEnable;
                pDecInfo->secAxiInfo.u.coda9.useDbkYEnable = secAxiUse.u.coda9.useDbkYEnable;
                pDecInfo->secAxiInfo.u.coda9.useDbkCEnable = secAxiUse.u.coda9.useDbkCEnable;
                pDecInfo->secAxiInfo.u.coda9.useOvlEnable  = secAxiUse.u.coda9.useOvlEnable;
                pDecInfo->secAxiInfo.u.coda9.useBtpEnable  = secAxiUse.u.coda9.useBtpEnable;
            }

            break;
        }
    case ENABLE_REP_USERDATA:
        {
            if (!pDecInfo->userDataBufAddr) {
                return RETCODE_USERDATA_BUF_NOT_SET;
            }
            if (pDecInfo->userDataBufSize == 0) {
                return RETCODE_USERDATA_BUF_NOT_SET;
            }
            switch (pCodecInst->productId) {
            case PRODUCT_ID_4102:
            case PRODUCT_ID_410:
            case PRODUCT_ID_412:
            case PRODUCT_ID_7Q:
            case PRODUCT_ID_510:
            case PRODUCT_ID_512:
            case PRODUCT_ID_515:
            case PRODUCT_ID_420:
            case PRODUCT_ID_420L:
                pDecInfo->userDataEnable = *(Uint32*)param;
                break;
            case PRODUCT_ID_960:
            case PRODUCT_ID_980:
                pDecInfo->userDataEnable = TRUE;
                break;
            default:
                VLOG(INFO, "%s(ENABLE_REP_DATA) invalid productId(%d)\n", __FUNCTION__, pCodecInst->productId);
                return RETCODE_INVALID_PARAM;
            }
            break;
        }
    case DISABLE_REP_USERDATA:
        {
            pDecInfo->userDataEnable = 0;
            break;
        }
    case SET_ADDR_REP_USERDATA:
        {
            PhysicalAddress userDataBufAddr;

            if (param == 0) {
                return RETCODE_INVALID_PARAM;
            }
            userDataBufAddr = *(PhysicalAddress *)param;
            if (userDataBufAddr % 8 != 0 || userDataBufAddr == 0) {
                return RETCODE_INVALID_PARAM;
            }

            pDecInfo->userDataBufAddr = userDataBufAddr;
            break;
        }
    case SET_VIRT_ADDR_REP_USERDATA:
        {
            unsigned long userDataVirtAddr;

            if (param == 0) {
                return RETCODE_INVALID_PARAM;
            }

            if (!pDecInfo->userDataBufAddr) {
                return RETCODE_USERDATA_BUF_NOT_SET;
            }
            if (pDecInfo->userDataBufSize == 0) {
                return RETCODE_USERDATA_BUF_NOT_SET;
            }

            userDataVirtAddr = *(unsigned long *)param;
            if (!userDataVirtAddr) {
                return RETCODE_INVALID_PARAM;
            }

            pDecInfo->vbUserData.phys_addr = pDecInfo->userDataBufAddr;
            pDecInfo->vbUserData.size = pDecInfo->userDataBufSize;
            pDecInfo->vbUserData.virt_addr = (unsigned long)userDataVirtAddr;
            if (vdi_attach_dma_memory(pCodecInst->coreIdx, &pDecInfo->vbUserData) != 0) {
                return RETCODE_INSUFFICIENT_RESOURCE;
            }
            break;
        }
    case SET_SIZE_REP_USERDATA:
        {
            PhysicalAddress userDataBufSize;

            if (param == 0) {
                return RETCODE_INVALID_PARAM;
            }
            userDataBufSize = *(PhysicalAddress *)param;

            pDecInfo->userDataBufSize = userDataBufSize;
            break;
        }

    case SET_USERDATA_REPORT_MODE:
        {
            int userDataMode;

            userDataMode = *(int *)param;
            if (userDataMode != 1 && userDataMode != 0) {
                return RETCODE_INVALID_PARAM;
            }
            pDecInfo->userDataReportMode = userDataMode;
            break;
        }
        /** WAVE4 CU DATA REPORT INTERFACE **/
    case ENABLE_REP_CUDATA:
        if (!pDecInfo->cuDataBufAddr) {
            /* share error code with USERDATA */
            return RETCODE_USERDATA_BUF_NOT_SET;
        }
        if (pDecInfo->cuDataBufSize == 0) {
            /* share error code with USERDATA */
            return RETCODE_USERDATA_BUF_NOT_SET;
        }
        pDecInfo->cuDataEnable = TRUE;
        break;
    case DISABLE_REP_CUDATA:
        pDecInfo->cuDataEnable = FALSE;
        break;
    case SET_ADDR_REP_CUDATA:
        if (param == 0) {
            return RETCODE_INVALID_PARAM;
        }
        addr = *(PhysicalAddress *)param;
        if ((addr % 8) != 0 || addr == 0) {
            return RETCODE_INVALID_PARAM;
        }

        pDecInfo->cuDataBufAddr = addr;
        break;
    case SET_SIZE_REP_CUDATA:
        if (param == 0) {
            return RETCODE_INVALID_PARAM;
        }
        pDecInfo->cuDataBufSize = *(PhysicalAddress *)param;
        break;
        /************************************/
    case SET_CACHE_CONFIG:
        {
            MaverickCacheConfig *mcCacheConfig;
            if (param == 0) {
                return RETCODE_INVALID_PARAM;
            }            
            mcCacheConfig = (MaverickCacheConfig *)param;
            pDecInfo->cacheConfig = *mcCacheConfig;
        }
        break;
    case SET_LOW_DELAY_CONFIG:
        {
            LowDelayInfo *lowDelayInfo;
            if (param == 0) {
                return RETCODE_INVALID_PARAM;
            }
            if (pCodecInst->productId != PRODUCT_ID_980) {
                return RETCODE_NOT_SUPPORTED_FEATURE;
            }
            lowDelayInfo = (LowDelayInfo *)param;

            if (lowDelayInfo->lowDelayEn) {
                if ( (pCodecInst->codecMode != AVC_DEC && pCodecInst->codecMode != C7_AVC_DEC) ||
                    pDecInfo->rotationEnable ||
                    pDecInfo->mirrorEnable ||
                    pDecInfo->tiled2LinearEnable ||
                    pDecInfo->deringEnable) {
                        return RETCODE_INVALID_PARAM;
                }
            }

            pDecInfo->lowDelayInfo.lowDelayEn = lowDelayInfo->lowDelayEn;
            pDecInfo->lowDelayInfo.numRows    = lowDelayInfo->numRows;
        }
        break;

    case SET_DECODE_FLUSH:    // interrupt mode to pic_end 
        ret = ProductCpbFlush((CodecInst*)handle);
        break;

    case DEC_SET_FRAME_DELAY:
        {
            pDecInfo->frameDelay = *(int *)param;
            break;
        }
    case DEC_ENABLE_REORDER:
        {
            if((handle->productId == PRODUCT_ID_980) || (handle->productId == PRODUCT_ID_960) || (handle->productId == PRODUCT_ID_950) || (handle->productId == PRODUCT_ID_7503) || (handle->productId == PRODUCT_ID_320))
            {
                if (pDecInfo->initialInfoObtained) {
                    return RETCODE_WRONG_CALL_SEQUENCE;
                }
            }
            
            pDecInfo->reorderEnable = 1;
            break;
        }
    case DEC_DISABLE_REORDER:
        {
            if((handle->productId == PRODUCT_ID_980) || (handle->productId == PRODUCT_ID_960) || (handle->productId == PRODUCT_ID_950) || (handle->productId == PRODUCT_ID_7503) || (handle->productId == PRODUCT_ID_320))
            {
                if (pDecInfo->initialInfoObtained) {
                    return RETCODE_WRONG_CALL_SEQUENCE;
                }

                if(pCodecInst->codecMode != AVC_DEC && pCodecInst->codecMode != VC1_DEC && pCodecInst->codecMode != AVS_DEC && pCodecInst->codecMode != C7_AVC_DEC && pCodecInst->codecMode != C7_VC1_DEC && pCodecInst->codecMode != C7_AVS_DEC) {
                    return RETCODE_INVALID_COMMAND;
                }
            }

            pDecInfo->reorderEnable = 0;
            break;
        }    
    case DEC_SET_AVC_ERROR_CONCEAL_MODE:
        {
            if(pCodecInst->codecMode != AVC_DEC && pCodecInst->codecMode != C7_AVC_DEC) {
                return RETCODE_INVALID_COMMAND;
            }

            pDecInfo->avcErrorConcealMode = *(int *)param;
            break;
        }    
    case DEC_FREE_FRAME_BUFFER:
        {
            int i;
            if (pDecInfo->vbSlice.size) 
                vdi_free_dma_memory(pCodecInst->coreIdx, &pDecInfo->vbSlice);

            if (pDecInfo->vbFrame.size){
                if (pDecInfo->frameAllocExt == 0) 
                    vdi_free_dma_memory(pCodecInst->coreIdx, &pDecInfo->vbFrame);
            }    
            for (i=0  ; i<MAX_REG_FRAME; i++) {
                if (pDecInfo->vbFbcYTbl[i].size) 
                    vdi_free_dma_memory(pCodecInst->coreIdx, &pDecInfo->vbFbcYTbl[i]);

                if (pDecInfo->vbFbcCTbl[i].size) 
                    vdi_free_dma_memory(pCodecInst->coreIdx, &pDecInfo->vbFbcCTbl[i]);

                if (pDecInfo->vbMV[i].size) 
                    vdi_free_dma_memory(pCodecInst->coreIdx, &pDecInfo->vbMV[i]);
            }

            if (pDecInfo->vbPPU.size) {
                if (pDecInfo->ppuAllocExt == 0) 
                    vdi_free_dma_memory(pCodecInst->coreIdx, &pDecInfo->vbPPU);
            }

            if (pDecInfo->wtlEnable) {
                if (pDecInfo->vbWTL.size) 
                    vdi_free_dma_memory(pCodecInst->coreIdx, &pDecInfo->vbWTL);
            }
            break;
        }
    case DEC_GET_FRAMEBUF_INFO:
        {
            DecGetFramebufInfo* fbInfo = (DecGetFramebufInfo*)param;
            Uint32 i;
            fbInfo->vbFrame   = pDecInfo->vbFrame;
            fbInfo->vbWTL     = pDecInfo->vbWTL;
            for (i=0  ; i<MAX_REG_FRAME; i++)
            {
                fbInfo->vbFbcYTbl[i] = pDecInfo->vbFbcYTbl[i];
                fbInfo->vbFbcCTbl[i] = pDecInfo->vbFbcCTbl[i];
                fbInfo->vbMvCol[i]   = pDecInfo->vbMV[i];
            }

            for (i=0; i<MAX_GDI_IDX*2; i++) {
                fbInfo->framebufPool[i] = pDecInfo->frameBufPool[i];
            }
        }
        break;
    case DEC_RESET_FRAMEBUF_INFO:
        {
            int i;

            pDecInfo->vbFrame.base          = 0;
            pDecInfo->vbFrame.phys_addr     = 0;
            pDecInfo->vbFrame.virt_addr     = 0;
            pDecInfo->vbFrame.size          = 0;
            pDecInfo->vbWTL.base            = 0;
            pDecInfo->vbWTL.phys_addr       = 0;
            pDecInfo->vbWTL.virt_addr       = 0;
            pDecInfo->vbWTL.size            = 0;
            for (i=0  ; i<MAX_REG_FRAME; i++)
            {
                pDecInfo->vbFbcYTbl[i].base        = 0;
                pDecInfo->vbFbcYTbl[i].phys_addr   = 0;
                pDecInfo->vbFbcYTbl[i].virt_addr   = 0;
                pDecInfo->vbFbcYTbl[i].size        = 0;
                pDecInfo->vbFbcCTbl[i].base        = 0;
                pDecInfo->vbFbcCTbl[i].phys_addr   = 0;
                pDecInfo->vbFbcCTbl[i].virt_addr   = 0;
                pDecInfo->vbFbcCTbl[i].size        = 0;
                pDecInfo->vbMV[i].base             = 0;
                pDecInfo->vbMV[i].phys_addr        = 0;
                pDecInfo->vbMV[i].virt_addr        = 0;
                pDecInfo->vbMV[i].size             = 0;
            }

            pDecInfo->frameDisplayFlag      = 0;
            pDecInfo->setDisplayIndexes     = 0;
            pDecInfo->clearDisplayIndexes   = 0;
            break;
        }
    case DEC_GET_QUEUE_STATUS:
        {
            DecQueueStatusInfo* queueInfo = (DecQueueStatusInfo*)param;
            queueInfo->instanceQueueCount = pDecInfo->instanceQueueCount;
            queueInfo->totalQueueCount    = pDecInfo->totalQueueCount;
            break;
        }


    case ENABLE_DEC_THUMBNAIL_MODE:
        {
            pDecInfo->thumbnailMode = 1;
            break;
        }
    case DEC_GET_SEQ_INFO:
        {
            DecInitialInfo* seqInfo = (DecInitialInfo*)param;
            *seqInfo = pDecInfo->initialInfo;
            break;
        }
    case DEC_GET_FIELD_PIC_TYPE:
        {
            return RETCODE_FAILURE;
        }
    case DEC_GET_DISPLAY_OUTPUT_INFO:
        {
            DecOutputInfo *pDecOutInfo = (DecOutputInfo *)param;
            *pDecOutInfo = pDecInfo->decOutInfo[pDecOutInfo->indexFrameDisplay];
            break;
        }
    case GET_TILEDMAP_CONFIG:
        {
            TiledMapConfig *pMapCfg = (TiledMapConfig *)param;
            if (!pMapCfg) {
                return RETCODE_INVALID_PARAM;
            }
            if (!pDecInfo->stride) {
                return RETCODE_WRONG_CALL_SEQUENCE;

            }
            *pMapCfg = pDecInfo->mapCfg;
            break;
        }
    case SET_DRAM_CONFIG:
        {
            DRAMConfig *cfg = (DRAMConfig *)param;

            if (!cfg) {
                return RETCODE_INVALID_PARAM;
            }

            pDecInfo->dramCfg = *cfg;
            break;
        }
    case GET_DRAM_CONFIG:
        {
            DRAMConfig *cfg = (DRAMConfig *)param;

            if (!cfg) {
                return RETCODE_INVALID_PARAM;
            }

            *cfg = pDecInfo->dramCfg;

            break;
        }
    case GET_LOW_DELAY_OUTPUT:
        {
            DecOutputInfo *lowDelayOutput;
            if (param == 0) {
                return RETCODE_INVALID_PARAM;
            }

            if (!pDecInfo->lowDelayInfo.lowDelayEn || pCodecInst->codecMode != AVC_DEC) {
                return RETCODE_INVALID_COMMAND;
            }

            if (pCodecInst != GetPendingInst(pCodecInst->coreIdx)) {
                return RETCODE_WRONG_CALL_SEQUENCE;
            }

            lowDelayOutput = (DecOutputInfo *)param;

            GetLowDelayOutput(pCodecInst, lowDelayOutput);
        }
        break;
    case ENABLE_LOGGING:
        {
            pCodecInst->loggingEnable = 1;            
        }
        break;
    case DISABLE_LOGGING:
        {
            pCodecInst->loggingEnable = 0;
        }
        break;
    case DEC_SET_SEQ_CHANGE_MASK:
        if (PRODUCT_ID_NOT_W_SERIES(pCodecInst->productId))
            return RETCODE_INVALID_PARAM;
        pDecInfo->seqChangeMask = *(int*)param;
        break;
    case DEC_SET_WTL_FRAME_FORMAT:
        pDecInfo->wtlFormat = *(FrameBufferFormat*)param;
        break;
    case DEC_SET_DISPLAY_FLAG:
        {
            Int32       index;
            VpuAttr*    pAttr = &g_VpuCoreAttributes[pCodecInst->coreIdx];
            BOOL        supportCommandQueue = FALSE;

            if (param == 0) {
                return RETCODE_INVALID_PARAM;
            }

            index = *(Int32 *)param;

            supportCommandQueue = (pAttr->supportCommandQueue == TRUE);
            if (supportCommandQueue == TRUE) {
                EnterLock(pCodecInst->coreIdx);
                ProductSetDispFlag(pCodecInst, index);
                LeaveLock(pCodecInst->coreIdx);
            }
            else {
                EnterDispFlagLock(pCodecInst->coreIdx);
                pDecInfo->setDisplayIndexes |= (1<<index);
                LeaveDispFlagLock(pCodecInst->coreIdx);
            }
            
        }
        break;
#ifdef DRAM_TEST
    case DRAM_READ_WRITE_TEST:
        {
            Uint32 dram_source_addr;   
            Uint32 dram_destination_addr;   
            Uint32 dram_data_size;
            Uint32 result;
            Uint32 core_idx;

            unsigned int i;    
            //unsigned char probeData[64];                  
            unsigned char *probeSourceData;                     
            unsigned char *probeDestinationData;                    

            dram_source_addr = 0x80000000;   
            dram_destination_addr = 0x90000000;   
            dram_data_size = 0x00000010;
            core_idx = 0;   

            result = ExecDRAMReadWriteTest(core_idx, &dram_source_addr, &dram_destination_addr, &dram_data_size); 
            if (result == RETCODE_SUCCESS)       
                VLOG(INFO, "DRAM Read/Write Test is successful result = %x\r\n", result);   
            else      
                VLOG(INFO, "DRAM Read/Write Test is not successful result = %x\r\n", result); 

            probeSourceData = (unsigned char *)osal_malloc(dram_data_size*4);
            probeDestinationData = (unsigned char *)osal_malloc(dram_data_size*4);
            //osal_memset(probeData, 0xff, 64);          

            vdi_read_memory(core_idx, dram_source_addr, probeSourceData, dram_data_size*4, VDI_BIG_ENDIAN);

            for (i=0; i < (dram_data_size*4); i=i+8)    
            {        
                printf("probeSourceData 0x%04xh: 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n", dram_source_addr+i, probeSourceData[i], probeSourceData[i+1], probeSourceData[i+2], probeSourceData[i+3], probeSourceData[i+4], probeSourceData[i+5], probeSourceData[i+6], probeSourceData[i+7]);    
            }

            vdi_read_memory(core_idx, dram_destination_addr, probeDestinationData, dram_data_size*4, VDI_BIG_ENDIAN);

            for (i=0; i < (dram_data_size*4); i=i+8)    
            {        
                printf("probeDestinationData 0x%04xh: 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n", dram_destination_addr+i, probeDestinationData[i], probeDestinationData[i+1], probeDestinationData[i+2], probeDestinationData[i+3], probeDestinationData[i+4], probeDestinationData[i+5], probeDestinationData[i+6], probeDestinationData[i+7]);    
            }

            if(memcmp(probeSourceData, probeDestinationData, dram_data_size*4) == 0)
            {
                printf("Source Data and Destination Data are identical\r\n");
                result = RETCODE_SUCCESS;
            }
            else
            {
                printf("Source Data and Destination Data are not identical\r\n");
                result = RETCODE_FAILURE;
            }

            free(probeDestinationData);
            free(probeSourceData);
            break;          
        }
#endif /* SUPPORT DRAM_TEST */
    case DEC_SET_TARGET_TEMPORAL_ID:
        if (param == NULL) {
            return RETCODE_INVALID_PARAM;
        }

        if (AVC_DEC == pCodecInst->codecMode || HEVC_DEC == pCodecInst->codecMode) {
            Uint32 targetSublayerId = *(Uint32*)param;
            Uint32 maxTargetId      = (AVC_DEC == pCodecInst->codecMode) ? AVC_MAX_SUB_LAYER_ID : HEVC_MAX_SUB_LAYER_ID;

            if (targetSublayerId > maxTargetId) {
                ret = RETCODE_INVALID_PARAM;
        }
        else {
                pDecInfo->targetSubLayerId = targetSublayerId;
            }
            }
        else {
            ret= RETCODE_NOT_SUPPORTED_FEATURE;
        }
        break;
    case DEC_SET_BWB_CUR_FRAME_IDX:
        pDecInfo->chBwbFrameIdx = *(Uint32*)param;
        break;
    case DEC_SET_FBC_CUR_FRAME_IDX:
        pDecInfo->chFbcFrameIdx = *(Uint32*)param;
        break;
    case DEC_SET_INTER_RES_INFO_ON:
        pDecInfo->interResChange = 1;
        break;
    case DEC_SET_INTER_RES_INFO_OFF:
        pDecInfo->interResChange = 0;
        break;
    case DEC_FREE_FBC_TABLE_BUFFER:
        {
            Uint32 fbcCurFrameIdx = *(Uint32*)param;
            if(pDecInfo->vbFbcYTbl[fbcCurFrameIdx].size > 0) {
                vdi_free_dma_memory(pCodecInst->coreIdx, &pDecInfo->vbFbcYTbl[fbcCurFrameIdx]);
                pDecInfo->vbFbcYTbl[fbcCurFrameIdx].size = 0;
            }
            if(pDecInfo->vbFbcCTbl[fbcCurFrameIdx].size > 0) {
                vdi_free_dma_memory(pCodecInst->coreIdx, &pDecInfo->vbFbcCTbl[fbcCurFrameIdx]);
                pDecInfo->vbFbcCTbl[fbcCurFrameIdx].size = 0;
            }
        }
        break;
    case DEC_FREE_MV_BUFFER:
        {
            Uint32 fbcCurFrameIdx = *(Uint32*)param;
            if(pDecInfo->vbMV[fbcCurFrameIdx].size > 0) {
                vdi_free_dma_memory(pCodecInst->coreIdx, &pDecInfo->vbMV[fbcCurFrameIdx]);
                pDecInfo->vbMV[fbcCurFrameIdx].size = 0;
            }
        }
        break;
    case DEC_ALLOC_MV_BUFFER:
        {
            Uint32 fbcCurFrameIdx = *(Uint32*)param;
            Uint32 size;
            size          = WAVE4_DEC_VP9_MVCOL_BUF_SIZE(pDecInfo->initialInfo.picWidth, pDecInfo->initialInfo.picHeight);
            pDecInfo->vbMV[fbcCurFrameIdx].phys_addr = 0;
            pDecInfo->vbMV[fbcCurFrameIdx].size      = ((size+4095)&~4095)+4096;   /* 4096 is a margin */
            if (vdi_allocate_dma_memory(pCodecInst->coreIdx, &pDecInfo->vbMV[fbcCurFrameIdx]) < 0)
                return RETCODE_INSUFFICIENT_RESOURCE;
        }
        break;
    case DEC_ALLOC_FBC_Y_TABLE_BUFFER:
        {
            Uint32 fbcCurFrameIdx = *(Uint32*)param;
            Uint32 size;
            size        = WAVE4_FBC_LUMA_TABLE_SIZE(VPU_ALIGN64(pDecInfo->initialInfo.picWidth), VPU_ALIGN64(pDecInfo->initialInfo.picHeight));
            size        = VPU_ALIGN16(size);
            pDecInfo->vbFbcYTbl[fbcCurFrameIdx].phys_addr = 0;
            pDecInfo->vbFbcYTbl[fbcCurFrameIdx].size      = ((size+4095)&~4095)+4096;
            if (vdi_allocate_dma_memory(pCodecInst->coreIdx, &pDecInfo->vbFbcYTbl[fbcCurFrameIdx]) < 0)
                return RETCODE_INSUFFICIENT_RESOURCE;
        }
        break;
    case DEC_ALLOC_FBC_C_TABLE_BUFFER:
        {
            Uint32 fbcCurFrameIdx = *(Uint32*)param;
            Uint32 size;
            size        = WAVE4_FBC_CHROMA_TABLE_SIZE(VPU_ALIGN64(pDecInfo->initialInfo.picWidth), VPU_ALIGN64(pDecInfo->initialInfo.picHeight));
            size        = VPU_ALIGN16(size);
            pDecInfo->vbFbcCTbl[fbcCurFrameIdx].phys_addr = 0;
            pDecInfo->vbFbcCTbl[fbcCurFrameIdx].size      = ((size+4095)&~4095)+4096;
            if (vdi_allocate_dma_memory(pCodecInst->coreIdx, &pDecInfo->vbFbcCTbl[fbcCurFrameIdx]) < 0)
                return RETCODE_INSUFFICIENT_RESOURCE;
        }
        break;
    default:
        return RETCODE_INVALID_COMMAND;
    }

    return ret;
}

RetCode VPU_DecAllocateFrameBuffer(DecHandle handle, FrameBufferAllocInfo info, FrameBuffer *frameBuffer)
{
    CodecInst*      pCodecInst;
    DecInfo*        pDecInfo;
    RetCode         ret;
    Uint32          gdiIndex;

    ret = CheckDecInstanceValidity(handle);
    if (ret != RETCODE_SUCCESS)
        return ret;

    pCodecInst = handle;
    pDecInfo   = &pCodecInst->CodecInfo->decInfo;

    if (!frameBuffer) {
        return RETCODE_INVALID_PARAM;
    }

    if (info.type == FB_TYPE_PPU) {
        if (pDecInfo->numFrameBuffers == 0) 
            return RETCODE_WRONG_CALL_SEQUENCE;
        if (frameBuffer[0].updateFbInfo == TRUE) {
            pDecInfo->ppuAllocExt = TRUE;
        }
        pDecInfo->ppuAllocExt = frameBuffer[0].updateFbInfo;
        gdiIndex = pDecInfo->numFbsForDecoding;
        ret = ProductVpuAllocateFramebuffer(
            pCodecInst,  frameBuffer, (TiledMapType)info.mapType, (Int32)info.num, 
            info.stride, info.height, info.format, info.cbcrInterleave, info.nv21, info.endian, &pDecInfo->vbPPU, gdiIndex, FB_TYPE_PPU);
    }
    else if (info.type == FB_TYPE_CODEC) {
        gdiIndex = 0;
        if (frameBuffer[0].updateFbInfo == TRUE) {
            pDecInfo->frameAllocExt = TRUE;
        }
        ret = ProductVpuAllocateFramebuffer(
            pCodecInst, frameBuffer, (TiledMapType)info.mapType, (Int32)info.num, 
            info.stride, info.height, info.format, info.cbcrInterleave, info.nv21, info.endian, &pDecInfo->vbFrame, gdiIndex, (FramebufferAllocType)info.type);

        pDecInfo->mapCfg.tiledBaseAddr = pDecInfo->vbFrame.phys_addr;
    }

    return ret; 
}

RetCode VPU_EncOpen(EncHandle* pHandle, EncOpenParam * pop)
{
    CodecInst*  pCodecInst;
    EncInfo*    pEncInfo;
    RetCode     ret;

    if ((ret=ProductCheckEncOpenParam(pop)) != RETCODE_SUCCESS) 
        return ret;

    EnterLock(pop->coreIdx);

    if (VPU_IsInit(pop->coreIdx) == 0) {
        LeaveLock(pop->coreIdx);
        return RETCODE_NOT_INITIALIZED;
    }

    ret = GetCodecInstance(pop->coreIdx, &pCodecInst);
    if (ret == RETCODE_FAILURE) {
        *pHandle = 0;
        LeaveLock(pop->coreIdx);
        return RETCODE_FAILURE;
    }

    pCodecInst->isDecoder = FALSE;
    *pHandle = pCodecInst;
    pEncInfo = &pCodecInst->CodecInfo->encInfo;

    osal_memset(pEncInfo, 0x00, sizeof(EncInfo));
    pEncInfo->openParam = *pop;

    SetClockGate(pop->coreIdx, TRUE);
    if ((ret=ProductVpuEncBuildUpOpenParam(pCodecInst, pop)) != RETCODE_SUCCESS) {
        *pHandle = 0;
    }
    SetClockGate(pop->coreIdx, FALSE);

    LeaveLock(pCodecInst->coreIdx);

    return ret;
}

RetCode VPU_EncClose(EncHandle handle)
{
    CodecInst*  pCodecInst;
    EncInfo*    pEncInfo;
    RetCode     ret;

    ret = CheckEncInstanceValidity(handle);
    if (ret != RETCODE_SUCCESS)
        return ret;

    pCodecInst = handle;
    pEncInfo = &pCodecInst->CodecInfo->encInfo;

    EnterLock(pCodecInst->coreIdx);

    if (pEncInfo->initialInfoObtained) {
        VpuWriteReg(pCodecInst->coreIdx, pEncInfo->streamWrPtrRegAddr, pEncInfo->streamWrPtr);
        VpuWriteReg(pCodecInst->coreIdx, pEncInfo->streamRdPtrRegAddr, pEncInfo->streamRdPtr);

        if ((ret=ProductVpuEncFiniSeq(pCodecInst)) != RETCODE_SUCCESS) {
            if (pCodecInst->loggingEnable)
                vdi_log(pCodecInst->coreIdx, ENC_SEQ_END, 0);
        }
        if (pCodecInst->loggingEnable)
            vdi_log(pCodecInst->coreIdx, ENC_SEQ_END, 0);
        pEncInfo->streamWrPtr = VpuReadReg(pCodecInst->coreIdx, pEncInfo->streamWrPtrRegAddr);
    }

    if (pEncInfo->vbScratch.size) {
        vdi_free_dma_memory(pCodecInst->coreIdx, &pEncInfo->vbScratch);
    }
    
    if (pEncInfo->vbWork.size) {
        vdi_free_dma_memory(pCodecInst->coreIdx, &pEncInfo->vbWork);
    }

    if (pEncInfo->vbFrame.size) {
        if (pEncInfo->frameAllocExt == 0) 
            vdi_free_dma_memory(pCodecInst->coreIdx, &pEncInfo->vbFrame);
    }    

    if (pCodecInst->codecMode == HEVC_ENC) {
        if (pEncInfo->vbSubSamBuf.size)
            vdi_free_dma_memory(pCodecInst->coreIdx, &pEncInfo->vbSubSamBuf);   

        if (pEncInfo->vbMV.size) 
            vdi_free_dma_memory(pCodecInst->coreIdx, &pEncInfo->vbMV);

        if (pEncInfo->vbFbcYTbl.size)
            vdi_free_dma_memory(pCodecInst->coreIdx, &pEncInfo->vbFbcYTbl);

        if (pEncInfo->vbFbcCTbl.size)
            vdi_free_dma_memory(pCodecInst->coreIdx, &pEncInfo->vbFbcCTbl);

        if (pEncInfo->vbTemp.size) 
            vdi_dettach_dma_memory(pCodecInst->coreIdx, &pEncInfo->vbTemp);
    }

    if (pCodecInst->codecMode == C7_AVC_ENC) {
        if (pEncInfo->vbFbcYTbl.size)
            vdi_free_dma_memory(pCodecInst->coreIdx, &pEncInfo->vbFbcYTbl);

        if (pEncInfo->vbFbcCTbl.size)
            vdi_free_dma_memory(pCodecInst->coreIdx, &pEncInfo->vbFbcCTbl);
    }

    if (pEncInfo->vbPPU.size) {
        if (pEncInfo->ppuAllocExt == 0) 
            vdi_free_dma_memory(pCodecInst->coreIdx, &pEncInfo->vbPPU);
    }
    if (pEncInfo->vbSubSampFrame.size)
        vdi_free_dma_memory(pCodecInst->coreIdx, &pEncInfo->vbSubSampFrame);
    if (pEncInfo->vbMvcSubSampFrame.size)
        vdi_free_dma_memory(pCodecInst->coreIdx, &pEncInfo->vbMvcSubSampFrame);

    LeaveLock(pCodecInst->coreIdx);

    FreeCodecInstance(pCodecInst);

    return ret;
}

RetCode VPU_EncGetInitialInfo(EncHandle handle, EncInitialInfo * info)
{
    CodecInst*  pCodecInst;
    EncInfo*    pEncInfo;
    RetCode     ret;

    ret = CheckEncInstanceValidity(handle);
    if (ret != RETCODE_SUCCESS)
        return ret;

    if (info == 0) {
        return RETCODE_INVALID_PARAM;
    }

    pCodecInst = handle;
    pEncInfo = &pCodecInst->CodecInfo->encInfo;

    EnterLock(pCodecInst->coreIdx);

    if (GetPendingInst(pCodecInst->coreIdx)) {
        LeaveLock(pCodecInst->coreIdx);
        return RETCODE_FRAME_NOT_COMPLETE;
    }

    if ((ret=ProductVpuEncSetup(pCodecInst)) != RETCODE_SUCCESS) {
        LeaveLock(pCodecInst->coreIdx);
        return ret;
    }

    if (pCodecInst->codecMode == AVC_ENC && pCodecInst->codecModeAux == AVC_AUX_MVC)
        info->minFrameBufferCount = 3; // reconstructed frame + 2 reference frame
    else if (pCodecInst->codecMode == C7_AVC_ENC && pCodecInst->codecModeAux == AVC_AUX_MVC)
        info->minFrameBufferCount = 3; // reconstructed frame + 2 reference frame
    else if(pCodecInst->codecMode == HEVC_ENC) {
        info->minFrameBufferCount   = pEncInfo->initialInfo.minFrameBufferCount;
        info->minSrcFrameCount      = pEncInfo->initialInfo.minSrcFrameCount;
    }
    else
        info->minFrameBufferCount = 2; // reconstructed frame + reference frame

    pEncInfo->initialInfo         = *info;
    pEncInfo->initialInfoObtained = TRUE;

    LeaveLock(pCodecInst->coreIdx);

    return RETCODE_SUCCESS;
}

RetCode VPU_EncRegisterFrameBuffer(EncHandle handle, FrameBuffer* bufArray, int num, int stride, int height, int mapType)
{
    CodecInst*      pCodecInst;
    EncInfo*        pEncInfo;
    Int32           i;
    RetCode         ret;
    EncOpenParam*   openParam;
    FrameBuffer*    fb;

    ret = CheckEncInstanceValidity(handle);
    // FIXME temp
    if (ret != RETCODE_SUCCESS)
        return ret;

    pCodecInst = handle;
    pEncInfo = &pCodecInst->CodecInfo->encInfo;
    openParam = &pEncInfo->openParam;

    if (pEncInfo->stride) 
        return RETCODE_CALLED_BEFORE;

    if (!pEncInfo->initialInfoObtained) 
        return RETCODE_WRONG_CALL_SEQUENCE;

    if (num < pEncInfo->initialInfo.minFrameBufferCount) 
        return RETCODE_INSUFFICIENT_FRAME_BUFFERS;

    if (stride == 0 || (stride % 8 != 0) || stride < 0) 
        return RETCODE_INVALID_STRIDE;

    if (height == 0 || height < 0)
        return RETCODE_INVALID_PARAM;

    if (openParam->bitstreamFormat == STD_HEVC) {
        if (stride % 32 != 0)
            return RETCODE_INVALID_STRIDE;
    }

    EnterLock(pCodecInst->coreIdx);

    if (GetPendingInst(pCodecInst->coreIdx)) {
        LeaveLock(pCodecInst->coreIdx);
        return RETCODE_FRAME_NOT_COMPLETE;
    }

    pEncInfo->numFrameBuffers   = num;
    pEncInfo->stride            = stride;
    pEncInfo->frameBufferHeight = height;
    pEncInfo->mapType           = mapType;
    pEncInfo->mapCfg.productId  = pCodecInst->productId;

    if (bufArray) {
        for(i=0; i<num; i++)
            pEncInfo->frameBufPool[i] = bufArray[i];
    }

    if (pEncInfo->frameAllocExt == FALSE) {
        fb = pEncInfo->frameBufPool;
        if (bufArray) {
            if (bufArray[0].bufCb == (Uint32)-1 && bufArray[0].bufCr == (Uint32)-1) {
                Uint32 size;
                pEncInfo->frameAllocExt = TRUE;
                size = ProductCalculateFrameBufSize(pCodecInst->productId, stride, height, 
                                                    (TiledMapType)mapType, (FrameBufferFormat)openParam->srcFormat, 
                                                    (BOOL)openParam->cbcrInterleave, NULL);
                if (mapType == LINEAR_FRAME_MAP)
                {
                    pEncInfo->vbFrame.phys_addr = bufArray[0].bufY;
                    pEncInfo->vbFrame.size      = size * num;
                }
            }
        }
        ret = ProductVpuAllocateFramebuffer(
            pCodecInst, fb, (TiledMapType)mapType, num, stride, height, (FrameBufferFormat)openParam->srcFormat,
            openParam->cbcrInterleave, FALSE, openParam->frameEndian, &pEncInfo->vbFrame, 0, FB_TYPE_CODEC);
        if (ret != RETCODE_SUCCESS) {
            SetPendingInst(pCodecInst->coreIdx, 0);
            LeaveLock(pCodecInst->coreIdx);
            return ret;
        }
    }
    ret = ProductVpuRegisterFramebuffer(pCodecInst);

    SetPendingInst(pCodecInst->coreIdx, 0);

    LeaveLock(pCodecInst->coreIdx);

    return ret;
}

RetCode VPU_EncGetFrameBuffer(
    EncHandle     handle, 
    int           frameIdx, 
    FrameBuffer * frameBuf)
{
    CodecInst * pCodecInst;
    EncInfo * pEncInfo;
    RetCode ret;

    ret = CheckEncInstanceValidity(handle);
    if (ret != RETCODE_SUCCESS)
        return ret;

    pCodecInst = handle;
    pEncInfo = &pCodecInst->CodecInfo->encInfo;

    if (frameIdx<0 || frameIdx>pEncInfo->numFrameBuffers)
        return RETCODE_INVALID_PARAM;

    if (frameBuf==0)
        return RETCODE_INVALID_PARAM;

    *frameBuf = pEncInfo->frameBufPool[frameIdx];

    return RETCODE_SUCCESS;

}

RetCode VPU_EncGetBitstreamBuffer( EncHandle handle,
    PhysicalAddress * prdPrt,
    PhysicalAddress * pwrPtr,
    int * size)
{
    CodecInst * pCodecInst;
    EncInfo * pEncInfo;
    PhysicalAddress rdPtr;
    PhysicalAddress wrPtr;
    Uint32 room;
    RetCode ret;

    ret = CheckEncInstanceValidity(handle);
    if (ret != RETCODE_SUCCESS)
        return ret;

    if ( prdPrt == 0 || pwrPtr == 0 || size == 0) {
        return RETCODE_INVALID_PARAM;
    }

    pCodecInst = handle;
    pEncInfo = &pCodecInst->CodecInfo->encInfo;

    rdPtr = pEncInfo->streamRdPtr;

    SetClockGate(pCodecInst->coreIdx, 1);

    if (GetPendingInst(pCodecInst->coreIdx) == pCodecInst)
        wrPtr = VpuReadReg(pCodecInst->coreIdx, pEncInfo->streamWrPtrRegAddr);
    else
        wrPtr = pEncInfo->streamWrPtr;

    SetClockGate(pCodecInst->coreIdx, 0);
    if(pEncInfo->ringBufferEnable == 1 || pEncInfo->lineBufIntEn == 1) {
        if (wrPtr >= rdPtr) {
            room = wrPtr - rdPtr;
        }
        else {
            room = (pEncInfo->streamBufEndAddr - rdPtr) + (wrPtr - pEncInfo->streamBufStartAddr);
        }
    }
    else {
        if(wrPtr >= rdPtr)
            room = wrPtr - rdPtr;    
        else
            return RETCODE_INVALID_PARAM;
    }

    *prdPrt = rdPtr;
    *pwrPtr = wrPtr;
    *size = room;

    return RETCODE_SUCCESS;
}

RetCode VPU_EncUpdateBitstreamBuffer(
    EncHandle handle,
    int size)
{
    CodecInst * pCodecInst;
    EncInfo * pEncInfo;
    PhysicalAddress wrPtr;
    PhysicalAddress rdPtr;
    RetCode ret;
    int        room = 0;
    ret = CheckEncInstanceValidity(handle);
    if (ret != RETCODE_SUCCESS)
        return ret;

    pCodecInst = handle;
    pEncInfo = &pCodecInst->CodecInfo->encInfo;

    rdPtr = pEncInfo->streamRdPtr;

    SetClockGate(pCodecInst->coreIdx, 1);

    if (GetPendingInst(pCodecInst->coreIdx) == pCodecInst)
        wrPtr = VpuReadReg(pCodecInst->coreIdx, pEncInfo->streamWrPtrRegAddr);
    else
        wrPtr = pEncInfo->streamWrPtr;

    if ( rdPtr < wrPtr ) {
        if ( rdPtr + size  > wrPtr ) {
            SetClockGate(pCodecInst->coreIdx, 0);
            return RETCODE_INVALID_PARAM;
        }
    }

    if (pEncInfo->ringBufferEnable == TRUE || pEncInfo->lineBufIntEn == TRUE) {
        rdPtr += size;
        if (rdPtr > pEncInfo->streamBufEndAddr) {
            if (pEncInfo->lineBufIntEn == TRUE) {
                return RETCODE_INVALID_PARAM;
            }
            room = rdPtr - pEncInfo->streamBufEndAddr;
            rdPtr = pEncInfo->streamBufStartAddr;
            rdPtr += room;
        }

        if (rdPtr == pEncInfo->streamBufEndAddr) {
            rdPtr = pEncInfo->streamBufStartAddr;
        }
    }
    else {
        rdPtr = pEncInfo->streamBufStartAddr;
    }

    pEncInfo->streamRdPtr = rdPtr;
    pEncInfo->streamWrPtr = wrPtr;
    if (GetPendingInst(pCodecInst->coreIdx) == pCodecInst)
        VpuWriteReg(pCodecInst->coreIdx, pEncInfo->streamRdPtrRegAddr, rdPtr);

    if (pEncInfo->lineBufIntEn == TRUE) {
        pEncInfo->streamRdPtr = pEncInfo->streamBufStartAddr;
    }

    SetClockGate(pCodecInst->coreIdx, 0);
    return RETCODE_SUCCESS;
}

RetCode VPU_EncStartOneFrame(
    EncHandle handle,
    EncParam * param 
    )
{
    CodecInst*          pCodecInst;
    EncInfo*            pEncInfo;
    RetCode             ret;
    VpuAttr*            pAttr   = NULL;
    vpu_instance_pool_t* vip;

    ret = CheckEncInstanceValidity(handle);
    if (ret != RETCODE_SUCCESS)
        return ret;

    pCodecInst = handle;
    pEncInfo   = &pCodecInst->CodecInfo->encInfo;
    vip        = (vpu_instance_pool_t *)vdi_get_instance_pool(pCodecInst->coreIdx);
    if (!vip) {
        return RETCODE_INVALID_HANDLE;
    }

    if (pEncInfo->stride == 0) { // This means frame buffers have not been registered.
        return RETCODE_WRONG_CALL_SEQUENCE;
    }

    ret = CheckEncParam(handle, param);
    if (ret != RETCODE_SUCCESS) {
        return ret;
    }

    pAttr = &g_VpuCoreAttributes[pCodecInst->coreIdx];

    EnterLock(pCodecInst->coreIdx);

    pEncInfo->ptsMap[param->srcIdx] = (pEncInfo->openParam.enablePTS == TRUE) ? GetTimestamp(handle) : param->pts;

    if (GetPendingInst(pCodecInst->coreIdx)) {
        LeaveLock(pCodecInst->coreIdx);
        return RETCODE_FRAME_NOT_COMPLETE;
    }

    ret = ProductVpuEncode(pCodecInst, param);

    if (pAttr->supportCommandQueue == TRUE) {
        SetPendingInst(pCodecInst->coreIdx, NULL);
        LeaveLock(pCodecInst->coreIdx);
    }
    else {
        SetPendingInst(pCodecInst->coreIdx, pCodecInst);
    }

    return ret;
}

RetCode VPU_EncGetOutputInfo(
    EncHandle       handle,
    EncOutputInfo*  info
    )
{
    CodecInst*  pCodecInst;
    EncInfo*    pEncInfo;
    RetCode     ret;
    VpuAttr*    pAttr;

    ret = CheckEncInstanceValidity(handle);
    if (ret != RETCODE_SUCCESS) {
        return ret;
    }

    if (info == 0) {
        return RETCODE_INVALID_PARAM;
    }

    pCodecInst = handle;
    pEncInfo   = &pCodecInst->CodecInfo->encInfo;
    pAttr      = &g_VpuCoreAttributes[pCodecInst->coreIdx];

    if (pAttr->supportCommandQueue == TRUE) {
        EnterLock(pCodecInst->coreIdx);
    }
    else {
        if (pCodecInst != GetPendingInst(pCodecInst->coreIdx)) {
            SetPendingInst(pCodecInst->coreIdx, 0);
            LeaveLock(pCodecInst->coreIdx);
            return RETCODE_WRONG_CALL_SEQUENCE;
        }
    }

    ret = ProductVpuEncGetResult(pCodecInst, info);

    if (ret == RETCODE_SUCCESS) {
        info->pts = pEncInfo->ptsMap[info->encSrcIdx];
    }
    else {
        info->pts = 0LL;
    }

    SetPendingInst(pCodecInst->coreIdx, 0);
    LeaveLock(pCodecInst->coreIdx);

    return ret;
}

RetCode VPU_EncGiveCommand(
    EncHandle       handle,
    CodecCommand    cmd,
    void*           param
    )
{
    CodecInst*  pCodecInst;
    EncInfo*    pEncInfo;
    RetCode     ret;

    ret = CheckEncInstanceValidity(handle);
    if (ret != RETCODE_SUCCESS) {
        return ret;
    }

    pCodecInst = handle;
    pEncInfo = &pCodecInst->CodecInfo->encInfo;
    switch (cmd) 
    {
    case ENABLE_ROTATION :
        {
            pEncInfo->rotationEnable = 1;            
        }
        break;        
    case DISABLE_ROTATION :
        {
            pEncInfo->rotationEnable = 0;
        }
        break;
    case ENABLE_MIRRORING :
        {
            pEncInfo->mirrorEnable = 1;
        }
        break;
    case DISABLE_MIRRORING :
        {
            pEncInfo->mirrorEnable = 0;            
        }
        break;
    case SET_MIRROR_DIRECTION :
        {    
            MirrorDirection mirDir;

            if (param == 0) {
                return RETCODE_INVALID_PARAM;
            }
            mirDir = *(MirrorDirection *)param;
            if ( !(mirDir == MIRDIR_NONE) && !(mirDir==MIRDIR_HOR) && !(mirDir==MIRDIR_VER) && !(mirDir==MIRDIR_HOR_VER)) {
                return RETCODE_INVALID_PARAM;
            }
            pEncInfo->mirrorDirection = mirDir;
        }
        break;        
    case SET_ROTATION_ANGLE :
        {
            int angle;

            if (param == 0) {
                return RETCODE_INVALID_PARAM;
            }
            angle = *(int *)param;
            if (angle != 0 && angle != 90 &&
                angle != 180 && angle != 270) {
                    return RETCODE_INVALID_PARAM;
            }
            if (pEncInfo->initialInfoObtained && (angle == 90 || angle ==270)) {
                return RETCODE_INVALID_PARAM;
            }
            pEncInfo->rotationAngle = angle;            
        }
        break;
    case SET_CACHE_CONFIG:
        {
            MaverickCacheConfig *mcCacheConfig;
            if (param == 0) {
                return RETCODE_INVALID_PARAM;
            }
            mcCacheConfig = (MaverickCacheConfig *)param;
            pEncInfo->cacheConfig = *mcCacheConfig;
        }
        break;
    case ENC_PUT_MP4_HEADER:
    case ENC_PUT_AVC_HEADER:
    case ENC_PUT_VIDEO_HEADER:
        {
            EncHeaderParam *encHeaderParam;

            if (param == 0) {
                return RETCODE_INVALID_PARAM;
            }                
            encHeaderParam = (EncHeaderParam *)param;
            if (pCodecInst->codecMode == MP4_ENC || pCodecInst->codecMode == C7_MP4_ENC ) {
                if (!( VOL_HEADER<=encHeaderParam->headerType && encHeaderParam->headerType <= VIS_HEADER)) {
                    return RETCODE_INVALID_PARAM;
                }
            } 
            else if  (pCodecInst->codecMode == AVC_ENC || pCodecInst->codecMode == C7_AVC_ENC) {
                if (!( SPS_RBSP<=encHeaderParam->headerType && encHeaderParam->headerType <= SVC_RBSP_SEI)) {
                    return RETCODE_INVALID_PARAM;
                }
            }
            else if (pCodecInst->codecMode == HEVC_ENC) {
                if (!( CODEOPT_ENC_VPS<=encHeaderParam->headerType && encHeaderParam->headerType <= (CODEOPT_ENC_VPS|CODEOPT_ENC_SPS|CODEOPT_ENC_PPS))) {
                    return RETCODE_INVALID_PARAM;
                }
                if (pEncInfo->ringBufferEnable == 0 ) {
                    if (encHeaderParam->buf % 16 || encHeaderParam->size == 0) 
                        return RETCODE_INVALID_PARAM;
                }
                if (encHeaderParam->headerType & CODEOPT_ENC_VCL)   // ENC_PUT_VIDEO_HEADER encode only non-vcl header.
                    return RETCODE_INVALID_PARAM;

            }
            else
                return RETCODE_INVALID_PARAM;

            if (pEncInfo->ringBufferEnable == 0 ) {
                if (encHeaderParam->buf % 8 || encHeaderParam->size == 0) {
                    return RETCODE_INVALID_PARAM;
                }
            }
            if (pCodecInst->codecMode == HEVC_ENC) {
                if (handle->productId == PRODUCT_ID_520)
                    return Wave5VpuEncGetHeader(handle, encHeaderParam);
                else
                    return Wave4VpuEncGetHeader(handle, encHeaderParam);
            }
            else {
                if (handle->productId == PRODUCT_ID_7Q) 
                    return Coda7qVpuEncGetHeader(handle, encHeaderParam);
                else 
                    return GetEncHeader(handle, encHeaderParam);
            }
        }        
    case ENC_SET_GOP_NUMBER:
        {
            int *pGopNumber =(int *)param;
            if (pCodecInst->codecMode != MP4_ENC && pCodecInst->codecMode != AVC_ENC && pCodecInst->codecMode != C7_MP4_ENC && pCodecInst->codecMode != C7_AVC_ENC ) {
                return RETCODE_INVALID_COMMAND;
            }
            if (*pGopNumber < 0)
                return RETCODE_INVALID_PARAM;
            pEncInfo->openParam.gopSize = *pGopNumber;
            SetGopNumber(handle, (Uint32 *)pGopNumber);        
        }
        break;
    case ENC_SET_INTRA_QP:
        {
            int *pIntraQp =(int *)param;
            if (pCodecInst->codecMode != MP4_ENC && pCodecInst->codecMode != AVC_ENC && pCodecInst->codecMode != C7_MP4_ENC && pCodecInst->codecMode != C7_AVC_ENC) {
                return RETCODE_INVALID_COMMAND;
            }
            if (pCodecInst->codecMode == MP4_ENC || pCodecInst->codecMode == C7_MP4_ENC)
            {    
                if(*pIntraQp<1 || *pIntraQp>31)
                    return RETCODE_INVALID_PARAM;
            }
            if (pCodecInst->codecMode == AVC_ENC || pCodecInst->codecMode == C7_AVC_ENC)
            {    
                if(*pIntraQp<0 || *pIntraQp>51)
                    return RETCODE_INVALID_PARAM;
            }
            SetIntraQp(handle, (Uint32 *)pIntraQp);        
        }
        break;
    case ENC_SET_BITRATE:
        {
            int *pBitrate = (int *)param;
            if (pCodecInst->codecMode != MP4_ENC && pCodecInst->codecMode != AVC_ENC && pCodecInst->codecMode != C7_MP4_ENC && pCodecInst->codecMode != C7_AVC_ENC) {
                return RETCODE_INVALID_COMMAND;
            }
            {
                if (*pBitrate < 0 || *pBitrate> 32767) {
                    return RETCODE_INVALID_PARAM;
                }
            }
            SetBitrate(handle, (Uint32 *)pBitrate);        
        }
        break;
    case ENC_SET_FRAME_RATE:
        {
            int *pFramerate = (int *)param;

            if (pCodecInst->codecMode != MP4_ENC && pCodecInst->codecMode != AVC_ENC && pCodecInst->codecMode != C7_MP4_ENC && pCodecInst->codecMode != C7_AVC_ENC) {
                return RETCODE_INVALID_COMMAND;
            }
            if (*pFramerate <= 0) {
                return RETCODE_INVALID_PARAM;
            }
            SetFramerate(handle, (Uint32 *)pFramerate);        
        }
        break;
    case ENC_SET_INTRA_MB_REFRESH_NUMBER:
        {
            int *pIntraRefreshNum =(int *)param;
            SetIntraRefreshNum(handle, (Uint32 *)pIntraRefreshNum); 
        }
        break;

    case ENC_SET_SLICE_INFO:
        {
            EncSliceMode *pSliceMode = (EncSliceMode *)param;
            if(pSliceMode->sliceMode<0 || pSliceMode->sliceMode>1)
            {
                return RETCODE_INVALID_PARAM;
            }
            if(pSliceMode->sliceSizeMode<0 || pSliceMode->sliceSizeMode>1)
            {
                return RETCODE_INVALID_PARAM;
            }

            SetSliceMode(handle, (EncSliceMode *)pSliceMode);
        }
        break;
    case ENC_ENABLE_HEC:
        {
            if (pCodecInst->codecMode != MP4_ENC && pCodecInst->codecMode != C7_MP4_ENC) {
                return RETCODE_INVALID_COMMAND;
            }
            SetHecMode(handle, 1);
        }
        break;
    case ENC_DISABLE_HEC:
        {
            if (pCodecInst->codecMode != MP4_ENC && pCodecInst->codecMode != C7_MP4_ENC) {
                return RETCODE_INVALID_COMMAND;
            }
            SetHecMode(handle, 0);
        }
        break;
    case SET_SEC_AXI:
        {
            SecAxiUse secAxiUse;

            if (param == 0) {
                return RETCODE_INVALID_PARAM;
            }
            secAxiUse = *(SecAxiUse *)param;
            if (handle->productId == PRODUCT_ID_410 || handle->productId == PRODUCT_ID_4102 || handle->productId == PRODUCT_ID_420 ||
                handle->productId == PRODUCT_ID_412  || handle->productId == PRODUCT_ID_420L || handle->productId == PRODUCT_ID_510 || 
                handle->productId == PRODUCT_ID_512 || handle->productId == PRODUCT_ID_515) {
                if (handle->codecMode == HEVC_DEC) {
                    pEncInfo->secAxiInfo.u.wave4.useIpEnable    = secAxiUse.u.wave4.useIpEnable;
                    pEncInfo->secAxiInfo.u.wave4.useLfRowEnable = secAxiUse.u.wave4.useLfRowEnable;
                    pEncInfo->secAxiInfo.u.wave4.useBitEnable   = secAxiUse.u.wave4.useBitEnable;
                }
                else {
                    pEncInfo->secAxiInfo.u.wave4.useEncImdEnable = secAxiUse.u.wave4.useEncImdEnable;
                    pEncInfo->secAxiInfo.u.wave4.useEncRdoEnable = secAxiUse.u.wave4.useEncRdoEnable;
                    pEncInfo->secAxiInfo.u.wave4.useEncLfEnable  = secAxiUse.u.wave4.useEncLfEnable;
                }
            } 
            else if (handle->productId == PRODUCT_ID_520) {
                pEncInfo->secAxiInfo.u.wave4.useEncRdoEnable = secAxiUse.u.wave4.useEncRdoEnable;
                pEncInfo->secAxiInfo.u.wave4.useEncLfEnable  = secAxiUse.u.wave4.useEncLfEnable;
            }
            else { // coda9 or coda7q or ... 
                pEncInfo->secAxiInfo.u.coda9.useBitEnable  = secAxiUse.u.coda9.useBitEnable;
                pEncInfo->secAxiInfo.u.coda9.useIpEnable   = secAxiUse.u.coda9.useIpEnable;
                pEncInfo->secAxiInfo.u.coda9.useDbkYEnable = secAxiUse.u.coda9.useDbkYEnable;
                pEncInfo->secAxiInfo.u.coda9.useDbkCEnable = secAxiUse.u.coda9.useDbkCEnable;
                pEncInfo->secAxiInfo.u.coda9.useOvlEnable  = secAxiUse.u.coda9.useOvlEnable;
                pEncInfo->secAxiInfo.u.coda9.useBtpEnable  = secAxiUse.u.coda9.useBtpEnable;            
            }        
        }
        break;        
    case GET_TILEDMAP_CONFIG:
        {
            TiledMapConfig *pMapCfg = (TiledMapConfig *)param;
            if (!pMapCfg) {
                return RETCODE_INVALID_PARAM;
            }
            *pMapCfg = pEncInfo->mapCfg;
            break;
        }
    case SET_DRAM_CONFIG:
        {
            DRAMConfig *cfg = (DRAMConfig *)param;

            if (!cfg) {
                return RETCODE_INVALID_PARAM;
            }

            pEncInfo->dramCfg = *cfg;
            break;
        }
    case GET_DRAM_CONFIG:
        {
            DRAMConfig *cfg = (DRAMConfig *)param;

            if (!cfg) {
                return RETCODE_INVALID_PARAM;
            }

            *cfg = pEncInfo->dramCfg;

            break;
        }
    case ENABLE_LOGGING:
        {
            pCodecInst->loggingEnable = 1;            
        }
        break;
    case DISABLE_LOGGING:
        {
            pCodecInst->loggingEnable = 0;
        }
        break;
    case ENC_SET_PARA_CHANGE:
        {
            EncChangeParam* option = (EncChangeParam*)param;
            if (handle->productId == PRODUCT_ID_520)
                return Wave5VpuEncParaChange(handle, option);
            else
                return Wave4VpuEncParaChange(handle, option);
        }
        break;
    case ENC_SET_SEI_NAL_DATA:
        {
            HevcSEIDataEnc* option = (HevcSEIDataEnc*)param;
            pEncInfo->prefixSeiNalEnable    = option->prefixSeiNalEnable;
            pEncInfo->prefixSeiDataSize     = option->prefixSeiDataSize;
            pEncInfo->prefixSeiDataEncOrder = option->prefixSeiDataEncOrder;
            pEncInfo->prefixSeiNalAddr      = option->prefixSeiNalAddr;

            pEncInfo->suffixSeiNalEnable    = option->suffixSeiNalEnable;
            pEncInfo->suffixSeiDataSize     = option->suffixSeiDataSize;
            pEncInfo->suffixSeiDataEncOrder = option->suffixSeiDataEncOrder;
            pEncInfo->suffixSeiNalAddr      = option->suffixSeiNalAddr;
        }
        break;
#ifdef SUPPORT_W5ENC_BW_REPORT
    case ENC_GET_BW_REPORT :
        {
            EncBwMonitor *encBwMon;

            if (param == 0) {
                return RETCODE_INVALID_PARAM;
            }                

            if (handle->productId != PRODUCT_ID_520)
                return RETCODE_INVALID_COMMAND;

            encBwMon = (EncBwMonitor *)param;
            return Wave5VpuEncGetBwReport(handle, encBwMon);
        }
        break;
#endif
    default:
        return RETCODE_INVALID_COMMAND;
    }
    return RETCODE_SUCCESS;
}

RetCode VPU_EncAllocateFrameBuffer(EncHandle handle, FrameBufferAllocInfo info, FrameBuffer *frameBuffer)
{
    CodecInst*  pCodecInst;
    EncInfo*    pEncInfo;
    RetCode     ret;
    int         gdiIndex;

    ret = CheckEncInstanceValidity(handle);
    if (ret != RETCODE_SUCCESS)
        return ret;

    pCodecInst = handle;
    pEncInfo   = &pCodecInst->CodecInfo->encInfo;

    if (!frameBuffer) {
        return RETCODE_INVALID_PARAM;
    }
    if (info.num == 0 || info.num < 0)  {
        return RETCODE_INVALID_PARAM;
    }
    if (info.stride == 0 || info.stride < 0) {
        return RETCODE_INVALID_PARAM;
    }
    if (info.height == 0 || info.height < 0) {
        return RETCODE_INVALID_PARAM;
    }

    if (info.type == FB_TYPE_PPU) {
        if (pEncInfo->numFrameBuffers == 0) {
            return RETCODE_WRONG_CALL_SEQUENCE;
        }
        pEncInfo->ppuAllocExt = frameBuffer[0].updateFbInfo;
        gdiIndex = pEncInfo->numFrameBuffers;        
        ret = ProductVpuAllocateFramebuffer(pCodecInst, frameBuffer, (TiledMapType)info.mapType, (Int32)info.num, 
                                            info.stride, info.height, info.format, info.cbcrInterleave, info.nv21, 
                                            info.endian, &pEncInfo->vbPPU, gdiIndex, (FramebufferAllocType)info.type);
    }
    else if (info.type == FB_TYPE_CODEC) {
        gdiIndex = 0;
        pEncInfo->frameAllocExt = frameBuffer[0].updateFbInfo;
        ret = ProductVpuAllocateFramebuffer(
            pCodecInst, frameBuffer, (TiledMapType)info.mapType, (Int32)info.num, 
            info.stride, info.height, info.format, info.cbcrInterleave, FALSE, info.endian, &pEncInfo->vbFrame, gdiIndex, (FramebufferAllocType)info.type);
    }
    else {
        ret = RETCODE_INVALID_PARAM;
    }

    return ret; 
}

RetCode VPU_EncIssueSeqInit(EncHandle handle)
{
    CodecInst*  pCodecInst;
    RetCode     ret;
    VpuAttr*    pAttr;

    ret = CheckEncInstanceValidity(handle);
    if (ret != RETCODE_SUCCESS)
        return ret;

    pCodecInst = handle;

    EnterLock(pCodecInst->coreIdx);

    pAttr = &g_VpuCoreAttributes[pCodecInst->coreIdx];

    if (GetPendingInst(pCodecInst->coreIdx)) {
        LeaveLock(pCodecInst->coreIdx);
        return RETCODE_FRAME_NOT_COMPLETE;
    }

    ret = ProductVpuEncInitSeq(handle);
    if (ret == RETCODE_SUCCESS) {
        SetPendingInst(pCodecInst->coreIdx, pCodecInst);
    }

    if (pAttr->supportCommandQueue == TRUE) {
        SetPendingInst(pCodecInst->coreIdx, NULL);
        LeaveLock(pCodecInst->coreIdx);
    }

    return ret;
}

RetCode VPU_EncCompleteSeqInit(EncHandle handle, EncInitialInfo * info)
{
    CodecInst*  pCodecInst;
    EncInfo*    pEncInfo;
    RetCode     ret;
    VpuAttr*    pAttr;

    ret = CheckEncInstanceValidity(handle);
    if (ret != RETCODE_SUCCESS) {
        return ret;
    }

    if (info == 0) {
        return RETCODE_INVALID_PARAM;
    }

    pCodecInst = handle;
    pEncInfo = &pCodecInst->CodecInfo->encInfo;

    pAttr = &g_VpuCoreAttributes[pCodecInst->coreIdx];

    if (pAttr->supportCommandQueue == TRUE) {
        EnterLock(pCodecInst->coreIdx);
    }
    else {
        if (pCodecInst != GetPendingInst(pCodecInst->coreIdx)) {
            SetPendingInst(pCodecInst->coreIdx, 0);
            LeaveLock(pCodecInst->coreIdx);    
            return RETCODE_WRONG_CALL_SEQUENCE;
        }
    }

    ret = ProductVpuEncGetSeqInfo(handle, info);
    if (ret == RETCODE_SUCCESS) {
        pEncInfo->initialInfoObtained = 1;
    }

    //info->rdPtr = VpuReadReg(pCodecInst->coreIdx, pEncInfo->streamRdPtrRegAddr);
    //info->wrPtr = VpuReadReg(pCodecInst->coreIdx, pEncInfo->streamWrPtrRegAddr);

    //pEncInfo->prevFrameEndPos = info->rdPtr;

    pEncInfo->initialInfo = *info;

    SetPendingInst(pCodecInst->coreIdx, NULL);

    LeaveLock(pCodecInst->coreIdx);

    return ret;
}

 
