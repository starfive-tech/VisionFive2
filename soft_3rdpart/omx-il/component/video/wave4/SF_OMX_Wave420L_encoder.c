// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2022 StarFive Technology Co., Ltd.
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "SF_OMX_Wave420L_encoder.h"
#include "SF_OMX_Core.h"
#include <sys/prctl.h>

#define STREAM_READ_ALL_SIZE            (0)
#define WAVE420L_CONFIG_FILE "/lib/firmware/encoder_defconfig.cfg"
#define OMX_Command_StopThread OMX_CommandMax

static Uint32 BitCodesizeInWord;
static Uint16* pusBitCode;

static Uint32 inputBufNum = 0;
static Uint32 outputBufNum = 0;

static Uint32 tmpFramerate;
static Uint64 tmpCounter=0;

static OMX_BOOL EmptyBufferDone(SF_OMX_COMPONENT *pSfOMXComponent, OMX_BUFFERHEADERTYPE *pBuffer);
static OMX_BOOL FillBufferDone(SF_OMX_COMPONENT *pSfOMXComponent, OMX_BUFFERHEADERTYPE *pBuffer);
static void FlushBuffer(SF_OMX_COMPONENT *pSfOMXComponent, OMX_U32 nPortNumber);
static void CmdThread(void *args);
static void EncoderThread(void *args);


static void sf_get_component_functions(SF_W420L_FUNCTIONS *funcs, OMX_PTR *sohandle)
{
    FunctionIn();
    funcs->VPU_GetProductId = dlsym(sohandle, "VPU_GetProductId");
    funcs->VPU_GetVersionInfo = dlsym(sohandle, "VPU_GetVersionInfo");
    funcs->VPU_InitWithBitcode = dlsym(sohandle, "VPU_InitWithBitcode");
    funcs->VPU_DeInit = dlsym(sohandle, "VPU_DeInit");
    funcs->GetEncOpenParam = dlsym(sohandle, "GetEncOpenParam");
    funcs->GetEncOpenParamDefault = dlsym(sohandle, "GetEncOpenParamDefault");
    funcs->VPU_EncOpen = dlsym(sohandle, "VPU_EncOpen");
    funcs->VPU_EncClose = dlsym(sohandle, "VPU_EncClose");
    funcs->VPU_GetFrameBufSize = dlsym(sohandle, "VPU_GetFrameBufSize");
    funcs->VPU_EncRegisterFrameBuffer = dlsym(sohandle, "VPU_EncRegisterFrameBuffer");
    funcs->VPU_EncAllocateFrameBuffer = dlsym(sohandle, "VPU_EncAllocateFrameBuffer");
    funcs->VPU_EncGiveCommand = dlsym(sohandle, "VPU_EncGiveCommand");
    funcs->VPU_EncGetInitialInfo = dlsym(sohandle, "VPU_EncGetInitialInfo");
    funcs->VPU_SWReset = dlsym(sohandle, "VPU_SWReset");
    funcs->VPU_EncStartOneFrame = dlsym(sohandle, "VPU_EncStartOneFrame");
    funcs->VPU_WaitInterrupt = dlsym(sohandle, "VPU_WaitInterrupt");
    funcs->VPU_ClearInterrupt = dlsym(sohandle, "VPU_ClearInterrupt");
    funcs->VPU_EncGetOutputInfo = dlsym(sohandle, "VPU_EncGetOutputInfo");
    funcs->vdi_allocate_dma_memory = dlsym(sohandle, "vdi_allocate_dma_memory");
    funcs->vdi_free_dma_memory = dlsym(sohandle, "vdi_free_dma_memory");
    funcs->YuvFeeder_Create = dlsym(sohandle, "YuvFeeder_Create");
    funcs->YuvFeeder_Feed = dlsym(sohandle, "YuvFeeder_Feed");
    funcs->YuvFeeder_Destroy = dlsym(sohandle, "YuvFeeder_Destroy");
    funcs->BufferStreamReader_Create = dlsym(sohandle, "BufferStreamReader_Create");
    funcs->BufferStreamReader_Act = dlsym(sohandle, "BufferStreamReader_Act");
    funcs->BufferStreamReader_Destroy = dlsym(sohandle, "BufferStreamReader_Destroy");
    funcs->Comparator_Create = dlsym(sohandle, "Comparator_Create");
    funcs->Comparator_Destroy = dlsym(sohandle, "Comparator_Destroy");
    funcs->Comparator_CheckEOF = dlsym(sohandle, "Comparator_CheckEOF");
    funcs->DisplayEncodedInformation = dlsym(sohandle, "DisplayEncodedInformation");
    funcs->PrintVpuVersionInfo = dlsym(sohandle, "PrintVpuVersionInfo");
    funcs->CalcStride = dlsym(sohandle, "CalcStride");
    funcs->GetPackedFormat = dlsym(sohandle, "GetPackedFormat");
    funcs->LoadFirmware = dlsym(sohandle, "LoadFirmware");
    funcs->SetMaxLogLevel = dlsym(sohandle, "SetMaxLogLevel");
    funcs->EnterLock = dlsym(sohandle, "EnterLock");
    funcs->LeaveLock = dlsym(sohandle, "LeaveLock");
    funcs->PrintVpuStatus = dlsym(sohandle, "PrintVpuStatus");
    FunctionOut();
}

static OMX_ERRORTYPE CreateThread(THREAD_HANDLE_TYPE **threadHandle, OMX_PTR function_name, OMX_PTR argument)
{
    FunctionIn();

    int result = 0;
    int detach_ret = 0;
    THREAD_HANDLE_TYPE *thread;
    OMX_ERRORTYPE ret = OMX_ErrorNone;

    thread = malloc(sizeof(THREAD_HANDLE_TYPE));
    memset(thread, 0, sizeof(THREAD_HANDLE_TYPE));

    pthread_attr_init(&thread->attr);
    if (thread->stack_size != 0)
        pthread_attr_setstacksize(&thread->attr, thread->stack_size);

    /* set priority */
    if (thread->schedparam.sched_priority != 0)
        pthread_attr_setschedparam(&thread->attr, &thread->schedparam);

    detach_ret = pthread_attr_setdetachstate(&thread->attr, PTHREAD_CREATE_JOINABLE);
    if (detach_ret != 0)
    {
        free(thread);
        *threadHandle = NULL;
        ret = OMX_ErrorUndefined;
        goto EXIT;
    }

    result = pthread_create(&thread->pthread, &thread->attr, function_name, (void *)argument);
    /* pthread_setschedparam(thread->pthread, SCHED_RR, &thread->schedparam); */

    switch (result)
    {
    case 0:
        *threadHandle = thread;
        ret = OMX_ErrorNone;
        break;
    case EAGAIN:
        free(thread);
        *threadHandle = NULL;
        ret = OMX_ErrorInsufficientResources;
        break;
    default:
        free(thread);
        *threadHandle = NULL;
        ret = OMX_ErrorUndefined;
        break;
    }

EXIT:
    FunctionOut();

    return ret;
}

