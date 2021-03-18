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

#include <string.h>
#include "component.h"

typedef enum {
    SRC_FB_TYPE_YUV,
    SRC_FB_TYPE_CFRAME
} SRC_FB_TYPE;

typedef enum {
    YUV_FEEDER_STATE_WAIT,
    YUV_FEEDER_STATE_FEEDING
} YuvFeederState;

typedef struct {
    EncHandle                   handle;
    char                        inputPath[MAX_FILE_PATH];
    EncOpenParam                encOpenParam;
    Int32                       rotAngle;
    Int32                       mirDir;
    Int32                       frameOutNum;
    Int32                       yuvMode;
    FrameBufferAllocInfo        srcFbAllocInfo;
    FrameBufferAllocInfo        reconFbAllocInfo;
    BOOL                        fbAllocated;
    ParamEncNeedFrameBufferNum  fbCount;
    FrameBuffer                 pFbRecon[MAX_REG_FRAME];
    vpu_buffer_t                pFbReconMem[MAX_REG_FRAME];
    FrameBuffer                 pFbSrc[ENC_SRC_BUF_NUM];
    vpu_buffer_t                pFbSrcMem[ENC_SRC_BUF_NUM];
    FrameBuffer                 pFbOffsetTbl[ENC_SRC_BUF_NUM];
    vpu_buffer_t                pFbOffsetTblMem[ENC_SRC_BUF_NUM];
    YuvFeeder                   yuvFeeder;
    BOOL                        last;
    YuvFeederState              state;
    BOOL                        stateDoing;
    Int32                       feedingNum;
    TiledMapConfig              mapConfig;
    Int32                       productID;
} YuvFeederContext;

static void InitYuvFeederContext(YuvFeederContext* ctx, CNMComponentConfig* componentParam) 
{
    osal_memset((void*)ctx, 0, sizeof(YuvFeederContext));
    strcpy(ctx->inputPath, componentParam->testEncConfig.yuvFileName);

    ctx->fbAllocated        = FALSE;
    ctx->encOpenParam       = componentParam->encOpenParam;
    ctx->yuvMode            = componentParam->testEncConfig.yuv_mode;
    ctx->fbCount.reconFbNum = 0;
    ctx->fbCount.srcFbNum   = 0;
    ctx->frameOutNum        = componentParam->testEncConfig.outNum;
    ctx->rotAngle           = componentParam->testEncConfig.rotAngle;
    ctx->mirDir             = componentParam->testEncConfig.mirDir;

    osal_memset(&ctx->srcFbAllocInfo,          0x00, sizeof(FrameBufferAllocInfo));
    osal_memset(&ctx->reconFbAllocInfo,        0x00, sizeof(FrameBufferAllocInfo));
    osal_memset((void*)ctx->pFbRecon,          0x00, sizeof(ctx->pFbRecon));
    osal_memset((void*)ctx->pFbReconMem,       0x00, sizeof(ctx->pFbReconMem));
    osal_memset((void*)ctx->pFbSrc,            0x00, sizeof(ctx->pFbSrc));
    osal_memset((void*)ctx->pFbSrcMem,         0x00, sizeof(ctx->pFbSrcMem));
    osal_memset((void*)ctx->pFbOffsetTbl,      0x00, sizeof(ctx->pFbOffsetTbl));
    osal_memset((void*)ctx->pFbOffsetTblMem,   0x00, sizeof(ctx->pFbOffsetTblMem));
}

