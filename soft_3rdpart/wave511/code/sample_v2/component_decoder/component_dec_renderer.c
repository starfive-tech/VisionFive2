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

typedef struct {
    DecHandle                   handle;                 /*!<< A decoder handle */
    osal_thread_t               threadHandle;
    TestDecConfig               testDecConfig;
    Uint32                      framebufStride;
    Uint32                      displayPeriodTime;
    FrameBuffer                 pFrame[MAX_REG_FRAME];
    vpu_buffer_t                pFbMem[MAX_REG_FRAME];
    BOOL                        enablePPU;
    FrameBuffer                 pPPUFrame[MAX_REG_FRAME];
    vpu_buffer_t                pPPUFbMem[MAX_REG_FRAME];
    Queue*                      ppuQ;
    BOOL                        fbAllocated;
    ParamDecNeedFrameBufferNum  fbCount;
    Queue*                      seqMemQ;
    osal_mutex_t                lock;
    char                        outputPath[256];
    FILE*                       fpOutput[OUTPUT_FP_NUMBER];
    BOOL                        MemoryOptimization;
    int                         totalBufferNumber;
    int                         currentBufferNumber;
    FrameBuffer                 pLinearFrame[MAX_REG_FRAME];
    vpu_buffer_t                pLinearFbMem[MAX_REG_FRAME];
} RendererContext;

typedef struct SequenceMemInfo {
    Uint32              nonLinearNum;
    Uint32              linearNum;
    Uint32              remainingCount;
    Uint32              sequenceNo;
    vpu_buffer_t        pFbMem[MAX_REG_FRAME];
    vpu_buffer_t        vbFbcYTbl[MAX_REG_FRAME];
    vpu_buffer_t        vbFbcCTbl[MAX_REG_FRAME];
    vpu_buffer_t        vbTask;
} SequenceMemInfo;

static void FreeFrameBuffer(DecHandle handle, Uint32 idx, SequenceMemInfo* info)
{
    Int32 coreIdx = handle->coreIdx;
    if (info->pFbMem[idx].size > 0) {
        if (idx < info->nonLinearNum)
            vdi_free_dma_memory(coreIdx, &info->pFbMem[idx], DEC_FBC, handle->instIndex);
        else
            vdi_free_dma_memory(coreIdx, &info->pFbMem[idx], DEC_FB_LINEAR, handle->instIndex);
        osal_memset((void*)&info->pFbMem[idx], 0x00, sizeof(vpu_buffer_t));
    }

    if (info->vbFbcYTbl[idx].size > 0) {
        vdi_free_dma_memory(coreIdx, &info->vbFbcYTbl[idx], DEC_FBCY_TBL, handle->instIndex);
        osal_memset((void*)&info->vbFbcYTbl[idx], 0x00, sizeof(vpu_buffer_t));
    }

    if (info->vbFbcCTbl[idx].size > 0) {
        vdi_free_dma_memory(coreIdx, &info->vbFbcCTbl[idx], DEC_FBCC_TBL, handle->instIndex);
        osal_memset((void*)&info->vbFbcCTbl[idx], 0x00, sizeof(vpu_buffer_t));
    }
    if (info->vbTask.size > 0) {
        vdi_free_dma_memory(coreIdx, &info->vbTask, DEC_TASK, handle->instIndex);
        osal_memset((void*)&info->vbTask, 0x00, sizeof(vpu_buffer_t));
    }
}