OMX_ERRORTYPE WaveOmxInit(SF_OMX_COMPONENT *pSfOMXComponent)
{
    OMX_ERRORTYPE ret = OMX_ErrorNone;
    char *strDebugLevel = NULL;
    int debugLevel = 0;
    SF_WAVE420L_IMPLEMEMT *pImp = NULL;

    FunctionIn();
    if (pSfOMXComponent == NULL)
    {
        ret = OMX_ErrorBadParameter;
        goto EXIT;
    }

    pSfOMXComponent->pOMXComponent = malloc(sizeof(OMX_COMPONENTTYPE));
    if (pSfOMXComponent->pOMXComponent == NULL)
    {
        ret = OMX_ErrorInsufficientResources;
        LOG(SF_LOG_ERR, "malloc fail\r\n");
        goto ERROR;
    }
    memset(pSfOMXComponent->pOMXComponent, 0, sizeof(OMX_COMPONENTTYPE));

    pSfOMXComponent->soHandle = dlopen(pSfOMXComponent->libName, RTLD_NOW);
    if (pSfOMXComponent->soHandle == NULL)
    {
        ret = OMX_ErrorInsufficientResources;
        LOG(SF_LOG_ERR, "could not open %s, error: %s\r\n", pSfOMXComponent->libName, dlerror());
        goto ERROR;
    }

    pSfOMXComponent->componentImpl = malloc(sizeof(SF_WAVE420L_IMPLEMEMT));
    if (pSfOMXComponent->componentImpl == NULL)
    {
        ret = OMX_ErrorInsufficientResources;
        LOG(SF_LOG_ERR, "malloc fail\r\n");
        goto ERROR;
    }
    memset(pSfOMXComponent->componentImpl, 0, sizeof(SF_WAVE420L_IMPLEMEMT));

    pImp = (SF_WAVE420L_IMPLEMEMT *)pSfOMXComponent->componentImpl;
    pImp->functions = malloc(sizeof(SF_W420L_FUNCTIONS));
    pImp->currentState = OMX_StateLoaded;
    //pImp->frameFormat = DEFAULT_FRAME_FORMAT;
    if (pImp->functions == NULL)
    {
        ret = OMX_ErrorInsufficientResources;
        LOG(SF_LOG_ERR, "malloc fail\r\n");
        goto ERROR;
    }
    memset(pImp->functions, 0, sizeof(SF_W420L_FUNCTIONS));
    sf_get_component_functions(pImp->functions, pSfOMXComponent->soHandle);

    // Init Vpu log
    if (pImp->functions->SetMaxLogLevel)
    {
        strDebugLevel = getenv("VPU_DEBUG");
        if (strDebugLevel)
        {
            debugLevel = atoi(strDebugLevel);
        }else{
            debugLevel = 0;
        }
        if (debugLevel >=0)
        {
            pImp->functions->SetMaxLogLevel(debugLevel);
        }
    }

    pSfOMXComponent->pOMXComponent->pComponentPrivate = pSfOMXComponent;

    pImp->EmptyQueue = SF_Queue_Create(ENC_SRC_BUF_NUM, sizeof(OMX_BUFFERHEADERTYPE*));
    if (NULL == pImp->EmptyQueue)
    {
        LOG(SF_LOG_ERR, "create EmptyQueue error");
        return OMX_ErrorInsufficientResources;
    }
    pImp->FillQueue = SF_Queue_Create(ENC_SRC_BUF_NUM, sizeof(OMX_BUFFERHEADERTYPE*));
    if (NULL == pImp->FillQueue)
    {
        LOG(SF_LOG_ERR, "create FillQueue error");
        return OMX_ErrorInsufficientResources;
    }
    pImp->CmdQueue = SF_Queue_Create(ENC_SRC_BUF_NUM, sizeof(SF_OMX_CMD));
    if (NULL == pImp->CmdQueue)
    {
        LOG(SF_LOG_ERR, "create CmdQueue error");
        return OMX_ErrorInsufficientResources;
    }

    for (int i = 0; i < OMX_PORT_MAX; i++)
    {
        OMX_PARAM_PORTDEFINITIONTYPE *pPortDefinition = &pSfOMXComponent->portDefinition[i];
        OMX_VIDEO_PARAM_HEVCTYPE *pHEVCComponent = &pImp->HEVCComponent[i];

        INIT_SET_SIZE_VERSION(pPortDefinition, OMX_PARAM_PORTDEFINITIONTYPE);
        INIT_SET_SIZE_VERSION(pHEVCComponent, OMX_VIDEO_PARAM_HEVCTYPE);

        pPortDefinition->nPortIndex = i;
        pPortDefinition->nBufferCountActual = (i == 0 ? WAVE_DEFAULT_INPUT_BUF_NUMBER : WAVE_DEFAULT_OUTPUT_BUF_NUMBER);
        pPortDefinition->nBufferCountMin = (i == 0 ? WAVE_DEFAULT_INPUT_BUF_NUMBER : WAVE_DEFAULT_OUTPUT_BUF_NUMBER);
        pPortDefinition->nBufferSize = (i == 0 ? DEFAULT_MJPEG_INPUT_BUFFER_SIZE : DEFAULT_MJPEG_OUTPUT_BUFFER_SIZE);
        pPortDefinition->eDomain = OMX_PortDomainVideo;

        pPortDefinition->format.video.nFrameWidth = DEFAULT_FRAME_WIDTH;
        pPortDefinition->format.video.nFrameHeight = DEFAULT_FRAME_HEIGHT;
        pPortDefinition->format.video.nStride = DEFAULT_FRAME_WIDTH;
        pPortDefinition->format.video.nSliceHeight = DEFAULT_FRAME_HEIGHT;
        pPortDefinition->format.video.eCompressionFormat = OMX_VIDEO_CodingUnused;
        pPortDefinition->format.video.xFramerate = DEFAULT_FRAMERATE;

        pPortDefinition->format.video.pNativeRender = 0;
        pPortDefinition->format.video.bFlagErrorConcealment = OMX_FALSE;
        pPortDefinition->format.video.eColorFormat = OMX_COLOR_FormatUnused;

        pPortDefinition->bEnabled = OMX_TRUE;
        pPortDefinition->eDir = (i == 0 ? OMX_DirInput : OMX_DirOutput);

        pHEVCComponent->nPortIndex = i;
        pHEVCComponent->nKeyFrameInterval = DEFAULT_GOP;
        pHEVCComponent->eProfile = OMX_VIDEO_HEVCProfileMain;
    }

    pSfOMXComponent->portDefinition[OMX_INPUT_PORT_INDEX].format.video.cMIMEType = "raw/video";
    pSfOMXComponent->portDefinition[OMX_OUTPUT_PORT_INDEX].format.video.cMIMEType = "video/H265";

    pSfOMXComponent->portDefinition[OMX_INPUT_PORT_INDEX].format.video.eColorFormat = OMX_COLOR_FormatYUV420Planar;

    memset(pSfOMXComponent->pBufferArray, 0, sizeof(pSfOMXComponent->pBufferArray));
    pSfOMXComponent->memory_optimization = OMX_TRUE;


    CreateThread(&pImp->pCmdThread, CmdThread, (void *)pSfOMXComponent);
    pImp->bCmdRunning = OMX_TRUE;

    FunctionOut();
EXIT:
    return ret;
ERROR:
    if (pSfOMXComponent->pOMXComponent)
    {
        free(pSfOMXComponent->pOMXComponent);
        pSfOMXComponent->pOMXComponent = NULL;
    }

    return ret;
}

static OMX_ERRORTYPE InitEncoder(SF_OMX_COMPONENT *pSfOMXComponent)
{
    OMX_ERRORTYPE ret = OMX_ErrorNone;
    SF_WAVE420L_IMPLEMEMT *pImp = (SF_WAVE420L_IMPLEMEMT *)pSfOMXComponent->componentImpl;
    TestEncConfig   *pEncConfig = &pImp->encConfig;
    Int32 productId;

    FunctionIn();

    pEncConfig->stdMode       = STD_HEVC;
    pEncConfig->frame_endian  = VPU_FRAME_ENDIAN;
    pEncConfig->stream_endian = VPU_STREAM_ENDIAN;
    pEncConfig->source_endian = VPU_SOURCE_ENDIAN;
    pEncConfig->mapType       = COMPRESSED_FRAME_MAP;

    pEncConfig->coreIdx       = 0;
    pEncConfig->picWidth      = DEFAULT_FRAME_WIDTH;
    pEncConfig->picHeight     = DEFAULT_FRAME_HEIGHT;
    pEncConfig->cbcrInterleave = OMX_FALSE;
    pEncConfig->nv21          = OMX_FALSE;
    pEncConfig->srcFormat     = FORMAT_420;
    pEncConfig->packedFormat  = NOT_PACKED;
    pEncConfig->yuv_mode      = NOT_PACKED;
    strcpy(pEncConfig->cfgFileName, WAVE420L_CONFIG_FILE);

    //Wave420L not support
    pEncConfig->rotAngle      = 0;
    pEncConfig->mirDir        = 0;
    pEncConfig->srcFormat3p4b = 0;
    pEncConfig->ringBufferEnable = 0;


    productId = Warp_VPU_GetProductId(pImp, pEncConfig->coreIdx);

    LOG(SF_LOG_INFO, "product ID = %d FW PATH = %s\r\n", productId, pSfOMXComponent->fwPath);
    if (Warp_LoadFirmware(pImp, productId, (Uint8**)&pusBitCode, &BitCodesizeInWord, pSfOMXComponent->fwPath) < 0) {
        LOG(SF_LOG_ERR, "%s:%d Failed to load firmware: %s\r\n", __FUNCTION__, __LINE__, pSfOMXComponent->fwPath);
        return OMX_ErrorInsufficientResources;
    }

    ret = Warp_VPU_InitWithBitcode(pImp, pEncConfig->coreIdx, (const Uint16*)pusBitCode, BitCodesizeInWord);
    if (ret)
    {
        LOG(SF_LOG_ERR, "Failed to boot up VPU with bit code\r\n");
        free(pusBitCode);
        return OMX_ErrorInsufficientResources;
    }

    Warp_PrintVpuVersionInfo(pImp, pEncConfig->coreIdx);

    return ret;
    FunctionOut();
}