static CNMComponentParamRet GetParameterYuvFeeder(ComponentImpl* from, ComponentImpl* com, GetParameterCMD commandType, void* data) 
{
    YuvFeederContext*    ctx = (YuvFeederContext*)com->context;
    BOOL                 result  = TRUE;
    ParamEncFrameBuffer* allocFb = NULL;

    switch(commandType) {
    case GET_PARAM_YUVFEEDER_FRAME_BUF:
        if (ctx->fbAllocated == FALSE) return CNM_COMPONENT_PARAM_NOT_READY;
        allocFb = (ParamEncFrameBuffer*)data;
        allocFb->reconFb          = ctx->pFbRecon;
        allocFb->srcFb            = ctx->pFbSrc;
        allocFb->reconFbAllocInfo = ctx->reconFbAllocInfo;
        allocFb->srcFbAllocInfo   = ctx->srcFbAllocInfo;
        break;
    default:
        return CNM_COMPONENT_PARAM_NOT_FOUND;
    }

    return (result == TRUE) ? CNM_COMPONENT_PARAM_SUCCESS : CNM_COMPONENT_PARAM_FAILURE;
}

static CNMComponentParamRet SetParameterYuvFeeder(ComponentImpl* from, ComponentImpl* com, SetParameterCMD commandType, void* data) 
{
    BOOL result = TRUE;

    switch(commandType) {
    default:
        VLOG(ERR, "Unknown SetParameterCMD Type : %d\n", commandType);
        result = FALSE;
        break;
    }

    return (result == TRUE) ? CNM_COMPONENT_PARAM_SUCCESS : CNM_COMPONENT_PARAM_FAILURE;
}