static BOOL SetParamFreeFrameBuffers(ComponentImpl* com, Uint32 fbFlags)
{
    RendererContext*    ctx          = (RendererContext*)com->context;
    BOOL                remainingFbs[MAX_REG_FRAME] = {0};
    Uint32              idx;
    Uint32              fbIndex;
    BOOL                wtlEnable    = ctx->testDecConfig.enableWTL;
    DecGetFramebufInfo  curFbInfo;
    Uint32              coreIdx      = ctx->testDecConfig.coreIdx;
    SequenceMemInfo     seqMem       = {0};

    osal_mutex_lock(ctx->lock);
    VPU_DecGiveCommand(ctx->handle, DEC_GET_FRAMEBUF_INFO, (void*)&curFbInfo);

    if (PRODUCT_ID_W_SERIES(ctx->testDecConfig.productId)) {
        for (idx=0; idx<MAX_GDI_IDX; idx++) {
            fbIndex = idx;
            if ((fbFlags>>idx) & 0x01) {
                if (wtlEnable == TRUE) {
                    fbIndex = VPU_CONVERT_WTL_INDEX(ctx->handle, idx);
                }
                seqMem.remainingCount++;
                remainingFbs[fbIndex] = TRUE;
            }
        }
    }

    seqMem.nonLinearNum = ctx->fbCount.nonLinearNum;
    seqMem.linearNum    = ctx->fbCount.linearNum;
    osal_memcpy((void*)seqMem.pFbMem,    ctx->pFbMem,         sizeof(ctx->pFbMem));
    osal_memcpy((void*)seqMem.vbFbcYTbl, curFbInfo.vbFbcYTbl, sizeof(curFbInfo.vbFbcYTbl));
    osal_memcpy((void*)seqMem.vbFbcCTbl, curFbInfo.vbFbcCTbl, sizeof(curFbInfo.vbFbcCTbl));
    osal_memcpy((void*)&seqMem.vbTask,  &curFbInfo.vbTask,    sizeof(curFbInfo.vbTask));

    for (idx=0; idx<MAX_REG_FRAME; idx++) {
        if (remainingFbs[idx] == FALSE) {
            FreeFrameBuffer(ctx->handle, idx, &seqMem);
        }
        // Free a mvcol buffer
        if(curFbInfo.vbMvCol[idx].size > 0) {
            vdi_free_dma_memory(coreIdx, &curFbInfo.vbMvCol[idx], DEC_MV, ctx->handle->instIndex);
        }
    }

    if (seqMem.remainingCount > 0) {
        Queue_Enqueue(ctx->seqMemQ, (void*)&seqMem);
    }

    ctx->fbAllocated = FALSE;
    osal_mutex_unlock(ctx->lock);

    return TRUE;
}

static BOOL ReallocateFrameBuffers(ComponentImpl* com, ParamDecReallocFB* param)
{
    RendererContext* ctx        = (RendererContext*)com->context;
    Int32            fbcIndex    = param->compressedIdx;
    Int32            linearIndex = param->linearIdx;
    vpu_buffer_t*    pFbMem      = ctx->pFbMem;
    FrameBuffer*     pFrame      = ctx->pFrame;
    FrameBuffer*     newFbs      = param->newFbs;

    if (fbcIndex >= 0) {
        /* Release the FBC framebuffer */
        vdi_free_dma_memory(ctx->testDecConfig.coreIdx, &pFbMem[fbcIndex], DEC_FBC, ctx->handle->instIndex);
        osal_memset((void*)&pFbMem[fbcIndex], 0x00, sizeof(vpu_buffer_t));
    }

    if (linearIndex >= 0) {
        /* Release the linear framebuffer */
        vdi_free_dma_memory(ctx->testDecConfig.coreIdx, &pFbMem[linearIndex], DEC_FB_LINEAR, ctx->handle->instIndex);
        osal_memset((void*)&pFbMem[linearIndex], 0x00, sizeof(vpu_buffer_t));
    }

    if (fbcIndex >= 0) {
        newFbs[0].myIndex = fbcIndex;
        newFbs[0].width   = param->width;
        newFbs[0].height  = param->height;
        pFrame[fbcIndex]  = newFbs[0];
    }

    if (linearIndex >= 0) {
        newFbs[1].myIndex = linearIndex;
        newFbs[1].width   = param->width;
        newFbs[1].height  = param->height;
        pFrame[linearIndex]  = newFbs[1];
    }

    return TRUE;
}

void Render_DecClrDispFlag(void *context, int index)
{
    RendererContext* ctx = (RendererContext*)context;
    VPU_DecClrDispFlag(ctx->handle, index);
}

