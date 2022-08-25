// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2021 StarFive Technology Co., Ltd.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "SF_OMX_video_common.h"
#include "SF_OMX_Core.h"

extern OMX_TICKS gInitTimeStamp;

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
    static OMX_U32 dec_cnt = 0;
    static struct timeval tv_old = {0};
    OMX_U32 fps = 0;
    OMX_U64 diff_time = 0; // ms
    FunctionIn();
    SF_OMX_COMPONENT *pSfOMXComponent = GetSFOMXComponrntByComponent(com);
    SF_WAVE5_IMPLEMEMT *pSfVideoImplement = (SF_WAVE5_IMPLEMEMT *)pSfOMXComponent->componentImpl;
    char *event_str = Event2Str(event);

    LOG(SF_LOG_INFO, "event=%lX %s\r\n", event, event_str);
    switch (event)
    {
    case COMPONENT_EVENT_DEC_EMPTY_BUFFER_DONE:
        pOMXBuffer = GetOMXBufferByAddr(pSfOMXComponent, (OMX_U8 *)pPortContainerExternal->pBuffer);
        if (pOMXBuffer == NULL)
        {
            LOG(SF_LOG_WARN, "Could not find omx buffer by address\r\n");
            return;
        }
        pSfOMXComponent->callbacks->EmptyBufferDone(pSfOMXComponent->pOMXComponent, pSfOMXComponent->pAppData, pOMXBuffer);
        break;
    case COMPONENT_EVENT_DEC_FILL_BUFFER_DONE:
    {
        struct timeval tv;

        pOMXBuffer = GetOMXBufferByAddr(pSfOMXComponent, (OMX_U8 *)pPortContainerExternal->pBuffer);
        if (pOMXBuffer == NULL)
        {
            LOG(SF_LOG_WARN, "Could not find omx buffer by address\r\n");
            return;
        }
        gettimeofday(&tv, NULL);
        if (gInitTimeStamp == 0)
        {
            gInitTimeStamp = tv.tv_sec * 1000000 + tv.tv_usec;
        }
        pOMXBuffer->nFilledLen = pPortContainerExternal->nFilledLen;
        pOMXBuffer->nTimeStamp = tv.tv_sec * 1000000 + tv.tv_usec - gInitTimeStamp;
#if 0
        {
            FILE *fb = fopen("./out.bcp", "ab+");
            LOG(SF_LOG_INFO, "%p %d\r\n", pOMXBuffer->pBuffer, pOMXBuffer->nFilledLen);
            fwrite(pOMXBuffer->pBuffer, 1, pOMXBuffer->nFilledLen, fb);
            fclose(fb);
        }
#endif

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

        LOG(SF_LOG_PERF, "OMX finish one buffer, address = %p, size = %d, nTimeStamp = %d, nFlags = %X\r\n",
            pOMXBuffer->pBuffer, pOMXBuffer->nFilledLen, pOMXBuffer->nTimeStamp, pOMXBuffer->nFlags);
        LOG(SF_LOG_INFO, "indexFrameDisplay = %d, OMXBuferFlag = %d, OMXBufferAddr = %p\r\n",
                        pPortContainerExternal->nFlags, (OMX_U32)pOMXBuffer->pOutputPortPrivate, pOMXBuffer->pBuffer);
        ComponentImpl *pRendererComponent = (ComponentImpl *)pSfVideoImplement->hSFComponentRender;
        LOG(SF_LOG_PERF, "output queue count=%d/%d\r\n", pSfVideoImplement->functions->Queue_Get_Cnt(pRendererComponent->sinkPort.inputQ),
                                                    pSfOMXComponent->portDefinition[1].nBufferCountActual);

        pSfOMXComponent->callbacks->FillBufferDone(pSfOMXComponent->pOMXComponent, pSfOMXComponent->pAppData, pOMXBuffer);
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
    case COMPONENT_EVENT_DEC_DECODED_ALL:
        pSfOMXComponent->callbacks->EventHandler(pSfOMXComponent->pOMXComponent, pSfOMXComponent->pAppData, OMX_EventBufferFlag,
                                                 1, 1, NULL);
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

    if (pSfVideoImplement->functions->Queue_Enqueue(pFeederComponent->srcPort.inputQ, (void *)pPortContainerExternal) != OMX_TRUE)
    {
        LOG(SF_LOG_ERR, "%p:%p FAIL\r\n", pFeederComponent->srcPort.inputQ, pPortContainerExternal);
        free(pPortContainerExternal);
        return OMX_ErrorInsufficientResources;
    }
    LOG(SF_LOG_PERF, "input queue count=%d/%d\r\n", pSfVideoImplement->functions->Queue_Get_Cnt(pFeederComponent->srcPort.inputQ),
                                                    pSfOMXComponent->portDefinition[0].nBufferCountActual);
    LOG(SF_LOG_INFO, "input buffer address = %p, size = %d, flag = %x\r\n", pBuffer->pBuffer, pBuffer->nFilledLen, pBuffer->nFlags);
    free(pPortContainerExternal);

    pFeederComponent->pause = OMX_FALSE;
EXIT:
    FunctionOut();

    return ret;
}

#define MAX_INDEX 1
static OMX_U64 frame_array[MAX_INDEX] = {-1};
static int frame_array_index = 0;

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
    ComponentImpl *pRendererComponent = (ComponentImpl *)pSfVideoImplement->hSFComponentRender;
    PortContainerExternal *pPortContainerExternal = malloc(sizeof(PortContainerExternal));
    if (pPortContainerExternal == NULL)
    {
        LOG(SF_LOG_ERR, "malloc fail\r\n");
        return OMX_ErrorInsufficientResources;
    }
    memset(pPortContainerExternal, 0, sizeof(PortContainerExternal));
    pPortContainerExternal->pBuffer = pBuffer->pBuffer;
    pPortContainerExternal->nFilledLen = pBuffer->nAllocLen;

    if (gInitTimeStamp != 0)
    {
        int clear = frame_array[frame_array_index];
        pSfVideoImplement->functions->Render_DecClrDispFlag(pRendererComponent->context, clear);
        frame_array[frame_array_index] = (OMX_U64)(pBuffer->pOutputPortPrivate);
        LOG(SF_LOG_INFO, "store display flag: %d, clear display flag: %d\r\n", frame_array[frame_array_index], clear);
        frame_array_index ++;
    }

    if (frame_array_index == MAX_INDEX) frame_array_index = 0;
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

static OMX_U64 nOutBufIndex = 0;

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
    FunctionIn();

    if (hComponent == NULL)
    {
        ret = OMX_ErrorBadParameter;
        goto EXIT;
    }
    if (pSfVideoImplement->functions->AttachDMABuffer(pComponentRender, (Uint64)pBuffer, nSizeBytes) == FALSE)
    {
        LOG(SF_LOG_ERR, "Failed to attach dma buffer\r\n");
        return OMX_ErrorInsufficientResources;
    }
    OMX_BUFFERHEADERTYPE *temp_bufferHeader = (OMX_BUFFERHEADERTYPE *)malloc(sizeof(OMX_BUFFERHEADERTYPE));
    if (temp_bufferHeader == NULL)
    {
        LOG(SF_LOG_ERR, "malloc fail\r\n");
        return OMX_ErrorInsufficientResources;
    }
    memset(temp_bufferHeader, 0, sizeof(OMX_BUFFERHEADERTYPE));
    temp_bufferHeader->nAllocLen = nSizeBytes;
    temp_bufferHeader->pAppPrivate = pAppPrivate;
    temp_bufferHeader->pBuffer = pBuffer;
    *ppBufferHdr = temp_bufferHeader;
    if (nPortIndex == 1)
    {
        temp_bufferHeader->pOutputPortPrivate = (OMX_PTR)nOutBufIndex;
        nOutBufIndex ++;
    }
    ret = StoreOMXBuffer(pSfOMXComponent, temp_bufferHeader);
    LOG(SF_LOG_INFO, "pBuffer address = %p, nOutBufIndex = %d\r\n", temp_bufferHeader->pBuffer, (OMX_U64)(temp_bufferHeader->pOutputPortPrivate));
    LOG(SF_LOG_PERF, "alloc size = %d, buffer count = %d\r\n",nSizeBytes, GetOMXBufferCount(pSfOMXComponent));
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

    temp_bufferHeader->nAllocLen = nSizeBytes;
    temp_bufferHeader->pAppPrivate = pAppPrivate;
    if (nPortIndex == 1)
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
            LOG(SF_LOG_PERF, "Use normal buffer\r\n");
        }
        else
        {
            LOG(SF_LOG_PERF, "Use DMA buffer\r\n");
        }
        temp_bufferHeader->pOutputPortPrivate = (OMX_PTR)nOutBufIndex;
        nOutBufIndex ++;
    }
    else if (nPortIndex == 0)
    {
        temp_bufferHeader->pBuffer = malloc(nSizeBytes);

        memset(temp_bufferHeader->pBuffer, 0, nSizeBytes);
    }
    if (temp_bufferHeader->pBuffer == NULL)
    {
        free(temp_bufferHeader);
        LOG(SF_LOG_ERR, "malloc fail\r\n");
        return OMX_ErrorInsufficientResources;
    }
    *ppBuffer = temp_bufferHeader;
    LOG(SF_LOG_INFO, "pBuffer address = %p\r\n", temp_bufferHeader->pBuffer);
    ret = StoreOMXBuffer(pSfOMXComponent, temp_bufferHeader);
    LOG(SF_LOG_PERF, "alloc size = %d, buffer count = %d\r\n",nSizeBytes, GetOMXBufferCount(pSfOMXComponent));

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
    if (hComponent == NULL)
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
    case OMX_IndexParamPortDefinition:
    {
        OMX_PARAM_PORTDEFINITIONTYPE *pPortDefinition = (OMX_PARAM_PORTDEFINITIONTYPE *)ComponentParameterStructure;
        OMX_U32 portIndex = pPortDefinition->nPortIndex;
        OMX_PARAM_PORTDEFINITIONTYPE *pSrcDefinition = &pSfOMXComponent->portDefinition[portIndex];
        LOG(SF_LOG_INFO, "Get parameter width = %d, height = %d from port %X\r\n",
            pSrcDefinition->format.video.nFrameWidth, pSrcDefinition->format.video.nFrameHeight, portIndex);
        memcpy(pPortDefinition, pSrcDefinition, pPortDefinition->nSize);
    }

    break;
    case OMX_IndexParamVideoIntraRefresh:

        break;

    case OMX_IndexParamStandardComponentRole:

        break;
    case OMX_IndexParamVideoAvc:
        break;
    case OMX_IndexParamVideoHevc:
        break;
    case OMX_IndexParamVideoProfileLevelQuerySupported:

        break;

    default:

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
    if (hComponent == NULL)
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
                pSfVideoImplement->functions->ComponentPortDestroy(&pFeederComponent->srcPort);
                pSfVideoImplement->functions->ComponentPortCreate(&pFeederComponent->srcPort, pSfVideoImplement->hSFComponentFeeder,
                                                                pPortDefinition->nBufferCountActual, sizeof(PortContainerExternal));
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
                ComponentImpl *pRenderComponent = (ComponentImpl *)pSfVideoImplement->hSFComponentRender;
                pSfVideoImplement->functions->SetRenderTotalBufferNumber(pRenderComponent, pPortDefinition->nBufferCountActual);
                LOG(SF_LOG_INFO, "Set output buffer count = %d\r\n", pPortDefinition->nBufferCountActual);
                pSfVideoImplement->functions->ComponentPortDestroy(&pRenderComponent->sinkPort);
                pSfVideoImplement->functions->ComponentPortCreate(&pRenderComponent->sinkPort, pSfVideoImplement->hSFComponentRender,
                                                                pPortDefinition->nBufferCountActual, sizeof(PortContainerExternal));
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
    }
    break;
    case OMX_IndexParamVideoIntraRefresh:

        break;

    case OMX_IndexParamStandardComponentRole:

        break;
    case OMX_IndexParamVideoAvc:
        testConfig->bitFormat = 0;
        break;
    case OMX_IndexParamVideoHevc:
        testConfig->bitFormat = 12;
        break;
    default:

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

