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
#include "misc/bw_monitor.h"
#include "sys/time.h"

#define EXTRA_FRAME_BUFFER_NUM 1
#define INDEX_FRAME_DIS_QUEUE_SIZE 255
#define MAX_SIZE_HEADER_DATA 255

typedef enum {
    DEC_INT_STATUS_NONE,        // Interrupt not asserted yet
    DEC_INT_STATUS_EMPTY,       // Need more es
    DEC_INT_STATUS_DONE,        // Interrupt asserted
    DEC_INT_STATUS_TIMEOUT,     // Interrupt not asserted during given time.
    DEC_INT_STATUS_FIELD,       // One filed picture(top or bottom) was decoded(need a next field picture)
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
    BOOL    isHdrData;
    Int32   hdrDataSize;
    Uint8   hdrData[MAX_SIZE_HEADER_DATA];
} HeaderData;

typedef struct {
    TestDecConfig               testDecConfig;
    DecOpenParam                decOpenParam;
    DecParam                    decParam;
    FrameBufferFormat           wtlFormat;
    DecHandle                   handle;
    Uint64                      startTimeout;
    vpu_buffer_t                vbUserData;
    BOOL                        doFlush;
    BOOL                        stateDoing;
    DecoderState                state;
    DecInitialInfo              initialInfo;
    Uint32                      numDecoded;             /*!<< The number of decoded frames */
    Uint32                      numOutput;
    PhysicalAddress             decodedAddr;
    Uint32                      frameNumToStop;
    BOOL                        doReset;
    Uint32                      cyclePerTick;
    ParamDecPPUFrameBuffer      ppuFbParam;
    Queue*                      idxFrameDisQ;
    BOOL                        needMoreStream;
    Int32                       linearFbCnt;
    BOOL                        seqChangeReqest;
    Int32                       seqChangedStreamEndFlag;
    Int32                       seqChangedRdPtr;
    Int32                       seqChangedWrPtr;
    BOOL                        prevScaleX;
    BOOL                        prevScaleY;
    BOOL                        isTheoraHeader;
    BOOL                        oneFiledDecoded;
    BOOL                        chunkReuseRequired;
    BOOL                        isDecodedFlag; // only used in pic_enc_mode
    HeaderData                  hdrInfo;
    ParamDecBitstreamBufPos     bsPos;
} DecoderContext;

void AppendPicHeaderData(DecoderContext* ctx, PortContainerES* streamData) {
    PhysicalAddress rdPtr, wrPtr;
    Uint32          room;
    Uint8*          pStreamBuffer = NULL;
    Int32           totalBufSize = 0;
    Int32           coreIdx = ctx->testDecConfig.coreIdx;

    //Append PPS data to bistreambuffer which will be decoded.
    VPU_DecGetBitstreamBuffer(ctx->handle ,&rdPtr, &wrPtr, &room);

    if (ctx->hdrInfo.hdrDataSize < room) {
        totalBufSize = (wrPtr - rdPtr) + ctx->hdrInfo.hdrDataSize;
        pStreamBuffer = (Uint8*)osal_malloc(totalBufSize+1);

        if (NULL != pStreamBuffer) {
            osal_memset(pStreamBuffer, 0x00, totalBufSize+1);
            osal_memcpy(pStreamBuffer, ctx->hdrInfo.hdrData, ctx->hdrInfo.hdrDataSize);
            vdi_read_memory(coreIdx, rdPtr, pStreamBuffer+ctx->hdrInfo.hdrDataSize, wrPtr - rdPtr,ctx->testDecConfig.streamEndian);
            vdi_write_memory(coreIdx, rdPtr, pStreamBuffer, totalBufSize,  ctx->testDecConfig.streamEndian);

            VPU_DecUpdateBitstreamBuffer(ctx->handle, ctx->hdrInfo.hdrDataSize);
        }
    }

    if (NULL != pStreamBuffer) {
        osal_free(pStreamBuffer);
    }
}

BOOL CheckConsumedHeaderStream(DecoderContext* ctx)
{
    PhysicalAddress rdPtr =0, wrPtr =0;
    Uint32          room =0;

    if (NULL == ctx->handle) {
        return FALSE;
    }

    VPU_DecGetBitstreamBuffer(ctx->handle, &rdPtr, &wrPtr, &room);

    if (wrPtr != rdPtr) {
        return FALSE;
    }
    return TRUE;
}

