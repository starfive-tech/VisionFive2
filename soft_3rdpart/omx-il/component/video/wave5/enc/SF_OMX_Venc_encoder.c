// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2021 StarFive Technology Co., Ltd.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "SF_OMX_video_common.h"
#include "SF_OMX_Core.h"

#define WAVE521_CONFIG_FILE "/lib/firmware/encoder_defconfig.cfg"
extern OMX_TICKS gInitTimeStamp;
OMX_U32 tmpFramerate;
OMX_S64 tmpCounter=0;

static char* Event2Str(unsigned long event)
{
    char *event_str = NULL;
    switch (event)
    {
    case COMPONENT_EVENT_ENC_OPEN:
        event_str = "COMPONENT_EVENT_ENC_OPEN";
        break;
    case COMPONENT_EVENT_ENC_ISSUE_SEQ:
        event_str = "COMPONENT_EVENT_ENC_ISSUE_SEQ";
        break;
    case COMPONENT_EVENT_ENC_COMPLETE_SEQ:
        event_str = "COMPONENT_EVENT_ENC_COMPLETE_SEQ";
        break;
    case COMPONENT_EVENT_ENC_REGISTER_FB:
        event_str = "COMPONENT_EVENT_ENC_REGISTER_FB";
        break;
    case COMPONENT_EVENT_ENC_READY_ONE_FRAME:
        event_str = "COMPONENT_EVENT_ENC_READY_ONE_FRAME";
        break;
    case COMPONENT_EVENT_ENC_START_ONE_FRAME:
        event_str = "COMPONENT_EVENT_ENC_START_ONE_FRAME";
        break;
    case COMPONENT_EVENT_ENC_GET_OUTPUT_INFO:
        event_str = "COMPONENT_EVENT_ENC_GET_OUTPUT_INFO";
        break;
    case COMPONENT_EVENT_ENC_ENCODED_ALL:
        event_str = "COMPONENT_EVENT_ENC_ENCODED_ALL";
        break;
    case COMPONENT_EVENT_ENC_CLOSE:
        event_str = "COMPONENT_EVENT_ENC_CLOSE";
        break;
    case COMPONENT_EVENT_ENC_EMPTY_BUFFER_DONE:
        event_str = "COMPONENT_EVENT_ENC_EMPTY_BUFFER_DONE";
        break;
    case COMPONENT_EVENT_ENC_FILL_BUFFER_DONE:
        event_str = "COMPONENT_EVENT_ENC_FILL_BUFFER_DONE";
        break;
    case COMPONENT_EVENT_ENC_ALL:
        event_str = "COMPONENT_EVENT_ENC_ALL";
        break;
    }
    return event_str;
}

