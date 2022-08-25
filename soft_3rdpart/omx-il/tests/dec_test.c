// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2021 StarFive Technology Co., Ltd.
 */
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <getopt.h>
#include <fcntl.h>
#include "libavformat/avformat.h"
#include <OMX_Core.h>
#include <OMX_Component.h>
#include <OMX_Video.h>
#include <OMX_IndexExt.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define OMX_INIT_STRUCTURE(a)         \
    memset(&(a), 0, sizeof(a));       \
    (a).nSize = sizeof(a);            \
    (a).nVersion.nVersion = 1;        \
    (a).nVersion.s.nVersionMajor = 1; \
    (a).nVersion.s.nVersionMinor = 1; \
    (a).nVersion.s.nRevision = 1;     \
    (a).nVersion.s.nStep = 1

typedef struct Message
{
    long msg_type;
    OMX_S32 msg_flag;
    OMX_BUFFERHEADERTYPE *pBuffer;
} Message;

typedef struct DecodeTestContext
{
    OMX_HANDLETYPE hComponentDecoder;
    char sOutputFilePath[64];
    char sInputFilePath[64];
    char sOutputFormat[64];
    OMX_U32 ScaleWidth;
    OMX_U32 ScaleHeight;
    OMX_BUFFERHEADERTYPE *pInputBufferArray[64];
    OMX_BUFFERHEADERTYPE *pOutputBufferArray[64];
    AVFormatContext *avContext;
    int msgid;
} DecodeTestContext;
DecodeTestContext *decodeTestContext;
static OMX_S32 FillInputBuffer(DecodeTestContext *decodeTestContext, OMX_BUFFERHEADERTYPE *pInputBuffer);

static OMX_ERRORTYPE event_handler(
    OMX_HANDLETYPE hComponent,
    OMX_PTR pAppData,
    OMX_EVENTTYPE eEvent,
    OMX_U32 nData1,
    OMX_U32 nData2,
    OMX_PTR pEventData)
{
    DecodeTestContext *pDecodeTestContext = (DecodeTestContext *)pAppData;

    switch (eEvent)
    {
    case OMX_EventPortSettingsChanged:
    {
        OMX_PARAM_PORTDEFINITIONTYPE pOutputPortDefinition;
        OMX_INIT_STRUCTURE(pOutputPortDefinition);
        pOutputPortDefinition.nPortIndex = 1;
        OMX_GetParameter(pDecodeTestContext->hComponentDecoder, OMX_IndexParamPortDefinition, &pOutputPortDefinition);
        OMX_U32 nOutputBufferSize = pOutputPortDefinition.nBufferSize;
        OMX_U32 nOutputBufferCount = pOutputPortDefinition.nBufferCountMin;
        for (int i = 0; i < nOutputBufferCount; i++)
        {
            OMX_BUFFERHEADERTYPE *pBuffer = NULL;
            OMX_AllocateBuffer(hComponent, &pBuffer, 1, NULL, nOutputBufferSize);
            pDecodeTestContext->pOutputBufferArray[i] = pBuffer;
            OMX_FillThisBuffer(hComponent, pBuffer);
        }
    }
    break;
    case OMX_EventBufferFlag:
    {
        Message data;
        data.msg_type = 1;
        data.msg_flag = -1;
        if (msgsnd(pDecodeTestContext->msgid, (void *)&data, sizeof(data) - sizeof(data.msg_type), 0) == -1)
        {
            fprintf(stderr, "msgsnd failed\n");
        }
    }
    break;
    default:
        break;
    }
    return OMX_ErrorNone;
}

static void help()
{
    printf("video_dec_test - omx video hardware decode unit test case\r\n\r\n");
    printf("Usage:\r\n\r\n");
    printf("./video_dec_test -i <input file>      input file\r\n");
    printf("                 -o <output file>     output file\r\n");
    printf("                 -f <format>          i420/nv12/nv21\r\n");
    printf("                 --scaleW=<width>     (optional) scale width down. ceil8(width/8) <= scaledW <= width\r\n");
    printf("                 --scaleH=<heitht>    (optional) scale height down, ceil8(height/8) <= scaledH <= height\r\n\r\n");
    printf("./video_dec_test --help: show this message\r\n");
}

