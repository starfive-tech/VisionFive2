// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2021 StarFive Technology Co., Ltd.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <unistd.h>
#include <fcntl.h>
#include "SF_OMX_video_common.h"
#include "SF_OMX_Core.h"

#define DEC_StopThread OMX_CommandMax

extern OMX_TICKS gInitTimeStamp;

typedef struct DEC_CMD
{
    OMX_COMMANDTYPE Cmd;
    OMX_U32 nParam;
    OMX_PTR pCmdData;
}DEC_CMD;

static char* Event2Str(unsigned long event)
{
    char *event_str = NULL;
    switch (event)
    {
    case COMPONENT_EVENT_DEC_OPEN:
        event_str = "COMPONENT_EVENT_DEC_OPEN";
        break;
    case COMPONENT_EVENT_DEC_ISSUE_SEQ:
        event_str = "COMPONENT_EVENT_DEC_ISSUE_SEQ";
        break;
    case COMPONENT_EVENT_DEC_COMPLETE_SEQ:
        event_str = "COMPONENT_EVENT_DEC_COMPLETE_SEQ";
        break;
    case COMPONENT_EVENT_DEC_REGISTER_FB:
        event_str = "COMPONENT_EVENT_DEC_REGISTER_FB";
        break;
    case COMPONENT_EVENT_DEC_READY_ONE_FRAME:
        event_str = "COMPONENT_EVENT_DEC_READY_ONE_FRAME";
        break;
    case COMPONENT_EVENT_DEC_START_ONE_FRAME:
        event_str = "COMPONENT_EVENT_DEC_START_ONE_FRAME";
        break;
    case COMPONENT_EVENT_DEC_INTERRUPT:
        event_str = "COMPONENT_EVENT_DEC_INTERRUPT";
        break;
    case COMPONENT_EVENT_DEC_GET_OUTPUT_INFO:
        event_str = "COMPONENT_EVENT_DEC_GET_OUTPUT_INFO";
        break;
    case COMPONENT_EVENT_DEC_DECODED_ALL:
        event_str = "COMPONENT_EVENT_DEC_DECODED_ALL";
        break;
    case COMPONENT_EVENT_DEC_CLOSE:
        event_str = "COMPONENT_EVENT_DEC_CLOSE";
        break;
    case COMPONENT_EVENT_DEC_RESET_DONE:
        event_str = "COMPONENT_EVENT_DEC_RESET_DONE";
        break;
    case COMPONENT_EVENT_DEC_EMPTY_BUFFER_DONE:
        event_str = "COMPONENT_EVENT_DEC_EMPTY_BUFFER_DONE";
        break;
    case COMPONENT_EVENT_DEC_FILL_BUFFER_DONE:
        event_str = "COMPONENT_EVENT_DEC_FILL_BUFFER_DONE";
        break;
    case COMPONENT_EVENT_DEC_ALL:
        event_str = "COMPONENT_EVENT_DEC_ALL";
        break;
    }
    return event_str;
}

static void OnEventArrived(Component com, unsigned long event, void *data, void *context)
{
    PortContainerExternal *pPortContainerExternal = (PortContainerExternal *)data;
    OMX_BUFFERHEADERTYPE *pOMXBuffer;
    OMX_BUFFERHEADERTYPE **ppBuffer;
    OMX_BUFFERHEADERTYPE *pBuffer;
    SF_OMX_BUF_INFO *pBufInfo;
    static OMX_U32 dec_cnt = 0;
    static struct timeval tv_old = {0};
    OMX_U32 fps = 0;
    OMX_U64 diff_time = 0; // ms
    FunctionIn();
    SF_OMX_COMPONENT *pSfOMXComponent = (SF_OMX_COMPONENT*)context;
    SF_WAVE5_IMPLEMEMT *pSfVideoImplement = (SF_WAVE5_IMPLEMEMT *)pSfOMXComponent->componentImpl;

    char *event_str = Event2Str(event);

    LOG(SF_LOG_INFO, "event=%lX %s\r\n", event, event_str);
    switch (event)
    {
    case COMPONENT_EVENT_DEC_EMPTY_BUFFER_DONE:
        if (pSfOMXComponent->bPortFlushing[0])
        {
            LOG(SF_LOG_INFO, "flushing,break\r\n");
            break;
        }
        pOMXBuffer = (OMX_BUFFERHEADERTYPE *)pPortContainerExternal->pAppPrivate;
        if (pOMXBuffer == NULL)
        {
            LOG(SF_LOG_WARN, "Could not find omx buffer by address\r\n");
            return;
        }

        ppBuffer = SF_Queue_Peek(pSfVideoImplement->inPortQ);

        if (ppBuffer)
            pBuffer = *ppBuffer;

        if (pBuffer == pOMXBuffer)
        {
            LOG(SF_LOG_INFO, "get buffer %p\r\n", pOMXBuffer);
            SF_Queue_Dequeue(pSfVideoImplement->inPortQ);
        }
        else
        {
            LOG(SF_LOG_INFO, "buffer %p gone, break.\r\n", pOMXBuffer);
            break;
        }

        LOG(SF_LOG_INFO, "Empty done h %p b %p\r\n", pOMXBuffer, pOMXBuffer->pBuffer);

        if (pSfOMXComponent->markType[0].hMarkTargetComponent != NULL)
        {
            LOG(SF_LOG_INFO, "set Component mark %p\r\n", pSfOMXComponent->markType[0].hMarkTargetComponent);
            pOMXBuffer->hMarkTargetComponent      = pSfOMXComponent->markType[0].hMarkTargetComponent;
            pOMXBuffer->pMarkData                 = pSfOMXComponent->markType[0].pMarkData;
            pSfOMXComponent->markType[0].hMarkTargetComponent = NULL;
            pSfOMXComponent->markType[0].pMarkData = NULL;
        }

        if (pOMXBuffer->hMarkTargetComponent != NULL)
        {
            if (pOMXBuffer->hMarkTargetComponent == pSfOMXComponent->pOMXComponent)
            {
                LOG(SF_LOG_INFO, "Component mark hit\r\n");
                pSfOMXComponent->callbacks->EventHandler(pSfOMXComponent->pOMXComponent,
                    pSfOMXComponent->pAppData,
                    OMX_EventMark,
                    0, 0, pOMXBuffer->pMarkData);
            } else {
                LOG(SF_LOG_INFO, "Component propagate mark from input port\r\n");
                pSfOMXComponent->propagateMarkType.hMarkTargetComponent = pOMXBuffer->hMarkTargetComponent;
                pSfOMXComponent->propagateMarkType.pMarkData = pOMXBuffer->pMarkData;
            }
        }

        if (pSfOMXComponent->state == OMX_StatePause)
        {
            LOG(SF_LOG_INFO, "tmp store buf when pause\r\n");
            SF_Queue_Enqueue(pSfVideoImplement->inPauseQ, &pOMXBuffer);
        }
        else{
            pSfOMXComponent->callbacks->EmptyBufferDone(pSfOMXComponent->pOMXComponent, pSfOMXComponent->pAppData, pOMXBuffer);
        }

        break;
    case COMPONENT_EVENT_DEC_FILL_BUFFER_DONE:
    {
        struct timeval tv;
        if (pSfOMXComponent->bPortFlushing[1] && !pSfOMXComponent->memory_optimization)
        {
            LOG(SF_LOG_INFO, "flushing,break\r\n");
            break;
        }

        pOMXBuffer = (OMX_BUFFERHEADERTYPE *)pPortContainerExternal->pAppPrivate;
        if (pOMXBuffer == NULL)
        {
            LOG(SF_LOG_WARN, "Could not find omx buffer by address\r\n");
            return;
        }

        if (!pSfOMXComponent->memory_optimization){
            ppBuffer = SF_Queue_Peek(pSfVideoImplement->outPortQ);

            if (ppBuffer)
                pBuffer = *ppBuffer;

            if (pBuffer == pOMXBuffer)
            {
                LOG(SF_LOG_INFO, "get buffer %p\r\n", pOMXBuffer);
                SF_Queue_Dequeue(pSfVideoImplement->outPortQ);
            }
            else
            {
                LOG(SF_LOG_INFO, "buffer %p gone, break.\r\n", pOMXBuffer);
                break;
            }
        }

        gettimeofday(&tv, NULL);
        if (gInitTimeStamp == 0)
        {
            gInitTimeStamp = tv.tv_sec * 1000000 + tv.tv_usec;
        }
        pOMXBuffer->nFilledLen = pPortContainerExternal->nFilledLen;
        pOMXBuffer->nTimeStamp = tv.tv_sec * 1000000 + tv.tv_usec - gInitTimeStamp;

        pBufInfo = pOMXBuffer->pOutputPortPrivate;

        // Following is to print the decoding fps
        if (dec_cnt == 0) {
            tv_old = tv;
        }
        if (dec_cnt++ >= 50) {
            diff_time = (tv.tv_sec - tv_old.tv_sec) * 1000 + (tv.tv_usec - tv_old.tv_usec) / 1000;
            fps = 1000  * (dec_cnt - 1) / diff_time;
            dec_cnt = 0;
            LOG(SF_LOG_WARN, "Decoding fps: %d \r\n", fps);
        }
        if (pPortContainerExternal->nFlags & 0x1)
            pOMXBuffer->nFlags |= OMX_BUFFERFLAG_EOS;

        LOG(SF_LOG_PERF, "OMX finish one buffer, address = %p, size = %d, nTimeStamp = %d, nFlags = %X\r\n",
            pOMXBuffer->pBuffer, pOMXBuffer->nFilledLen, pOMXBuffer->nTimeStamp, pOMXBuffer->nFlags);
        LOG(SF_LOG_INFO, "indexFrameDisplay = %d, OMXBuferFlag = %d, OMXBufferAddr = %p\r\n",
                        pPortContainerExternal->index, pBufInfo->index, pOMXBuffer->pBuffer);
        ComponentImpl *pRendererComponent = (ComponentImpl *)pSfVideoImplement->hSFComponentRender;
        LOG(SF_LOG_PERF, "output queue count=%d/%d\r\n", pSfVideoImplement->functions->Queue_Get_Cnt(pRendererComponent->sinkPort.inputQ),
                                                    pSfOMXComponent->portDefinition[1].nBufferCountActual);

        if (pSfOMXComponent->propagateMarkType.hMarkTargetComponent != NULL) {
            LOG(SF_LOG_INFO, "Component propagate mark to output port\r\n");
            pOMXBuffer->hMarkTargetComponent = pSfOMXComponent->propagateMarkType.hMarkTargetComponent;
            pOMXBuffer->pMarkData = pSfOMXComponent->propagateMarkType.pMarkData;
            pSfOMXComponent->propagateMarkType.hMarkTargetComponent = NULL;
            pSfOMXComponent->propagateMarkType.pMarkData = NULL;
        }

        if (pSfOMXComponent->state == OMX_StatePause)
        {
            LOG(SF_LOG_INFO, "tmp store buf when pause\r\n");
            SF_Queue_Enqueue(pSfVideoImplement->outPauseQ, &pOMXBuffer);
        }
        else{
#if 0
            SF_OMX_BUF_INFO *pBufInfo = pOMXBuffer->pOutputPortPrivate;
            FILE *fb = fopen("./out.yuv", "ab+");
            LOG(SF_LOG_INFO, "%p %d %p\r\n", pOMXBuffer->pBuffer, pOMXBuffer->nFilledLen, pBufInfo->remap_vaddr);
            fwrite(pOMXBuffer->pBuffer, 1, pOMXBuffer->nFilledLen, fb);
            fclose(fb);
            }
#endif
            if (pOMXBuffer->nFlags & OMX_BUFFERFLAG_EOS)
            {
                pSfOMXComponent->callbacks->EventHandler(pSfOMXComponent->pOMXComponent, pSfOMXComponent->pAppData, OMX_EventBufferFlag,
                                                    1, pOMXBuffer->nFlags, NULL);
            }
            pSfOMXComponent->callbacks->FillBufferDone(pSfOMXComponent->pOMXComponent, pSfOMXComponent->pAppData, pOMXBuffer);
        }
    }
    break;
    case COMPONENT_EVENT_DEC_REGISTER_FB:
    {
        /*The width and height vpu return will align to 16, eg: 1080 will be 1088. so check scale value at first.*/
        TestDecConfig *testConfig = (TestDecConfig *)pSfVideoImplement->testConfig;
        OMX_U32 nWidth = testConfig->scaleDownWidth;
        OMX_U32 nHeight = testConfig->scaleDownHeight;
        if (nWidth <= 0 || nHeight <= 0)
        {
            /*If scale value not set, then get value from vpu*/
            FrameBuffer *frameBuffer = (FrameBuffer *)data;
            nWidth = frameBuffer->width;
            nHeight = frameBuffer->height;
        }

        LOG(SF_LOG_INFO, "Get Output width = %d, height = %d\r\n", nWidth, nHeight);
        pSfOMXComponent->portDefinition[1].format.video.nFrameWidth = nWidth;
        pSfOMXComponent->portDefinition[1].format.video.nFrameHeight = nHeight;
        pSfOMXComponent->portDefinition[1].format.video.nStride = nWidth;
        pSfOMXComponent->portDefinition[1].format.video.nSliceHeight = nHeight;

        ComponentImpl *pRendererComponent = (ComponentImpl *)pSfVideoImplement->hSFComponentRender;
        pSfOMXComponent->portDefinition[1].nBufferCountActual = pSfVideoImplement->functions->GetRenderTotalBufferNumber(pRendererComponent);
        pSfOMXComponent->portDefinition[1].nBufferCountMin = pSfOMXComponent->portDefinition[1].nBufferCountActual;

        /*Caculate buffer size by eColorFormat*/
        switch (pSfOMXComponent->portDefinition[1].format.video.eColorFormat)
        {
        case OMX_COLOR_FormatYUV420Planar:
        case OMX_COLOR_FormatYUV420SemiPlanar:
        case OMX_COLOR_FormatYVU420SemiPlanar:
            if (nWidth && nHeight) {
                pSfOMXComponent->portDefinition[1].nBufferSize = (nWidth * nHeight * 3) / 2;
            }
            break;
        default:
            if (nWidth && nHeight) {
                pSfOMXComponent->portDefinition[1].nBufferSize = nWidth * nHeight * 2;
            }
            break;
        }
        pSfOMXComponent->callbacks->EventHandler(pSfOMXComponent->pOMXComponent, pSfOMXComponent->pAppData, OMX_EventPortSettingsChanged,
                                                 1, OMX_IndexParamPortDefinition, NULL);
    }
    break;
    case COMPONENT_EVENT_DEC_INSUFFIC_RESOURCE:
    {
        LOG(SF_LOG_ERR, "Lack of memory for decoder buffer, process stop!\r\n");
        pSfOMXComponent->callbacks->EventHandler(pSfOMXComponent->pOMXComponent, pSfOMXComponent->pAppData, OMX_EventError,
                                                 OMX_ErrorInsufficientResources, 0, NULL);
        ComponentImpl *pSFComponentDecoder = (ComponentImpl *)pSfVideoImplement->hSFComponentExecoder;
        ComponentImpl *pSFComponentFeeder = (ComponentImpl *)pSfVideoImplement->hSFComponentFeeder;
        ComponentImpl *pSFComponentRender = (ComponentImpl *)pSfVideoImplement->hSFComponentRender;
        void *ThreadRet = NULL;
        DEC_CMD cmd;
        cmd.Cmd = DEC_StopThread;
        SF_Queue_Enqueue(pSfVideoImplement->CmdQueue, &cmd);
        pthread_cancel(pSfVideoImplement->pCmdThread->pthread);
        pthread_join(pSfVideoImplement->pCmdThread->pthread, &ThreadRet);
        LOG(SF_LOG_INFO, "Cmd thread end %ld\r\n", (Uint64)ThreadRet);
        if (!pSfOMXComponent->memory_optimization)
        {
            LOG(SF_LOG_INFO,"feeder src q cnt %d\r\n",
                pSfVideoImplement->functions->Queue_Get_Cnt(((ComponentImpl *)(pSfVideoImplement->hSFComponentFeeder))->srcPort.inputQ));
            pSfVideoImplement->functions->ComponentStop(pSFComponentRender);
            pSfVideoImplement->functions->ComponentStop(pSFComponentDecoder);
            pSfVideoImplement->functions->ComponentStop(pSFComponentFeeder);
            FlushBuffer(pSfOMXComponent, 0);
            FlushBuffer(pSfOMXComponent, 1);
            pSfVideoImplement->functions->ComponentWait(pSFComponentRender);
            pSfVideoImplement->functions->ComponentWait(pSFComponentDecoder);
            pSfVideoImplement->functions->ComponentWait(pSFComponentFeeder);
            pSFComponentRender->terminate = OMX_TRUE;
            pSFComponentDecoder->terminate = OMX_TRUE;
            pSFComponentFeeder->terminate = OMX_TRUE;
        }
        else
        {
            pSFComponentDecoder->pause = OMX_TRUE;
            pSFComponentFeeder->pause = OMX_TRUE;
            pSFComponentRender->pause = OMX_TRUE;
            FlushBuffer(pSfOMXComponent, 0);
            FlushBuffer(pSfOMXComponent, 1);
        }
        pSfOMXComponent->state = OMX_StateInvalid;
        LOG(SF_LOG_ERR, "Component into invalid state!\r\n");
        pSfOMXComponent->callbacks->EventHandler(pSfOMXComponent->pOMXComponent, pSfOMXComponent->pAppData, OMX_EventError,
                                                 OMX_ErrorInvalidState, 0, NULL);
    }
    break;
    case COMPONENT_EVENT_TERMINATED:
    break;
    case COMPONENT_EVENT_DEC_DECODED_ALL:
        LOG(SF_LOG_DEBUG,"renderer meet end\r\n");
    default:
        break;
    }
    FunctionOut();
}