static void OnEventArrived(Component com, unsigned long event, void *data, void *context)
{
    FunctionIn();
    SF_OMX_COMPONENT *pSfOMXComponent = GetSFOMXComponrntByComponent(com);
    SF_WAVE5_IMPLEMEMT *pSfVideoImplement = (SF_WAVE5_IMPLEMEMT *)pSfOMXComponent->componentImpl;
    static OMX_U32 enc_cnt = 0;
    static struct timeval tv_old = {0};
    OMX_U32 fps = 0;
    OMX_U64 diff_time = 0; // ms
    PortContainerExternal *pPortContainerExternal = (PortContainerExternal *)data;
    OMX_BUFFERHEADERTYPE *pOMXBuffer;
    char *event_str = Event2Str(event);

    LOG(SF_LOG_INFO, "event=%lX %s\r\n", event, event_str);
    switch (event)
    {
    case COMPONENT_EVENT_ENC_EMPTY_BUFFER_DONE:
        pOMXBuffer = GetOMXBufferByAddr(pSfOMXComponent, (OMX_U8 *)pPortContainerExternal->pBuffer);
        if (pOMXBuffer == NULL)
        {
            LOG(SF_LOG_WARN, "Could not find omx buffer by address\r\n");
            return;
        }
        pSfOMXComponent->callbacks->EmptyBufferDone(pSfOMXComponent->pOMXComponent, pSfOMXComponent->pAppData, pOMXBuffer);
        LOG(SF_LOG_PERF, "OMX empty one buffer, address = %p, size = %d, nTimeStamp = %d, nFlags = %X\r\n",
        pOMXBuffer->pBuffer, pOMXBuffer->nFilledLen, pOMXBuffer->nTimeStamp, pOMXBuffer->nFlags);
        ClearOMXBuffer(pSfOMXComponent, pOMXBuffer);
        break;
    case COMPONENT_EVENT_ENC_FILL_BUFFER_DONE:
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
        // pOMXBuffer->nTimeStamp = tv.tv_sec * 1000000 + tv.tv_usec - gInitTimeStamp;
        pOMXBuffer->nTimeStamp = (tmpCounter*1000000)/tmpFramerate;
        tmpCounter++;
        LOG(SF_LOG_INFO,"test nTimeStamp:%ld\r\n",pOMXBuffer->nTimeStamp);
        pOMXBuffer->nFlags = pPortContainerExternal->nFlags;
#if 0
            {
                FILE *fb = fopen("./out.bcp", "ab+");
                LOG(SF_LOG_ERR, "%d %d\r\n", pOMXBuffer->nFilledLen, pOMXBuffer->nOffset);
                fwrite(pOMXBuffer->pBuffer, 1, pOMXBuffer->nFilledLen, fb);
                fclose(fb);
            }
#endif
        if (enc_cnt == 0) {
            tv_old = tv;
        }
        if (enc_cnt++ >= 50) {
            diff_time = (tv.tv_sec - tv_old.tv_sec) * 1000 + (tv.tv_usec - tv_old.tv_usec) / 1000;
            fps = 1000  * (enc_cnt - 1) / diff_time;
            enc_cnt = 0;
            LOG(SF_LOG_WARN, "Encoding fps: %d \r\n", fps);
        }
        LOG(SF_LOG_PERF, "OMX finish one buffer, address = %p, size = %d, nTimeStamp = %d, nFlags = %X\r\n", pOMXBuffer->pBuffer, pOMXBuffer->nFilledLen, pOMXBuffer->nTimeStamp, pOMXBuffer->nFlags);
        pSfOMXComponent->callbacks->FillBufferDone(pSfOMXComponent->pOMXComponent, pSfOMXComponent->pAppData, pOMXBuffer);
        ComponentImpl *pRendererComponent = (ComponentImpl *)pSfVideoImplement->hSFComponentRender;
        LOG(SF_LOG_PERF, "output queue count=%d/%d\r\n", pSfVideoImplement->functions->Queue_Get_Cnt(pRendererComponent->sinkPort.inputQ),
                                                    pSfOMXComponent->portDefinition[1].nBufferCountActual);
        ClearOMXBuffer(pSfOMXComponent, pOMXBuffer);
    }
    break;
    case COMPONENT_EVENT_ENC_REGISTER_FB:
    {
        pSfOMXComponent->callbacks->EventHandler(pSfOMXComponent->pOMXComponent, pSfOMXComponent->pAppData, OMX_EventPortSettingsChanged,
                                                 1, OMX_IndexParamPortDefinition, NULL);
    }
    break;
    case COMPONENT_EVENT_ENC_ENCODED_ALL:
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
    ComponentImpl *pFeederComponent = (ComponentImpl *)(pSfVideoImplement->hSFComponentFeeder);

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
    pPortContainerExternal->nBufferIndex = (OMX_U32)pBuffer->pInputPortPrivate;
    LOG(SF_LOG_INFO, "Index = %lu, Address = %p, Flag = %X\r\n",(OMX_U64)(pBuffer->pInputPortPrivate), pBuffer->pBuffer, pBuffer->nFlags);
    if (pSfVideoImplement->functions->Queue_Enqueue(pFeederComponent->srcPort.inputQ, (void *)pPortContainerExternal) != OMX_TRUE)
    {
        LOG(SF_LOG_ERR, "%p:%p FAIL\r\n", pFeederComponent->srcPort.inputQ, pPortContainerExternal);
        free(pPortContainerExternal);
        return OMX_ErrorInsufficientResources;
    }
    LOG(SF_LOG_PERF, "input queue count=%d/%d\r\n", pSfVideoImplement->functions->Queue_Get_Cnt(pFeederComponent->srcPort.inputQ),
                                                    pSfOMXComponent->portDefinition[0].nBufferCountActual);
    free(pPortContainerExternal);
    pFeederComponent->pause = OMX_FALSE;

    ret = StoreOMXBuffer(pSfOMXComponent, pBuffer);
    LOG(SF_LOG_PERF, "buffer count = %d\r\n", GetOMXBufferCount(pSfOMXComponent));

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
    ComponentImpl *pRendererComponent = (ComponentImpl *)(pSfVideoImplement->hSFComponentRender);
    PortContainerExternal *pPortContainerExternal = malloc(sizeof(PortContainerExternal));
    if (pPortContainerExternal == NULL)
    {
        LOG(SF_LOG_ERR, "malloc fail\r\n");
        return OMX_ErrorInsufficientResources;
    }
    memset(pPortContainerExternal, 0, sizeof(PortContainerExternal));
    pPortContainerExternal->pBuffer = pBuffer->pBuffer;
    pPortContainerExternal->nFilledLen = pBuffer->nAllocLen;
    if (pSfVideoImplement->functions->Queue_Enqueue(pRendererComponent->sinkPort.inputQ, (void *)pPortContainerExternal) == -1)
    {
        LOG(SF_LOG_ERR, "%p:%p FAIL\r\n", pRendererComponent->sinkPort.inputQ, pPortContainerExternal);
        free(pPortContainerExternal);
        return OMX_ErrorInsufficientResources;
    }
    LOG(SF_LOG_PERF, "output queue count=%d/%d\r\n", pSfVideoImplement->functions->Queue_Get_Cnt(pRendererComponent->sinkPort.inputQ),
                                                        pSfOMXComponent->portDefinition[1].nBufferCountActual);
    free(pPortContainerExternal);
    pRendererComponent->pause = OMX_FALSE;
    ret = StoreOMXBuffer(pSfOMXComponent, pBuffer);
    LOG(SF_LOG_PERF, "buffer count = %d\r\n", GetOMXBufferCount(pSfOMXComponent));
EXIT:
    FunctionOut();

    return ret;
}