void AlignPictureSize(DecoderContext* ctx, CNMComListenerDecDone* lsnpPicDone)
{
    TestDecConfig*  decConfig = NULL;

    if (NULL == ctx || NULL == lsnpPicDone) {
        VLOG(INFO, "%s:%d NULL pointer exception\n", __FUNCTION__, __LINE__);
        return;
    }

    decConfig = &(ctx->testDecConfig);

    if (0 > lsnpPicDone->output->indexFrameDisplay) {
        return;
    }

    switch (decConfig->compareType) {
    default :
        break;
    }
}

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
    MaverickCacheConfig     cacheCfg;

    ctx->stateDoing = TRUE;

    ret = ComponentGetParameter(com, com->sinkPort.connectedComponent, GET_PARAM_RENDERER_FRAME_BUF, (void*)&paramFb);
    if (ComponentParamReturnTest(ret, &success) == FALSE) {
        return success;
    }

    MaverickCache2Config(&cacheCfg, TRUE /*Decoder*/, ctx->testDecConfig.cbcrInterleave,
                         ctx->testDecConfig.coda9.frameCacheBypass,
                         ctx->testDecConfig.coda9.frameCacheBurst,
                         ctx->testDecConfig.coda9.frameCacheMerge,
                         ctx->testDecConfig.mapType,
                         ctx->testDecConfig.coda9.frameCacheWayShape);
    VPU_DecGiveCommand(ctx->handle, SET_CACHE_CONFIG, &cacheCfg);


    pFrame               = paramFb.fb;
    framebufStride       = paramFb.stride;

    VLOG(INFO, "<%s> LINEAR: %d, Tiled: %d\n", __FUNCTION__, paramFb.linearNum, paramFb.nonLinearNum);
    ctx->linearFbCnt = paramFb.linearNum;

    result = VPU_DecRegisterFrameBuffer(ctx->handle, pFrame, paramFb.linearNum, framebufStride, codecInfo->picHeight, ctx->testDecConfig.mapType);

    lsnpRegisterFb.handle          = ctx->handle;
    lsnpRegisterFb.numLinearFb     = paramFb.linearNum;
    lsnpRegisterFb.numNonLinearFb = paramFb.nonLinearNum;
    ComponentNotifyListeners(com, COMPONENT_EVENT_DEC_REGISTER_FB, (void*)&lsnpRegisterFb);

    if (result != RETCODE_SUCCESS) {
        VLOG(ERR, "%s:%d Failed to VPU_DecRegisterFrameBufferEx(%d)\n", __FUNCTION__, __LINE__, result);
        return FALSE;
    }

    ctx->stateDoing = FALSE;

    ret = ComponentGetParameter(com, com->sinkPort.connectedComponent, GET_PARAM_RENDERER_PPU_FRAME_BUF, &(ctx->ppuFbParam));
    if (ComponentParamReturnTest(ret, &success) == FALSE) {
        return FALSE;
    }

    if(TRUE == ctx->ppuFbParam.enablePPU) {
        if(NULL != ctx->ppuFbParam.fb) {
            VPU_DecGiveCommand(ctx->handle, SET_ROTATOR_OUTPUT, ctx->ppuFbParam.fb);
            if (ctx->testDecConfig.coda9.rotate > 0) {
                VPU_DecGiveCommand(ctx->handle, ENABLE_ROTATION, NULL);
            }
            if (ctx->testDecConfig.coda9.mirror > 0) {
                VPU_DecGiveCommand(ctx->handle, ENABLE_MIRRORING, NULL);
            }
            if (ctx->testDecConfig.coda9.enableDering == TRUE) {
                VPU_DecGiveCommand(ctx->handle, ENABLE_DERING, NULL);
            }
        } else {
            VLOG(ERR, "%s:%d NULL pointer Exception - ppuFb\n", __FUNCTION__, __LINE__);
            return FALSE;
        }
    }

    PrepareDecoderTest(ctx->handle);

    return TRUE;
}

static BOOL PreSequenceChange(ComponentImpl* com, DecOutputInfo* outputInfo)
{
    DecoderContext* ctx               = (DecoderContext*)com->context;
    RetCode         ret;

    VLOG(INFO, "-----PRE SEQUENCE CHANGED -----\n");

    ctx->seqChangedRdPtr = outputInfo->rdPtr;
    ctx->seqChangedWrPtr = outputInfo->wrPtr;

    VLOG(INFO, "seqChangeRdPtr: 0x%08x, WrPtr: 0x%08x\n", ctx->seqChangedRdPtr, ctx->seqChangedWrPtr);

    if (RETCODE_SUCCESS != (ret=VPU_DecSetRdPtr(ctx->handle, ctx->seqChangedRdPtr, TRUE))) {
        VLOG(ERR, "%s:%d Failed to VPU_DecSetRdPtr(%d), ret(%d)\n", __FUNCTION__, __LINE__, ctx->seqChangedRdPtr, ret);
        return FALSE;
    }
    ctx->seqChangedStreamEndFlag = outputInfo->streamEndFlag; //BIT_BIT_STREAM_PARAM(0x114)
    VPU_DecUpdateBitstreamBuffer(ctx->handle, 1); // let f/w to know stream end condition in bitstream buffer. force to know that bitstream buffer will be empty.
    VPU_DecUpdateBitstreamBuffer(ctx->handle, STREAM_END_SET_FLAG); // set to stream end condition to pump out a delayed framebuffer.

    return TRUE;
}

