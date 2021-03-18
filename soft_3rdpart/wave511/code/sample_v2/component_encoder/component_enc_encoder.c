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
#include <time.h>
#include <sys/time.h>
#include "cnm_app.h"
#include "component.h"
#include "misc/debug.h"

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif
#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif


typedef enum {
    ENC_INT_STATUS_NONE,        // Interrupt not asserted yet
    ENC_INT_STATUS_FULL,        // Need more buffer
    ENC_INT_STATUS_DONE,        // Interrupt asserted
    ENC_INT_STATUS_LOW_LATENCY,
    ENC_INT_STATUS_TIMEOUT,     // Interrupt not asserted during given time.
    ENC_INT_STATUS_SRC_RELEASED,
} ENC_INT_STATUS;

typedef enum {
    ENCODER_STATE_OPEN,
    ENCODER_STATE_INIT_SEQ,
    ENCODER_STATE_REGISTER_FB,
    ENCODER_STATE_ENCODE_HEADER,
    ENCODER_STATE_ENCODING,
} EncoderState;

typedef struct {
    EncHandle                   handle;
    TestEncConfig               testEncConfig;
    EncOpenParam                encOpenParam;
    ParamEncNeedFrameBufferNum  fbCount;
    Uint32                      fbCountValid;
    Uint32                      frameIdx;
#ifdef SUPPORT_LOOK_AHEAD_RC
    Uint32                      encPicCnt;
#endif
    vpu_buffer_t                vbCustomLambda;
    vpu_buffer_t                vbScalingList;
    Uint32                      customLambda[NUM_CUSTOM_LAMBDA];
    UserScalingList             scalingList;
    vpu_buffer_t                vbCustomMap[MAX_REG_FRAME];
    EncoderState                state;
    BOOL                        stateDoing;
    EncInitialInfo              initialInfo;
    Queue*                      encOutQ;
    EncParam                    encParam;
    Int32                       encodedSrcFrmIdxArr[ENC_SRC_BUF_NUM];
    ParamEncBitstreamBuffer     bsBuf;
    BOOL                        fullInterrupt;
    Uint32                      changedCount;
    Uint64                      startTimeout;
#ifdef SUPPORT_LOOK_AHEAD_RC
    int                         larcEnable;
    int                         larcPass;
    int                         larcSize;
    EncLarcInfo                 larcInfo;
    int                         larcPass2Started;
    int                         larcPass1Finish;
#endif
    Uint32                      cyclePerTick;
    vpu_buffer_t                *bsBuffer[20];
} EncoderContext;

static BOOL FindEsBuffer(EncoderContext* ctx, PhysicalAddress addr, vpu_buffer_t* bs)
{
    Uint32 i;

    for (i=0; i<ctx->bsBuf.num; i++) {
        if (addr == ctx->bsBuf.bs[i].phys_addr) {
            *bs = ctx->bsBuf.bs[i];
            return TRUE;
        }
    }

    return FALSE;
}

static void SetEncPicParam(ComponentImpl* com, PortContainerYuv* in, EncParam* encParam)
{
    EncoderContext* ctx           = (EncoderContext*)com->context;
    TestEncConfig   testEncConfig = ctx->testEncConfig;
    Uint32          frameIdx      = ctx->frameIdx;
    Int32           srcFbWidth    = VPU_ALIGN8(ctx->encOpenParam.picWidth);
    Int32           srcFbHeight   = VPU_ALIGN8(ctx->encOpenParam.picHeight);
    Uint32          productId;
    vpu_buffer_t*   buf           = (vpu_buffer_t*)Queue_Peek(ctx->encOutQ);

    productId = VPU_GetProductId(ctx->encOpenParam.coreIdx);

    encParam->picStreamBufferAddr                = buf->phys_addr;
    encParam->picStreamBufferSize                = buf->size;
    encParam->srcIdx                             = in->srcFbIndex;
    encParam->srcEndFlag                         = in->last;
    encParam->sourceFrame                        = &in->fb;
    encParam->sourceFrame->sourceLBurstEn        = 0;
    if (testEncConfig.useAsLongtermPeriod > 0 && testEncConfig.refLongtermPeriod > 0) {
        encParam->useCurSrcAsLongtermPic         = (frameIdx % testEncConfig.useAsLongtermPeriod) == 0 ? 1 : 0;
        encParam->useLongtermRef                 = (frameIdx % testEncConfig.refLongtermPeriod)   == 0 ? 1 : 0;
    }
    encParam->skipPicture                        = 0;
    encParam->forceAllCtuCoefDropEnable	         = 0;

    encParam->forcePicQpEnable		             = 0;
    encParam->forcePicQpI			             = 0;
    encParam->forcePicQpP			             = 0;
    encParam->forcePicQpB			             = 0;
    encParam->forcePicTypeEnable	             = 0;
    encParam->forcePicType			             = 0;

    if (testEncConfig.forceIdrPicIdx == frameIdx) {
        encParam->forcePicTypeEnable = 1;
        encParam->forcePicType = 3;    // IDR
    }

    // FW will encode header data implicitly when changing the header syntaxes
    encParam->codeOption.implicitHeaderEncode    = 1;
    encParam->codeOption.encodeAUD               = testEncConfig.encAUD;
    encParam->codeOption.encodeEOS               = testEncConfig.encEOS;
    encParam->codeOption.encodeEOB               = testEncConfig.encEOB;

    // set custom map param
    if (productId != PRODUCT_ID_521)
        encParam->customMapOpt.roiAvgQp		     = testEncConfig.roi_avg_qp;
    encParam->customMapOpt.customLambdaMapEnable = testEncConfig.lambda_map_enable;
    encParam->customMapOpt.customModeMapEnable	 = testEncConfig.mode_map_flag & 0x1;
    encParam->customMapOpt.customCoefDropEnable  = (testEncConfig.mode_map_flag & 0x2) >> 1;
    encParam->customMapOpt.customRoiMapEnable    = testEncConfig.roi_enable;

    if (in->prevMapReuse == FALSE) {
        // packaging roi/lambda/mode data to custom map buffer.
        if (encParam->customMapOpt.customRoiMapEnable  || encParam->customMapOpt.customLambdaMapEnable ||
            encParam->customMapOpt.customModeMapEnable || encParam->customMapOpt.customCoefDropEnable) {
                SetMapData(testEncConfig.coreIdx, testEncConfig, ctx->encOpenParam, encParam, srcFbWidth, srcFbHeight, ctx->vbCustomMap[encParam->srcIdx].phys_addr);
        }

        // host should set proper value.
        // set weighted prediction param.
        if ((testEncConfig.wp_param_flag & 0x1)) {
            char lineStr[256] = {0,};
            Uint32 meanY, meanCb, meanCr, sigmaY, sigmaCb, sigmaCr;
            Uint32 maxMean  = (ctx->encOpenParam.bitstreamFormat == STD_AVC) ? 0xffff : ((1 << ctx->encOpenParam.EncStdParam.waveParam.internalBitDepth) - 1);
            Uint32 maxSigma = (ctx->encOpenParam.bitstreamFormat == STD_AVC) ? 0xffff : ((1 << (ctx->encOpenParam.EncStdParam.waveParam.internalBitDepth + 6)) - 1);
            fgets(lineStr, 256, testEncConfig.wp_param_file);
            sscanf(lineStr, "%d %d %d %d %d %d\n", &meanY, &meanCb, &meanCr, &sigmaY, &sigmaCb, &sigmaCr);

            meanY	= max(min(maxMean, meanY), 0);
            meanCb	= max(min(maxMean, meanCb), 0);
            meanCr	= max(min(maxMean, meanCr), 0);
            sigmaY	= max(min(maxSigma, sigmaY), 0);
            sigmaCb = max(min(maxSigma, sigmaCb), 0);
            sigmaCr = max(min(maxSigma, sigmaCr), 0);

            // set weighted prediction param.
            encParam->wpPixSigmaY	= sigmaY;
            encParam->wpPixSigmaCb	= sigmaCb;
            encParam->wpPixSigmaCr	= sigmaCr;
            encParam->wpPixMeanY    = meanY;
            encParam->wpPixMeanCb	= meanCb;
            encParam->wpPixMeanCr	= meanCr;
        }
    }
}