static OMX_ERRORTYPE fill_output_buffer_done_handler(
    OMX_HANDLETYPE hComponent,
    OMX_PTR pAppData,
    OMX_BUFFERHEADERTYPE *pBuffer)
{
    DecodeTestContext *pDecodeTestContext = (DecodeTestContext *)pAppData;

    Message data;
    data.msg_type = 1;
    if ((pBuffer->nFlags) & (OMX_BUFFERFLAG_EOS == OMX_BUFFERFLAG_EOS))
    {
        data.msg_flag = -1;
    }
    else
    {
        data.msg_flag = 1;
        data.pBuffer = pBuffer;
    }
    if (msgsnd(pDecodeTestContext->msgid, (void *)&data, sizeof(data) - sizeof(data.msg_type), 0) == -1)
    {
        fprintf(stderr, "msgsnd failed\n");
    }

    return OMX_ErrorNone;
}

static OMX_ERRORTYPE empty_buffer_done_handler(
    OMX_HANDLETYPE hComponent,
    OMX_PTR pAppData,
    OMX_BUFFERHEADERTYPE *pBuffer)
{
    DecodeTestContext *pDecodeTestContext = (DecodeTestContext *)pAppData;
    Message data;
    data.msg_type = 1;
    data.msg_flag = 0;
    data.pBuffer = pBuffer;
    if (msgsnd(pDecodeTestContext->msgid, (void *)&data, sizeof(data) - sizeof(data.msg_type), 0) == -1)
    {
        fprintf(stderr, "msgsnd failed\n");
    }
    return OMX_ErrorNone;
}

static void signal_handle(int sig)
{
    printf("[%s,%d]: receive sig=%d \n", __FUNCTION__, __LINE__, sig);
    Message data;
    data.msg_type = 1;
    data.msg_flag = -1;
    if (msgsnd(decodeTestContext->msgid, (void *)&data, sizeof(data) - sizeof(data.msg_type), 0) == -1)
    {
        fprintf(stderr, "msgsnd failed\n");
    }
}

static OMX_S32 FillInputBuffer(DecodeTestContext *decodeTestContext, OMX_BUFFERHEADERTYPE *pInputBuffer)
{
    AVFormatContext *avFormatContext = decodeTestContext->avContext;
    AVPacket *avpacket;
    OMX_S32 error;
    avpacket = av_packet_alloc();
    error = av_read_frame(avFormatContext, avpacket);
    if (error < 0)
    {
        if (error == AVERROR_EOF || avFormatContext->pb->eof_reached == OMX_TRUE)
        {
            pInputBuffer->nFlags = 0x1;
            pInputBuffer->nFilledLen = 0;
            return 0;
        }
        else
        {
            printf("%s:%d failed to av_read_frame, error: %s\n",
                    __FUNCTION__, __LINE__, av_err2str(error));
            return 0;
        }
    }
    pInputBuffer->nFlags = 0x10;
    pInputBuffer->nFilledLen = avpacket->size;
    if(!pInputBuffer->pBuffer || !avpacket->data)
        return 0;
    memcpy(pInputBuffer->pBuffer, avpacket->data, avpacket->size);
    return avpacket->size;
}