static OMX_ERRORTYPE SF_OMX_EmptyThisBuffer(
    OMX_IN OMX_HANDLETYPE hComponent,
    OMX_IN OMX_BUFFERHEADERTYPE *pBuffer)
{
    OMX_ERRORTYPE ret = OMX_ErrorNone;
    FunctionIn();

    if (hComponent == NULL || pBuffer == NULL)
    {
        ret = OMX_ErrorBadParameter;
        goto EXIT;
    }

    OMX_COMPONENTTYPE *pOMXComponent = (OMX_COMPONENTTYPE *)hComponent;

    SF_OMX_COMPONENT *pSfOMXComponent = pOMXComponent->pComponentPrivate;
    SF_WAVE5_IMPLEMEMT *pSfVideoImplement = (SF_WAVE5_IMPLEMEMT *)pSfOMXComponent->componentImpl;

    ComponentImpl *pFeederComponent = (ComponentImpl *)pSfVideoImplement->hSFComponentFeeder;

    LOG(SF_LOG_DEBUG, "bufheader %p nFilledLen = %d, nFlags = %d, pBuffer = %p\r\n", pBuffer, pBuffer->nFilledLen, pBuffer->nFlags, pBuffer->pBuffer);

    if (!pSfOMXComponent->portDefinition[0].bEnabled)
    {
        LOG(SF_LOG_INFO, "feed buffer when input port stop\r\n");
        return OMX_ErrorIncorrectStateOperation;
    }

    if (pBuffer->nInputPortIndex != 0)
    {
        LOG(SF_LOG_INFO, "Incorrect nInputPortIndex %d\r\n", pBuffer->nInputPortIndex);
        return OMX_ErrorBadPortIndex;
    }

    PortContainerExternal *pPortContainerExternal = malloc(sizeof(PortContainerExternal));
    if (pPortContainerExternal == NULL)
    {
        LOG(SF_LOG_ERR, "malloc fail\r\n");
        return OMX_ErrorInsufficientResources;
    }
    memset(pPortContainerExternal, 0, sizeof(PortContainerExternal));
    pPortContainerExternal->pBuffer = pBuffer->pBuffer;
    pPortContainerExternal->nFilledLen = pBuffer->nFilledLen;
    pPortContainerExternal->nFlags = pBuffer->nFlags;
    pPortContainerExternal->pAppPrivate = (void*)pBuffer;

    SF_Queue_Enqueue(pSfVideoImplement->inPortQ, &pBuffer);

    if (pSfVideoImplement->functions->Queue_Enqueue(pFeederComponent->srcPort.inputQ, (void *)pPortContainerExternal) != OMX_TRUE)
    {
        LOG(SF_LOG_ERR, "%p:%p FAIL\r\n", pFeederComponent->srcPort.inputQ, pPortContainerExternal);
        free(pPortContainerExternal);
        return OMX_ErrorInsufficientResources;
    }
    LOG(SF_LOG_PERF, "input queue count=%d/%d\r\n", pSfVideoImplement->functions->Queue_Get_Cnt(pFeederComponent->srcPort.inputQ),
                                                    pSfOMXComponent->portDefinition[0].nBufferCountActual);
    //LOG(SF_LOG_INFO, "input buffer address = %p, size = %d, flag = %x\r\n", pBuffer->pBuffer, pBuffer->nFilledLen, pBuffer->nFlags);

    if (pBuffer->nFlags & OMX_BUFFERFLAG_EOS)
    {
        pSfVideoImplement->rev_eos = OMX_TRUE;
    }

    free(pPortContainerExternal);

    pFeederComponent->pause = OMX_FALSE;
EXIT:
    FunctionOut();

    return ret;
}

static OMX_ERRORTYPE SF_OMX_FillThisBuffer(
    OMX_IN OMX_HANDLETYPE hComponent,
    OMX_IN OMX_BUFFERHEADERTYPE *pBuffer)
{
    OMX_ERRORTYPE ret = OMX_ErrorNone;
    FunctionIn();

    if (hComponent == NULL || pBuffer == NULL)
    {
        ret = OMX_ErrorBadParameter;
        goto EXIT;
    }

    OMX_COMPONENTTYPE *pOMXComponent = (OMX_COMPONENTTYPE *)hComponent;
    SF_OMX_COMPONENT *pSfOMXComponent = (SF_OMX_COMPONENT *)pOMXComponent->pComponentPrivate;
    SF_WAVE5_IMPLEMEMT *pSfVideoImplement = (SF_WAVE5_IMPLEMEMT *)pSfOMXComponent->componentImpl;
    SF_OMX_BUF_INFO *pBufInfo = (SF_OMX_BUF_INFO *)pBuffer->pOutputPortPrivate;
    ComponentImpl *pRendererComponent = (ComponentImpl *)pSfVideoImplement->hSFComponentRender;
    LOG(SF_LOG_DEBUG, "bufheader %p nFilledLen = %d, nFlags = %d, pBuffer = %p\r\n", pBuffer, pBuffer->nFilledLen, pBuffer->nFlags, pBuffer->pBuffer);

    if (!pSfOMXComponent->portDefinition[1].bEnabled)
    {
        LOG(SF_LOG_INFO, "feed buffer when output port stop\r\n");
        return OMX_ErrorIncorrectStateOperation;
    }

    if (pBuffer->nOutputPortIndex != 1)
    {
        LOG(SF_LOG_INFO, "Incorrect nOutputPortIndex %d\r\n", pBuffer->nOutputPortIndex);
        return OMX_ErrorBadPortIndex;
    }

    PortContainerExternal *pPortContainerExternal = malloc(sizeof(PortContainerExternal));
    if (pPortContainerExternal == NULL)
    {
        LOG(SF_LOG_ERR, "malloc fail\r\n");
        return OMX_ErrorInsufficientResources;
    }
    memset(pPortContainerExternal, 0, sizeof(PortContainerExternal));
    pPortContainerExternal->pBuffer = pBuffer->pBuffer;
    pPortContainerExternal->nFilledLen = pBuffer->nAllocLen;
    pPortContainerExternal->pAppPrivate = (void*)pBuffer;

    if (gInitTimeStamp != 0 && pSfOMXComponent->memory_optimization)
    {
        int clear = pSfVideoImplement->frame_array[pSfVideoImplement->frame_array_index];
        pSfVideoImplement->functions->Render_DecClrDispFlag(pRendererComponent->context, clear);
        pSfVideoImplement->frame_array[pSfVideoImplement->frame_array_index] = pBufInfo->index;
        LOG(SF_LOG_INFO, "store display flag: %d, clear display flag: %d\r\n",
                            pSfVideoImplement->frame_array[pSfVideoImplement->frame_array_index], clear);
        pSfVideoImplement->frame_array_index ++;
    }

    if (pSfVideoImplement->frame_array_index == MAX_INDEX) pSfVideoImplement->frame_array_index = 0;

    if (!pSfOMXComponent->memory_optimization)
        SF_Queue_Enqueue(pSfVideoImplement->outPortQ, &pBuffer);

    if (pSfVideoImplement->functions->Queue_Enqueue(pRendererComponent->sinkPort.inputQ, (void *)pPortContainerExternal) == FALSE)
    {
        LOG(SF_LOG_ERR, "%p:%p FAIL\r\n", pRendererComponent->sinkPort.inputQ, pPortContainerExternal);
        free(pPortContainerExternal);
        return OMX_ErrorInsufficientResources;
    }
    LOG(SF_LOG_PERF, "output queue count=%d/%d\r\n", pSfVideoImplement->functions->Queue_Get_Cnt(pRendererComponent->sinkPort.inputQ),
                                                    pSfOMXComponent->portDefinition[1].nBufferCountActual);
    free(pPortContainerExternal);
    pRendererComponent->pause = OMX_FALSE;
EXIT:
    FunctionOut();

    return ret;
}