static BOOL RegisterFrameBuffers(ComponentImpl* com)
{
    EncoderContext*         ctx= (EncoderContext*)com->context;
    FrameBuffer*            pReconFb      = NULL;
    FrameBuffer*            pSrcFb        = NULL;
    FrameBufferAllocInfo    srcFbAllocInfo;
    Uint32                  reconFbStride = 0;
    Uint32                  reconFbHeight = 0;
    ParamEncFrameBuffer     paramFb;
    RetCode                 result;
    CNMComponentParamRet    ret;
    BOOL                    success;
    Uint32                  idx;
    EncInfo*                pEncInfo = VPU_HANDLE_TO_ENCINFO(ctx->handle);

    ctx->stateDoing = TRUE;
    ret = ComponentGetParameter(com, com->srcPort.connectedComponent, GET_PARAM_YUVFEEDER_FRAME_BUF, (void*)&paramFb);
    if (ComponentParamReturnTest(ret, &success) == FALSE) return success;

    pReconFb      = paramFb.reconFb;
    reconFbStride = paramFb.reconFbAllocInfo.stride;
    reconFbHeight = paramFb.reconFbAllocInfo.height;

    if (pEncInfo->productCode == WAVE521C_DUAL_CODE) {
        TiledMapType        mapType = ctx->encOpenParam.EncStdParam.waveParam.internalBitDepth==8?COMPRESSED_FRAME_MAP_DUAL_CORE_8BIT:COMPRESSED_FRAME_MAP_DUAL_CORE_10BIT;
        result = VPU_EncRegisterFrameBuffer(ctx->handle, pReconFb, ctx->fbCount.reconFbNum, reconFbStride, reconFbHeight, mapType);
    }
    else
        result = VPU_EncRegisterFrameBuffer(ctx->handle, pReconFb, ctx->fbCount.reconFbNum, reconFbStride, reconFbHeight, COMPRESSED_FRAME_MAP);
    if (result != RETCODE_SUCCESS) {
        VLOG(ERR, "%s:%d Failed to VPU_EncRegisterFrameBuffer(%d)\n", __FUNCTION__, __LINE__, result);
        ChekcAndPrintDebugInfo(ctx->handle, TRUE, result);
        return FALSE;
    }
    ComponentNotifyListeners(com, COMPONENT_EVENT_ENC_REGISTER_FB, NULL);

    pSrcFb         = paramFb.srcFb;
    srcFbAllocInfo = paramFb.srcFbAllocInfo;
    result = VPU_EncAllocateFrameBuffer(ctx->handle, srcFbAllocInfo, pSrcFb);
    if (result != RETCODE_SUCCESS) {
        VLOG(ERR, "VPU_EncAllocateFrameBuffer fail to allocate source frame buffer\n");
        ChekcAndPrintDebugInfo(ctx->handle, TRUE, result);
        return FALSE;
    }

    if (ctx->testEncConfig.roi_enable || ctx->testEncConfig.lambda_map_enable || ctx->testEncConfig.mode_map_flag) {
        for (idx = 0; idx < ctx->fbCount.srcFbNum; idx++) {
            ctx->vbCustomMap[idx].size = (ctx->encOpenParam.bitstreamFormat == STD_AVC) ? MAX_MB_NUM : MAX_CTU_NUM * 8;
            if (vdi_allocate_dma_memory(ctx->testEncConfig.coreIdx, &ctx->vbCustomMap[idx], ENC_ETC, ctx->handle->instIndex) < 0) {
                VLOG(ERR, "fail to allocate ROI buffer\n");
                return FALSE;
            }
        }
    }

    ctx->stateDoing = FALSE;

    return TRUE;
}

static CNMComponentParamRet GetParameterEncoder(ComponentImpl* from, ComponentImpl* com, GetParameterCMD commandType, void* data)
{
    EncoderContext*             ctx = (EncoderContext*)com->context;
    BOOL                        result  = TRUE;
    ParamEncNeedFrameBufferNum* fbCount;
    PortContainerYuv*           container;
    ParamVpuStatus*             status;
    QueueStatusInfo             cqInfo;
#ifdef SUPPORT_LOOK_AHEAD_RC
    EncLarcInfo                 *larcInfo;
#endif

    switch(commandType) {
    case GET_PARAM_COM_IS_CONTAINER_CONUSUMED:
        container = (PortContainerYuv*)data;
        if (ctx->encodedSrcFrmIdxArr[container->srcFbIndex]) {
            ctx->encodedSrcFrmIdxArr[container->srcFbIndex] = 0;
            container->consumed = TRUE;
        }
        break;
    case GET_PARAM_ENC_HANDLE:
        if (ctx->handle == NULL) return CNM_COMPONENT_PARAM_NOT_READY;
        *(EncHandle*)data = ctx->handle;
        break;
    case GET_PARAM_ENC_FRAME_BUF_NUM:
        if (ctx->fbCountValid == FALSE) return CNM_COMPONENT_PARAM_NOT_READY;
        fbCount = (ParamEncNeedFrameBufferNum*)data;
        fbCount->reconFbNum = ctx->fbCount.reconFbNum;
        fbCount->srcFbNum   = ctx->fbCount.srcFbNum;
        break;
    case GET_PARAM_ENC_FRAME_BUF_REGISTERED:
        if (ctx->state <= ENCODER_STATE_REGISTER_FB) return CNM_COMPONENT_PARAM_NOT_READY;
        *(BOOL*)data = TRUE;
        break;
#ifdef SUPPORT_LOOK_AHEAD_RC
    case GET_PARAM_ENC_LARC_INFO:
        if ( ctx->larcEnable == FALSE ) {
            result = FALSE;
            break;
        }
        larcInfo = (EncLarcInfo*)data;
        if ( ctx->larcPass  == LOOK_AHEAD_RC_PASS1 ) {
            larcInfo->Pass1encPicCnt   = ctx->encPicCnt;
            larcInfo->larcPass1Finish = ctx->larcPass1Finish;
            osal_memcpy(larcInfo->larcData, ctx->larcInfo.larcData, sizeof(larcInfo->larcData));
        }
        else {
            larcInfo->Pass2encPicCnt  = ctx->encPicCnt;
        }
        break;
#endif
    case GET_PARAM_VPU_STATUS:
        if (ctx->state != ENCODER_STATE_ENCODING) return CNM_COMPONENT_PARAM_NOT_READY;
        VPU_EncGiveCommand(ctx->handle, ENC_GET_QUEUE_STATUS, &cqInfo);
        status = (ParamVpuStatus*)data;
        status->cq = cqInfo;
        break;
    default:
        result = FALSE;
        break;
    }

    return (result == TRUE) ? CNM_COMPONENT_PARAM_SUCCESS : CNM_COMPONENT_PARAM_FAILURE;
}