static void DisplayFrame(RendererContext* ctx, DecOutputInfo* result)
{
    Int32 productID = ctx->testDecConfig.productId;

    osal_mutex_lock(ctx->lock);

    if (result->indexFrameDisplay >= 0) {
        DecInitialInfo  initialInfo;
        VPU_DecGiveCommand(ctx->handle, DEC_GET_SEQ_INFO, &initialInfo);
        // No renderer device
        if (PRODUCT_ID_W_SERIES(productID) && initialInfo.sequenceNo != result->sequenceNo) {
            // Free a framebuffer of previous sequence
            SequenceMemInfo* memInfo = Queue_Peek(ctx->seqMemQ);
            if (memInfo != NULL) {
                FreeFrameBuffer(ctx->handle, result->indexFrameDisplay, memInfo);
                if (memInfo->remainingCount == 0) {
                    VLOG(ERR, "%s:%d remainingCout must be greater than zero\n", __FUNCTION__, __LINE__);
                }
                memInfo->remainingCount--;
                if (memInfo->remainingCount == 0) {
                    Queue_Dequeue(ctx->seqMemQ);
                }
            }
        }
        else {
            VPU_DecClrDispFlag(ctx->handle, result->indexFrameDisplay);
        }
    }
    osal_mutex_unlock(ctx->lock);
}

static BOOL FlushFrameBuffers(ComponentImpl* com, Uint32* flushedIndexes)
{
    PortContainerDisplay*   srcData;
    RendererContext*        ctx = (RendererContext*)com->context;
    Int32                   idx = 0;

    osal_mutex_lock(ctx->lock);
    if (flushedIndexes) *flushedIndexes = 0;
    while ((srcData=(PortContainerDisplay*)ComponentPortGetData(&com->srcPort)) != NULL) {
        idx = srcData->decInfo.indexFrameDisplay;
        if (0 <= idx) {
            VPU_DecClrDispFlag(ctx->handle, idx);
            if (flushedIndexes) *flushedIndexes |= (1<<idx);
        }
        ComponentPortSetData(&com->srcPort, (PortContainer*)srcData);
    }
    osal_mutex_unlock(ctx->lock);

    return TRUE;
}



void SetRenderTotalBufferNumber(ComponentImpl* com, Uint32 number)
{
    RendererContext*     ctx            = (RendererContext*)com->context;
    ctx->fbCount.linearNum = ctx->totalBufferNumber = number;
}

void* AllocateFrameBuffer2(ComponentImpl* com, Uint32 size)
{
    RendererContext*     ctx            = (RendererContext*)com->context;
    int i = 0;

    if (ctx->handle == NULL) {
        ctx->MemoryOptimization = FALSE;
        return NULL;
    }
    for (i = 0;i < MAX_REG_FRAME; i ++){
        if (ctx->pLinearFbMem[i].phys_addr == 0)
        {
            VLOG(INFO, "Found empty frame at index %d\r\n", i);
            break;
        }
    }
    if (i == MAX_REG_FRAME)
    {
        VLOG(ERR, "Could not found empty frame at index\r\n");
        return NULL;
    }
    // TODO: Adjust ctx->fbCount
    void *ret = AllocateDecFrameBuffer2(ctx->handle, &ctx->testDecConfig, size, &ctx->pLinearFrame[i], &ctx->pLinearFbMem[i]);
    ctx->currentBufferNumber ++;
    return ret;
}

BOOL AttachDMABuffer(ComponentImpl* com, Uint64 virtAddress, Uint32 size)
{
    BOOL ret = FALSE;
    RendererContext*     ctx            = (RendererContext*)com->context;
    int i = 0;

    if (ctx->handle == NULL) {
        ctx->MemoryOptimization = FALSE;
        return FALSE;
    }
    for (i = 0;i < MAX_REG_FRAME; i ++){
        if (ctx->pLinearFbMem[i].phys_addr == 0)
        {
            VLOG(INFO, "Found empty frame at index %d\r\n", i);
            break;
        }
    }
    if (i == MAX_REG_FRAME)
    {
        VLOG(ERR, "Could not found empty frame at index\r\n");
        return FALSE;
    }
    ret = AttachDecDMABuffer(ctx->handle, &ctx->testDecConfig, virtAddress, size, &ctx->pLinearFrame[i], &ctx->pLinearFbMem[i]);
    ctx->currentBufferNumber ++;
    return ret;
}

