// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2021 StarFive Technology Co., Ltd.
 */
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <getopt.h>
#include <fcntl.h>
#include <OMX_Core.h>
#include <OMX_Component.h>
#include <OMX_Video.h>
#include <OMX_IndexExt.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

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

typedef struct EncodeTestContext
{
    OMX_HANDLETYPE hComponentEncoder;
    char sOutputFilePath[64];
    char sInputFilePath[64];
    FILE *pInputFile;
    char sInputFormat[64];
    char sOutputFormat[64];
    OMX_U32 nFrameBufferSize;
    OMX_U32 nBitrate;
    OMX_BUFFERHEADERTYPE *pInputBufferArray[64];
    OMX_BUFFERHEADERTYPE *pOutputBufferArray[64];
    int msgid;
} EncodeTestContext;
EncodeTestContext *encodeTestContext;
static OMX_S32 FillInputBuffer(EncodeTestContext *encodeTestContext, OMX_BUFFERHEADERTYPE *pInputBuffer);

static OMX_ERRORTYPE event_handler(
    OMX_HANDLETYPE hComponent,
    OMX_PTR pAppData,
    OMX_EVENTTYPE eEvent,
    OMX_U32 nData1,
    OMX_U32 nData2,
    OMX_PTR pEventData)
{
    EncodeTestContext *pEncodeTestContext = (EncodeTestContext *)pAppData;

    switch (eEvent)
    {
    case OMX_EventPortSettingsChanged:
    {
        OMX_PARAM_PORTDEFINITIONTYPE pOutputPortDefinition;
        OMX_INIT_STRUCTURE(pOutputPortDefinition);
        pOutputPortDefinition.nPortIndex = 1;
        OMX_GetParameter(pEncodeTestContext->hComponentEncoder, OMX_IndexParamPortDefinition, &pOutputPortDefinition);
        OMX_U32 nOutputBufferSize = pOutputPortDefinition.nBufferSize;
        OMX_U32 nOutputBufferCount = pOutputPortDefinition.nBufferCountMin;
        for (int i = 0; i < nOutputBufferCount; i++)
        {
            OMX_BUFFERHEADERTYPE *pBuffer = NULL;
            OMX_AllocateBuffer(hComponent, &pBuffer, 1, NULL, nOutputBufferSize);
            pEncodeTestContext->pOutputBufferArray[i] = pBuffer;
            OMX_FillThisBuffer(hComponent, pBuffer);
        }
    }
    break;
    case OMX_EventBufferFlag:
    {
        Message data;
        data.msg_type = 1;
        data.msg_flag = -1;
        if (msgsnd(pEncodeTestContext->msgid, (void *)&data, sizeof(data) - sizeof(data.msg_type), 0) == -1)
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
    printf("./video_enc_test -i <input file> -o <output file> -w <width> -h <height> "
           "-c <yuv/nv12/nv21> -s <h264/h265> -b <bitrate>\r\n");
}

static OMX_ERRORTYPE fill_output_buffer_done_handler(
    OMX_HANDLETYPE hComponent,
    OMX_PTR pAppData,
    OMX_BUFFERHEADERTYPE *pBuffer)
{
    EncodeTestContext *pEncodeTestContext = (EncodeTestContext *)pAppData;

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
    if (msgsnd(pEncodeTestContext->msgid, (void *)&data, sizeof(data) - sizeof(data.msg_type), 0) == -1)
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
    EncodeTestContext *pEncodeTestContext = (EncodeTestContext *)pAppData;
    Message data;
    data.msg_type = 1;
    data.msg_flag = 0;
    data.pBuffer = pBuffer;
    if (msgsnd(pEncodeTestContext->msgid, (void *)&data, sizeof(data) - sizeof(data.msg_type), 0) == -1)
    {
        fprintf(stderr, "msgsnd failed\n");
    }
    return OMX_ErrorNone;
}

static void signal_handle(int sig)
{
    printf("[%s,%d]: receive sig=%d \n", __FUNCTION__, __LINE__, sig);

    OMX_FreeHandle(encodeTestContext->hComponentEncoder);
    OMX_Deinit();
    exit(0);
    // Message data;
    // data.msg_type = 1;
    // data.msg_flag = -1;
    // if (msgsnd(encodeTestContext->msgid, (void *)&data, sizeof(data) - sizeof(data.msg_type), 0) == -1)
    // {
    //     fprintf(stderr, "msgsnd failed\n");
    // }
}

static OMX_S32 FillInputBuffer(EncodeTestContext *encodeTestContext, OMX_BUFFERHEADERTYPE *pInputBuffer)
{
    FILE *fp = encodeTestContext->pInputFile;
    OMX_U32 size = encodeTestContext->nFrameBufferSize;
    OMX_U32 count;

    count = fread(pInputBuffer->pBuffer, 1, size, fp);
    if (count < size)
    {
        Message data;
        data.msg_type = 1;
        data.msg_flag = -1;
        pInputBuffer->nFlags = 0x1;
        pInputBuffer->nFilledLen = 0;
        count = 0;

        if (msgsnd(encodeTestContext->msgid, (void *)&data, sizeof(data) - sizeof(data.msg_type), 0) == -1)
        {
            fprintf(stderr, "msgsnd failed\n");
        }
    }
    else
    {
        pInputBuffer->nFlags = 0x10;
        pInputBuffer->nFilledLen = size;
    }

    return count;
}

int main(int argc, char **argv)
{
    FILE *fb;
    printf("=============================\r\n");
    encodeTestContext = malloc(sizeof(EncodeTestContext));
    memset(encodeTestContext, 0, sizeof(EncodeTestContext));

    OMX_S32 msgid = -1;
    msgid = msgget(IPC_PRIVATE, 0666 | IPC_CREAT);
    if (msgid < 0)
    {
        perror("get ipc_id error");
        return -1;
    }
    encodeTestContext->msgid = msgid;
    struct option longOpt[] = {
        {"output", required_argument, NULL, 'o'},
        {"input", required_argument, NULL, 'f'},
        {"color format", required_argument, NULL, 'c'},
        {"stream format", required_argument, NULL, 's'},
        {"bit rate", optional_argument, NULL, 'b'},
        {"width", required_argument, NULL, 'w'},
        {"height", required_argument, NULL, 'h'},
        {NULL, no_argument, NULL, 0},
    };
    char *shortOpt = "i:o:f:s:w:h:b:c:";
    OMX_U32 c;
    OMX_S32 l;

    if (argc == 0)
    {
        help();
        return 0;
    }
    OMX_U32 width = 0;
    OMX_U32 height = 0;
    while ((c = getopt_long(argc, argv, shortOpt, longOpt, (int *)&l)) != -1)
    {
        switch (c)
        {
        case 'i':
            printf("input: %s\r\n", optarg);
            if (access(optarg, R_OK) != -1)
            {
                memcpy(encodeTestContext->sInputFilePath, optarg, strlen(optarg));
                encodeTestContext->pInputFile = fopen(optarg, "r");
            }
            else
            {
                printf("input file not exist!\r\n");
                return -1;
            }
            break;
        case 'o':
            printf("output: %s\r\n", optarg);
            memcpy(encodeTestContext->sOutputFilePath, optarg, strlen(optarg));
            break;
        case 'c':
            printf("color format: %s\r\n", optarg);
            memcpy(encodeTestContext->sInputFormat, optarg, strlen(optarg));
            break;
        case 's':
            printf("stream format: %s\r\n", optarg);
            memcpy(encodeTestContext->sOutputFormat, optarg, strlen(optarg));
            break;
        case 'w':
            printf("width: %s\r\n", optarg);
            width = atoi(optarg);
            break;
        case 'h':
            printf("height: %s\r\n", optarg);
            height = atoi(optarg);
            break;
        case 'b':
            printf("bit rate: %s\r\n", optarg);
            encodeTestContext->nBitrate = atoi(optarg);
            break;
        default:
            help();
            return 0;
        }
    }

    encodeTestContext->nFrameBufferSize = width * height * 3 / 2;
    if (encodeTestContext->sInputFilePath == NULL ||
        encodeTestContext->sOutputFilePath == NULL ||
        encodeTestContext->nFrameBufferSize == 0 ||
        encodeTestContext->pInputFile == NULL)
    {
        help();
        return -1;
    }

    /*omx init*/
    OMX_HANDLETYPE hComponentEncoder = NULL;
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
    printf("get handle %s\r\n", encodeTestContext->sOutputFormat);
    if (strstr(encodeTestContext->sOutputFormat, "h264") != NULL)
    {
        OMX_GetHandle(&hComponentEncoder, "sf.enc.encoder.h264", encodeTestContext, &callbacks);
    }
    else if (strstr(encodeTestContext->sOutputFormat, "h265") != NULL)
    {
        OMX_GetHandle(&hComponentEncoder, "sf.enc.encoder.h265", encodeTestContext, &callbacks);
    }
    if (hComponentEncoder == NULL)
    {
        printf("could not get handle\r\n");
        return 0;
    }
    encodeTestContext->hComponentEncoder = hComponentEncoder;

    OMX_PARAM_PORTDEFINITIONTYPE pInputPortDefinition;
    OMX_INIT_STRUCTURE(pInputPortDefinition);
    pInputPortDefinition.nPortIndex = 0;
    OMX_GetParameter(hComponentEncoder, OMX_IndexParamPortDefinition, &pInputPortDefinition);
    pInputPortDefinition.format.video.nFrameWidth = width;
    pInputPortDefinition.format.video.nFrameHeight = height;
    if (strstr(encodeTestContext->sInputFormat, "i420") != NULL)
    {
        pInputPortDefinition.format.video.eColorFormat = OMX_COLOR_FormatYUV420Planar;
    }
    else if (strstr(encodeTestContext->sInputFormat, "nv12") != NULL)
    {
        pInputPortDefinition.format.video.eColorFormat = OMX_COLOR_FormatYUV420SemiPlanar;
    }
    else if (strstr(encodeTestContext->sInputFormat, "nv21") != NULL)
    {
        pInputPortDefinition.format.video.eColorFormat = OMX_COLOR_FormatYVU420SemiPlanar;
    }
    else
    {
        printf("unsupported color format: %s\r\n", encodeTestContext->sInputFormat);
        goto end;
    }
    OMX_SetParameter(hComponentEncoder, OMX_IndexParamPortDefinition, &pInputPortDefinition);
    OMX_GetParameter(hComponentEncoder, OMX_IndexParamPortDefinition, &pInputPortDefinition);

    OMX_PARAM_PORTDEFINITIONTYPE pOutputPortDefinition;
    OMX_INIT_STRUCTURE(pOutputPortDefinition);
    pOutputPortDefinition.nPortIndex = 1;
    OMX_GetParameter(hComponentEncoder, OMX_IndexParamPortDefinition, &pOutputPortDefinition);
    pOutputPortDefinition.format.video.nFrameWidth = width;
    pOutputPortDefinition.format.video.nFrameHeight = height;
    pOutputPortDefinition.format.video.nBitrate = encodeTestContext->nBitrate;
    OMX_SetParameter(hComponentEncoder, OMX_IndexParamPortDefinition, &pOutputPortDefinition);

    /*Alloc input buffer*/
    OMX_U32 nInputBufferSize = pInputPortDefinition.nBufferSize;
    OMX_U32 nInputBufferCount = pInputPortDefinition.nBufferCountActual;

    OMX_SendCommand(hComponentEncoder, OMX_CommandStateSet, OMX_StateIdle, NULL);

    for (int i = 0; i < nInputBufferCount; i++)
    {
        OMX_BUFFERHEADERTYPE *pBuffer = NULL;
        OMX_AllocateBuffer(hComponentEncoder, &pBuffer, 0, NULL, nInputBufferSize);
        encodeTestContext->pInputBufferArray[i] = pBuffer;
        /*Fill Input Buffer*/
        FillInputBuffer(encodeTestContext, pBuffer);
        OMX_EmptyThisBuffer(hComponentEncoder, pBuffer);
    }
    fb = fopen(encodeTestContext->sOutputFilePath, "wb+");

    OMX_SendCommand(hComponentEncoder, OMX_CommandStateSet, OMX_StateExecuting, NULL);

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
            FillInputBuffer(encodeTestContext, pBuffer);
            OMX_EmptyThisBuffer(encodeTestContext->hComponentEncoder, pBuffer);
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
                OMX_FillThisBuffer(encodeTestContext->hComponentEncoder, pBuffer);
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
    fclose(encodeTestContext->pInputFile);
    OMX_FreeHandle(hComponentEncoder);
    OMX_Deinit();
}
