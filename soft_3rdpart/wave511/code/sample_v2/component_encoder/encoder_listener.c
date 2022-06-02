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

#include "cnm_app.h"
#include "encoder_listener.h"
#include "misc/debug.h"
#include "misc/bw_monitor.h"



static void HandleEncHandlingIntEvent(Component com, CNMComListenerHandlingInt* param, EncListenerContext* ctx)
{
    if (ctx->bwCtx != NULL) {
        BWMonitorUpdate(ctx->bwCtx, 1);
    }
}

void HandleEncFullEvent(Component com, CNMComListenerEncFull* param, EncListenerContext* ctx)
{
}

void HandleEncGetEncCloseEvent(Component com, CNMComListenerEncClose* param, EncListenerContext* ctx)
{
    if (ctx->pfCtx != NULL) {
        PFMonitorRelease(ctx->pfCtx);
    }
    if (ctx->bwCtx != NULL) {
        BWMonitorRelease(ctx->bwCtx);
    }
}

void HandleEncCompleteSeqEvent(Component com, CNMComListenerEncCompleteSeq* param, EncListenerContext* ctx)
{
    if (ctx->performance == TRUE) {
        Uint32 fps = (ctx->fps == 0) ? 30 : ctx->fps;
        ctx->pfCtx = PFMonitorSetup(ctx->coreIdx, 0, ctx->pfClock, fps, GetBasename((const char *)ctx->cfgFileName), 1);
    }
    if (ctx->bandwidth == TRUE) {
        ctx->bwCtx = BWMonitorSetup(param->handle, TRUE, GetBasename((const char *)ctx->cfgFileName));
    }
}

void HandleEncGetOutputEvent(Component com, CNMComListenerEncDone* param, EncListenerContext* ctx)
{
    EncOutputInfo* output = param->output;
    if (output->reconFrameIndex == RECON_IDX_FLAG_ENC_END)
        return;


    if (ctx->pfCtx != NULL) {
        PFMonitorUpdate(ctx->coreIdx, ctx->pfCtx, output->frameCycle, output->encPrepareEndTick - output->encPrepareStartTick,
            output->encProcessingEndTick - output->encProcessingStartTick, output->encEncodeEndTick- output->encEncodeStartTick);
    }
    if (ctx->bwCtx != NULL) {
        BWMonitorUpdatePrint(ctx->bwCtx, output->picType);
    }

    if (ctx->headerEncDone[param->handle->instIndex] == FALSE) {
        ctx->headerEncDone[param->handle->instIndex] = TRUE;
    }

    if (ctx->match == FALSE) CNMAppStop();
}

void Coda9HandleEncGetOutputEvent(Component com, CNMComListenerEncDone* param, EncListenerContext* ctx)
{
    EncOutputInfo* output = param->output;
    EncHandle      handle = param->handle;

    if (output->reconFrameIndex == RECON_IDX_FLAG_ENC_END) return;

    if (ctx->pfCtx != NULL) {
        PFMonitorUpdate(ctx->coreIdx, ctx->pfCtx, output->frameCycle, output->encPrepareEndTick - output->encPrepareStartTick,
            output->encProcessingEndTick - output->encProcessingStartTick, output->encEncodeEndTick- output->encEncodeStartTick);
    }
    if (ctx->bwCtx != NULL) {
        BWMonitorUpdatePrint(ctx->bwCtx, output->picType);
    }

    if (output->bitstreamSize > 0) {
        if (ctx->es != NULL) {
            Uint8*          buf      = NULL;
            int             compSize = output->bitstreamSize;
            PhysicalAddress addr;

            if (param->fullInterrupted == TRUE) {
                PhysicalAddress rd, wr;
                VPU_EncGetBitstreamBuffer(param->handle, &rd, &wr, &compSize);
                addr     = rd;
            }
            else {
                addr     = output->bitstreamBuffer;
                compSize = output->bitstreamSize;
            }

            buf = (Uint8*)osal_malloc(compSize);
            vdi_read_memory(ctx->coreIdx, addr, buf, compSize, ctx->streamEndian);

            if ( ctx->es != NULL ) {
                if ((ctx->match=Comparator_Act(ctx->es, buf, compSize)) == FALSE) {
                    VLOG(ERR, "<%s:%d> INSTANCE #%d Bitstream Mismatch\n", __FUNCTION__, __LINE__, handle->instIndex);
                }
            }
            osal_free(buf);
        }
    }
    else if (TRUE == param->encodedStreamInfo.ringBufferEnable && NULL != param->encodedStreamInfo.encodedStreamBuf) {

        if ( ctx->es != NULL ) {
            if ((ctx->match=Comparator_Act(ctx->es, param->encodedStreamInfo.encodedStreamBuf, param->encodedStreamInfo.encodedStreamBufLength)) == FALSE) {
                VLOG(ERR, "<%s:%d> INSTANCE #%d Bitstream Mismatch\n", __FUNCTION__, __LINE__, handle->instIndex);
            }
        }
    }

    if (ctx->headerEncDone[param->handle->instIndex] == FALSE) {
        ctx->headerEncDone[param->handle->instIndex] = TRUE;
    }

    if (ctx->match == FALSE) CNMAppStop();
}