static CNMComponentParamRet SetParameterEncoder(ComponentImpl* from, ComponentImpl* com, SetParameterCMD commandType, void* data)
{
    BOOL result = TRUE;

    switch (commandType) {
    case SET_PARAM_COM_PAUSE:
        com->pause   = *(BOOL*)data;
        break;
    default:
        VLOG(ERR, "Unknown SetParameterCMD Type : %d\n", commandType);
        result = FALSE;
        break;
    }

    return (result == TRUE) ? CNM_COMPONENT_PARAM_SUCCESS : CNM_COMPONENT_PARAM_FAILURE;
}

static ENC_INT_STATUS HandlingInterruptFlag(ComponentImpl* com)
{
    EncoderContext*   ctx               = (EncoderContext*)com->context;
    EncHandle       handle                = ctx->handle;
    Int32           interruptFlag         = 0;
    Uint32          interruptWaitTime     = VPU_WAIT_TIME_OUT_CQ;
    Uint32          interruptTimeout      = VPU_ENC_TIMEOUT;
    ENC_INT_STATUS  status                = ENC_INT_STATUS_NONE;

    if (ctx->startTimeout == 0ULL) {
        ctx->startTimeout = osal_gettime();
    }
    do {
        interruptFlag = VPU_WaitInterruptEx(handle, interruptWaitTime);
        if (INTERRUPT_TIMEOUT_VALUE == interruptFlag) {
            Uint64   currentTimeout = osal_gettime();

            if ((currentTimeout - ctx->startTimeout) > interruptTimeout) {
                VLOG(ERR, "<%s:%d> startTimeout(%lld) currentTime(%lld) diff(%d)\n",
                    __FUNCTION__, __LINE__, ctx->startTimeout, currentTimeout, (Uint32)(currentTimeout - ctx->startTimeout));
                CNMErrorSet(CNM_ERROR_HANGUP);
                status = ENC_INT_STATUS_TIMEOUT;
                break;
            }
            interruptFlag = 0;
        }

        if (interruptFlag < 0) {
            VLOG(ERR, "<%s:%d> interruptFlag is negative value! %08x\n", __FUNCTION__, __LINE__, interruptFlag);
        }

        if (interruptFlag > 0) {
            VPU_ClearInterruptEx(handle, interruptFlag);
            ctx->startTimeout = 0ULL;

            if (interruptFlag & (1<<INT_WAVE5_ENC_SET_PARAM)) {
                status = ENC_INT_STATUS_DONE;
                break;
            }

            if (interruptFlag & (1<<INT_WAVE5_ENC_PIC)) {
                status = ENC_INT_STATUS_DONE;
                break;
            }

            if (interruptFlag & (1<<INT_WAVE5_BSBUF_FULL)) {
                status = ENC_INT_STATUS_FULL;
                break;
            }

            if (interruptFlag & (1<<INT_WAVE5_ENC_LOW_LATENCY)) {
                status = ENC_INT_STATUS_LOW_LATENCY;
            }
#ifdef SUPPORT_SOURCE_RELEASE_INTERRUPT
            if (interruptFlag & (1 << INT_WAVE5_ENC_SRC_RELEASE)) {
                status = ENC_INT_STATUS_SRC_RELEASED;
            }
#endif
        }
    } while (FALSE);

    return status;
}

static BOOL SetSequenceInfo(ComponentImpl* com)
{
    EncoderContext* ctx = (EncoderContext*)com->context;
    EncHandle       handle  = ctx->handle;
    RetCode         ret     = RETCODE_SUCCESS;
    ENC_INT_STATUS  status;
    EncInitialInfo* initialInfo = &ctx->initialInfo;
    CNMComListenerEncCompleteSeq lsnpCompleteSeq   = {0};

#ifdef SUPPORT_LOOK_AHEAD_RC
    if ( ctx->larcPass  == LOOK_AHEAD_RC_PASS2 && ctx->larcPass2Started == FALSE) {
        BOOL                    success;
        ret = ComponentGetParameter(com, com->srcPort.connectedComponent, GET_PARAM_ENC_LARC_INFO, &ctx->larcInfo);
        if (ComponentParamReturnTest(ret, &success) == FALSE) {
            return success;
        }
        ctx->stateDoing = FALSE;
        if ( ctx->larcInfo.Pass1encPicCnt < ctx->larcSize  && FALSE == ctx->larcInfo.larcPass1Finish) {
            ctx->stateDoing = TRUE;
            return success;
        }
        if (ctx->larcInfo.larcPass1Finish == TRUE)
            ctx->larcInfo.larcData[ctx->larcInfo.Pass1encPicCnt][0] = -1;
        ctx->larcPass2Started = TRUE;
        if ( ctx->larcPass  == LOOK_AHEAD_RC_PASS2 ) {
            VPU_EncGiveCommand(handle, ENC_SET_LARC_DATA, &ctx->larcInfo);
        }
    }
#endif

    if (ctx->stateDoing == FALSE) {
        do {
            ret = VPU_EncIssueSeqInit(handle);
        } while (ret == RETCODE_QUEUEING_FAILURE && com->terminate == FALSE);

        if (ret != RETCODE_SUCCESS) {
            VLOG(ERR, "%s:%d Failed to VPU_EncIssueSeqInit() ret(%d)\n", __FUNCTION__, __LINE__, ret);
            ChekcAndPrintDebugInfo(ctx->handle, TRUE, ret);
            return FALSE;
        }
        ComponentNotifyListeners(com, COMPONENT_EVENT_ENC_ISSUE_SEQ, NULL);
    }
    ctx->stateDoing = TRUE;

    while (com->terminate == FALSE) {
        if ((status=HandlingInterruptFlag(com)) == ENC_INT_STATUS_DONE) {
            break;
        }
        else if (status == ENC_INT_STATUS_NONE) {
            return TRUE;
        }
        else if (status == ENC_INT_STATUS_TIMEOUT) {
            VLOG(INFO, "%s:%d INSTANCE #%d INTERRUPT TIMEOUT\n", __FUNCTION__, __LINE__, handle->instIndex);
            HandleEncoderError(ctx->handle, ctx->frameIdx, NULL);
            return FALSE;
        }
        else {
            VLOG(INFO, "%s:%d Unknown interrupt status: %d\n", __FUNCTION__, __LINE__, status);
            return FALSE;
        }
    }

    if ((ret=VPU_EncCompleteSeqInit(handle, initialInfo)) != RETCODE_SUCCESS) {
        VLOG(ERR, "%s:%d FAILED TO ENC_PIC_HDR: ret(%d), SEQERR(%08x)\n",
            __FUNCTION__, __LINE__, ret, initialInfo->seqInitErrReason);
        ChekcAndPrintDebugInfo(ctx->handle, TRUE, ret);
        return FALSE;
    }

    lsnpCompleteSeq.handle = handle;
    ComponentNotifyListeners(com, COMPONENT_EVENT_ENC_COMPLETE_SEQ, (void*)&lsnpCompleteSeq);

    ctx->fbCount.reconFbNum = initialInfo->minFrameBufferCount;
    ctx->fbCount.srcFbNum   = initialInfo->minSrcFrameCount + COMMAND_QUEUE_DEPTH + EXTRA_SRC_BUFFER_NUM;

    if ( ctx->encOpenParam.sourceBufCount > ctx->fbCount.srcFbNum)
        ctx->fbCount.srcFbNum = ctx->encOpenParam.sourceBufCount;

    ctx->fbCountValid = TRUE;

    VLOG(INFO, "[ENCODER] Required  reconFbCount=%d, srcFbCount=%d, outNum=%d, %dx%d\n",
        ctx->fbCount.reconFbNum, ctx->fbCount.srcFbNum, ctx->testEncConfig.outNum, ctx->encOpenParam.picWidth, ctx->encOpenParam.picHeight);
    ctx->stateDoing = FALSE;

    return TRUE;
}