static BOOL SequenceChange(ComponentImpl* com, DecOutputInfo* outputInfo)
{
    DecoderContext* ctx               = (DecoderContext*)com->context;
    DecInitialInfo  initialInfo;
    ParamDecBitstreamBuffer bsBuf;
    BOOL            resOf = TRUE;
    Uint32          comState = COMPONENT_STATE_NONE;

    VLOG(INFO, "----- SEQUENCE CHANGED -----\n");
    // Get current(changed) sequence information.
    VPU_DecGiveCommand(ctx->handle, DEC_GET_SEQ_INFO, &initialInfo);
    // Flush all remaining framebuffers of previous sequence.

    VLOG(INFO, "sequenceChanged : %x\n", outputInfo->sequenceChanged);
    VLOG(INFO, "SEQUENCE NO     : %d\n", initialInfo.sequenceNo);
    VLOG(INFO, "DPB COUNT       : %d\n", initialInfo.minFrameBufferCount);
    VLOG(INFO, "BITDEPTH        : LUMA(%d), CHROMA(%d)\n", initialInfo.lumaBitdepth, initialInfo.chromaBitdepth);
    VLOG(INFO, "SIZE            : WIDTH(%d), HEIGHT(%d)\n", initialInfo.picWidth, initialInfo.picHeight);

    ComponentSetParameter(com, com->sinkPort.connectedComponent, SET_PARAM_RENDERER_FREE_FRAMEBUFFERS, (void*)&outputInfo->frameDisplayFlag);

    VPU_DecGiveCommand(ctx->handle, DEC_RESET_FRAMEBUF_INFO, NULL);

    VPU_DecSetRdPtr(ctx->handle, ctx->seqChangedRdPtr, TRUE);

    if (ctx->seqChangedStreamEndFlag == 1) { // No more bitstream in the buffer.
        VPU_DecUpdateBitstreamBuffer(ctx->handle, STREAM_END_SET_FLAG);
    }
    else {
        VPU_DecUpdateBitstreamBuffer(ctx->handle, STREAM_END_CLEAR_FLAG);
    }
    if (ctx->seqChangedWrPtr >= ctx->seqChangedRdPtr) {
        resOf = (RETCODE_SUCCESS == VPU_DecUpdateBitstreamBuffer(ctx->handle, ctx->seqChangedWrPtr - ctx->seqChangedRdPtr));
    }
    else {
        ComponentGetParameter(com, com->srcPort.connectedComponent, GET_PARAM_FEEDER_BITSTREAM_BUF, &bsBuf);
        resOf = (RETCODE_SUCCESS == VPU_DecUpdateBitstreamBuffer(ctx->handle, (bsBuf.bs->phys_addr + bsBuf.bs->size) - ctx->seqChangedRdPtr +
            (ctx->seqChangedWrPtr - bsBuf.bs->phys_addr)));
    }

    //ctx->state = DEC_STATE_REGISTER_FB;
    ctx->state = DEC_STATE_INIT_SEQ;
    osal_memcpy((void*)&ctx->initialInfo, (void*)&initialInfo, sizeof(DecInitialInfo));

    comState = COMPONENT_STATE_CREATED;
    ComponentSetParameter(com, com->sinkPort.connectedComponent, SET_PARAM_RENDERER_CHANGE_COM_STATE, (void*)&comState);

    VLOG(INFO, "----------------------------\n");

    return resOf;
}

static BOOL CheckAndDoSequenceChange(ComponentImpl* com, DecOutputInfo* outputInfo)
{
    DecoderContext* ctx = (DecoderContext*)com->context;
    BOOL resOf = TRUE;

    if (0 != outputInfo->sequenceChanged) {
        ctx->seqChangeReqest = TRUE;
        resOf = PreSequenceChange(com, outputInfo);
    }

    if (TRUE == ctx->seqChangeReqest && DISPLAY_IDX_FLAG_SEQ_END == outputInfo->indexFrameDisplay) {
        resOf = SequenceChange(com, outputInfo);
    }

    return resOf;
}

static DEC_INT_STATUS HandlingInterruptFlagNoCQ(ComponentImpl* com, InterruptBit waitInterrptFlag)
{
    DecoderContext*      ctx               = (DecoderContext*)com->context;
    DecHandle            handle            = ctx->handle;
    Int32                interruptFlag     = 0;
    Uint32               interruptWaitTime = VPU_WAIT_TIME_OUT;
    Uint32               interruptTimeout  = VPU_DEC_TIMEOUT;
    DEC_INT_STATUS       status            = DEC_INT_STATUS_NONE;
    CNMComListenerDecInt lsn;
    BOOL                 repeat            = TRUE;

    ctx->startTimeout = osal_gettime();

    do {
        interruptFlag = VPU_WaitInterruptEx(handle, interruptWaitTime);
        if (INTERRUPT_TIMEOUT_VALUE == interruptFlag) {
            Uint64   currentTimeout = osal_gettime();
            if ((currentTimeout - ctx->startTimeout) > interruptTimeout) {
                VLOG(ERR, "\n INSNTANCE #%d INTERRUPT TIMEOUT.\n", handle->instIndex);
                status = DEC_INT_STATUS_TIMEOUT;
                break;
            }
            interruptFlag = 0;
        }

        if (interruptFlag < 0) {
            VLOG(ERR, "<%s:%d> interruptFlag is negative value! %08x\n", __FUNCTION__, __LINE__, interruptFlag);
            status = DEC_INT_STATUS_NONE;
        }

        if (interruptFlag & (1 << INT_BIT_DEC_FIELD)) {
            if (BS_MODE_PIC_END == ctx->testDecConfig.bitstreamMode) {
                status = DEC_INT_STATUS_FIELD;
                break;
            }
        }

        if (interruptFlag) {
            VPU_ClearInterruptEx(handle, interruptFlag);
        }

        if (interruptFlag & (1 << waitInterrptFlag)) {
            status = DEC_INT_STATUS_DONE;
            repeat = FALSE;
        }

        if (interruptFlag & (1 << INT_BIT_BIT_BUF_EMPTY)) {
            status = DEC_INT_STATUS_EMPTY;
            break;
        }

    } while (repeat);

    lsn.handle = handle;
    lsn.flag   = interruptFlag;
    ComponentNotifyListeners(com, COMPONENT_EVENT_DEC_INTERRUPT, (void*)&lsn);

    return status;
}

