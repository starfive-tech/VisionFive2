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
#include "cnm_app.h"
#include "misc/debug.h"


#define EXTRA_FRAME_BUFFER_NUM 1

typedef enum {
    DEC_INT_STATUS_NONE,        // Interrupt not asserted yet
    DEC_INT_STATUS_EMPTY,       // Need more es
    DEC_INT_STATUS_DONE,        // Interrupt asserted
    DEC_INT_STATUS_TIMEOUT,     // Interrupt not asserted during given time.
} DEC_INT_STATUS;

typedef enum {
    DEC_STATE_NONE,
    DEC_STATE_OPEN_DECODER,
    DEC_STATE_INIT_SEQ,
    DEC_STATE_REGISTER_FB,
    DEC_STATE_DECODING,
    DEC_STATE_CLOSE,
} DecoderState;

typedef struct {
    TestDecConfig       testDecConfig;
    DecOpenParam        decOpenParam;
    DecParam            decParam;
    FrameBufferFormat   wtlFormat;
    DecHandle           handle;
    Uint64              startTimeout;
    vpu_buffer_t        vbUserData;
    BOOL                doFlush;
    BOOL                stateDoing;
    DecoderState        state;
    DecInitialInfo      initialInfo;
    Uint32              numDecoded;             /*!<< The number of decoded frames */
    Uint32              numOutput;
    PhysicalAddress     decodedAddr;
    Uint32              frameNumToStop;
    BOOL                doReset;
    Uint32              cyclePerTick;
    VpuAttr             attr;
    struct {
        BOOL    enable;
        Uint32  skipCmd;                        /*!<< a skip command to be restored */
    }                   autoErrorRecovery;
} DecoderContext;

static BOOL RegisterFrameBuffers(ComponentImpl* com)
{
    DecoderContext*         ctx               = (DecoderContext*)com->context;
    FrameBuffer*            pFrame            = NULL;
    Uint32                  framebufStride    = 0;
    ParamDecFrameBuffer     paramFb;
    RetCode                 result;
    DecInitialInfo*         codecInfo         = &ctx->initialInfo;
    BOOL                    success;
    CNMComponentParamRet    ret;
    CNMComListenerDecRegisterFb  lsnpRegisterFb;

    ctx->stateDoing = TRUE;
    ret = ComponentGetParameter(com, com->sinkPort.connectedComponent, GET_PARAM_RENDERER_FRAME_BUF, (void*)&paramFb);
    if (ComponentParamReturnTest(ret, &success) == FALSE) {
        return success;
    }

    pFrame               = paramFb.fb;
    framebufStride       = paramFb.stride;
    VLOG(TRACE, "<%s> COMPRESSED: %d, LINEAR: %d\n", __FUNCTION__, paramFb.nonLinearNum, paramFb.linearNum);

    if (ctx->attr.productId == PRODUCT_ID_521 && ctx->attr.supportDualCore == TRUE) {
        if (ctx->initialInfo.lumaBitdepth == 8 && ctx->initialInfo.chromaBitdepth == 8)
            result = VPU_DecRegisterFrameBufferEx(ctx->handle, pFrame, paramFb.nonLinearNum, paramFb.linearNum, framebufStride, codecInfo->picHeight, COMPRESSED_FRAME_MAP_DUAL_CORE_8BIT);
        else
            result = VPU_DecRegisterFrameBufferEx(ctx->handle, pFrame, paramFb.nonLinearNum, paramFb.linearNum, framebufStride, codecInfo->picHeight, COMPRESSED_FRAME_MAP_DUAL_CORE_10BIT);
    }
    else {
        result = VPU_DecRegisterFrameBufferEx(ctx->handle, pFrame, paramFb.nonLinearNum, paramFb.linearNum, framebufStride, codecInfo->picHeight, COMPRESSED_FRAME_MAP);
    }

    lsnpRegisterFb.handle          = ctx->handle;
    lsnpRegisterFb.numNonLinearFb  = paramFb.nonLinearNum;
    lsnpRegisterFb.numLinearFb     = paramFb.linearNum;
    ComponentNotifyListeners(com, COMPONENT_EVENT_DEC_REGISTER_FB, (void*)&lsnpRegisterFb);

    if (result != RETCODE_SUCCESS) {
        VLOG(ERR, "%s:%d Failed to VPU_DecRegisterFrameBufferEx(%d)\n", __FUNCTION__, __LINE__, result);
        ChekcAndPrintDebugInfo(ctx->handle, FALSE, result);
        return FALSE;
    }

    ctx->stateDoing = FALSE;

    return TRUE;
}

