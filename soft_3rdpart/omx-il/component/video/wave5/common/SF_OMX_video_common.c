// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2021 StarFive Technology Co., Ltd.
 */
#include "SF_OMX_video_common.h"

extern SF_OMX_COMPONENT *sf_omx_component_list[];
static void sf_get_component_functions(SF_COMPONENT_FUNCTIONS *funcs, OMX_PTR *sohandle);

OMX_ERRORTYPE GetStateCommon(OMX_IN OMX_HANDLETYPE hComponent, OMX_OUT OMX_STATETYPE *pState)
{
    OMX_ERRORTYPE ret = OMX_ErrorNone;
    OMX_COMPONENTTYPE *pOMXComponent = NULL;
    SF_OMX_COMPONENT *pSfOMXComponent = NULL;
    ComponentState state;
    OMX_STATETYPE nextState;
    SF_WAVE5_IMPLEMEMT *pSfVideoImplement = NULL;
    FunctionIn();
    if (hComponent == NULL)
    {
        ret = OMX_ErrorBadParameter;
        goto EXIT;
    }
    pOMXComponent = (OMX_COMPONENTTYPE *)hComponent;
    pSfOMXComponent = pOMXComponent->pComponentPrivate;
    pSfVideoImplement = (SF_WAVE5_IMPLEMEMT *)pSfOMXComponent->componentImpl;
    nextState = pSfOMXComponent->nextState;
    state = pSfVideoImplement->functions->ComponentGetState(pSfVideoImplement->hSFComponentExecoder);
    LOG(SF_LOG_INFO, "state = %d\r\n", state);

    switch (state)
    {
    case COMPONENT_STATE_CREATED:
        *pState = OMX_StateIdle;
        break;
    case COMPONENT_STATE_NONE:
    case COMPONENT_STATE_TERMINATED:
        *pState = OMX_StateLoaded;
        break;
    case COMPONENT_STATE_PREPARED:
    case COMPONENT_STATE_EXECUTED:
        if (nextState == OMX_StateIdle || nextState == OMX_StateExecuting || nextState == OMX_StatePause)
        {
            *pState = nextState;
        }
        break;
    default:
        LOG(SF_LOG_WARN, "unknown state:%d \r\n", state);
        ret = OMX_ErrorUndefined;
        break;
    }
EXIT:
    FunctionOut();
    return ret;
}

OMX_ERRORTYPE ComponentClearCommon(SF_OMX_COMPONENT *pSfOMXComponent)
{
    SF_WAVE5_IMPLEMEMT *pSfVideoImplement = (SF_WAVE5_IMPLEMEMT *)pSfOMXComponent->componentImpl;

    pSfVideoImplement->functions->ComponentRelease(pSfVideoImplement->hSFComponentExecoder);
    pSfVideoImplement->functions->ComponentDestroy(pSfVideoImplement->hSFComponentExecoder, NULL);
    pSfVideoImplement->functions->DeInitLog();
    dlclose(pSfOMXComponent->soHandle);
    free(pSfVideoImplement->functions);
    free(pSfVideoImplement->testConfig);
    free(pSfVideoImplement->config);
    free(pSfVideoImplement->lsnCtx);
    free(pSfOMXComponent->pOMXComponent);
    for (int i = 0; i < 2; i++)
    {
        OMX_PARAM_PORTDEFINITIONTYPE *pPortDefinition = &pSfOMXComponent->portDefinition[i];
        free(pPortDefinition->format.video.cMIMEType);
    }
    return OMX_ErrorNone;
}

BOOL CheckEncTestConfig(TestEncConfig *testConfig)
{
    FunctionIn();
    if ((testConfig->compareType & (1 << MODE_SAVE_ENCODED)) && testConfig->bitstreamFileName[0] == 0)
    {
        testConfig->compareType &= ~(1 << MODE_SAVE_ENCODED);
        LOG(SF_LOG_ERR, "You want to Save bitstream data. Set the path\r\n");
        return FALSE;
    }

    if ((testConfig->compareType & (1 << MODE_COMP_ENCODED)) && testConfig->ref_stream_path[0] == 0)
    {
        testConfig->compareType &= ~(1 << MODE_COMP_ENCODED);
        LOG(SF_LOG_ERR, "You want to Compare bitstream data. Set the path\r\n");
        return FALSE;
    }

    FunctionOut();
    return TRUE;
}