static void CmdThread(void *args)
{
    SF_OMX_COMPONENT *pSfOMXComponent = (SF_OMX_COMPONENT *)args;
    SF_WAVE420L_IMPLEMEMT *pImp = (SF_WAVE420L_IMPLEMEMT *)pSfOMXComponent->componentImpl;
    SF_OMX_CMD *pCmd;
    void *pNull = NULL;
    void *ThreadRet;

    while(1){
        pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
        pCmd = (SF_OMX_CMD*)SF_Queue_Dequeue_Block(pImp->CmdQueue);
        pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
        pthread_testcancel();
        pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

        if(pCmd->Cmd == OMX_Command_StopThread)
            break;

        switch (pCmd->Cmd)
        {
        case OMX_CommandStateSet:
            pSfOMXComponent->nextState = pCmd->nParam;
            LOG(SF_LOG_INFO, "OMX dest state = %X, current state = %X\r\n", pCmd->nParam, pImp->currentState);
            switch (pCmd->nParam)
            {
            case OMX_StateLoaded:
                break;
            case OMX_StateIdle:
                switch (pImp->currentState)
                {
                    case OMX_StateExecuting:
                    {
                        pImp->bThreadRunning = 0;
                        // enqueue null mean encoder thread cycle end
                        SF_Queue_Enqueue(pImp->EmptyQueue, &pNull);
                        SF_Queue_Enqueue(pImp->FillQueue, &pNull);
                        pthread_join(pImp->pProcessThread->pthread, &ThreadRet);
                        LOG(SF_LOG_INFO, "Encoder thread end %ld\r\n", (Uint64)ThreadRet);
                        FlushBuffer(pSfOMXComponent,OMX_INPUT_PORT_INDEX);
                        FlushBuffer(pSfOMXComponent,OMX_OUTPUT_PORT_INDEX);
                        SF_Queue_Flush(pImp->EmptyQueue);
                        SF_Queue_Flush(pImp->FillQueue);
                        pImp->currentState = OMX_StateIdle;
                        break;
                    }
                    case OMX_StateIdle:
                        break;
                    case OMX_StateLoaded:
                        // InitDecoder(pSfOMXComponent);
                        pImp->currentState = OMX_StateIdle;
                        break;
                    default:
                        LOG(SF_LOG_ERR, "Can't go to state %d from %d\r\n",pImp->currentState, pCmd->nParam);
                        break;
                }
                break;
            case OMX_StateExecuting:
                switch (pImp->currentState)
                {
                case OMX_StateIdle:
                    pImp->bThreadRunning = OMX_TRUE;
                    CreateThread(&pImp->pProcessThread, EncoderThread, (void *)pSfOMXComponent);
                    pImp->currentState = OMX_StateExecuting;
                    break;
                case OMX_StateExecuting:
                    break;
                case OMX_StateLoaded:
                default:
                    LOG(SF_LOG_ERR, "Can't go to state %d from %d\r\n",pImp->currentState, pCmd->nParam);
                    break;
                }
                break;
            default:
                break;
            }
            pSfOMXComponent->callbacks->EventHandler(pSfOMXComponent->pOMXComponent, pSfOMXComponent->pAppData,
                                                    OMX_EventCmdComplete, OMX_CommandStateSet, pCmd->nParam, NULL);
            LOG(SF_LOG_DEBUG, "complete cmd StateSet %d\r\n", pCmd->nParam);
            break;

        case OMX_CommandFlush:
            FlushBuffer(pSfOMXComponent, pCmd->nParam);
            pSfOMXComponent->callbacks->EventHandler(pSfOMXComponent->pOMXComponent, pSfOMXComponent->pAppData,
                                                    OMX_EventCmdComplete, OMX_CommandFlush, pCmd->nParam, NULL);
            LOG(SF_LOG_DEBUG, "complete cmd flush port %d\r\n", pCmd->nParam);
            break;

        case OMX_CommandPortDisable:
            if(pCmd->nParam < OMX_PORT_MAX)
                pSfOMXComponent->portDefinition[pCmd->nParam].bEnabled = OMX_FALSE;
            pSfOMXComponent->callbacks->EventHandler(pSfOMXComponent->pOMXComponent, pSfOMXComponent->pAppData,
                                                    OMX_EventCmdComplete, OMX_CommandPortDisable, pCmd->nParam, NULL);
            LOG(SF_LOG_DEBUG, "complete cmd disable port %d\r\n", pCmd->nParam);
            break;

        case OMX_CommandPortEnable:
            if(pCmd->nParam < OMX_PORT_MAX)
                pSfOMXComponent->portDefinition[pCmd->nParam].bEnabled = OMX_TRUE;
            pSfOMXComponent->callbacks->EventHandler(pSfOMXComponent->pOMXComponent, pSfOMXComponent->pAppData,
                                                    OMX_EventCmdComplete, OMX_CommandPortEnable, pCmd->nParam, NULL);
            LOG(SF_LOG_DEBUG, "complete cmd enable port %d\r\n", pCmd->nParam);
            break;

        default:
            break;
        }
    }

    pthread_exit(NULL);
}

