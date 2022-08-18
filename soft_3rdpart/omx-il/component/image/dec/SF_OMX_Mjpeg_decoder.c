// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2021 StarFive Technology Co., Ltd.
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "SF_OMX_mjpeg_common.h"
#include "SF_OMX_Core.h"
#include <sys/prctl.h>

extern OMX_TICKS gInitTimeStamp;
#define TEMP_DEBUG 1
#define NUM_OF_PORTS 2

#define DEC_StopThread OMX_CommandMax

typedef struct DEC_CMD
{
    OMX_COMMANDTYPE Cmd;
    OMX_U32 nParam;
    OMX_PTR pCmdData;
}DEC_CMD;

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
    SF_OMX_COMPONENT *pSfOMXComponent = (SF_OMX_COMPONENT *)pOMXComponent->pComponentPrivate;
    SF_CODAJ12_IMPLEMEMT *pSfCodaj12Implement = (SF_CODAJ12_IMPLEMEMT *)pSfOMXComponent->componentImpl;

    LOG(SF_LOG_DEBUG, "nFilledLen = %d, nFlags = %d, header = %p, pBuffer = %p\r\n", pBuffer->nFilledLen, pBuffer->nFlags, pBuffer, pBuffer->pBuffer);

    if (!pSfOMXComponent->portDefinition[OMX_INPUT_PORT_INDEX].bEnabled)
    {
        LOG(SF_LOG_INFO, "feed buffer when input port stop\r\n");
        return OMX_ErrorIncorrectStateOperation;
    }

    if (pBuffer->nInputPortIndex != OMX_INPUT_PORT_INDEX)
    {
        LOG(SF_LOG_INFO, "Incorrect nInputPortIndex %d\r\n", pBuffer->nInputPortIndex);
        return OMX_ErrorBadPortIndex;
    }

    Message data;
    data.msg_type = 1;
    data.msg_flag = 0;
    data.pBuffer = pBuffer;
    pSfOMXComponent->handlingBufferNum[OMX_INPUT_PORT_INDEX]++;
    if (msgsnd(pSfCodaj12Implement->sInputMessageQueue, (void *)&data, sizeof(data) - sizeof(data.msg_type), 0) == -1)
    {
        LOG(SF_LOG_ERR, "msgsnd failed\n");
    }
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
    SF_CODAJ12_IMPLEMEMT *pSfCodaj12Implement = (SF_CODAJ12_IMPLEMEMT *)pSfOMXComponent->componentImpl;
    LOG(SF_LOG_DEBUG, "nFilledLen = %d, nFlags = %d, header = %p, pBuffer = %p\r\n", pBuffer->nFilledLen, pBuffer->nFlags, pBuffer, pBuffer->pBuffer);

    if (!pSfOMXComponent->portDefinition[OMX_OUTPUT_PORT_INDEX].bEnabled)
    {
        LOG(SF_LOG_INFO, "feed buffer when output port stop\r\n");
        return OMX_ErrorIncorrectStateOperation;
    }

    if (pBuffer->nOutputPortIndex != OMX_OUTPUT_PORT_INDEX)
    {
        LOG(SF_LOG_INFO, "Incorrect nOutputPortIndex %d\r\n", pBuffer->nOutputPortIndex);
        return OMX_ErrorBadPortIndex;
    }

    Message data;
    data.msg_type = 1;
    data.msg_flag = 0;
    data.pBuffer = pBuffer;
    pSfOMXComponent->handlingBufferNum[OMX_OUTPUT_PORT_INDEX]++;

    LOG(SF_LOG_DEBUG, "Send to message queue\r\n");
    if (msgsnd(pSfCodaj12Implement->sOutputMessageQueue, (void *)&data, sizeof(data) - sizeof(data.msg_type), 0) == -1)
    {
        LOG(SF_LOG_ERR, "msgsnd failed\n");
    }
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
    OMX_BUFFERHEADERTYPE *temp_bufferHeader;
    SF_OMX_BUF_INFO *pBufInfo;
    OMX_COMPONENTTYPE *pOMXComponent = (OMX_COMPONENTTYPE *)hComponent;
    SF_OMX_COMPONENT *pSfOMXComponent = pOMXComponent->pComponentPrivate;
    OMX_U32 i;

    FunctionIn();

    temp_bufferHeader = (OMX_BUFFERHEADERTYPE *)malloc(sizeof(OMX_BUFFERHEADERTYPE));
    if (temp_bufferHeader == NULL)
    {
        LOG(SF_LOG_ERR, "malloc fail\r\n");
        return OMX_ErrorInsufficientResources;
    }
    pBufInfo = (SF_OMX_BUF_INFO *)malloc(sizeof(SF_OMX_BUF_INFO));
    if (pBufInfo == NULL)
    {
        LOG(SF_LOG_ERR, "malloc fail\r\n");
        free(temp_bufferHeader);
        return OMX_ErrorInsufficientResources;
    }
    memset(temp_bufferHeader, 0, sizeof(OMX_BUFFERHEADERTYPE));
    temp_bufferHeader->nAllocLen = nSizeBytes;
    temp_bufferHeader->pAppPrivate = pAppPrivate;
    temp_bufferHeader->pBuffer = pBuffer;
    temp_bufferHeader->nSize = sizeof(OMX_BUFFERHEADERTYPE);
    memcpy(&(temp_bufferHeader->nVersion), &(pSfOMXComponent->componentVersion), sizeof(OMX_VERSIONTYPE));

    if(nPortIndex == OMX_INPUT_PORT_INDEX)
    {
        for (i = 0; i < MAX_BUFF_NUM; i++)
        {
            if (!pSfOMXComponent->pBufferArray[OMX_INPUT_PORT_INDEX][i])
            {
                pBufInfo->type = SF_BUFFER_NOMAL_EXTERNAL;
                pBufInfo->index = i;
                temp_bufferHeader->pInputPortPrivate = (OMX_PTR)pBufInfo;
                temp_bufferHeader->nInputPortIndex = OMX_INPUT_PORT_INDEX;
                pSfOMXComponent->pBufferArray[OMX_INPUT_PORT_INDEX][i] = temp_bufferHeader;
                pSfOMXComponent->assignedBufferNum[OMX_INPUT_PORT_INDEX] ++;

                if (pSfOMXComponent->assignedBufferNum[OMX_INPUT_PORT_INDEX] ==
                        pSfOMXComponent->portDefinition[OMX_INPUT_PORT_INDEX].nBufferCountActual)
                {
                    pSfOMXComponent->portDefinition[OMX_INPUT_PORT_INDEX].bPopulated = OMX_TRUE;
                    SF_SemaphorePost(pSfOMXComponent->portSemaphore[OMX_INPUT_PORT_INDEX]);
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
    else if(nPortIndex == OMX_OUTPUT_PORT_INDEX)
    {
        for (i = 0; i < MAX_BUFF_NUM; i++)
        {
            if (!pSfOMXComponent->pBufferArray[OMX_OUTPUT_PORT_INDEX][i])
            {
                pBufInfo->type = SF_BUFFER_NOMAL_EXTERNAL;
                pBufInfo->index = i;
                temp_bufferHeader->pOutputPortPrivate = (OMX_PTR)pBufInfo;
                temp_bufferHeader->nOutputPortIndex = OMX_OUTPUT_PORT_INDEX;
                pSfOMXComponent->pBufferArray[OMX_OUTPUT_PORT_INDEX][i] = temp_bufferHeader;
                pSfOMXComponent->assignedBufferNum[OMX_OUTPUT_PORT_INDEX] ++;

                if (pSfOMXComponent->assignedBufferNum[OMX_OUTPUT_PORT_INDEX] ==
                        pSfOMXComponent->portDefinition[OMX_OUTPUT_PORT_INDEX].nBufferCountActual)
                {
                    pSfOMXComponent->portDefinition[OMX_OUTPUT_PORT_INDEX].bPopulated = OMX_TRUE;
                    SF_SemaphorePost(pSfOMXComponent->portSemaphore[OMX_OUTPUT_PORT_INDEX]);
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

    *ppBufferHdr = temp_bufferHeader;

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
    OMX_COMPONENTTYPE *pOMXComponent = (OMX_COMPONENTTYPE *)hComponent;
    SF_OMX_COMPONENT *pSfOMXComponent = pOMXComponent->pComponentPrivate;
    OMX_ERRORTYPE ret = OMX_ErrorNone;
    OMX_BUFFERHEADERTYPE *temp_bufferHeader;
    SF_OMX_BUF_INFO *pBufInfo;
    OMX_U32 i;

    FunctionIn();
    if (nSizeBytes == 0)
    {
        LOG(SF_LOG_ERR, "nSizeBytes = %d\r\n", nSizeBytes);
        return OMX_ErrorBadParameter;
    }
    temp_bufferHeader = (OMX_BUFFERHEADERTYPE *)malloc(sizeof(OMX_BUFFERHEADERTYPE));
    if (temp_bufferHeader == NULL)
    {
        LOG(SF_LOG_ERR, "malloc fail\r\n");
        return OMX_ErrorInsufficientResources;
    }
    memset(temp_bufferHeader, 0, sizeof(OMX_BUFFERHEADERTYPE));
    pBufInfo = (SF_OMX_BUF_INFO *)malloc(sizeof(SF_OMX_BUF_INFO));
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

    if (nPortIndex == OMX_OUTPUT_PORT_INDEX)
    {
        for (i = 0; i < MAX_BUFF_NUM; i++)
        {
            if (!pSfOMXComponent->pBufferArray[OMX_OUTPUT_PORT_INDEX][i])
            {
                temp_bufferHeader->pBuffer = malloc(nSizeBytes);
                memset(temp_bufferHeader->pBuffer, 0, nSizeBytes);
                temp_bufferHeader->nOutputPortIndex = OMX_OUTPUT_PORT_INDEX;
                pBufInfo->type = SF_BUFFER_NOMAL;
                pBufInfo->index = i;
                temp_bufferHeader->pOutputPortPrivate = (void*)pBufInfo;
                LOG(SF_LOG_PERF, "alloc normal output buffer %d \r\n", i);

                pSfOMXComponent->pBufferArray[OMX_OUTPUT_PORT_INDEX][i] = temp_bufferHeader;
                pSfOMXComponent->assignedBufferNum[OMX_OUTPUT_PORT_INDEX]++;

                if (pSfOMXComponent->assignedBufferNum[OMX_OUTPUT_PORT_INDEX] ==
                        pSfOMXComponent->portDefinition[OMX_OUTPUT_PORT_INDEX].nBufferCountActual)
                {
                    pSfOMXComponent->portDefinition[OMX_OUTPUT_PORT_INDEX].bPopulated = OMX_TRUE;
                    SF_SemaphorePost(pSfOMXComponent->portSemaphore[OMX_OUTPUT_PORT_INDEX]);
                }
                break;
            }
        }
        if (i == MAX_BUFF_NUM){
            LOG(SF_LOG_ERR, "buffer array full\r\n");
            temp_bufferHeader->pBuffer = NULL;
        }
    }
    else if (nPortIndex == OMX_INPUT_PORT_INDEX)
    {
        for (i = 0; i < MAX_BUFF_NUM; i++)
        {
            if (!pSfOMXComponent->pBufferArray[OMX_INPUT_PORT_INDEX][i])
            {
                temp_bufferHeader->pBuffer = malloc(nSizeBytes);
                memset(temp_bufferHeader->pBuffer, 0, nSizeBytes);
                temp_bufferHeader->nInputPortIndex = OMX_INPUT_PORT_INDEX;
                pBufInfo->type = SF_BUFFER_NOMAL;
                pBufInfo->index = i;
                temp_bufferHeader->pInputPortPrivate = (void*)pBufInfo;
                LOG(SF_LOG_PERF, "alloc normal intput buffer %d \r\n", i);

                pSfOMXComponent->pBufferArray[OMX_INPUT_PORT_INDEX][i] = temp_bufferHeader;
                pSfOMXComponent->assignedBufferNum[OMX_INPUT_PORT_INDEX]++;

                if (pSfOMXComponent->assignedBufferNum[OMX_INPUT_PORT_INDEX] ==
                        pSfOMXComponent->portDefinition[OMX_INPUT_PORT_INDEX].nBufferCountActual)
                {
                    pSfOMXComponent->portDefinition[OMX_INPUT_PORT_INDEX].bPopulated = OMX_TRUE;
                    SF_SemaphorePost(pSfOMXComponent->portSemaphore[OMX_INPUT_PORT_INDEX]);
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
        free(pBufInfo);
        free(temp_bufferHeader);
        LOG(SF_LOG_ERR, "malloc fail\r\n");
        return OMX_ErrorInsufficientResources;
    }

    *ppBuffer = temp_bufferHeader;
    LOG(SF_LOG_INFO, "nPortIndex = %d, buffheader %p Buffer address = %p, nFilledLen = %d, nSizeBytes = %d\r\n",
            nPortIndex, temp_bufferHeader, temp_bufferHeader->pBuffer, temp_bufferHeader->nFilledLen, nSizeBytes);
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
    OMX_PARAM_PORTDEFINITIONTYPE *pOutputPort = &pSfOMXComponent->portDefinition[OMX_OUTPUT_PORT_INDEX];

    FunctionIn();

    if (hComponent == NULL || ComponentParameterStructure == NULL)
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
        portParam->nPorts           = OMX_PORT_MAX;
        portParam->nStartPortNumber = OMX_INPUT_PORT_INDEX;
        break;
    }

    case OMX_IndexParamAudioInit:
    case OMX_IndexParamImageInit:
    case OMX_IndexParamOtherInit:
    {
        OMX_PORT_PARAM_TYPE *portParam = (OMX_PORT_PARAM_TYPE *)ComponentParameterStructure;
        portParam->nPorts           = 0;
        portParam->nStartPortNumber = 0;
        break;
    }

    case OMX_IndexParamVideoPortFormat:
    {
        OMX_VIDEO_PARAM_PORTFORMATTYPE *portFormat = (OMX_VIDEO_PARAM_PORTFORMATTYPE *)ComponentParameterStructure;
        OMX_U32 index = portFormat->nIndex;
        LOG(SF_LOG_INFO, "Get video port format at index %d\r\n", index);
        portFormat->eCompressionFormat = OMX_VIDEO_CodingUnused;
        portFormat->xFramerate = 30;
        switch (index)
        {
        case 0:
            portFormat->eColorFormat = OMX_COLOR_FormatYUV420Planar;
            break;
        case 1:
            portFormat->eColorFormat = OMX_COLOR_FormatYUV422Planar;
            break;
        case 2:
            portFormat->eColorFormat = OMX_COLOR_FormatYUV444Interleaved;
            break;
        case 3:
            portFormat->eColorFormat = OMX_COLOR_FormatYUV420SemiPlanar; //NV12
            break;
        case 4:
            portFormat->eColorFormat = OMX_COLOR_FormatYVU420SemiPlanar; //NV21
            break;
        default:
            if (index > 0)
            {
                ret = OMX_ErrorNoMore;
            }
        }
        break;
    }

    case OMX_IndexParamVideoBitrate:
    {
        OMX_VIDEO_PARAM_BITRATETYPE     *videoRateControl = (OMX_VIDEO_PARAM_BITRATETYPE *)ComponentParameterStructure;
        OMX_U32                          portIndex = videoRateControl->nPortIndex;
        if ((portIndex != OMX_OUTPUT_PORT_INDEX)) {
            ret = OMX_ErrorBadPortIndex;
            goto EXIT;
        }
        else
        {
            videoRateControl->nTargetBitrate = pOutputPort->format.video.nBitrate;
        }
        LOG(SF_LOG_INFO, "Get nTargetBitrate = %u on port %d\r\n",videoRateControl->nTargetBitrate, videoRateControl->nPortIndex);
        break;
    }

    case OMX_IndexParamVideoQuantization:
        break;

    case OMX_IndexParamPortDefinition:
    {
        OMX_PARAM_PORTDEFINITIONTYPE *pPortDefinition = (OMX_PARAM_PORTDEFINITIONTYPE *)ComponentParameterStructure;
        OMX_U32 portIndex = pPortDefinition->nPortIndex;
        OMX_PARAM_PORTDEFINITIONTYPE *pSrcDefinition;

        if (portIndex < OMX_PORT_MAX)
        {
            pSrcDefinition = &pSfOMXComponent->portDefinition[portIndex];
            LOG(SF_LOG_DEBUG, "Get parameter port %X\r\n",portIndex);
            LOG_APPEND(SF_LOG_DEBUG, "width = %d, height = %d\r\n", pSrcDefinition->format.video.nFrameWidth, pSrcDefinition->format.video.nFrameHeight);
            LOG_APPEND(SF_LOG_DEBUG, "eColorFormat = %d\r\n", pSrcDefinition->format.video.eColorFormat);
            LOG_APPEND(SF_LOG_DEBUG, "xFramerate = %d\r\n", pSrcDefinition->format.video.xFramerate);
            LOG_APPEND(SF_LOG_DEBUG, "bufferSize = %d\r\n",pSrcDefinition->nBufferSize);
            LOG_APPEND(SF_LOG_DEBUG, "Buffer count = %d\r\n", pSrcDefinition->nBufferCountActual);

            memcpy(pPortDefinition, pSrcDefinition, pPortDefinition->nSize);
        }
        else
        {
            LOG(SF_LOG_INFO,"Bad port index %d\r\n",portIndex);
            ret = OMX_ErrorBadPortIndex;
        }
        break;
    }

    case OMX_IndexParamVideoIntraRefresh:
        break;

    case OMX_IndexParamStandardComponentRole:
    {
        OMX_PARAM_COMPONENTROLETYPE *pComponentRole = (OMX_PARAM_COMPONENTROLETYPE *)ComponentParameterStructure;
        strcpy((OMX_STRING)(pComponentRole->cRole), pSfOMXComponent->componentRule);
        break;
    }

    case OMX_IndexParamCompBufferSupplier:
    {
        OMX_PARAM_BUFFERSUPPLIERTYPE *bufferSupplier = (OMX_PARAM_BUFFERSUPPLIERTYPE *)ComponentParameterStructure;
        OMX_U32 portIndex = bufferSupplier->nPortIndex;
        if (portIndex >= OMX_PORT_MAX)
            ret = OMX_ErrorBadPortIndex;
        break;
    }

    case OMX_IndexParamImagePortFormat:
    {
        OMX_IMAGE_PARAM_PORTFORMATTYPE *portFormat = (OMX_IMAGE_PARAM_PORTFORMATTYPE *)ComponentParameterStructure;
        OMX_U32 index = portFormat->nIndex;

        if (index < OMX_PORT_MAX)
        {
            portFormat->eCompressionFormat = OMX_VIDEO_CodingUnused;
            portFormat->eColorFormat = OMX_COLOR_FormatYUV420SemiPlanar;
        }
        else
        {
            LOG(SF_LOG_INFO,"Bad port index %d\r\n",index);
            ret = OMX_ErrorBadPortIndex;
        }
        break;
    }

    default:
    {
        ret = OMX_ErrorUnsupportedIndex;
        break;
    }

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

    if (ComponentParameterStructure == NULL)
    {
        ret = OMX_ErrorBadParameter;
        goto EXIT;
    }
    LOG(SF_LOG_INFO, "Set parameter on index %X\r\n", nIndex);
    OMX_COMPONENTTYPE *pOMXComponent = (OMX_COMPONENTTYPE *)hComponent;
    SF_OMX_COMPONENT *pSfOMXComponent = pOMXComponent->pComponentPrivate;
    SF_CODAJ12_IMPLEMEMT *pSfMjpegImplement = (SF_CODAJ12_IMPLEMEMT *)pSfOMXComponent->componentImpl;

    if (pSfOMXComponent == NULL)
    {
        ret = OMX_ErrorBadParameter;
        goto EXIT;
    }

    switch ((OMX_U32)nIndex)
    {
    case OMX_IndexParamPortDefinition:
    {
        OMX_PARAM_PORTDEFINITIONTYPE *pPortDefinition =
            (OMX_PARAM_PORTDEFINITIONTYPE *)ComponentParameterStructure;
        OMX_PARAM_PORTDEFINITIONTYPE *pInputPort = &pSfOMXComponent->portDefinition[0];
        OMX_PARAM_PORTDEFINITIONTYPE *pOutputPort = &pSfOMXComponent->portDefinition[1];
        DecConfigParam *decConfig = pSfMjpegImplement->config;

        OMX_U32 portIndex = pPortDefinition->nPortIndex;
        OMX_U32 width = pPortDefinition->format.video.nFrameWidth;
        OMX_U32 height = pPortDefinition->format.video.nFrameHeight;
        LOG(SF_LOG_INFO, "Set parameter on port %d\r\n", pPortDefinition->nPortIndex);
        LOG_APPEND(SF_LOG_DEBUG, "width = %d, height = %d\r\n",width, height);
        LOG_APPEND(SF_LOG_DEBUG, "eColorFormat = %d\r\n", pPortDefinition->format.video.eColorFormat);
        LOG_APPEND(SF_LOG_DEBUG, "xFramerate = %d\r\n", pPortDefinition->format.video.xFramerate);
        LOG_APPEND(SF_LOG_DEBUG, "bufferSize = %d\r\n",pPortDefinition->nBufferSize);
        LOG_APPEND(SF_LOG_DEBUG, "Buffer count = %d\r\n", pPortDefinition->nBufferCountActual);
        if (portIndex == (OMX_U32)OMX_INPUT_PORT_INDEX)
        {
            memcpy(&pSfOMXComponent->portDefinition[portIndex], pPortDefinition, pPortDefinition->nSize);
            pInputPort->format.video.nStride = width;
            pInputPort->format.video.nSliceHeight = height;
            pInputPort->nBufferSize = width * height * 2;
        }
        else if (portIndex == (OMX_U32)(OMX_OUTPUT_PORT_INDEX))
        {
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
                                     "OutPut[%d, %d]. Input[%d, %d]\r\n",
                        width, height, nInputWidth, nInputHeight);
                    width = nInputWidth;
                    height = nInputHeight;
                }

                pOutputPort->format.video.nFrameWidth = width;
                pOutputPort->format.video.nFrameHeight = height;
                pOutputPort->format.video.nStride = width;
                pOutputPort->format.video.nSliceHeight = height;
                switch (pOutputPort->format.video.eColorFormat)
                {
                    case OMX_COLOR_FormatYUV420Planar:
                        decConfig->cbcrInterleave = CBCR_SEPARATED;
                        decConfig->packedFormat = PACKED_FORMAT_NONE;
                        pSfMjpegImplement->frameFormat = FORMAT_420;
                        pOutputPort->nBufferSize = (width * height * 3) / 2;
                        break;
                    case OMX_COLOR_FormatYUV420SemiPlanar: //NV12
                        decConfig->cbcrInterleave = CBCR_INTERLEAVE;
                        decConfig->packedFormat = PACKED_FORMAT_NONE;
                        pSfMjpegImplement->frameFormat = FORMAT_420;
                        pOutputPort->nBufferSize = (width * height * 3) / 2;
                        break;
                    case OMX_COLOR_FormatYVU420SemiPlanar: //NV21
                        decConfig->cbcrInterleave = CRCB_INTERLEAVE;
                        decConfig->packedFormat = PACKED_FORMAT_NONE;
                        pSfMjpegImplement->frameFormat = FORMAT_420;
                        pOutputPort->nBufferSize = (width * height * 3) / 2;
                        break;
                    case OMX_COLOR_FormatYUV422Planar:
                        decConfig->cbcrInterleave = CBCR_SEPARATED;
                        decConfig->packedFormat = PACKED_FORMAT_NONE;
                        pSfMjpegImplement->frameFormat = FORMAT_422;
                        pOutputPort->nBufferSize = width * height * 2;
                        break;
                    case OMX_COLOR_FormatYUV422SemiPlanar: //NV16
                        decConfig->cbcrInterleave = CBCR_INTERLEAVE;
                        decConfig->packedFormat = PACKED_FORMAT_NONE;
                        pSfMjpegImplement->frameFormat = FORMAT_422;
                        pOutputPort->nBufferSize = width * height * 2;
                        break;
                    case OMX_COLOR_FormatYVU422SemiPlanar: //NV61
                        decConfig->cbcrInterleave = CRCB_INTERLEAVE;
                        decConfig->packedFormat = PACKED_FORMAT_NONE;
                        pSfMjpegImplement->frameFormat = FORMAT_422;
                        pOutputPort->nBufferSize = width * height * 2;
                        break;
                    case OMX_COLOR_FormatYCbYCr: //YUYV
                        decConfig->cbcrInterleave = CBCR_SEPARATED;
                        decConfig->packedFormat   = PACKED_FORMAT_422_YUYV;
                        pSfMjpegImplement->frameFormat = FORMAT_422;
                        pOutputPort->nBufferSize = width * height * 2;
                        break;
                    case OMX_COLOR_FormatYCrYCb: //YVYU
                        decConfig->cbcrInterleave = CBCR_SEPARATED;
                        decConfig->packedFormat   = PACKED_FORMAT_422_YVYU;
                        pSfMjpegImplement->frameFormat = FORMAT_422;
                        pOutputPort->nBufferSize = width * height * 2;
                        break;
                    case OMX_COLOR_FormatCbYCrY: //UYVY
                        decConfig->cbcrInterleave = CBCR_SEPARATED;
                        decConfig->packedFormat   = PACKED_FORMAT_422_UYVY;
                        pSfMjpegImplement->frameFormat = FORMAT_422;
                        pOutputPort->nBufferSize = width * height * 2;
                        break;
                    case OMX_COLOR_FormatCrYCbY: //VYUY
                        decConfig->cbcrInterleave = CBCR_SEPARATED;
                        decConfig->packedFormat   = PACKED_FORMAT_422_VYUY;
                        pSfMjpegImplement->frameFormat = FORMAT_422;
                        pOutputPort->nBufferSize = width * height * 2;
                        break;
                    case OMX_COLOR_FormatYUV444Planar: //I444
                        decConfig->cbcrInterleave = CBCR_SEPARATED;
                        decConfig->packedFormat   = PACKED_FORMAT_NONE;
                        pSfMjpegImplement->frameFormat = FORMAT_444;
                        pOutputPort->nBufferSize = width * height * 3;
                        break;
                    case OMX_COLOR_FormatYUV444Interleaved: //YUV444PACKED
                        decConfig->cbcrInterleave = CBCR_SEPARATED;
                        decConfig->packedFormat   = PACKED_FORMAT_444;
                        pSfMjpegImplement->frameFormat = FORMAT_444;
                        pOutputPort->nBufferSize = width * height * 3;
                        break;
                    default:
                        pOutputPort->nBufferSize = width * height * 4;
                        break;
                }
            }
        }
        else
        {
            LOG(SF_LOG_INFO,"Bad port index %d\r\n",portIndex);
            ret = OMX_ErrorBadPortIndex;
        }
        break;
    }
    case OMX_IndexParamVideoPortFormat:
    {
        OMX_VIDEO_PARAM_PORTFORMATTYPE *portFormat = (OMX_VIDEO_PARAM_PORTFORMATTYPE *)ComponentParameterStructure;
        OMX_U32 nPortIndex = portFormat->nPortIndex;
        OMX_PARAM_PORTDEFINITIONTYPE *pPort = &pSfOMXComponent->portDefinition[nPortIndex];
        DecConfigParam *decConfig = pSfMjpegImplement->config;
        LOG(SF_LOG_DEBUG, "Set video format to port %d color %d\r\n", portFormat->nPortIndex, portFormat->eColorFormat);

        if (nPortIndex >= OMX_PORT_MAX)
            return OMX_ErrorBadPortIndex;

        switch (portFormat->eColorFormat)
        {
        case OMX_COLOR_FormatYUV420Planar:
            decConfig->cbcrInterleave = CBCR_SEPARATED;
            decConfig->packedFormat = PACKED_FORMAT_NONE;
            pSfMjpegImplement->frameFormat = FORMAT_420;
            break;
        case OMX_COLOR_FormatYUV420SemiPlanar: //NV12
            decConfig->cbcrInterleave = CBCR_INTERLEAVE;
            decConfig->packedFormat = PACKED_FORMAT_NONE;
            pSfMjpegImplement->frameFormat = FORMAT_420;
            break;
        case OMX_COLOR_FormatYVU420SemiPlanar: //NV21
            decConfig->cbcrInterleave = CRCB_INTERLEAVE;
            decConfig->packedFormat = PACKED_FORMAT_NONE;
            pSfMjpegImplement->frameFormat = FORMAT_420;
            break;
        case OMX_COLOR_FormatYUV422Planar:
            decConfig->cbcrInterleave = CBCR_SEPARATED;
            decConfig->packedFormat = PACKED_FORMAT_NONE;
            pSfMjpegImplement->frameFormat = FORMAT_422;
            break;
        case OMX_COLOR_FormatYUV422SemiPlanar: //NV16
            decConfig->cbcrInterleave = CBCR_INTERLEAVE;
            decConfig->packedFormat = PACKED_FORMAT_NONE;
            pSfMjpegImplement->frameFormat = FORMAT_422;
            break;
        case OMX_COLOR_FormatYVU422SemiPlanar: //NV61
            decConfig->cbcrInterleave = CRCB_INTERLEAVE;
            decConfig->packedFormat = PACKED_FORMAT_NONE;
            pSfMjpegImplement->frameFormat = FORMAT_422;
            break;
        case OMX_COLOR_FormatYCbYCr: //YUYV
            decConfig->cbcrInterleave = CBCR_SEPARATED;
            decConfig->packedFormat   = PACKED_FORMAT_422_YUYV;
            pSfMjpegImplement->frameFormat = FORMAT_422;
            break;
        case OMX_COLOR_FormatYCrYCb: //YVYU
            decConfig->cbcrInterleave = CBCR_SEPARATED;
            decConfig->packedFormat   = PACKED_FORMAT_422_YVYU;
            pSfMjpegImplement->frameFormat = FORMAT_422;
            break;
        case OMX_COLOR_FormatCbYCrY: //UYVY
            decConfig->cbcrInterleave = CBCR_SEPARATED;
            decConfig->packedFormat   = PACKED_FORMAT_422_UYVY;
            pSfMjpegImplement->frameFormat = FORMAT_422;
            break;
        case OMX_COLOR_FormatCrYCbY: //VYUY
            decConfig->cbcrInterleave = CBCR_SEPARATED;
            decConfig->packedFormat   = PACKED_FORMAT_422_VYUY;
            pSfMjpegImplement->frameFormat = FORMAT_422;
            break;
        case OMX_COLOR_FormatYUV444Planar: //I444
            decConfig->cbcrInterleave = CBCR_SEPARATED;
            decConfig->packedFormat   = PACKED_FORMAT_NONE;
            pSfMjpegImplement->frameFormat = FORMAT_444;
            break;
        case OMX_COLOR_FormatYUV444Interleaved: //YUV444PACKED
            decConfig->cbcrInterleave = CBCR_SEPARATED;
            decConfig->packedFormat   = PACKED_FORMAT_444;
            pSfMjpegImplement->frameFormat = FORMAT_444;
            pPort->format.video.eColorFormat = portFormat->eColorFormat;
            break;
        default:
            ret = OMX_ErrorNotImplemented;
            break;
        }
        if(!ret)
            pPort->format.video.eColorFormat = portFormat->eColorFormat;
        break;
    }
    case OMX_IndexParamVideoInit:
    {
        OMX_PORT_PARAM_TYPE *portParam = (OMX_PORT_PARAM_TYPE *)ComponentParameterStructure;
        portParam->nPorts           = 2;
        portParam->nStartPortNumber = 0;
        break;
    }

    case OMX_IndexParamImagePortFormat:
        break;

    case OMX_IndexParamStandardComponentRole:
    {
        if ((pSfOMXComponent->state != OMX_StateLoaded) &&
                (pSfOMXComponent->state != OMX_StateWaitForResources)) {
            ret = OMX_ErrorIncorrectStateOperation;
        }
        break;
    }

    case OMX_IndexParamCompBufferSupplier:
    {
        OMX_PARAM_BUFFERSUPPLIERTYPE *bufferSupplier = (OMX_PARAM_BUFFERSUPPLIERTYPE *)ComponentParameterStructure;
        OMX_U32 portIndex = bufferSupplier->nPortIndex;
        if (portIndex >= OMX_PORT_MAX)
            ret = OMX_ErrorBadPortIndex;
        break;
    }

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

    if (pComponentConfigStructure == NULL)
    {
        ret = OMX_ErrorBadParameter;
        goto EXIT;
    }

    LOG(SF_LOG_INFO, "Get config on index %X\r\n", nIndex);
    OMX_COMPONENTTYPE *pOMXComponent = (OMX_COMPONENTTYPE *)hComponent;
    SF_OMX_COMPONENT *pSfOMXComponent = pOMXComponent->pComponentPrivate;
    SF_CODAJ12_IMPLEMEMT *pSfMjpegImplement = (SF_CODAJ12_IMPLEMEMT *)pSfOMXComponent->componentImpl;

    switch ((OMX_U32)nIndex)
    {
    case OMX_IndexConfigCommonOutputCrop:
    {
        OMX_CONFIG_RECTTYPE *rectParam = (OMX_CONFIG_RECTTYPE *)pComponentConfigStructure;
        DecConfigParam *decConfig = pSfMjpegImplement->config;
        if(rectParam->nPortIndex == (OMX_U32)(OMX_OUTPUT_PORT_INDEX))
        {
            rectParam->nLeft = decConfig->roiOffsetX;
            rectParam->nTop = decConfig->roiOffsetY;
            rectParam->nWidth = decConfig->roiWidth;
            rectParam->nHeight = decConfig->roiHeight;
            LOG(SF_LOG_INFO, "Get OutputCrop left %d top %d width %d height %d \r\n",
                decConfig->roiOffsetX, decConfig->roiOffsetY, decConfig->roiWidth, decConfig->roiHeight);
        }
        else
        {
            LOG(SF_LOG_WARN, "Only output port support OutputCrop param\r\n");
            ret = OMX_ErrorBadParameter;
        }
        break;
    }

    case OMX_IndexConfigCommonRotate:
    {
        OMX_CONFIG_ROTATIONTYPE *rotatParam = (OMX_CONFIG_ROTATIONTYPE *)pComponentConfigStructure;
        DecConfigParam *decConfig = pSfMjpegImplement->config;
        if(rotatParam->nPortIndex == (OMX_U32)(OMX_OUTPUT_PORT_INDEX))
        {
            rotatParam->nRotation = decConfig->rotation;
            LOG(SF_LOG_INFO, "Get Rotation %d \r\n", decConfig->rotation);
        }
        else
        {
            LOG(SF_LOG_WARN, "Only output port support Rotation config\r\n");
            ret = OMX_ErrorBadParameter;
        }
        break;
    }

    case OMX_IndexConfigCommonMirror:
    {
        OMX_CONFIG_MIRRORTYPE *mirrParam = (OMX_CONFIG_MIRRORTYPE *)pComponentConfigStructure;
        DecConfigParam *decConfig = pSfMjpegImplement->config;
        if(mirrParam->nPortIndex == (OMX_U32)(OMX_OUTPUT_PORT_INDEX))
        {
            mirrParam->eMirror = decConfig->mirror;
            LOG(SF_LOG_INFO, "Get Mirror %d \r\n", decConfig->mirror);
        }
        else
        {
            LOG(SF_LOG_WARN, "Only output port support Mirror config\r\n");
            ret = OMX_ErrorBadParameter;
        }
        break;
    }

    case OMX_IndexConfigCommonScale:
    {
        OMX_CONFIG_SCALEFACTORTYPE *scaleParam = (OMX_CONFIG_SCALEFACTORTYPE *)pComponentConfigStructure;
        DecConfigParam *decConfig = pSfMjpegImplement->config;
        if(scaleParam->nPortIndex == (OMX_U32)(OMX_OUTPUT_PORT_INDEX))
        {
            /* In Q16 format */
            scaleParam->xWidth = (1 << 16) >> decConfig->iHorScaleMode;
            scaleParam->xHeight = (1 << 16) >> decConfig->iVerScaleMode;
            LOG(SF_LOG_INFO, "Get scale  %d(Q16),%d(Q16) \r\n", scaleParam->xWidth, scaleParam->xHeight);
        }
        else
        {
            LOG(SF_LOG_WARN, "Only output port support Scale config\r\n");
            ret = OMX_ErrorBadParameter;
        }
        break;
    }

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

    if (pComponentConfigStructure == NULL)
    {
        ret = OMX_ErrorBadParameter;
        goto EXIT;
    }

    LOG(SF_LOG_INFO, "Set config on index %X\r\n", nIndex);
    OMX_COMPONENTTYPE *pOMXComponent = (OMX_COMPONENTTYPE *)hComponent;
    SF_OMX_COMPONENT *pSfOMXComponent = pOMXComponent->pComponentPrivate;
    SF_CODAJ12_IMPLEMEMT *pSfMjpegImplement = (SF_CODAJ12_IMPLEMEMT *)pSfOMXComponent->componentImpl;

    switch ((OMX_U32)nIndex)
    {
    case OMX_IndexConfigCommonOutputCrop:
    {
        OMX_CONFIG_RECTTYPE *rectParam = (OMX_CONFIG_RECTTYPE *)pComponentConfigStructure;
        DecConfigParam *decConfig = pSfMjpegImplement->config;
        if(rectParam->nPortIndex == (OMX_U32)(OMX_OUTPUT_PORT_INDEX))
        {
            decConfig->roiOffsetX = rectParam->nLeft;
            decConfig->roiOffsetY = rectParam->nTop;
            decConfig->roiWidth = rectParam->nWidth;
            decConfig->roiHeight = rectParam->nHeight;
            decConfig->roiEnable = OMX_TRUE;
            LOG(SF_LOG_INFO, "Set OutputCrop left %d top %d width %d height %d \r\n",
                decConfig->roiOffsetX, decConfig->roiOffsetY, decConfig->roiWidth, decConfig->roiHeight);
        }
        else
        {
            LOG(SF_LOG_WARN, "Only support set OutputCrop config to output port\r\n");
            ret = OMX_ErrorBadParameter;
        }
        break;
    }

    case OMX_IndexConfigCommonRotate:
    {
        OMX_CONFIG_ROTATIONTYPE *rotatParam = (OMX_CONFIG_ROTATIONTYPE *)pComponentConfigStructure;
        DecConfigParam *decConfig = pSfMjpegImplement->config;
        if(rotatParam->nPortIndex == (OMX_U32)(OMX_OUTPUT_PORT_INDEX))
        {
            decConfig->rotation = rotatParam->nRotation;
            LOG(SF_LOG_INFO, "Set Rotation %d to output port\r\n", decConfig->rotation);
        }
        else
        {
            LOG(SF_LOG_WARN, "Only support set Rotation config to output port\r\n");
            ret = OMX_ErrorBadParameter;
        }
        break;
    }

    case OMX_IndexConfigCommonMirror:
    {
        OMX_CONFIG_MIRRORTYPE *mirrParam = (OMX_CONFIG_MIRRORTYPE *)pComponentConfigStructure;
        DecConfigParam *decConfig = pSfMjpegImplement->config;
        if(mirrParam->nPortIndex == (OMX_U32)(OMX_OUTPUT_PORT_INDEX))
        {
            decConfig->mirror = mirrParam->eMirror;
            LOG(SF_LOG_INFO, "Set Mirror %d \r\n", decConfig->mirror);
        }
        else
        {
            LOG(SF_LOG_WARN, "Only output port support Mirror config\r\n");
            ret = OMX_ErrorBadParameter;
        }
        break;
    }

    case OMX_IndexConfigCommonScale:
    {
        OMX_CONFIG_SCALEFACTORTYPE *scaleParam = (OMX_CONFIG_SCALEFACTORTYPE *)pComponentConfigStructure;
        DecConfigParam *decConfig = pSfMjpegImplement->config;
        OMX_U32 x;
        if(scaleParam->nPortIndex == (OMX_U32)(OMX_OUTPUT_PORT_INDEX))
        {
            /* In Q16 format */
            x = (1 << 16) / (scaleParam->xWidth);
            if(x >= 8)
                decConfig->iHorScaleMode = 3;
            else if(x >= 4)
                decConfig->iHorScaleMode = 2;
            else if(x >= 2)
                decConfig->iHorScaleMode = 1;
            else
                decConfig->iHorScaleMode = 0;

            x = (1 << 16) / (scaleParam->xHeight);
            if(x >= 8)
                decConfig->iVerScaleMode = 3;
            else if(x >= 4)
                decConfig->iVerScaleMode = 2;
            else if(x >= 2)
                decConfig->iVerScaleMode = 1;
            else
                decConfig->iVerScaleMode = 0;

            LOG(SF_LOG_INFO, "Set scale H 1/%d, V 1/%d \r\n", 1 << decConfig->iHorScaleMode, 1 << scaleParam->xHeight);
        }
        else
        {
            LOG(SF_LOG_WARN, "Only output port support Scale config\r\n");
            ret = OMX_ErrorBadParameter;
        }
        break;
    }

    default:
        break;
    }
EXIT:
    FunctionOut();

    return ret;
}

static OMX_ERRORTYPE InitDecoder(SF_OMX_COMPONENT *pSfOMXComponent)
{
    FunctionIn();
    SF_CODAJ12_IMPLEMEMT *pSfCodaj12Implement = pSfOMXComponent->componentImpl;
    JpgRet ret = JPG_RET_SUCCESS;
    DecConfigParam *decConfig = pSfCodaj12Implement->config;

    ret = pSfCodaj12Implement->functions->JPU_Init();
    if (ret != JPG_RET_SUCCESS && ret != JPG_RET_CALLED_BEFORE)
    {
        LOG(SF_LOG_ERR, "JPU_Init failed Error code is 0x%x \r\n", ret);
        return ret;
    }

    decConfig->feedingMode = FEEDING_METHOD_BUFFER;
    LOG(SF_LOG_DEBUG, "feedingMode = %d, StreamEndian = %d\r\n", decConfig->feedingMode, decConfig->StreamEndian);
    if ((pSfCodaj12Implement->feeder = pSfCodaj12Implement->functions->BitstreamFeeder_Create(decConfig->bitstreamFileName, decConfig->feedingMode, (EndianMode)decConfig->StreamEndian)) == NULL)
    {
        return OMX_ErrorResourcesLost;
    }

    FunctionOut();
    return OMX_ErrorNone;
}


static OMX_BOOL FillBufferDone(SF_OMX_COMPONENT *pSfOMXComponent, OMX_BUFFERHEADERTYPE *pBuffer)
{
    SF_CODAJ12_IMPLEMEMT *pSfCodaj12Implement = pSfOMXComponent->componentImpl;
    struct timeval tv;
    static OMX_U32 dec_cnt = 0;
    OMX_U32 fps = 0;
    OMX_U64 diff_time = 0; // ms
    static struct timeval tv_old = {0};

    FunctionIn();

    gettimeofday(&tv, NULL);
    if (gInitTimeStamp == 0)
    {
        gInitTimeStamp = tv.tv_sec * 1000000 + tv.tv_usec;
    }
    pBuffer->nTimeStamp = tv.tv_sec * 1000000 + tv.tv_usec - gInitTimeStamp;

    if (dec_cnt == 0) {
        tv_old = tv;
    }
    if (dec_cnt++ >= 50) {
        diff_time = (tv.tv_sec - tv_old.tv_sec) * 1000 + (tv.tv_usec - tv_old.tv_usec) / 1000;
        fps = 1000  * (dec_cnt - 1) / diff_time;
        dec_cnt = 0;
        LOG(SF_LOG_WARN, "Decoding fps: %d \r\n", fps);
    }

    LOG(SF_LOG_PERF, "OMX finish one buffer, header = %p, address = %p, size = %d, nTimeStamp = %d, nFlags = %X\r\n",
            pBuffer, pBuffer->pBuffer, pBuffer->nFilledLen, pBuffer->nTimeStamp, pBuffer->nFlags);
    // Following comment store data loal
    // {
    //     FILE *fb = fopen("./out.bcp", "ab+");
    //     LOG(SF_LOG_INFO, "%p %d\r\n", pBuffer->pBuffer, pBuffer->nFilledLen);
    //     fwrite(pBuffer->pBuffer, 1, pBuffer->nFilledLen, fb);
    //     fclose(fb);
    // }
    if (pSfOMXComponent->propagateMarkType.hMarkTargetComponent != NULL) {
        LOG(SF_LOG_INFO, "Component propagate mark to output port\r\n");
        pBuffer->hMarkTargetComponent = pSfOMXComponent->propagateMarkType.hMarkTargetComponent;
        pBuffer->pMarkData = pSfOMXComponent->propagateMarkType.pMarkData;
        pSfOMXComponent->propagateMarkType.hMarkTargetComponent = NULL;
        pSfOMXComponent->propagateMarkType.pMarkData = NULL;
    }

    if (pSfOMXComponent->state == OMX_StatePause && !pSfOMXComponent->bPortFlushing[OMX_OUTPUT_PORT_INDEX])
    {
        LOG(SF_LOG_INFO, "tmp store buf when pause\r\n");
        SF_Queue_Enqueue(pSfCodaj12Implement->outPauseQ, &pBuffer);
    }
    else{
        if (pBuffer->nFlags & OMX_BUFFERFLAG_EOS)
        {
            pSfOMXComponent->callbacks->EventHandler(pSfOMXComponent->pOMXComponent, pSfOMXComponent->pAppData, OMX_EventBufferFlag,
                                                 OMX_OUTPUT_PORT_INDEX, pBuffer->nFlags, NULL);
        }
        pSfOMXComponent->callbacks->FillBufferDone(pSfOMXComponent->pOMXComponent, pSfOMXComponent->pAppData, pBuffer);
        pSfOMXComponent->handlingBufferNum[OMX_OUTPUT_PORT_INDEX]--;
        if((pSfOMXComponent->handlingBufferNum[OMX_OUTPUT_PORT_INDEX] == 0) && pSfOMXComponent->bPortFlushing[OMX_OUTPUT_PORT_INDEX])
        {
            LOG(SF_LOG_INFO, "return all out buff\r\n");
            SF_SemaphorePost(pSfOMXComponent->portFlushSemaphore[OMX_OUTPUT_PORT_INDEX]);
        }
    }

    FunctionOut();
    return OMX_TRUE;
}
static OMX_BOOL EmptyBufferDone(SF_OMX_COMPONENT *pSfOMXComponent)
{
    SF_CODAJ12_IMPLEMEMT *pSfCodaj12Implement = pSfOMXComponent->componentImpl;
    OMX_BUFFERHEADERTYPE *pOMXBuffer = NULL;
    Message data;
    ssize_t ret;

    FunctionIn();
    LOG(SF_LOG_INFO, "Wait for done used buffer\r\n");
    ret = msgrcv(pSfCodaj12Implement->sBufferDoneQueue, (void *)&data, BUFSIZ, 0, IPC_NOWAIT);
    if (ret < 0)
    {
        LOG(SF_LOG_INFO, "msgrcv failed ret = %x\r\n", ret);
        return OMX_FALSE;
    }
    // switch (ret)
    // {
    // case ENOMSG:
    //     LOG(SF_LOG_INFO, "msgrcv no message\n");
    //     return OMX_TRUE;
    // default:
    //     LOG(SF_LOG_ERR, "msgrcv failed ret = %x\r\n", ret);
    //     return OMX_FALSE;
    // }

    pOMXBuffer = data.pBuffer;

    if (pSfOMXComponent->markType[OMX_INPUT_PORT_INDEX].hMarkTargetComponent != NULL)
    {
        LOG(SF_LOG_INFO, "set Component mark %p\r\n", pSfOMXComponent->markType[OMX_INPUT_PORT_INDEX].hMarkTargetComponent);
        pOMXBuffer->hMarkTargetComponent      = pSfOMXComponent->markType[OMX_INPUT_PORT_INDEX].hMarkTargetComponent;
        pOMXBuffer->pMarkData                 = pSfOMXComponent->markType[OMX_INPUT_PORT_INDEX].pMarkData;
        pSfOMXComponent->markType[OMX_INPUT_PORT_INDEX].hMarkTargetComponent = NULL;
        pSfOMXComponent->markType[OMX_INPUT_PORT_INDEX].pMarkData = NULL;
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

    //TODO: input/output Buffer done
    if (pSfOMXComponent->state == OMX_StatePause && !pSfOMXComponent->bPortFlushing[OMX_INPUT_PORT_INDEX])
    {
        LOG(SF_LOG_INFO, "tmp store buf when pause\r\n");
        SF_Queue_Enqueue(pSfCodaj12Implement->inPauseQ, &pOMXBuffer);
    }else{
        LOG(SF_LOG_DEBUG, "Finish inpu buffer header %p pBuffer %p\r\n",pOMXBuffer, pOMXBuffer->pBuffer);
        pSfOMXComponent->callbacks->EmptyBufferDone(pSfOMXComponent->pOMXComponent, pSfOMXComponent->pAppData, pOMXBuffer);
        pSfOMXComponent->handlingBufferNum[OMX_INPUT_PORT_INDEX]--;
        if((pSfOMXComponent->handlingBufferNum[OMX_INPUT_PORT_INDEX] == 0) && pSfOMXComponent->bPortFlushing[OMX_INPUT_PORT_INDEX])
        {
            LOG(SF_LOG_INFO, "return all in buff\r\n");
            SF_SemaphorePost(pSfOMXComponent->portFlushSemaphore[OMX_INPUT_PORT_INDEX]);
        }
    }

    FunctionOut();
    return OMX_TRUE;
}

static OMX_S32 FeedData(SF_OMX_COMPONENT *pSfOMXComponent)
{
    FunctionIn();
    SF_CODAJ12_IMPLEMEMT *pSfCodaj12Implement = pSfOMXComponent->componentImpl;
    BSFeeder feeder = pSfCodaj12Implement->feeder;
    JpgDecHandle handle = pSfCodaj12Implement->handle;
    jpu_buffer_t *vbStream = &pSfCodaj12Implement->vbStream;
    OMX_U32 nFilledLen = 0;

    OMX_BUFFERHEADERTYPE *pOMXBuffer = NULL;
    Message data;
    LOG(SF_LOG_INFO, "Wait for input buffer\r\n");
    if (msgrcv(pSfCodaj12Implement->sInputMessageQueue, (void *)&data, BUFSIZ, 0, 0) == -1)
    {
        LOG(SF_LOG_ERR, "msgrcv failed\n");
        return 0;
    }
    pOMXBuffer = data.pBuffer;
    LOG(SF_LOG_DEBUG, "get buffer %p\n", pOMXBuffer);

    if (!pOMXBuffer)
    {
        return -1;
    }

    if(!pOMXBuffer->nFilledLen){
        LOG(SF_LOG_DEBUG, "end of stream,end\n");
        msgsnd(pSfCodaj12Implement->sBufferDoneQueue, (void *)&data, sizeof(data) - sizeof(data.msg_type), 0);
        return 0;
    }
    else if(pOMXBuffer->nFlags == OMX_BUFFERFLAG_EOS)
    {
        LOG(SF_LOG_DEBUG, "end of stream,end\n");
        pSfCodaj12Implement->gotEos = OMX_TRUE;
    }

    if (pOMXBuffer->pBuffer != NULL && pOMXBuffer->nFilledLen > 0)
    {
        nFilledLen = pOMXBuffer->nFilledLen;
        LOG(SF_LOG_INFO, "Address = %p, size = %d\r\n", pOMXBuffer->pBuffer, nFilledLen);
        pSfCodaj12Implement->functions->BitstreamFeeder_SetData(feeder, pOMXBuffer->pBuffer, nFilledLen);
        pSfCodaj12Implement->functions->BitstreamFeeder_Act(feeder, handle, vbStream);
    }
    LOG(SF_LOG_INFO, "Send %p to sBufferDoneQueue\r\n", pOMXBuffer);
    if (msgsnd(pSfCodaj12Implement->sBufferDoneQueue, (void *)&data, sizeof(data) - sizeof(data.msg_type), 0) == -1)
    {
        LOG(SF_LOG_ERR, "msgsnd failed\n");
        return 0;
    }

    // LOG(SF_LOG_DEBUG, "EmptyBufferDone IN\r\n");
    // pSfOMXComponent->callbacks->EmptyBufferDone(pSfOMXComponent->pOMXComponent, pSfOMXComponent->pAppData, pOMXBuffer);
    // LOG(SF_LOG_DEBUG, "EmptyBufferDone OUT\r\n");

    // if (pOMXBuffer->nFlags & 0x1 == 0x1)
    // {
    //     pSfCodaj12Implement->bThreadRunning = OMX_FALSE;
    // }
    FunctionOut();
    return nFilledLen;
}

static OMX_ERRORTYPE WaitForOutputBufferReady(SF_OMX_COMPONENT *pSfOMXComponent,OMX_BUFFERHEADERTYPE **ppOMXBuffer)
{
    FunctionIn();
    OMX_ERRORTYPE ret = OMX_ErrorNone;
    SF_CODAJ12_IMPLEMEMT *pSfCodaj12Implement = pSfOMXComponent->componentImpl;
    OMX_BUFFERHEADERTYPE *pOMXBuffer = NULL;
    Message data;
    LOG(SF_LOG_INFO, "Wait for output buffer\r\n");
    if (msgrcv(pSfCodaj12Implement->sOutputMessageQueue, (void *)&data, BUFSIZ, 0, 0) == -1)
    {
        LOG(SF_LOG_ERR, "msgrcv failed\n");
        return OMX_ErrorInsufficientResources;
    }
    pOMXBuffer = data.pBuffer;
    LOG(SF_LOG_INFO, "get buffer header %p\r\n",pOMXBuffer);
    if (pOMXBuffer == NULL)
    {
        LOG(SF_LOG_INFO, "Buffer end flag detected! \r\n");
        pSfCodaj12Implement->bThreadRunning = OMX_FALSE;
        ret = OMX_ErrorNoMore;
    }
    *ppOMXBuffer = pOMXBuffer;
    FunctionOut();
    return ret;
}

static void ProcessThread(void *args)
{
    SF_OMX_COMPONENT *pSfOMXComponent = (SF_OMX_COMPONENT *)args;
    SF_CODAJ12_IMPLEMEMT *pSfCodaj12Implement = pSfOMXComponent->componentImpl;
    FrameBuffer *pFrameBuf = pSfCodaj12Implement->frameBuf;
    jpu_buffer_t *vbStream = &pSfCodaj12Implement->vbStream;
    JpgDecInitialInfo *initialInfo = &pSfCodaj12Implement->initialInfo;

    JpgDecOpenParam *decOP = &pSfCodaj12Implement->decOP;
    DecConfigParam *decConfig = pSfCodaj12Implement->config;
    OMX_BUFFERHEADERTYPE *pBuffer = NULL;

    JpgDecHandle handle;
    Int32 frameIdx = 0;
    JpgDecParam decParam = {0};
    JpgDecOutputInfo outputInfo = {0};
    Uint32 framebufWidth = 0, framebufHeight = 0, framebufStride = 0;
    Uint32 decodingWidth, decodingHeight;
    Uint32 displayWidth, displayHeight;
    Int32 int_reason = 0;
    Uint32 bitDepth = 0;
    Int32 size = 0;
    Int32 instIdx;
    Uint32 temp;
    BOOL scalerOn = FALSE;
    JpgRet ret = JPG_RET_SUCCESS;

    CbCrInterLeave chromaInterleave;
    PackedFormat packedFormat;
    FrameFormat subsample;

    FunctionIn();
    prctl(PR_SET_NAME, pSfOMXComponent->componentName);

    OMX_U32 nSizeBytes = pSfOMXComponent->portDefinition[0].nBufferSize;
    if (!vbStream->phys_addr)
    {
        vbStream->size = (nSizeBytes == 0) ? STREAM_BUF_SIZE : nSizeBytes;
        vbStream->size = (vbStream->size + 1023) & ~1023; // ceil128(size)
        if (pSfCodaj12Implement->functions->jdi_allocate_dma_memory(vbStream) < 0)
        {
            LOG(SF_LOG_ERR, "fail to allocate bitstream buffer\r\n");
            goto  thread_end;
        }
    }

    LOG(SF_LOG_INFO, "vbStream phys_addr = %x, size = %d, virt_addr = %x\r\n", vbStream->phys_addr, vbStream->size, vbStream->virt_addr);
    decOP->streamEndian = decConfig->StreamEndian;
    decOP->frameEndian = decConfig->FrameEndian;
    decOP->bitstreamBuffer = vbStream->phys_addr;
    decOP->bitstreamBufferSize = vbStream->size;
    //set virtual address mapped of physical address
    decOP->pBitStream = (BYTE *)vbStream->virt_addr; //lint !e511
    decOP->chromaInterleave = decConfig->cbcrInterleave;
    decOP->packedFormat = decConfig->packedFormat;
    decOP->roiEnable = decConfig->roiEnable;
    decOP->roiOffsetX = decConfig->roiOffsetX;
    decOP->roiOffsetY = decConfig->roiOffsetY;
    decOP->roiWidth = decConfig->roiWidth;
    decOP->roiHeight = decConfig->roiHeight;
    decOP->rotation = decConfig->rotation;
    decOP->mirror = decConfig->mirror;
    decOP->pixelJustification = decConfig->pixelJustification;

    if (decConfig->packedFormat)
    {
        decOP->outputFormat = FORMAT_MAX;
    }
    else
    {
        decOP->outputFormat = pSfCodaj12Implement->frameFormat;
    }

    decOP->intrEnableBit = ((1 << INT_JPU_DONE) | (1 << INT_JPU_ERROR) | (1 << INT_JPU_BIT_BUF_EMPTY));

    LOG(SF_LOG_DEBUG, "streamEndian = %x\r\n", decOP->streamEndian);
    LOG_APPEND(SF_LOG_DEBUG, "frameEndian = %x\r\n", decOP->frameEndian);
    LOG_APPEND(SF_LOG_DEBUG, "bitstreamBuffer = %x\r\n", decOP->bitstreamBuffer);
    LOG_APPEND(SF_LOG_DEBUG, "bitstreamBufferSize = %x\r\n", decOP->bitstreamBufferSize);
    LOG_APPEND(SF_LOG_DEBUG, "pBitStream = %x\r\n", decOP->pBitStream);
    LOG_APPEND(SF_LOG_DEBUG, "chromaInterleave = %x\r\n", decOP->chromaInterleave);
    LOG_APPEND(SF_LOG_DEBUG, "packedFormat = %x\r\n", decOP->packedFormat);
    LOG_APPEND(SF_LOG_DEBUG, "roiEnable = %x\r\n", decOP->roiEnable);
    LOG_APPEND(SF_LOG_DEBUG, "roiOffsetX = %x\r\n", decOP->roiOffsetX);
    LOG_APPEND(SF_LOG_DEBUG, "roiOffsetY = %x\r\n", decOP->roiOffsetY);
    LOG_APPEND(SF_LOG_DEBUG, "roiWidth = %x\r\n", decOP->roiWidth);
    LOG_APPEND(SF_LOG_DEBUG, "roiHeight = %x\r\n", decOP->roiHeight);
    LOG_APPEND(SF_LOG_DEBUG, "rotation = %x\r\n", decOP->rotation);
    LOG_APPEND(SF_LOG_DEBUG, "mirror = %x\r\n", decOP->mirror);
    LOG_APPEND(SF_LOG_DEBUG, "pixelJustification = %x\r\n", decOP->pixelJustification);
    LOG_APPEND(SF_LOG_DEBUG, "outputFormat = %x\r\n", decOP->outputFormat);
    LOG_APPEND(SF_LOG_DEBUG, "intrEnableBit = %x\r\n", decOP->intrEnableBit);

    ret = pSfCodaj12Implement->functions->JPU_DecOpen(&handle, decOP);
    if (ret != JPG_RET_SUCCESS)
    {
        LOG(SF_LOG_ERR, "JPU_DecOpen failed Error code is 0x%x \r\n", ret);
        return;
    }
    pSfCodaj12Implement->handle = handle;
    instIdx = pSfCodaj12Implement->instIdx = pSfCodaj12Implement->handle->instIndex;

    do
    {
        size = FeedData(pSfOMXComponent);
        if (size == 0)
        {
            LOG(SF_LOG_INFO, "FeedData = %d, end thread\r\n", size);
            WaitForOutputBufferReady(pSfOMXComponent,&pBuffer);
            pBuffer->nFilledLen = 0;
            pBuffer->nFlags = OMX_BUFFERFLAG_EOS;
            FillBufferDone(pSfOMXComponent, pBuffer);
            pBuffer = NULL;
            pSfCodaj12Implement->bThreadRunning = OMX_FALSE;
            goto thread_end;
        }
        else if (size < 0)
        {
            LOG(SF_LOG_INFO, "get null buf, end thread\r\n");
            pSfCodaj12Implement->bThreadRunning = OMX_FALSE;
            goto thread_end;
        }

        if ((ret = pSfCodaj12Implement->functions->JPU_DecGetInitialInfo(handle, initialInfo)) != JPG_RET_SUCCESS)
        {
            if (JPG_RET_BIT_EMPTY == ret)
            {
                LOG(SF_LOG_INFO, "<%s:%d> BITSTREAM EMPTY\n", __FUNCTION__, __LINE__);
                continue;
            }
            else
            {
                LOG(SF_LOG_ERR, "JPU_DecGetInitialInfo failed Error code is 0x%x, inst=%d \n", ret, instIdx);
                return;
            }
        }
    } while (JPG_RET_SUCCESS != ret);

    if (initialInfo->sourceFormat == FORMAT_420 || initialInfo->sourceFormat == FORMAT_422)
        framebufWidth = JPU_CEIL(16, initialInfo->picWidth);
    else
        framebufWidth = JPU_CEIL(8, initialInfo->picWidth);

    if (initialInfo->sourceFormat == FORMAT_420 || initialInfo->sourceFormat == FORMAT_440)
        framebufHeight = JPU_CEIL(16, initialInfo->picHeight);
    else
        framebufHeight = JPU_CEIL(8, initialInfo->picHeight);

    LOG(SF_LOG_DEBUG, "framebufWidth: %d, framebufHeight: %d\r\n", framebufWidth, framebufHeight);

    decodingWidth = framebufWidth >> decConfig->iHorScaleMode;
    decodingHeight = framebufHeight >> decConfig->iVerScaleMode;
    if (decOP->packedFormat != PACKED_FORMAT_NONE && decOP->packedFormat != PACKED_FORMAT_444)
    {
        // When packed format, scale-down resolution should be multiple of 2.
        decodingWidth = JPU_CEIL(2, decodingWidth);
    }

    if (decConfig->packedFormat)
    {
        subsample = initialInfo->sourceFormat;
    }
    else
    {
        subsample = pSfCodaj12Implement->frameFormat;
    }

    temp = decodingWidth;
    decodingWidth = (decConfig->rotation == 90 || decConfig->rotation == 270) ? decodingHeight : decodingWidth;
    decodingHeight = (decConfig->rotation == 90 || decConfig->rotation == 270) ? temp : decodingHeight;
    if (decConfig->roiEnable == TRUE)
    {
        decodingWidth = framebufWidth = initialInfo->roiFrameWidth;
        decodingHeight = framebufHeight = initialInfo->roiFrameHeight;
    }

    LOG(SF_LOG_DEBUG, "decodingWidth: %d, decodingHeight: %d\n", decodingWidth, decodingHeight);

    if (0 != decConfig->iHorScaleMode || 0 != decConfig->iVerScaleMode) {
        displayWidth  = JPU_FLOOR(2, (framebufWidth >> decConfig->iHorScaleMode));
        displayHeight = JPU_FLOOR(2, (framebufHeight >> decConfig->iVerScaleMode));
    }
    else {
        displayWidth  = decodingWidth;
        displayHeight = decodingHeight;
    }

    LOG(SF_LOG_DEBUG, "displayWidth: %d, displayHeight: %d\n", displayWidth, displayHeight);

    if (decOP->rotation != 0 || decOP->mirror != MIRDIR_NONE)
    {
        if (decOP->outputFormat != FORMAT_MAX && decOP->outputFormat != initialInfo->sourceFormat)
        {
            LOG(SF_LOG_ERR, "The rotator cannot work with the format converter together.\n");
            goto thread_end;
        }
    }

    bitDepth = initialInfo->bitDepth;
    scalerOn = (BOOL)(decConfig->iHorScaleMode || decConfig->iVerScaleMode);
    // may be handle == NULL on ffmpeg case
    if (bitDepth == 0)
    {
        bitDepth = 8;
    }
    chromaInterleave = decOP->chromaInterleave;
    packedFormat = decOP->packedFormat;

    LOG(SF_LOG_DEBUG, "AllocateFrameBuffer\r\n");
    LOG_APPEND(SF_LOG_DEBUG, "instIdx = %d subsample = %d chromaInterleave = %d packedFormat = %d rotation = %d\r\n",
                instIdx, subsample, chromaInterleave, packedFormat, decConfig->rotation);
    LOG_APPEND(SF_LOG_DEBUG, "scalerOn = %d decodingWidth = %d decodingHeight = %d bitDepth = %d\r\n",
                scalerOn, decodingWidth, decodingHeight, bitDepth);

    if (pSfCodaj12Implement->functions->AllocateFrameBuffer(instIdx, subsample, chromaInterleave, packedFormat, decConfig->rotation,
            scalerOn, decodingWidth, decodingHeight, bitDepth, initialInfo->minFrameBufferCount) == FALSE) {
        LOG(SF_LOG_ERR, "Failed to AllocateFrameBuffer()\n");
        goto thread_end;
    }
    pSfCodaj12Implement->allocBufFlag = OMX_TRUE;

    for( int i = 0; i < initialInfo->minFrameBufferCount; ++i ) {
        FRAME_BUF *pFrame = pSfCodaj12Implement->functions->GetFrameBuffer(instIdx, i);
        pFrameBuf[i].bufY  = pFrame->vbY.phys_addr;
        pFrameBuf[i].bufCb = pFrame->vbCb.phys_addr;
        if (decOP->chromaInterleave == CBCR_SEPARATED)
            pFrameBuf[i].bufCr = pFrame->vbCr.phys_addr;
        pFrameBuf[i].stride  = pFrame->strideY;
        pFrameBuf[i].strideC = pFrame->strideC;
        pFrameBuf[i].endian  = decOP->frameEndian;
        pFrameBuf[i].format  = (FrameFormat)pFrame->Format;
    }
    framebufStride = pFrameBuf[0].stride;

    if ((ret = pSfCodaj12Implement->functions->JPU_DecRegisterFrameBuffer
                (handle, pFrameBuf, initialInfo->minFrameBufferCount, framebufStride)) != JPG_RET_SUCCESS) {
        LOG(SF_LOG_ERR, "JPU_DecRegisterFrameBuffer failed Error code is 0x%x \n", ret );
        goto thread_end;
    }

    //TODO:Set output info
    OMX_PARAM_PORTDEFINITIONTYPE *pPortDefinition = &pSfOMXComponent->portDefinition[OMX_OUTPUT_PORT_INDEX];
    LOG(SF_LOG_DEBUG, "picWidth = %d, picHeight = %d\r\n", initialInfo->picWidth, initialInfo->picHeight);
    pPortDefinition->format.video.nFrameWidth = initialInfo->picWidth;
    pPortDefinition->format.video.nFrameHeight = initialInfo->picHeight;
    pPortDefinition->format.video.nStride = initialInfo->picWidth;
    pPortDefinition->format.video.nSliceHeight = initialInfo->picHeight;
    pPortDefinition->format.video.eCompressionFormat = OMX_IMAGE_CodingUnused;
    switch (pSfCodaj12Implement->frameFormat)
    {
    case FORMAT_400:
    case FORMAT_420:
        pPortDefinition->nBufferSize = initialInfo->picWidth * initialInfo->picHeight * 3 / 2;
        break;
    case FORMAT_422:
    case FORMAT_440:
        pPortDefinition->nBufferSize = initialInfo->picWidth * initialInfo->picHeight * 2;
        break;
    case FORMAT_444:
    case FORMAT_MAX:
        pPortDefinition->nBufferSize = initialInfo->picWidth * initialInfo->picHeight * 3;
        break;
    default:
        pPortDefinition->nBufferSize = initialInfo->picWidth * initialInfo->picHeight * 4;
        break;
    }
    LOG(SF_LOG_DEBUG, "Output buffer size = %d\r\n", pPortDefinition->nBufferSize);
    if (pPortDefinition->nBufferCountActual < initialInfo->minFrameBufferCount)
    {
        pPortDefinition->nBufferCountActual = initialInfo->minFrameBufferCount;
    }
    if (pPortDefinition->nBufferCountMin < initialInfo->minFrameBufferCount)
    {
        pPortDefinition->nBufferCountMin = initialInfo->minFrameBufferCount;
    }

    LOG(SF_LOG_DEBUG, "OMX_EventPortSettingsChanged IN\r\n");
    pSfOMXComponent->callbacks->EventHandler(pSfOMXComponent->pOMXComponent, pSfOMXComponent->pAppData, OMX_EventPortSettingsChanged,
                                             1, OMX_IndexParamPortDefinition, NULL);
    LOG(SF_LOG_DEBUG, "OMX_EventPortSettingsChanged OUT\r\n");
    LOG(SF_LOG_INFO, "Wait for out buffers ready\r\n");

    OMX_U32 nBufferNumber = pSfOMXComponent->portDefinition[OMX_OUTPUT_PORT_INDEX].nBufferCountActual;

    FRAME_BUF *pFrame = pSfCodaj12Implement->frame;
    LOG(SF_LOG_INFO, "Update and regist %d FrameBuffers\r\n", nBufferNumber);
    LOG(SF_LOG_DEBUG, "%20s%20s%20s%20s\r\n", "virtAddr", "Y", "CB", "CR");
    for (int i = 0; i < nBufferNumber; i ++)
    {
        LOG_APPEND(SF_LOG_DEBUG, "%20lX%20lX%20lX%20lX\r\n", pFrame[i].vbY.virt_addr,
            pFrame[i].vbY.phys_addr, pFrame[i].vbCb.phys_addr, pFrame[i].vbCr.phys_addr);
    }

    pSfCodaj12Implement->functions->JPU_DecGiveCommand(handle, SET_JPG_SCALE_HOR, &(decConfig->iHorScaleMode));
    pSfCodaj12Implement->functions->JPU_DecGiveCommand(handle, SET_JPG_SCALE_VER, &(decConfig->iVerScaleMode));

    /* LOG HEADER */
    LOG(SF_LOG_INFO, "I   F    FB_INDEX  FRAME_START  ECS_START  CONSUME   RD_PTR   WR_PTR      CYCLE\n");
    LOG(SF_LOG_INFO, "-------------------------------------------------------------------------------\n");

    while (pSfCodaj12Implement->bThreadRunning)
    {
        if (WaitForOutputBufferReady(pSfOMXComponent,&pBuffer) != OMX_ErrorNone)
        {
            continue;
        }

        LOG(SF_LOG_INFO, "pBuffer index:%d\r\n", ((SF_OMX_BUF_INFO*)(pBuffer->pOutputPortPrivate))->index);
refill:
        ret = pSfCodaj12Implement->functions->JPU_DecStartOneFrame(handle, &decParam);
        if (ret != JPG_RET_SUCCESS && ret != JPG_RET_EOS)
        {
            if (ret == JPG_RET_BIT_EMPTY)
            {
                LOG(SF_LOG_INFO, "BITSTREAM NOT ENOUGH.............\n");
                EmptyBufferDone(pSfOMXComponent);
                size = FeedData(pSfOMXComponent);
                if (size == 0)
                {
                    LOG(SF_LOG_INFO, "FeedData = %d, end thread\r\n", size);
                    pBuffer->nFilledLen = 0;
                    pBuffer->nFlags |= OMX_BUFFERFLAG_EOS;
                    FillBufferDone(pSfOMXComponent, pBuffer);
                    pBuffer = NULL;
                    pSfCodaj12Implement->bThreadRunning = OMX_FALSE;
                    break;
                }
                else if (size < 0)
                {
                    LOG(SF_LOG_INFO, "get null buf, end thread\r\n");
                    pSfCodaj12Implement->bThreadRunning = OMX_FALSE;
                    goto thread_end;
                }
                goto refill;
            }

            LOG(SF_LOG_ERR, "JPU_DecStartOneFrameBySerialNum failed Error code is 0x%x \n", ret);
            return;
        }
        if (ret == JPG_RET_EOS)
        {
            pSfCodaj12Implement->functions->JPU_DecGetOutputInfo(handle, &outputInfo);
            break;
        }

        LOG(SF_LOG_INFO, "wait process frame \n");

        while (1)
        {
            if ((int_reason = pSfCodaj12Implement->functions->JPU_WaitInterrupt(handle, JPU_INTERRUPT_TIMEOUT_MS)) == -1)
            {
                LOG(SF_LOG_ERR, "Error : timeout happened\n");
                pSfCodaj12Implement->functions->JPU_SWReset(handle);
                break;
            }

            if (int_reason & ((1 << INT_JPU_DONE) | (1 << INT_JPU_ERROR) | (1 << INT_JPU_SLICE_DONE)))
            {
                // Do no clear INT_JPU_DONE and INT_JPU_ERROR interrupt. these will be cleared in JPU_DecGetOutputInfo.
                LOG(SF_LOG_INFO, "\tINSTANCE #%d int_reason: %08x\n", handle->instIndex, int_reason);
                break;
            }

            // if (int_reason & (1 << INT_JPU_BIT_BUF_EMPTY))
            // {
            //     if (decConfig->feedingMode != FEEDING_METHOD_FRAME_SIZE)
            //     {
            //         pSfCodaj12Implement->functions->BitstreamFeeder_Act(feeder, handle, &vbStream);
            //     }
            //     pSfCodaj12Implement->functions->JPU_ClrStatus(handle, (1 << INT_JPU_BIT_BUF_EMPTY));
            // }
        }
        LOG(SF_LOG_INFO, "process frame done\n");
        EmptyBufferDone(pSfOMXComponent);
        if ((ret = pSfCodaj12Implement->functions->JPU_DecGetOutputInfo(handle, &outputInfo)) != JPG_RET_SUCCESS)
        {
            LOG(SF_LOG_ERR, "JPU_DecGetOutputInfo failed Error code is 0x%x \n", ret);
            break;
        }

        if (outputInfo.indexFrameDisplay == -1)
        {
            LOG(SF_LOG_ERR, "indexFrameDisplay -1\r\n");
            goto thread_end;;
        }

        pBuffer->nFilledLen = pSfCodaj12Implement->functions->SaveYuvImageHelper
                        (pBuffer->pBuffer, &pFrameBuf[outputInfo.indexFrameDisplay],
                        decOP->chromaInterleave, decOP->packedFormat, initialInfo->picWidth, initialInfo->picHeight, bitDepth);
        if (!pBuffer->nFilledLen)
        {
            LOG(SF_LOG_ERR, "fill out buffer err -1\r\n");
            goto thread_end;
        }

        LOG(SF_LOG_DEBUG, "decPicSize = [%d %d], PicSize = [%d %d], pBuffer = %p, fillen %d\r\n",
            outputInfo.decPicWidth, outputInfo.decPicHeight, initialInfo->picWidth,
            initialInfo->picHeight, pBuffer->pBuffer, pBuffer->nFilledLen);
        // AdjustFrame(pBuffer, outputInfo.decPicWidth, outputInfo.decPicHeight, nDestStride, nDestHeight);
        if (pSfCodaj12Implement->gotEos)
        {
            pSfCodaj12Implement->gotEos = OMX_FALSE;
            pBuffer->nFlags |= OMX_BUFFERFLAG_EOS;
        }
        FillBufferDone(pSfOMXComponent, pBuffer);
        pBuffer = NULL;

        if(pSfCodaj12Implement->bPause){
            LOG(SF_LOG_DEBUG,"in pause\n");
            SF_SemaphoreWait(pSfCodaj12Implement->pauseOutSemaphore);
            LOG(SF_LOG_DEBUG,"out pause\n");
        }
        // LOG(SF_LOG_DEBUG, "FillBufferDone IN\r\n");
        // Following comment store data loal
        // {
        //     FILE *fb = fopen("./out.bcp", "ab+");
        //     LOG(SF_LOG_INFO, "%p %d\r\n", pBuffer->pBuffer, pBuffer->nFilledLen);
        //     fwrite(pBuffer->pBuffer, 1, pBuffer->nFilledLen, fb);
        //     fclose(fb);
        // }
        // pSfOMXComponent->callbacks->FillBufferDone(pSfOMXComponent->pOMXComponent, pSfOMXComponent->pAppData, pBuffer);
        // LOG(SF_LOG_DEBUG, "FillBufferDone OUT\r\n");
        if (outputInfo.numOfErrMBs)
        {
            Int32 errRstIdx, errPosX, errPosY;
            errRstIdx = (outputInfo.numOfErrMBs & 0x0F000000) >> 24;
            errPosX = (outputInfo.numOfErrMBs & 0x00FFF000) >> 12;
            errPosY = (outputInfo.numOfErrMBs & 0x00000FFF);
            LOG(SF_LOG_ERR, "Error restart Idx : %d, MCU x:%d, y:%d, in Frame : %d \n", errRstIdx, errPosX, errPosY, frameIdx);
        }

        // if (pSfCodaj12Implement->bThreadRunning == OMX_FALSE)
        // {
        //     break;
        // }
        pSfCodaj12Implement->functions->JPU_DecSetRdPtrEx(handle, vbStream->phys_addr, TRUE);
        size = 0;
        while (!size)
        {
            size = FeedData(pSfOMXComponent);
            if (size == 0)
            {
                LOG(SF_LOG_INFO, "FeedData = %d, end thread\r\n", size);
                WaitForOutputBufferReady(pSfOMXComponent,&pBuffer);
                pBuffer->nFilledLen = 0;
                pBuffer->nFlags = OMX_BUFFERFLAG_EOS;
                EmptyBufferDone(pSfOMXComponent);
                FillBufferDone(pSfOMXComponent, pBuffer);
                pBuffer = NULL;
            }
            else if (size < 0)
            {
                LOG(SF_LOG_INFO, "get null buf, end thread\r\n");
                pSfCodaj12Implement->bThreadRunning = OMX_FALSE;
                goto thread_end;
            }
        }
    }
thread_end:
    LOG(SF_LOG_DEBUG, "decoder thread end\r\n");
    if (pBuffer)
    {
        pBuffer->nFilledLen = 0;
        pBuffer->nFlags = OMX_BUFFERFLAG_EOS;
        FillBufferDone(pSfOMXComponent, pBuffer);
    }
    CodaJ12FlushBuffer(pSfOMXComponent, OMX_INPUT_PORT_INDEX);
    CodaJ12FlushBuffer(pSfOMXComponent, OMX_OUTPUT_PORT_INDEX);
    pSfCodaj12Implement->currentState = OMX_StateIdle;
    pSfCodaj12Implement->functions->JPU_DecClose(handle);
    FunctionOut();
    ThreadExit(NULL);
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
    SF_CODAJ12_IMPLEMEMT *pSfCodaj12Implement = pSfOMXComponent->componentImpl;
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

    ret = SF_Queue_Enqueue(pSfCodaj12Implement->CmdQueue, &cmd);

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
    SF_OMX_BUF_INFO *pBufInfo;
    OMX_COMPONENTTYPE *pOMXComponent = (OMX_COMPONENTTYPE *)hComponent;
    SF_OMX_COMPONENT *pSfOMXComponent = pOMXComponent->pComponentPrivate;

    FunctionIn();

    if(pBufferHdr == NULL)
        return OMX_ErrorBadParameter;
    if(nPortIndex >= OMX_PORT_MAX)
        return OMX_ErrorBadParameter;

    if(nPortIndex == OMX_INPUT_PORT_INDEX)
        pBufInfo = pBufferHdr->pInputPortPrivate;
    else if(nPortIndex == OMX_OUTPUT_PORT_INDEX)
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

    if (pSfOMXComponent->assignedBufferNum[nPortIndex] == 0) {
        LOG(SF_LOG_INFO, "unloadedResource signal set\r\n");
        SF_SemaphorePost(pSfOMXComponent->portUnloadSemaphore[nPortIndex]);
        pSfOMXComponent->portDefinition[nPortIndex].bPopulated = OMX_FALSE;
    }

    FunctionOut();
    return OMX_ErrorNone;
}

static void CmdThread(void *args)
{
    SF_OMX_COMPONENT *pSfOMXComponent = (SF_OMX_COMPONENT *)args;
    SF_CODAJ12_IMPLEMEMT *pSfCodaj12Implement = pSfOMXComponent->componentImpl;
    OMX_BUFFERHEADERTYPE *pOMXBuffer = NULL;
    OMX_BUFFERHEADERTYPE **ppBuffer = NULL;
    OMX_ERRORTYPE ret = OMX_ErrorNone;
    OMX_STATETYPE comCurrentState;
    OMX_U32 i = 0, cnt = 0;
    DEC_CMD *pCmd;
    Message data;
    void *ThreadRet;

    while(1){
        pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
        pCmd = (DEC_CMD*)SF_Queue_Dequeue_Block(pSfCodaj12Implement->CmdQueue);
        pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
        pthread_testcancel();
        pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

        if(pCmd->Cmd == DEC_StopThread)
            break;

        switch (pCmd->Cmd)
        {
        case OMX_CommandStateSet:
            LOG(SF_LOG_INFO, "OMX dest state = %X\r\n", pCmd->nParam);

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
                    // enqueue null mean encoder thread cycle end
                    pSfCodaj12Implement->bThreadRunning = 0;
                    data.msg_type = 1;
                    data.msg_flag = OMX_BUFFERFLAG_EOS;
                    data.pBuffer = NULL;
                    LOG(SF_LOG_DEBUG, "Send to message queue\r\n");
                    if (msgsnd(pSfCodaj12Implement->sOutputMessageQueue, (void *)&data, sizeof(data) - sizeof(data.msg_type), 0) == -1)
                    {
                        LOG(SF_LOG_ERR, "msgsnd failed\n");
                    }
                    if (msgsnd(pSfCodaj12Implement->sInputMessageQueue, (void *)&data, sizeof(data) - sizeof(data.msg_type), 0) == -1)
                    {
                        LOG(SF_LOG_ERR, "msgsnd failed\n");
                    }
                    pthread_join(pSfCodaj12Implement->pProcessThread->pthread, &ThreadRet);
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
                    if (pSfCodaj12Implement->feeder){
                        pSfCodaj12Implement->functions->BitstreamFeeder_Destroy(pSfCodaj12Implement->feeder);
                        pSfCodaj12Implement->feeder = NULL;
                    }

                    if(pSfCodaj12Implement->allocBufFlag == OMX_TRUE)
                    {
                        pSfCodaj12Implement->functions->FreeFrameBuffer(pSfCodaj12Implement->instIdx);
                        pSfCodaj12Implement->allocBufFlag = OMX_FALSE;
                    }

                    if (pSfCodaj12Implement->vbStream.phys_addr)
                        pSfCodaj12Implement->functions->jdi_free_dma_memory(&pSfCodaj12Implement->vbStream);

                    pSfCodaj12Implement->functions->JPU_DeInit();
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
                    InitDecoder(pSfOMXComponent);
                    for (i = 0; i < 2; i++)
                    {
                        if (pSfOMXComponent->portDefinition[i].bEnabled){
                            LOG(SF_LOG_INFO,"SemaphoreWait \r\n");
                            SF_SemaphoreWait(pSfOMXComponent->portSemaphore[i]);
                            LOG(SF_LOG_INFO,"SemaphoreWait out\r\n");
                        }
                    }
                    break;
                case OMX_StateExecuting:
                    pSfCodaj12Implement->bThreadRunning = 0;
                    data.msg_type = 1;
                    data.msg_flag = OMX_BUFFERFLAG_EOS;
                    data.pBuffer = NULL;
                    LOG(SF_LOG_DEBUG, "Send to message queue\r\n");
                    if (msgsnd(pSfCodaj12Implement->sOutputMessageQueue, (void *)&data, sizeof(data) - sizeof(data.msg_type), 0) == -1)
                    {
                        LOG(SF_LOG_ERR, "msgsnd failed\n");
                    }
                    if (msgsnd(pSfCodaj12Implement->sInputMessageQueue, (void *)&data, sizeof(data) - sizeof(data.msg_type), 0) == -1)
                    {
                        LOG(SF_LOG_ERR, "msgsnd failed\n");
                    }
                    pthread_join(pSfCodaj12Implement->pProcessThread->pthread, &ThreadRet);
                    LOG(SF_LOG_INFO, "Encoder thread end %ld\r\n", (Uint64)ThreadRet);
                    CodaJ12FlushBuffer(pSfOMXComponent,OMX_INPUT_PORT_INDEX);
                    CodaJ12FlushBuffer(pSfOMXComponent,OMX_OUTPUT_PORT_INDEX);
                    break;
                case OMX_StateIdle:
                    pSfCodaj12Implement->bPause = OMX_FALSE;
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
                    pSfCodaj12Implement->bThreadRunning = OMX_TRUE;
                    pSfCodaj12Implement->bPause =OMX_FALSE;
                    CreateThread(&pSfCodaj12Implement->pProcessThread, ProcessThread, (void *)pSfOMXComponent);
                    break;
                case OMX_StateExecuting:
                    pSfCodaj12Implement->bPause = OMX_FALSE;
                    SF_SemaphorePost(pSfCodaj12Implement->pauseOutSemaphore);
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
                    pSfCodaj12Implement->bPause = OMX_TRUE;
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
                    ppBuffer = SF_Queue_Dequeue(pSfCodaj12Implement->outPauseQ);
                    while (ppBuffer)
                    {
                        pOMXBuffer = *ppBuffer;
                        pSfOMXComponent->callbacks->FillBufferDone(pSfOMXComponent->pOMXComponent, pSfOMXComponent->pAppData, pOMXBuffer);
                        pSfOMXComponent->handlingBufferNum[1]--;
                        ppBuffer = SF_Queue_Dequeue(pSfCodaj12Implement->outPauseQ);
                    }
                    ppBuffer = SF_Queue_Dequeue(pSfCodaj12Implement->inPauseQ);
                    while (ppBuffer)
                    {
                        pOMXBuffer = *ppBuffer;
                        pSfOMXComponent->callbacks->EmptyBufferDone(pSfOMXComponent->pOMXComponent, pSfOMXComponent->pAppData, pOMXBuffer);
                        pSfOMXComponent->handlingBufferNum[0]--;
                        ppBuffer = SF_Queue_Dequeue(pSfCodaj12Implement->inPauseQ);
                    }
                }
            }
            else
            {
                pSfOMXComponent->callbacks->EventHandler(pSfOMXComponent->pOMXComponent, pSfOMXComponent->pAppData,
                                                    OMX_EventError, ret, 0, NULL);
                LOG(SF_LOG_DEBUG, "Can't go to state %d from %d, ret %X \r\n",pCmd->nParam, pSfOMXComponent->state, ret);
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
                    CodaJ12FlushBuffer(pSfOMXComponent, nPort);
                    if (pSfOMXComponent->state == OMX_StatePause)
                    {
                        ppBuffer = SF_Queue_Dequeue(pSfCodaj12Implement->inPauseQ);
                        while (ppBuffer)
                        {
                            pOMXBuffer = *ppBuffer;
                            pSfOMXComponent->callbacks->EmptyBufferDone(pSfOMXComponent->pOMXComponent, pSfOMXComponent->pAppData, pOMXBuffer);
                            pSfOMXComponent->handlingBufferNum[0]--;
                            if((pSfOMXComponent->handlingBufferNum[0] == 0) && pSfOMXComponent->bPortFlushing[0])
                            {
                                LOG(SF_LOG_INFO, "return all input buff\r\n");
                                SF_SemaphorePost(pSfOMXComponent->portFlushSemaphore[0]);
                            }
                            ppBuffer = SF_Queue_Dequeue(pSfCodaj12Implement->inPauseQ);
                        }
                    }
                    if(pSfOMXComponent->handlingBufferNum[0] > 0)
                        SF_SemaphoreWait(pSfOMXComponent->portFlushSemaphore[0]);
                    pSfOMXComponent->callbacks->EventHandler(pSfOMXComponent->pOMXComponent, pSfOMXComponent->pAppData,
                                                    OMX_EventCmdComplete, OMX_CommandFlush, nPort, NULL);
                    pSfOMXComponent->bPortFlushing[0] = OMX_FALSE;
                }
                else
                {
                    pSfOMXComponent->bPortFlushing[1] = OMX_TRUE;
                    CodaJ12FlushBuffer(pSfOMXComponent, nPort);
                    if (pSfOMXComponent->state == OMX_StatePause)
                    {
                        ppBuffer = SF_Queue_Dequeue(pSfCodaj12Implement->outPauseQ);
                        while (ppBuffer)
                        {
                            pOMXBuffer = *ppBuffer;
                            pSfOMXComponent->callbacks->FillBufferDone(pSfOMXComponent->pOMXComponent, pSfOMXComponent->pAppData, pOMXBuffer);
                            pSfOMXComponent->handlingBufferNum[1]--;
                            if((pSfOMXComponent->handlingBufferNum[1] == 0) && pSfOMXComponent->bPortFlushing[1])
                            {
                                LOG(SF_LOG_INFO, "return all out buff\r\n");
                                SF_SemaphorePost(pSfOMXComponent->portFlushSemaphore[1]);
                            }
                            ppBuffer = SF_Queue_Dequeue(pSfCodaj12Implement->outPauseQ);
                        }
                    }
                    if(pSfOMXComponent->handlingBufferNum[1] > 0)
                        SF_SemaphoreWait(pSfOMXComponent->portFlushSemaphore[1]);
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

                CodaJ12FlushBuffer(pSfOMXComponent, nPort);
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
    compUUID[0] = (OMX_U32)pOMXComponent;
    compUUID[1] = (OMX_U32)pOMXComponent;
    compUUID[2] = (OMX_U32)pOMXComponent;
    compUUID[3] = (OMX_U32)pOMXComponent;
    //compUUID[1] = getpid();
    //compUUID[2] = getuid();
    memcpy(*pComponentUUID, compUUID, 4 * sizeof(*compUUID));

    ret = OMX_ErrorNone;

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

static OMX_U32 nInstance = 0;

static OMX_ERRORTYPE SF_OMX_ComponentConstructor(SF_OMX_COMPONENT *pSfOMXComponent)
{
    OMX_ERRORTYPE ret = OMX_ErrorNone;
    SF_CODAJ12_IMPLEMEMT *pSfCodaj12Implement;
    FunctionIn();

    if (nInstance >= 1)
    {
        ret = OMX_ErrorInsufficientResources;
        goto EXIT;
    }

    nInstance++;

    ret = InitMjpegStructorCommon(pSfOMXComponent);
    if (ret != OMX_ErrorNone)
    {
        nInstance--;
        goto EXIT;
    }

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
    pSfCodaj12Implement = pSfOMXComponent->componentImpl;
    pSfCodaj12Implement->currentState = OMX_StateLoaded;

    pSfCodaj12Implement->CmdQueue = SF_Queue_Create(20, sizeof(DEC_CMD));
    if (NULL == pSfCodaj12Implement->CmdQueue)
    {
        LOG(SF_LOG_ERR, "create CmdQueue error");
        nInstance--;
        return OMX_ErrorInsufficientResources;
    }
    pSfCodaj12Implement->outPauseQ = SF_Queue_Create(20, sizeof(OMX_BUFFERHEADERTYPE*));
    if (NULL == pSfCodaj12Implement->outPauseQ)
    {
        LOG(SF_LOG_ERR, "create outPauseQ error");
        nInstance--;
        return OMX_ErrorInsufficientResources;
    }
    pSfCodaj12Implement->inPauseQ = SF_Queue_Create(20, sizeof(OMX_BUFFERHEADERTYPE*));
    if (NULL == pSfCodaj12Implement->inPauseQ)
    {
        LOG(SF_LOG_ERR, "create inPauseQ error");
        nInstance--;
        return OMX_ErrorInsufficientResources;
    }

    CreateThread(&pSfCodaj12Implement->pCmdThread, CmdThread, (void *)pSfOMXComponent);
    pSfCodaj12Implement->bCmdRunning = OMX_TRUE;

    //InitDecoder(pSfOMXComponent);
EXIT:
    FunctionOut();

    return ret;
}

static OMX_ERRORTYPE SF_OMX_ComponentClear(SF_OMX_COMPONENT *pSfOMXComponent)
{
    OMX_ERRORTYPE ret = OMX_ErrorNone;
    SF_CODAJ12_IMPLEMEMT *pSfCodaj12Implement = pSfOMXComponent->componentImpl;
    jpu_buffer_t *vbStream = &pSfCodaj12Implement->vbStream;
    DEC_CMD cmd;
    void *ThreadRet = NULL;
    FunctionIn();

    if (pSfCodaj12Implement->feeder){
        pSfCodaj12Implement->functions->BitstreamFeeder_Destroy(pSfCodaj12Implement->feeder);
        pSfCodaj12Implement->feeder = NULL;
    }

    if(pSfCodaj12Implement->allocBufFlag == OMX_TRUE)
    {
        pSfCodaj12Implement->functions->FreeFrameBuffer(pSfCodaj12Implement->instIdx);
        pSfCodaj12Implement->allocBufFlag = OMX_FALSE;
    }

    if (vbStream->phys_addr)
        pSfCodaj12Implement->functions->jdi_free_dma_memory(vbStream);

    cmd.Cmd = DEC_StopThread;
    SF_Queue_Enqueue(pSfCodaj12Implement->CmdQueue, &cmd);
    pthread_cancel(pSfCodaj12Implement->pCmdThread->pthread);
	pthread_join(pSfCodaj12Implement->pCmdThread->pthread, &ThreadRet);
    LOG(SF_LOG_INFO, "Cmd thread end %ld\r\n", (Uint64)ThreadRet);
    SF_Queue_Destroy(pSfCodaj12Implement->CmdQueue);
    SF_Queue_Destroy(pSfCodaj12Implement->inPauseQ);
    SF_Queue_Destroy(pSfCodaj12Implement->outPauseQ);
    // TODO
    // MjpegClearCommon(hComponent);
    nInstance--;
    FunctionOut();

    return ret;
}

//TODO happer
SF_OMX_COMPONENT sf_dec_decoder_mjpeg = {
    .componentName = "OMX.sf.video_decoder.mjpeg",
    .libName = "libcodadec.so",
    .pOMXComponent = NULL,
    .SF_OMX_ComponentConstructor = SF_OMX_ComponentConstructor,
    .SF_OMX_ComponentClear = SF_OMX_ComponentClear,
    // .functions = NULL,
    // .bitFormat = STD_HEVC,
    .componentImpl = NULL,
    .fwPath = "/lib/firmware/chagall.bin",
    .componentRule = "video_decoder.mjpeg"};