BOOL CheckDecTestConfig(TestDecConfig *testConfig)
{
    BOOL isValidParameters = TRUE;

    /* Check parameters */
    if (testConfig->skipMode < 0 || testConfig->skipMode == 3 || testConfig->skipMode > 4)
    {
        LOG(SF_LOG_WARN, "Invalid skip mode: %d\n", testConfig->skipMode);
        isValidParameters = FALSE;
    }
    if ((FORMAT_422 < testConfig->wtlFormat && testConfig->wtlFormat <= FORMAT_400) ||
        testConfig->wtlFormat < FORMAT_420 || testConfig->wtlFormat >= FORMAT_MAX)
    {
        LOG(SF_LOG_WARN, "Invalid WTL format(%d)\n", testConfig->wtlFormat);
        isValidParameters = FALSE;
    }
    if (isValidParameters == TRUE && (testConfig->scaleDownWidth > 0 || testConfig->scaleDownHeight > 0))
    {
    }
    if (testConfig->renderType < RENDER_DEVICE_NULL || testConfig->renderType >= RENDER_DEVICE_MAX)
    {
        LOG(SF_LOG_WARN, "unknown render device type(%d)\n", testConfig->renderType);
        isValidParameters = FALSE;
    }
    if (testConfig->thumbnailMode == TRUE && testConfig->skipMode != 0)
    {
        LOG(SF_LOG_WARN, "Turn off thumbnail mode or skip mode\n");
        isValidParameters = FALSE;
    }

    return isValidParameters;
}