void Coda9HandleEncMakeHeaderEvent(Component com, CNMComListenerEncMakeHeader* param, EncListenerContext* ctx)
{
    EncodedHeaderBufInfo encHeaderInfo = param->encHeaderInfo;
    if (0 < encHeaderInfo.encodedHeaderBufSize && NULL != encHeaderInfo.encodedHeaderBuf) {
        if (NULL != ctx->es) {
                if ((ctx->match=Comparator_Act(ctx->es, encHeaderInfo.encodedHeaderBuf, encHeaderInfo.encodedHeaderBufSize)) == FALSE) {
                    VLOG(ERR, "<%s:%d> Header Mismatch\n", __FUNCTION__, __LINE__, param->handle->instIndex);
                    VLOG(ERR, "<%s:%d> INSTANCE #%d Bitstream Mismatch\n", __FUNCTION__, __LINE__, param->handle->instIndex);
                }
        }
        if (encHeaderInfo.fp) {
            osal_fwrite(encHeaderInfo.encodedHeaderBuf, encHeaderInfo.encodedHeaderBufSize, 1, encHeaderInfo.fp);
            osal_fflush(encHeaderInfo.fp);
        }
    } else {
        VLOG(WARN, "<%s:%d> INSTANCE #%d Empty Header\n", __FUNCTION__, __LINE__, param->handle->instIndex);
    }

    if (ctx->match == FALSE) CNMAppStop();
}

void EncoderListener(Component com, Uint64 event, void* data, void* context)
{
    int         productId;
    EncHandle   handle;
#if defined(SUPPORT_MULTI_INSTANCE_TEST) || defined(SUPPORT_LOOK_AHEAD_RC)
#else
    /* int         key=0;

    if (osal_kbhit()) {
        key = osal_getch();
        osal_flush_ch();
        if (key) {
            if (key == 'q' || key == 'Q') {
                CNMAppStop();
                return;
            }
        }
    } */
#endif
    switch (event) {
    case COMPONENT_EVENT_ENC_OPEN:
        break;
    case COMPONENT_EVENT_ENC_ISSUE_SEQ:
        break;
    case COMPONENT_EVENT_ENC_COMPLETE_SEQ:
        HandleEncCompleteSeqEvent(com, (CNMComListenerEncCompleteSeq*)data, (EncListenerContext*)context);
        break;
    case COMPONENT_EVENT_ENC_REGISTER_FB:
        break;
    case COMPONENT_EVENT_CODA9_ENC_MAKE_HEADER:
        Coda9HandleEncMakeHeaderEvent(com, (CNMComListenerEncMakeHeader*)data, (EncListenerContext*)context);
        break;
    case COMPONENT_EVENT_ENC_READY_ONE_FRAME:
        break;
    case COMPONENT_EVENT_ENC_START_ONE_FRAME:
        break;
    case COMPONENT_EVENT_ENC_HANDLING_INT:
        HandleEncHandlingIntEvent(com, (CNMComListenerHandlingInt*)data, (EncListenerContext*)context);
        break;
    case COMPONENT_EVENT_ENC_GET_OUTPUT_INFO:
        handle = ((CNMComListenerEncDone*)data)->handle;
        productId = VPU_GetProductId(VPU_HANDLE_CORE_INDEX(handle));
        if (TRUE == PRODUCT_ID_W_SERIES(productId)) {
            HandleEncGetOutputEvent(com, (CNMComListenerEncDone*)data, (EncListenerContext*)context);
        }
        else {
            Coda9HandleEncGetOutputEvent(com, (CNMComListenerEncDone*)data, (EncListenerContext*)context);
        }
        break;
    case COMPONENT_EVENT_ENC_ENCODED_ALL:
        break;
    case COMPONENT_EVENT_ENC_CLOSE:
        HandleEncGetEncCloseEvent(com, (CNMComListenerEncClose*)data, (EncListenerContext*)context);
        break;
    case COMPONENT_EVENT_ENC_FULL_INTERRUPT:
        HandleEncFullEvent(com, (CNMComListenerEncFull*)data, (EncListenerContext*)context);
        break;
    default:
        break;
    }
}

BOOL SetupEncListenerContext(EncListenerContext* ctx, CNMComponentConfig* config)
{
    TestEncConfig* encConfig = &config->testEncConfig;

    osal_memset((void*)ctx, 0x00, sizeof(EncListenerContext));

    if (encConfig->compareType & (1 << MODE_COMP_ENCODED)) {
        if ((ctx->es=Comparator_Create(STREAM_COMPARE, encConfig->ref_stream_path, encConfig->cfgFileName)) == NULL) {
            VLOG(ERR, "%s:%d Failed to Comparator_Create(%s)\n", __FUNCTION__, __LINE__, encConfig->ref_stream_path);
            return FALSE;
        }
    }
    ctx->coreIdx       = encConfig->coreIdx;
    ctx->streamEndian  = encConfig->stream_endian;
    ctx->match         = TRUE;
    ctx->matchOtherInfo= TRUE;
    ctx->performance   = encConfig->performance;
    ctx->bandwidth     = encConfig->bandwidth;
    ctx->pfClock       = encConfig->pfClock;
    osal_memcpy(ctx->cfgFileName, encConfig->cfgFileName, sizeof(ctx->cfgFileName));
    ctx->ringBufferEnable     = encConfig->ringBufferEnable;
    ctx->ringBufferWrapEnable = encConfig->ringBufferWrapEnable;

    return TRUE;
}

void ClearEncListenerContext(EncListenerContext* ctx)
{
    if (ctx->es)    Comparator_Destroy(ctx->es);
}