static void EncoderThread(void *args)
{
    SF_OMX_COMPONENT *pSfOMXComponent = (SF_OMX_COMPONENT *)args;
    SF_WAVE420L_IMPLEMEMT *pImp = (SF_WAVE420L_IMPLEMEMT *)pSfOMXComponent->componentImpl;
    TestEncConfig   *pEncConfig = &pImp->encConfig;
    EncOpenParam    *pEncOP = &pImp->encOP;
    EncHandle       *pHandle = &pImp->handle;
    EncInitialInfo  *pInitialInfo = &pImp->initialInfo;
    EncOutputInfo       outputInfo;
    FrameBufferAllocInfo fbAllocInfo;
    FrameBufferFormat   srcFrameFormat;
    EncParam            encParam;
    EncHeaderParam encHeaderParam;
    YuvInfo        yuvFeederInfo;
    OMX_BUFFERHEADERTYPE *pOutputBuffer;
    OMX_BUFFERHEADERTYPE *pInputBuffer;
    BitstreamReader bsReader = NULL;
    SecAxiUse secAxiUse;
    Comparator comparatorBitStream  = NULL;//todo use comparator
    Int32 srcFrameWidth, srcFrameHeight, srcFrameStride;
    Int32 framebufStride = 0, framebufWidth = 0, framebufHeight = 0, FrameBufSize = 0;
    Int32 regFrameBufCount;
    Int32 productId, instIdx, coreIdx, mapType;
    Int32 i, ret, bsBufferCount, bsQueueIndex = 0;
    Int32 srcFrameIdx = 0, frameIdx = 0;
    Int32 timeoutCount = 0, int_reason = 0;
    Uint32 interruptTimeout = VPU_ENC_TIMEOUT;
    Uint32 size;
    void* yuvFeeder = NULL;

    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);

    instIdx = pEncConfig->instNum;
    coreIdx = pEncConfig->coreIdx;

    productId = Warp_VPU_GetProductId(pImp, coreIdx);

    pEncOP->bitstreamFormat = pEncConfig->stdMode;
    mapType = (pEncConfig->mapType & 0x0f);

    //Warp_GetEncOpenParamDefault(pImp, pEncOP, pEncConfig);
    if (strlen(pEncConfig->cfgFileName) != 0) {
        Warp_GetEncOpenParam(pImp, pEncOP, pEncConfig, NULL);
    }
    else {
        Warp_GetEncOpenParamDefault(pImp, pEncOP, pEncConfig);
    }

    //if modify encop should be here

    srcFrameWidth  = ((pEncOP->picWidth+7)&~7);        // width = 8-aligned (CU unit)
    srcFrameHeight = ((pEncOP->picHeight+7)&~7);       // height = 8-aligned (CU unit)
    srcFrameStride = ((pEncOP->picWidth+31)&~31);      // stride should be a 32-aligned.

    if (pEncConfig->packedFormat >= 1)
        pEncConfig->srcFormat = FORMAT_422;

    if (pEncConfig->srcFormat == FORMAT_422 && pEncConfig->packedFormat >= PACKED_YUYV) {
        Int32 p10bits = pEncConfig->srcFormat3p4b == 0 ? 16 : 32;
        Int32 packedFormat =
            pImp->functions->GetPackedFormat(pEncOP->srcBitDepth, pEncConfig->packedFormat, p10bits, 1);

        if (packedFormat == -1) {
            LOG(SF_LOG_ERR, "fail to GetPackedFormat\n" );
            goto ERR_ENC_INIT;
        }
        pEncOP->srcFormat = packedFormat;
        srcFrameFormat  = (FrameBufferFormat)packedFormat;
        pEncOP->nv21      = 0;
        pEncOP->cbcrInterleave = 0;
    }
    else {
        pEncOP->srcFormat    = pEncConfig->srcFormat;
        srcFrameFormat     = (FrameBufferFormat)pEncConfig->srcFormat;
        pEncOP->nv21         = pEncConfig->nv21;
    }
    pEncOP->packedFormat = pEncConfig->packedFormat;

    framebufWidth  = (pEncOP->picWidth  +7)&~7;
    framebufHeight = (pEncOP->picHeight +7)&~7;

    bsBufferCount = DEFAULT_BS_BUFF;
    for (i=0; i<bsBufferCount; i++ ) {
        pImp->vbStream[i].size = framebufWidth * framebufWidth *2;
        if (Warp_vdi_allocate_dma_memory(pImp, coreIdx, &pImp->vbStream[i]) < 0) {
            LOG(SF_LOG_ERR, "fail to allocate bitstream buffer\n" );
            goto ERR_ENC_INIT;
        }
        LOG(SF_LOG_DEBUG, "STREAM_BUF=0x%x STREAM_BUF_SIZE=%d(0x%x)\n",pImp->vbStream[i].phys_addr, pImp->vbStream[i].size, pImp->vbStream[i].size);
    }

    pSfOMXComponent->portDefinition[OMX_OUTPUT_PORT_INDEX].format.video.nStride = framebufWidth;
    pSfOMXComponent->portDefinition[OMX_OUTPUT_PORT_INDEX].format.video.nSliceHeight = framebufHeight;
    pSfOMXComponent->portDefinition[OMX_OUTPUT_PORT_INDEX].nBufferSize = framebufWidth * framebufHeight * 2;
    LOG(SF_LOG_DEBUG, "OMX_EventPortSettingsChanged IN\r\n");
    pSfOMXComponent->callbacks->EventHandler(pSfOMXComponent->pOMXComponent, pSfOMXComponent->pAppData, OMX_EventPortSettingsChanged,
                                             OMX_OUTPUT_PORT_INDEX, OMX_IndexParamPortDefinition, NULL);
    LOG(SF_LOG_DEBUG, "OMX_EventPortSettingsChanged OUT\r\n");

    pEncOP->bitstreamBuffer = pImp->vbStream[0].phys_addr;
    pEncOP->bitstreamBufferSize = pImp->vbStream[0].size;//* bsBufferCount;//

    pEncOP->ringBufferEnable =  pEncConfig->ringBufferEnable;
    pEncOP->cbcrInterleave = pEncConfig->cbcrInterleave;
    pEncOP->frameEndian   = pEncConfig->frame_endian;
    pEncOP->streamEndian  = pEncConfig->stream_endian;
    pEncOP->sourceEndian  = pEncConfig->source_endian;

    pEncOP->lineBufIntEn  =  pEncConfig->lineBufIntEn;
    pEncOP->coreIdx       = coreIdx;
    pEncOP->cbcrOrder     = CBCR_ORDER_NORMAL;
    // host can set useLongTerm to 1 or 0 directly
    pEncOP->EncStdParam.hevcParam.useLongTerm =
            (pEncConfig->useAsLongtermPeriod > 0 && pEncConfig->refLongtermPeriod > 0) ? 1 : 0;

    /* set framerate and bitrate */
    /* If xFramerate value from user is stored in Q16 format, should convert into integer */
    if(pSfOMXComponent->portDefinition[0].format.video.xFramerate > (1 << 16))
    {
        pEncOP->frameRateInfo = pSfOMXComponent->portDefinition[0].format.video.xFramerate >> 16;
    }
    else
    {
        pEncOP->frameRateInfo = pSfOMXComponent->portDefinition[0].format.video.xFramerate;
    }

    pEncOP->EncStdParam.hevcParam.numUnitsInTick = 1000;
    pEncOP->EncStdParam.hevcParam.timeScale = (pEncOP->frameRateInfo)*1000;

    if(pSfOMXComponent->portDefinition[1].format.video.nBitrate)
    {
        pEncOP->rcEnable = 1;
        pEncOP->bitRate = pSfOMXComponent->portDefinition[1].format.video.nBitrate;
    }
    else if(pEncOP->rcEnable == 1)
    {
        pSfOMXComponent->portDefinition[1].format.video.nBitrate = pEncOP->bitRate;
    }

    ret = Warp_VPU_EncOpen(pImp, pHandle, pEncOP);
    if (ret) {
        LOG(SF_LOG_ERR, "VPU_EncOpen failed Error code is 0x%x \n", ret );
        goto ERR_ENC_INIT;
    }

    ret = Warp_VPU_EncGetInitialInfo(pImp, *pHandle, pInitialInfo);
    if (ret) {
        LOG(SF_LOG_ERR, "VPU_EncGetInitialInfo failed Error code is 0x%x \n", ret );
        goto ERR_ENC_OPEN;
    }

    LOG(SF_LOG_INFO, "* Enc InitialInfo =>\n instance #%d, \n minframeBuffercount: %u\n minSrcBufferCount: %d\n",
            instIdx, pInitialInfo->minFrameBufferCount, pInitialInfo->minSrcFrameCount);
    LOG(SF_LOG_INFO, " picWidth: %u\n picHeight: %u\n ",pEncOP->picWidth, pEncOP->picHeight);

    //create comparator here

    secAxiUse.u.wave4.useEncImdEnable  = (pEncConfig->secondary_axi & 0x1)?TRUE:FALSE;  //USE_IMD_INTERNAL_BUF
    secAxiUse.u.wave4.useEncRdoEnable  = (pEncConfig->secondary_axi & 0x2)?TRUE:FALSE;  //USE_RDO_INTERNAL_BUF
    secAxiUse.u.wave4.useEncLfEnable   = (pEncConfig->secondary_axi & 0x4)?TRUE:FALSE;  //USE_LF_INTERNAL_BUF
    Warp_VPU_EncGiveCommand(pImp, *pHandle, SET_SEC_AXI, &secAxiUse);

    /* Allocate framebuffers for recon. */
    framebufStride = Warp_CalcStride(pImp, framebufWidth, framebufHeight,
            (FrameBufferFormat)pEncOP->srcFormat, pEncOP->cbcrInterleave, (TiledMapType)mapType, FALSE);
    FrameBufSize   = Warp_VPU_GetFrameBufSize(pImp, coreIdx, framebufStride,
            framebufHeight, (TiledMapType)mapType, pEncOP->srcFormat, pEncOP->cbcrInterleave, NULL);

    regFrameBufCount = pInitialInfo->minFrameBufferCount;
    for (i = 0; i < regFrameBufCount; i++) {
        pImp->vbReconFrameBuf[i].size = FrameBufSize;
        if (Warp_vdi_allocate_dma_memory(pImp,coreIdx, &pImp->vbReconFrameBuf[i]) < 0) {
            LOG(SF_LOG_ERR, "SF_LOG_fail to allocate recon buffer\n" );
            goto ERR_ENC_OPEN;
        }
        pImp->fbRecon[i].bufY  = pImp->vbReconFrameBuf[i].phys_addr;
        pImp->fbRecon[i].bufCb = (PhysicalAddress)-1;
        pImp->fbRecon[i].bufCr = (PhysicalAddress)-1;
        pImp->fbRecon[i].size  = FrameBufSize;
        pImp->fbRecon[i].updateFbInfo = TRUE;
        LOG(SF_LOG_DEBUG, "Recon_BUF=0x%x Recon_BUF_SIZE=%d(0x%x)\n",pImp->vbReconFrameBuf[i].phys_addr, pImp->vbReconFrameBuf[i].size, pImp->vbReconFrameBuf[i].size);
    }

    ret = Warp_VPU_EncRegisterFrameBuffer(pImp, *pHandle, pImp->fbRecon,
                regFrameBufCount, framebufStride, framebufHeight, mapType);
    if(ret) {
        LOG(SF_LOG_ERR, "VPU_EncRegisterFrameBuffer failed Error code is 0x%x \n", ret );
        goto ERR_ENC_OPEN;
    }


    fbAllocInfo.mapType = LINEAR_FRAME_MAP;

    srcFrameStride = Warp_CalcStride(pImp, srcFrameWidth, srcFrameHeight, (FrameBufferFormat)srcFrameFormat,
                                            pEncOP->cbcrInterleave, (TiledMapType)fbAllocInfo.mapType, FALSE);
    FrameBufSize   = Warp_VPU_GetFrameBufSize(pImp, coreIdx, srcFrameStride, srcFrameHeight,
                                            (TiledMapType)fbAllocInfo.mapType, srcFrameFormat, pEncOP->cbcrInterleave, NULL);

    fbAllocInfo.format = (FrameBufferFormat)srcFrameFormat;
    fbAllocInfo.cbcrInterleave = pEncOP->cbcrInterleave;
    fbAllocInfo.stride = srcFrameStride;
    fbAllocInfo.height = srcFrameHeight;
    fbAllocInfo.endian = pEncOP->sourceEndian;
    fbAllocInfo.type   = FB_TYPE_PPU;
    fbAllocInfo.num    = pInitialInfo->minSrcFrameCount + EXTRA_SRC_BUFFER_NUM;
    fbAllocInfo.nv21   = pEncOP->nv21;

    LOG(SF_LOG_INFO, "Allocated source framebuffers : %d, size : %d\n", fbAllocInfo.num, FrameBufSize);
    for (i = 0; i < fbAllocInfo.num; i++) {
        pImp->vbSourceFrameBuf[i].size = FrameBufSize;
        if (Warp_vdi_allocate_dma_memory(pImp, coreIdx, &pImp->vbSourceFrameBuf[i]) < 0)
        {
            LOG(SF_LOG_ERR, "fail to allocate frame buffer\n" );
            goto ERR_ENC_OPEN;
        }
        pImp->fbSrc[i].bufY  = pImp->vbSourceFrameBuf[i].phys_addr;
        pImp->fbSrc[i].bufCb = (PhysicalAddress)-1;
        pImp->fbSrc[i].bufCr = (PhysicalAddress)-1;
        pImp->fbSrc[i].size  = FrameBufSize;
        pImp->fbSrc[i].updateFbInfo = TRUE;
        LOG(SF_LOG_DEBUG, "Source_BUF=0x%x Source_BUF_SIZE=%d(0x%x)\n",pImp->vbSourceFrameBuf[i].phys_addr, pImp->vbSourceFrameBuf[i].size, pImp->vbSourceFrameBuf[i].size);
    }

    ret = Warp_VPU_EncAllocateFrameBuffer(pImp, *pHandle, fbAllocInfo, pImp->fbSrc);
    if (ret) {
        LOG(SF_LOG_ERR, "VPU_EncAllocateFrameBuffer fail to allocate source frame buffer is 0x%x \n", ret );
        goto ERR_ENC_OPEN;
    }

    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_testcancel();
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

    memset(&encParam, 0x00, sizeof(EncParam));
    encParam.skipPicture        = 0;
    encParam.quantParam         = pEncConfig->picQpY;
    encParam.skipPicture        = 0;
    encParam.forcePicQpEnable   = 0;
    encParam.forcePicQpI        = 0;
    encParam.forcePicQpP        = 0;
    encParam.forcePicQpB        = 0;
    encParam.forcePicTypeEnable = 0;
    encParam.forcePicType       = 0;
    encParam.codeOption.implicitHeaderEncode = 1;      // FW will encode header data implicitly when changing the header syntaxes
    encParam.codeOption.encodeAUD   = pEncConfig->encAUD;
    encParam.codeOption.encodeEOS   = 0;

    memset(&encHeaderParam, 0x00, sizeof(EncHeaderParam));
    encHeaderParam.buf = pEncOP->bitstreamBuffer;
    encHeaderParam.size = pEncOP->bitstreamBufferSize;

    encHeaderParam.headerType = CODEOPT_ENC_VPS | CODEOPT_ENC_SPS | CODEOPT_ENC_PPS;
    ret = Warp_VPU_EncGiveCommand(pImp, *pHandle, ENC_PUT_VIDEO_HEADER, &encHeaderParam);
    if (ret) {
        LOG(SF_LOG_ERR, "VPU_EncGiveCommand ( ENC_PUT_VIDEO_HEADER ) for VPS/SPS/PPS failed Error Reason code : 0x%x \n", ret);
        goto ERR_ENC_OPEN;
    }
    if (encHeaderParam.size == 0) {
        LOG(SF_LOG_ERR, "encHeaderParam.size=0\n");
        goto ERR_ENC_OPEN;
    }

    bsReader = Warp_BufferStreamReader_Create(pImp, pEncOP->ringBufferEnable, (EndianMode)pEncOP->streamEndian, pHandle);

    pOutputBuffer = *(OMX_BUFFERHEADERTYPE**)SF_Queue_Dequeue_Block(pImp->FillQueue);

    if(pOutputBuffer == NULL){
        pImp->bThreadRunning = 0;
        goto ERR_ENC_OPEN;
    }

    LOG(SF_LOG_DEBUG, "get output buff %p index %d\r\n", pOutputBuffer->pBuffer, pOutputBuffer->nOutputPortIndex);

    Warp_EnterLock(pImp, coreIdx);
    size = Warp_BufferStreamReader_Act(pImp, bsReader, encHeaderParam.buf,
                pEncOP->bitstreamBufferSize, encHeaderParam.size, pOutputBuffer->pBuffer, comparatorBitStream);
    Warp_LeaveLock(pImp, coreIdx);
    if (size == 0) {
        LOG(SF_LOG_ERR, "fill header size 0, end\n");
        goto ERR_ENC_OPEN;
    }
    pOutputBuffer->nFilledLen = size;
    pOutputBuffer->nFlags = OMX_BUFFERFLAG_CODECCONFIG;
    FillBufferDone(pSfOMXComponent, pOutputBuffer);

    memset(&yuvFeederInfo,   0x00, sizeof(YuvInfo));
    yuvFeederInfo.cbcrInterleave = pEncConfig->cbcrInterleave;
    yuvFeederInfo.nv21           = pEncConfig->nv21;
    yuvFeederInfo.packedFormat   = pEncConfig->packedFormat;
    yuvFeederInfo.srcFormat      = pEncOP->srcFormat;
    yuvFeederInfo.srcPlanar      = TRUE;
    yuvFeederInfo.srcStride      = srcFrameStride;
    yuvFeederInfo.srcHeight      = srcFrameHeight;
    yuvFeeder = Warp_YuvFeeder_Create(pImp, SOURCE_YUV_WITH_BUFFER, NULL, yuvFeederInfo);
    if ( yuvFeeder == NULL ) {
        LOG(SF_LOG_ERR, "YuvFeeder_Create error");
        goto ERR_ENC_OPEN;
    }

    LOG(SF_LOG_INFO, "Enc Start :\n");
    Warp_DisplayEncodedInformation(pImp, *pHandle, STD_HEVC, 0, NULL, 0, 0);

    while(1){
        if(!pImp->bThreadRunning){
            LOG(SF_LOG_DEBUG,"encoder thread end\n");
            break;
        }

        pInputBuffer = *(OMX_BUFFERHEADERTYPE**)SF_Queue_Dequeue_Block(pImp->EmptyQueue);

        if(pInputBuffer == NULL){
            pImp->bThreadRunning = 0;
            LOG(SF_LOG_DEBUG, "null input buffer,end\n");
            break;
        }
        if(!pInputBuffer->nFilledLen){
            LOG(SF_LOG_DEBUG, "end of stream,end\n");
            pImp->bThreadRunning = 0;
            EmptyBufferDone(pSfOMXComponent, pInputBuffer);
            break;
        }else if(pInputBuffer->nFlags == OMX_BUFFERFLAG_EOS){
            LOG(SF_LOG_DEBUG, "end of stream,end\n");
            encParam.srcEndFlag = 1;
        }

        LOG(SF_LOG_DEBUG, "get input buff %p index %d\r\n", pInputBuffer->pBuffer, pInputBuffer->nInputPortIndex);

        srcFrameIdx = (frameIdx%fbAllocInfo.num);
        encParam.srcIdx = srcFrameIdx;
        ret = Warp_YuvFeeder_Feed(pImp, yuvFeeder, coreIdx, &pImp->fbSrc[srcFrameIdx], pEncOP->picWidth, pEncOP->picHeight, pInputBuffer->pBuffer);
        LOG(SF_LOG_DEBUG,"feed src %p size %ld fbSrcIdx %d ret %d\n",pInputBuffer->pBuffer, pInputBuffer->nFilledLen, srcFrameIdx, ret);
        if ( ret == 0 ) {
            encParam.srcEndFlag = 1;            // when there is no more source image to be encoded, srcEndFlag should be set 1. because of encoding delay for WAVE420
        }

        EmptyBufferDone(pSfOMXComponent, pInputBuffer);

        if ( encParam.srcEndFlag != 1) {
            pImp->fbSrc[srcFrameIdx].srcBufState = SRC_BUFFER_USE_ENCODE;
            encParam.sourceFrame = &pImp->fbSrc[srcFrameIdx];
            encParam.sourceFrame->sourceLBurstEn = 0;///???
        }

        bsQueueIndex = (bsQueueIndex+1)%bsBufferCount;
        encParam.picStreamBufferAddr = pImp->vbStream[bsQueueIndex].phys_addr;   // can set the newly allocated buffer.
        encParam.picStreamBufferSize = pEncOP->bitstreamBufferSize;

        if (pEncConfig->useAsLongtermPeriod > 0 && pEncConfig->refLongtermPeriod > 0) {
            encParam.useCurSrcAsLongtermPic = (frameIdx % pEncConfig->useAsLongtermPeriod) == 0 ? 1 : 0;
            encParam.useLongtermRef         = (frameIdx % pEncConfig->refLongtermPeriod)   == 0 ? 1 : 0;
        }

        // Start encoding a frame.
        frameIdx++;

        LOG(SF_LOG_DEBUG, "vpu start one frame\r\n");
        ret = Warp_VPU_EncStartOneFrame(pImp,*pHandle, &encParam);
        if(ret) {
            LOG(SF_LOG_ERR,  "VPU_EncStartOneFrame failed Error code is 0x%x \n", ret );
            Warp_LeaveLock(pImp, coreIdx);
            goto ERR_ENC_OPEN;
        }

        timeoutCount = 0;

        while(1){
            int_reason = Warp_VPU_WaitInterrupt(pImp, coreIdx, VPU_WAIT_TIME_OUT);

            if (int_reason == -1) {
                if (interruptTimeout > 0 && timeoutCount*VPU_WAIT_TIME_OUT > interruptTimeout) {
                    LOG(SF_LOG_ERR, "Error : encoder timeout happened\n");
                    Warp_PrintVpuStatus(pImp, coreIdx, productId);
                    Warp_VPU_SWReset(pImp, coreIdx, SW_RESET_SAFETY, *pHandle);
                    break;
                }
                int_reason = 0;
                timeoutCount++;
            }

            if (int_reason & (1<<INT_BIT_BIT_BUF_FULL)) {
                LOG(SF_LOG_WARN,"INT_BIT_BIT_BUF_FULL \n");
                pOutputBuffer = *(OMX_BUFFERHEADERTYPE**)SF_Queue_Dequeue_Block(pImp->FillQueue);

                if(pOutputBuffer == NULL){
                    pImp->bThreadRunning = 0;
                    LOG(SF_LOG_DEBUG, "null output buffer,end\n");
                    goto ERR_ENC_OPEN;
                }

                LOG(SF_LOG_DEBUG, "get output buff %p index %d\r\n", pOutputBuffer->pBuffer, pOutputBuffer->nOutputPortIndex);
                size = Warp_BufferStreamReader_Act(pImp, bsReader, pEncOP->bitstreamBuffer, pEncOP->bitstreamBufferSize, STREAM_READ_ALL_SIZE, pOutputBuffer->pBuffer, comparatorBitStream);
                if (size == 0) {
                    LOG(SF_LOG_ERR, "BufferStreamReader acc err \r\n");
                    goto ERR_ENC_OPEN;
                }
                pOutputBuffer->nFilledLen = size;
                pOutputBuffer->nFlags = 0;
                FillBufferDone(pSfOMXComponent, pOutputBuffer);
            }

            if (int_reason) {
                Warp_VPU_ClearInterrupt(pImp, coreIdx);
                if (int_reason & (1<<INT_WAVE_ENC_PIC)) {
                    break;
                }
            }

        }

        ret = Warp_VPU_EncGetOutputInfo(pImp, *pHandle, &outputInfo);
        if (ret ) {
            LOG(SF_LOG_ERR, "VPU_EncGetOutputInfo failed Error code is 0x%x \n", ret );
            if (ret == RETCODE_STREAM_BUF_FULL) {
                LOG(SF_LOG_ERR, "RETCODE_STREAM_BUF_FULL\n");
                continue;
            }
            else if ( ret == RETCODE_MEMORY_ACCESS_VIOLATION || ret == RETCODE_CP0_EXCEPTION || ret == RETCODE_ACCESS_VIOLATION_HW)
            {
                Warp_EnterLock(pImp, coreIdx);
                Warp_PrintVpuStatus(pImp, coreIdx, productId);
                Warp_VPU_SWReset(pImp,coreIdx, SW_RESET_SAFETY, *pHandle);
                Warp_LeaveLock(pImp, coreIdx);
            } else {
                Warp_EnterLock(pImp, coreIdx);
                Warp_PrintVpuStatus(pImp, coreIdx, productId);
                Warp_VPU_SWReset(pImp, coreIdx, SW_RESET_SAFETY, *pHandle);
                Warp_LeaveLock(pImp, coreIdx);
            }
            LOG(SF_LOG_ERR, "something err \r\n");
            goto ERR_ENC_OPEN;
        }

        Warp_DisplayEncodedInformation(pImp, *pHandle, STD_HEVC, 0, &outputInfo, encParam.srcEndFlag , srcFrameIdx);

        if (outputInfo.bitstreamWrapAround == 1) {
            LOG(SF_LOG_WARN, "Warnning!! BitStream buffer wrap arounded. prepare more large buffer \n", ret );
        }
        if (outputInfo.bitstreamSize == 0 && outputInfo.reconFrameIndex >= 0) {
            LOG(SF_LOG_ERR, "ERROR!!! bitstreamsize = 0 \n");
        }

        if (pEncOP->lineBufIntEn == 0) {
            if (outputInfo.wrPtr < outputInfo.rdPtr)
            {
                LOG(SF_LOG_ERR, "wrptr < rdptr\n");
                goto ERR_ENC_OPEN;
            }
        }

        if ( outputInfo.bitstreamSize ) {
            pOutputBuffer = *(OMX_BUFFERHEADERTYPE**)SF_Queue_Dequeue_Block(pImp->FillQueue);

            if(pOutputBuffer == NULL){
                pImp->bThreadRunning = 0;
                LOG(SF_LOG_DEBUG, "null output buffer,end\n");
                goto ERR_ENC_OPEN;
            }

            LOG(SF_LOG_DEBUG, "get output buff %p index %d\r\n", pOutputBuffer->pBuffer, pOutputBuffer->nOutputPortIndex);
            Warp_EnterLock(pImp, coreIdx);
            size = Warp_BufferStreamReader_Act(pImp, bsReader, outputInfo.bitstreamBuffer, pEncOP->bitstreamBufferSize,
                                    outputInfo.bitstreamSize, pOutputBuffer->pBuffer,comparatorBitStream);
            Warp_LeaveLock(pImp, coreIdx);
            if (size == 0) {
                LOG(SF_LOG_ERR, "BufferStreamReader acc err \r\n");
                goto ERR_ENC_OPEN;
            }
            pOutputBuffer->nFilledLen = size;
            pOutputBuffer->nFlags = OMX_BUFFERFLAG_ENDOFFRAME;
            FillBufferDone(pSfOMXComponent, pOutputBuffer);
        }

        if (outputInfo.reconFrameIndex == -1)       // end of encoding
        {
            LOG(SF_LOG_DEBUG, "end of coding\n");
            break;
        }

    }