static OMX_ERRORTYPE SF_OMX_UseBuffer(
    OMX_IN OMX_HANDLETYPE hComponent,
    OMX_INOUT OMX_BUFFERHEADERTYPE **ppBufferHdr,
    OMX_IN OMX_U32 nPortIndex,
    OMX_IN OMX_PTR pAppPrivate,
    OMX_IN OMX_U32 nSizeBytes,
    OMX_IN OMX_U8 *pBuffer)
{
    OMX_ERRORTYPE ret = OMX_ErrorNone;
    OMX_COMPONENTTYPE *pOMXComponent = (OMX_COMPONENTTYPE *)hComponent;
    SF_OMX_COMPONENT *pSfOMXComponent = pOMXComponent->pComponentPrivate;
    SF_WAVE5_IMPLEMEMT *pSfVideoImplement = (SF_WAVE5_IMPLEMEMT *)pSfOMXComponent->componentImpl;
    ComponentImpl *pComponentRender = (ComponentImpl *)pSfVideoImplement->hSFComponentRender;
    OMX_U32 i;

    FunctionIn();

    if (hComponent == NULL)
    {
        ret = OMX_ErrorBadParameter;
        goto EXIT;
    }

    OMX_BUFFERHEADERTYPE *temp_bufferHeader = (OMX_BUFFERHEADERTYPE *)malloc(sizeof(OMX_BUFFERHEADERTYPE));
    if (temp_bufferHeader == NULL)
    {
        LOG(SF_LOG_ERR, "malloc fail\r\n");
        return OMX_ErrorInsufficientResources;
    }
    memset(temp_bufferHeader, 0, sizeof(OMX_BUFFERHEADERTYPE));

    SF_OMX_BUF_INFO *pBufInfo = (SF_OMX_BUF_INFO *)malloc(sizeof(SF_OMX_BUF_INFO));
    if (pBufInfo == NULL)
    {
        LOG(SF_LOG_ERR, "malloc fail\r\n");
        free(temp_bufferHeader);
        return OMX_ErrorInsufficientResources;
    }

    temp_bufferHeader->nAllocLen = nSizeBytes;
    temp_bufferHeader->pAppPrivate = pAppPrivate;
    temp_bufferHeader->pBuffer = pBuffer;
    temp_bufferHeader->nSize = sizeof(OMX_BUFFERHEADERTYPE);
    memcpy(&(temp_bufferHeader->nVersion), &(pSfOMXComponent->componentVersion), sizeof(OMX_VERSIONTYPE));
    *ppBufferHdr = temp_bufferHeader;

    if (nPortIndex == 1)
    {
        for (i = 0; i < MAX_BUFF_NUM; i++)
        {
            if (!pSfOMXComponent->pBufferArray[1][i])
            {
                if ((!pComponentRender) || (!pSfOMXComponent->memory_optimization)
                                || (pSfVideoImplement->functions->AttachDMABuffer(pComponentRender, (Uint64)pBuffer, nSizeBytes) == FALSE))
                {
                    LOG(SF_LOG_INFO, "Failed to attach dma buffer, no memory optimization\r\n");
                    pSfOMXComponent->memory_optimization = OMX_FALSE;
                }

                pBufInfo->type = SF_BUFFER_DMA_EXTERNAL;
                pBufInfo->index = i;
                temp_bufferHeader->pOutputPortPrivate = (OMX_PTR)pBufInfo;
                temp_bufferHeader->nOutputPortIndex = 1;
                pSfOMXComponent->pBufferArray[1][i] = temp_bufferHeader;
                pSfOMXComponent->assignedBufferNum[1] ++;

                if (pSfOMXComponent->assignedBufferNum[1] == pSfOMXComponent->portDefinition[1].nBufferCountActual)
                {
                    pSfOMXComponent->portDefinition[1].bPopulated = OMX_TRUE;
                    SF_SemaphorePost(pSfOMXComponent->portSemaphore[1]);
                }
                break;
            }
        }
        if (i == MAX_BUFF_NUM){
            LOG(SF_LOG_ERR, "buffer array full\r\n");
            free(temp_bufferHeader);
            free(pBufInfo);
            ret = OMX_ErrorInsufficientResources;
        }
    }
    else if (nPortIndex == 0)
    {
        for (i = 0; i < MAX_BUFF_NUM; i++)
        {
            if (!pSfOMXComponent->pBufferArray[0][i])
            {
                pBufInfo->type = SF_BUFFER_DMA_EXTERNAL;
                pBufInfo->index = i;
                temp_bufferHeader->pInputPortPrivate = (OMX_PTR)pBufInfo;
                temp_bufferHeader->nInputPortIndex = 0;
                pSfOMXComponent->pBufferArray[0][i] = temp_bufferHeader;
                pSfOMXComponent->assignedBufferNum[0] ++;

                if (pSfOMXComponent->assignedBufferNum[0] == pSfOMXComponent->portDefinition[0].nBufferCountActual)
                {
                    pSfOMXComponent->portDefinition[0].bPopulated = OMX_TRUE;
                    SF_SemaphorePost(pSfOMXComponent->portSemaphore[0]);
                }
                break;
            }
        }
        if (i == MAX_BUFF_NUM){
            LOG(SF_LOG_ERR, "buffer array full\r\n");
            free(temp_bufferHeader);
            free(pBufInfo);
            ret = OMX_ErrorInsufficientResources;
        }
    }

    LOG(SF_LOG_INFO, "header %p pBuffer address = %p on port %d\r\n", temp_bufferHeader, temp_bufferHeader->pBuffer, nPortIndex);
EXIT:
    FunctionOut();
    return ret;
}

static OMX_ERRORTYPE SF_OMX_AllocateBuffer(
    OMX_IN OMX_HANDLETYPE hComponent,
    OMX_INOUT OMX_BUFFERHEADERTYPE **ppBuffer,
    OMX_IN OMX_U32 nPortIndex,
    OMX_IN OMX_PTR pAppPrivate,
    OMX_IN OMX_U32 nSizeBytes)
{
    OMX_ERRORTYPE ret = OMX_ErrorNone;
    OMX_COMPONENTTYPE *pOMXComponent = (OMX_COMPONENTTYPE *)hComponent;
    SF_OMX_COMPONENT *pSfOMXComponent = pOMXComponent->pComponentPrivate;
    SF_WAVE5_IMPLEMEMT *pSfVideoImplement = (SF_WAVE5_IMPLEMEMT *)pSfOMXComponent->componentImpl;
    ComponentImpl *pComponentRender = (ComponentImpl *)pSfVideoImplement->hSFComponentRender;
    OMX_U32 i;

    FunctionIn();
    if (nSizeBytes == 0)
    {
        LOG(SF_LOG_WARN, "nSizeBytes = %d, use default buffer size\r\n", nSizeBytes);
        nSizeBytes = DEFAULT_VIDEO_OUTPUT_BUFFER_SIZE;
    }

    OMX_BUFFERHEADERTYPE *temp_bufferHeader = (OMX_BUFFERHEADERTYPE *)malloc(sizeof(OMX_BUFFERHEADERTYPE));
    if (temp_bufferHeader == NULL)
    {
        LOG(SF_LOG_ERR, "malloc fail\r\n");
        return OMX_ErrorInsufficientResources;
    }
    memset(temp_bufferHeader, 0, sizeof(OMX_BUFFERHEADERTYPE));

    SF_OMX_BUF_INFO *pBufInfo = (SF_OMX_BUF_INFO *)malloc(sizeof(SF_OMX_BUF_INFO));
    if (pBufInfo == NULL)
    {
        LOG(SF_LOG_ERR, "malloc fail\r\n");
        free(temp_bufferHeader);
        return OMX_ErrorInsufficientResources;
    }

    temp_bufferHeader->nAllocLen = nSizeBytes;
    temp_bufferHeader->pAppPrivate = pAppPrivate;
    temp_bufferHeader->nSize = sizeof(OMX_BUFFERHEADERTYPE);
    memcpy(&(temp_bufferHeader->nVersion), &(pSfOMXComponent->componentVersion), sizeof(OMX_VERSIONTYPE));
    if (nPortIndex == 1)
    {
        for (i = 0; i < MAX_BUFF_NUM; i++)
        {
            if (!pSfOMXComponent->pBufferArray[1][i])
            {
                // Alloc DMA memory first
                if (pSfOMXComponent->memory_optimization)
                {
                    temp_bufferHeader->pBuffer = pSfVideoImplement->functions->AllocateFrameBuffer2(pComponentRender, nSizeBytes);
                }
                // DMA Memory alloc fail, goto normal alloc
                if (temp_bufferHeader->pBuffer == NULL)
                {
                    pSfOMXComponent->memory_optimization = OMX_FALSE;
                    temp_bufferHeader->pBuffer = malloc(nSizeBytes);
                    memset(temp_bufferHeader->pBuffer, 0, nSizeBytes);
                    pBufInfo->type = SF_BUFFER_NOMAL;
                    LOG(SF_LOG_PERF, "Use normal buffer\r\n");
                }
                else
                {
                    pBufInfo->type = SF_BUFFER_DMA;
                    LOG(SF_LOG_PERF, "Use DMA buffer\r\n");
                }
                pBufInfo->index = i;
                temp_bufferHeader->pOutputPortPrivate = (OMX_PTR)pBufInfo;
                temp_bufferHeader->nOutputPortIndex = 1;
                pSfOMXComponent->pBufferArray[1][i] = temp_bufferHeader;
                pSfOMXComponent->assignedBufferNum[1] ++;

                if (pSfOMXComponent->assignedBufferNum[1] == pSfOMXComponent->portDefinition[1].nBufferCountActual)
                {
                    pSfOMXComponent->portDefinition[1].bPopulated = OMX_TRUE;
                    SF_SemaphorePost(pSfOMXComponent->portSemaphore[1]);
                }
                break;
            }
        }
        if (i == MAX_BUFF_NUM){
            LOG(SF_LOG_ERR, "buffer array full\r\n");
            temp_bufferHeader->pBuffer = NULL;
        }
    }
    else if (nPortIndex == 0)
    {
        for (i = 0; i < MAX_BUFF_NUM; i++)
        {
            if (!pSfOMXComponent->pBufferArray[0][i])
            {
                temp_bufferHeader->pBuffer = malloc(nSizeBytes);
                memset(temp_bufferHeader->pBuffer, 0, nSizeBytes);

                pBufInfo->type = SF_BUFFER_NOMAL;
                pBufInfo->index = i;
                temp_bufferHeader->pInputPortPrivate = (OMX_PTR)pBufInfo;
                temp_bufferHeader->nInputPortIndex = 0;
                pSfOMXComponent->pBufferArray[0][i] = temp_bufferHeader;
                pSfOMXComponent->assignedBufferNum[0] ++;

                if (pSfOMXComponent->assignedBufferNum[0] == pSfOMXComponent->portDefinition[0].nBufferCountActual)
                {
                    pSfOMXComponent->portDefinition[0].bPopulated = OMX_TRUE;
                    SF_SemaphorePost(pSfOMXComponent->portSemaphore[0]);
                }
                break;
            }
        }
        if (i == MAX_BUFF_NUM){
            LOG(SF_LOG_ERR, "buffer array full\r\n");
            temp_bufferHeader->pBuffer = NULL;
        }
    }

    if (temp_bufferHeader->pBuffer == NULL)
    {
        free(temp_bufferHeader);
        free(pBufInfo);
        LOG(SF_LOG_ERR, "malloc fail\r\n");
        return OMX_ErrorInsufficientResources;
    }
    *ppBuffer = temp_bufferHeader;
    LOG(SF_LOG_INFO, "header %p pBuffer address = %p on port %d\r\n", temp_bufferHeader, temp_bufferHeader->pBuffer, nPortIndex);

    FunctionOut();

    return ret;
}