static BOOL EncodeHeader(ComponentImpl* com)
{
    EncoderContext*         ctx = (EncoderContext*)com->context;
    EncHandle               handle  = ctx->handle;
    RetCode                 ret     = RETCODE_SUCCESS;
    EncHeaderParam          encHeaderParam;
    vpu_buffer_t*           buf;

    ctx->stateDoing = TRUE;
    buf = Queue_Dequeue(ctx->encOutQ);
    osal_memset(&encHeaderParam, 0x00, sizeof(EncHeaderParam));

    encHeaderParam.buf  = buf->phys_addr;
    encHeaderParam.size = buf->size;

    if (ctx->encOpenParam.bitstreamFormat == STD_HEVC) {
        encHeaderParam.headerType = CODEOPT_ENC_VPS | CODEOPT_ENC_SPS | CODEOPT_ENC_PPS;
    }
    else {
        // H.264
        encHeaderParam.headerType = CODEOPT_ENC_SPS | CODEOPT_ENC_PPS;
    }

    while(1) {
        ret = VPU_EncGiveCommand(handle, ENC_PUT_VIDEO_HEADER, &encHeaderParam);
        if ( ret != RETCODE_QUEUEING_FAILURE )
            break;
#if defined(HAPS_SIM) || defined(CNM_SIM_DPI_INTERFACE)
        osal_msleep(1);
#endif
    }

    DisplayEncodedInformation(ctx->handle, ctx->encOpenParam.bitstreamFormat, 0, NULL, 0, 0, ctx->testEncConfig.performance);

    ctx->stateDoing = FALSE;

    return TRUE;
}