static BOOL DoReset(ComponentImpl* com)
{
    DecoderContext* ctx    = (DecoderContext*)com->context;
    BitStreamMode   bsMode = ctx->decOpenParam.bitstreamMode;
    DecHandle       handle = ctx->handle;
    Uint32          timeoutCount;

    VLOG(INFO, "========== %s ==========\n", __FUNCTION__);

    VLOG(INFO, "> FLUSH RENDERER\n");
    /* Send the renderer the signal to drop all frames. */
    ComponentSetParameter(com, com->sinkPort.connectedComponent, SET_PARAM_RENDERER_FLUSH, NULL);

    VLOG(INFO, "> EXPLICIT_END_SET_FLAG\n");
    // In order to stop processing bitstream.
    VPU_DecUpdateBitstreamBuffer(handle, EXPLICIT_END_SET_FLAG);

    // Clear DPB
    timeoutCount = 0;
    VLOG(INFO, "> Flush VPU internal buffer\n");
    while (VPU_DecFrameBufferFlush(handle, NULL, NULL) == RETCODE_VPU_STILL_RUNNING) {
        if (timeoutCount >= VPU_DEC_TIMEOUT) {
            VLOG(ERR, "NO RESPONSE FROM VPU_DecFrameBufferFlush()\n");
            return FALSE;
        }
        timeoutCount++;
    }

    VLOG(INFO, "> Reset VPU\n");
    if (VPU_SWReset(handle->coreIdx, SW_RESET_SAFETY, handle) != RETCODE_SUCCESS) {
        VLOG(ERR, "<%s:%d> Failed to VPU_SWReset()\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    // Clear CPB
    if (bsMode == BS_MODE_INTERRUPT) {
        PhysicalAddress curWrPtr;
        VLOG(INFO, "> Clear CPB\n");
        VPU_DecGetBitstreamBuffer(handle, NULL, &curWrPtr, NULL);
        VPU_DecSetRdPtr(handle, curWrPtr, TRUE);
    }

    VLOG(INFO, "> STREAM_END_CLEAR_FLAG\n");
    // Clear stream-end flag
    VPU_DecUpdateBitstreamBuffer(handle, STREAM_END_CLEAR_FLAG);

    VLOG(INFO, "========== %s ==========\n", __FUNCTION__);

    ComponentNotifyListeners(com, COMPONENT_EVENT_DEC_RESET_DONE, NULL);

    ctx->doReset      = FALSE;
    ctx->startTimeout = 0ULL;
    return TRUE;
}

static BOOL Decode(ComponentImpl* com, PortContainerES* in, PortContainerDisplay* out)
{
    DecoderContext*                 ctx           = (DecoderContext*)com->context;
    DecOutputInfo                   decOutputInfo;
    DEC_INT_STATUS                  intStatus = DEC_INT_STATUS_NONE;
    CNMComListenerStartDecOneFrame  lsnpPicStart;
    BitStreamMode                   bsMode        = ctx->decOpenParam.bitstreamMode;
    RetCode                         result;
    CNMComListenerDecDone           lsnpPicDone;
    CNMComListenerDecReadyOneFrame  lsnpReadyOneFrame = {0,};
    BOOL                            doDecode;

    lsnpReadyOneFrame.handle = ctx->handle;
    ComponentNotifyListeners(com, COMPONENT_EVENT_DEC_READY_ONE_FRAME, (void*)&lsnpReadyOneFrame);

    ctx->stateDoing = TRUE;

    /* decode a frame except when the bitstream mode is PIC_END and no data */
    doDecode  = !(bsMode == BS_MODE_PIC_END && in == NULL);
    if (FALSE == doDecode) {
        return TRUE;
    }

    if (TRUE == com->pause) {
        return TRUE;
    }

    if (BS_MODE_PIC_END == bsMode) {
        if (TRUE == ctx->hdrInfo.isHdrData) {
            ctx->hdrInfo.isHdrData = FALSE;
            AppendPicHeaderData(ctx, in);
        }
    }

    if (ctx->needMoreStream == FALSE) {
        /* Empty interrupt wasn't asserted */

        result = VPU_DecStartOneFrame(ctx->handle, &ctx->decParam);

        lsnpPicStart.handle   = ctx->handle;
        lsnpPicStart.result   = result;
        lsnpPicStart.decParam = ctx->decParam;
        ComponentNotifyListeners(com, COMPONENT_EVENT_DEC_START_ONE_FRAME, (void*)&lsnpPicStart);

        if (result != RETCODE_SUCCESS) {
            return FALSE;
        }
    }

    if (TRUE == ctx->oneFiledDecoded && BS_MODE_PIC_END == ctx->testDecConfig.bitstreamMode) {
        VPU_ClearInterrupt(ctx->testDecConfig.coreIdx);
        ctx->oneFiledDecoded = FALSE;
    }

    ctx->needMoreStream = FALSE;

    intStatus = HandlingInterruptFlagNoCQ(com, INT_BIT_PIC_RUN);
    if ((DEC_INT_STATUS_TIMEOUT == intStatus)) {
        HandleDecoderError(ctx->handle, ctx->numDecoded, NULL);
        PrintDecVpuStatus(ctx->handle);
        DoReset(com);
        return FALSE;
    }
    else if (DEC_INT_STATUS_EMPTY == intStatus) {
        ctx->needMoreStream = TRUE;
        return TRUE;
    }
    else if (DEC_INT_STATUS_FIELD == intStatus) {
        ctx->needMoreStream = TRUE;
        ctx->oneFiledDecoded = TRUE;
        if(out) out->reuse = TRUE;
        return TRUE;
    }

    //default value - chunkReuseRequired
    ctx->chunkReuseRequired = FALSE;
    ctx->isDecodedFlag = FALSE;

    // Get data from the sink component.
    if ((result=VPU_DecGetOutputInfo(ctx->handle, &decOutputInfo)) == RETCODE_SUCCESS) {
        DisplayDecodedInformation(ctx->handle, ctx->decOpenParam.bitstreamFormat, ctx->numDecoded, &decOutputInfo, ctx->testDecConfig.performance, ctx->cyclePerTick);
    }


    if (TRUE == ctx->ppuFbParam.enablePPU) {
        if (decOutputInfo.indexFrameDisplay >= 0) {
            Queue_Enqueue(ctx->idxFrameDisQ, &(decOutputInfo.indexFrameDisplay));
            if(1 == ctx->idxFrameDisQ->count) {
                if (TRUE == decOutputInfo.chunkReuseRequired) {
                    ctx->chunkReuseRequired = TRUE;
                    if (0 <= decOutputInfo.indexFrameDecoded) ctx->isDecodedFlag = TRUE;
                    if (in) in->reuse = TRUE;
                }
                return TRUE;
            }
        }
    }


    lsnpPicDone.handle     = ctx->handle;
    lsnpPicDone.ret        = result;
    lsnpPicDone.decParam   = &ctx->decParam;
    lsnpPicDone.output     = &decOutputInfo;
    lsnpPicDone.numDecoded = ctx->numDecoded;
    lsnpPicDone.vbUser     = ctx->vbUserData;

    AlignPictureSize(ctx, &lsnpPicDone);

    ComponentNotifyListeners(com, COMPONENT_EVENT_DEC_GET_OUTPUT_INFO, (void*)&lsnpPicDone);

    if (result == RETCODE_REPORT_NOT_READY) {
        return TRUE; // Not decoded yet. Try again
    }
    else if (result != RETCODE_SUCCESS) {
        /* ERROR */
        VLOG(ERR, "Failed to decode error : %d \n", result);
        return FALSE;
    }

    if ((decOutputInfo.decodingSuccess & 0x01) == 0) {
        VLOG(ERR, "VPU_DecGetOutputInfo decode fail framdIdx %d error(0x%08x) reason(0x%08x), reasonExt(0x%08x)\n",
            ctx->numDecoded, decOutputInfo.decodingSuccess, decOutputInfo.errorReason, decOutputInfo.errorReasonExt);
        return FALSE;
    }

    if (FALSE == CheckAndDoSequenceChange(com, &decOutputInfo)) {
        return FALSE;
    }
    else {
        if (TRUE == ctx->seqChangeReqest && DISPLAY_IDX_FLAG_SEQ_END == decOutputInfo.indexFrameDisplay) {
            ctx->seqChangeReqest = FALSE;
            ctx->stateDoing = FALSE; //Go back to DecodeHeader()
            return TRUE;
        }
    }

    if (decOutputInfo.indexFrameDecoded >= 0) {
        // Return a used data to a source port.
        ctx->numDecoded++;
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

    if (BS_MODE_PIC_END == ctx->testDecConfig.bitstreamMode) {
        if (DECODED_IDX_FLAG_NO_FB == decOutputInfo.indexFrameDecoded) {
            //reuse a chunk of bitstream
            if (in) in->reuse = TRUE;
        }
    }

    if (TRUE == decOutputInfo.chunkReuseRequired) {
        //reuse a chunk of bitstream
        if (in) in->reuse = TRUE;
        ctx->chunkReuseRequired = TRUE;
        if (0 <= decOutputInfo.indexFrameDecoded) {
            ctx->isDecodedFlag = TRUE;
        }
    }

    if (out->reuse == FALSE) {
        osal_memcpy((void*)&out->decInfo, (void*)&decOutputInfo, sizeof(DecOutputInfo));
        if (ctx->ppuFbParam.enablePPU && 1 < ctx->idxFrameDisQ->count) {
            if (NULL != Queue_Peek(ctx->idxFrameDisQ)) {
                out->decInfo.indexFrameDisplay = *((Int32*)Queue_Dequeue(ctx->idxFrameDisQ));
            }
        }
    }
    else { // TRUE == out->reuse
        if (ctx->ppuFbParam.enablePPU && 1 < ctx->idxFrameDisQ->count) {
            if (NULL != Queue_Peek(ctx->idxFrameDisQ) && DECODED_IDX_FLAG_NO_FB == decOutputInfo.indexFrameDecoded) {
                out->decInfo.indexFrameDisplay = *((Int32*)Queue_Dequeue(ctx->idxFrameDisQ));
                VPU_DecClrDispFlag(ctx->handle, out->decInfo.indexFrameDisplay);
            }
        }
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
    ParamDecNeedFrameBufferNum* fbNum;
    ParamVpuStatus*             status;
    QueueStatusInfo             cqInfo;
    PortContainerES*            container;
    vpu_buffer_t                vb;
    Int32                       productID = -1;

    if (NULL == ctx) {
        VLOG(ERR, "%s:%d NULL Pointer Exception\n", __FUNCTION__, __LINE__);
        return CNM_COMPONENT_PARAM_FAILURE;
    }

    productID = ctx->testDecConfig.productId;

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
        if (PRODUCT_ID_960 == productID || PRODUCT_ID_980 == productID) {
            //linear Buffer
            fbNum->linearNum = ctx->initialInfo.minFrameBufferCount + EXTRA_FRAME_BUFFER_NUM;   // max_dec_pic_buffering
            if(ctx->testDecConfig.enableWTL) {
                //num of tiled buffers are same with num of linear buffers
                //CODA series do not use compressed buffers,
                //In this case, compressNum means number of tiled buffers
                fbNum->nonLinearNum = fbNum->linearNum;
            }
            else {
                fbNum->nonLinearNum =0;
            }
        }
        break;
    case GET_PARAM_DEC_BITSTREAM_BUF_POS:
        if (0 == ctx->bsPos.rdPtr && 0 == ctx->bsPos.wrPtr) return CNM_COMPONENT_PARAM_NOT_READY;
        *(ParamDecBitstreamBufPos*)data = ctx->bsPos;
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
    BOOL            result = TRUE;
    DecoderContext* ctx    = (DecoderContext*)com->context;
    Int32           skipCmd;

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
    default:
        result = FALSE;
        break;
    }

    return (result == TRUE) ? CNM_COMPONENT_PARAM_SUCCESS : CNM_COMPONENT_PARAM_FAILURE;
}

static BOOL UpdateBitstream(DecoderContext* ctx, PortContainerES* in)
{
    RetCode         ret    = RETCODE_SUCCESS;
    BitStreamMode   bsMode = ctx->decOpenParam.bitstreamMode;
    BOOL            update = TRUE;
    Uint32          updateSize;
    PhysicalAddress rdPtr, wrPtr;
    Uint32          room;

    if (NULL == in) return TRUE;

    if (BS_MODE_PIC_END == bsMode) {
        if (TRUE == ctx->isTheoraHeader) {
            //Theora decoder need to parse bitstream before decoding
            ctx->isTheoraHeader = FALSE;
            in->reuse = FALSE;
            return TRUE;
        }

        if (FALSE == ctx->chunkReuseRequired) {
            VPU_DecSetRdPtr(ctx->handle, in->buf.phys_addr, TRUE);
        }
        else if (TRUE == ctx->chunkReuseRequired && FALSE == ctx->isDecodedFlag) {
            VPU_DecSetRdPtr(ctx->handle, in->buf.phys_addr, TRUE);
        }
    }
    else {
        if (in->size > 0) {
            VPU_DecGetBitstreamBuffer(ctx->handle, &rdPtr, &wrPtr, &room);
            if (room < in->size) {
                in->reuse = TRUE;
                return TRUE;
            }
        }
    }

    if (TRUE == in->last) {
        updateSize = (in->size == 0) ? STREAM_END_SET_FLAG : in->size;
    }
    else {
        updateSize = in->size;
        update     = (in->size > 0 && in->last == FALSE);
    }

    if (TRUE == update && FALSE == ctx->seqChangeReqest) {
        if ((ret=VPU_DecUpdateBitstreamBuffer(ctx->handle, updateSize)) != RETCODE_SUCCESS) {
            VLOG(INFO, "<%s:%d> Failed to VPU_DecUpdateBitstreamBuffer() ret(%d)\n", __FUNCTION__, __LINE__, ret);
            return FALSE;
        }
        if (TRUE == in->last) {
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
    RetCode                 retCode;
    VpuReportConfig_t       decReportCfg;

    ctx->stateDoing = TRUE;
    ret = ComponentGetParameter(com, com->srcPort.connectedComponent, GET_PARAM_FEEDER_BITSTREAM_BUF, &bsBuf);
    if (ComponentParamReturnTest(ret, &success) == FALSE) {
        return success;
    }

    ctx->decOpenParam.bitstreamBuffer     = bsBuf.bs->phys_addr;
    ctx->decOpenParam.bitstreamBufferSize = bsBuf.bs->size * bsBuf.num;

    retCode = VPU_DecOpen(&ctx->handle, &ctx->decOpenParam);

    lspn.handle = ctx->handle;
    lspn.ret    = retCode;
    ComponentNotifyListeners(com, COMPONENT_EVENT_DEC_OPEN, (void*)&lspn);

    if (retCode != RETCODE_SUCCESS) {
        VLOG(ERR, "<%s:%d> Failed to VPU_DecOpen(ret:%d)\n", __FUNCTION__, __LINE__, retCode);
        HandleDecoderError(NULL, 0, NULL);
        return FALSE;
    }

    //Enable Register logs
    //VPU_DecGiveCommand(ctx->handle, ENABLE_LOGGING, 0);

    osal_memset((void*)&decReportCfg, 0x00, sizeof(VpuReportConfig_t));
    decReportCfg.userDataEnable     = ctx->testDecConfig.enableUserData;
    decReportCfg.userDataReportMode = 1;
    OpenDecReport(ctx->testDecConfig.coreIdx, &decReportCfg);
    ConfigDecReport(ctx->testDecConfig.coreIdx, ctx->handle, ctx->testDecConfig.bitFormat);

    if (ctx->testDecConfig.thumbnailMode == TRUE) {
        VPU_DecGiveCommand(ctx->handle, ENABLE_DEC_THUMBNAIL_MODE, NULL);
    }


    ctx->stateDoing = FALSE;

    return TRUE;
}

static BOOL DecodeHeader(ComponentImpl* com, PortContainerES* in, PortContainerDisplay* out)
{
    DecoderContext*                 ctx     = (DecoderContext*)com->context;
    DecHandle                       handle  = ctx->handle;
    Uint32                          coreIdx = ctx->testDecConfig.coreIdx;
    RetCode                         ret     = RETCODE_SUCCESS;
    DEC_INT_STATUS                  status;
    DecInitialInfo*                 initialInfo = &ctx->initialInfo;
    SecAxiUse                       secAxiUse;
    CNMComListenerDecCompleteSeq    lsnpCompleteSeq;
    CodStd                          bitFormat = ctx->testDecConfig.bitFormat;
    PhysicalAddress                 rdPtr, wrPtr;
    Uint32                          room;

    if (FALSE == ctx->stateDoing) {
        ret = VPU_DecIssueSeqInit(handle);
        ComponentNotifyListeners(com, COMPONENT_EVENT_DEC_ISSUE_SEQ, NULL);
        if (ret != RETCODE_SUCCESS) {
            VLOG(ERR, "%s:%d Failed to VPU_DecIssueSeqInit() ret(%d)\n", __FUNCTION__, __LINE__, ret);
            return FALSE;
        }
    }

    ctx->stateDoing = TRUE;

    if (FALSE == com->terminate) {
        status=HandlingInterruptFlagNoCQ(com, INT_BIT_SEQ_INIT);
        if (status == DEC_INT_STATUS_TIMEOUT) {
            VPU_DecUpdateBitstreamBuffer(handle, STREAM_END_SIZE);
            VPU_SWReset(coreIdx, SW_RESET_FORCE, handle);
            VPU_DecUpdateBitstreamBuffer(handle, STREAM_END_CLEAR_FLAG);    // To finish bitstream empty status
            return FALSE;
        }
        else if (status == DEC_INT_STATUS_DONE) {
            VLOG(INFO, "[INT] INT_BIT_SEQ_INIT \n");
        }
        else if (status == DEC_INT_STATUS_NONE) {
            //just re-try
            return TRUE;
        }
        else if (status == DEC_INT_STATUS_EMPTY) {
            //it occurs only when the total picture size is smaller than chunk size.
            return TRUE;
        }
        else {
            VLOG(INFO, "%s:%d Unknown interrupt status: %d\n", __FUNCTION__, __LINE__, status);
            return FALSE;
        }
    }

    ret = VPU_DecCompleteSeqInit(handle, initialInfo);

    strcpy(lsnpCompleteSeq.refYuvPath, ctx->testDecConfig.refYuvPath);
    lsnpCompleteSeq.ret             = ret;
    lsnpCompleteSeq.initialInfo     = initialInfo;
    lsnpCompleteSeq.wtlFormat       = ctx->wtlFormat;
    lsnpCompleteSeq.cbcrInterleave  = ctx->decOpenParam.cbcrInterleave;
    lsnpCompleteSeq.bitstreamFormat = ctx->decOpenParam.bitstreamFormat;
    ComponentNotifyListeners(com, COMPONENT_EVENT_DEC_COMPLETE_SEQ, (void*)&lsnpCompleteSeq);

    if (ret != RETCODE_SUCCESS) {
        VLOG(ERR, "%s:%d FAILED TO DEC_PIC_HDR: ret(%d), SEQERR(%08x)\n", __FUNCTION__, __LINE__, ret, initialInfo->seqInitErrReason);
        return FALSE;
    }

    if (BS_MODE_PIC_END == ctx->testDecConfig.bitstreamMode) {
        // first picture includes a coded frame in sequence header of Theora, ffmpeg
        if (STD_THO != bitFormat && STD_H263 != bitFormat && STD_RV != bitFormat && STD_VP8 !=bitFormat) {
            //Parsing of Header has been done.
            if (FALSE == CheckConsumedHeaderStream(ctx)) {
                ctx->hdrInfo.isHdrData = TRUE;
                osal_memset(ctx->hdrInfo.hdrData, 0x00, MAX_SIZE_HEADER_DATA);
                VPU_DecGetBitstreamBuffer(ctx->handle ,&rdPtr, &wrPtr, &room);
                ctx->hdrInfo.hdrDataSize = wrPtr - rdPtr;
                if (MAX_SIZE_HEADER_DATA < ctx->hdrInfo.hdrDataSize) {
                    ctx->hdrInfo.hdrDataSize = 0;
                    ctx->hdrInfo.isHdrData = FALSE;
                } else {
                    vdi_read_memory(ctx->testDecConfig.coreIdx, rdPtr,  ctx->hdrInfo.hdrData, wrPtr - rdPtr,ctx->testDecConfig.streamEndian);
                    if (in) {
                        in->reuse = FALSE;
                        in->consumed = TRUE;
                    }
                }
            } else {
                if (in) {
                    in->reuse = FALSE;
                    in->consumed = TRUE;
                }
            }

        }
    }

    if (STD_THO == bitFormat) {
        // first picture includes a coded frame in sequence header of Theora, ffmpeg
        ctx->isTheoraHeader = TRUE;
    }

    if (ctx->decOpenParam.wtlEnable == TRUE) {
        VPU_DecGiveCommand(ctx->handle, DEC_SET_WTL_FRAME_FORMAT, &ctx->wtlFormat);
    }

   /* Setting up the secondary AXI is depending on H/W configuration.
    * Note that turn off all the secondary AXI configuration
    * if target ASIC has no memory only for IP, LF and BIT.
    */

    osal_memset(&secAxiUse,     0x00, sizeof(SecAxiUse));
    secAxiUse.u.coda9.useBitEnable  = (ctx->testDecConfig.secondaryAXI>>0) & 0x01;
    secAxiUse.u.coda9.useIpEnable   = (ctx->testDecConfig.secondaryAXI>>1) & 0x01;
    secAxiUse.u.coda9.useDbkYEnable = (ctx->testDecConfig.secondaryAXI>>2) & 0x01;
    secAxiUse.u.coda9.useDbkCEnable = (ctx->testDecConfig.secondaryAXI>>3) & 0x01;
    secAxiUse.u.coda9.useOvlEnable  = (ctx->testDecConfig.secondaryAXI>>4) & 0x01;
    secAxiUse.u.coda9.useBtpEnable  = (ctx->testDecConfig.secondaryAXI>>5) & 0x01;

    VPU_DecGiveCommand(ctx->handle, SET_SEC_AXI, &secAxiUse);

    ctx->stateDoing = FALSE;

    return TRUE;
}

static BOOL ExecuteDecoder(ComponentImpl* com, PortContainer* in, PortContainer* out)
{
    DecoderContext* ctx    = (DecoderContext*)com->context;
    BOOL            ret    = FALSE;
    BitStreamMode   bsMode = ctx->decOpenParam.bitstreamMode;

    if (ctx->doReset == TRUE) {
        if (DoReset(com) == FALSE) return FALSE;
    }

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
                in->consumed = (in->reuse == FALSE);
            }
            else {
                in->consumed = (in->reuse == FALSE);
            }
        } else { // NULL == in
            if (BS_MODE_PIC_END == bsMode) {
                if (DEC_STATE_INIT_SEQ == ctx->state || DEC_STATE_DECODING == ctx->state) {
                    return TRUE;
                }
            }
            else {
                if (DEC_STATE_INIT_SEQ == ctx->state) {
                    return TRUE;
                }
            }
        }
    }

    switch (ctx->state) {
    case DEC_STATE_OPEN_DECODER:
        ret = OpenDecoder(com);
        if (ctx->stateDoing == FALSE) ctx->state = DEC_STATE_INIT_SEQ;
        break;
    case DEC_STATE_INIT_SEQ:
        ret = DecodeHeader(com, (PortContainerES*)in, (PortContainerDisplay*)out);
        if (ctx->stateDoing == FALSE) ctx->state = DEC_STATE_REGISTER_FB;
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

    if (ctx->handle) {
        PhysicalAddress rdPtr, wrPtr;
        Uint32          room;

        VPU_DecGetBitstreamBuffer(ctx->handle, &rdPtr, &wrPtr, &room);
        ctx->bsPos.rdPtr = rdPtr;
        ctx->bsPos.wrPtr = wrPtr;
        ctx->bsPos.avail = room;
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
}

static BOOL DestroyDecoder(ComponentImpl* com)
{
    DecoderContext* ctx         = (DecoderContext*)com->context;
    BOOL            success     = TRUE;
    RetCode         ret         = RETCODE_SUCCESS;

    CloseDecReport (ctx->handle);

    VPU_DecUpdateBitstreamBuffer(ctx->handle, STREAM_END_SET_FLAG);

    ret = VPU_DecClose(ctx->handle);
    if (RETCODE_SUCCESS != ret) {
        VLOG(ERR, "%s:%d FAILED TO VPU_DecClose() ret(%d)\n", __FUNCTION__, __LINE__, ret);
    }

    Queue_Destroy(ctx->idxFrameDisQ);

    ComponentNotifyListeners(com, COMPONENT_EVENT_DEC_CLOSE, NULL);

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
    Int32           productId = PRODUCT_ID_NONE;

    retCode = VPU_InitWithBitcode(coreIdx, firmware, firmwareSize);
    if (retCode != RETCODE_SUCCESS && retCode != RETCODE_CALLED_BEFORE) {
        VLOG(INFO, "%s:%d Failed to VPU_InitiWithBitcode, ret(%08x)\n", __FUNCTION__, __LINE__, retCode);
        return FALSE;
    }

    com->context = (DecoderContext*)osal_malloc(sizeof(DecoderContext));
    osal_memset(com->context, 0, sizeof(DecoderContext));
    ctx = (DecoderContext*)com->context;

    productId = VPU_GetProductId(coreIdx);
    VLOG(INFO, "PRODUCT ID: %d\n", productId);

    PrintVpuVersionInfo(coreIdx);

    memcpy(&(ctx->decOpenParam), &(componentParam->decOpenParam), sizeof(DecOpenParam));

    ctx->wtlFormat                          = componentParam->testDecConfig.wtlFormat;
    ctx->frameNumToStop                     = componentParam->testDecConfig.forceOutNum;
    ctx->testDecConfig                      = componentParam->testDecConfig;
    ctx->state                              = DEC_STATE_OPEN_DECODER;
    ctx->stateDoing                         = FALSE;

    ctx->idxFrameDisQ = Queue_Create(INDEX_FRAME_DIS_QUEUE_SIZE, sizeof(Int32));

    return (Component)com;
}

ComponentImpl coda9DecoderComponentImpl = {
    "coda9_decoder",
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