static BOOL AllocateFrameBuffer(ComponentImpl* com) {
    YuvFeederContext*       ctx = (YuvFeederContext*)com->context;
    EncOpenParam            encOpenParam = ctx->encOpenParam;
    Uint32                  fbWidth = 0;
    Uint32                  fbHeight = 0;
    Uint32                  fbStride = 0;
    Uint32                  fbSize = 0;
    SRC_FB_TYPE             srcFbType = SRC_FB_TYPE_YUV;
    DRAMConfig              dramConfig;
    DRAMConfig*             pDramConfig = NULL;
    TiledMapType            mapType;
    FrameBufferAllocInfo    fbAllocInfo;

    osal_memset(&fbAllocInfo, 0x00, sizeof(FrameBufferAllocInfo));
    osal_memset(&dramConfig, 0x00, sizeof(DRAMConfig));

    //Buffers for source frames
    if (PRODUCT_ID_W_SERIES(ctx->productID)) {
        fbWidth = VPU_ALIGN8(encOpenParam.picWidth);
        fbHeight = VPU_ALIGN8(encOpenParam.picHeight);
        fbAllocInfo.endian  = encOpenParam.sourceEndian;
    } else {
        //CODA
        fbWidth = VPU_ALIGN16(encOpenParam.picWidth);
        fbHeight = VPU_ALIGN16(encOpenParam.picHeight);
        fbAllocInfo.endian  = encOpenParam.frameEndian;
        VPU_EncGiveCommand(ctx->handle, GET_DRAM_CONFIG, &dramConfig);
        pDramConfig = &dramConfig;
    }


    if (SRC_FB_TYPE_YUV == srcFbType) {
        mapType = LINEAR_FRAME_MAP;
    }
    fbStride = CalcStride(fbWidth, fbHeight, (FrameBufferFormat)encOpenParam.srcFormat, encOpenParam.cbcrInterleave, mapType, FALSE);
    fbSize = VPU_GetFrameBufSize(ctx->handle, encOpenParam.coreIdx, fbStride, fbHeight, mapType, (FrameBufferFormat)encOpenParam.srcFormat, encOpenParam.cbcrInterleave, pDramConfig);

    fbAllocInfo.format  = (FrameBufferFormat)encOpenParam.srcFormat;
    fbAllocInfo.cbcrInterleave = encOpenParam.cbcrInterleave;
    fbAllocInfo.mapType = mapType;
    fbAllocInfo.stride  = fbStride;
    fbAllocInfo.height  = fbHeight;
    fbAllocInfo.size    = fbSize;
    fbAllocInfo.type    = FB_TYPE_PPU;
    fbAllocInfo.num     = ctx->fbCount.srcFbNum;
    fbAllocInfo.nv21    = encOpenParam.nv21;

    if (PRODUCT_ID_NOT_W_SERIES(ctx->productID)) {
        fbAllocInfo.lumaBitDepth = 8;
        fbAllocInfo.chromaBitDepth = 8;
    }

    if (FALSE == AllocFBMemory(encOpenParam.coreIdx, ctx->pFbSrcMem, ctx->pFbSrc, fbSize, ctx->fbCount.srcFbNum, ENC_SRC, ctx->handle->instIndex)) {
        VLOG(ERR, "failed to allocate source buffers\n");
        return FALSE;
    }
    ctx->srcFbAllocInfo = fbAllocInfo;

    //Buffers for reconstructed frames
    osal_memset(&fbAllocInfo, 0x00, sizeof(FrameBufferAllocInfo));

    if (PRODUCT_ID_W_SERIES(ctx->productID)) {
        pDramConfig = NULL;
        if (ctx->encOpenParam.bitstreamFormat == STD_AVC) {
            fbWidth  = VPU_ALIGN16(encOpenParam.picWidth);
            fbHeight = VPU_ALIGN16(encOpenParam.picHeight);
        } else {
            fbWidth  = VPU_ALIGN8(encOpenParam.picWidth);
            fbHeight = VPU_ALIGN8(encOpenParam.picHeight);
        }

        if ((ctx->rotAngle != 0 || ctx->mirDir != 0) && !(ctx->rotAngle == 180 && ctx->mirDir == MIRDIR_HOR_VER)) {
            fbWidth  = VPU_ALIGN32(encOpenParam.picWidth);
            fbHeight = VPU_ALIGN32(encOpenParam.picHeight);
        }
        //TODO if --> else if
        if (ctx->rotAngle == 90 || ctx->rotAngle == 270) {
            fbWidth  = VPU_ALIGN32(encOpenParam.picHeight);
            fbHeight = VPU_ALIGN32(encOpenParam.picWidth);
        }

        if (WAVE521C_DUAL_CODE == (VPU_HANDLE_TO_ENCINFO(ctx->handle)->productCode)) {
            mapType = encOpenParam.EncStdParam.waveParam.internalBitDepth==8?COMPRESSED_FRAME_MAP_DUAL_CORE_8BIT:COMPRESSED_FRAME_MAP_DUAL_CORE_10BIT;
        } else {
            mapType = COMPRESSED_FRAME_MAP;
        }
    } else {
        //CODA
        pDramConfig = &dramConfig;
        fbWidth = encOpenParam.picWidth;
        fbHeight = encOpenParam.picHeight;
        if (90 == ctx->rotAngle || 270 == ctx->rotAngle) {
            fbWidth = encOpenParam.picHeight;
            fbHeight = encOpenParam.picWidth;
        }
        fbWidth     = VPU_ALIGN16(fbWidth);
        fbHeight    = VPU_ALIGN32(fbHeight); 
        mapType     = mapType;
    }

    fbStride = CalcStride(fbWidth, fbHeight, (FrameBufferFormat)encOpenParam.outputFormat, encOpenParam.cbcrInterleave, mapType, FALSE);
    fbSize   = VPU_GetFrameBufSize(ctx->handle, encOpenParam.coreIdx, fbStride, fbHeight, mapType, (FrameBufferFormat)encOpenParam.outputFormat, encOpenParam.cbcrInterleave, pDramConfig);

    if (FALSE == AllocFBMemory(encOpenParam.coreIdx, ctx->pFbReconMem, ctx->pFbRecon, fbSize, ctx->fbCount.reconFbNum, ENC_FBC, ctx->handle->instIndex)) {
        VLOG(ERR, "failed to allocate recon buffers\n");
        return FALSE;
    }

    fbAllocInfo.stride  = fbStride;
    fbAllocInfo.height  = fbHeight;
    fbAllocInfo.type    = FB_TYPE_CODEC;
    fbAllocInfo.num     = ctx->fbCount.reconFbNum;

    ctx->reconFbAllocInfo = fbAllocInfo;

    return TRUE;
}