static BOOL Encode(ComponentImpl* com, PortContainerYuv* in, PortContainerES* out)
{
    EncoderContext*         ctx             = (EncoderContext*)com->context;
    BOOL                    doEncode        = FALSE;
    BOOL                    doChangeParam   = FALSE;
    EncParam*               encParam        = &ctx->encParam;
    EncOutputInfo           encOutputInfo;
    ENC_INT_STATUS          intStatus;
    RetCode                 result;
    CNMComListenerHandlingInt lsnpHandlingInt   = {0};
    CNMComListenerEncDone   lsnpPicDone     = {0};
    CNMComListenerDecReadyOneFrame  lsnpReadyOneFrame = {0,};
    ENC_QUERY_WRPTR_SEL     encWrPtrSel     = GET_ENC_PIC_DONE_WRPTR;
    QueueStatusInfo         qStatus;
    int i=0;
#ifdef SUPPORT_LOOK_AHEAD_RC
    CNMComponentParamRet    ret;
    BOOL                    success;
#endif

    lsnpReadyOneFrame.handle = ctx->handle;
    ComponentNotifyListeners(com, COMPONENT_EVENT_ENC_READY_ONE_FRAME, &lsnpReadyOneFrame);

#ifdef SUPPORT_LOOK_AHEAD_RC
    if ( ctx->larcPass  == LOOK_AHEAD_RC_PASS1 ) {
        ret = ComponentGetParameter(com, com->sinkPort.connectedComponent, GET_PARAM_ENC_LARC_INFO, &ctx->larcInfo);
        if (ComponentParamReturnTest(ret, &success) == FALSE) {
            if (in)  in->reuse  = TRUE;
            if (out) out->reuse = TRUE;
            return success;
        }
        if (!(ctx->larcInfo.Pass2encPicCnt + LOOK_AHEAD_RC_DATA_SIZE > ctx->frameIdx)) {
            if (in)  in->reuse  = TRUE;
            if (out) out->reuse = TRUE;
            return TRUE;
        }
    }

    if ( ctx->larcPass  == LOOK_AHEAD_RC_PASS2 ) {
        ret = ComponentGetParameter(com, com->srcPort.connectedComponent, GET_PARAM_ENC_LARC_INFO, &ctx->larcInfo);
        if (ComponentParamReturnTest(ret, &success) == FALSE) {
            return success;
        }
        if ( !(ctx->larcInfo.Pass1encPicCnt > ctx->frameIdx + ctx->larcSize) && ctx->larcInfo.larcPass1Finish == FALSE) {
            if (in)  in->reuse  = TRUE;
            if (out) out->reuse = TRUE;
            return TRUE;
        }

        for (i = 0; i < 3; i++) {
            if ( ctx->larcInfo.larcPass1Finish == TRUE && ctx->larcInfo.Pass1encPicCnt <= ctx->frameIdx + ctx->larcSize - 1) {//-larcSize-1:already set in Wave5VpuEncSetLarcData
                encParam->larcData[i] = -1;
                break;
            }
            else
                encParam->larcData[i] = ctx->larcInfo.larcData[(ctx->frameIdx+ctx->larcSize-1) % LOOK_AHEAD_RC_DATA_SIZE][i];
        }
    }
#endif

    ctx->stateDoing = TRUE;
    if (out) {
        if (out->buf.phys_addr != 0) {
            Queue_Enqueue(ctx->encOutQ, (void*)&out->buf);
            out->buf.phys_addr = 0;
            out->buf.size      = 0;
        }
        if (encParam->srcEndFlag == TRUE) {
            doEncode = (BOOL)(Queue_Get_Cnt(ctx->encOutQ) > 0);
        }
    }
    if (in) {
        if (Queue_Get_Cnt(ctx->encOutQ) > 0) {
            SetEncPicParam(com, in, encParam);
            doEncode = TRUE;
            in->prevMapReuse = TRUE;
        }
    }

    if (TRUE == com->pause)
        doEncode = FALSE;

    VPU_EncGiveCommand(ctx->handle, ENC_GET_QUEUE_STATUS, &qStatus);
    if (COMMAND_QUEUE_DEPTH == qStatus.instanceQueueCount) {
        doEncode = FALSE;
    }

    if ((ctx->testEncConfig.numChangeParam > ctx->changedCount) &&
        (ctx->testEncConfig.changeParam[ctx->changedCount].setParaChgFrmNum == ctx->frameIdx)) {
        doChangeParam = TRUE;
    }

    if (doChangeParam == TRUE) {
        result = SetChangeParam(ctx->handle, ctx->testEncConfig, ctx->encOpenParam, ctx->changedCount);
        if (result == RETCODE_SUCCESS) {
            VLOG(TRACE, "ENC_SET_PARA_CHANGE queue success\n");
            ctx->changedCount++;
        }
        else if (result == RETCODE_QUEUEING_FAILURE) { // Just retry
            VLOG(INFO, "ENC_SET_PARA_CHANGE Queue Full\n");
            doEncode  = FALSE;
        }
        else { // Error
            VLOG(ERR, "VPU_EncGiveCommand[ENC_SET_PARA_CHANGE] failed Error code is 0x%x \n", result);
            ChekcAndPrintDebugInfo(ctx->handle, TRUE, result);
            return FALSE;
        }
    }

    /* The simple load balancer : To use this function, call InitLoadBalancer() before decoding process. */
    if (TRUE == doEncode) {
        if (in) in->reuse = TRUE;
        doEncode = LoadBalancerGetMyTurn(ctx->handle->instIndex);
    }

    if (doEncode == TRUE) {
        CNMComListenerEncStartOneFrame lsn;
        result = VPU_EncStartOneFrame(ctx->handle, encParam);
        if (result == RETCODE_SUCCESS) {
            /* The simple load balancer : pass its turn */
            LoadBalancerSetNextTurn();

            if (in) in->prevMapReuse = FALSE;
            Queue_Dequeue(ctx->encOutQ);
            ctx->frameIdx++;
            if (in) in->reuse = FALSE;
            if (out) out->reuse = FALSE;
        }
        else if (result == RETCODE_QUEUEING_FAILURE) { // Just retry
            // Just retry
            VPU_EncGiveCommand(ctx->handle, ENC_GET_QUEUE_STATUS, (void*)&qStatus);
            if (qStatus.instanceQueueCount == 0) {
                VLOG(ERR, "<%s:%d> The queue is empty but it can't add a command\n", __FUNCTION__, __LINE__);
                return FALSE;
            }
        }
        else { // Error
            VLOG(ERR, "VPU_EncStartOneFrame failed Error code is 0x%x \n", result);
            ChekcAndPrintDebugInfo(ctx->handle, TRUE, result);
            CNMErrorSet(CNM_ERROR_HANGUP);
            HandleEncoderError(ctx->handle, ctx->frameIdx, NULL);
            return FALSE;
        }
        lsn.handle = ctx->handle;
        lsn.result = result;
        ComponentNotifyListeners(com, COMPONENT_EVENT_ENC_START_ONE_FRAME, (void*)&lsn);
#ifdef SUPPORT_TESTCASE_CQ_16
        if (lsn.result == COMMAND_QUEUE_NOT_FULL) {
            if (out) {
                out->size  = 0;
                out->reuse = FALSE;
            }
            return TRUE; /* Try again */
        }
#endif
    }

    if ((intStatus=HandlingInterruptFlag(com)) == ENC_INT_STATUS_TIMEOUT) {
        HandleEncoderError(ctx->handle, ctx->frameIdx, NULL);
        VPU_SWReset(ctx->testEncConfig.coreIdx, SW_RESET_SAFETY, ctx->handle);
        return FALSE;
    }
    else if (intStatus == ENC_INT_STATUS_FULL || intStatus == ENC_INT_STATUS_LOW_LATENCY) {
        CNMComListenerEncFull   lsnpFull;
        PhysicalAddress         paRdPtr;
        PhysicalAddress         paWrPtr;
        int                     size;

        encWrPtrSel = (intStatus==ENC_INT_STATUS_FULL) ? GET_ENC_BSBUF_FULL_WRPTR : GET_ENC_LOW_LATENCY_WRPTR;
        VPU_EncGiveCommand(ctx->handle, ENC_WRPTR_SEL, &encWrPtrSel);
        VPU_EncGetBitstreamBuffer(ctx->handle, &paRdPtr, &paWrPtr, &size);
        VLOG(TRACE, "<%s:%d> INT_BSBUF_FULL inst=%d, %p, %p\n", __FUNCTION__, __LINE__, ctx->handle->instIndex, paRdPtr, paWrPtr);


        lsnpFull.handle = ctx->handle;
        ComponentNotifyListeners(com, COMPONENT_EVENT_ENC_FULL_INTERRUPT, (void*)&lsnpFull);

        if ( out ) {
            if (ctx->encOpenParam.ringBufferEnable ==  TRUE) {
                out->buf.phys_addr = paRdPtr;
                out->buf.size = size;
                out->size  = size;
                out->reuse = FALSE;
                out->streamBufFull = TRUE;
                out->rdPtr = paRdPtr;
                out->wrPtr = paWrPtr;
                out->paBsBufStart = ctx->encOpenParam.bitstreamBuffer;
                out->paBsBufEnd = ctx->encOpenParam.bitstreamBuffer + ctx->encOpenParam.bitstreamBufferSize;
            }
            else {
                if (FindEsBuffer(ctx, paRdPtr, &out->buf) == FALSE) {
                    VLOG(ERR, "%s:%d Failed to find buffer(%p)\n", __FUNCTION__, __LINE__, paRdPtr);
                    return FALSE;
                }
                out->size  = size;
                out->reuse = FALSE;
                out->streamBufFull = TRUE;
            }
        }
        ctx->fullInterrupt = TRUE;
        return TRUE;
    }
#ifdef SUPPORT_SOURCE_RELEASE_INTERRUPT
    else if (intStatus == ENC_INT_STATUS_SRC_RELEASED) {
        Uint32 srcBufFlag = 0;
        VPU_EncGiveCommand(ctx->handle, ENC_GET_SRC_BUF_FLAG, &srcBufFlag);
        for (i = 0; i < ctx->fbCount.srcFbNum; i++) {
            if ( (srcBufFlag >> i) & 0x01) {
                ctx->encodedSrcFrmIdxArr[i] = 1;
            }
        }
        return TRUE;
    }
#endif
    else if (intStatus == ENC_INT_STATUS_NONE) {
        if (out) {
            out->size  = 0;
            out->reuse = FALSE;
        }
        return TRUE; /* Try again */
    }
    ComponentNotifyListeners(com, COMPONENT_EVENT_ENC_HANDLING_INT, (void*)&lsnpHandlingInt);

    VPU_EncGiveCommand(ctx->handle, ENC_WRPTR_SEL, &encWrPtrSel);
    osal_memset(&encOutputInfo, 0x00, sizeof(EncOutputInfo));
    encOutputInfo.result = VPU_EncGetOutputInfo(ctx->handle, &encOutputInfo);
    if (encOutputInfo.result == RETCODE_REPORT_NOT_READY) {
        return TRUE; /* Not encoded yet */
    }
    else if (encOutputInfo.result == RETCODE_VLC_BUF_FULL) {
        VLOG(ERR, "VLC BUFFER FULL!!! ALLOCATE MORE TASK BUFFER(%d)!!!\n", ONE_TASKBUF_SIZE_FOR_CQ);
    }
    else if (encOutputInfo.result != RETCODE_SUCCESS) {
        /* ERROR */
        VLOG(ERR, "Failed to encode error = %d, %x\n", encOutputInfo.result, encOutputInfo.errorReason);
        ChekcAndPrintDebugInfo(ctx->handle, TRUE, encOutputInfo.result);
        HandleEncoderError(ctx->handle, encOutputInfo.encPicCnt, &encOutputInfo);
        VPU_SWReset(ctx->testEncConfig.coreIdx, SW_RESET_SAFETY, ctx->handle);
        return FALSE;
    }
    else {
        ;/* SUCCESS */
    }

    if (encOutputInfo.reconFrameIndex == RECON_IDX_FLAG_CHANGE_PARAM) {
        VLOG(TRACE, "CHANGE PARAMETER!\n");
        return TRUE; /* Try again */
    }
    else {
        DisplayEncodedInformation(ctx->handle, ctx->encOpenParam.bitstreamFormat, ctx->frameIdx, &encOutputInfo, encParam->srcEndFlag, encParam->srcIdx, ctx->testEncConfig.performance);
    }
    lsnpPicDone.handle = ctx->handle;
    lsnpPicDone.output = &encOutputInfo;
    lsnpPicDone.fullInterrupted = ctx->fullInterrupt;
    ComponentNotifyListeners(com, COMPONENT_EVENT_ENC_GET_OUTPUT_INFO, (void*)&lsnpPicDone);
    if ( encOutputInfo.result != RETCODE_SUCCESS )
        return FALSE;

    for (i = 0; i < ctx->fbCount.srcFbNum; i++) {
        if ( (encOutputInfo.releaseSrcFlag >> i) & 0x01) {
            ctx->encodedSrcFrmIdxArr[i] = 1;
        }
    }

    ctx->fullInterrupt      = FALSE;

    if ( out ) {
        if (ctx->encOpenParam.ringBufferEnable ==  TRUE) {
            out->buf.phys_addr = encOutputInfo.rdPtr;
            out->size  = encOutputInfo.bitstreamSize;
            out->reuse = (BOOL)(out->size == 0);
        }
        else {
            if (FindEsBuffer(ctx, encOutputInfo.bitstreamBuffer, &out->buf) == FALSE) {
                VLOG(ERR, "%s:%d Failed to find buffer(%p)\n", __FUNCTION__, __LINE__, encOutputInfo.bitstreamBuffer);
                return FALSE;
            }
            out->size  = encOutputInfo.bitstreamSize;
            out->reuse = (BOOL)(out->size == 0);
        }
    }

    // Finished encoding a frame
    if (encOutputInfo.reconFrameIndex == RECON_IDX_FLAG_ENC_END) {
        if (ctx->testEncConfig.outNum != encOutputInfo.encPicCnt && ctx->testEncConfig.outNum != -1) {
            VLOG(ERR, "outnum(%d) != encoded cnt(%d)\n", ctx->testEncConfig.outNum, encOutputInfo.encPicCnt);
            return FALSE;
        }
#ifdef SUPPORT_LOOK_AHEAD_RC
        if (TRUE == ctx->larcEnable && ctx->larcPass  == LOOK_AHEAD_RC_PASS1) {
            //no terminate here. PASS2 will terminate PASS1 component
            ctx->larcPass1Finish = TRUE;
            if (out) out->reuse = FALSE;
            ctx->stateDoing    = FALSE;
        }
        else {
#endif
        if(out) out->last  = TRUE;  // Send finish signal
        if(out) out->reuse = FALSE;
        ctx->stateDoing    = FALSE;
        com->terminate     = TRUE;
#ifdef SUPPORT_LOOK_AHEAD_RC
        }
#endif
        //to read remain data
        if (ctx->encOpenParam.ringBufferEnable ==  TRUE && ctx->encOpenParam.ringBufferWrapEnable == FALSE) {
            if(out) out->rdPtr = encOutputInfo.rdPtr;
            if(out) out->wrPtr = encOutputInfo.wrPtr;
            if(out) out->size = encOutputInfo.wrPtr - encOutputInfo.rdPtr;
        }
    }

#ifdef SUPPORT_LOOK_AHEAD_RC
    ctx->encPicCnt = encOutputInfo.encPicCnt;
    if (ctx->larcPass  == LOOK_AHEAD_RC_PASS1
        && encOutputInfo.reconFrameIndex != RECON_IDX_FLAG_HEADER_ONLY
        && encOutputInfo.bitstreamSize > 0) {
            for (i = 0; i < 3; i++) {
                ctx->larcInfo.larcData[(ctx->encPicCnt-1) % LOOK_AHEAD_RC_DATA_SIZE][i] = encOutputInfo.larcData[i];
            }
    }
#endif

    return TRUE;
}