static BOOL AllocateFrameBuffer(ComponentImpl* com)
{
    RendererContext*     ctx            = (RendererContext*)com->context;
    BOOL                 success;
    Uint32               compressedNum;
    Uint32               linearNum;
    CNMComponentParamRet ret;

    ret = ComponentGetParameter(com, com->srcPort.connectedComponent, GET_PARAM_DEC_FRAME_BUF_NUM, &ctx->fbCount);
    if (ComponentParamReturnTest(ret, &success) == FALSE) {
        return success;
    }

    osal_memset((void*)ctx->pFbMem, 0x00, sizeof(ctx->pFbMem));
    osal_memset((void*)ctx->pFrame, 0x00, sizeof(ctx->pFrame));

    compressedNum  = ctx->fbCount.nonLinearNum;
    linearNum      = ctx->fbCount.linearNum;

    if (compressedNum == 0 && linearNum == 0) {
        VLOG(ERR, "%s:%d The number of framebuffers are zero. compressed %d, linear: %d\n",
            __FUNCTION__, __LINE__, compressedNum, linearNum);
        return FALSE;
    }
    if (ctx->MemoryOptimization){
        linearNum = 0;
    }

    if (AllocateDecFrameBuffer(ctx->handle, &ctx->testDecConfig, compressedNum, linearNum, ctx->pFrame, ctx->pFbMem, &ctx->framebufStride) == FALSE) {
        VLOG(INFO, "%s:%d Failed to AllocateDecFrameBuffer()\n", __FUNCTION__, __LINE__);
        return FALSE;
    }
    ctx->fbAllocated = TRUE;

    return TRUE;
}

static BOOL AllocatePPUFrameBuffer(ComponentImpl* com)
{
    RendererContext*        ctx = (RendererContext*)com->context;
    TestDecConfig*          decConfig = &(ctx->testDecConfig);
    BOOL                    resOf = FALSE;

    osal_memset((void*)ctx->pPPUFbMem, 0x00, sizeof(ctx->pPPUFbMem));
    osal_memset((void*)ctx->pPPUFrame, 0x00, sizeof(ctx->pPPUFrame));

    if(PRODUCT_ID_960 == decConfig->productId || PRODUCT_ID_980 == decConfig->productId) {
        if (FALSE == Coda9AllocateDecPPUFrameBuffer(&(ctx->enablePPU), ctx->handle, &ctx->testDecConfig, ctx->pPPUFrame, ctx->pPPUFbMem, ctx->ppuQ)) {
            VLOG(INFO, "%s:%d Failed to Coda9AllocateDecPPUFrameBuffer()\n", __FUNCTION__, __LINE__);
        }
        resOf = TRUE;
    }

    return resOf;
}

