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

static OMX_ERRORTYPE SF_OMX_EmptyThisBuffer(
    OMX_IN OMX_HANDLETYPE hComponent,
    OMX_IN OMX_BUFFERHEADERTYPE *pBuffer)
{
    OMX_ERRORTYPE ret = OMX_ErrorNone;
    OMX_BOOL bSendNULL = OMX_FALSE;
    OMX_BOOL bSendMessage = OMX_TRUE;
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

    if (pBuffer->nFilledLen == 0 || (pBuffer->nFlags & 0x1) == 0x1)
    {
        bSendNULL = OMX_TRUE;
    }
    else
    {
        bSendNULL = OMX_FALSE;
    }

    if (pBuffer->pBuffer == NULL)
    {
        bSendMessage = OMX_FALSE;
    }
    else
    {
        bSendMessage = OMX_TRUE;
    }

    Message data;
    data.msg_type = 1;
    data.msg_flag = 0;
    if (bSendMessage)
    {
        data.pBuffer = pBuffer;
        if (msgsnd(pSfCodaj12Implement->sInputMessageQueue, (void *)&data, sizeof(data) - sizeof(data.msg_type), 0) == -1)
        {
            LOG(SF_LOG_ERR, "msgsnd failed\n");
        }
    }
    if (bSendNULL)
    {
        data.pBuffer = NULL;
        if (msgsnd(pSfCodaj12Implement->sInputMessageQueue, (void *)&data, sizeof(data) - sizeof(data.msg_type), 0) == -1)
        {
            LOG(SF_LOG_ERR, "msgsnd failed\n");
        }
    }
EXIT:
    FunctionOut();

    return ret;
}