static BOOL AllocateCustomBuffer(EncHandle handle, ComponentImpl* com)
{
    EncoderContext* ctx       = (EncoderContext*)com->context;
    TestEncConfig testEncConfig = ctx->testEncConfig;
    EncInfo*        pEncInfo;
    EncOpenParam*   pOpenParam;
    EncWaveParam*   pParam;

    pEncInfo    = &handle->CodecInfo->encInfo;
    pOpenParam  = &pEncInfo->openParam;
    pParam      = &pOpenParam->EncStdParam.waveParam;

    /* Allocate Buffer and Set Data */
    if (pParam->scalingListEnable) {
        ctx->vbScalingList.size = 0x1000;
        if (vdi_allocate_dma_memory(testEncConfig.coreIdx, &ctx->vbScalingList, ENC_ETC, ctx->handle->instIndex) < 0) {
            VLOG(ERR, "fail to allocate scaling list buffer\n");
            return FALSE;
        }
        pParam->userScalingListAddr = ctx->vbScalingList.phys_addr;

        parse_user_scaling_list(&ctx->scalingList, testEncConfig.scaling_list_file, testEncConfig.stdMode);
        vdi_write_memory(testEncConfig.coreIdx, ctx->vbScalingList.phys_addr, (unsigned char*)&ctx->scalingList, ctx->vbScalingList.size, VDI_LITTLE_ENDIAN);
    }

    if (pParam->customLambdaEnable) {
        ctx->vbCustomLambda.size = 0x200;
        if (vdi_allocate_dma_memory(testEncConfig.coreIdx, &ctx->vbCustomLambda, ENC_ETC, ctx->handle->instIndex) < 0) {
            VLOG(ERR, "fail to allocate Lambda map buffer\n");
            return FALSE;
        }
        pParam->customLambdaAddr = ctx->vbCustomLambda.phys_addr;

        parse_custom_lambda(ctx->customLambda, testEncConfig.custom_lambda_file);
        vdi_write_memory(testEncConfig.coreIdx, ctx->vbCustomLambda.phys_addr, (unsigned char*)&ctx->customLambda[0], ctx->vbCustomLambda.size, VDI_LITTLE_ENDIAN);
    }


    return TRUE;
}