OMX_ERRORTYPE InitComponentStructorCommon(SF_OMX_COMPONENT *pSfOMXComponent)
{
    OMX_ERRORTYPE ret = OMX_ErrorNone;
    char *strDebugLevel = NULL;
    int debugLevel = 0;
    SF_WAVE5_IMPLEMEMT *pSfVideoImplement = NULL;

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
        LOG(SF_LOG_ERR, "could not open %s\r\n", pSfOMXComponent->libName);
        goto ERROR;
    }

    pSfOMXComponent->componentImpl = malloc(sizeof(SF_WAVE5_IMPLEMEMT));
    if (pSfOMXComponent->componentImpl == NULL)
    {
        ret = OMX_ErrorInsufficientResources;
        LOG(SF_LOG_ERR, "malloc fail\r\n");
        goto ERROR;
    }
    pSfVideoImplement = (SF_WAVE5_IMPLEMEMT *)pSfOMXComponent->componentImpl;
    memset(pSfVideoImplement, 0, sizeof(SF_WAVE5_IMPLEMEMT));

    pSfVideoImplement->functions = malloc(sizeof(SF_COMPONENT_FUNCTIONS));
    if (pSfVideoImplement->functions == NULL)
    {
        ret = OMX_ErrorInsufficientResources;
        LOG(SF_LOG_ERR, "malloc fail\r\n");
        goto ERROR;
    }
    memset(pSfVideoImplement->functions, 0, sizeof(SF_COMPONENT_FUNCTIONS));
    sf_get_component_functions(pSfVideoImplement->functions, pSfOMXComponent->soHandle);

    // Init VPU log
    if (pSfVideoImplement->functions->InitLog && pSfVideoImplement->functions->SetMaxLogLevel)
    {
        pSfVideoImplement->functions->InitLog();
        strDebugLevel = getenv("VPU_DEBUG");
        if (strDebugLevel)
        {
            debugLevel = atoi(strDebugLevel);
            if (debugLevel >=0)
            {
                pSfVideoImplement->functions->SetMaxLogLevel(debugLevel);
            }
        }
    }

    if (strstr(pSfOMXComponent->componentName, "sf.video_encoder") != NULL)
    {
        pSfVideoImplement->testConfig = malloc(sizeof(TestEncConfig));
        if (pSfVideoImplement->testConfig == NULL)
        {
            ret = OMX_ErrorInsufficientResources;
            LOG(SF_LOG_ERR, "malloc fail\r\n");
            goto ERROR;
        }
        pSfVideoImplement->lsnCtx = malloc(sizeof(EncListenerContext));
        if (pSfVideoImplement->lsnCtx == NULL)
        {
            ret = OMX_ErrorInsufficientResources;
            LOG(SF_LOG_ERR, "malloc fail\r\n");
            goto ERROR;
        }
        memset(pSfVideoImplement->testConfig, 0, sizeof(TestEncConfig));
        memset(pSfVideoImplement->lsnCtx, 0, sizeof(EncListenerContext));
        pSfVideoImplement->functions->SetDefaultEncTestConfig(pSfVideoImplement->testConfig);
    }
    else if (strstr(pSfOMXComponent->componentName, "sf.video_decoder") != NULL)
    {
        pSfVideoImplement->testConfig = malloc(sizeof(TestDecConfig));
        if (pSfVideoImplement->testConfig == NULL)
        {
            ret = OMX_ErrorInsufficientResources;
            LOG(SF_LOG_ERR, "malloc fail\r\n");
            goto ERROR;
        }
        pSfVideoImplement->lsnCtx = malloc(sizeof(DecListenerContext));
        if (pSfVideoImplement->lsnCtx == NULL)
        {
            ret = OMX_ErrorInsufficientResources;
            LOG(SF_LOG_ERR, "malloc fail\r\n");
            goto ERROR;
        }
        memset(pSfVideoImplement->testConfig, 0, sizeof(TestDecConfig));
        memset(pSfVideoImplement->lsnCtx, 0, sizeof(DecListenerContext));
        pSfVideoImplement->functions->SetDefaultDecTestConfig(pSfVideoImplement->testConfig);
    }
    else
    {
        ret = OMX_ErrorBadParameter;
        LOG(SF_LOG_ERR, "unknown component!\r\n");
        goto ERROR;
    }

    if (strstr(pSfOMXComponent->componentName, "avc") != NULL)
    {
        pSfVideoImplement->bitFormat = STD_AVC;
    }
    else if (strstr(pSfOMXComponent->componentName, "hevc") != NULL)
    {
        pSfVideoImplement->bitFormat = STD_HEVC;
    }
    else
    {
        ret = OMX_ErrorBadParameter;
        LOG(SF_LOG_ERR, "unknown format!\r\n");
        goto ERROR;
    }

    pSfVideoImplement->config = malloc(sizeof(CNMComponentConfig));
    if (pSfVideoImplement->config == NULL)
    {
        ret = OMX_ErrorInsufficientResources;
        LOG(SF_LOG_ERR, "malloc fail\r\n");
        goto ERROR;
    }
    memset(pSfVideoImplement->config, 0, sizeof(CNMComponentConfig));

    pSfOMXComponent->pOMXComponent->pComponentPrivate = pSfOMXComponent;
    for (int i = 0; i < 2; i++)
    {
        OMX_PARAM_PORTDEFINITIONTYPE *pPortDefinition = &pSfOMXComponent->portDefinition[i];
        OMX_VIDEO_PARAM_AVCTYPE *pAVCComponent = &pSfVideoImplement->AVCComponent[i];
        OMX_VIDEO_PARAM_HEVCTYPE *pHEVCComponent = &pSfVideoImplement->HEVCComponent[i];
        INIT_SET_SIZE_VERSION(pPortDefinition, OMX_PARAM_PORTDEFINITIONTYPE);
        INIT_SET_SIZE_VERSION(pAVCComponent, OMX_VIDEO_PARAM_AVCTYPE);
        INIT_SET_SIZE_VERSION(pHEVCComponent, OMX_VIDEO_PARAM_HEVCTYPE);
        pPortDefinition->nPortIndex = i;
        pPortDefinition->nBufferCountActual = VPU_OUTPUT_BUF_NUMBER;
        pPortDefinition->nBufferCountMin = VPU_OUTPUT_BUF_NUMBER;
        pPortDefinition->nBufferSize = 0;
        pPortDefinition->eDomain = OMX_PortDomainVideo;
        pPortDefinition->format.video.nFrameWidth = DEFAULT_FRAME_WIDTH;
        pPortDefinition->format.video.nFrameHeight = DEFAULT_FRAME_HEIGHT;
        pPortDefinition->format.video.nStride = DEFAULT_FRAME_WIDTH;
        pPortDefinition->format.video.nSliceHeight = DEFAULT_FRAME_HEIGHT;
        pPortDefinition->format.video.eCompressionFormat = OMX_VIDEO_CodingUnused;
        pPortDefinition->format.video.cMIMEType = malloc(OMX_MAX_STRINGNAME_SIZE);
        pPortDefinition->format.video.xFramerate = 30;
        if (pPortDefinition->format.video.cMIMEType == NULL)
        {
            ret = OMX_ErrorInsufficientResources;
            free(pPortDefinition->format.video.cMIMEType);
            pPortDefinition->format.video.cMIMEType = NULL;
            LOG(SF_LOG_ERR, "malloc fail\r\n");
            goto ERROR;
        }
        memset(pPortDefinition->format.video.cMIMEType, 0, OMX_MAX_STRINGNAME_SIZE);
        pPortDefinition->format.video.pNativeRender = 0;
        pPortDefinition->format.video.bFlagErrorConcealment = OMX_FALSE;
        pPortDefinition->format.video.eColorFormat = OMX_COLOR_FormatUnused;
        pPortDefinition->bEnabled = OMX_TRUE;

        pAVCComponent->nPortIndex = i;
        pAVCComponent->nPFrames = 30;
        pAVCComponent->eProfile = OMX_VIDEO_AVCProfileHigh;
        pHEVCComponent->nPortIndex = i;
        pHEVCComponent->nKeyFrameInterval = 30;
        pHEVCComponent->eProfile = OMX_VIDEO_HEVCProfileMain;
    }

    pSfOMXComponent->portDefinition[0].eDir = OMX_DirInput;
    pSfOMXComponent->portDefinition[0].nBufferSize = DEFAULT_VIDEO_INPUT_BUFFER_SIZE;
    pSfOMXComponent->portDefinition[0].nBufferCountActual = VPU_INPUT_BUF_NUMBER;
    pSfOMXComponent->portDefinition[0].nBufferCountMin = VPU_INPUT_BUF_NUMBER;

    strcpy(pSfOMXComponent->portDefinition[1].format.video.cMIMEType, "raw/video");
    pSfOMXComponent->portDefinition[1].format.video.eColorFormat = OMX_COLOR_FormatYUV420SemiPlanar;
    pSfOMXComponent->portDefinition[1].eDir = OMX_DirOutput;
    pSfOMXComponent->portDefinition[1].nBufferSize = DEFAULT_VIDEO_OUTPUT_BUFFER_SIZE;
    pSfOMXComponent->portDefinition[1].nBufferCountActual = VPU_OUTPUT_BUF_NUMBER;
    pSfOMXComponent->portDefinition[1].nBufferCountMin = VPU_OUTPUT_BUF_NUMBER;

    memset(pSfOMXComponent->pBufferArray, 0, sizeof(pSfOMXComponent->pBufferArray));
    pSfOMXComponent->memory_optimization = OMX_TRUE;

    FunctionOut();