static OMX_ERRORTYPE InitDecoder(SF_OMX_COMPONENT *pSfOMXComponent)
{
    TestDecConfig *testConfig = NULL;
    CNMComponentConfig *config = NULL;
    Uint32 sizeInWord;
    char *fwPath = NULL;
    SF_WAVE5_IMPLEMEMT *pSfVideoImplement = (SF_WAVE5_IMPLEMEMT *)pSfOMXComponent->componentImpl;

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

    config = pSfVideoImplement->config;
    memcpy(&(config->testDecConfig), testConfig, sizeof(TestDecConfig));
    config->bitcode = (Uint8 *)pSfVideoImplement->pusBitCode;
    config->sizeOfBitcode = sizeInWord;

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
    pSfVideoImplement->functions->ComponentPortCreate(&pFeederComponent->srcPort, pSfVideoImplement->hSFComponentFeeder, VPU_INPUT_BUF_NUMBER, sizeof(PortContainerExternal));

    ComponentImpl *pRenderComponent = (ComponentImpl *)pSfVideoImplement->hSFComponentRender;
    pSfVideoImplement->functions->ComponentPortDestroy(&pRenderComponent->sinkPort);
    pSfVideoImplement->functions->ComponentPortCreate(&pRenderComponent->sinkPort, pSfVideoImplement->hSFComponentRender, VPU_OUTPUT_BUF_NUMBER, sizeof(PortContainerExternal));

    if (pSfVideoImplement->functions->SetupDecListenerContext(pSfVideoImplement->lsnCtx, config, NULL) == TRUE)
    {
        pSfVideoImplement->functions->ComponentRegisterListener(pSfVideoImplement->hSFComponentExecoder,
                                                              COMPONENT_EVENT_DEC_ALL, pSfVideoImplement->functions->DecoderListener, (void *)pSfVideoImplement->lsnCtx);
        pSfVideoImplement->functions->ComponentRegisterListener(pSfVideoImplement->hSFComponentRender,
                                                              COMPONENT_EVENT_DEC_DECODED_ALL, OnEventArrived, (void *)pSfVideoImplement->lsnCtx);
        pSfVideoImplement->functions->ComponentRegisterListener(pSfVideoImplement->hSFComponentFeeder,
                                                              COMPONENT_EVENT_DEC_ALL, OnEventArrived, (void *)pSfVideoImplement->lsnCtx);
        pSfVideoImplement->functions->ComponentRegisterListener(pSfVideoImplement->hSFComponentRender,
                                                              COMPONENT_EVENT_DEC_ALL, OnEventArrived, (void *)pSfVideoImplement->lsnCtx);
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
    ComponentImpl *pSFComponentDecoder = NULL;
    ComponentImpl *pSFComponentFeeder = NULL;
    ComponentImpl *pSFComponentRender = NULL;
    ComponentState componentState;

    FunctionIn();
    if (hComponent == NULL || pSfOMXComponent == NULL)
    {
        ret = OMX_ErrorBadParameter;
        goto EXIT;
    }


    LOG(SF_LOG_INFO, "cmd = %X, nParam = %X\r\n", Cmd, nParam);
    switch (Cmd)
    {
    case OMX_CommandStateSet:
        pSfOMXComponent->nextState = nParam;
        LOG(SF_LOG_INFO, "OMX dest state = %X\r\n", nParam);
        switch (nParam)
        {
        case OMX_StateLoaded:
            if (pSfVideoImplement->hSFComponentExecoder == NULL)
            {
                break;
            }
            pSFComponentDecoder = (ComponentImpl *)pSfVideoImplement->hSFComponentExecoder;
            pSFComponentFeeder = (ComponentImpl *)pSfVideoImplement->hSFComponentFeeder;
            pSFComponentRender = (ComponentImpl *)pSfVideoImplement->hSFComponentRender;
            componentState = pSfVideoImplement->functions->ComponentGetState(pSfVideoImplement->hSFComponentExecoder);
            LOG(SF_LOG_INFO, "VPU Current state = %X\r\n", componentState);
            switch(componentState)
            {
                case COMPONENT_STATE_CREATED:
                case COMPONENT_STATE_PREPARED:
                case COMPONENT_STATE_EXECUTED:
                case COMPONENT_STATE_TERMINATED:
                    if (pSfVideoImplement->functions->Queue_Get_Cnt(pSFComponentRender->sinkPort.inputQ) > 0 ||
                        pSfVideoImplement->functions->Queue_Get_Cnt(pSFComponentFeeder->srcPort.inputQ) > 0)
                    {
                        LOG(SF_LOG_ERR, "Buffer not flush!\r\n")
                        // ret = OMX_ErrorIncorrectStateTransition;
                        break;
                    }
                    pSFComponentDecoder->terminate = OMX_TRUE;
                    pSFComponentFeeder->terminate = OMX_TRUE;
                    pSFComponentRender->terminate = OMX_TRUE;
                    break;
                case COMPONENT_STATE_NONE:
                default:
                    ret = OMX_ErrorIncorrectStateTransition;
                    break;
            }
            break;
        case OMX_StateIdle:
            if (pSfVideoImplement->hSFComponentExecoder == NULL)
            {
                ret = InitDecoder(pSfOMXComponent);
                if (ret != OMX_ErrorNone)
                {
                    goto EXIT;
                }
            }
            pSFComponentDecoder = (ComponentImpl *)pSfVideoImplement->hSFComponentExecoder;
            pSFComponentFeeder = (ComponentImpl *)pSfVideoImplement->hSFComponentFeeder;
            pSFComponentRender = (ComponentImpl *)pSfVideoImplement->hSFComponentRender;
            componentState = pSfVideoImplement->functions->ComponentGetState(pSfVideoImplement->hSFComponentExecoder);
            LOG(SF_LOG_INFO, "VPU Current state = %X\r\n", componentState);
            switch(componentState)
            {
            case COMPONENT_STATE_NONE:
                ret = OMX_ErrorIncorrectStateTransition;
                break;
            case COMPONENT_STATE_CREATED:
                break;
            case COMPONENT_STATE_PREPARED:
            case COMPONENT_STATE_EXECUTED:
            case COMPONENT_STATE_TERMINATED:
                {
                    pSFComponentDecoder->pause = OMX_TRUE;
                    pSFComponentFeeder->pause = OMX_TRUE;
                    pSFComponentRender->pause = OMX_TRUE;
                    FlushBuffer(pSfOMXComponent, 0);
                    FlushBuffer(pSfOMXComponent, 1);
                }
                break;
            case COMPONENT_STATE_MAX:
                break;
            }
            pSfOMXComponent->callbacks->EventHandler(pSfOMXComponent->pOMXComponent, pSfOMXComponent->pAppData,
                                                     OMX_EventCmdComplete, OMX_CommandStateSet, nParam, NULL);
            break;

        case OMX_StateExecuting:
            ret = InitDecoder(pSfOMXComponent);
            if (ret != OMX_ErrorNone)
            {
                goto EXIT;
            }
            pSFComponentDecoder = (ComponentImpl *)pSfVideoImplement->hSFComponentExecoder;
            pSFComponentFeeder = (ComponentImpl *)pSfVideoImplement->hSFComponentFeeder;
            pSFComponentRender = (ComponentImpl *)pSfVideoImplement->hSFComponentRender;

            componentState = pSfVideoImplement->functions->ComponentGetState(pSfVideoImplement->hSFComponentExecoder);
            LOG(SF_LOG_INFO, "VPU Current state = %X\r\n", componentState);
            switch(componentState)
            {
            case COMPONENT_STATE_NONE:
            case COMPONENT_STATE_TERMINATED:
                ret = OMX_ErrorIncorrectStateTransition;
                break;
            case COMPONENT_STATE_CREATED:
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
                break;
            case COMPONENT_STATE_PREPARED:
            case COMPONENT_STATE_EXECUTED:
                pSFComponentDecoder->pause = OMX_FALSE;
                pSFComponentFeeder->pause = OMX_FALSE;
                pSFComponentRender->pause = OMX_FALSE;
                break;
            case COMPONENT_STATE_MAX:
                break;
            }

            break;
        default:
            ret = OMX_ErrorIncorrectStateTransition;
            break;
        }
        pSfOMXComponent->callbacks->EventHandler(pSfOMXComponent->pOMXComponent, pSfOMXComponent->pAppData,
                                                 OMX_EventCmdComplete, OMX_CommandStateSet, nParam, NULL);
        break;
    case OMX_CommandFlush:
    {
        OMX_U32 nPort = nParam;
        FlushBuffer(pSfOMXComponent, nPort);
        pSfOMXComponent->callbacks->EventHandler(pSfOMXComponent->pOMXComponent, pSfOMXComponent->pAppData,
                                                 OMX_EventCmdComplete, OMX_CommandFlush, nParam, NULL);
    }
    break;
    case OMX_CommandPortDisable:
        pSfOMXComponent->callbacks->EventHandler(pSfOMXComponent->pOMXComponent, pSfOMXComponent->pAppData,
                                                 OMX_EventCmdComplete, OMX_CommandPortDisable, nParam, NULL);
        break;
    case OMX_CommandPortEnable:
        pSfOMXComponent->callbacks->EventHandler(pSfOMXComponent->pOMXComponent, pSfOMXComponent->pAppData,
                                                 OMX_EventCmdComplete, OMX_CommandPortEnable, nParam, NULL);
        break;
    case OMX_CommandMarkBuffer:

        break;
    default:
        break;
    }

EXIT:
    FunctionOut();
    return ret;
}

static OMX_ERRORTYPE SF_OMX_GetState(
    OMX_IN OMX_HANDLETYPE hComponent,
    OMX_OUT OMX_STATETYPE *pState)
{
    OMX_ERRORTYPE ret = OMX_ErrorNone;
    FunctionIn();
    GetStateCommon(hComponent, pState);

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

    FunctionIn();
    if (ClearOMXBuffer(pSfOMXComponent, pBufferHdr) != OMX_ErrorNone)
    {
        LOG(SF_LOG_ERR, "Could not found pBufferHdr = %p\r\n", pBufferHdr);
        return OMX_ErrorBadParameter;
    }
    LOG(SF_LOG_PERF, "buffer count = %d\r\n", GetOMXBufferCount(pSfOMXComponent));
    if (nPortIndex == 1) {
        LOG(SF_LOG_INFO, "free %p on output buffer\r\n", pBufferHdr->pBuffer);
        pBufferHdr->pBuffer = NULL;
    } else {
        free(pBufferHdr->pBuffer);
        pBufferHdr->pBuffer = NULL;
    }

    free(pBufferHdr);

    FunctionOut();
    return ret;
}

static OMX_ERRORTYPE SF_OMX_ComponentConstructor(SF_OMX_COMPONENT *pSfOMXComponent)
{
    OMX_ERRORTYPE ret = OMX_ErrorNone;
    FunctionIn();

    ret = InitComponentStructorCommon(pSfOMXComponent);
    if (ret != OMX_ErrorNone)
    {
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
    // pSfOMXComponent->pOMXComponent->ComponentTunnelRequest = &SF_OMX_ComponentTunnelRequest;
    pSfOMXComponent->pOMXComponent->GetParameter = &SF_OMX_GetParameter;
    pSfOMXComponent->pOMXComponent->SetParameter = &SF_OMX_SetParameter;
    pSfOMXComponent->pOMXComponent->GetConfig = &SF_OMX_GetConfig;
    pSfOMXComponent->pOMXComponent->SetConfig = &SF_OMX_SetConfig;
    pSfOMXComponent->pOMXComponent->SendCommand = &SF_OMX_SendCommand;
    pSfOMXComponent->pOMXComponent->GetState = &SF_OMX_GetState;
    // pSfOMXComponent->pOMXComponent->GetExtensionIndex = &SF_OMX_GetExtensionIndex;
    // pSfOMXComponent->pOMXComponent->ComponentRoleEnum = &SF_OMX_ComponentRoleEnum;
    // pSfOMXComponent->pOMXComponent->ComponentDeInit = &SF_OMX_ComponentDeInit;

EXIT:
    FunctionOut();

    return ret;
}

static OMX_ERRORTYPE SF_OMX_ComponentClear(SF_OMX_COMPONENT *pSfOMXComponent)
{
    OMX_ERRORTYPE ret = OMX_ErrorNone;
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

    free(pSfVideoImplement->pusBitCode);
    pSfVideoImplement->functions->ClearDecListenerContext(pSfVideoImplement->lsnCtx);
    ComponentClearCommon(pSfOMXComponent);
EXIT:
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