static BOOL OpenEncoder(ComponentImpl* com)
{
    EncoderContext*         ctx = (EncoderContext*)com->context;
    SecAxiUse               secAxiUse;
    MirrorDirection         mirrorDirection;
    RetCode                 result;
    CNMComListenerEncOpen   lspn    = {0};

    ctx->stateDoing = TRUE;

    ctx->encOpenParam.bitstreamBuffer     = ctx->bsBuf.bs[0].phys_addr;
    ctx->encOpenParam.bitstreamBufferSize = ctx->bsBuf.bs[0].size;

    if ((result = VPU_EncOpen(&ctx->handle, &ctx->encOpenParam)) != RETCODE_SUCCESS) {
        VLOG(ERR, "VPU_EncOpen failed Error code is 0x%x \n", result);
        if ( result == RETCODE_VPU_RESPONSE_TIMEOUT ) {
            CNMErrorSet(CNM_ERROR_HANGUP);
        }
        CNMAppStop();
        return FALSE;
    }
    //VPU_EncGiveCommand(ctx->handle, ENABLE_LOGGING, 0);
    LoadBalancerAddInstance(ctx->handle->instIndex);

    if (AllocateCustomBuffer(ctx->handle, com) == FALSE) return FALSE;
    lspn.handle = ctx->handle;
    ComponentNotifyListeners(com, COMPONENT_EVENT_ENC_OPEN, (void*)&lspn);

    if (ctx->testEncConfig.rotAngle != 0 || ctx->testEncConfig.mirDir != 0) {
        VPU_EncGiveCommand(ctx->handle, ENABLE_ROTATION, 0);
        VPU_EncGiveCommand(ctx->handle, ENABLE_MIRRORING, 0);
        VPU_EncGiveCommand(ctx->handle, SET_ROTATION_ANGLE, &ctx->testEncConfig.rotAngle);
        mirrorDirection = (MirrorDirection)ctx->testEncConfig.mirDir;
        VPU_EncGiveCommand(ctx->handle, SET_MIRROR_DIRECTION, &mirrorDirection);
    }

    osal_memset(&secAxiUse,   0x00, sizeof(SecAxiUse));
    secAxiUse.u.wave.useEncRdoEnable = (ctx->testEncConfig.secondaryAXI & 0x1) ? TRUE : FALSE;  //USE_RDO_INTERNAL_BUF
    secAxiUse.u.wave.useEncLfEnable  = (ctx->testEncConfig.secondaryAXI & 0x2) ? TRUE : FALSE;  //USE_LF_INTERNAL_BUF
    VPU_EncGiveCommand(ctx->handle, SET_SEC_AXI, &secAxiUse);
    VPU_EncGiveCommand(ctx->handle, SET_CYCLE_PER_TICK,   (void*)&ctx->cyclePerTick);

    ctx->stateDoing = FALSE;

    return TRUE;
}

static BOOL ExecuteEncoder(ComponentImpl* com, PortContainer* in, PortContainer* out)
{
    EncoderContext* ctx             = (EncoderContext*)com->context;
    BOOL            ret;

    switch (ctx->state) {
    case ENCODER_STATE_OPEN:
        ret = OpenEncoder(com);
        if (ctx->stateDoing == FALSE) ctx->state = ENCODER_STATE_INIT_SEQ;
        break;
    case ENCODER_STATE_INIT_SEQ:
        ret = SetSequenceInfo(com);
        if (ctx->stateDoing == FALSE) ctx->state = ENCODER_STATE_REGISTER_FB;
        break;
    case ENCODER_STATE_REGISTER_FB:
        ret = RegisterFrameBuffers(com);
        if (ctx->stateDoing == FALSE) ctx->state = ENCODER_STATE_ENCODE_HEADER;
        break;
    case ENCODER_STATE_ENCODE_HEADER:
        ret = EncodeHeader(com);
        if (ctx->stateDoing == FALSE) ctx->state = ENCODER_STATE_ENCODING;
        break;
    case ENCODER_STATE_ENCODING:
        ret = Encode(com, (PortContainerYuv*)in, (PortContainerES*)out);
        break;
    default:
        ret = FALSE;
        break;
    }

    if (ret == FALSE || com->terminate == TRUE) {
        ComponentNotifyListeners(com, COMPONENT_EVENT_ENC_ENCODED_ALL, (void*)ctx->handle);
        if (out != NULL) {
            out->reuse = FALSE;
            out->last  = TRUE;
        }
    }
    return ret;
}

static BOOL PrepareEncoder(ComponentImpl* com, BOOL* done)
{
    EncoderContext*      ctx       = (EncoderContext*)com->context;
    TestEncConfig*       testEncConfig = &ctx->testEncConfig;
    CNMComponentParamRet ret;
    BOOL                 success;
    Uint32               i;

    *done = FALSE;

    ret = ComponentGetParameter(com, com->sinkPort.connectedComponent, GET_PARAM_READER_BITSTREAM_BUF, &ctx->bsBuf);
    if (ComponentParamReturnTest(ret, &success) == FALSE) return success;

    if (ctx->encOpenParam.ringBufferEnable ==  TRUE) {
        ctx->encOutQ = Queue_Create(com->numSinkPortQueue, sizeof(vpu_buffer_t));
        for (i=0; i<com->numSinkPortQueue; i++) {
            if ( i < ctx->bsBuf.num) {
                ctx->bsBuffer[i] = &ctx->bsBuf.bs[i]; }
            else {
                ctx->bsBuffer[i] = osal_malloc(sizeof(vpu_buffer_t));
                osal_memcpy(ctx->bsBuffer[i], ctx->bsBuffer[0], sizeof(vpu_buffer_t));
            }

            Queue_Enqueue(ctx->encOutQ, (void*)&ctx->bsBuffer[i]);//same addr enqueue
        }
    } else {
        ctx->encOutQ = Queue_Create(ctx->bsBuf.num, sizeof(vpu_buffer_t));
        for (i=0; i<ctx->bsBuf.num; i++) {
            Queue_Enqueue(ctx->encOutQ, (void*)&ctx->bsBuf.bs[i]);
        }
    }

    /* Open Data File*/
    if (ctx->encOpenParam.EncStdParam.waveParam.scalingListEnable) {
        if (testEncConfig->scaling_list_fileName) {
            ChangePathStyle(testEncConfig->scaling_list_fileName);
            if ((testEncConfig->scaling_list_file = osal_fopen(testEncConfig->scaling_list_fileName, "r")) == NULL) {
                VLOG(ERR, "fail to open scaling list file, %s\n", testEncConfig->scaling_list_fileName);
                return FALSE;
            }
        }
    }

    if (ctx->encOpenParam.EncStdParam.waveParam.customLambdaEnable) {
        if (testEncConfig->custom_lambda_fileName) {
            ChangePathStyle(testEncConfig->custom_lambda_fileName);
            if ((testEncConfig->custom_lambda_file = osal_fopen(testEncConfig->custom_lambda_fileName, "r")) == NULL) {
                VLOG(ERR, "fail to open custom lambda file, %s\n", testEncConfig->custom_lambda_fileName);
                return FALSE;
            }
        }
    }

    if (testEncConfig->roi_enable) {
        if (testEncConfig->roi_file_name) {
            ChangePathStyle(testEncConfig->roi_file_name);
            if ((testEncConfig->roi_file = osal_fopen(testEncConfig->roi_file_name, "r")) == NULL) {
                VLOG(ERR, "fail to open ROI file, %s\n", testEncConfig->roi_file_name);
                return FALSE;
            }
        }
    }

    if (testEncConfig->lambda_map_enable) {
        if (testEncConfig->lambda_map_fileName) {
            ChangePathStyle(testEncConfig->lambda_map_fileName);
            if ((testEncConfig->lambda_map_file = osal_fopen(testEncConfig->lambda_map_fileName, "r")) == NULL) {
                VLOG(ERR, "fail to open lambda map file, %s\n", testEncConfig->lambda_map_fileName);
                return FALSE;
            }
        }
    }

    if (testEncConfig->mode_map_flag) {
        if (testEncConfig->mode_map_fileName) {
            ChangePathStyle(testEncConfig->mode_map_fileName);
            if ((testEncConfig->mode_map_file = osal_fopen(testEncConfig->mode_map_fileName, "r")) == NULL) {
                VLOG(ERR, "fail to open custom mode map file, %s\n", testEncConfig->mode_map_fileName);
                return FALSE;
            }
        }
    }

    if (testEncConfig->wp_param_flag & 0x1) {
        if (testEncConfig->wp_param_fileName) {
            ChangePathStyle(testEncConfig->wp_param_fileName);
            if ((testEncConfig->wp_param_file = osal_fopen(testEncConfig->wp_param_fileName, "r")) == NULL) {
                VLOG(ERR, "fail to open Weight Param file, %s\n", testEncConfig->wp_param_fileName);
                return FALSE;
            }
        }
    }


    *done = TRUE;

    return TRUE;
}