static OMX_U64 nInputBufIndex = 0;

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
    ComponentImpl *pComponentFeeder = (ComponentImpl *)pSfVideoImplement->hSFComponentFeeder;
    FunctionIn();

    if (hComponent == NULL)
    {
        ret = OMX_ErrorBadParameter;
        goto EXIT;
    }
    if (pSfVideoImplement->functions->AttachDMABuffer(pComponentFeeder, (Uint64)pBuffer, nSizeBytes) == FALSE)
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
    if (nPortIndex == 0)
    {
        temp_bufferHeader->pInputPortPrivate = (OMX_PTR)nInputBufIndex;
        nInputBufIndex ++;
    }
    LOG(SF_LOG_INFO, "pBuffer address = %p, nInputBufIndex = %d\r\n", temp_bufferHeader->pBuffer, (OMX_U64)temp_bufferHeader->pInputPortPrivate);
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
    ComponentImpl *pComponentFeeder = (ComponentImpl *)pSfVideoImplement->hSFComponentFeeder;
    // OMX_U32 i = 0;

    FunctionIn();
    if (nSizeBytes == 0)
    {
        LOG(SF_LOG_ERR, "nSizeBytes = %d\r\n", nSizeBytes);
        return OMX_ErrorBadParameter;
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

    if (nPortIndex == 0)
    {
        // Alloc DMA memory first
        if (pSfOMXComponent->memory_optimization)
        {
            temp_bufferHeader->pBuffer = pSfVideoImplement->functions->AllocateFrameBuffer2(pComponentFeeder, nSizeBytes);
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
            temp_bufferHeader->pInputPortPrivate = (OMX_PTR)nInputBufIndex;
            nInputBufIndex ++;
        }
    }
    else if (nPortIndex == 1)
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
    SF_WAVE5_IMPLEMEMT *pSfVideoImplement = (SF_WAVE5_IMPLEMEMT *)pSfOMXComponent->componentImpl;
    OMX_PARAM_PORTDEFINITIONTYPE *pOutputPort = &pSfOMXComponent->portDefinition[1];

    FunctionIn();

    if (hComponent == NULL)
    {
        ret = OMX_ErrorBadParameter;
        goto EXIT;
    }
    LOG(SF_LOG_INFO, "Get parameter on index %X\r\n", nParamIndex);
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
        LOG(SF_LOG_INFO, "Get video port format at index %d\r\n", index);
        switch (index)
        {
        case 0:
            portFormat->eCompressionFormat = OMX_VIDEO_CodingUnused;
            portFormat->eColorFormat = OMX_COLOR_FormatYUV420SemiPlanar;
            portFormat->xFramerate = 30;
            break;
        case 1:
            portFormat->eCompressionFormat = OMX_VIDEO_CodingUnused;
            portFormat->eColorFormat = OMX_COLOR_FormatYUV420Planar;
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
    {
        OMX_VIDEO_PARAM_BITRATETYPE     *videoRateControl = (OMX_VIDEO_PARAM_BITRATETYPE *)ComponentParameterStructure;
        OMX_U32                          portIndex = videoRateControl->nPortIndex;
        if ((portIndex != 1)) {
            ret = OMX_ErrorBadPortIndex;
            goto EXIT;
        }
        else
        {
            videoRateControl->nTargetBitrate = pOutputPort->format.video.nBitrate;
        }
        LOG(SF_LOG_INFO, "Get nTargetBitrate = %u on port %d\r\n",videoRateControl->nTargetBitrate, videoRateControl->nPortIndex);
    }
        break;
    case OMX_IndexParamVideoQuantization:

        break;
    case OMX_IndexParamPortDefinition:
    {
        OMX_PARAM_PORTDEFINITIONTYPE *pPortDefinition = (OMX_PARAM_PORTDEFINITIONTYPE *)ComponentParameterStructure;
        OMX_U32 portIndex = pPortDefinition->nPortIndex;
        memcpy(pPortDefinition, &pSfOMXComponent->portDefinition[portIndex], pPortDefinition->nSize);
        LOG(SF_LOG_INFO, "Get portDefinition on port %d\r\n", portIndex);
        LOG(SF_LOG_INFO, "Got width = %d, height = %d\r\n", pPortDefinition->format.video.nFrameWidth, pPortDefinition->format.video.nFrameHeight);
    }

    break;
    case OMX_IndexParamVideoIntraRefresh:

        break;

    case OMX_IndexParamStandardComponentRole:

        break;
    case OMX_IndexParamVideoAvc:
    {
        OMX_VIDEO_PARAM_AVCTYPE *pDstAVCComponent = (OMX_VIDEO_PARAM_AVCTYPE *)ComponentParameterStructure;
        OMX_U32 nPortIndex = pDstAVCComponent->nPortIndex;
        OMX_VIDEO_PARAM_AVCTYPE *pSrcAVCComponent = &pSfVideoImplement->AVCComponent[nPortIndex];

        LOG(SF_LOG_INFO, "Get nPFrames = %d on port %d\r\n", pSrcAVCComponent->nPFrames, nPortIndex);
        memcpy(pDstAVCComponent, pSrcAVCComponent, sizeof(OMX_VIDEO_PARAM_AVCTYPE));
    }
    break;
    case OMX_IndexParamVideoHevc:
    {
        OMX_VIDEO_PARAM_HEVCTYPE *pDstHEVCComponent = (OMX_VIDEO_PARAM_HEVCTYPE *)ComponentParameterStructure;
        OMX_U32 nPortIndex = pDstHEVCComponent->nPortIndex;
        OMX_VIDEO_PARAM_HEVCTYPE *pSrcHEVCComponent = &pSfVideoImplement->HEVCComponent[nPortIndex];

        LOG(SF_LOG_INFO, "Get nKeyFrameInterval = %d on port %d\r\n", pSrcHEVCComponent->nKeyFrameInterval, nPortIndex);
        memcpy(pDstHEVCComponent, pSrcHEVCComponent, sizeof(OMX_VIDEO_PARAM_HEVCTYPE));
    }
    break;
    case OMX_IndexParamVideoProfileLevelQuerySupported:

        break;
    case OMX_IndexParamVideoProfileLevelCurrent:
    {
        OMX_VIDEO_PARAM_PROFILELEVELTYPE *pParam = (OMX_VIDEO_PARAM_PROFILELEVELTYPE *)ComponentParameterStructure;
        LOG(SF_LOG_INFO, "Get ProfileLevel on port %d\r\n", pParam->nPortIndex);
        if (pParam->nPortIndex == 1)
        {
            //TODO
            pParam->eProfile = OMX_VIDEO_HEVCProfileMain;
            pParam->eLevel = OMX_VIDEO_HEVCMainTierLevel5;
            LOG(SF_LOG_INFO, "eProfile = OMX_VIDEO_HEVCProfileMain\r\n");
        }
    }
        // ret = OMX_ErrorNotImplemented;
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
    if (hComponent == NULL)
    {
        ret = OMX_ErrorBadParameter;
        goto EXIT;
    }

    OMX_COMPONENTTYPE *pOMXComponent = (OMX_COMPONENTTYPE *)hComponent;
    SF_OMX_COMPONENT *pSfOMXComponent = pOMXComponent->pComponentPrivate;
    SF_WAVE5_IMPLEMEMT *pSfVideoImplement = (SF_WAVE5_IMPLEMEMT *)pSfOMXComponent->componentImpl;
    OMX_PARAM_PORTDEFINITIONTYPE *pInputPort = &pSfOMXComponent->portDefinition[0];
    OMX_PARAM_PORTDEFINITIONTYPE *pOutputPort = &pSfOMXComponent->portDefinition[1];

    if (pSfOMXComponent == NULL)
    {
        ret = OMX_ErrorBadParameter;
        goto EXIT;
    }
    LOG(SF_LOG_INFO, "Set parameter on index %X\r\n", nIndex);
    switch ((OMX_U32)nIndex)
    {
    case OMX_IndexParamPortDefinition:
    {
        OMX_PARAM_PORTDEFINITIONTYPE *pPortDefinition = (OMX_PARAM_PORTDEFINITIONTYPE *)ComponentParameterStructure;
        OMX_U32 portIndex = pPortDefinition->nPortIndex;
        OMX_U32 width = pPortDefinition->format.video.nFrameWidth;
        OMX_U32 height = pPortDefinition->format.video.nFrameHeight;
        OMX_U32 nBitrate = pPortDefinition->format.video.nBitrate;
        OMX_U32 xFramerate = pPortDefinition->format.video.xFramerate;
        tmpFramerate = xFramerate;
        OMX_U32 nBufferCountActual = pPortDefinition->nBufferCountActual;
        LOG(SF_LOG_INFO, "Set width = %d, height = %d, xFramerate = %d, nBitrate = %d, nBufferCountActual = %d on port %d\r\n",
            width, height, xFramerate, nBitrate, nBufferCountActual, portIndex);
        OMX_COLOR_FORMATTYPE eColorFormat = pPortDefinition->format.video.eColorFormat;
        TestEncConfig *pTestEncConfig = (TestEncConfig *)pSfVideoImplement->testConfig;

        if (xFramerate == 0)
        {
            LOG(SF_LOG_ERR, "Fail to set xFramerate = %d\r\n", xFramerate);
            return OMX_ErrorBadParameter;
        }
        if (portIndex == 0)
        {
            if (nBufferCountActual != pInputPort->nBufferCountActual)
            {
                LOG(SF_LOG_INFO, "Set input buffer count = %d\r\n", nBufferCountActual);
                ComponentImpl *pFeederComponent = (ComponentImpl *)pSfVideoImplement->hSFComponentFeeder;
                pSfVideoImplement->functions->SetFeederTotalBufferNumber(pFeederComponent, nBufferCountActual);
                pSfVideoImplement->functions->ComponentPortDestroy(&pFeederComponent->srcPort);
                pSfVideoImplement->functions->ComponentPortCreate(&pFeederComponent->srcPort, pSfVideoImplement->hSFComponentFeeder,
                                                                nBufferCountActual, sizeof(PortContainerExternal));
            }

            memcpy(&pSfOMXComponent->portDefinition[portIndex], pPortDefinition, pPortDefinition->nSize);
            pInputPort->format.video.nStride = width;
            pInputPort->format.video.nSliceHeight = height;
            pInputPort->nBufferSize = width * height * 3 / 2;
            LOG(SF_LOG_INFO, "Set eColorFormat to %d\r\n", eColorFormat);
            switch (eColorFormat)
            {
            case OMX_COLOR_FormatYUV420Planar: //I420
                pTestEncConfig->cbcrInterleave = FALSE;
                pTestEncConfig->nv21 = FALSE;
                break;
            case OMX_COLOR_FormatYUV420SemiPlanar: //NV12
                pTestEncConfig->cbcrInterleave = TRUE;
                pTestEncConfig->nv21 = FALSE;
                break;
            case OMX_COLOR_FormatYVU420SemiPlanar: //NV21
                pTestEncConfig->cbcrInterleave = TRUE;
                pTestEncConfig->nv21 = TRUE;
                break;
            default:
                break;
            }
        }
        else if (portIndex == 1)
        {
            LOG(SF_LOG_INFO, "Current output buffer count = %d\r\n", pOutputPort->nBufferCountActual);
            if (nBufferCountActual != pOutputPort->nBufferCountActual)
            {
                ComponentImpl *pRenderComponent = (ComponentImpl *)pSfVideoImplement->hSFComponentRender;
                LOG(SF_LOG_INFO, "Set output buffer count = %d\r\n", nBufferCountActual);
                pSfVideoImplement->functions->ComponentPortDestroy(&pRenderComponent->sinkPort);
                pSfVideoImplement->functions->ComponentPortCreate(&pRenderComponent->sinkPort, pSfVideoImplement->hSFComponentRender,
                                                                nBufferCountActual, sizeof(PortContainerExternal));
            }

            memcpy(&pSfOMXComponent->portDefinition[portIndex], pPortDefinition, pPortDefinition->nSize);
            width = pInputPort->format.video.nFrameWidth;
            height = pInputPort->format.video.nFrameHeight;
            pOutputPort->format.video.nFrameWidth = width;
            pOutputPort->format.video.nFrameHeight = height;
            pOutputPort->format.video.nStride = width;
            pOutputPort->format.video.nSliceHeight = height;
            switch (eColorFormat)
            {
            case OMX_COLOR_FormatYUV420Planar:
            case OMX_COLOR_FormatYUV420SemiPlanar:
            case OMX_COLOR_FormatYVU420SemiPlanar:
                if (width && height)
                    pOutputPort->nBufferSize = (width * height * 3) / 2;
                break;
            default:
                if (width && height)
                    pOutputPort->nBufferSize = width * height * 2;
                break;
            }
        }
    }
    break;
    case OMX_IndexParamVideoPortFormat:
        {
            OMX_VIDEO_PARAM_PORTFORMATTYPE *portFormat = (OMX_VIDEO_PARAM_PORTFORMATTYPE *)ComponentParameterStructure;
            TestEncConfig *pTestEncConfig = (TestEncConfig *)pSfVideoImplement->testConfig;
            OMX_COLOR_FORMATTYPE eColorFormat = portFormat->eColorFormat;
            LOG(SF_LOG_INFO, "Set eColorFormat to %d\r\n", eColorFormat);
            switch (eColorFormat)
            {
            case OMX_COLOR_FormatYUV420Planar:
                pTestEncConfig->cbcrInterleave = OMX_FALSE;
                pTestEncConfig->nv21 = OMX_FALSE;
                break;
            case OMX_COLOR_FormatYUV420SemiPlanar:
                pTestEncConfig->cbcrInterleave = OMX_TRUE;
                pTestEncConfig->nv21 = OMX_FALSE;
                break;
            default:
                ret = OMX_ErrorBadParameter;
                break;
            }
        }
    break;
    case OMX_IndexParamVideoBitrate:
    {
        OMX_VIDEO_PARAM_BITRATETYPE     *videoRateControl = (OMX_VIDEO_PARAM_BITRATETYPE *)ComponentParameterStructure;
        OMX_U32                          portIndex = videoRateControl->nPortIndex;

        LOG(SF_LOG_INFO, "Set nTargetBitrate = %u on port %d\r\n",videoRateControl->nTargetBitrate, portIndex);

        if ((portIndex != 1)) {
            ret = OMX_ErrorBadPortIndex;
            goto EXIT;
        }
        else
        {
            pOutputPort->format.video.nBitrate = videoRateControl->nTargetBitrate;
        }
        ret = OMX_ErrorNone;
    }
    break;
    case OMX_IndexConfigVideoAVCIntraPeriod:
    {
        OMX_VIDEO_CONFIG_AVCINTRAPERIOD *pAVCIntraPeriod = (OMX_VIDEO_CONFIG_AVCINTRAPERIOD *)ComponentParameterStructure;
        OMX_U32           portIndex = pAVCIntraPeriod->nPortIndex;

        LOG(SF_LOG_INFO, "Set nIDRPeriod = %d nPFrames = %d on port %d\r\n",pAVCIntraPeriod->nIDRPeriod, pAVCIntraPeriod->nPFrames, portIndex);
        if ((portIndex != 1)) {
            ret = OMX_ErrorBadPortIndex;
            goto EXIT;
        }
        else
        {
            pSfVideoImplement->AVCComponent[1].nPFrames = pAVCIntraPeriod->nPFrames;
        }
    }
    break;
    case OMX_IndexParamVideoAvc:
    {
        OMX_VIDEO_PARAM_AVCTYPE *pSrcAVCComponent= (OMX_VIDEO_PARAM_AVCTYPE *)ComponentParameterStructure;
        OMX_U32 nPortIndex = pSrcAVCComponent->nPortIndex;
        OMX_VIDEO_PARAM_AVCTYPE *pDstAVCComponent = &pSfVideoImplement->AVCComponent[nPortIndex];
        OMX_U32 nPFrames = pSrcAVCComponent->nPFrames;

        LOG(SF_LOG_INFO, "Set nPFrames = %d on port %d\r\n", nPFrames, nPortIndex);
        if (nPFrames == 1)
        {
            LOG(SF_LOG_ERR, "Set nPFrames = %d not supported!\r\n", nPFrames);
            ret = OMX_ErrorBadPortIndex;
            goto EXIT;
        }
        memcpy(pDstAVCComponent, pSrcAVCComponent, sizeof(OMX_VIDEO_PARAM_AVCTYPE));
    }
    break;
    case OMX_IndexParamVideoHevc:
    {
        OMX_VIDEO_PARAM_HEVCTYPE *pSrcHEVCComponent= (OMX_VIDEO_PARAM_HEVCTYPE *)ComponentParameterStructure;
        OMX_U32 nPortIndex = pSrcHEVCComponent->nPortIndex;
        OMX_VIDEO_PARAM_HEVCTYPE *pDstHEVCComponent = &pSfVideoImplement->HEVCComponent[nPortIndex];
        OMX_U32 nKeyFrameInterval = pSrcHEVCComponent->nKeyFrameInterval;

        LOG(SF_LOG_INFO, "Set nKeyFrameInterval = %d on port %d\r\n", nKeyFrameInterval, nPortIndex);
        if (nKeyFrameInterval == 1)
        {
            LOG(SF_LOG_ERR, "Set nKeyFrameInterval = %d not supported!\r\n", nKeyFrameInterval);
            ret = OMX_ErrorBadPortIndex;
            goto EXIT;
        }
        memcpy(pDstHEVCComponent, pSrcHEVCComponent, sizeof(OMX_VIDEO_PARAM_HEVCTYPE));
    }
    break;
    case OMX_IndexParamVideoQuantization:
    case OMX_IndexParamVideoIntraRefresh:
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

static OMX_ERRORTYPE InitEncoder(SF_OMX_COMPONENT *pSfOMXComponent)
{
    TestEncConfig *testConfig = NULL;
    CNMComponentConfig *config = NULL;
    Uint32 sizeInWord;
    Int32 alignedWidth = 0, alignedHeight = 0;
    char *fwPath = NULL;
    SF_WAVE5_IMPLEMEMT *pSfVideoImplement = (SF_WAVE5_IMPLEMEMT *)pSfOMXComponent->componentImpl;

    if (pSfVideoImplement->hSFComponentExecoder != NULL)
    {
        return OMX_ErrorNone;
    }

    FunctionIn();

    testConfig = (TestEncConfig *)pSfVideoImplement->testConfig;

    testConfig->productId = (ProductId)pSfVideoImplement->functions->VPU_GetProductId(testConfig->coreIdx);
    if (CheckEncTestConfig(testConfig) == FALSE)
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
    memcpy(&(config->testEncConfig), testConfig, sizeof(TestEncConfig));
    config->bitcode = (Uint8 *)pSfVideoImplement->pusBitCode;
    config->sizeOfBitcode = sizeInWord;
    LOG(SF_LOG_INFO, "cbcrInterleave= %d, nv21 = %d\r\n", config->testEncConfig.cbcrInterleave, config->testEncConfig.nv21);
    memcpy(config->testEncConfig.cfgFileName, WAVE521_CONFIG_FILE, sizeof(WAVE521_CONFIG_FILE));
    LOG(SF_LOG_INFO, "Get width = %d, height = %d \r\n", config->encOpenParam.picWidth, config->encOpenParam.picHeight);
    if (pSfVideoImplement->functions->SetupEncoderOpenParam(&config->encOpenParam, &config->testEncConfig, NULL) == OMX_FALSE)
    {
        LOG(SF_LOG_ERR, "SetupEncoderOpenParam error\n");
        return OMX_ErrorBadParameter;
    }
    config->encOpenParam.picWidth = pSfOMXComponent->portDefinition[0].format.video.nFrameWidth;
    config->encOpenParam.picHeight = pSfOMXComponent->portDefinition[0].format.video.nFrameHeight;

    /* If xFramerate value from user is stored in Q16 format, should convert into integer */
    if(pSfOMXComponent->portDefinition[0].format.video.xFramerate > (1 << 16))
    {
        config->encOpenParam.frameRateInfo = pSfOMXComponent->portDefinition[0].format.video.xFramerate >> 16;
    }
    else
    {
        config->encOpenParam.frameRateInfo = pSfOMXComponent->portDefinition[0].format.video.xFramerate;
    }

    if(pSfOMXComponent->portDefinition[1].format.video.nBitrate)
    {
        config->encOpenParam.rcEnable = 1;
        config->encOpenParam.bitRate = pSfOMXComponent->portDefinition[1].format.video.nBitrate;
    }
    else if(config->encOpenParam.rcEnable == 1)
    {
        pSfOMXComponent->portDefinition[1].format.video.nBitrate = config->encOpenParam.bitRate;
    }

    if (pSfVideoImplement->bitFormat == STD_AVC)
    {
        config->encOpenParam.EncStdParam.waveParam.intraPeriod = pSfVideoImplement->AVCComponent[1].nPFrames;
        alignedWidth = (config->encOpenParam.picWidth + 15) & ~15;
        alignedHeight= (config->encOpenParam.picHeight+ 15) & ~15;
        config->encOpenParam.EncStdParam.waveParam.confWinRight = alignedWidth - config->encOpenParam.picWidth;
        config->encOpenParam.EncStdParam.waveParam.confWinBot = alignedHeight - config->encOpenParam.picHeight;
    }
    else if (pSfVideoImplement->bitFormat == STD_HEVC)
    {
        config->encOpenParam.EncStdParam.waveParam.intraPeriod = pSfVideoImplement->HEVCComponent[1].nKeyFrameInterval;
    }
    LOG(SF_LOG_INFO, "Get width = %d, height = %d frameRateInfo = %d intraPeriod = %d bitrate = %d confWinRight = %d confWinBot = %d\r\n",
        config->encOpenParam.picWidth, config->encOpenParam.picHeight, config->encOpenParam.frameRateInfo,
        config->encOpenParam.EncStdParam.waveParam.intraPeriod, config->encOpenParam.bitRate,
        config->encOpenParam.EncStdParam.waveParam.confWinRight, config->encOpenParam.EncStdParam.waveParam.confWinBot);

    pSfVideoImplement->hSFComponentExecoder = pSfVideoImplement->functions->ComponentCreate("wave_encoder", config);
    pSfVideoImplement->hSFComponentFeeder = pSfVideoImplement->functions->ComponentCreate("yuvfeeder", config);
    pSfVideoImplement->hSFComponentRender = pSfVideoImplement->functions->ComponentCreate("reader", config);

    ComponentImpl *pFeederComponent = (ComponentImpl *)pSfVideoImplement->hSFComponentFeeder;
    pSfVideoImplement->functions->ComponentPortCreate(&pFeederComponent->srcPort, pSfVideoImplement->hSFComponentFeeder,
                    pSfOMXComponent->portDefinition[0].nBufferCountActual, sizeof(PortContainerExternal));

    ComponentImpl *pRenderComponent = (ComponentImpl *)pSfVideoImplement->hSFComponentRender;
    pSfVideoImplement->functions->ComponentPortDestroy(&pRenderComponent->sinkPort);
    pSfVideoImplement->functions->ComponentPortCreate(&pRenderComponent->sinkPort, pSfVideoImplement->hSFComponentRender,
                    pSfOMXComponent->portDefinition[1].nBufferCountActual, sizeof(PortContainerExternal));

    if (pSfVideoImplement->functions->SetupEncListenerContext(pSfVideoImplement->lsnCtx, config) == TRUE)
    {
        pSfVideoImplement->functions->ComponentRegisterListener(pSfVideoImplement->hSFComponentExecoder,
                                                              COMPONENT_EVENT_ENC_ALL, pSfVideoImplement->functions->EncoderListener, (void *)pSfVideoImplement->lsnCtx);
        pSfVideoImplement->functions->ComponentRegisterListener(pSfVideoImplement->hSFComponentExecoder,
                                                              COMPONENT_EVENT_ENC_REGISTER_FB, OnEventArrived, (void *)pSfVideoImplement->lsnCtx);
        pSfVideoImplement->functions->ComponentRegisterListener(pSfVideoImplement->hSFComponentExecoder,
                                                              COMPONENT_EVENT_ENC_ENCODED_ALL, OnEventArrived, (void *)pSfVideoImplement->lsnCtx);
        pSfVideoImplement->functions->ComponentRegisterListener(pSfVideoImplement->hSFComponentFeeder,
                                                              COMPONENT_EVENT_ENC_ALL, OnEventArrived, (void *)pSfVideoImplement->lsnCtx);
        pSfVideoImplement->functions->ComponentRegisterListener(pSfVideoImplement->hSFComponentRender,
                                                              COMPONENT_EVENT_ENC_ALL, OnEventArrived, (void *)pSfVideoImplement->lsnCtx);
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
    ComponentImpl *pSFComponentEncoder = NULL;
    ComponentImpl *pSFComponentFeeder = NULL;
    ComponentImpl *pSFComponentRender = NULL;
    ComponentState currentState;

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
        pSfOMXComponent->state = nParam;
        LOG(SF_LOG_INFO, "OMX dest state = %X\r\n", nParam);
        switch (nParam)
        {
        case OMX_StateLoaded:
            if (pSfVideoImplement->hSFComponentExecoder == NULL)
            {
                break;
            }
            pSFComponentEncoder = (ComponentImpl *)pSfVideoImplement->hSFComponentExecoder;
            pSFComponentFeeder = (ComponentImpl *)pSfVideoImplement->hSFComponentFeeder;
            pSFComponentRender = (ComponentImpl *)pSfVideoImplement->hSFComponentRender;
            currentState = pSfVideoImplement->functions->ComponentGetState(pSfVideoImplement->hSFComponentExecoder);
            LOG(SF_LOG_INFO, "VPU Current state = %X\r\n", currentState);
            switch(currentState)
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
                }
                break;
                pSFComponentEncoder->terminate = OMX_TRUE;
                pSFComponentFeeder->terminate = OMX_TRUE;
                pSFComponentRender->terminate = OMX_TRUE;
                case COMPONENT_STATE_NONE:
                default:
                    ret = OMX_ErrorIncorrectStateTransition;
                    break;
            }
            break;
        case OMX_StateIdle:
            if (pSfVideoImplement->hSFComponentExecoder == NULL)
            {
                ret = InitEncoder(pSfOMXComponent);
                if (ret != OMX_ErrorNone)
                {
                    goto EXIT;
                }
            }

            pSFComponentEncoder = (ComponentImpl *)pSfVideoImplement->hSFComponentExecoder;
            pSFComponentFeeder = (ComponentImpl *)pSfVideoImplement->hSFComponentFeeder;
            pSFComponentRender = (ComponentImpl *)pSfVideoImplement->hSFComponentRender;
            currentState = pSfVideoImplement->functions->ComponentGetState(pSfVideoImplement->hSFComponentExecoder);
            LOG(SF_LOG_INFO, "VPU Current state = %X\r\n", currentState);
            switch(currentState)
            {
            case COMPONENT_STATE_NONE:
                ret = OMX_ErrorIncorrectStateTransition;
                break;
            case COMPONENT_STATE_CREATED:
                if (pSFComponentEncoder->thread == NULL)
                {
                    LOG(SF_LOG_INFO, "execute component %s\r\n", pSFComponentEncoder->name);
                    currentState = pSfVideoImplement->functions->ComponentExecute(pSFComponentEncoder);
                    LOG(SF_LOG_INFO, "ret = %d\r\n", currentState);
                }
                if (pSFComponentFeeder->thread == NULL)
                {
                    LOG(SF_LOG_INFO, "execute component %s\r\n", pSFComponentFeeder->name);
                    currentState = pSfVideoImplement->functions->ComponentExecute(pSFComponentFeeder);
                    LOG(SF_LOG_INFO, "ret = %d\r\n", currentState);
                }
                if (pSFComponentRender->thread == NULL)
                {
                    LOG(SF_LOG_INFO, "execute component %s\r\n", pSFComponentRender->name);
                    currentState = pSfVideoImplement->functions->ComponentExecute(pSFComponentRender);
                    LOG(SF_LOG_INFO, "ret = %d\r\n", currentState);
                }
                pSfVideoImplement->functions->WaitForExecoderReady(pSFComponentEncoder);
                break;
            case COMPONENT_STATE_PREPARED:
            case COMPONENT_STATE_EXECUTED:
            case COMPONENT_STATE_TERMINATED:
                {
                    pSFComponentEncoder->pause = OMX_TRUE;
                    pSFComponentFeeder->pause = OMX_TRUE;
                    pSFComponentRender->pause = OMX_TRUE;
                    FlushBuffer(pSfOMXComponent, 0);
                    FlushBuffer(pSfOMXComponent, 1);
                }
                break;
            case COMPONENT_STATE_MAX:
                break;
            }
            break;

        case OMX_StateExecuting:
            pSFComponentEncoder = (ComponentImpl *)pSfVideoImplement->hSFComponentExecoder;
            pSFComponentFeeder = (ComponentImpl *)pSfVideoImplement->hSFComponentFeeder;
            pSFComponentRender = (ComponentImpl *)pSfVideoImplement->hSFComponentRender;


            currentState = pSfVideoImplement->functions->ComponentGetState(pSfVideoImplement->hSFComponentExecoder);
            LOG(SF_LOG_INFO, "VPU Current state = %X\r\n", currentState);
            switch(currentState)
            {
            case COMPONENT_STATE_NONE:
            case COMPONENT_STATE_TERMINATED:
                ret = OMX_ErrorIncorrectStateTransition;
                break;
            case COMPONENT_STATE_CREATED:
                break;
            case COMPONENT_STATE_PREPARED:
            case COMPONENT_STATE_EXECUTED:
                pSFComponentEncoder->pause = OMX_FALSE;
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
    LOG(SF_LOG_PERF, "buffer count = %d\r\n", GetOMXBufferCount(pSfOMXComponent));
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
    TestEncConfig *pTestEncConfig = (TestEncConfig *)pSfVideoImplement->testConfig;
    pTestEncConfig->stdMode = pSfVideoImplement->bitFormat;
    pTestEncConfig->frame_endian = VPU_FRAME_ENDIAN;
    pTestEncConfig->stream_endian = VPU_STREAM_ENDIAN;
    pTestEncConfig->source_endian = VPU_SOURCE_ENDIAN;
    pTestEncConfig->mapType = COMPRESSED_FRAME_MAP;
    pTestEncConfig->lineBufIntEn = TRUE;
    pTestEncConfig->ringBufferEnable = FALSE;
    pTestEncConfig->ringBufferWrapEnable = FALSE;
    pTestEncConfig->yuv_mode = SOURCE_YUV_WITH_BUFFER;
    /*
     if cbcrInterleave is FALSE and nv21 is FALSE, the default enc format is I420
     if cbcrInterleave is TRUE and nv21 is FALSE, then the enc format is NV12
     if cbcrInterleave is TRUE and nv21 is TRUE, then the enc format is NV21
    */
    pTestEncConfig->cbcrInterleave = TRUE;
    pTestEncConfig->nv21 = FALSE;

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

EXIT:
    FunctionOut();

    return ret;
}

static OMX_ERRORTYPE SF_OMX_ComponentClear(SF_OMX_COMPONENT *pSfOMXComponent)
{
    OMX_ERRORTYPE ret = OMX_ErrorNone;
    SF_WAVE5_IMPLEMEMT *pSfVideoImplement = (SF_WAVE5_IMPLEMEMT *)pSfOMXComponent->componentImpl;
    ComponentImpl *pSFComponentEncoder = (ComponentImpl *)pSfVideoImplement->hSFComponentExecoder;
    ComponentImpl *pSFComponentFeeder = (ComponentImpl *)pSfVideoImplement->hSFComponentFeeder;
    ComponentImpl *pSFComponentRender = (ComponentImpl *)pSfVideoImplement->hSFComponentRender;

    FunctionIn();
    if (pSFComponentEncoder == NULL || pSFComponentFeeder == NULL || pSFComponentRender == NULL)
    {
        goto EXIT;
    }
    pSFComponentEncoder->terminate = OMX_TRUE;
    pSFComponentFeeder->terminate = OMX_TRUE;
    pSFComponentRender->terminate = OMX_TRUE;
    pSfVideoImplement->functions->ComponentWait(pSfVideoImplement->hSFComponentFeeder);
    pSfVideoImplement->functions->ComponentWait(pSfVideoImplement->hSFComponentExecoder);
    pSfVideoImplement->functions->ComponentWait(pSfVideoImplement->hSFComponentRender);

    free(pSfVideoImplement->pusBitCode);
    pSfVideoImplement->functions->ClearEncListenerContext(pSfVideoImplement->lsnCtx);
    ComponentClearCommon(pSfOMXComponent);
EXIT:
    FunctionOut();

    return ret;
}

SF_OMX_COMPONENT sf_enc_encoder_h265 = {
    .componentName = "sf.enc.encoder.h265",
    .libName = "libsfenc.so",
    .pOMXComponent = NULL,
    .SF_OMX_ComponentConstructor = SF_OMX_ComponentConstructor,
    .SF_OMX_ComponentClear = SF_OMX_ComponentClear,
    // .bitFormat = STD_HEVC,
    .componentImpl = NULL,
    .fwPath = "/lib/firmware/chagall.bin",
    .componentRule = "video_encoder.hevc"};

SF_OMX_COMPONENT sf_enc_encoder_h264 = {
    .componentName = "sf.enc.encoder.h264",
    .libName = "libsfenc.so",
    .pOMXComponent = NULL,
    .SF_OMX_ComponentConstructor = SF_OMX_ComponentConstructor,
    .SF_OMX_ComponentClear = SF_OMX_ComponentClear,
    // .bitFormat = STD_AVC,
    .componentImpl = NULL,
    .fwPath = "/lib/firmware/chagall.bin",
    .componentRule = "video_encoder.avc"};