EXIT:
    return ret;
ERROR:
    if (pSfOMXComponent->pOMXComponent)
    {
        free(pSfOMXComponent->pOMXComponent);
        pSfOMXComponent->pOMXComponent = NULL;
    }
    if (pSfVideoImplement)
    {
        if (pSfVideoImplement->functions)
        {
            free(pSfVideoImplement->functions);
            pSfVideoImplement->functions = NULL;
        }
        if (pSfVideoImplement->testConfig)
        {
            free(pSfVideoImplement->testConfig);
            pSfVideoImplement->testConfig = NULL;
        }
        if (pSfVideoImplement->lsnCtx)
        {
            free(pSfVideoImplement->lsnCtx);
            pSfVideoImplement->lsnCtx = NULL;
        }
        if (pSfVideoImplement->config)
        {
            free(pSfVideoImplement->config);
            pSfVideoImplement->config = NULL;
        }
        free(pSfVideoImplement);
        pSfVideoImplement = NULL;
    }
    return ret;
}

OMX_ERRORTYPE FlushBuffer(SF_OMX_COMPONENT *pSfOMXComponent, OMX_U32 nPort)
{
    SF_WAVE5_IMPLEMEMT *pSfVideoImplement = (SF_WAVE5_IMPLEMEMT *)pSfOMXComponent->componentImpl;

    FunctionIn();
    if (nPort == 0)
    {
        ComponentImpl *pFeederComponent = (ComponentImpl *)(pSfVideoImplement->hSFComponentFeeder);
        OMX_U32 inputQueueCount = pSfVideoImplement->functions->Queue_Get_Cnt(pFeederComponent->srcPort.inputQ);
        LOG(SF_LOG_PERF, "Flush %d buffers on inputPort\r\n", inputQueueCount);
        if (inputQueueCount > 0)
        {
            PortContainerExternal *input = NULL;
            while ((input = (PortContainerExternal*)pSfVideoImplement->functions->ComponentPortGetData(&pFeederComponent->srcPort)) != NULL)
            {
                if (strstr(pSfOMXComponent->componentName, "OMX.sf.video_decoder") != NULL)
                {
                    pSfVideoImplement->functions->ComponentNotifyListeners(pFeederComponent, COMPONENT_EVENT_DEC_EMPTY_BUFFER_DONE, (void *)input);
                }
                else if (strstr(pSfOMXComponent->componentName, "OMX.sf.video_encoder") != NULL)
                {
                    pSfVideoImplement->functions->ComponentNotifyListeners(pFeederComponent, COMPONENT_EVENT_ENC_EMPTY_BUFFER_DONE, (void *)input);
                }
            }
        }
    }
    else if (nPort == 1)
    {
        ComponentImpl *pRendererComponent = (ComponentImpl *)(pSfVideoImplement->hSFComponentRender);
        OMX_U32 OutputQueueCount = pSfVideoImplement->functions->Queue_Get_Cnt(pRendererComponent->sinkPort.inputQ);
        LOG(SF_LOG_PERF, "Flush %d buffers on outputPort\r\n", OutputQueueCount);
        if (OutputQueueCount > 0)
        {
            PortContainerExternal *output = NULL;
            while ((output = (PortContainerExternal*)pSfVideoImplement->functions->ComponentPortGetData(&pRendererComponent->sinkPort)) != NULL)
            {
                output->nFlags = 0x1;
                output->nFilledLen = 0;
                if (strstr(pSfOMXComponent->componentName, "OMX.sf.video_decoder") != NULL)
                {
                    pSfVideoImplement->functions->ComponentNotifyListeners(pRendererComponent, COMPONENT_EVENT_DEC_FILL_BUFFER_DONE, (void *)output);
                }
                else if (strstr(pSfOMXComponent->componentName, "OMX.sf.video_encoder") != NULL)
                {
                    pSfVideoImplement->functions->ComponentNotifyListeners(pRendererComponent, COMPONENT_EVENT_ENC_FILL_BUFFER_DONE, (void *)output);
                }
            }
        }
    }
    FunctionOut();
    return OMX_ErrorNone;
}