static BOOL SequenceChange(ComponentImpl* com, DecOutputInfo* outputInfo)
{
    DecoderContext* ctx               = (DecoderContext*)com->context;
    BOOL            dpbChanged, sizeChanged, bitDepthChanged;
    Uint32          sequenceChangeFlag = outputInfo->sequenceChanged;

    dpbChanged      = (sequenceChangeFlag&SEQ_CHANGE_ENABLE_DPB_COUNT) ? TRUE : FALSE;
    sizeChanged     = (sequenceChangeFlag&SEQ_CHANGE_ENABLE_SIZE)      ? TRUE : FALSE;
    bitDepthChanged = (sequenceChangeFlag&SEQ_CHANGE_ENABLE_BITDEPTH)  ? TRUE : FALSE;

    if (dpbChanged || sizeChanged || bitDepthChanged) {
        DecInitialInfo  initialInfo;

        VLOG(INFO, "----- SEQUENCE CHANGED -----\n");
        // Get current(changed) sequence information.
        VPU_DecGiveCommand(ctx->handle, DEC_GET_SEQ_INFO, &initialInfo);
        // Flush all remaining framebuffers of previous sequence.

        VLOG(INFO, "sequenceChanged : %x\n", sequenceChangeFlag);
        VLOG(INFO, "SEQUENCE NO     : %d\n", initialInfo.sequenceNo);
        VLOG(INFO, "DPB COUNT       : %d\n", initialInfo.minFrameBufferCount);
        VLOG(INFO, "BITDEPTH        : LUMA(%d), CHROMA(%d)\n", initialInfo.lumaBitdepth, initialInfo.chromaBitdepth);
        VLOG(INFO, "SIZE            : WIDTH(%d), HEIGHT(%d)\n", initialInfo.picWidth, initialInfo.picHeight);

        ComponentSetParameter(com, com->sinkPort.connectedComponent, SET_PARAM_RENDERER_FREE_FRAMEBUFFERS, (void*)&outputInfo->frameDisplayFlag);

        VPU_DecGiveCommand(ctx->handle, DEC_RESET_FRAMEBUF_INFO, NULL);

        if (ctx->testDecConfig.scaleDownWidth > 0 || ctx->testDecConfig.scaleDownHeight > 0) {
            ScalerInfo sclInfo = {0};

            sclInfo.scaleWidth  = CalcScaleDown(initialInfo.picWidth, ctx->testDecConfig.scaleDownWidth);
            sclInfo.scaleHeight = CalcScaleDown(initialInfo.picHeight, ctx->testDecConfig.scaleDownHeight);
            VLOG(INFO, "[SCALE INFO] %dx%d to %dx%d\n", initialInfo.picWidth, initialInfo.picHeight, sclInfo.scaleWidth, sclInfo.scaleHeight);
            sclInfo.enScaler    = TRUE;
            if (VPU_DecGiveCommand(ctx->handle, DEC_SET_SCALER_INFO, (void*)&sclInfo) != RETCODE_SUCCESS) {
                VLOG(ERR, "Failed to VPU_DecGiveCommand(DEC_SET_SCALER_INFO)\n");
                return FALSE;
            }
        }
        ctx->state = DEC_STATE_REGISTER_FB;
        osal_memcpy((void*)&ctx->initialInfo, (void*)&initialInfo, sizeof(DecInitialInfo));
        ComponentSetParameter(com, com->sinkPort.connectedComponent, SET_PARAM_RENDERER_ALLOC_FRAMEBUFFERS, NULL);

        VLOG(INFO, "----------------------------\n");
    }

    return TRUE;
}

static BOOL CheckAndDoSequenceChange(ComponentImpl* com, DecOutputInfo* outputInfo)
{
    if (outputInfo->sequenceChanged == 0) {
        return TRUE;
    }
    else {
        return SequenceChange(com, outputInfo);
    }
}

static void ClearDpb(ComponentImpl* com, BOOL backupDpb)
{
    DecoderContext* ctx             = (DecoderContext*)com->context;
    Uint32          timeoutCount;
    Int32           intReason;
    DecOutputInfo   outputInfo;
    BOOL            pause;
    Uint32          idx;
    Uint32          flushedFbs      = 0;
    QueueStatusInfo cqInfo;
    const Uint32    flushTimeout    = 100;

    if (TRUE == backupDpb) {
        pause = TRUE;
        ComponentSetParameter(com, com->sinkPort.connectedComponent, SET_PARAM_COM_PAUSE, (void*)&pause);
    }

    /* Send the renderer the signal to drop all frames.
     * VPU_DecClrDispFlag() is called in SE_PARAM_RENDERER_FLUSH.
     */
    ComponentSetParameter(com, com->sinkPort.connectedComponent, SET_PARAM_RENDERER_FLUSH, (void*)&flushedFbs);

    while (RETCODE_SUCCESS == VPU_DecGetOutputInfo(ctx->handle, &outputInfo)) {
        if (0 <= outputInfo.indexFrameDisplay) {
            flushedFbs |= outputInfo.indexFrameDisplay;
            VPU_DecClrDispFlag(ctx->handle, outputInfo.indexFrameDisplay);
            VLOG(INFO, "<%s> FLUSH DPB INDEX: %d\n", __FUNCTION__, outputInfo.indexFrameDisplay);
        }
        osal_msleep(1);
    }

    VLOG(INFO, "========== FLUSH FRAMEBUFFER & CMDs ========== \n");
    timeoutCount = 0;
    while (VPU_DecFrameBufferFlush(ctx->handle, NULL, NULL) == RETCODE_VPU_STILL_RUNNING) {
        /* Clear an interrupt */
        if (0 < (intReason=VPU_WaitInterruptEx(ctx->handle, VPU_WAIT_TIME_OUT_CQ))) {
            VPU_ClearInterruptEx(ctx->handle, intReason);
            VPU_DecGetOutputInfo(ctx->handle, &outputInfo);  // ignore the return value and outputinfo
            if (0 <= outputInfo.indexFrameDisplay) {
                flushedFbs |= outputInfo.indexFrameDisplay;
                VPU_DecClrDispFlag(ctx->handle, outputInfo.indexFrameDisplay);
            }
        }

        if (timeoutCount >= flushTimeout) {
            VLOG(ERR, "NO RESPONSE FROM VPU_DecFrameBufferFlush()\n");
            return;
        }
        timeoutCount++;
    }

    VPU_DecGetOutputInfo(ctx->handle, &outputInfo);
    VPU_DecGiveCommand(ctx->handle, DEC_GET_QUEUE_STATUS, &cqInfo);
    VLOG(INFO, "<%s> REPORT_QUEUE(%d), INSTANCE_QUEUE(%d)\n", __FUNCTION__, cqInfo.reportQueueCount, cqInfo.instanceQueueCount);

    if (TRUE == backupDpb) {
        for (idx=0; idx<32; idx++) {
            if (flushedFbs & (1<<idx)) {
                VLOG(INFO, "SET DISPLAY FLAG : %d\n", idx);
                VPU_DecGiveCommand(ctx->handle, DEC_SET_DISPLAY_FLAG , &idx);
            }
        }
        pause = FALSE;
        ComponentSetParameter(com, com->sinkPort.connectedComponent, SET_PARAM_COM_PAUSE, (void*)&pause);
    }
}

static void ClearCpb(ComponentImpl* com)
{
    DecoderContext* ctx = (DecoderContext*)com->context;
    PhysicalAddress curRdPtr, curWrPtr;

    if (BS_MODE_INTERRUPT == ctx->decOpenParam.bitstreamMode) {
        /* Clear CPB */
        // In order to stop processing bitstream.
        VLOG(INFO, "CLEAR CPB\n");
        VPU_DecUpdateBitstreamBuffer(ctx->handle, EXPLICIT_END_SET_FLAG);
        VPU_DecGetBitstreamBuffer(ctx->handle, &curRdPtr, &curWrPtr, NULL);
        VPU_DecSetRdPtr(ctx->handle, curWrPtr, TRUE);
    }
}

