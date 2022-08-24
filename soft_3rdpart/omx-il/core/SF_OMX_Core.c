// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2021 StarFive Technology Co., Ltd.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include "SF_OMX_Core.h"

static int gInitialized = 0;
OMX_TICKS gInitTimeStamp = 0;
int gDebugLevel = SF_LOG_ERR;
extern SF_OMX_COMPONENT sf_dec_decoder_h265;
extern SF_OMX_COMPONENT sf_dec_decoder_h264;
extern SF_OMX_COMPONENT sf_enc_encoder_h265;
//extern SF_OMX_COMPONENT sf_enc_encoder_h264;
extern SF_OMX_COMPONENT sf_dec_decoder_mjpeg;

SF_OMX_COMPONENT *sf_omx_component_list[] = {
    &sf_dec_decoder_h265,
    &sf_dec_decoder_h264,
    &sf_enc_encoder_h265,
 //   &sf_enc_encoder_h264,
    &sf_dec_decoder_mjpeg,
    NULL,
};

#define SF_OMX_COMPONENT_NUM ((sizeof(sf_omx_component_list) / sizeof(SF_OMX_COMPONENT *)) - 1)

int GetNumberOfComponent()
{
    return sizeof(sf_omx_component_list) / sizeof(SF_OMX_COMPONENT *);
}

OMX_ERRORTYPE SF_OMX_ComponentLoad(SF_OMX_COMPONENT *sf_component)
{
    OMX_ERRORTYPE ret = OMX_ErrorNone;

    FunctionIn();

    sf_component->SF_OMX_ComponentConstructor(sf_component);

    FunctionOut();

    return ret;
}

OMX_API OMX_ERRORTYPE OMX_APIENTRY OMX_Init(void)
{
    OMX_ERRORTYPE ret = OMX_ErrorNone;
    char *debugLevel = NULL;
    FunctionIn();
    if (gInitialized == 0)
    {
        gInitialized = 1;
    }

    debugLevel = getenv("OMX_DEBUG");
    if (debugLevel != NULL)
    {
        gDebugLevel = atoi(debugLevel);
    }

    FunctionOut();
    return ret;
}

OMX_API OMX_ERRORTYPE OMX_APIENTRY OMX_Deinit(void)
{
    OMX_ERRORTYPE ret = OMX_ErrorNone;

    FunctionIn();

    FunctionOut();
    return ret;
}

OMX_API OMX_ERRORTYPE OMX_APIENTRY OMX_ComponentNameEnum(
    OMX_OUT OMX_STRING cComponentName,
    OMX_IN OMX_U32 nNameLength,
    OMX_IN OMX_U32 nIndex)
{
    OMX_ERRORTYPE ret = OMX_ErrorNone;

    FunctionIn();

    if (nIndex >= SF_OMX_COMPONENT_NUM)
    {
        ret = OMX_ErrorNoMore;
        goto EXIT;
    }
    snprintf(cComponentName, nNameLength, "%s", sf_omx_component_list[nIndex]->componentName);
EXIT:
    FunctionOut();

    return ret;
}

OMX_API OMX_ERRORTYPE OMX_APIENTRY OMX_GetHandle(
    OMX_OUT OMX_HANDLETYPE *pHandle,
    OMX_IN OMX_STRING cComponentName,
    OMX_IN OMX_PTR pAppData,
    OMX_IN OMX_CALLBACKTYPE *pCallBacks)
{
    OMX_ERRORTYPE ret = OMX_ErrorNone;
    unsigned int i = 0;
    SF_OMX_COMPONENT *pSf_omx_component = NULL;

    FunctionIn();
    if (gInitialized != 1)
    {
        ret = OMX_ErrorNotReady;
        goto EXIT;
    }

    if ((pHandle == NULL) || (cComponentName == NULL))
    {
        ret = OMX_ErrorBadParameter;
        goto EXIT;
    }

    for (i = 0; i < SF_OMX_COMPONENT_NUM; i++)
    {
        pSf_omx_component = sf_omx_component_list[i];
        if (pSf_omx_component == NULL)
        {
            ret = OMX_ErrorBadParameter;
            goto EXIT;
        }
        if (strcmp(cComponentName, pSf_omx_component->componentName) == 0)
        {
            ret = pSf_omx_component->SF_OMX_ComponentConstructor(pSf_omx_component);
            if (ret != OMX_ErrorNone)
            {
                goto EXIT;
            }
            *pHandle = pSf_omx_component->pOMXComponent;
            pSf_omx_component->callbacks = pCallBacks;
            pSf_omx_component->pAppData = pAppData;
            break;
        }
    }
EXIT:
    FunctionOut();
    return ret;
}