static OMX_ERRORTYPE SF_OMX_GetParameter(
    OMX_IN OMX_HANDLETYPE hComponent,
    OMX_IN OMX_INDEXTYPE nParamIndex,
    OMX_INOUT OMX_PTR ComponentParameterStructure)
{
    OMX_ERRORTYPE ret = OMX_ErrorNone;
    OMX_COMPONENTTYPE *pOMXComponent = (OMX_COMPONENTTYPE *)hComponent;
    SF_OMX_COMPONENT *pSfOMXComponent = pOMXComponent->pComponentPrivate;
    FunctionIn();
    LOG(SF_LOG_INFO, "Get parameter on index %X\r\n", nParamIndex);
    if (hComponent == NULL || ComponentParameterStructure == NULL)
    {
        ret = OMX_ErrorBadParameter;
        goto EXIT;
    }
    switch ((OMX_U32)nParamIndex)
    {
    case OMX_IndexParamVideoInit:
    {
        OMX_PORT_PARAM_TYPE *portParam = (OMX_PORT_PARAM_TYPE *)ComponentParameterStructure;
        portParam->nPorts           = 2;
        portParam->nStartPortNumber = 0;
    }
    break;
    case OMX_IndexParamAudioInit:
    case OMX_IndexParamImageInit:
    case OMX_IndexParamOtherInit:
    {
        OMX_PORT_PARAM_TYPE *portParam = (OMX_PORT_PARAM_TYPE *)ComponentParameterStructure;
        portParam->nPorts           = 0;
        portParam->nStartPortNumber = 0;
    }
    break;
    case OMX_IndexParamVideoPortFormat:
    {
        OMX_VIDEO_PARAM_PORTFORMATTYPE *portFormat = (OMX_VIDEO_PARAM_PORTFORMATTYPE *)ComponentParameterStructure;
        OMX_U32 index = portFormat->nIndex;
        switch (index)
        {
        case 0:
            portFormat->eCompressionFormat = OMX_VIDEO_CodingUnused;
            portFormat->eColorFormat = OMX_COLOR_FormatYUV420Planar;
            portFormat->xFramerate = 30;
            break;
        case 1:
            portFormat->eCompressionFormat = OMX_VIDEO_CodingUnused;
            portFormat->eColorFormat = OMX_COLOR_FormatYUV420SemiPlanar;
            portFormat->xFramerate = 30;
            break;
        case 2:
            portFormat->eCompressionFormat = OMX_VIDEO_CodingUnused;
            portFormat->eColorFormat = OMX_COLOR_FormatYVU420SemiPlanar;
            portFormat->xFramerate = 30;
            break;
        default:
            if (index > 0)
            {
                ret = OMX_ErrorNoMore;
            }
            break;
        }
    }

    break;
    case OMX_IndexParamVideoBitrate:
        break;
    case OMX_IndexParamVideoQuantization:

        break;
    case OMX_IndexParamPortDefinition:{
        OMX_PARAM_PORTDEFINITIONTYPE *pPortDefinition = (OMX_PARAM_PORTDEFINITIONTYPE *)ComponentParameterStructure;
        OMX_U32 portIndex = pPortDefinition->nPortIndex;
        OMX_PARAM_PORTDEFINITIONTYPE *pSrcDefinition;
        if (portIndex < 2)
        {
            pSrcDefinition = &pSfOMXComponent->portDefinition[portIndex];
            LOG(SF_LOG_INFO, "Get parameter width = %d, height = %d from port %X\r\n",
                pSrcDefinition->format.video.nFrameWidth, pSrcDefinition->format.video.nFrameHeight, portIndex);
            memcpy(pPortDefinition, pSrcDefinition, pPortDefinition->nSize);
        }
        else
        {
            LOG(SF_LOG_INFO,"Bad port index %d\r\n",portIndex);
            ret = OMX_ErrorBadPortIndex;
        }
    }
        break;

    case OMX_IndexParamVideoIntraRefresh:

        break;

    case OMX_IndexParamStandardComponentRole:
    {
        OMX_PARAM_COMPONENTROLETYPE *pComponentRole = (OMX_PARAM_COMPONENTROLETYPE *)ComponentParameterStructure;
        strcpy((OMX_STRING)(pComponentRole->cRole), pSfOMXComponent->componentRule);
    }
        break;

    case OMX_IndexParamVideoAvc:
        break;
    case OMX_IndexParamVideoHevc:
        break;
    case OMX_IndexParamVideoProfileLevelQuerySupported:
        break;
    case OMX_IndexParamCompBufferSupplier:
    {
        OMX_PARAM_BUFFERSUPPLIERTYPE *bufferSupplier = (OMX_PARAM_BUFFERSUPPLIERTYPE *)ComponentParameterStructure;
        OMX_U32 portIndex = bufferSupplier->nPortIndex;
        if (portIndex >= 2)
            ret = OMX_ErrorBadPortIndex;
    }
        break;

    default:
        ret = OMX_ErrorUnsupportedIndex;
        break;
    }

EXIT:
    FunctionOut();

    return ret;
}

static OMX_ERRORTYPE SF_OMX_SetParameter(
    OMX_IN OMX_HANDLETYPE hComponent,
    OMX_IN OMX_INDEXTYPE nIndex,
    OMX_IN OMX_PTR ComponentParameterStructure)
{
    OMX_ERRORTYPE ret = OMX_ErrorNone;

    FunctionIn();
    if (hComponent == NULL || ComponentParameterStructure == NULL)
    {
        ret = OMX_ErrorBadParameter;
        goto EXIT;
    }

    OMX_COMPONENTTYPE *pOMXComponent = (OMX_COMPONENTTYPE *)hComponent;
    SF_OMX_COMPONENT *pSfOMXComponent = pOMXComponent->pComponentPrivate;
    SF_WAVE5_IMPLEMEMT *pSfVideoImplement = (SF_WAVE5_IMPLEMEMT *)pSfOMXComponent->componentImpl;

    if (pSfOMXComponent == NULL)
    {
        ret = OMX_ErrorBadParameter;
        goto EXIT;
    }
    TestDecConfig *testConfig = pSfVideoImplement->testConfig;
    LOG(SF_LOG_INFO, "Set parameter on index %X\r\n", nIndex);
    switch ((OMX_U32)nIndex)
    {
    case OMX_IndexParamVideoPortFormat:
    {
        OMX_VIDEO_PARAM_PORTFORMATTYPE *portFormat = (OMX_VIDEO_PARAM_PORTFORMATTYPE *)ComponentParameterStructure;
        OMX_U32 nPortIndex = portFormat->nPortIndex;
        TestDecConfig *pTestDecConfig = (TestDecConfig *)pSfVideoImplement->testConfig;
        OMX_PARAM_PORTDEFINITIONTYPE *pPort = &pSfOMXComponent->portDefinition[nPortIndex];
        LOG(SF_LOG_DEBUG, "Set video format to port %d color %d\r\n", portFormat->nPortIndex, portFormat->eColorFormat);
        switch (portFormat->eColorFormat)
        {
             case OMX_COLOR_FormatYUV420Planar: //I420
                pTestDecConfig->cbcrInterleave = FALSE;
                pTestDecConfig->nv21 = FALSE;
                pPort->format.video.eColorFormat = portFormat->eColorFormat;
                break;
            case OMX_COLOR_FormatYUV420SemiPlanar: //NV12
                pTestDecConfig->cbcrInterleave = TRUE;
                pTestDecConfig->nv21 = FALSE;
                pPort->format.video.eColorFormat = portFormat->eColorFormat;
                break;
            case OMX_COLOR_FormatYVU420SemiPlanar: //NV21
                pTestDecConfig->cbcrInterleave = TRUE;
                pTestDecConfig->nv21 = TRUE;
                pPort->format.video.eColorFormat = portFormat->eColorFormat;
                break;
            default:
                LOG(SF_LOG_ERR, "Error to set parameter: %d, only nv12 nv21 i420 supported\r\n", portFormat->eColorFormat);
                return OMX_ErrorBadParameter;
                break;
        }
        break;
    }
    case OMX_IndexParamVideoBitrate:
        break;
    case OMX_IndexParamVideoQuantization:

        break;
    break;
    case OMX_IndexParamPortDefinition:
    {
        OMX_PARAM_PORTDEFINITIONTYPE *pPortDefinition = (OMX_PARAM_PORTDEFINITIONTYPE *)ComponentParameterStructure;
        OMX_PARAM_PORTDEFINITIONTYPE *pInputPort = &pSfOMXComponent->portDefinition[0];
        OMX_PARAM_PORTDEFINITIONTYPE *pOutputPort = &pSfOMXComponent->portDefinition[1];
        TestDecConfig *pTestDecConfig = (TestDecConfig *)pSfVideoImplement->testConfig;
        OMX_U32 portIndex = pPortDefinition->nPortIndex;
        OMX_U32 width = pPortDefinition->format.video.nFrameWidth;
        OMX_U32 height = pPortDefinition->format.video.nFrameHeight;
        LOG(SF_LOG_INFO, "Set width = %d, height = %d on port %d\r\n", width, height, pPortDefinition->nPortIndex);

        if (portIndex == 0)
        {
            if (pPortDefinition->nBufferCountActual != pInputPort->nBufferCountActual)
            {
                LOG(SF_LOG_INFO, "Set input buffer count = %d\r\n", pPortDefinition->nBufferCountActual);
                ComponentImpl *pFeederComponent = (ComponentImpl *)pSfVideoImplement->hSFComponentFeeder;
                if (pFeederComponent)
                {
                    pSfVideoImplement->functions->ComponentPortDestroy(&pFeederComponent->srcPort);
                    pSfVideoImplement->functions->ComponentPortCreate(&pFeederComponent->srcPort, pSfVideoImplement->hSFComponentFeeder,
                                                                        pPortDefinition->nBufferCountActual, sizeof(PortContainerExternal));
                }
            }

            memcpy(&pSfOMXComponent->portDefinition[portIndex], pPortDefinition, pPortDefinition->nSize);

            pInputPort->format.video.nStride = width;
            pInputPort->format.video.nSliceHeight = height;
            pInputPort->nBufferSize = width * height * 2;
        }
        else if (portIndex == 1)
        {
            if (pPortDefinition->nBufferCountActual != pOutputPort->nBufferCountActual)
            {
                LOG(SF_LOG_INFO, "Set output buffer count = %d\r\n", pPortDefinition->nBufferCountActual);
                ComponentImpl *pRenderComponent = (ComponentImpl *)pSfVideoImplement->hSFComponentRender;
                if (pRenderComponent)
                {
                    pSfVideoImplement->functions->SetRenderTotalBufferNumber(pRenderComponent, pPortDefinition->nBufferCountActual);
                    pSfVideoImplement->functions->ComponentPortDestroy(&pRenderComponent->sinkPort);
                    pSfVideoImplement->functions->ComponentPortCreate(&pRenderComponent->sinkPort, pSfVideoImplement->hSFComponentRender,
                                                                        pPortDefinition->nBufferCountActual, sizeof(PortContainerExternal));
                }
            }

            memcpy(&pSfOMXComponent->portDefinition[portIndex], pPortDefinition, pPortDefinition->nSize);

            /*
              Some client may set '0' or '1' to output port, in this case use input port parameters
            */
            if (width <= 1)
            {
                width = pInputPort->format.video.nFrameWidth;
            }
            if (height <= 1)
            {
                height = pInputPort->format.video.nFrameHeight;
            }
            if (width > 0 && height > 0)
            {
                int scalew = pInputPort->format.video.nFrameWidth / width;
                int scaleh = pInputPort->format.video.nFrameHeight / height;
                if (scalew > 8 || scaleh > 8 || scalew < 1 || scaleh < 1)
                {
                    int nInputWidth = pInputPort->format.video.nFrameWidth;
                    int nInputHeight = pInputPort->format.video.nFrameHeight;
                    LOG(SF_LOG_WARN, "Scaling should be 1 to 1/8 (down-scaling only)! Use input parameter. "
                        "OutPut[%d, %d]. Input[%d, %d]\r\n", width, height, nInputWidth, nInputHeight);
                    width = nInputWidth;
                    height = nInputHeight;
                }
            }
            pOutputPort->format.video.nFrameWidth = width;
            pOutputPort->format.video.nFrameHeight = height;
            pOutputPort->format.video.nStride = width;
            pOutputPort->format.video.nSliceHeight = height;
            pTestDecConfig->scaleDownWidth = VPU_CEIL(width, 2);
            pTestDecConfig->scaleDownHeight = VPU_CEIL(height, 2);
            LOG(SF_LOG_INFO, "Set scale = %d, %d\r\n", pTestDecConfig->scaleDownWidth , pTestDecConfig->scaleDownHeight);
             /*
                if cbcrInterleave is FALSE and nv21 is FALSE, the default dec format is I420
                if cbcrInterleave is TRUE and nv21 is FALSE, then the dec format is NV12
                if cbcrInterleave is TRUE and nv21 is TRUE, then the dec format is NV21
            */
            LOG(SF_LOG_INFO, "Set format = %d\r\n", pOutputPort->format.video.eColorFormat);
            switch (pOutputPort->format.video.eColorFormat)
            {
            case OMX_COLOR_FormatYUV420Planar: //I420
                pTestDecConfig->cbcrInterleave = FALSE;
                pTestDecConfig->nv21 = FALSE;
                if (width && height)
                    pOutputPort->nBufferSize = (width * height * 3) / 2;
                break;
            case OMX_COLOR_FormatYUV420SemiPlanar: //NV12
                pTestDecConfig->cbcrInterleave = TRUE;
                pTestDecConfig->nv21 = FALSE;
                if (width && height)
                    pOutputPort->nBufferSize = (width * height * 3) / 2;
                break;
            case OMX_COLOR_FormatYVU420SemiPlanar: //NV21
                pTestDecConfig->cbcrInterleave = TRUE;
                pTestDecConfig->nv21 = TRUE;
                if (width && height)
                    pOutputPort->nBufferSize = (width * height * 3) / 2;
                break;
            default:
                LOG(SF_LOG_ERR, "Error to set parameter: %d, only nv12 nv21 i420 supported\r\n",
                    pOutputPort->format.video.eColorFormat);
                return OMX_ErrorBadParameter;
                break;
            }
        }
        else
        {
            LOG(SF_LOG_INFO,"Bad port index %d\r\n",portIndex);
            ret = OMX_ErrorBadPortIndex;
        }
    }
    break;
    case OMX_IndexParamVideoIntraRefresh:

        break;

    case OMX_IndexParamStandardComponentRole:
    {
        if ((pSfOMXComponent->state != OMX_StateLoaded) &&
                (pSfOMXComponent->state != OMX_StateWaitForResources)) {
            ret = OMX_ErrorIncorrectStateOperation;
        }
    }
        break;

    case OMX_IndexParamVideoAvc:
        testConfig->bitFormat = 0;
        break;
    case OMX_IndexParamVideoHevc:
        testConfig->bitFormat = 12;
        break;
    case OMX_IndexParamCompBufferSupplier:
    {
        OMX_PARAM_BUFFERSUPPLIERTYPE *bufferSupplier = (OMX_PARAM_BUFFERSUPPLIERTYPE *)ComponentParameterStructure;
        OMX_U32 portIndex = bufferSupplier->nPortIndex;
        if (portIndex >= 2)
            ret = OMX_ErrorBadPortIndex;
    }
        break;

    default:
        ret = OMX_ErrorUnsupportedIndex;
        break;
    }

EXIT:
    FunctionOut();

    return ret;
}