ERR_ENC_OPEN:
    FlushBuffer(pSfOMXComponent,OMX_INPUT_PORT_INDEX);
    FlushBuffer(pSfOMXComponent,OMX_OUTPUT_PORT_INDEX);
    for (i = 0; i < regFrameBufCount; i++) {
        if (pImp->vbReconFrameBuf[i].size > 0) {
            Warp_vdi_free_dma_memory(pImp, coreIdx, &pImp->vbReconFrameBuf[i]);
        }
    }
    for (i = 0; i < fbAllocInfo.num; i++) {
        if (pImp->vbSourceFrameBuf[i].size > 0) {
            Warp_vdi_free_dma_memory(pImp, coreIdx, &pImp->vbSourceFrameBuf[i]);
        }
    }
    Warp_VPU_EncClose(pImp, *pHandle);
    LOG(SF_LOG_DEBUG,"close vpu \r\n");
ERR_ENC_INIT:
    for (i=0; i< bsBufferCount ; i++) {
        if (pImp->vbStream[i].size){
            Warp_vdi_free_dma_memory(pImp, coreIdx, &pImp->vbStream[i]);
        }
    }

    pthread_exit(NULL);
}

static void FlushBuffer(SF_OMX_COMPONENT *pSfOMXComponent, OMX_U32 nPortNumber)
{
    SF_WAVE420L_IMPLEMEMT *pImp = (SF_WAVE420L_IMPLEMEMT *)pSfOMXComponent->componentImpl;
    OMX_BUFFERHEADERTYPE *pOMXBuffer = NULL;
    OMX_BUFFERHEADERTYPE **ppBuffer = NULL;

    FunctionIn();
    switch (nPortNumber)
    {
    case OMX_INPUT_PORT_INDEX:
        while (OMX_TRUE)
        {
            ppBuffer = SF_Queue_Dequeue(pImp->EmptyQueue);
            if (NULL == ppBuffer)
            {
                LOG(SF_LOG_INFO, "No more buffer in input port\r\n");
                break;
            }
            pOMXBuffer = *ppBuffer;
            if(!pOMXBuffer)
                continue;
            LOG(SF_LOG_INFO, "Flush Buffer %p\r\n", pOMXBuffer);
            pOMXBuffer->nFlags = OMX_BUFFERFLAG_EOS;
            EmptyBufferDone(pSfOMXComponent, pOMXBuffer);
        }
        break;
    case OMX_OUTPUT_PORT_INDEX:
        while (OMX_TRUE)
        {
            ppBuffer = SF_Queue_Dequeue(pImp->FillQueue);
            if (NULL == ppBuffer)
            {
                LOG(SF_LOG_INFO, "No more buffer in output port\r\n");
                break;
            }
            pOMXBuffer = *ppBuffer;
            if(!pOMXBuffer)
                continue;
            LOG(SF_LOG_INFO, "Flush Buffer %p\r\n", pOMXBuffer);
            pOMXBuffer->nFilledLen = 0;
            pOMXBuffer->nFlags = OMX_BUFFERFLAG_EOS;
            FillBufferDone(pSfOMXComponent, pOMXBuffer);
        }
        break;
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
    SF_OMX_COMPONENT *pSfOMXComponent = (SF_OMX_COMPONENT *)pOMXComponent->pComponentPrivate;
    SF_WAVE420L_IMPLEMEMT *pImp = (SF_WAVE420L_IMPLEMEMT *)pSfOMXComponent->componentImpl;

    LOG(SF_LOG_DEBUG, "bufheader %p nFilledLen = %d, nFlags = %d, pBuffer = %p\r\n", pBuffer, pBuffer->nFilledLen, pBuffer->nFlags, pBuffer->pBuffer);

    ret = SF_Queue_Enqueue(pImp->EmptyQueue, &pBuffer);
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
    SF_WAVE420L_IMPLEMEMT *pImp = (SF_WAVE420L_IMPLEMEMT *)pSfOMXComponent->componentImpl;
    LOG(SF_LOG_DEBUG, "bufheader %p nFilledLen = %d, nFlags = %d, pBuffer = %p\r\n", pBuffer, pBuffer->nFilledLen, pBuffer->nFlags, pBuffer->pBuffer);

    ret = SF_Queue_Enqueue(pImp->FillQueue, &pBuffer);
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
    OMX_BUFFERHEADERTYPE *temp_bufferHeader;
    SF_OMX_BUF_INFO *pBufInfo;
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
    pBufInfo->type = SF_BUFFER_NOMAL;

    if(nPortIndex == OMX_INPUT_PORT_INDEX)
        temp_bufferHeader->pInputPortPrivate = pBufInfo;
    else if(nPortIndex == OMX_OUTPUT_PORT_INDEX)
        temp_bufferHeader->pOutputPortPrivate = pBufInfo;

    *ppBufferHdr = temp_bufferHeader;

    FunctionOut();

    return OMX_ErrorNone;
}

static OMX_BOOL FillBufferDone(SF_OMX_COMPONENT *pSfOMXComponent, OMX_BUFFERHEADERTYPE *pBuffer)
{
    FunctionIn();

    pBuffer->nTimeStamp = (tmpCounter*1000000)/tmpFramerate;
    tmpCounter++;

    LOG(SF_LOG_PERF, "OMX finish one buffer, address = %p, size = %d, nTimeStamp = %d, nFlags = %X\r\n",
            pBuffer->pBuffer, pBuffer->nFilledLen, pBuffer->nTimeStamp, pBuffer->nFlags);

    pSfOMXComponent->callbacks->FillBufferDone(pSfOMXComponent->pOMXComponent, pSfOMXComponent->pAppData, pBuffer);
    FunctionOut();
    return OMX_TRUE;
}

static OMX_BOOL EmptyBufferDone(SF_OMX_COMPONENT *pSfOMXComponent, OMX_BUFFERHEADERTYPE *pBuffer)
{
    FunctionIn();
    LOG(SF_LOG_DEBUG, "EmptyBufferDone IN\r\n");
    LOG(SF_LOG_PERF, "OMX empty one buffer, address = %p, size = %d, nTimeStamp = %d, nFlags = %X\r\n",
        pBuffer->pBuffer, pBuffer->nFilledLen, pBuffer->nTimeStamp, pBuffer->nFlags);
    pSfOMXComponent->callbacks->EmptyBufferDone(pSfOMXComponent->pOMXComponent, pSfOMXComponent->pAppData, pBuffer);
    LOG(SF_LOG_DEBUG, "EmptyBufferDone OUT\r\n");

    FunctionOut();
    return OMX_TRUE;
}

static OMX_ERRORTYPE SF_OMX_AllocateBuffer(
    OMX_IN OMX_HANDLETYPE hComponent,
    OMX_INOUT OMX_BUFFERHEADERTYPE **ppBuffer,
    OMX_IN OMX_U32 nPortIndex,
    OMX_IN OMX_PTR pAppPrivate,
    OMX_IN OMX_U32 nSizeBytes)
{
    OMX_ERRORTYPE ret = OMX_ErrorNone;
    OMX_BUFFERHEADERTYPE *temp_bufferHeader;
    SF_OMX_BUF_INFO *pBufInfo;

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

    if (nPortIndex == OMX_OUTPUT_PORT_INDEX)
    {
        temp_bufferHeader->pBuffer = malloc(nSizeBytes);
        memset(temp_bufferHeader->pBuffer, 0, nSizeBytes);
        temp_bufferHeader->nOutputPortIndex = outputBufNum;
        pBufInfo->type = SF_BUFFER_NOMAL;
        temp_bufferHeader->pOutputPortPrivate = (void*)pBufInfo;
        LOG(SF_LOG_PERF, "alloc normal output buffer %d \r\n", outputBufNum);
        outputBufNum ++;
    }
    else if (nPortIndex == OMX_INPUT_PORT_INDEX)
    {
        temp_bufferHeader->pBuffer = malloc(nSizeBytes);
        memset(temp_bufferHeader->pBuffer, 0, nSizeBytes);
        temp_bufferHeader->nInputPortIndex = inputBufNum;
        pBufInfo->type = SF_BUFFER_NOMAL;
        temp_bufferHeader->pInputPortPrivate = (void*)pBufInfo;
        LOG(SF_LOG_PERF, "alloc normal intput buffer %d \r\n", inputBufNum);
        inputBufNum ++;
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

static OMX_ERRORTYPE SF_OMX_FreeBuffer(
    OMX_IN OMX_HANDLETYPE hComponent,
    OMX_IN OMX_U32 nPortIndex,
    OMX_IN OMX_BUFFERHEADERTYPE *pBufferHdr)
{
    SF_OMX_BUF_INFO *pBufInfo;
    FunctionIn();

    if(pBufferHdr == NULL)
        return OMX_ErrorBadParameter;

    if(nPortIndex == OMX_INPUT_PORT_INDEX)
        pBufInfo = pBufferHdr->pInputPortPrivate;
    else if(nPortIndex == OMX_OUTPUT_PORT_INDEX)
        pBufInfo = pBufferHdr->pOutputPortPrivate;

    if(pBufInfo->type == SF_BUFFER_NOMAL)
    {
        if(pBufferHdr->pBuffer)
            free(pBufferHdr->pBuffer);
        free(pBufferHdr);
        free(pBufInfo);
    }

    FunctionOut();
    return OMX_ErrorNone;
}

static OMX_ERRORTYPE SF_OMX_GetParameter(
    OMX_IN OMX_HANDLETYPE hComponent,
    OMX_IN OMX_INDEXTYPE nParamIndex,
    OMX_INOUT OMX_PTR ComponentParameterStructure)
{
    OMX_ERRORTYPE ret = OMX_ErrorNone;
    OMX_COMPONENTTYPE *pOMXComponent = (OMX_COMPONENTTYPE *)hComponent;
    SF_OMX_COMPONENT *pSfOMXComponent = pOMXComponent->pComponentPrivate;
    SF_WAVE420L_IMPLEMEMT *pImp = (SF_WAVE420L_IMPLEMEMT *)pSfOMXComponent->componentImpl;
    OMX_PARAM_PORTDEFINITIONTYPE *pOutputPort = &pSfOMXComponent->portDefinition[OMX_OUTPUT_PORT_INDEX];

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
        portParam->nPorts           = OMX_PORT_MAX;
        portParam->nStartPortNumber = OMX_INPUT_PORT_INDEX;
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
            portFormat->eColorFormat = OMX_COLOR_FormatYUV420SemiPlanar;
            break;
        case 1:
            portFormat->eColorFormat = OMX_COLOR_FormatYUV420Planar;
            break;
        case 2:
            portFormat->eColorFormat = OMX_COLOR_FormatYVU420SemiPlanar;
            break;
        default:
            if (index > 0)
            {
                ret = OMX_ErrorNoMore;
            }
            break;
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
        memcpy(pPortDefinition, &pSfOMXComponent->portDefinition[portIndex], pPortDefinition->nSize);
        LOG(SF_LOG_DEBUG, "Get parameter port %X\r\n",portIndex);
        LOG_APPEND(SF_LOG_DEBUG, "width = %d, height = %d\r\n", pPortDefinition->format.video.nFrameWidth, pPortDefinition->format.video.nFrameHeight);
        LOG_APPEND(SF_LOG_DEBUG, "eColorFormat = %d\r\n", pPortDefinition->format.video.eColorFormat);
        LOG_APPEND(SF_LOG_DEBUG, "xFramerate = %d\r\n", pPortDefinition->format.video.xFramerate);
        LOG_APPEND(SF_LOG_DEBUG, "bufferSize = %d\r\n",pPortDefinition->nBufferSize);
        LOG_APPEND(SF_LOG_DEBUG, "Buffer count = %d\r\n", pPortDefinition->nBufferCountActual);
        break;
    }

    case OMX_IndexParamVideoIntraRefresh:
        break;

    case OMX_IndexParamStandardComponentRole:
        break;

    case OMX_IndexParamVideoAvc:
        break;

    case OMX_IndexParamVideoHevc:
    {
        OMX_VIDEO_PARAM_HEVCTYPE *pDstHEVCComponent = (OMX_VIDEO_PARAM_HEVCTYPE *)ComponentParameterStructure;
        OMX_U32 nPortIndex = pDstHEVCComponent->nPortIndex;
        OMX_VIDEO_PARAM_HEVCTYPE *pSrcHEVCComponent = &pImp->HEVCComponent[nPortIndex];

        LOG(SF_LOG_INFO, "Get nKeyFrameInterval = %d on port %d\r\n", pSrcHEVCComponent->nKeyFrameInterval, nPortIndex);
        memcpy(pDstHEVCComponent, pSrcHEVCComponent, sizeof(OMX_VIDEO_PARAM_HEVCTYPE));
        break;
    }

    case OMX_IndexParamVideoProfileLevelQuerySupported:
        break;

    case OMX_IndexParamVideoProfileLevelCurrent:
    {
        OMX_VIDEO_PARAM_PROFILELEVELTYPE *pParam = (OMX_VIDEO_PARAM_PROFILELEVELTYPE *)ComponentParameterStructure;
        LOG(SF_LOG_INFO, "Get ProfileLevel on port %d\r\n", pParam->nPortIndex);
        if (pParam->nPortIndex == OMX_OUTPUT_PORT_INDEX)
        {
            //TODO
            pParam->eProfile = OMX_VIDEO_HEVCProfileMain;
            pParam->eLevel = OMX_VIDEO_HEVCMainTierLevel5;
            LOG(SF_LOG_INFO, "eProfile = OMX_VIDEO_HEVCProfileMain\r\n");
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
    SF_WAVE420L_IMPLEMEMT *pImp = (SF_WAVE420L_IMPLEMEMT *)pSfOMXComponent->componentImpl;
    OMX_PARAM_PORTDEFINITIONTYPE *pInputPort = &pSfOMXComponent->portDefinition[OMX_INPUT_PORT_INDEX];
    OMX_PARAM_PORTDEFINITIONTYPE *pOutputPort = &pSfOMXComponent->portDefinition[OMX_OUTPUT_PORT_INDEX];

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
        OMX_U32 bufFrameWidth, bufFrameHeight, bufFrameStride;
        OMX_U32 nBufferCountActual = pPortDefinition->nBufferCountActual;
        LOG(SF_LOG_INFO, "Set width = %d, height = %d, xFramerate = %d, nBitrate = %d, nBufferCountActual = %d on port %d\r\n",
            width, height, xFramerate, nBitrate, nBufferCountActual, portIndex);
        OMX_COLOR_FORMATTYPE eColorFormat = pPortDefinition->format.video.eColorFormat;

        if (xFramerate == 0)
        {
            LOG(SF_LOG_ERR, "Fail to set xFramerate = %d\r\n", xFramerate);
            return OMX_ErrorBadParameter;
        }
        tmpFramerate = xFramerate;
        if (portIndex == OMX_INPUT_PORT_INDEX)
        {
            switch (eColorFormat)
            {
            case OMX_COLOR_FormatYUV420Planar:
                pImp->encConfig.cbcrInterleave = OMX_FALSE;
                pImp->encConfig.nv21 = OMX_FALSE;
                pImp->encConfig.srcFormat = FORMAT_420;
                pImp->encConfig.packedFormat = NOT_PACKED;
                break;
            case OMX_COLOR_FormatYUV420SemiPlanar:
                pImp->encConfig.cbcrInterleave = OMX_TRUE;
                pImp->encConfig.nv21 = OMX_FALSE;
                pImp->encConfig.srcFormat = FORMAT_420;
                pImp->encConfig.packedFormat = NOT_PACKED;
                break;
            case OMX_COLOR_FormatYVU420SemiPlanar:
                pImp->encConfig.cbcrInterleave = OMX_TRUE;
                pImp->encConfig.nv21 = OMX_TRUE;
                pImp->encConfig.srcFormat = FORMAT_420;
                pImp->encConfig.packedFormat = NOT_PACKED;
                break;
            default:
                ret = OMX_ErrorBadParameter;
                break;
            }
            if(ret)
                return ret;

            memcpy(pInputPort, pPortDefinition, pPortDefinition->nSize);
            pImp->encConfig.picWidth = width;
            pImp->encConfig.picHeight = height;
            bufFrameWidth  = ((width+7)&~7);        // width = 8-aligned (CU unit)
            bufFrameHeight = ((height+7)&~7);       // height = 8-aligned (CU unit)
            bufFrameStride = Warp_CalcStride(pImp, bufFrameWidth, bufFrameHeight, pImp->encConfig.srcFormat,
                    pImp->encConfig.cbcrInterleave, LINEAR_FRAME_MAP, FALSE);

            pInputPort->nBufferSize = Warp_VPU_GetFrameBufSize(pImp, pImp->coreIdx, bufFrameStride, bufFrameHeight,
                    LINEAR_FRAME_MAP, pImp->encConfig.srcFormat, pImp->encConfig.cbcrInterleave, NULL);

            LOG(SF_LOG_INFO, "alloc buff size %d on port %d\r\n",
                    pInputPort->nBufferSize, portIndex);
        }
        else if (portIndex == OMX_OUTPUT_PORT_INDEX)
        {
            memcpy(pOutputPort, pPortDefinition, pPortDefinition->nSize);
            pOutputPort->format.video.nStride = width;
            pOutputPort->format.video.nSliceHeight = height;
            pOutputPort->nBufferSize = width * height * 2;
        }
        break;
    }

    case OMX_IndexParamVideoPortFormat:
    {
        OMX_VIDEO_PARAM_PORTFORMATTYPE *portFormat = (OMX_VIDEO_PARAM_PORTFORMATTYPE *)ComponentParameterStructure;
        OMX_COLOR_FORMATTYPE eColorFormat = portFormat->eColorFormat;
        LOG(SF_LOG_INFO, "Set eColorFormat to %d\r\n", eColorFormat);
        switch (eColorFormat)
        {
        case OMX_COLOR_FormatYUV420Planar:
            pImp->encConfig.cbcrInterleave = OMX_FALSE;
            pImp->encConfig.nv21 = OMX_FALSE;
            pImp->encConfig.srcFormat = FORMAT_420;
            pImp->encConfig.packedFormat = NOT_PACKED;
            break;
        case OMX_COLOR_FormatYUV420SemiPlanar:
            pImp->encConfig.cbcrInterleave = OMX_TRUE;
            pImp->encConfig.nv21 = OMX_FALSE;
            pImp->encConfig.srcFormat = FORMAT_420;
            pImp->encConfig.packedFormat = NOT_PACKED;
            break;
        case OMX_COLOR_FormatYVU420SemiPlanar:
            pImp->encConfig.cbcrInterleave = OMX_TRUE;
            pImp->encConfig.nv21 = OMX_TRUE;
            pImp->encConfig.srcFormat = FORMAT_420;
            pImp->encConfig.packedFormat = NOT_PACKED;
            break;
        default:
            ret = OMX_ErrorBadParameter;
            break;
        }
        if(!ret){
            pInputPort->format.video.eColorFormat = eColorFormat;
        }
        break;
    }

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
        break;
    }

    case OMX_IndexConfigVideoAVCIntraPeriod:
    {
        break;
    }

    case OMX_IndexParamVideoAvc:
    {
        break;
    }

    case OMX_IndexParamVideoHevc:
    {
        OMX_VIDEO_PARAM_HEVCTYPE *pSrcHEVCComponent= (OMX_VIDEO_PARAM_HEVCTYPE *)ComponentParameterStructure;
        OMX_U32 nPortIndex = pSrcHEVCComponent->nPortIndex;
        OMX_VIDEO_PARAM_HEVCTYPE *pDstHEVCComponent = &pImp->HEVCComponent[nPortIndex];
        OMX_U32 nKeyFrameInterval = pSrcHEVCComponent->nKeyFrameInterval;

        LOG(SF_LOG_INFO, "Set nKeyFrameInterval = %d on port %d\r\n", nKeyFrameInterval, nPortIndex);
        if (nKeyFrameInterval == 1)
        {
            LOG(SF_LOG_ERR, "Set nKeyFrameInterval = %d not supported!\r\n", nKeyFrameInterval);
            ret = OMX_ErrorBadPortIndex;
            goto EXIT;
        }
        memcpy(pDstHEVCComponent, pSrcHEVCComponent, sizeof(OMX_VIDEO_PARAM_HEVCTYPE));
        break;
    }

    case OMX_IndexParamVideoQuantization:
    case OMX_IndexParamVideoIntraRefresh:
    default:
        break;
    }

EXIT:
    FunctionOut();

    return ret;
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
    SF_WAVE420L_IMPLEMEMT *pImp = (SF_WAVE420L_IMPLEMEMT *)pSfOMXComponent->componentImpl;
    SF_OMX_CMD cmd;

    FunctionIn();
    if (hComponent == NULL || pSfOMXComponent == NULL)
    {
        ret = OMX_ErrorBadParameter;
        goto EXIT;
    }

    LOG(SF_LOG_INFO, "cmd = %X, nParam = %X\r\n", Cmd, nParam);

    cmd.Cmd = Cmd;
    cmd.nParam = nParam;
    cmd.pCmdData = pCmdData;

    ret = SF_Queue_Enqueue(pImp->CmdQueue, &cmd);

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
    SF_WAVE420L_IMPLEMEMT *pImp = (SF_WAVE420L_IMPLEMEMT *)pSfOMXComponent->componentImpl;
    FunctionIn();
    *pState = pImp->currentState;
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

static OMX_ERRORTYPE SF_OMX_ComponentConstructor(SF_OMX_COMPONENT *pSfOMXComponent)
{
    OMX_ERRORTYPE ret = OMX_ErrorNone;
    SF_WAVE420L_IMPLEMEMT *pImp;
    FunctionIn();

    ret = WaveOmxInit(pSfOMXComponent);
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
    pImp = (SF_WAVE420L_IMPLEMEMT *)pSfOMXComponent->componentImpl;
    pImp->currentState = OMX_StateLoaded;
    InitEncoder(pSfOMXComponent);
EXIT:
    FunctionOut();

    return ret;
}

static OMX_ERRORTYPE SF_OMX_ComponentClear(SF_OMX_COMPONENT *pSfOMXComponent)
{
    OMX_ERRORTYPE ret = OMX_ErrorNone;
    SF_WAVE420L_IMPLEMEMT *pImp = (SF_WAVE420L_IMPLEMEMT *)pSfOMXComponent->componentImpl;
    SF_OMX_CMD cmd;
    void *pNull = NULL;
    void *ThreadRet = NULL;

    FunctionIn();

    if(pImp->bThreadRunning)
    {
        pImp->bThreadRunning = 0;
        /* enqueue null mean encoder thread cycle end */
        SF_Queue_Enqueue(pImp->EmptyQueue, &pNull);
        SF_Queue_Enqueue(pImp->FillQueue, &pNull);
        pthread_join(pImp->pProcessThread->pthread, &ThreadRet);
        LOG(SF_LOG_INFO, "Encoder thread end %ld\r\n", (Uint64)ThreadRet);
        SF_Queue_Flush(pImp->EmptyQueue);
        SF_Queue_Flush(pImp->FillQueue);
    }

    pImp->bCmdRunning = 0;
    /* enqueue OMX_Command_StopThread mean cmd thread cycle end */
    cmd.Cmd = OMX_Command_StopThread;
    SF_Queue_Enqueue(pImp->CmdQueue, &cmd);
    pthread_cancel(pImp->pCmdThread->pthread);
	pthread_join(pImp->pCmdThread->pthread, &ThreadRet);
    LOG(SF_LOG_INFO, "Cmd thread end %ld\r\n", (Uint64)ThreadRet);

    Warp_VPU_DeInit(pImp, pImp->coreIdx);

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
    .fwPath = "/lib/firmware/monet.bin",
    .componentRule = "video_encoder.hevc"};