OMX_API OMX_ERRORTYPE OMX_APIENTRY OMX_FreeHandle(OMX_IN OMX_HANDLETYPE hComponent)
{
    OMX_ERRORTYPE ret = OMX_ErrorNone;
    OMX_COMPONENTTYPE *pOMXComponent = (OMX_COMPONENTTYPE *)hComponent;
    SF_OMX_COMPONENT *pSfOMXComponent = (SF_OMX_COMPONENT *)pOMXComponent->pComponentPrivate;
    FunctionIn();
    ret = pSfOMXComponent->SF_OMX_ComponentClear(pSfOMXComponent);

    FunctionOut();
    return ret;
}

OMX_API OMX_ERRORTYPE OMX_APIENTRY OMX_SetupTunnel(
    OMX_IN OMX_HANDLETYPE hOutput,
    OMX_IN OMX_U32 nPortOutput,
    OMX_IN OMX_HANDLETYPE hInput,
    OMX_IN OMX_U32 nPortInput)
{
    OMX_ERRORTYPE ret = OMX_ErrorNone;
    // OMX_COMPONENTTYPE *pOMXComponent_hOutput = (OMX_COMPONENTTYPE *)hOutput;
    // OMX_COMPONENTTYPE *pOMXComponent_hInput = (OMX_COMPONENTTYPE *)hInput;

    FunctionIn();
    ret = OMX_ErrorNotImplemented;

    FunctionOut();
    return ret;
}

OMX_API OMX_ERRORTYPE OMX_GetContentPipe(
    OMX_OUT OMX_HANDLETYPE *hPipe,
    OMX_IN OMX_STRING szURI)
{
    OMX_ERRORTYPE ret = OMX_ErrorNone;
    FunctionIn();

    FunctionOut();
    return ret;
}

OMX_API OMX_ERRORTYPE OMX_GetComponentsOfRole(
    OMX_IN OMX_STRING role,
    OMX_INOUT OMX_U32 *pNumComps,
    OMX_INOUT OMX_U8 **compNames)
{
    OMX_ERRORTYPE ret = OMX_ErrorNone;
    FunctionIn();

    LOG(SF_LOG_INFO, "Role = %s\r\n", role);
    *pNumComps = 0;
    for (int i = 0; i < SF_OMX_COMPONENT_NUM; i ++)
    {
        if (sf_omx_component_list[i] == NULL)
        {
            break;
        }
        if (strcmp(sf_omx_component_list[i]->componentRule, role) == 0)
        {
            if (compNames != NULL) {
                strcpy((OMX_STRING)compNames[*pNumComps], sf_omx_component_list[i]->componentName);
            }
            *pNumComps = (*pNumComps + 1);
            LOG(SF_LOG_INFO,"Get component %s, Role = %s\r\n", sf_omx_component_list[i]->componentName, sf_omx_component_list[i]->componentRule)
        }
    }
    FunctionOut();
    return ret;
}

OMX_API OMX_ERRORTYPE OMX_GetRolesOfComponent(
    OMX_IN OMX_STRING compName,
    OMX_INOUT OMX_U32 *pNumRoles,
    OMX_OUT OMX_U8 **roles)
{
    OMX_ERRORTYPE ret = OMX_ErrorNone;
    FunctionIn();

    FunctionOut();
    return ret;
}

OMX_ERRORTYPE ClearOMXBuffer(SF_OMX_COMPONENT *pSfOMXComponent, OMX_BUFFERHEADERTYPE *pBuffer)
{
    OMX_U32 i = 0;
    for (i = 0; i < sizeof(pSfOMXComponent->pBufferArray) / sizeof(pSfOMXComponent->pBufferArray[0]); i++)
    {
        if (pSfOMXComponent->pBufferArray[i] == pBuffer)
        {
            pSfOMXComponent->pBufferArray[i] = NULL;
            LOG(SF_LOG_DEBUG, "Clear OMX buffer %p at index %d\r\n", pBuffer, i);
            return OMX_ErrorNone;
        }
    }
    if (i == sizeof(pSfOMXComponent->pBufferArray) / sizeof(pSfOMXComponent->pBufferArray[0]))
    {
        LOG(SF_LOG_ERR, "Could Not found omx buffer!\r\n");
    }
    return OMX_ErrorBadParameter;
}