static void ReleaseEncoder(ComponentImpl* com)
{
    // Nothing to do
}

static BOOL DestroyEncoder(ComponentImpl* com)
{
    EncoderContext* ctx = (EncoderContext*)com->context;
    Uint32          i   = 0;
    BOOL            success = TRUE;
    ENC_INT_STATUS  intStatus;

    if ( NULL == ctx )
        return FALSE;
    if ( ctx && ctx->handle) {
        LoadBalancerRemoveInstance(ctx->handle->instIndex);
    }

    while (VPU_EncClose(ctx->handle) == RETCODE_VPU_STILL_RUNNING) {
        if ((intStatus = HandlingInterruptFlag(com)) == ENC_INT_STATUS_TIMEOUT) {
            HandleEncoderError(ctx->handle, ctx->frameIdx, NULL);
            VLOG(ERR, "NO RESPONSE FROM VPU_EncClose2()\n");
            success = FALSE;
            break;
        }
        else if (intStatus == ENC_INT_STATUS_DONE) {
            EncOutputInfo   outputInfo;
            VLOG(INFO, "VPU_EncClose() : CLEAR REMAIN INTERRUPT\n");
            VPU_EncGetOutputInfo(ctx->handle, &outputInfo);
            continue;
        }

        osal_msleep(10);
    }

    ComponentNotifyListeners(com, COMPONENT_EVENT_ENC_CLOSE, NULL);

    for (i = 0; i < ctx->fbCount.srcFbNum; i++) {
        if (ctx->vbCustomMap[i].size)
            vdi_free_dma_memory(ctx->testEncConfig.coreIdx, &ctx->vbCustomMap[i], ENC_ETC, ctx->handle->instIndex);
    }
    if (ctx->vbCustomLambda.size)
        vdi_free_dma_memory(ctx->testEncConfig.coreIdx, &ctx->vbCustomLambda, ENC_ETC, ctx->handle->instIndex);
    if (ctx->vbScalingList.size)
        vdi_free_dma_memory(ctx->testEncConfig.coreIdx, &ctx->vbScalingList, ENC_ETC, ctx->handle->instIndex);

    if (ctx->testEncConfig.roi_file)
        osal_fclose(ctx->testEncConfig.roi_file);
    if (ctx->testEncConfig.lambda_map_file)
        osal_fclose(ctx->testEncConfig.lambda_map_file);
    if (ctx->testEncConfig.mode_map_file)
        osal_fclose(ctx->testEncConfig.mode_map_file);
    if (ctx->testEncConfig.scaling_list_file)
        osal_fclose(ctx->testEncConfig.scaling_list_file);
    if (ctx->testEncConfig.custom_lambda_file)
        osal_fclose(ctx->testEncConfig.custom_lambda_file);
    if (ctx->testEncConfig.wp_param_file)
        osal_fclose(ctx->testEncConfig.wp_param_file);

    if (ctx->encOutQ) Queue_Destroy(ctx->encOutQ);

    VPU_DeInit(ctx->testEncConfig.coreIdx);

    osal_free(ctx);

    return success;
}

static Component CreateEncoder(ComponentImpl* com, CNMComponentConfig* componentParam)
{
    EncoderContext* ctx;
    RetCode         retCode;
    Uint32          coreIdx      = componentParam->testEncConfig.coreIdx;
    Uint16*         firmware     = (Uint16*)componentParam->bitcode;
    Uint32          firmwareSize = componentParam->sizeOfBitcode;
    Uint32          i;
    VpuAttr         productInfo;

    retCode = VPU_InitWithBitcode(coreIdx, firmware, firmwareSize);
    if (retCode != RETCODE_SUCCESS && retCode != RETCODE_CALLED_BEFORE) {
        VLOG(INFO, "%s:%d Failed to VPU_InitWidthBitCode, ret(%08x)\n", __FUNCTION__, __LINE__, retCode);
        return FALSE;
    }

    com->context = osal_malloc(sizeof(EncoderContext));
    ctx     = (EncoderContext*)com->context;
    osal_memset((void*)ctx, 0, sizeof(EncoderContext));

    retCode = PrintVpuProductInfo(coreIdx, &productInfo);
    if (retCode == RETCODE_VPU_RESPONSE_TIMEOUT ) {
        CNMErrorSet(CNM_ERROR_HANGUP);
        VLOG(INFO, "<%s:%d> Failed to PrintVpuProductInfo()\n", __FUNCTION__, __LINE__);
        HandleEncoderError(ctx->handle, 0, NULL);
        return FALSE;
    }
    ctx->cyclePerTick = 32768;
    if (TRUE == productInfo.supportNewTimer)
        ctx->cyclePerTick = 256;

    ctx->handle                      = NULL;
    ctx->frameIdx                    = 0;
    ctx->fbCount.reconFbNum          = 0;
    ctx->fbCount.srcFbNum            = 0;
    ctx->testEncConfig               = componentParam->testEncConfig;
    ctx->encOpenParam                = componentParam->encOpenParam;
    for (i=0; i<ENC_SRC_BUF_NUM ; i++ ) {
        ctx->encodedSrcFrmIdxArr[i] = 0;
    }
    osal_memset(&ctx->vbCustomLambda,  0x00, sizeof(vpu_buffer_t));
    osal_memset(&ctx->vbScalingList,   0x00, sizeof(vpu_buffer_t));
    osal_memset(&ctx->scalingList,     0x00, sizeof(UserScalingList));
    osal_memset(&ctx->customLambda[0], 0x00, sizeof(ctx->customLambda));
    osal_memset(ctx->vbCustomMap,      0x00, sizeof(ctx->vbCustomMap));
    if (ctx->encOpenParam.ringBufferEnable)
        com->numSinkPortQueue = 10;
    else
        com->numSinkPortQueue = componentParam->encOpenParam.streamBufCount;

#ifdef SUPPORT_LOOK_AHEAD_RC
    ctx->larcEnable = ctx->encOpenParam.EncStdParam.waveParam.larcEnable;
    if (TRUE == ctx->larcEnable) {
        ctx->larcPass   = ctx->encOpenParam.EncStdParam.waveParam.larcPass;
        ctx->larcSize   = ctx->encOpenParam.EncStdParam.waveParam.larcSize;
        osal_memset((void*)ctx->larcInfo.larcData, -1, sizeof(ctx->larcInfo.larcData));
    }
#endif

    return (Component)com;
}

ComponentImpl waveEncoderComponentImpl = {
    "wave_encoder",
    NULL,
    {0,},
    {0,},
    sizeof(PortContainerES),
    5,                       /* encoder's numSinkPortQueue(relates to streambufcount) */
    CreateEncoder,
    GetParameterEncoder,
    SetParameterEncoder,
    PrepareEncoder,
    ExecuteEncoder,
    ReleaseEncoder,
    DestroyEncoder
};