static CNMComponentParamRet GetParameterRenderer(ComponentImpl* from, ComponentImpl* com, GetParameterCMD commandType, void* data)
{
    RendererContext*        ctx     = (RendererContext*)com->context;
    ParamDecFrameBuffer*    allocFb = NULL;
    ParamDecPPUFrameBuffer* allocPPUFb = NULL;
    PortContainer*         container;

    if (ctx->fbAllocated == FALSE) return CNM_COMPONENT_PARAM_NOT_READY;

    switch(commandType) {
    case GET_PARAM_COM_IS_CONTAINER_CONUSUMED:
        // This query command is sent from the comonponent core.
        // If input data are consumed in sequence, it should return TRUE through PortContainer::consumed.
        container = (PortContainer*)data;
        container->consumed = TRUE;
        break;
    case GET_PARAM_RENDERER_FRAME_BUF:
        if (ctx->MemoryOptimization){
            if (ctx->currentBufferNumber < ctx->totalBufferNumber) return CNM_COMPONENT_PARAM_NOT_READY;
            int i = 0;
            for (i = 0;i < MAX_REG_FRAME; i ++) {
                if (ctx->pFbMem[i].phys_addr == 0) {
                    break;
                }
            }
            VLOG(INFO, "The start index of LINEAR buffer = %d\n", i);
            for (int j = 0; j < MAX_REG_FRAME; j ++) {
                if (ctx->pLinearFbMem[j].phys_addr != 0) {
                    memcpy(&ctx->pFbMem[i + j], &ctx->pLinearFbMem[j], sizeof(vpu_buffer_t));
                    memcpy(&ctx->pFrame[i + j], &ctx->pLinearFrame[j], sizeof(FrameBuffer));
                }
            }
        }
        //TODO: Adjust Liner number
        allocFb = (ParamDecFrameBuffer*)data;
        allocFb->stride        = ctx->framebufStride;
        allocFb->linearNum     = ctx->fbCount.linearNum;
        allocFb->nonLinearNum   = ctx->fbCount.nonLinearNum;
        allocFb->fb            = ctx->pFrame;
        break;
    case GET_PARAM_RENDERER_PPU_FRAME_BUF:
        allocPPUFb = (ParamDecPPUFrameBuffer*)data;
        if (TRUE == AllocatePPUFrameBuffer(com)) {
            allocPPUFb->enablePPU   = ctx->enablePPU;
            allocPPUFb->ppuQ        = ctx->ppuQ;
            allocPPUFb->fb          = ctx->pPPUFrame;
        }
        else {
            allocPPUFb->enablePPU   = FALSE;
            allocPPUFb->ppuQ        = NULL;
            allocPPUFb->fb          = NULL;
        }
        break;
    default:
        return CNM_COMPONENT_PARAM_NOT_FOUND;
    }

    return CNM_COMPONENT_PARAM_SUCCESS;
}

static CNMComponentParamRet SetParameterRenderer(ComponentImpl* from, ComponentImpl* com, SetParameterCMD commandType, void* data)
{
    RendererContext*    ctx    = (RendererContext*)com->context;
    BOOL                result = TRUE;

    UNREFERENCED_PARAMETER(ctx);
    switch(commandType) {
    case SET_PARAM_RENDERER_REALLOC_FRAMEBUFFER:
        result = ReallocateFrameBuffers(com, (ParamDecReallocFB*)data);
        break;
    case SET_PARAM_RENDERER_FREE_FRAMEBUFFERS:
        result = SetParamFreeFrameBuffers(com, *(Uint32*)data);
        break;
    case SET_PARAM_RENDERER_FLUSH:
        result = FlushFrameBuffers(com, (Uint32*)data);
        break;
    case SET_PARAM_RENDERER_ALLOC_FRAMEBUFFERS:
        result = AllocateFrameBuffer(com);
        break;
    case SET_PARAM_RENDERER_CHANGE_COM_STATE:
        result = ComponentChangeState(com, *(Uint32*)data);
        break;
    default:
        return CNM_COMPONENT_PARAM_NOT_FOUND;
    }

    if (result == TRUE) return CNM_COMPONENT_PARAM_SUCCESS;
    else                return CNM_COMPONENT_PARAM_FAILURE;
}