static BOOL PrepareSkip(ComponentImpl* com)
{
    DecoderContext*   ctx = (DecoderContext*)com->context;

    // Flush the decoder
    if (ctx->doFlush == TRUE) {
        ClearDpb(com, FALSE);
        ClearCpb(com);
        ctx->doFlush = FALSE;
    }

    return TRUE;
}

static DEC_INT_STATUS HandlingInterruptFlag(ComponentImpl* com)
{
    DecoderContext*      ctx               = (DecoderContext*)com->context;
    DecHandle            handle            = ctx->handle;
    Int32                interruptFlag     = 0;
    Uint32               interruptWaitTime = VPU_WAIT_TIME_OUT_CQ;
    Uint32               interruptTimeout  = VPU_DEC_TIMEOUT;
    DEC_INT_STATUS       status            = DEC_INT_STATUS_NONE;
    CNMComListenerDecInt lsn;

    if (ctx->startTimeout == 0ULL) {
        ctx->startTimeout = osal_gettime();
    }
    do {
        interruptFlag = VPU_WaitInterruptEx(handle, interruptWaitTime);
        if (INTERRUPT_TIMEOUT_VALUE == interruptFlag) {
            Uint64   currentTimeout = osal_gettime();
            if (0 < interruptTimeout && (currentTimeout - ctx->startTimeout) > interruptTimeout) {
                VLOG(ERR, "\n INSNTANCE #%d INTERRUPT TIMEOUT.\n", handle->instIndex);
                status = DEC_INT_STATUS_TIMEOUT;
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
            status = DEC_INT_STATUS_DONE;
            if (interruptFlag & (1<<INT_WAVE5_INIT_SEQ)) {
                break;
            }

            if (interruptFlag & (1<<INT_WAVE5_DEC_PIC)) {
                break;
            }

            if (interruptFlag & (1<<INT_WAVE5_BSBUF_EMPTY)) {
                status = DEC_INT_STATUS_EMPTY;
                break;
            }
        }
    } while (FALSE);

    if (interruptFlag != 0) {
        lsn.handle   = handle;
        lsn.flag     = interruptFlag;
        lsn.decIndex = ctx->numDecoded;
        ComponentNotifyListeners(com, COMPONENT_EVENT_DEC_INTERRUPT, (void*)&lsn);
    }

    return status;
}

static BOOL DoReset(ComponentImpl* com)
{
    DecoderContext* ctx    = (DecoderContext*)com->context;
    DecHandle       handle = ctx->handle;
    BOOL            pause  = TRUE;


    VLOG(INFO, "========== %s ==========\n", __FUNCTION__);

    ComponentSetParameter(com, com->srcPort.connectedComponent, SET_PARAM_COM_PAUSE, (void*)&pause);
    ComponentSetParameter(com, com->srcPort.connectedComponent, SET_PARAM_FEEDER_RESET, (void*)&pause);

    ClearDpb(com, FALSE);

    VLOG(INFO, "> Reset VPU\n");
    if (VPU_SWReset(handle->coreIdx, SW_RESET_SAFETY, handle) != RETCODE_SUCCESS) {
        VLOG(ERR, "<%s:%d> Failed to VPU_SWReset()\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    ClearCpb(com);

    VLOG(INFO, "========== %s ==========\n", __FUNCTION__);

    ComponentNotifyListeners(com, COMPONENT_EVENT_DEC_RESET_DONE, NULL);

    VLOG(INFO, "> FLUSH INPUT PORT\n");
    ComponentPortFlush(com);

    pause = FALSE;
    ComponentSetParameter(com, com->srcPort.connectedComponent, SET_PARAM_COM_PAUSE, (void*)&pause);

    ctx->doReset                    = FALSE;
    ctx->startTimeout               = 0ULL;
    ctx->autoErrorRecovery.enable   = TRUE;
    ctx->autoErrorRecovery.skipCmd  = ctx->decParam.skipframeMode;
    ctx->decParam.skipframeMode     = WAVE_SKIPMODE_NON_IRAP;

    return TRUE;
}

static BOOL Decode(ComponentImpl* com, PortContainerES* in, PortContainerDisplay* out)
{
    DecoderContext*                 ctx           = (DecoderContext*)com->context;
    DecOutputInfo                   decOutputInfo;
    DEC_INT_STATUS                  intStatus;
    CNMComListenerStartDecOneFrame  lsnpPicStart;
    BitStreamMode                   bsMode        = ctx->decOpenParam.bitstreamMode;
    RetCode                         result;
    CNMComListenerDecDone           lsnpPicDone;
    CNMComListenerDecReadyOneFrame  lsnpReadyOneFrame = {0,};
    BOOL                            doDecode;
    QueueStatusInfo                 qStatus;

    lsnpReadyOneFrame.handle = ctx->handle;
    ComponentNotifyListeners(com, COMPONENT_EVENT_DEC_READY_ONE_FRAME, (void*)&lsnpReadyOneFrame);

    ctx->stateDoing = TRUE;

    if (PrepareSkip(com) == FALSE) {
        return FALSE;
    }

    /* decode a frame except when the bitstream mode is PIC_END and no data */
    doDecode  = !(bsMode == BS_MODE_PIC_END && in == NULL);
    doDecode &= (BOOL)(com->pause == FALSE);

    VPU_DecGiveCommand(ctx->handle, DEC_GET_QUEUE_STATUS, &qStatus);
    if (COMMAND_QUEUE_DEPTH == qStatus.instanceQueueCount) {
        doDecode = FALSE;
    }


    /* The simple load balancer : To use this function, call InitLoadBalancer() before decoding process. */
    if (TRUE == doDecode) {
        doDecode = LoadBalancerGetMyTurn(ctx->handle->instIndex);
    }

    if (TRUE == doDecode) {
        result = VPU_DecStartOneFrame(ctx->handle, &ctx->decParam);

        lsnpPicStart.result   = result;
        lsnpPicStart.decParam = ctx->decParam;
        ComponentNotifyListeners(com, COMPONENT_EVENT_DEC_START_ONE_FRAME, (void*)&lsnpPicStart);

        if (result == RETCODE_SUCCESS) {
            /* The simple load balancer : pass its turn */
            LoadBalancerSetNextTurn();
        }
        else if (result == RETCODE_QUEUEING_FAILURE) {
            // Just retry
            if (in) in->reuse = (bsMode == BS_MODE_PIC_END);
            VPU_DecGiveCommand(ctx->handle, DEC_GET_QUEUE_STATUS, (void*)&qStatus);
        }
        else if (result == RETCODE_VPU_RESPONSE_TIMEOUT) {
            VLOG(INFO, "<%s:%d> Failed to VPU_DecStartOneFrame() ret(%d)\n", __FUNCTION__, __LINE__, result);
            CNMErrorSet(CNM_ERROR_HANGUP);
            HandleDecoderError(ctx->handle, ctx->numDecoded, NULL);
            return FALSE;
        }
        else {
            ChekcAndPrintDebugInfo(ctx->handle, FALSE, result);
            return FALSE;
        }
    }
    else {
        if (in) in->reuse = (bsMode == BS_MODE_PIC_END);
    }

    intStatus=HandlingInterruptFlag(com);
    switch (intStatus) {
    case DEC_INT_STATUS_TIMEOUT:
        DoReset(com);
        return FALSE;
    case DEC_INT_STATUS_EMPTY:
    case DEC_INT_STATUS_NONE:
        return TRUE;
    default:
        break;
    }

    // Get data from the sink component.
    if ((result=VPU_DecGetOutputInfo(ctx->handle, &decOutputInfo)) == RETCODE_SUCCESS) {
        DisplayDecodedInformation(ctx->handle, ctx->decOpenParam.bitstreamFormat, ctx->numDecoded, &decOutputInfo, ctx->testDecConfig.performance, ctx->cyclePerTick);
    }

    lsnpPicDone.handle     = ctx->handle;
    lsnpPicDone.ret        = result;
    lsnpPicDone.decParam   = &ctx->decParam;
    lsnpPicDone.output     = &decOutputInfo;
    lsnpPicDone.numDecoded = ctx->numDecoded;
    lsnpPicDone.vbUser     = ctx->vbUserData;
    ComponentNotifyListeners(com, COMPONENT_EVENT_DEC_GET_OUTPUT_INFO, (void*)&lsnpPicDone);

    if (result == RETCODE_REPORT_NOT_READY) {
        return TRUE; // Not decoded yet. Try again
    }
    else if (result != RETCODE_SUCCESS) {
        /* ERROR */
        VLOG(ERR, "Failed to decode error\n");
        ChekcAndPrintDebugInfo(ctx->handle, FALSE, result);
        return FALSE;
    }

    if ((decOutputInfo.decodingSuccess & 0x01) == 0) {
        VLOG(ERR, "VPU_DecGetOutputInfo decode fail framdIdx %d error(0x%08x) reason(0x%08x), reasonExt(0x%08x)\n",
            ctx->numDecoded, decOutputInfo.decodingSuccess, decOutputInfo.errorReason, decOutputInfo.errorReasonExt);
        if (WAVE5_SYSERR_WATCHDOG_TIMEOUT == decOutputInfo.errorReason || WAVE5_SYSERR_DEC_VLC_BUF_FULL == decOutputInfo.errorReason) {
            VLOG(ERR, "WAVE5_SYSERR_WATCHDOG_TIMEOUT\n");
        }
        else if ( WAVE5_SYSERR_DEC_VLC_BUF_FULL == decOutputInfo.errorReason) {
            VLOG(ERR, "VLC_BUFFER FULL\n");
        }
        else if (decOutputInfo.errorReason == WAVE5_SPECERR_OVER_PICTURE_WIDTH_SIZE || decOutputInfo.errorReason == WAVE5_SPECERR_OVER_PICTURE_HEIGHT_SIZE) {
            VLOG(ERR, "Not supported Width or Height(%dx%d)\n", decOutputInfo.decPicWidth, decOutputInfo.decPicHeight);
            return FALSE;
        }
    }
    else {
        if (TRUE == ctx->autoErrorRecovery.enable) {
            ctx->decParam.skipframeMode     = ctx->autoErrorRecovery.skipCmd;
            ctx->autoErrorRecovery.enable   = FALSE;
            ctx->autoErrorRecovery.skipCmd  = WAVE_SKIPMODE_WAVE_NONE;
        }
    }

    if (CheckAndDoSequenceChange(com, &decOutputInfo) == FALSE) {
        return FALSE;
    }

    if (decOutputInfo.indexFrameDecoded >= 0 || decOutputInfo.indexFrameDecoded == DECODED_IDX_FLAG_SKIP) {
        // Return a used data to a source port.
        ctx->numDecoded++;
        ctx->decodedAddr = decOutputInfo.bytePosFrameStart;
        out->reuse = FALSE;
    }

    if ((decOutputInfo.indexFrameDisplay >= 0) || (decOutputInfo.indexFrameDisplay == DISPLAY_IDX_FLAG_SEQ_END)) {
        ctx->numOutput++;
        out->last  = (BOOL)(decOutputInfo.indexFrameDisplay == DISPLAY_IDX_FLAG_SEQ_END);
        out->reuse = FALSE;
    }
    if (decOutputInfo.indexFrameDisplay == DISPLAY_IDX_FLAG_SEQ_END) {
        ctx->stateDoing = FALSE;
        com->terminate  = TRUE;
    }
    if (out->reuse == FALSE) {
        osal_memcpy((void*)&out->decInfo, (void*)&decOutputInfo, sizeof(DecOutputInfo));
    }

    if (ctx->frameNumToStop > 0) {
        if (ctx->frameNumToStop == ctx->numOutput) {
            com->terminate = TRUE;
        }
    }

    return TRUE;
}

static CNMComponentParamRet GetParameterDecoder(ComponentImpl* from, ComponentImpl* com, GetParameterCMD commandType, void* data)
{
    DecoderContext*             ctx     = (DecoderContext*)com->context;
    BOOL                        result  = TRUE;
    PhysicalAddress             rdPtr, wrPtr;
    Uint32                      room;
    ParamDecBitstreamBufPos*    bsPos   = NULL;
    ParamDecNeedFrameBufferNum* fbNum;
    ParamVpuStatus*             status;
    QueueStatusInfo             cqInfo;
    PortContainerES*            container;
    vpu_buffer_t                vb;

    if (ctx->handle == NULL)  return CNM_COMPONENT_PARAM_NOT_READY;
    if (ctx->doReset == TRUE) return CNM_COMPONENT_PARAM_NOT_READY;

    switch(commandType) {
    case GET_PARAM_COM_IS_CONTAINER_CONUSUMED:
        // This query command is sent from the comonponent core.
        // If input data are consumed in sequence, it should return TRUE through PortContainer::consumed.
        container = (PortContainerES*)data;
        vb = container->buf;
        if (vb.phys_addr <= ctx->decodedAddr && ctx->decodedAddr < (vb.phys_addr+vb.size)) {
            container->consumed = TRUE;
            ctx->decodedAddr = 0;
        }
        break;
    case GET_PARAM_DEC_HANDLE:
        *(DecHandle*)data = ctx->handle;
        break;
    case GET_PARAM_DEC_FRAME_BUF_NUM:
        if (ctx->state <= DEC_STATE_INIT_SEQ) return CNM_COMPONENT_PARAM_NOT_READY;
        fbNum = (ParamDecNeedFrameBufferNum*)data;
        fbNum->nonLinearNum = ctx->initialInfo.minFrameBufferCount + EXTRA_FRAME_BUFFER_NUM;   // max_dec_pic_buffering
        if (ctx->decOpenParam.wtlEnable == TRUE) {
            fbNum->linearNum = (ctx->initialInfo.frameBufDelay+1) + EXTRA_FRAME_BUFFER_NUM;     // The frameBufDelay can be zero.
            if ((ctx->decOpenParam.bitstreamFormat == STD_VP9) || (ctx->decOpenParam.bitstreamFormat == STD_AVS2) || (ctx->decOpenParam.bitstreamFormat == STD_AV1)) {
                fbNum->linearNum = fbNum->nonLinearNum;
            }
            if (ctx->testDecConfig.performance == TRUE) {
                if ((ctx->decOpenParam.bitstreamFormat == STD_VP9) || (ctx->decOpenParam.bitstreamFormat == STD_AVS2)) {
                    fbNum->linearNum++;
                    fbNum->nonLinearNum++;
                } else if (ctx->decOpenParam.bitstreamFormat == STD_AV1) {
                    fbNum->linearNum++;
                    fbNum->nonLinearNum++;
                }
                else {
                    fbNum->linearNum += 3;
                }
            }

        }
        else {
            fbNum->linearNum = 0;
        }
#ifdef USE_FEEDING_METHOD_BUFFER
        fbNum->linearNum = 5;
#endif
        break;
    case GET_PARAM_DEC_BITSTREAM_BUF_POS:
        if (ctx->state < DEC_STATE_INIT_SEQ) return CNM_COMPONENT_PARAM_NOT_READY;
        VPU_DecGetBitstreamBuffer(ctx->handle, &rdPtr, &wrPtr, &room);
        bsPos = (ParamDecBitstreamBufPos*)data;
        bsPos->rdPtr = rdPtr;
        bsPos->wrPtr = wrPtr;
        bsPos->avail = room;
        break;
    case GET_PARAM_DEC_CODEC_INFO:
        if (ctx->state <= DEC_STATE_INIT_SEQ) return CNM_COMPONENT_PARAM_NOT_READY;
        VPU_DecGiveCommand(ctx->handle, DEC_GET_SEQ_INFO, data);
        break;
    case GET_PARAM_VPU_STATUS:
        if (ctx->state != DEC_STATE_DECODING) return CNM_COMPONENT_PARAM_NOT_READY;
        VPU_DecGiveCommand(ctx->handle, DEC_GET_QUEUE_STATUS, &cqInfo);
        status = (ParamVpuStatus*)data;
        status->cq = cqInfo;
        break;
    default:
        result = FALSE;
        break;
    }

    return (result == TRUE) ? CNM_COMPONENT_PARAM_SUCCESS : CNM_COMPONENT_PARAM_FAILURE;
}

static CNMComponentParamRet SetParameterDecoder(ComponentImpl* from, ComponentImpl* com, SetParameterCMD commandType, void* data)
{
    BOOL                result  = TRUE;
    DecoderContext*     ctx     = (DecoderContext*)com->context;
    Int32               skipCmd;
    ParamDecTargetTid*  tid     = NULL;

    switch(commandType) {
    case SET_PARAM_COM_PAUSE:
        com->pause   = *(BOOL*)data;
        break;
    case SET_PARAM_DEC_SKIP_COMMAND:
        skipCmd = *(Int32*)data;
        ctx->decParam.skipframeMode = skipCmd;
        if (skipCmd == WAVE_SKIPMODE_NON_IRAP) {
            Uint32 userDataMask = (1<<H265_USERDATA_FLAG_RECOVERY_POINT);
            ctx->decParam.craAsBlaFlag = TRUE;
            /* For finding recovery point */
            VPU_DecGiveCommand(ctx->handle, ENABLE_REP_USERDATA, &userDataMask);
        }
        else {
            ctx->decParam.craAsBlaFlag = FALSE;
        }
        if (ctx->numDecoded > 0) {
            ctx->doFlush = (BOOL)(ctx->decParam.skipframeMode == WAVE_SKIPMODE_NON_IRAP);
        }
        break;
    case SET_PARAM_DEC_RESET:
        ctx->doReset = TRUE;
        break;
    case SET_PARAM_DEC_TARGET_TID:
        tid = (ParamDecTargetTid*)data;
        /*! NOTE: DO NOT CHANGE THE ORDER OF COMMANDS BELOW. */
        VPU_DecGiveCommand(ctx->handle, DEC_SET_TEMPORAL_ID_MODE,   (void*)&tid->tidMode);
        VPU_DecGiveCommand(ctx->handle, DEC_SET_TARGET_TEMPORAL_ID, (void*)&tid->targetTid);
        break;
    case SET_PARAM_DEC_FLUSH:
        ClearDpb(com, TRUE);
        break;
    default:
        result = FALSE;
        break;
    }

    return (result == TRUE) ? CNM_COMPONENT_PARAM_SUCCESS : CNM_COMPONENT_PARAM_FAILURE;
}

static BOOL UpdateBitstream(DecoderContext* ctx, PortContainerES* in)
{
    RetCode       ret    = RETCODE_SUCCESS;
    BitStreamMode bsMode = ctx->decOpenParam.bitstreamMode;
    BOOL          update = TRUE;
    Uint32        updateSize;

    if (in == NULL) return TRUE;

    if (bsMode == BS_MODE_PIC_END) {
        VPU_DecSetRdPtr(ctx->handle, in->buf.phys_addr, TRUE);
    }
    else {
        if (in->size > 0) {
            PhysicalAddress rdPtr, wrPtr;
            Uint32          room;
            VPU_DecGetBitstreamBuffer(ctx->handle, &rdPtr, &wrPtr, &room);
            if ((Int32)room < in->size) {
                in->reuse = TRUE;
                return TRUE;
            }
        }
    }

    if (in->last == TRUE) {
        updateSize = (in->size == 0) ? STREAM_END_SET_FLAG : in->size;
    }
    else {
        updateSize = in->size;
        update     = (in->size > 0 && in->last == FALSE);
    }

    if (update == TRUE) {
        if ((ret=VPU_DecUpdateBitstreamBuffer(ctx->handle, updateSize)) != RETCODE_SUCCESS) {
            VLOG(INFO, "<%s:%d> Failed to VPU_DecUpdateBitstreamBuffer() ret(%d)\n", __FUNCTION__, __LINE__, ret);
            ChekcAndPrintDebugInfo(ctx->handle, FALSE, ret);
            return FALSE;
        }
        if (in->last == TRUE) {
            VPU_DecUpdateBitstreamBuffer(ctx->handle, STREAM_END_SET_FLAG);
        }
    }

    in->reuse = FALSE;

    return TRUE;
}

static BOOL OpenDecoder(ComponentImpl* com)
{
    DecoderContext*         ctx     = (DecoderContext*)com->context;
    ParamDecBitstreamBuffer bsBuf;
    CNMComponentParamRet    ret;
    CNMComListenerDecOpen   lspn    = {0};
    BOOL                    success = FALSE;
    BitStreamMode           bsMode  = ctx->decOpenParam.bitstreamMode;
    vpu_buffer_t            vbUserData;
    RetCode                 retCode;

    ctx->stateDoing = TRUE;
    ret = ComponentGetParameter(com, com->srcPort.connectedComponent, GET_PARAM_FEEDER_BITSTREAM_BUF, &bsBuf);
    if (ComponentParamReturnTest(ret, &success) == FALSE) {
        return success;
    }

    ctx->decOpenParam.bitstreamBuffer     = (bsMode == BS_MODE_PIC_END) ? 0 : bsBuf.bs->phys_addr;
    ctx->decOpenParam.bitstreamBufferSize = (bsMode == BS_MODE_PIC_END) ? 0 : bsBuf.bs->size;
    retCode = VPU_DecOpen(&ctx->handle, &ctx->decOpenParam);


    lspn.handle = ctx->handle;
    lspn.ret    = retCode;
    ComponentNotifyListeners(com, COMPONENT_EVENT_DEC_OPEN, (void*)&lspn);

    if (retCode != RETCODE_SUCCESS) {
        VLOG(ERR, "<%s:%d> Failed to VPU_DecOpen(ret:%d)\n", __FUNCTION__, __LINE__, retCode);
        if ( retCode == RETCODE_VPU_RESPONSE_TIMEOUT)
            CNMErrorSet(CNM_ERROR_HANGUP);

        return FALSE;
    }
    LoadBalancerAddInstance(ctx->handle->instIndex);
    // VPU_DecGiveCommand(ctx->handle, ENABLE_LOGGING, 0);

    if (ctx->vbUserData.size == 0) {
        vbUserData.size = (1320*1024);  /* 40KB * (queue_depth + report_queue_depth+1) = 40KB * (16 + 16 +1) */
        vdi_allocate_dma_memory(ctx->testDecConfig.coreIdx, &vbUserData, DEC_ETC, ctx->handle->instIndex);
    }
    VPU_DecGiveCommand(ctx->handle, SET_ADDR_REP_USERDATA, (void*)&vbUserData.phys_addr);
    VPU_DecGiveCommand(ctx->handle, SET_SIZE_REP_USERDATA, (void*)&vbUserData.size);
    VPU_DecGiveCommand(ctx->handle, ENABLE_REP_USERDATA,   (void*)&ctx->testDecConfig.enableUserData);

    VPU_DecGiveCommand(ctx->handle, SET_CYCLE_PER_TICK,   (void*)&ctx->cyclePerTick);
    if (ctx->testDecConfig.thumbnailMode == TRUE) {
        VPU_DecGiveCommand(ctx->handle, ENABLE_DEC_THUMBNAIL_MODE, NULL);
    }


    ctx->vbUserData = vbUserData;
    ctx->stateDoing = FALSE;

    return TRUE;
}

static BOOL DecodeHeader(ComponentImpl* com, PortContainerES* in, BOOL* done)
{
    DecoderContext*                ctx     = (DecoderContext*)com->context;
    DecHandle                      handle  = ctx->handle;
    Uint32                         coreIdx = ctx->testDecConfig.coreIdx;
    RetCode                        ret     = RETCODE_SUCCESS;
    DEC_INT_STATUS                 status;
    DecInitialInfo*                initialInfo = &ctx->initialInfo;
    SecAxiUse                      secAxiUse;
    CNMComListenerDecCompleteSeq   lsnpCompleteSeq;
    DecInfo*                       pDecInfo = VPU_HANDLE_TO_DECINFO(ctx->handle);

    *done = FALSE;

    if (BS_MODE_PIC_END == ctx->decOpenParam.bitstreamMode && NULL == in) {
        return TRUE;
    }

    if (ctx->stateDoing == FALSE) {
        /* previous state done */
        ret = VPU_DecIssueSeqInit(handle);
        if (RETCODE_QUEUEING_FAILURE == ret) {
            return TRUE; // Try again
        }
        ComponentNotifyListeners(com, COMPONENT_EVENT_DEC_ISSUE_SEQ, NULL);
        if (ret != RETCODE_SUCCESS) {
            ChekcAndPrintDebugInfo(ctx->handle, FALSE, ret);
            VLOG(ERR, "%s:%d Failed to VPU_DecIssueSeqInit() ret(%d)\n", __FUNCTION__, __LINE__, ret);
            return FALSE;
        }
    }

    ctx->stateDoing = TRUE;

    while (com->terminate == FALSE) {
        if ((status=HandlingInterruptFlag(com)) == DEC_INT_STATUS_DONE) {
            break;
        }
        else if (status == DEC_INT_STATUS_TIMEOUT) {
            HandleDecoderError(ctx->handle, 0, NULL);
            VPU_DecUpdateBitstreamBuffer(handle, STREAM_END_SIZE);    /* To finish bitstream empty status */
            VPU_SWReset(coreIdx, SW_RESET_SAFETY, handle);
            VPU_DecUpdateBitstreamBuffer(handle, STREAM_END_CLEAR_FLAG);    /* To finish bitstream empty status */
            return FALSE;
        }
        else if (status == DEC_INT_STATUS_EMPTY) {
            return TRUE;
        }
        else if (status == DEC_INT_STATUS_NONE) {
            return TRUE;
        }
        else {
            VLOG(INFO, "%s:%d Unknown interrupt status: %d\n", __FUNCTION__, __LINE__, status);
            return FALSE;
        }
    }

    ret = VPU_DecCompleteSeqInit(handle, initialInfo);
    if (pDecInfo->productCode == WAVE521C_DUAL_CODE) {
        if ( initialInfo->lumaBitdepth == 8 && initialInfo->chromaBitdepth == 8) {
            ctx->testDecConfig.mapType = COMPRESSED_FRAME_MAP_DUAL_CORE_8BIT;
        }
        else {
            ctx->testDecConfig.mapType = COMPRESSED_FRAME_MAP_DUAL_CORE_10BIT;
        }
    }

    strcpy(lsnpCompleteSeq.refYuvPath, ctx->testDecConfig.refYuvPath);
    lsnpCompleteSeq.ret             = ret;
    lsnpCompleteSeq.initialInfo     = initialInfo;
    lsnpCompleteSeq.wtlFormat       = ctx->wtlFormat;
    lsnpCompleteSeq.cbcrInterleave  = ctx->decOpenParam.cbcrInterleave;
    lsnpCompleteSeq.bitstreamFormat = ctx->decOpenParam.bitstreamFormat;
    ComponentNotifyListeners(com, COMPONENT_EVENT_DEC_COMPLETE_SEQ, (void*)&lsnpCompleteSeq);

    if (ret != RETCODE_SUCCESS) {
        VLOG(ERR, "%s:%d FAILED TO DEC_PIC_HDR: ret(%d), SEQERR(%08x)\n", __FUNCTION__, __LINE__, ret, initialInfo->seqInitErrReason);
        ChekcAndPrintDebugInfo(ctx->handle, FALSE, ret);
        return FALSE;
    }

    if (ctx->decOpenParam.wtlEnable == TRUE) {
        VPU_DecGiveCommand(ctx->handle, DEC_SET_WTL_FRAME_FORMAT, &ctx->wtlFormat);
    }

   /* Set up the secondary AXI is depending on H/W configuration.
    * Note that turn off all the secondary AXI configuration
    * if target ASIC has no memory only for IP, LF and BIT.
    */
    secAxiUse.u.wave.useIpEnable    = (ctx->testDecConfig.secondaryAXI&0x01) ? TRUE : FALSE;
    secAxiUse.u.wave.useLfRowEnable = (ctx->testDecConfig.secondaryAXI&0x02) ? TRUE : FALSE;
    secAxiUse.u.wave.useBitEnable   = (ctx->testDecConfig.secondaryAXI&0x04) ? TRUE : FALSE;
    secAxiUse.u.wave.useSclEnable   = (ctx->testDecConfig.secondaryAXI&0x08) ? TRUE : FALSE;
    VPU_DecGiveCommand(ctx->handle, SET_SEC_AXI, &secAxiUse);
    // Set up scale
    if (ctx->testDecConfig.scaleDownWidth > 0 || ctx->testDecConfig.scaleDownHeight > 0) {
        ScalerInfo sclInfo = {0};

        sclInfo.scaleWidth  = ctx->testDecConfig.scaleDownWidth;
        sclInfo.scaleHeight = ctx->testDecConfig.scaleDownHeight;
        VLOG(INFO, "[SCALE INFO] %dx%d\n", sclInfo.scaleWidth, sclInfo.scaleHeight);
        sclInfo.enScaler    = TRUE;
        if (VPU_DecGiveCommand(ctx->handle, DEC_SET_SCALER_INFO, (void*)&sclInfo) != RETCODE_SUCCESS) {
            VLOG(ERR, "Failed to VPU_DecGiveCommand(DEC_SET_SCALER_INFO)\n");
            return FALSE;
        }
    }

    ctx->stateDoing = FALSE;
    *done = TRUE;

    return TRUE;
}

static BOOL ExecuteDecoder(ComponentImpl* com, PortContainer* in , PortContainer* out)
{
    DecoderContext* ctx    = (DecoderContext*)com->context;
    BOOL            ret    = FALSE;;
    BitStreamMode   bsMode = ctx->decOpenParam.bitstreamMode;
    BOOL            done   = FALSE;

    if (in)  in->reuse = TRUE;
    if (out) out->reuse = TRUE;
    if (ctx->state == DEC_STATE_INIT_SEQ || ctx->state == DEC_STATE_DECODING) {
        if (UpdateBitstream(ctx, (PortContainerES*)in) == FALSE) {
            return FALSE;
        }
        if (in) {
            // In ring-buffer mode, it has to return back a container immediately.
            if (bsMode == BS_MODE_PIC_END) {
                if (ctx->state == DEC_STATE_INIT_SEQ) {
                    in->reuse = TRUE;
                }
                in->consumed = FALSE;
            }
            else {
                in->consumed = (in->reuse == FALSE);
            }
        }
    }

    if (ctx->doReset == TRUE) {
        if (in) in->reuse = FALSE;
        return DoReset(com);
    }

    switch (ctx->state) {
    case DEC_STATE_OPEN_DECODER:
        ret = OpenDecoder(com);
        if (ctx->stateDoing == FALSE) ctx->state = DEC_STATE_INIT_SEQ;
        break;
    case DEC_STATE_INIT_SEQ:
        ret = DecodeHeader(com, (PortContainerES*)in, &done);
        if (TRUE == done) ctx->state = DEC_STATE_REGISTER_FB;
        break;
    case DEC_STATE_REGISTER_FB:
        ret = RegisterFrameBuffers(com);
        if (ctx->stateDoing == FALSE) {
            ctx->state = DEC_STATE_DECODING;
            DisplayDecodedInformation(ctx->handle, ctx->decOpenParam.bitstreamFormat, 0, NULL, ctx->testDecConfig.performance, 0);
        }
        break;
    case DEC_STATE_DECODING:
        ret = Decode(com, (PortContainerES*)in, (PortContainerDisplay*)out);
        break;
    default:
        ret = FALSE;
        break;
    }

    if (ret == FALSE || com->terminate == TRUE) {
        ComponentNotifyListeners(com, COMPONENT_EVENT_DEC_DECODED_ALL, (void*)ctx->handle);
        if (out) {
            out->reuse = FALSE;
            out->last  = TRUE;
        }
    }

    return ret;
}

static BOOL PrepareDecoder(ComponentImpl* com, BOOL* done)
{
    *done = TRUE;

    return TRUE;
}

static void ReleaseDecoder(ComponentImpl* com)
{
    // Nothing to do
}

static BOOL DestroyDecoder(ComponentImpl* com)
{
    DecoderContext* ctx         = (DecoderContext*)com->context;
    DEC_INT_STATUS  intStatus;
    BOOL            success     = TRUE;
    Uint32          timeout     = 0;
    Uint32          i           = 0;

    if (NULL != ctx->handle) {
        LoadBalancerRemoveInstance(ctx->handle->instIndex);
        VPU_DecUpdateBitstreamBuffer(ctx->handle, STREAM_END_SET_FLAG);
        while (VPU_DecClose(ctx->handle) == RETCODE_VPU_STILL_RUNNING) {
            if ((intStatus=HandlingInterruptFlag(com)) == DEC_INT_STATUS_TIMEOUT) {
                HandleDecoderError(ctx->handle, ctx->numDecoded, NULL);
                VLOG(ERR, "<%s:%d> NO RESPONSE FROM VPU_DecClose()\n", __FUNCTION__, __LINE__);
                success = FALSE;
                break;
            }
            else if (intStatus == DEC_INT_STATUS_DONE) {
                DecOutputInfo outputInfo;
                VLOG(INFO, "VPU_DecClose() : CLEAR REMAIN INTERRUPT\n");
                VPU_DecGetOutputInfo(ctx->handle, &outputInfo);
                continue;
            }

            if (timeout > VPU_BUSY_CHECK_TIMEOUT) {
                VLOG(ERR, "<%s:%d> Failed to VPU_DecClose\n", __FUNCTION__, __LINE__);
            }

            for (i=0; i<MAX_REG_FRAME; i++) {
                VPU_DecClrDispFlag(ctx->handle, i);
            }

            timeout++;
        }
        ComponentNotifyListeners(com, COMPONENT_EVENT_DEC_CLOSE, NULL);
    }

    if (ctx->vbUserData.size) {
        vdi_free_dma_memory(ctx->testDecConfig.coreIdx, &ctx->vbUserData, DEC_ETC, ctx->handle->instIndex);
    }


    VPU_DeInit(ctx->decOpenParam.coreIdx);

    osal_free(ctx);

    return success;
}

static Component CreateDecoder(ComponentImpl* com, CNMComponentConfig* componentParam)
{
    DecoderContext* ctx;
    Uint32          coreIdx      = componentParam->testDecConfig.coreIdx;
    Uint16*         firmware     = (Uint16*)componentParam->bitcode;
    Uint32          firmwareSize = componentParam->sizeOfBitcode;
    RetCode         retCode;

    retCode = VPU_InitWithBitcode(coreIdx, firmware, firmwareSize);
    if (retCode != RETCODE_SUCCESS && retCode != RETCODE_CALLED_BEFORE) {
        VLOG(INFO, "%s:%d Failed to VPU_InitiWithBitcode, ret(%08x)\n", __FUNCTION__, __LINE__, retCode);
        return FALSE;
    }

    com->context = (DecoderContext*)osal_malloc(sizeof(DecoderContext));
    osal_memset(com->context, 0, sizeof(DecoderContext));
    ctx = (DecoderContext*)com->context;

    retCode = PrintVpuProductInfo(coreIdx, &ctx->attr);
    if (retCode == RETCODE_VPU_RESPONSE_TIMEOUT ) {
        CNMErrorSet(CNM_ERROR_HANGUP);
        VLOG(INFO, "<%s:%d> Failed to PrintVpuProductInfo()\n", __FUNCTION__, __LINE__);
        HandleDecoderError(ctx->handle, ctx->numDecoded, NULL);
        return FALSE;
    }
    ctx->cyclePerTick = 32768;
    if (TRUE == ctx->attr.supportNewTimer)
        ctx->cyclePerTick = 256;

    memcpy(&(ctx->decOpenParam), &(componentParam->decOpenParam), sizeof(DecOpenParam));

    ctx->wtlFormat                    = componentParam->testDecConfig.wtlFormat;
    ctx->frameNumToStop               = componentParam->testDecConfig.forceOutNum;
    ctx->testDecConfig                = componentParam->testDecConfig;
    ctx->state                        = DEC_STATE_OPEN_DECODER;
    ctx->stateDoing                   = FALSE;
    VLOG(INFO, "PRODUCT ID: %d\n", ctx->attr.productId);

    return (Component)com;
}

ComponentImpl waveDecoderComponentImpl = {
    "wave_decoder",
    NULL,
    {0,},
    {0,},
    sizeof(PortContainerDisplay),
    5,
    CreateDecoder,
    GetParameterDecoder,
    SetParameterDecoder,
    PrepareDecoder,
    ExecuteDecoder,
    ReleaseDecoder,
    DestroyDecoder
};