static BOOL PrepareYuvFeeder(ComponentImpl* com, BOOL* done) 
{
    CNMComponentParamRet    ret;
    YuvFeederContext*       ctx = (YuvFeederContext*)com->context;
    EncOpenParam*           encOpenParam = &ctx->encOpenParam;
    BOOL                    success;
    Uint32                  i;
    YuvFeeder               yuvFeeder    = NULL;
    YuvInfo                 yuvFeederInfo;

    *done = FALSE;
    // wait signal GET_PARAM_ENC_FRAME_BUF_NUM
    ret = ComponentGetParameter(com, com->sinkPort.connectedComponent, GET_PARAM_ENC_FRAME_BUF_NUM, &ctx->fbCount);
    if (ComponentParamReturnTest(ret, &success) == FALSE) {
        return success;
    }

    ret = ComponentGetParameter(com, com->sinkPort.connectedComponent, GET_PARAM_ENC_HANDLE, &ctx->handle);
    if (ComponentParamReturnTest(ret, &success) == FALSE) {
        return success;
    }

    if (FALSE == AllocateFrameBuffer(com)) {
        VLOG(ERR, "AllocateFramBuffer() error\n");
        return FALSE;
    }

    osal_memset(&yuvFeederInfo,   0x00, sizeof(YuvInfo));

    yuvFeederInfo.cbcrInterleave = encOpenParam->cbcrInterleave;
    yuvFeederInfo.nv21           = encOpenParam->nv21;
    yuvFeederInfo.packedFormat   = encOpenParam->packedFormat;
    yuvFeederInfo.srcFormat      = encOpenParam->srcFormat;
    yuvFeederInfo.srcPlanar      = TRUE;
    yuvFeederInfo.srcStride      = ctx->srcFbAllocInfo.stride;
    yuvFeederInfo.srcHeight      = VPU_CEIL(encOpenParam->picHeight, 8);
    yuvFeeder = YuvFeeder_Create(ctx->yuvMode, ctx->inputPath, yuvFeederInfo);
    if (yuvFeeder == NULL) {
        VLOG(ERR, "YuvFeeder_Create Error\n");
        return FALSE;
    }

    ((AbstractYuvFeeder*)yuvFeeder)->impl->handle = ctx->handle;
    ctx->yuvFeeder = yuvFeeder;

    // Fill data into the input queue of the sink port.
    // The empty input queue of the sink port occurs a hangup problem in this example.
    while (Queue_Dequeue(com->sinkPort.inputQ) != NULL);
    for (i = 0; i < com->sinkPort.inputQ->size; i++) {
        PortContainerYuv defaultData; 

        osal_memset(&defaultData, 0x00, sizeof(PortContainerYuv));
        defaultData.srcFbIndex   = -1;
        if (i<ctx->fbCount.srcFbNum) {
            defaultData.srcFbIndex   = i;
            Queue_Enqueue(com->sinkPort.inputQ, (void*)&defaultData);
        }
    }

    if (PRODUCT_ID_NOT_W_SERIES(ctx->productID)) {
        VPU_EncGiveCommand(ctx->handle, GET_TILEDMAP_CONFIG, &(ctx->mapConfig));
    }

    ctx->fbAllocated = TRUE;

    *done = TRUE;

    return TRUE;
}