static BOOL ExecuteRenderer(ComponentImpl* com, PortContainer* in, PortContainer* out)
{
    RendererContext*        ctx               = (RendererContext*)com->context;
    PortContainerDisplay*   srcData           = (PortContainerDisplay*)in;
    Int32                   indexFrameDisplay;
    TestDecConfig*          decConfig         = &ctx->testDecConfig;

    if (TRUE == com->pause) {
        return TRUE;
    }
#ifdef USE_FEEDING_METHOD_BUFFER
    PortContainerExternal *output = (PortContainerExternal*)ComponentPortPeekData(&com->sinkPort);
    if (output == NULL) {
        in->reuse = TRUE;
        return TRUE;
    }
#endif
    in->reuse = TRUE;

    indexFrameDisplay = srcData->decInfo.indexFrameDisplay;

    if (indexFrameDisplay == DISPLAY_IDX_FLAG_SEQ_END) {

        while ((output = (PortContainerExternal*)ComponentPortGetData(&com->sinkPort)) != NULL)
        {
            output->nFlags = 0x1;
            output->nFilledLen = 0;
            ComponentNotifyListeners(com, COMPONENT_EVENT_DEC_FILL_BUFFER_DONE, (void *)output);
        }
        com->terminate = TRUE;
        ComponentNotifyListeners(com, COMPONENT_EVENT_DEC_DECODED_ALL, NULL);
    }
    else if (indexFrameDisplay >= 0) {
        VpuRect      rcDisplay  = {0,};
        TiledMapType mapType    = srcData->decInfo.dispFrame.mapType;

        rcDisplay.right  = srcData->decInfo.dispPicWidth;
        rcDisplay.bottom = srcData->decInfo.dispPicHeight;

#ifdef USE_FEEDING_METHOD_BUFFER
        Uint32 width, height, bpp;
        size_t sizeYuv;
        Uint32 count = 0, total_count = 0;
        if (ctx->MemoryOptimization){
            uint8_t *dmaBuffer = NULL;
            if (FALSE ==GetYUVFromFrameBuffer2(NULL, &dmaBuffer, output->nFilledLen, ctx->handle, &srcData->decInfo.dispFrame, rcDisplay, &width, &height, &bpp, &sizeYuv)) {
                VLOG(ERR, "GetYUVFromFrameBuffer2 FAIL!\n");
            }
            total_count = Queue_Get_Cnt(com->sinkPort.inputQ);
            while (output->pBuffer != dmaBuffer)
            {
                output = (PortContainerExternal*)ComponentPortGetData(&com->sinkPort);
                Queue_Enqueue(com->sinkPort.inputQ, (void *)output);
                output = (PortContainerExternal*)ComponentPortPeekData(&com->sinkPort);
                VLOG(INFO, "pBuffer = %p, dmaBuffer = %p, index = %d/%d\n", output->pBuffer, dmaBuffer, count, total_count);
                if (count >= total_count)
                {
                    VLOG(ERR, "A wrong Frame Found\n");
                    output->nFilledLen = 0;
                    break;
                }
                count ++;
            }
        } else {
            if (FALSE ==GetYUVFromFrameBuffer2(output->pBuffer, NULL, output->nFilledLen, ctx->handle, &srcData->decInfo.dispFrame, rcDisplay, &width, &height, &bpp, &sizeYuv)) {
                VLOG(ERR, "GetYUVFromFrameBuffer2 FAIL!\n");
            }
        }
        output->nFilledLen = sizeYuv;
        output->nFlags = srcData->decInfo.indexFrameDisplay;
        if(ComponentPortGetData(&com->sinkPort))
        {
            ComponentNotifyListeners(com, COMPONENT_EVENT_DEC_FILL_BUFFER_DONE, (void *)output);
        }
        (void)DisplayFrame;
        (void)decConfig;
        (void)mapType;
#else
        if (decConfig->scaleDownWidth > 0 || decConfig->scaleDownHeight > 0) {
            rcDisplay.right  = VPU_CEIL(srcData->decInfo.dispPicWidth, 16);
        }

        if (strlen((const char*)decConfig->outputPath) > 0) {
            if (ctx->fpOutput[0] == NULL) {
                if (OpenDisplayBufferFile(decConfig->bitFormat, decConfig->outputPath, rcDisplay, mapType, ctx->fpOutput) == FALSE) {
                    return FALSE;
                }
            }
            SaveDisplayBufferToFile(ctx->handle, decConfig->bitFormat, srcData->decInfo.dispFrame, rcDisplay, ctx->fpOutput);
        }
        DisplayFrame(ctx, &srcData->decInfo);
#endif
        //osal_msleep(ctx->displayPeriodTime);
    }

    srcData->consumed = TRUE;
    srcData->reuse    = FALSE;

    if (srcData->last == TRUE) {
        while ((output = (PortContainerExternal*)ComponentPortGetData(&com->sinkPort)) != NULL)
        {
            output->nFlags = 0x1;
            output->nFilledLen = 0;
            VLOG(ERR, "Flush output port\r\n");
            ComponentNotifyListeners(com, COMPONENT_EVENT_DEC_FILL_BUFFER_DONE, (void *)output);
        }
        com->terminate = TRUE;
    }

    return TRUE;
}