int main(int argc, char **argv)
{
    printf("=============================\r\n");
    OMX_S32 error;
    FILE *fb;
    decodeTestContext = malloc(sizeof(DecodeTestContext));
    memset(decodeTestContext, 0, sizeof(DecodeTestContext));

    OMX_S32 msgid = -1;
    msgid = msgget(IPC_PRIVATE, 0666 | IPC_CREAT);
    if (msgid < 0)
    {
        perror("get ipc_id error");
        return -1;
    }
    decodeTestContext->msgid = msgid;
    struct option longOpt[] = {
        {"output", required_argument, NULL, 'o'},
        {"input", required_argument, NULL, 'i'},
        {"format", required_argument, NULL, 'f'},
        {"scaleW", required_argument, NULL, 'w'},
        {"scaleH", required_argument, NULL, 'h'},
        {"help", no_argument, NULL, '0'},
        {NULL, no_argument, NULL, 0},
    };
    char *shortOpt = "i:o:f:w:h:";
    OMX_U32 c;
    OMX_S32 l;

    if (argc == 0)
    {
        help();
        return -1;
    }

    while ((c = getopt_long(argc, argv, shortOpt, longOpt, (int *)&l)) != -1)
    {
        switch (c)
        {
        case 'i':
            printf("input: %s\r\n", optarg);
            if (access(optarg, R_OK) != -1)
            {
                memcpy(decodeTestContext->sInputFilePath, optarg, strlen(optarg));
            }
            else
            {
                printf("input file not exist!\r\n");
                return -1;
            }
            break;
        case 'o':
            printf("output: %s\r\n", optarg);
            memcpy(decodeTestContext->sOutputFilePath, optarg, strlen(optarg));
            break;
        case 'f':
            printf("format: %s\r\n", optarg);
            memcpy(decodeTestContext->sOutputFormat, optarg, strlen(optarg));
            break;
        case 'w':
            printf("ScaleWidth: %s\r\n", optarg);
            decodeTestContext->ScaleWidth = atoi(optarg);
            break;
        case 'h':
            printf("ScaleHeight: %s\r\n", optarg);
            decodeTestContext->ScaleHeight = atoi(optarg);
            break;
        case '0':
        default:
            help();
            return -1;
        }
    }

    if (decodeTestContext->sInputFilePath == NULL || decodeTestContext->sOutputFilePath == NULL)
    {
        help();
        return -1;
    }
    /*ffmpeg init*/
    printf("init ffmpeg\r\n");
    AVFormatContext *avContext = NULL;
    AVCodecParameters *codecParameters = NULL;
    AVInputFormat *fmt = NULL;
    OMX_S32 videoIndex;
    if ((avContext = avformat_alloc_context()) == NULL)
    {
        printf("avformat_alloc_context fail\r\n");
        return -1;
    }
    avContext->flags |= AV_CODEC_FLAG_TRUNCATED;

    printf("avformat_open_input\r\n");
    if ((error = avformat_open_input(&avContext, decodeTestContext->sInputFilePath, fmt, NULL)))
    {
        printf("%s:%d failed to av_open_input_file error(%s), %s\n",
               __FILE__, __LINE__, av_err2str(error), decodeTestContext->sInputFilePath);
        return -1;
    }

    printf("avformat_find_stream_info\r\n");
    if ((error = avformat_find_stream_info(avContext, NULL)) < 0)
    {
        printf("%s:%d failed to avformat_find_stream_info. error(%s)\n",
               __FUNCTION__, __LINE__, av_err2str(error));
        return -1;
    }

    printf("av_find_best_stream\r\n");
    videoIndex = av_find_best_stream(avContext, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
    if (videoIndex < 0)
    {
        printf("%s:%d failed to av_find_best_stream.\n", __FUNCTION__, __LINE__);
        return -1;
    }
    printf("video index = %ld\r\n", videoIndex);
    decodeTestContext->avContext = avContext;
    /*get video info*/
    codecParameters = avContext->streams[videoIndex]->codecpar;
    printf("codec_id = %d, width = %d, height = %d\r\n", (int)codecParameters->codec_id,
           codecParameters->width, codecParameters->height);

    /*omx init*/
    OMX_HANDLETYPE hComponentDecoder  = NULL;
    OMX_CALLBACKTYPE callbacks;
    int ret = OMX_ErrorNone;
    signal(SIGINT, signal_handle);
    printf("init omx\r\n");
    ret = OMX_Init();
    if (ret != OMX_ErrorNone)
    {
        printf("[%s,%d]: run OMX_Init failed. ret is %d \n", __FUNCTION__, __LINE__, ret);
        return 1;
    }

    callbacks.EventHandler = event_handler;
    callbacks.FillBufferDone = fill_output_buffer_done_handler;
    callbacks.EmptyBufferDone = empty_buffer_done_handler;
    printf("get handle\r\n");
    if (codecParameters->codec_id == AV_CODEC_ID_H264)
    {
        OMX_GetHandle(&hComponentDecoder, "OMX.sf.video_decoder.avc", decodeTestContext, &callbacks);
    }
    else if (codecParameters->codec_id == AV_CODEC_ID_HEVC)
    {
        OMX_GetHandle(&hComponentDecoder, "OMX.sf.video_decoder.hevc", decodeTestContext, &callbacks);
    }
    if (hComponentDecoder == NULL)
    {
        printf("could not get handle\r\n");
        return 0;
    }
    decodeTestContext->hComponentDecoder = hComponentDecoder;
     
    OMX_PARAM_PORTDEFINITIONTYPE pOutputPortDefinition;
    OMX_INIT_STRUCTURE(pOutputPortDefinition);
    pOutputPortDefinition.nPortIndex = 1;
    OMX_GetParameter(decodeTestContext->hComponentDecoder, OMX_IndexParamPortDefinition, &pOutputPortDefinition);
    pOutputPortDefinition.format.video.nFrameWidth
                = decodeTestContext->ScaleWidth? decodeTestContext->ScaleWidth:codecParameters->width;
    pOutputPortDefinition.format.video.nFrameHeight
                = decodeTestContext->ScaleHeight? decodeTestContext->ScaleHeight:codecParameters->height;
    if (strstr(decodeTestContext->sOutputFormat, "nv12") != NULL)
    {
        pOutputPortDefinition.format.video.eColorFormat = OMX_COLOR_FormatYUV420SemiPlanar;
    }
    else if (strstr(decodeTestContext->sOutputFormat, "nv21") != NULL)
    {
        pOutputPortDefinition.format.video.eColorFormat = OMX_COLOR_FormatYVU420SemiPlanar;
    }
    else if (strstr(decodeTestContext->sOutputFormat, "i420") != NULL)
    {
        pOutputPortDefinition.format.video.eColorFormat = OMX_COLOR_FormatYUV420Planar;
    }
    else
    {
        printf("Unsupported color format!\r\n");
        goto end;
    }
    OMX_SetParameter(hComponentDecoder, OMX_IndexParamPortDefinition, &pOutputPortDefinition);

    OMX_SendCommand(hComponentDecoder, OMX_CommandStateSet, OMX_StateIdle, NULL);

    OMX_PARAM_PORTDEFINITIONTYPE pInputPortDefinition;
    OMX_INIT_STRUCTURE(pInputPortDefinition);
    pInputPortDefinition.nPortIndex = 0;
    OMX_GetParameter(hComponentDecoder, OMX_IndexParamPortDefinition, &pInputPortDefinition);
    pInputPortDefinition.format.video.nFrameWidth = codecParameters->width;
    pInputPortDefinition.format.video.nFrameHeight = codecParameters->height;
    OMX_SetParameter(hComponentDecoder, OMX_IndexParamPortDefinition, &pInputPortDefinition);
    /*Alloc input buffer*/
    OMX_U32 nInputWidth = codecParameters->width;
    OMX_U32 nInputHeight = codecParameters->height;
    OMX_U32 nInputBufferSize = nInputWidth * nInputHeight * 2;
    OMX_U32 nInputBufferCount = pInputPortDefinition.nBufferCountActual;
    for (int i = 0; i < nInputBufferCount; i++)
    {
        OMX_BUFFERHEADERTYPE *pBuffer = NULL;
        OMX_AllocateBuffer(hComponentDecoder, &pBuffer, 0, NULL, nInputBufferSize);
        decodeTestContext->pInputBufferArray[i] = pBuffer;
        /*Fill Input Buffer*/
        FillInputBuffer(decodeTestContext, pBuffer);
        OMX_EmptyThisBuffer(hComponentDecoder, pBuffer);
    }

    fb = fopen(decodeTestContext->sOutputFilePath, "wb+");

    OMX_SendCommand(hComponentDecoder, OMX_CommandStateSet, OMX_StateExecuting, NULL);

    /*wait until decode finished*/
    Message data;
    while (OMX_TRUE)
    {
        if (msgrcv(msgid, (void *)&data, BUFSIZ, 0, 0) == -1)
        {
            fprintf(stderr, "msgrcv failed with errno: %d\n", errno);
            goto end;
        }
        switch (data.msg_flag)
        {
        case 0:
        {
            OMX_BUFFERHEADERTYPE *pBuffer = data.pBuffer;
            FillInputBuffer(decodeTestContext, pBuffer);
            OMX_EmptyThisBuffer(decodeTestContext->hComponentDecoder, pBuffer);
        }
        break;
        case 1:
        {
            OMX_BUFFERHEADERTYPE *pBuffer = data.pBuffer;
            fwrite(pBuffer->pBuffer, 1, pBuffer->nFilledLen, fb);
            if ((pBuffer->nFlags) & (OMX_BUFFERFLAG_EOS == OMX_BUFFERFLAG_EOS))
            {
                goto end;
            }
            else
            {
                OMX_FillThisBuffer(decodeTestContext->hComponentDecoder, pBuffer);
            }
        }
        break;
        case 2:
            break;
        default:
            goto end;
        }
    }

end:
    /*free resource*/
    fclose(fb);
    OMX_FreeHandle(hComponentDecoder);
    OMX_Deinit();
}