static BOOL ExecuteYuvFeeder(ComponentImpl* com, PortContainer* in, PortContainer* out) 
{
    YuvFeederContext*   ctx          = (YuvFeederContext*)com->context;
    EncOpenParam*       encOpenParam = &ctx->encOpenParam;
    int                 ret          = 0;
    PortContainerYuv*   sinkData     = (PortContainerYuv*)out;
    void*               extraFeedingInfo = NULL;

    out->reuse = FALSE;
    if (ctx->state == YUV_FEEDER_STATE_WAIT) {
        CNMComponentParamRet ParamRet;
        BOOL                 success, done = FALSE;

        out->reuse = TRUE;
        ParamRet = ComponentGetParameter(com, com->sinkPort.connectedComponent, GET_PARAM_ENC_FRAME_BUF_REGISTERED, &done);
        if (ComponentParamReturnTest(ParamRet, &success) == FALSE) {
            return success;
        }
        if (done == FALSE) return TRUE;
        ctx->state = YUV_FEEDER_STATE_FEEDING;
        out->reuse = FALSE;
    }

    if ( ctx->last ) {
        sinkData->last = TRUE;
        return TRUE;
    }

    sinkData->fb              = ctx->pFbSrc[sinkData->srcFbIndex];

    if (PRODUCT_ID_NOT_W_SERIES(ctx->productID)) {
        extraFeedingInfo = &(ctx->mapConfig);
    }

    ctx->feedingNum++;
    if (ctx->feedingNum > ctx->frameOutNum && ctx->frameOutNum != -1) {
        ctx->last = sinkData->last = TRUE;
        return TRUE;
    }
    ret = YuvFeeder_Feed(ctx->yuvFeeder, encOpenParam->coreIdx, &sinkData->fb, encOpenParam->picWidth, encOpenParam->picHeight, sinkData->srcFbIndex, extraFeedingInfo);

    if (ret == FALSE) {
        ctx->last = sinkData->last = TRUE;
    }


    return TRUE;
}

static void ReleaseYuvFeeder(ComponentImpl* com)
{
    YuvFeederContext* ctx = (YuvFeederContext*)com->context;
    Uint32          i   = 0;

    for (i = 0; i < ctx->fbCount.reconFbNum*2; i++) {
        if (ctx->pFbReconMem[i].size)
            vdi_free_dma_memory(ctx->encOpenParam.coreIdx, &ctx->pFbReconMem[i], ENC_FBC, ctx->handle->instIndex);
    }
    for (i = 0; i < ctx->fbCount.srcFbNum; i++) {
        if (ctx->pFbSrcMem[i].size)
            vdi_free_dma_memory(ctx->encOpenParam.coreIdx, &ctx->pFbSrcMem[i], ENC_SRC, ctx->handle->instIndex);
        if (ctx->pFbOffsetTblMem[i].size)
            vdi_free_dma_memory(ctx->encOpenParam.coreIdx, &ctx->pFbOffsetTblMem[i], ENC_FBCY_TBL, ctx->handle->instIndex);
    }
}

static BOOL DestroyYuvFeeder(ComponentImpl* com) 
{
    YuvFeederContext* ctx = (YuvFeederContext*)com->context;

    if (ctx->yuvFeeder)   YuvFeeder_Destroy(ctx->yuvFeeder);

    osal_free(ctx);

    return TRUE;
}

static Component CreateYuvFeeder(ComponentImpl* com, CNMComponentConfig* componentParam) 
{
    YuvFeederContext*   ctx; 

    com->context = osal_malloc(sizeof(YuvFeederContext));
    ctx          = (YuvFeederContext*)com->context;

    InitYuvFeederContext(ctx, componentParam);

    if ( ctx->encOpenParam.sourceBufCount > com->numSinkPortQueue )
        com->numSinkPortQueue = ctx->encOpenParam.sourceBufCount;//set requested sourceBufCount


    ctx->productID = componentParam->testEncConfig.productId;

    return (Component)com;
}

ComponentImpl yuvFeederComponentImpl = {
    "yuvfeeder",
    NULL,
    {0,},
    {0,},
    sizeof(PortContainerYuv),
    ENC_SRC_BUF_NUM,              /* minimum feeder's numSinkPortQueue(relates to source buffer count)*/
    CreateYuvFeeder,
    GetParameterYuvFeeder,
    SetParameterYuvFeeder,
    PrepareYuvFeeder,
    ExecuteYuvFeeder,
    ReleaseYuvFeeder,
    DestroyYuvFeeder
};