static void sf_get_component_functions(SF_COMPONENT_FUNCTIONS *funcs, OMX_PTR *sohandle)
{
    FunctionIn();
    funcs->ComponentCreate = dlsym(sohandle, "ComponentCreate");
    funcs->ComponentExecute = dlsym(sohandle, "ComponentExecute");
    funcs->ComponentGetParameter = dlsym(sohandle, "ComponentGetParameter");
    funcs->ComponentGetState = dlsym(sohandle, "ComponentGetState");
    funcs->ComponentNotifyListeners = dlsym(sohandle, "ComponentNotifyListeners");
    funcs->ComponentPortCreate = dlsym(sohandle, "ComponentPortCreate");
    funcs->ComponentPortDestroy = dlsym(sohandle, "ComponentPortDestroy");
    funcs->ComponentPortFlush = dlsym(sohandle, "ComponentPortFlush");
    funcs->ComponentPortGetData = dlsym(sohandle, "ComponentPortGetData");
    funcs->ComponentPortPeekData = dlsym(sohandle, "ComponentPortPeekData");
    funcs->ComponentPortSetData = dlsym(sohandle, "ComponentPortSetData");
    funcs->ComponentPortWaitReadyStatus = dlsym(sohandle, "ComponentPortWaitReadyStatus");
    funcs->ComponentRelease = dlsym(sohandle, "ComponentRelease");
    funcs->ComponentSetParameter = dlsym(sohandle, "ComponentSetParameter");
    funcs->ComponentStop = dlsym(sohandle, "ComponentStop");
    funcs->ComponentSetupTunnel = dlsym(sohandle, "ComponentSetupTunnel");
    funcs->ComponentWait = dlsym(sohandle, "ComponentWait");
    funcs->WaitBeforeComponentPortGetData = dlsym(sohandle, "WaitBeforeComponentPortGetData");
    funcs->ComponentChangeState = dlsym(sohandle, "ComponentChangeState");
    funcs->ComponentDestroy = dlsym(sohandle, "ComponentDestroy");
    funcs->ComponentRegisterListener = dlsym(sohandle, "ComponentRegisterListener");
    funcs->ComponentPortHasInputData = dlsym(sohandle, "ComponentPortHasInputData");
    funcs->ComponentPortGetSize = dlsym(sohandle, "ComponentPortGetSize");
    funcs->ComponentParamReturnTest = dlsym(sohandle, "ComponentParamReturnTest");
    //Listener
    funcs->SetupDecListenerContext = dlsym(sohandle, "SetupDecListenerContext");
    funcs->SetupEncListenerContext = dlsym(sohandle, "SetupEncListenerContext");
    funcs->ClearDecListenerContext = dlsym(sohandle, "ClearDecListenerContext");
    funcs->HandleDecCompleteSeqEvent = dlsym(sohandle, "HandleDecCompleteSeqEvent");
    funcs->HandleDecRegisterFbEvent = dlsym(sohandle, "HandleDecRegisterFbEvent");
    funcs->HandleDecGetOutputEvent = dlsym(sohandle, "HandleDecGetOutputEvent");
    funcs->HandleDecCloseEvent = dlsym(sohandle, "HandleDecCloseEvent");
    funcs->ClearEncListenerContext = dlsym(sohandle, "ClearEncListenerContext");
    funcs->HandleEncFullEvent = dlsym(sohandle, "HandleEncFullEvent");
    funcs->HandleEncGetOutputEvent = dlsym(sohandle, "HandleEncGetOutputEvent");
    funcs->HandleEncCompleteSeqEvent = dlsym(sohandle, "HandleEncCompleteSeqEvent");
    funcs->HandleEncGetEncCloseEvent = dlsym(sohandle, "HandleEncGetEncCloseEvent");
    funcs->EncoderListener = dlsym(sohandle, "EncoderListener");
    funcs->DecoderListener = dlsym(sohandle, "DecoderListener");
    // Helper
    funcs->SetDefaultEncTestConfig = dlsym(sohandle, "SetDefaultEncTestConfig");
    funcs->SetDefaultDecTestConfig = dlsym(sohandle, "SetDefaultDecTestConfig");
    funcs->LoadFirmware = dlsym(sohandle, "LoadFirmware");
    funcs->SetupEncoderOpenParam = dlsym(sohandle, "SetupEncoderOpenParam");
    funcs->SetUpDecoderOpenParam = dlsym(sohandle, "SetUpDecoderOpenParam");
    // VPU
    funcs->VPU_GetProductId = dlsym(sohandle, "VPU_GetProductId");
    funcs->Queue_Enqueue = dlsym(sohandle, "Queue_Enqueue");
    funcs->Queue_Get_Cnt = dlsym(sohandle, "Queue_Get_Cnt");
    funcs->VPU_DecClrDispFlag = dlsym(sohandle, "VPU_DecClrDispFlag");
    funcs->VPU_DecGetFrameBuffer = dlsym(sohandle, "VPU_DecGetFrameBuffer");
    funcs->Render_DecClrDispFlag = dlsym(sohandle, "Render_DecClrDispFlag");
    // VPU Log
    funcs->InitLog = dlsym(sohandle, "InitLog");
    funcs->DeInitLog = dlsym(sohandle, "DeInitLog");
    funcs->SetMaxLogLevel = dlsym(sohandle, "SetMaxLogLevel");
    funcs->GetMaxLogLevel = dlsym(sohandle, "GetMaxLogLevel");

    //Renderer
    funcs->AllocateFrameBuffer2 = dlsym(sohandle, "AllocateFrameBuffer2");
    funcs->AttachDMABuffer = dlsym(sohandle, "AttachDMABuffer");
    funcs->SetRenderTotalBufferNumber = dlsym(sohandle, "SetRenderTotalBufferNumber");
    funcs->SetFeederTotalBufferNumber = dlsym(sohandle, "SetFeederTotalBufferNumber");
    funcs->WaitForExecoderReady = dlsym(sohandle, "WaitForExecoderReady");
    FunctionOut();
}

SF_OMX_COMPONENT *GetSFOMXComponrntByComponent(Component *pComponent)
{
    SF_OMX_COMPONENT *pSfOMXComponent = NULL;
    SF_WAVE5_IMPLEMEMT *pSfVideoImplement = NULL;
    int size = GetNumberOfComponent();

    FunctionIn();
    for (int i = 0; i < size; i++)
    {
        pSfOMXComponent = sf_omx_component_list[i];
        if (pSfOMXComponent == NULL)
        {
            continue;
        }
        pSfVideoImplement = (SF_WAVE5_IMPLEMEMT *)pSfOMXComponent->componentImpl;
        if (pSfVideoImplement == NULL)
        {
            continue;
        }
        if (pSfVideoImplement->hSFComponentExecoder == pComponent || pSfVideoImplement->hSFComponentFeeder == pComponent || pSfVideoImplement->hSFComponentRender == pComponent)
        {
            break;
        }
    }

    if (pSfOMXComponent == NULL)
    {
        LOG(SF_LOG_ERR, "Could not get SfOMXComponent buy %p\r\n", pComponent);
    }
    FunctionOut();

    return pSfOMXComponent;
}