int MCAIndex = 0;
int dataCount = 0;

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
    // if (pSfCodaj12Implement->functions->Queue_Enqueue(&pSfCodaj12Implement->decodeBufferQueue, (void *)pBuffer->pBuffer) == FALSE)
    // {
    //     return OMX_ErrorInsufficientResources;
    // }
    Message data,outputData;
    data.msg_type = 1;
    data.msg_flag = 0;
    data.pBuffer = pBuffer;

    // TODO start:
    // Temporary store one buff here to prevent the buff from being refill too soon.
    // It is a workaround for issue and should be fixed further.
    MCAIndex = (dataCount)%MCA_MAX_INDEX;
    if(dataCount++ < MCA_MAX_INDEX)
    {
        pSfCodaj12Implement->mesCacheArr[MCAIndex] = data;
        FunctionOut();
        return ret;
    }
    else
    {
        outputData = pSfCodaj12Implement->mesCacheArr[MCAIndex];
        pSfCodaj12Implement->mesCacheArr[MCAIndex] = data;
    }
    // TODO end

    LOG(SF_LOG_DEBUG, "Send to message queue\r\n");
    if (msgsnd(pSfCodaj12Implement->sOutputMessageQueue, (void *)&outputData, sizeof(data) - sizeof(data.msg_type), 0) == -1)
    {
        LOG(SF_LOG_ERR, "msgsnd failed\n");
    }
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
    SF_CODAJ12_IMPLEMEMT *pSfCodaj12Implement = pSfOMXComponent->componentImpl;
    FunctionIn();

    if (hComponent == NULL)
    {
        ret = OMX_ErrorBadParameter;
        goto EXIT;
    }
    if (AttachOutputBuffer(pSfOMXComponent, pBuffer, nSizeBytes) == OMX_FALSE)
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
    LOG(SF_LOG_DEBUG, "pBuffer address = %p, nOutBufIndex = %d\r\n", temp_bufferHeader->pBuffer, (OMX_U64)temp_bufferHeader->pOutputPortPrivate);

    pSfCodaj12Implement->allocBufFlag = OMX_FALSE;
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
    SF_CODAJ12_IMPLEMEMT *pSfCodaj12Implement = pSfOMXComponent->componentImpl;

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
    if (nPortIndex == 1)
    {
        temp_bufferHeader->pBuffer = AllocateOutputBuffer(pSfOMXComponent, nSizeBytes);
        temp_bufferHeader->pOutputPortPrivate = (OMX_PTR)nOutBufIndex;
        nOutBufIndex ++;
        pSfCodaj12Implement->allocBufFlag = OMX_TRUE;
        // temp_bufferHeader->nFilledLen = nSizeBytes;
    }
    else if (nPortIndex == 0)
    {
        jpu_buffer_t *vbStream = &pSfCodaj12Implement->vbStream;
        vbStream->size = (nSizeBytes == 0) ? STREAM_BUF_SIZE : nSizeBytes;
        vbStream->size = (vbStream->size + 1023) & ~1023; // ceil128(size)
        if (pSfCodaj12Implement->functions->jdi_allocate_dma_memory(vbStream) < 0)
        {
            LOG(SF_LOG_ERR, "fail to allocate bitstream buffer\r\n");
            return OMX_ErrorInsufficientResources;
        }
        temp_bufferHeader->pBuffer = (OMX_U8 *)vbStream->virt_addr;
        temp_bufferHeader->pInputPortPrivate = (void *)vbStream->phys_addr;
    }

    if (temp_bufferHeader->pBuffer == NULL)
    {
        free(temp_bufferHeader);
        LOG(SF_LOG_ERR, "malloc fail\r\n");
        return OMX_ErrorInsufficientResources;
    }
    *ppBuffer = temp_bufferHeader;
    LOG(SF_LOG_INFO, "nPortIndex = %d, header = %p, pBuffer address = %p, nFilledLen = %d, nSizeBytes = %d\r\n",
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

    LOG(SF_LOG_INFO, "Get parameter on index %X\r\n", nParamIndex);
    switch ((OMX_U32)nParamIndex)
    {
    case OMX_IndexParamImageInit:
    case OMX_IndexParamVideoInit:
    {
        OMX_PORT_PARAM_TYPE *portParam = (OMX_PORT_PARAM_TYPE *)ComponentParameterStructure;
        portParam->nPorts = NUM_OF_PORTS;
        portParam->nStartPortNumber = OMX_INPUT_PORT_INDEX;
        break;
    }

    case OMX_IndexParamImagePortFormat:
    {
        OMX_IMAGE_PARAM_PORTFORMATTYPE *portFormat = (OMX_IMAGE_PARAM_PORTFORMATTYPE *)ComponentParameterStructure;
        OMX_U32 index = portFormat->nIndex;
        switch (index)
        {
        case OMX_INPUT_PORT_INDEX:
            portFormat->eCompressionFormat = OMX_VIDEO_CodingUnused;
            portFormat->eColorFormat = OMX_COLOR_FormatYUV420SemiPlanar;
        default:
            if (index > 0)
            {
                ret = OMX_ErrorNoMore;
            }
        }
        break;
    }

    case OMX_IndexParamPortDefinition:
    {
        OMX_PARAM_PORTDEFINITIONTYPE *pPortDefinition = (OMX_PARAM_PORTDEFINITIONTYPE *)ComponentParameterStructure;
        OMX_U32 portIndex = pPortDefinition->nPortIndex;
        OMX_PARAM_PORTDEFINITIONTYPE *pSrcDefinition = &pSfOMXComponent->portDefinition[portIndex];

        LOG(SF_LOG_DEBUG, "Get parameter port %X\r\n",portIndex);
        LOG_APPEND(SF_LOG_DEBUG, "width = %d, height = %d\r\n", pSrcDefinition->format.video.nFrameWidth, pSrcDefinition->format.video.nFrameHeight);
        LOG_APPEND(SF_LOG_DEBUG, "eColorFormat = %d\r\n", pSrcDefinition->format.video.eColorFormat);
        LOG_APPEND(SF_LOG_DEBUG, "xFramerate = %d\r\n", pSrcDefinition->format.video.xFramerate);
        LOG_APPEND(SF_LOG_DEBUG, "bufferSize = %d\r\n",pSrcDefinition->nBufferSize);
        LOG_APPEND(SF_LOG_DEBUG, "Buffer count = %d\r\n", pSrcDefinition->nBufferCountActual);

        memcpy(pPortDefinition, pSrcDefinition, pPortDefinition->nSize);
        break;
    }

    case OMX_IndexParamVideoPortFormat:
    {
        OMX_VIDEO_PARAM_PORTFORMATTYPE *portFormat = (OMX_VIDEO_PARAM_PORTFORMATTYPE *)ComponentParameterStructure;
        OMX_U32 index = portFormat->nIndex;
        portFormat->eCompressionFormat = OMX_VIDEO_CodingUnused;
        portFormat->xFramerate = 30;
        LOG(SF_LOG_DEBUG, "Get video format at port %d index %d\r\n",portFormat->nPortIndex, index);
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
    default:
    {
        ret = OMX_ErrorNotImplemented;
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
            ret = OMX_ErrorNoMore;
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
    jpu_buffer_t *vbStream = &pSfCodaj12Implement->vbStream;

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

    pSfCodaj12Implement->functions->FreeFrameBuffer(pSfCodaj12Implement->instIdx);

    pSfCodaj12Implement->functions->jdi_free_dma_memory(vbStream);

    pSfCodaj12Implement->functions->JPU_DeInit();
    FunctionOut();
    return OMX_ErrorHardware;
}


static OMX_BOOL FillBufferDone(SF_OMX_COMPONENT *pSfOMXComponent, OMX_BUFFERHEADERTYPE *pBuffer)
{
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
    pSfOMXComponent->callbacks->FillBufferDone(pSfOMXComponent->pOMXComponent, pSfOMXComponent->pAppData, pBuffer);
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
        LOG(SF_LOG_ERR, "msgrcv failed ret = %x\r\n", ret);
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
    //TODO: input/output Buffer done
    LOG(SF_LOG_DEBUG, "EmptyBufferDone IN\r\n");
    pSfOMXComponent->callbacks->EmptyBufferDone(pSfOMXComponent->pOMXComponent, pSfOMXComponent->pAppData, pOMXBuffer);
    LOG(SF_LOG_DEBUG, "EmptyBufferDone OUT\r\n");

    FunctionOut();
    return OMX_TRUE;
}

static OMX_U32 FeedData(SF_OMX_COMPONENT *pSfOMXComponent)
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
    // if (pOMXBuffer == NULL || data.msg_flag == OMX_BUFFERFLAG_EOS)
    // {
    //     return 0;
    // }

    if (pOMXBuffer != NULL)
    {
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
    // if (pOMXBuffer == NULL || data.msg_flag == OMX_BUFFERFLAG_EOS)
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
    jpu_buffer_t *vbStream = &pSfCodaj12Implement->vbStream;
    JpgDecInitialInfo *initialInfo = &pSfCodaj12Implement->initialInfo;

    JpgDecOpenParam *decOP = &pSfCodaj12Implement->decOP;
    DecConfigParam *decConfig = pSfCodaj12Implement->config;

    JpgDecHandle handle;
    Int32 frameIdx = 0;
    JpgDecParam decParam = {0};
    JpgDecOutputInfo outputInfo = {0};
    Uint32 framebufWidth = 0, framebufHeight = 0;
    Uint32 decodingWidth, decodingHeight;
    Uint32 displayWidth, displayHeight;
    Int32 int_reason = 0;
    Int32 instIdx;
    Uint32 temp;
    JpgRet ret = JPG_RET_SUCCESS;

    FunctionIn();
    prctl(PR_SET_NAME, pSfOMXComponent->componentName);
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
        OMX_U32 size = FeedData(pSfOMXComponent);
        if (size <= 0)
        {
            LOG(SF_LOG_INFO, "FeedData = %d, end thread\r\n", size);
            pSfCodaj12Implement->bThreadRunning = OMX_FALSE;
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
            return;
        }
    }

    //TODO:Set output info
    OMX_PARAM_PORTDEFINITIONTYPE *pPortDefinition = &pSfOMXComponent->portDefinition[OMX_OUTPUT_PORT_INDEX];
    LOG(SF_LOG_DEBUG, "picWidth = %d, picHeight = %d, output Width = %d, output Height = %d\r\n", initialInfo->picWidth, initialInfo->picHeight,displayWidth, displayHeight);
    pPortDefinition->format.video.nFrameWidth = displayWidth;
    pPortDefinition->format.video.nFrameHeight = displayHeight;
    pPortDefinition->format.video.nStride = displayWidth;
    pPortDefinition->format.video.nSliceHeight = displayHeight;
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
    OMX_U32 nTimeout = 100;
    OMX_U32 nBufferNumber = pSfOMXComponent->portDefinition[OMX_OUTPUT_PORT_INDEX].nBufferCountActual;
    while (OMX_TRUE)
    {
        // LOG(SF_LOG_INFO, "GetFrameBufferCount addr = %p\r\n", pSfCodaj12Implement->functions->GetFrameBufferCount);
        nBufferNumber = pSfOMXComponent->portDefinition[OMX_OUTPUT_PORT_INDEX].nBufferCountActual;
        OMX_U32 number = pSfCodaj12Implement->functions->GetFrameBufferCount(instIdx);
        OMX_S32 oldNumber = -1;
        if (number < nBufferNumber)
        {
            if (oldNumber < number)
            {
                LOG(SF_LOG_INFO, "Wait for output buffer %d/%d\r\n", number, initialInfo->minFrameBufferCount);
                oldNumber = number;
            }
            usleep(50 * 1000);
            nTimeout --;
            if (nTimeout < 0)
            {
                LOG(SF_LOG_ERR, "No buffers found on output port\r\n");
                LOG(SF_LOG_DEBUG, "OMX_EventBufferFlag IN\r\n");
                pSfOMXComponent->callbacks->EventHandler(pSfOMXComponent->pOMXComponent, pSfOMXComponent->pAppData, OMX_EventBufferFlag,
                                             1, 1, NULL);
                LOG(SF_LOG_DEBUG, "OMX_EventBufferFlag OUT\r\n");
                return;
            }
        }
        else
        {
            break;
        }
    }

    FRAME_BUF *pFrame = pSfCodaj12Implement->frame;
    LOG(SF_LOG_INFO, "Update and regist %d FrameBuffers\r\n", nBufferNumber);
    LOG(SF_LOG_DEBUG, "%20s%20s%20s%20s\r\n", "virtAddr", "Y", "CB", "CR");
    for (int i = 0; i < nBufferNumber; i ++)
    {
        LOG_APPEND(SF_LOG_DEBUG, "%20lX%20lX%20lX%20lX\r\n", pFrame[i].vbY.virt_addr,
            pFrame[i].vbY.phys_addr, pFrame[i].vbCb.phys_addr, pFrame[i].vbCr.phys_addr);
    }

    // ffmpeg: may register frame buffer append
    if (pSfCodaj12Implement->functions->JPU_GetFrameBufPool(pSfCodaj12Implement->handle) == NULL)
    {
        JpgRet ret = JPG_RET_SUCCESS;
        pSfCodaj12Implement->functions->UpdateFrameBuffers(instIdx, nBufferNumber, pFrame);
        ret = pSfCodaj12Implement->functions->JPU_DecRegisterFrameBuffer(pSfCodaj12Implement->handle, pSfCodaj12Implement->frameBuf,
                                        nBufferNumber, pSfCodaj12Implement->frameBuf[0].stride);
        if (ret != JPG_RET_SUCCESS)
        {
            LOG(SF_LOG_ERR, "JPU_DecRegisterFrameBuffer fail ret = %d\r\n", ret);
            pSfCodaj12Implement->bThreadRunning = OMX_FALSE;
        }
    }

    pSfCodaj12Implement->functions->JPU_DecGiveCommand(handle, SET_JPG_SCALE_HOR, &(decConfig->iHorScaleMode));
    pSfCodaj12Implement->functions->JPU_DecGiveCommand(handle, SET_JPG_SCALE_VER, &(decConfig->iVerScaleMode));

    /* LOG HEADER */
    LOG(SF_LOG_INFO, "I   F    FB_INDEX  FRAME_START  ECS_START  CONSUME   RD_PTR   WR_PTR      CYCLE\n");
    LOG(SF_LOG_INFO, "-------------------------------------------------------------------------------\n");

    while (pSfCodaj12Implement->bThreadRunning)
    {
        OMX_BUFFERHEADERTYPE *pBuffer;

        if (WaitForOutputBufferReady(pSfOMXComponent,&pBuffer) != OMX_ErrorNone)
        {
            continue;
        }

        LOG(SF_LOG_INFO, "pBuffer->pOutputPortPrivate:%d\r\n", (OMX_U64)(pBuffer->pOutputPortPrivate));
        ret = pSfCodaj12Implement->functions->JPU_DecStartOneFrameBySerialNum(handle, &decParam,(OMX_U64)(pBuffer->pOutputPortPrivate));
        if (ret != JPG_RET_SUCCESS && ret != JPG_RET_EOS)
        {
            if (ret == JPG_RET_BIT_EMPTY)
            {
                LOG(SF_LOG_INFO, "BITSTREAM NOT ENOUGH.............\n");
                OMX_U32 size = FeedData(pSfOMXComponent);
                if (size <= 0)
                {
                    LOG(SF_LOG_INFO, "FeedData = %d, end thread\r\n", size);
                    pSfCodaj12Implement->bThreadRunning = OMX_FALSE;
                }
                continue;
            }

            LOG(SF_LOG_ERR, "JPU_DecStartOneFrameBySerialNum failed Error code is 0x%x \n", ret);
            return;
        }
        if (ret == JPG_RET_EOS)
        {
            pSfCodaj12Implement->functions->JPU_DecGetOutputInfo(handle, &outputInfo);
            break;
        }

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
        //LOG(SF_LOG_INFO, "\t<->INSTANCE #%d JPU_WaitInterrupt\n", handle->instIndex);
        EmptyBufferDone(pSfOMXComponent);
        if ((ret = pSfCodaj12Implement->functions->JPU_DecGetOutputInfo(handle, &outputInfo)) != JPG_RET_SUCCESS)
        {
            LOG(SF_LOG_ERR, "JPU_DecGetOutputInfo failed Error code is 0x%x \n", ret);
            return;
        }

        pBuffer->nFilledLen = pSfCodaj12Implement->functions->SaveYuvImageHelper
                        (pBuffer->pBuffer, &pSfCodaj12Implement->frameBuf[(OMX_U64)(pBuffer->pOutputPortPrivate)],
                        decOP->chromaInterleave, decOP->packedFormat, displayWidth, displayHeight, initialInfo->bitDepth);

        LOG(SF_LOG_DEBUG, "decPicSize = [%d %d], pBuffer = %p\r\n",
            outputInfo.decPicWidth, outputInfo.decPicHeight, pBuffer->pBuffer);
        // AdjustFrame(pBuffer, outputInfo.decPicWidth, outputInfo.decPicHeight, nDestStride, nDestHeight);
        FillBufferDone(pSfOMXComponent, pBuffer);
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
        OMX_U32 size = FeedData(pSfOMXComponent);
        if (size <= 0)
        {
            LOG(SF_LOG_INFO, "FeedData = %d, end thread\r\n", size);
            pSfCodaj12Implement->bThreadRunning = OMX_FALSE;
        }
    }
    LOG(SF_LOG_DEBUG, "OMX_EventBufferFlag IN\r\n");
    pSfOMXComponent->callbacks->EventHandler(pSfOMXComponent->pOMXComponent, pSfOMXComponent->pAppData, OMX_EventBufferFlag,
                                    1, 1, NULL);
    LOG(SF_LOG_DEBUG, "OMX_EventBufferFlag OUT\r\n");
    CodaJ12FlushBuffer(pSfOMXComponent, OMX_INPUT_PORT_INDEX);
    CodaJ12FlushBuffer(pSfOMXComponent, OMX_OUTPUT_PORT_INDEX);
    pSfCodaj12Implement->currentState = OMX_StateIdle;
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
        LOG(SF_LOG_INFO, "OMX dest state = %X, current state = %X\r\n", nParam, pSfCodaj12Implement->currentState);
        switch (nParam)
        {
        case OMX_StateLoaded:
            break;
        case OMX_StateIdle:
            switch (pSfCodaj12Implement->currentState)
            {
                case OMX_StateExecuting:
                {
                    Message data;
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
                    pSfCodaj12Implement->currentState = OMX_StateIdle;
                    break;
                }
                case OMX_StateIdle:
                    break;
                case OMX_StateLoaded:
                    // InitDecoder(pSfOMXComponent);
                    pSfCodaj12Implement->currentState = OMX_StateIdle;
                    break;
                default:
                    LOG(SF_LOG_ERR, "Can't go to state %d from %d\r\n",pSfCodaj12Implement->currentState, nParam);
                    return OMX_ErrorBadParameter;
                    break;
            }
            break;
        case OMX_StateExecuting:
            switch (pSfCodaj12Implement->currentState)
            {
            case OMX_StateIdle:
                pSfCodaj12Implement->bThreadRunning = OMX_TRUE;
                CreateThread(&pSfCodaj12Implement->pProcessThread, ProcessThread, (void *)pSfOMXComponent);
                pSfCodaj12Implement->currentState = OMX_StateExecuting;
                break;
            case OMX_StateExecuting:
                break;
            case OMX_StateLoaded:
            default:
                LOG(SF_LOG_ERR, "Can't go to state %d from %d\r\n",pSfCodaj12Implement->currentState, nParam);
                return OMX_ErrorBadParameter;
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
        CodaJ12FlushBuffer(pSfOMXComponent, nPort);
        pSfOMXComponent->callbacks->EventHandler(pSfOMXComponent->pOMXComponent, pSfOMXComponent->pAppData,
                                                 OMX_EventCmdComplete, OMX_CommandFlush, nParam, NULL);
    }
    case OMX_CommandPortDisable:
        pSfOMXComponent->callbacks->EventHandler(pSfOMXComponent->pOMXComponent, pSfOMXComponent->pAppData,
                                                 OMX_EventCmdComplete, OMX_CommandPortDisable, nParam, NULL);
        break;
    case OMX_CommandPortEnable:
        pSfOMXComponent->callbacks->EventHandler(pSfOMXComponent->pOMXComponent, pSfOMXComponent->pAppData,
                                                 OMX_EventCmdComplete, OMX_CommandPortEnable, nParam, NULL);
    default:
        break;
    }
    //TODO
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
    SF_CODAJ12_IMPLEMEMT *pSfCodaj12Implement = pSfOMXComponent->componentImpl;
    FunctionIn();
    *pState = pSfCodaj12Implement->currentState;
    FunctionOut();
    return ret;
}

static OMX_ERRORTYPE SF_OMX_FreeBuffer(
    OMX_IN OMX_HANDLETYPE hComponent,
    OMX_IN OMX_U32 nPortIndex,
    OMX_IN OMX_BUFFERHEADERTYPE *pBufferHdr)
{
    OMX_ERRORTYPE ret = OMX_ErrorNone;
    FunctionIn();
    //TODO
    FunctionOut();
    return ret;
}


static OMX_ERRORTYPE SF_OMX_ComponentConstructor(SF_OMX_COMPONENT *pSfOMXComponent)
{
    OMX_ERRORTYPE ret = OMX_ErrorNone;
    SF_CODAJ12_IMPLEMEMT *pSfCodaj12Implement;
    FunctionIn();

    ret = InitMjpegStructorCommon(pSfOMXComponent);
    if (ret != OMX_ErrorNone)
    {
        goto EXIT;
    }

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
    pSfCodaj12Implement = pSfOMXComponent->componentImpl;
    pSfCodaj12Implement->currentState = OMX_StateLoaded;
    InitDecoder(pSfOMXComponent);
EXIT:
    FunctionOut();

    return ret;
}

static OMX_ERRORTYPE SF_OMX_ComponentClear(SF_OMX_COMPONENT *pSfOMXComponent)
{
    OMX_ERRORTYPE ret = OMX_ErrorNone;
    SF_CODAJ12_IMPLEMEMT *pSfCodaj12Implement = pSfOMXComponent->componentImpl;
    jpu_buffer_t *vbStream = &pSfCodaj12Implement->vbStream;
    JpgDecHandle handle = pSfCodaj12Implement->handle;
    FunctionIn();

    if (pSfCodaj12Implement->functions->JPU_DecClose(handle) != JPG_RET_SUCCESS)
    {
        LOG(SF_LOG_ERR, "\nDec End\r\n");
        ret = OMX_ErrorHardware;
    }
    pSfCodaj12Implement->functions->BitstreamFeeder_Destroy(pSfCodaj12Implement->feeder);


    if(pSfCodaj12Implement->allocBufFlag == OMX_TRUE)
    {
        pSfCodaj12Implement->functions->FreeFrameBuffer(pSfCodaj12Implement->instIdx);
    }
    pSfCodaj12Implement->functions->jdi_free_dma_memory(vbStream);
    pSfCodaj12Implement->functions->JPU_DeInit();
    // TODO
    // MjpegClearCommon(hComponent);

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