static BOOL PrepareRenderer(ComponentImpl* com, BOOL* done)
{
    RendererContext* ctx = (RendererContext*)com->context;
    BOOL             ret;

    if (ctx->handle == NULL) {
        CNMComponentParamRet paramRet;
        BOOL                 success;
        paramRet = ComponentGetParameter(com, com->srcPort.connectedComponent, GET_PARAM_DEC_HANDLE, &ctx->handle);
        if (ComponentParamReturnTest(paramRet, &success) == FALSE) {
            return success;
        }
    }

    if ((ret=AllocateFrameBuffer(com)) == FALSE || ctx->fbAllocated == FALSE) {
        return ret;
    }
    ComponentNotifyListeners(com, COMPONENT_EVENT_DEC_REGISTER_FB, &ctx->pFrame[0]);
    // Nothing to do
    *done = TRUE;
    return TRUE;
}

static void ReleaseRenderer(ComponentImpl* com)
{
    RendererContext* ctx = (RendererContext*)com->context;
    Uint32           coreIdx = ctx->testDecConfig.coreIdx;
    Uint32           i;

    for (i=0; i<MAX_REG_FRAME; i++) {
        if (ctx->pFbMem[i].size) {
            if (i < ctx->fbCount.linearNum)
                vdi_free_dma_memory(coreIdx, &ctx->pFbMem[i], DEC_FBC, ctx->handle->instIndex);
            else
                vdi_free_dma_memory(coreIdx, &ctx->pFbMem[i], DEC_FB_LINEAR, ctx->handle->instIndex);
        }
    }

    for (i=0; i<MAX_REG_FRAME; i++) {
        if (ctx->pPPUFrame[i].size) vdi_free_dma_memory(coreIdx, &ctx->pPPUFbMem[i], DEC_ETC, ctx->handle->instIndex);
    }
}

static BOOL DestroyRenderer(ComponentImpl* com)
{
    RendererContext* ctx = (RendererContext*)com->context;

    CloseDisplayBufferFile(ctx->fpOutput);
    Queue_Destroy(ctx->seqMemQ);
    Queue_Destroy(ctx->ppuQ);
    osal_mutex_destroy(ctx->lock);
    osal_free(ctx);

    return TRUE;
}

static Component CreateRenderer(ComponentImpl* com, CNMComponentConfig* componentParam)
{
    RendererContext*    ctx;

    com->context = (RendererContext*)osal_malloc(sizeof(RendererContext));
    osal_memset(com->context, 0, sizeof(RendererContext));
    ctx = com->context;

    osal_memset(ctx->pLinearFbMem, 0, sizeof(ctx->pLinearFbMem));
    osal_memset(ctx->pLinearFrame, 0, sizeof(ctx->pLinearFrame));
    osal_memcpy((void*)&ctx->testDecConfig, (void*)&componentParam->testDecConfig, sizeof(TestDecConfig));
    if (componentParam->testDecConfig.fps > 0)
        ctx->displayPeriodTime = (1000 / componentParam->testDecConfig.fps);
    else
        ctx->displayPeriodTime = 1000/30;
    ctx->fbAllocated       = FALSE;
    ctx->seqMemQ           = Queue_Create(10, sizeof(SequenceMemInfo));
    ctx->lock              = osal_mutex_create();
    ctx->ppuQ              = Queue_Create(MAX_REG_FRAME, sizeof(FrameBuffer));
    ctx->MemoryOptimization = TRUE;
    ctx->totalBufferNumber = 10;
    ctx->currentBufferNumber = 0;
    return (Component)com;
}

ComponentImpl rendererComponentImpl = {
    "renderer",
    NULL,
    {0,},
    {0,},
    sizeof(PortContainer),     /* No output */
    5,
    CreateRenderer,
    GetParameterRenderer,
    SetParameterRenderer,
    PrepareRenderer,
    ExecuteRenderer,
    ReleaseRenderer,
    DestroyRenderer
};