static OMX_ERRORTYPE SF_OMX_GetConfig(
    OMX_HANDLETYPE hComponent,
    OMX_INDEXTYPE nIndex,
    OMX_PTR pComponentConfigStructure)
{
    OMX_ERRORTYPE ret = OMX_ErrorNone;

    FunctionIn();

    if (hComponent == NULL)
    {
        ret = OMX_ErrorBadParameter;
        goto EXIT;
    }

    switch (nIndex)
    {
    case OMX_IndexConfigVideoAVCIntraPeriod:

        break;
    case OMX_IndexConfigVideoBitrate:

        break;
    case OMX_IndexConfigVideoFramerate:

        break;

    default:
        break;
    }

EXIT:
    FunctionOut();

    return ret;
}

static OMX_ERRORTYPE SF_OMX_SetConfig(
    OMX_HANDLETYPE hComponent,
    OMX_INDEXTYPE nIndex,
    OMX_PTR pComponentConfigStructure)
{
    OMX_ERRORTYPE ret = OMX_ErrorNone;

    FunctionIn();

    if (hComponent == NULL)
    {
        ret = OMX_ErrorBadParameter;
        goto EXIT;
    }

    switch ((OMX_U32)nIndex)
    {
    case OMX_IndexConfigVideoAVCIntraPeriod:
        break;
    case OMX_IndexConfigVideoBitrate:
        break;
    case OMX_IndexConfigVideoFramerate:
        break;
    case OMX_IndexConfigVideoIntraVOPRefresh:
        break;
    default:

        break;
    }

EXIT:
    FunctionOut();

    return ret;
}

static OMX_ERRORTYPE SF_OMX_ComponentTunnelRequest(
    OMX_IN OMX_HANDLETYPE hComp,
    OMX_IN OMX_U32        nPort,
    OMX_IN OMX_HANDLETYPE hTunneledComp,
    OMX_IN OMX_U32        nTunneledPort,
    OMX_INOUT OMX_TUNNELSETUPTYPE *pTunnelSetup)
{
    OMX_ERRORTYPE ret = OMX_ErrorNone;
    (void) hComp;
    (void) nPort;
    (void) hTunneledComp;
    (void) nTunneledPort;
    (void) pTunnelSetup;
    ret = OMX_ErrorTunnelingUnsupported;
    goto EXIT;
EXIT:
    return ret;
}

static OMX_ERRORTYPE InitDecoder(SF_OMX_COMPONENT *pSfOMXComponent)
{
    TestDecConfig *testConfig = NULL;
    CNMComponentConfig *config = NULL;
    Uint32 sizeInWord;
    char *fwPath = NULL;
    SF_WAVE5_IMPLEMEMT *pSfVideoImplement = (SF_WAVE5_IMPLEMEMT *)pSfOMXComponent->componentImpl;
    OMX_PARAM_PORTDEFINITIONTYPE *pInputPort = &pSfOMXComponent->portDefinition[0];
    OMX_PARAM_PORTDEFINITIONTYPE *pOutputPort = &pSfOMXComponent->portDefinition[1];

    if (pSfVideoImplement->hSFComponentExecoder != NULL)
    {
        return OMX_ErrorNone;
    }

    FunctionIn();

    testConfig = (TestDecConfig *)pSfVideoImplement->testConfig;

    testConfig->productId = (ProductId)pSfVideoImplement->functions->VPU_GetProductId(testConfig->coreIdx);
    if (CheckDecTestConfig(testConfig) == FALSE)
    {
        LOG(SF_LOG_ERR, "fail to CheckTestConfig()\n");
        return OMX_ErrorBadParameter;
    }

    switch (testConfig->productId)
    {
    case PRODUCT_ID_521:
        fwPath = pSfOMXComponent->fwPath;
        break;
    case PRODUCT_ID_511:
        fwPath = pSfOMXComponent->fwPath;
        break;
    case PRODUCT_ID_517:
        fwPath = CORE_7_BIT_CODE_FILE_PATH;
        break;
    default:
        LOG(SF_LOG_ERR, "Unknown product id: %d, whether kernel module loaded?\n", testConfig->productId);
        return OMX_ErrorBadParameter;
    }
    LOG(SF_LOG_INFO, "FW PATH = %s\n", fwPath);
    if (pSfVideoImplement->functions->LoadFirmware(testConfig->productId, (Uint8 **)&(pSfVideoImplement->pusBitCode), &sizeInWord, fwPath) < 0)
    {
        LOG(SF_LOG_ERR, "Failed to load firmware: %s\n", fwPath);
        return OMX_ErrorInsufficientResources;
    }
    testConfig->scaleDownWidth = VPU_CEIL(pOutputPort->format.video.nFrameWidth, 2);
    testConfig->scaleDownHeight = VPU_CEIL(pOutputPort->format.video.nFrameHeight, 2);

    config = pSfVideoImplement->config;
    memcpy(&(config->testDecConfig), testConfig, sizeof(TestDecConfig));
    config->bitcode = (Uint8 *)pSfVideoImplement->pusBitCode;
    config->sizeOfBitcode = sizeInWord;
    if (pInputPort->format.video.nFrameWidth && pInputPort->format.video.nFrameHeight)
        config->testDecConfig.bsSize =  (pInputPort->format.video.nFrameWidth * pInputPort->format.video.nFrameHeight / 2);

    if (pSfOMXComponent->memory_optimization)
    {
        config->MemoryOptimization = TRUE;
    }

    if (pSfVideoImplement->functions->SetUpDecoderOpenParam(&config->decOpenParam, &config->testDecConfig) != RETCODE_SUCCESS)
    {
        LOG(SF_LOG_ERR, "SetupDecoderOpenParam error\n");
        return OMX_ErrorBadParameter;
    }
    LOG(SF_LOG_DEBUG, "cbcrInterleave = %d, nv21 = %d\r\n", testConfig->cbcrInterleave, testConfig->nv21);
    pSfVideoImplement->hSFComponentExecoder = pSfVideoImplement->functions->ComponentCreate("wave_decoder", config);
    pSfVideoImplement->hSFComponentFeeder = pSfVideoImplement->functions->ComponentCreate("feeder", config);
    pSfVideoImplement->hSFComponentRender = pSfVideoImplement->functions->ComponentCreate("renderer", config);

    ComponentImpl *pFeederComponent = (ComponentImpl *)pSfVideoImplement->hSFComponentFeeder;
    pSfVideoImplement->functions->ComponentPortCreate(&pFeederComponent->srcPort, pSfVideoImplement->hSFComponentFeeder,
                                        pSfOMXComponent->portDefinition[0].nBufferCountActual, sizeof(PortContainerExternal));

    ComponentImpl *pRenderComponent = (ComponentImpl *)pSfVideoImplement->hSFComponentRender;
    pSfVideoImplement->functions->ComponentPortDestroy(&pRenderComponent->sinkPort);
    pSfVideoImplement->functions->ComponentPortCreate(&pRenderComponent->sinkPort, pSfVideoImplement->hSFComponentRender,
                                                                                    MAX_GDI_IDX, sizeof(PortContainerExternal));

    if (pSfVideoImplement->functions->SetupDecListenerContext(pSfVideoImplement->lsnCtx, config, NULL) == TRUE)
    {
        pSfVideoImplement->functions->ComponentRegisterListener(pSfVideoImplement->hSFComponentExecoder,
                                                              COMPONENT_EVENT_DEC_ALL, pSfVideoImplement->functions->DecoderListener, (void *)pSfVideoImplement->lsnCtx);
        pSfVideoImplement->functions->ComponentRegisterListener(pSfVideoImplement->hSFComponentExecoder,
                                                              COMPONENT_EVENT_DEC_INSUFFIC_RESOURCE, OnEventArrived, (void *)pSfOMXComponent);
        pSfVideoImplement->functions->ComponentRegisterListener(pSfVideoImplement->hSFComponentRender,
                                                              COMPONENT_EVENT_DEC_DECODED_ALL, OnEventArrived, (void *)pSfOMXComponent);
        pSfVideoImplement->functions->ComponentRegisterListener(pSfVideoImplement->hSFComponentFeeder,
                                                              COMPONENT_EVENT_DEC_ALL, OnEventArrived, (void *)pSfOMXComponent);
        pSfVideoImplement->functions->ComponentRegisterListener(pSfVideoImplement->hSFComponentRender,
                                                              COMPONENT_EVENT_DEC_ALL, OnEventArrived, (void *)pSfOMXComponent);
    }
    else
    {
        LOG(SF_LOG_ERR, "ComponentRegisterListener fail\r\n");
        return OMX_ErrorBadParameter;
    }

    pSfVideoImplement->functions->ComponentSetupTunnel(pSfVideoImplement->hSFComponentFeeder, pSfVideoImplement->hSFComponentExecoder);
    pSfVideoImplement->functions->ComponentSetupTunnel(pSfVideoImplement->hSFComponentExecoder, pSfVideoImplement->hSFComponentRender);
    FunctionOut();
    return OMX_ErrorNone;
}