OMX_U32 GetOMXBufferCount(SF_OMX_COMPONENT *pSfOMXComponent)
{
    OMX_U32 i = 0;
    OMX_U32 count = 0;
    for (i = 0; i < sizeof(pSfOMXComponent->pBufferArray) / sizeof(pSfOMXComponent->pBufferArray[0]); i++)
    {
        if (pSfOMXComponent->pBufferArray[i] != NULL)
        {
            LOG(SF_LOG_DEBUG, "find OMX buffer %p at index %d\r\n", pSfOMXComponent->pBufferArray[i], i);
            count++;
        }
    }

    return count;
}

OMX_ERRORTYPE StoreOMXBuffer(SF_OMX_COMPONENT *pSfOMXComponent, OMX_BUFFERHEADERTYPE *pBuffer)
{
    OMX_U32 i = 0;
    for (i = 0; i < sizeof(pSfOMXComponent->pBufferArray) / sizeof(pSfOMXComponent->pBufferArray[0]); i++)
    {
        if (pSfOMXComponent->pBufferArray[i] == NULL)
        {
            pSfOMXComponent->pBufferArray[i] = pBuffer;
            LOG(SF_LOG_DEBUG, "Store OMX buffer %p at index %d\r\n", pBuffer, i);
            return OMX_ErrorNone;
        }
    }
    if (i == sizeof(pSfOMXComponent->pBufferArray))
    {
        LOG(SF_LOG_ERR, "Buffer arrary full!\r\n");
    }
    return OMX_ErrorInsufficientResources;
}

OMX_BUFFERHEADERTYPE *GetOMXBufferByAddr(SF_OMX_COMPONENT *pSfOMXComponent, OMX_U8 *pAddr)
{
    OMX_U32 i = 0;
    OMX_BUFFERHEADERTYPE *pOMXBuffer;
    FunctionIn();
    LOG(SF_LOG_INFO, "ADDR=%p\r\n", pAddr);
    for (i = 0; i < sizeof(pSfOMXComponent->pBufferArray) / sizeof(pSfOMXComponent->pBufferArray[0]); i++)
    {
        pOMXBuffer = pSfOMXComponent->pBufferArray[i];
        LOG(SF_LOG_DEBUG, "Compare OMX buffer %p at index %d with addr %p\r\n", pOMXBuffer, i, pAddr);
        if (pOMXBuffer == NULL)
        {
            continue;
        }
        if (pOMXBuffer->pBuffer == pAddr)
        {
            return pOMXBuffer;
        }
    }
    LOG(SF_LOG_ERR, "could not find buffer!\r\n");

    FunctionOut();
    return NULL;
}
void SF_LogMsgAppend(int level, const char *format, ...)
{
    va_list ptr;

    if (level > gDebugLevel)
    {
        return;
    }
    
    printf("%66s\t", " ");
    va_start(ptr, format);
    vprintf(format, ptr);
    va_end(ptr);
}
void SF_LogMsg(int level, const char *function, int line, const char *format, ...)
{
    char *prefix = "";
    va_list ptr;
    struct timeval tv;

    if (level > gDebugLevel)
    {
        return;
    }

    switch (level)
    {
    case SF_LOG_ERR:
        prefix = "[ERROR]";
        break;
    case SF_LOG_WARN:
        prefix = "[WARN ]";
        break;
    case SF_LOG_PERF:
        prefix = "[PERF ]";
        break;
    case SF_LOG_INFO:
        prefix = "[INFO ]";
        break;
    case SF_LOG_DEBUG:
        prefix = "[DEBUG]";
        break;
    default:
        break;
    }
    gettimeofday(&tv, NULL);
    printf("[%06ld:%06ld]%10s%32s%10d\t", tv.tv_sec, tv.tv_usec, prefix, function, line);
    va_start(ptr, format);
    vprintf(format, ptr);
    va_end(ptr);
}