static OMX_ERRORTYPE SF_OMX_SendCommand(
    OMX_IN OMX_HANDLETYPE hComponent,
    OMX_IN OMX_COMMANDTYPE Cmd,
    OMX_IN OMX_U32 nParam,
    OMX_IN OMX_PTR pCmdData)
{
    OMX_ERRORTYPE ret = OMX_ErrorNone;
    OMX_COMPONENTTYPE *pOMXComponent = (OMX_COMPONENTTYPE *)hComponent;
    SF_OMX_COMPONENT *pSfOMXComponent = pOMXComponent->pComponentPrivate;
    SF_WAVE5_IMPLEMEMT *pSfVideoImplement = (SF_WAVE5_IMPLEMEMT *)pSfOMXComponent->componentImpl;
    OMX_STATETYPE comCurrentState;
    DEC_CMD cmd;

    //BOOL done = 0;

    FunctionIn();
    if (hComponent == NULL || pSfOMXComponent == NULL)
    {
        ret = OMX_ErrorBadParameter;
        goto EXIT;
    }


    LOG(SF_LOG_INFO, "cmd = %X, nParam = %X\r\n", Cmd, nParam);

    comCurrentState = pSfOMXComponent->state;

    if (comCurrentState == OMX_StateInvalid) {
        return OMX_ErrorInvalidState;
    }

    switch (Cmd)
    {
    case OMX_CommandStateSet:
    {
        if ((nParam == OMX_StateLoaded) &&
            (pSfOMXComponent->state == OMX_StateIdle))
        {
            pSfOMXComponent->traningState = OMX_TransStateIdleToLoaded;
        }
        else if ((nParam == OMX_StateIdle) &&
            (pSfOMXComponent->state == OMX_StateLoaded))
        {
            pSfOMXComponent->traningState = OMX_TransStateLoadedToIdle;
        }
    }
    break;

    case OMX_CommandFlush:
    {
        if (nParam >= 2 && nParam != OMX_ALL)
            return OMX_ErrorBadPortIndex;
    }
    break;

    case OMX_CommandMarkBuffer:
    {
        if (nParam >= 2)
            return OMX_ErrorBadPortIndex;
        if ((pSfOMXComponent->state != OMX_StateExecuting) &&
            (pSfOMXComponent->state != OMX_StatePause))
            return OMX_ErrorIncorrectStateOperation;
    }
    break;

    case OMX_CommandPortDisable:
    {
        if (nParam >= 2 && nParam != OMX_ALL)
            return OMX_ErrorBadPortIndex;

        if (nParam == OMX_ALL)
        {
            for (int i = 0; i < 2; i++)
            {
                if(pSfOMXComponent->portDefinition[i].bEnabled == OMX_FALSE)
                    return OMX_ErrorIncorrectStateOperation;
            }
            for (int i = 0; i < 2; i++)
            {
                pSfOMXComponent->portDefinition[i].bEnabled = OMX_FALSE;
            }
        }
        else
        {
            if (pSfOMXComponent->portDefinition[nParam].bEnabled == OMX_FALSE)
                return OMX_ErrorIncorrectStateOperation;
            pSfOMXComponent->portDefinition[nParam].bEnabled = OMX_FALSE;
        }
    }
    break;

    case OMX_CommandPortEnable:
    {
        if (nParam >= 2 && nParam != OMX_ALL)
            return OMX_ErrorBadPortIndex;

        if (nParam == OMX_ALL)
        {
            for (int i = 0; i < 2; i++)
            {
                if(pSfOMXComponent->portDefinition[i].bEnabled == OMX_TRUE)
                    return OMX_ErrorIncorrectStateOperation;
            }
            for (int i = 0; i < 2; i++)
            {
                pSfOMXComponent->portDefinition[i].bEnabled = OMX_TRUE;
            }
        }
        else
        {
            if (pSfOMXComponent->portDefinition[nParam].bEnabled == OMX_TRUE)
                return OMX_ErrorIncorrectStateOperation;
            pSfOMXComponent->portDefinition[nParam].bEnabled = OMX_TRUE;
        }
    }
    break;

    default:
    break;
    }

    cmd.Cmd = Cmd;
    cmd.nParam = nParam;
    cmd.pCmdData = pCmdData;

    ret = SF_Queue_Enqueue(pSfVideoImplement->CmdQueue, &cmd);

EXIT:
    FunctionOut();
    return ret;
}

static OMX_ERRORTYPE SF_OMX_GetState(
    OMX_IN OMX_HANDLETYPE hComponent,
    OMX_OUT OMX_STATETYPE *pState)
{
    OMX_ERRORTYPE ret = OMX_ErrorNone;
    OMX_COMPONENTTYPE *pOMXComponent = (OMX_COMPONENTTYPE *)hComponent;
    SF_OMX_COMPONENT *pSfOMXComponent = pOMXComponent->pComponentPrivate;
    FunctionIn();
    *pState = pSfOMXComponent->state;

    FunctionOut();
    return ret;
}

static OMX_ERRORTYPE SF_OMX_FreeBuffer(
    OMX_IN OMX_HANDLETYPE hComponent,
    OMX_IN OMX_U32 nPortIndex,
    OMX_IN OMX_BUFFERHEADERTYPE *pBufferHdr)
{
    OMX_ERRORTYPE ret = OMX_ErrorNone;
    OMX_COMPONENTTYPE *pOMXComponent = (OMX_COMPONENTTYPE *)hComponent;
    SF_OMX_COMPONENT *pSfOMXComponent = pOMXComponent->pComponentPrivate;
    SF_OMX_BUF_INFO *pBufInfo;

    FunctionIn();
    if(pBufferHdr == NULL)
        return OMX_ErrorBadParameter;

    if(nPortIndex == 0)
        pBufInfo = pBufferHdr->pInputPortPrivate;
    else if(nPortIndex == 1)
        pBufInfo = pBufferHdr->pOutputPortPrivate;

    LOG(SF_LOG_INFO, "free header %p pBuffer address = %p on port %d\r\n",
                                            pBufferHdr, pBufferHdr->pBuffer, nPortIndex);

    if ((pSfOMXComponent->traningState != OMX_TransStateIdleToLoaded) &&
            (pSfOMXComponent->portDefinition[nPortIndex].bEnabled))
    {
        LOG(SF_LOG_INFO, "port Unpopulated\r\n");
        pSfOMXComponent->callbacks->EventHandler(pSfOMXComponent->pOMXComponent, pSfOMXComponent->pAppData,
                                                    OMX_EventError, OMX_ErrorPortUnpopulated, nPortIndex, NULL);
    }

    if(pBufInfo->type == SF_BUFFER_NOMAL)
    {
        if(pBufferHdr->pBuffer)
            free(pBufferHdr->pBuffer);
    }

    pSfOMXComponent->pBufferArray[nPortIndex][pBufInfo->index] = NULL;
    pSfOMXComponent->assignedBufferNum[nPortIndex]--;
    LOG(SF_LOG_INFO, "remain %d buff port %d assgin\r\n",pSfOMXComponent->assignedBufferNum[nPortIndex], nPortIndex);

    free(pBufferHdr);
    free(pBufInfo);

    if (ret == OMX_ErrorNone) {
        if (pSfOMXComponent->assignedBufferNum[nPortIndex] == 0) {
            LOG(SF_LOG_INFO, "unloadedResource signal set\r\n");
            SF_SemaphorePost(pSfOMXComponent->portUnloadSemaphore[nPortIndex]);
            pSfOMXComponent->portDefinition[nPortIndex].bPopulated = OMX_FALSE;
        }
    }

    FunctionOut();
    return ret;
}

static void FlushInputQ(SF_OMX_COMPONENT *pSfOMXComponent, SF_Queue *pQueue)
{
    OMX_BUFFERHEADERTYPE *pOMXBuffer = NULL;
    OMX_BUFFERHEADERTYPE **ppBuffer = NULL;

    ppBuffer = SF_Queue_Dequeue(pQueue);
    while (ppBuffer)
    {
        pOMXBuffer = *ppBuffer;
        LOG(SF_LOG_PERF, "OMX empty one buffer, address = %p, size = %d, nTimeStamp = %d, nFlags = %X\r\n",
                        pOMXBuffer->pBuffer, pOMXBuffer->nFilledLen, pOMXBuffer->nTimeStamp, pOMXBuffer->nFlags);
        pSfOMXComponent->callbacks->EmptyBufferDone(pSfOMXComponent->pOMXComponent, pSfOMXComponent->pAppData, pOMXBuffer);
        ppBuffer = SF_Queue_Dequeue(pQueue);
    }
}

static void FlushOutputQ(SF_OMX_COMPONENT *pSfOMXComponent, SF_Queue *pQueue)
{
    OMX_BUFFERHEADERTYPE *pOMXBuffer = NULL;
    OMX_BUFFERHEADERTYPE **ppBuffer = NULL;

    ppBuffer = SF_Queue_Dequeue(pQueue);
    while (ppBuffer)
    {
        pOMXBuffer = *ppBuffer;
        LOG(SF_LOG_PERF, "OMX finish one buffer, address = %p, size = %d, nTimeStamp = %d, nFlags = %X\r\n",
                                    pOMXBuffer->pBuffer, pOMXBuffer->nFilledLen, pOMXBuffer->nTimeStamp, pOMXBuffer->nFlags);
        pSfOMXComponent->callbacks->FillBufferDone(pSfOMXComponent->pOMXComponent, pSfOMXComponent->pAppData, pOMXBuffer);
        ppBuffer = SF_Queue_Dequeue(pQueue);
    }
}

static void CmdThread(void *args)
{
    OMX_ERRORTYPE ret = OMX_ErrorNone;
    SF_OMX_COMPONENT *pSfOMXComponent = (SF_OMX_COMPONENT *)args;
    SF_WAVE5_IMPLEMEMT *pSfVideoImplement = (SF_WAVE5_IMPLEMEMT *)pSfOMXComponent->componentImpl;
    OMX_STATETYPE comCurrentState;
    ComponentImpl *pSFComponentDecoder = NULL;
    ComponentImpl *pSFComponentFeeder = NULL;
    ComponentImpl *pSFComponentRender = NULL;
    ComponentState componentState;
    DEC_CMD *pCmd;
    OMX_U32 i = 0, cnt = 0;

    while(1){
        pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
        pCmd = (DEC_CMD*)SF_Queue_Dequeue_Block(pSfVideoImplement->CmdQueue);
        pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
        pthread_testcancel();
        pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

        if(!pCmd)
            continue;

        if(pCmd->Cmd == DEC_StopThread)
            break;

        switch (pCmd->Cmd)
        {
        case OMX_CommandStateSet:
            LOG(SF_LOG_INFO, "OMX dest state = %X\r\n", pCmd->nParam);

            //GetStateCommon(hComponent, &comCurrentState);
            comCurrentState = pSfOMXComponent->state;

            if (comCurrentState == pCmd->nParam)
            {
                LOG(SF_LOG_DEBUG, "same state %d\r\n", pCmd->nParam);
                pSfOMXComponent->callbacks->EventHandler(pSfOMXComponent->pOMXComponent, pSfOMXComponent->pAppData,
                                                    OMX_EventError, OMX_ErrorSameState, pCmd->nParam, NULL);
                break;
            }

            ret = OMX_ErrorNone;

            switch (pCmd->nParam)
            {
            case OMX_StateInvalid:
                switch (comCurrentState)
                {
                case OMX_StateExecuting:
                    pSfVideoImplement->functions->ComponentStop(pSfVideoImplement->hSFComponentFeeder);
                    pSfVideoImplement->functions->ComponentStop(pSfVideoImplement->hSFComponentExecoder);
                    pSfVideoImplement->functions->ComponentStop(pSfVideoImplement->hSFComponentRender);
                case OMX_StateIdle:
                case OMX_StatePause:
                case OMX_StateLoaded:
                case OMX_StateWaitForResources:
                default:
                    pSfOMXComponent->state = OMX_StateInvalid;
                    break;
                }
                ret = OMX_ErrorInvalidState;
                break;

            case OMX_StateLoaded:
                switch (comCurrentState)
                {
                case OMX_StateWaitForResources:
                    break;
                case OMX_StateIdle:
                    for (i = 0; i < 2; i++)
                    {
                        if (pSfOMXComponent->portDefinition[i].bEnabled){
                            LOG(SF_LOG_INFO,"unload SemaphoreWait \r\n");
                            SF_SemaphoreWait(pSfOMXComponent->portUnloadSemaphore[i]);
                            LOG(SF_LOG_INFO,"unload SemaphoreWait out\r\n");
                        }
                    }
                    break;
                default:
                    ret = OMX_ErrorIncorrectStateTransition;
                    break;
                }
                break;
            case OMX_StateIdle:
                if (comCurrentState == OMX_StatePause)
                {
                    comCurrentState = pSfOMXComponent->stateBeforePause;
                }
                switch (comCurrentState)
                {
                case OMX_StateLoaded:
                    ret = InitDecoder(pSfOMXComponent);
                    if (ret != OMX_ErrorNone)
                    {
                        break;
                    }
                    pSFComponentDecoder = (ComponentImpl *)pSfVideoImplement->hSFComponentExecoder;
                    pSFComponentFeeder = (ComponentImpl *)pSfVideoImplement->hSFComponentFeeder;
                    pSFComponentRender = (ComponentImpl *)pSfVideoImplement->hSFComponentRender;
                    pSFComponentFeeder->pause = OMX_FALSE;
                    pSFComponentDecoder->pause = OMX_FALSE;
                    pSFComponentRender->pause = OMX_FALSE;
                    for (i = 0; i < 2; i++)
                    {
                        if (pSfOMXComponent->portDefinition[i].bEnabled){
                            LOG(SF_LOG_INFO,"SemaphoreWait \r\n");
                            SF_SemaphoreWait(pSfOMXComponent->portSemaphore[i]);
                            LOG(SF_LOG_INFO,"SemaphoreWait out\r\n");
                        }
                    }
                    pSfVideoImplement->rev_eos =OMX_FALSE;
                    break;
                case OMX_StateExecuting:
                    pSFComponentDecoder = (ComponentImpl *)pSfVideoImplement->hSFComponentExecoder;
                    pSFComponentFeeder = (ComponentImpl *)pSfVideoImplement->hSFComponentFeeder;
                    pSFComponentRender = (ComponentImpl *)pSfVideoImplement->hSFComponentRender;
                    if (!pSfOMXComponent->memory_optimization)
                    {
                        LOG(SF_LOG_INFO,"feeder src q cnt %d\r\n",
                            pSfVideoImplement->functions->Queue_Get_Cnt(((ComponentImpl *)(pSfVideoImplement->hSFComponentFeeder))->srcPort.inputQ));
                        pSfVideoImplement->functions->ComponentStop(pSFComponentRender);
                        pSfVideoImplement->functions->ComponentStop(pSFComponentDecoder);
                        pSfVideoImplement->functions->ComponentStop(pSFComponentFeeder);
                        FlushBuffer(pSfOMXComponent, 0);
                        FlushInputQ(pSfOMXComponent, pSfVideoImplement->inPortQ);
                        FlushBuffer(pSfOMXComponent, 1);
                        FlushOutputQ(pSfOMXComponent, pSfVideoImplement->outPortQ);
                        pSfVideoImplement->functions->ComponentWait(pSFComponentRender);
                        pSfVideoImplement->functions->ComponentWait(pSFComponentDecoder);
                        pSfVideoImplement->functions->ComponentWait(pSFComponentFeeder);
                        pSFComponentRender->terminate = OMX_TRUE;
                        pSFComponentDecoder->terminate = OMX_TRUE;
                        pSFComponentFeeder->terminate = OMX_TRUE;
                        pSfVideoImplement->functions->ComponentRelease(pSFComponentFeeder);
                        pSfVideoImplement->functions->ComponentRelease(pSFComponentDecoder);
                        pSfVideoImplement->functions->ComponentRelease(pSFComponentRender);
                        pSfVideoImplement->functions->ComponentDestroy(pSFComponentFeeder, NULL);
                        pSfVideoImplement->functions->ComponentDestroy(pSFComponentDecoder, NULL);
                        pSfVideoImplement->functions->ComponentDestroy(pSFComponentRender, NULL);
                        pSfVideoImplement->hSFComponentExecoder = NULL;
                        pSfVideoImplement->hSFComponentFeeder = NULL;
                        pSfVideoImplement->hSFComponentRender = NULL;
                    }
                    else
                    {
                        pSFComponentDecoder->pause = OMX_TRUE;
                        pSFComponentFeeder->pause = OMX_TRUE;
                        pSFComponentRender->pause = OMX_TRUE;
                        FlushBuffer(pSfOMXComponent, 0);
                        FlushBuffer(pSfOMXComponent, 1);
                    }
                    pSfVideoImplement->rev_eos =OMX_FALSE;
                    break;
                case OMX_StateIdle:
                    pSFComponentFeeder->pause = OMX_FALSE;
                    pSFComponentDecoder->pause = OMX_FALSE;
                    pSFComponentRender->pause = OMX_FALSE;
                    break;
                default:
                    ret = OMX_ErrorIncorrectStateTransition;
                    break;
                }
                break;

            case OMX_StateWaitForResources:
                switch (comCurrentState)
                {
                case OMX_StateLoaded:
                    break;
                default:
                    ret = OMX_ErrorIncorrectStateTransition;
                    break;
                }
                break;

            case OMX_StateExecuting:
                if (comCurrentState == OMX_StatePause)
                {
                    comCurrentState = pSfOMXComponent->stateBeforePause;
                }
                switch (comCurrentState)
                {
                case OMX_StateIdle:
                    ret = InitDecoder(pSfOMXComponent);
                    if (ret != OMX_ErrorNone)
                    {
                        break;
                    }
                    pSFComponentDecoder = (ComponentImpl *)pSfVideoImplement->hSFComponentExecoder;
                    pSFComponentFeeder = (ComponentImpl *)pSfVideoImplement->hSFComponentFeeder;
                    pSFComponentRender = (ComponentImpl *)pSfVideoImplement->hSFComponentRender;
                    if (pSFComponentDecoder->thread == NULL)
                    {
                        LOG(SF_LOG_INFO, "execute component %s\r\n", pSFComponentDecoder->name);
                        componentState = pSfVideoImplement->functions->ComponentExecute(pSFComponentDecoder);
                        LOG(SF_LOG_INFO, "ret = %d\r\n", componentState);
                    }
                    if (pSFComponentFeeder->thread == NULL)
                    {
                        LOG(SF_LOG_INFO, "execute component %s\r\n", pSFComponentFeeder->name);
                        componentState = pSfVideoImplement->functions->ComponentExecute(pSFComponentFeeder);
                        LOG(SF_LOG_INFO, "ret = %d\r\n", componentState);
                    }
                    if (pSFComponentRender->thread == NULL)
                    {
                        LOG(SF_LOG_INFO, "execute component %s\r\n", pSFComponentRender->name);
                        componentState = pSfVideoImplement->functions->ComponentExecute(pSFComponentRender);
                        LOG(SF_LOG_INFO, "ret = %d\r\n", componentState);
                    }
                    pSFComponentFeeder->pause = OMX_FALSE;
                    pSFComponentDecoder->pause = OMX_FALSE;
                    pSFComponentRender->pause = OMX_FALSE;
                    break;
                case OMX_StateExecuting:
                    pSFComponentFeeder->pause = OMX_FALSE;
                    pSFComponentDecoder->pause = OMX_FALSE;
                    pSFComponentRender->pause = OMX_FALSE;
                    break;
                default:
                    ret = OMX_ErrorIncorrectStateTransition;
                    break;
                }
                break;

            case OMX_StatePause:
                switch (comCurrentState)
                {
                case OMX_StateIdle:
                case OMX_StateExecuting:
                    pSFComponentFeeder->pause = OMX_TRUE;
                    pSFComponentDecoder->pause = OMX_TRUE;
                    pSFComponentRender->pause = OMX_TRUE;
                    pSfOMXComponent->stateBeforePause = comCurrentState;
                    break;
                default:
                    ret = OMX_ErrorIncorrectStateTransition;
                    break;
                }
                break;

            default:
                ret = OMX_ErrorIncorrectStateTransition;
                break;
            }

            if (ret == OMX_ErrorNone)
            {
                pSfOMXComponent->state = pCmd->nParam;
                pSfOMXComponent->traningState = OMX_TransStateInvalid;
                pSfOMXComponent->callbacks->EventHandler(pSfOMXComponent->pOMXComponent, pSfOMXComponent->pAppData,
                                                    OMX_EventCmdComplete, OMX_CommandStateSet, pCmd->nParam, NULL);
                LOG(SF_LOG_DEBUG, "complete cmd StateSet %d\r\n", pCmd->nParam);

                if (pSfOMXComponent->state == OMX_StateExecuting)
                {
                    FlushOutputQ(pSfOMXComponent, pSfVideoImplement->outPauseQ);

                    FlushInputQ(pSfOMXComponent, pSfVideoImplement->inPauseQ);
                }
            }
            else
            {
                pSfOMXComponent->callbacks->EventHandler(pSfOMXComponent->pOMXComponent, pSfOMXComponent->pAppData,
                                                    OMX_EventError, ret, 0, NULL);
                LOG(SF_LOG_DEBUG, "Can't go to state %d from %d, ret %X \r\n",pSfOMXComponent->state, pCmd->nParam, ret);
            }
            break;

        case OMX_CommandFlush:
        {
            LOG(SF_LOG_INFO, "flush port %d\r\n", pCmd->nParam);
            OMX_U32 nPort = pCmd->nParam;
            cnt = (pCmd->nParam == OMX_ALL) ? 2 : 1;

            for (i = 0; i < cnt; i++) {
                if (pCmd->nParam == OMX_ALL)
                    nPort = i;
                else
                    nPort = pCmd->nParam;

                if (nPort == 0)
                {
                    pSfOMXComponent->bPortFlushing[0] = OMX_TRUE;
                    FlushBuffer(pSfOMXComponent, nPort);
                    if (pSfOMXComponent->state == OMX_StatePause)
                    {
                        FlushInputQ(pSfOMXComponent, pSfVideoImplement->inPauseQ);
                    }

                    FlushInputQ(pSfOMXComponent, pSfVideoImplement->inPortQ);

                    pSfOMXComponent->callbacks->EventHandler(pSfOMXComponent->pOMXComponent, pSfOMXComponent->pAppData,
                                                    OMX_EventCmdComplete, OMX_CommandFlush, nPort, NULL);
                    pSfOMXComponent->bPortFlushing[0] = OMX_FALSE;
                }
                else
                {
                    pSfOMXComponent->bPortFlushing[1] = OMX_TRUE;
                    FlushBuffer(pSfOMXComponent, nPort);
                    if (pSfOMXComponent->state == OMX_StatePause)
                    {
                        FlushOutputQ(pSfOMXComponent, pSfVideoImplement->outPauseQ);
                    }

                    if (!pSfOMXComponent->memory_optimization){
                        FlushOutputQ(pSfOMXComponent, pSfVideoImplement->outPortQ);
                    }

                    pSfOMXComponent->callbacks->EventHandler(pSfOMXComponent->pOMXComponent, pSfOMXComponent->pAppData,
                                                    OMX_EventCmdComplete, OMX_CommandFlush, nPort, NULL);
                    pSfOMXComponent->bPortFlushing[1] = OMX_FALSE;
                }
            }
        }
        break;

        case OMX_CommandPortDisable:
        {
            LOG(SF_LOG_INFO, "disable port %d\r\n", pCmd->nParam);
            OMX_U32 nPort;
            cnt = (pCmd->nParam == OMX_ALL) ? 2 : 1;

            for (i = 0; i < cnt; i++) {
                if (pCmd->nParam == OMX_ALL)
                    nPort = i;
                else
                    nPort = pCmd->nParam;

                FlushBuffer(pSfOMXComponent, nPort);

                if (nPort == 0)
                    FlushInputQ(pSfOMXComponent, pSfVideoImplement->inPortQ);
                else if (nPort == 1)
                    FlushOutputQ(pSfOMXComponent, pSfVideoImplement->outPortQ);
            }

            for (i = 0; i < cnt; i++) {
                if (pCmd->nParam == OMX_ALL)
                    nPort = i;
                else
                    nPort = pCmd->nParam;

                if (pSfOMXComponent->state != OMX_StateLoaded)
                    SF_SemaphoreWait(pSfOMXComponent->portUnloadSemaphore[nPort]);
                pSfOMXComponent->callbacks->EventHandler(pSfOMXComponent->pOMXComponent, pSfOMXComponent->pAppData,
                                                    OMX_EventCmdComplete, OMX_CommandPortDisable, nPort, NULL);
            }
        }
        break;

        case OMX_CommandPortEnable:
        {
            LOG(SF_LOG_INFO, "enable port %d\r\n", pCmd->nParam);
            OMX_U32 nPort;
            cnt = (pCmd->nParam == OMX_ALL) ? 2 : 1;

            for (i = 0; i < cnt; i++) {
                if (pCmd->nParam == OMX_ALL)
                    nPort = i;
                else
                    nPort = pCmd->nParam;

                if ((pSfOMXComponent->state != OMX_StateLoaded) &&
                        (pSfOMXComponent->state != OMX_StateWaitForResources))
                    SF_SemaphoreWait(pSfOMXComponent->portSemaphore[nPort]);
                pSfOMXComponent->callbacks->EventHandler(pSfOMXComponent->pOMXComponent, pSfOMXComponent->pAppData,
                                                    OMX_EventCmdComplete, OMX_CommandPortEnable, nPort, NULL);
            }
        }
        break;

        case OMX_CommandMarkBuffer:
        {
            LOG(SF_LOG_INFO, "set mark %d\r\n", pCmd->nParam);
            OMX_U32 nPort;
            nPort = pCmd->nParam;
            pSfOMXComponent->markType[nPort].hMarkTargetComponent =
                                ((OMX_MARKTYPE *)pCmd->pCmdData)->hMarkTargetComponent;
            pSfOMXComponent->markType[nPort].pMarkData =
                                ((OMX_MARKTYPE *)pCmd->pCmdData)->pMarkData;
        }
        break;

        default:
            break;
        }
    }

    pthread_exit(NULL);
}

static OMX_ERRORTYPE SF_OMX_GetComponentVersion(
    OMX_IN  OMX_HANDLETYPE   hComponent,
    OMX_OUT OMX_STRING       pComponentName,
    OMX_OUT OMX_VERSIONTYPE *pComponentVersion,
    OMX_OUT OMX_VERSIONTYPE *pSpecVersion,
    OMX_OUT OMX_UUIDTYPE    *pComponentUUID)
{
    OMX_ERRORTYPE ret = OMX_ErrorNone;
    OMX_COMPONENTTYPE *pOMXComponent = NULL;
    SF_OMX_COMPONENT *pSfOMXComponent = NULL;
    OMX_U32 compUUID[4];

    FunctionIn();

    /* check parameters */
    if (hComponent     == NULL ||
        pComponentName == NULL || pComponentVersion == NULL ||
        pSpecVersion   == NULL || pComponentUUID    == NULL) {
        ret = OMX_ErrorBadParameter;
        goto EXIT;
    }
    pOMXComponent = (OMX_COMPONENTTYPE *)hComponent;

    if (pOMXComponent->pComponentPrivate == NULL) {
        ret = OMX_ErrorBadParameter;
        goto EXIT;
    }
    pSfOMXComponent = (SF_OMX_COMPONENT *)pOMXComponent->pComponentPrivate;

    if (pSfOMXComponent->state == OMX_StateInvalid) {
        ret = OMX_ErrorInvalidState;
        goto EXIT;
    }

    strcpy(pComponentName, pSfOMXComponent->componentName);
    memcpy(pComponentVersion, &(pSfOMXComponent->componentVersion), sizeof(OMX_VERSIONTYPE));
    memcpy(pSpecVersion, &(pSfOMXComponent->specVersion), sizeof(OMX_VERSIONTYPE));

    /* Fill UUID with handle address, PID and UID.
     * This should guarantee uiniqness */
    compUUID[0] = (OMX_U32)getpid();
    compUUID[1] = (OMX_U32)getpid();
    compUUID[2] = (OMX_U32)getpid();
    compUUID[3] = (OMX_U32)getpid();
    //compUUID[1] = getpid();
    //compUUID[2] = getuid();
    memcpy(*pComponentUUID, compUUID, 4 * sizeof(*compUUID));

    ret = OMX_ErrorNone;

EXIT:
    FunctionOut();

    return ret;
}

static OMX_U32 nInstance = 0;

static OMX_ERRORTYPE SF_OMX_ComponentConstructor(SF_OMX_COMPONENT *pSfOMXComponent)
{
    OMX_ERRORTYPE ret = OMX_ErrorNone;
    FunctionIn();

    if (nInstance >= 1)
    {
        ret = OMX_ErrorInsufficientResources;
        goto EXIT;
    }

    nInstance++;

    ret = InitComponentStructorCommon(pSfOMXComponent);
    if (ret != OMX_ErrorNone)
    {
        nInstance--;
        goto EXIT;
    }
    SF_WAVE5_IMPLEMEMT *pSfVideoImplement = (SF_WAVE5_IMPLEMEMT *)pSfOMXComponent->componentImpl;
    TestDecConfig *pTestDecConfig = (TestDecConfig *)pSfVideoImplement->testConfig;
    pTestDecConfig->feedingMode = FEEDING_METHOD_BUFFER;
    pTestDecConfig->bitstreamMode = BS_MODE_PIC_END;
    pTestDecConfig->bitFormat = pSfVideoImplement->bitFormat;
    /*
     if cbcrInterleave is FALSE and nv21 is FALSE, the default dec format is I420
     if cbcrInterleave is TRUE and nv21 is FALSE, then the dec format is NV12
     if cbcrInterleave is TRUE and nv21 is TRUE, then the dec format is NV21
    */
    pTestDecConfig->cbcrInterleave = TRUE;
    pTestDecConfig->nv21 = FALSE;

    pSfOMXComponent->pOMXComponent->UseBuffer = &SF_OMX_UseBuffer;
    pSfOMXComponent->pOMXComponent->AllocateBuffer = &SF_OMX_AllocateBuffer;
    pSfOMXComponent->pOMXComponent->EmptyThisBuffer = &SF_OMX_EmptyThisBuffer;
    pSfOMXComponent->pOMXComponent->FillThisBuffer = &SF_OMX_FillThisBuffer;
    pSfOMXComponent->pOMXComponent->FreeBuffer = &SF_OMX_FreeBuffer;
    pSfOMXComponent->pOMXComponent->ComponentTunnelRequest = &SF_OMX_ComponentTunnelRequest;
    pSfOMXComponent->pOMXComponent->GetParameter = &SF_OMX_GetParameter;
    pSfOMXComponent->pOMXComponent->SetParameter = &SF_OMX_SetParameter;
    pSfOMXComponent->pOMXComponent->GetConfig = &SF_OMX_GetConfig;
    pSfOMXComponent->pOMXComponent->SetConfig = &SF_OMX_SetConfig;
    pSfOMXComponent->pOMXComponent->SendCommand = &SF_OMX_SendCommand;
    pSfOMXComponent->pOMXComponent->GetState = &SF_OMX_GetState;
    pSfOMXComponent->pOMXComponent->GetComponentVersion = &SF_OMX_GetComponentVersion;
    // pSfOMXComponent->pOMXComponent->GetExtensionIndex = &SF_OMX_GetExtensionIndex;
    // pSfOMXComponent->pOMXComponent->ComponentRoleEnum = &SF_OMX_ComponentRoleEnum;
    // pSfOMXComponent->pOMXComponent->ComponentDeInit = &SF_OMX_ComponentDeInit;

    for (int i = 0; i < MAX_INDEX; i++)
    {
        pSfVideoImplement->frame_array[i] = -1;
    }
    pSfVideoImplement->frame_array_index = 0;

    pSfVideoImplement->CmdQueue = SF_Queue_Create(20, sizeof(DEC_CMD));
    if (NULL == pSfVideoImplement->CmdQueue)
    {
        LOG(SF_LOG_ERR, "create CmdQueue error");
        nInstance--;
        return OMX_ErrorInsufficientResources;
    }
    pSfVideoImplement->inPauseQ = SF_Queue_Create(20, sizeof(OMX_BUFFERHEADERTYPE*));
    if (NULL == pSfVideoImplement->inPauseQ)
    {
        LOG(SF_LOG_ERR, "create inPauseQ error");
        nInstance--;
        return OMX_ErrorInsufficientResources;
    }
    pSfVideoImplement->outPauseQ = SF_Queue_Create(20, sizeof(OMX_BUFFERHEADERTYPE*));
    if (NULL == pSfVideoImplement->outPauseQ)
    {
        LOG(SF_LOG_ERR, "create outPauseQ error");
        nInstance--;
        return OMX_ErrorInsufficientResources;
    }
    pSfVideoImplement->inPortQ = SF_Queue_Create(20, sizeof(OMX_BUFFERHEADERTYPE*));
    if (NULL == pSfVideoImplement->inPortQ)
    {
        LOG(SF_LOG_ERR, "create inPortQ error");
        nInstance--;
        return OMX_ErrorInsufficientResources;
    }
    pSfVideoImplement->outPortQ = SF_Queue_Create(20, sizeof(OMX_BUFFERHEADERTYPE*));
    if (NULL == pSfVideoImplement->outPortQ)
    {
        LOG(SF_LOG_ERR, "create outPortQ error");
        nInstance--;
        return OMX_ErrorInsufficientResources;
    }

    CreateThread(&pSfVideoImplement->pCmdThread, CmdThread, (void *)pSfOMXComponent);
    pSfVideoImplement->bCmdRunning = OMX_TRUE;

EXIT:
    FunctionOut();

    return ret;
}

static OMX_ERRORTYPE SF_OMX_ComponentClear(SF_OMX_COMPONENT *pSfOMXComponent)
{
    OMX_ERRORTYPE ret = OMX_ErrorNone;
    DEC_CMD cmd;
    void *ThreadRet = NULL;
    SF_WAVE5_IMPLEMEMT *pSfVideoImplement = (SF_WAVE5_IMPLEMEMT *)pSfOMXComponent->componentImpl;
    ComponentImpl *pSFComponentDecoder = (ComponentImpl *)pSfVideoImplement->hSFComponentExecoder;
    ComponentImpl *pSFComponentFeeder = (ComponentImpl *)pSfVideoImplement->hSFComponentFeeder;
    ComponentImpl *pSFComponentRender = (ComponentImpl *)pSfVideoImplement->hSFComponentRender;

    FunctionIn();
    if (pSFComponentDecoder == NULL || pSFComponentFeeder == NULL || pSFComponentRender == NULL)
    {
        goto EXIT;
    }
    pSFComponentDecoder->terminate = OMX_TRUE;
    pSFComponentFeeder->terminate = OMX_TRUE;
    pSFComponentRender->terminate = OMX_TRUE;
    pSfVideoImplement->functions->ComponentWait(pSfVideoImplement->hSFComponentFeeder);
    pSfVideoImplement->functions->ComponentWait(pSfVideoImplement->hSFComponentExecoder);
    pSfVideoImplement->functions->ComponentWait(pSfVideoImplement->hSFComponentRender);

    pSfVideoImplement->bCmdRunning = 0;
    /* enqueue DEC_StopThread mean cmd thread cycle end */
    cmd.Cmd = DEC_StopThread;
    SF_Queue_Enqueue(pSfVideoImplement->CmdQueue, &cmd);
    pthread_cancel(pSfVideoImplement->pCmdThread->pthread);
	pthread_join(pSfVideoImplement->pCmdThread->pthread, &ThreadRet);
    LOG(SF_LOG_INFO, "Cmd thread end %ld\r\n", (Uint64)ThreadRet);
    SF_Queue_Destroy(pSfVideoImplement->CmdQueue);
    SF_Queue_Destroy(pSfVideoImplement->outPauseQ);
    SF_Queue_Destroy(pSfVideoImplement->inPauseQ);
    SF_Queue_Destroy(pSfVideoImplement->inPortQ);
    SF_Queue_Destroy(pSfVideoImplement->outPortQ);

    free(pSfVideoImplement->pusBitCode);
    pSfVideoImplement->functions->ClearDecListenerContext(pSfVideoImplement->lsnCtx);
    ComponentClearCommon(pSfOMXComponent);
EXIT:
    nInstance--;
    FunctionOut();

    return ret;
}

SF_OMX_COMPONENT sf_dec_decoder_h265 = {
    .componentName = "OMX.sf.video_decoder.hevc",
    .libName = "libsfdec.so",
    .pOMXComponent = NULL,
    .SF_OMX_ComponentConstructor = SF_OMX_ComponentConstructor,
    .SF_OMX_ComponentClear = SF_OMX_ComponentClear,
    // .functions = NULL,
    // .bitFormat = STD_HEVC,
    .componentImpl = NULL,
    .fwPath = "/lib/firmware/chagall.bin",
    .componentRule = "video_decoder.hevc"};

SF_OMX_COMPONENT sf_dec_decoder_h264 = {
    .componentName = "OMX.sf.video_decoder.avc",
    .libName = "libsfdec.so",
    .pOMXComponent = NULL,
    .SF_OMX_ComponentConstructor = SF_OMX_ComponentConstructor,
    .SF_OMX_ComponentClear = SF_OMX_ComponentClear,
    // .functions = NULL,
    // .bitFormat = STD_AVC,
    .componentImpl = NULL,
    .fwPath = "/lib/firmware/chagall.bin",
    .componentRule = "video_decoder.avc"};
