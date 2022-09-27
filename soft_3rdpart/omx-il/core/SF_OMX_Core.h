// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2021 StarFive Technology Co., Ltd.
 */
#ifndef OMX__CORE
#define OMX__CORE

#include "OMX_Types.h"
#include "OMX_Core.h"
#include "OMX_Component.h"
#include "OMX_VideoExt.h"

#include <sys/time.h>
#include <stdarg.h>
#include <dlfcn.h>
#include <string.h>

#define ENABLE_DEBUG
#ifdef ENABLE_DEBUG
enum
{
    SF_LOG_ERR = 0,
    SF_LOG_WARN,
    SF_LOG_PERF,
    SF_LOG_INFO,
    SF_LOG_DEBUG,
    SF_LOG_ALL,
};
void SF_LogMsg(int level, const char *function, int line, const char *format, ...);
void SF_LogMsgAppend(int level, const char *format, ...);
#define LOG(level, ...) SF_LogMsg(level, __FUNCTION__, __LINE__, __VA_ARGS__);
#define LOG_APPEND(level, ...) SF_LogMsgAppend(level, __VA_ARGS__);
#define FunctionIn() SF_LogMsg(SF_LOG_DEBUG, __FUNCTION__, __LINE__, "FUN IN\r\n");
#define FunctionOut() SF_LogMsg(SF_LOG_DEBUG, __FUNCTION__, __LINE__, "FUN OUT\r\n");
#else
#define FunctionIn()
#define FunctionOut()
#endif
#define VPU_OUTPUT_BUF_NUMBER 7
#define VPU_INPUT_BUF_NUMBER 3

typedef enum SF_BUFFER_TYPE
{
    SF_BUFFER_NOMAL = 0,
    SF_BUFFER_DMA,
    SF_BUFFER_DMA_EXTERNAL,
    SF_BUFFER_DMA_FD,
    SF_BUFFER_DMA_FD_EXTERNAL,
    SF_BUFFER_ALL,
}SF_BUFFER_TYPE;

typedef struct _SF_OMX_BUF_INFO
{
    SF_BUFFER_TYPE type;
    OMX_U64 PhysicalAddress;
    OMX_S32 fd;
}SF_OMX_BUF_INFO;

typedef struct _SF_OMX_COMPONENT
{
    OMX_STRING componentName;
    OMX_STRING libName;
    OMX_COMPONENTTYPE *pOMXComponent;
    OMX_ERRORTYPE (*SF_OMX_ComponentConstructor)(struct _SF_OMX_COMPONENT *hComponent);
    OMX_ERRORTYPE (*SF_OMX_ComponentClear)(struct _SF_OMX_COMPONENT *hComponent);
    OMX_PTR soHandle;
    // SF_COMPONENT_FUNCTIONS *functions;
    OMX_PTR componentImpl;

    // OMX_PTR functions;
    // Component *hSFComponentExecoder;
    // Component *hSFComponentFeeder;
    // Component *hSFComponentRender;
    // void *testConfig;
    // CNMComponentConfig *config;
    // void *lsnCtx;
    // Uint16 *pusBitCode;
    // CodStd bitFormat;
    // OMX_VIDEO_PARAM_AVCTYPE AVCComponent[2];
    // OMX_VIDEO_PARAM_HEVCTYPE HEVCComponent[2];

    OMX_CALLBACKTYPE *callbacks;
    OMX_PTR pAppData;
    OMX_PARAM_PORTDEFINITIONTYPE portDefinition[2];
    OMX_BUFFERHEADERTYPE *pBufferArray[64];
    OMX_STRING fwPath;
    OMX_STRING componentRule;
    OMX_STATETYPE nextState;
    OMX_BOOL memory_optimization;
} SF_OMX_COMPONENT;

typedef struct _SF_PORT_PRIVATE
{
    OMX_U32 nPortnumber;
} SF_PORT_PRIVATE;

#define PRINT_STUCT(a, b)                                   \
    do                                                      \
    {                                                       \
        printf("size = %d\r\n", sizeof(b));                 \
        for (int i = 0; i < sizeof(b); i += sizeof(void *)) \
        {                                                   \
            for (int j = 0; j < sizeof(void *); j++)        \
            {                                               \
                printf("%02X ", *((char *)a + i + j));      \
            }                                               \
            for (int j = 0; j < sizeof(void *); j++)        \
            {                                               \
                printf("%c", *((char *)a + i + j));         \
            }                                               \
            printf("\r\n");                                 \
        }                                                   \
    } while (0)

#ifdef __cplusplus
extern "C"
{
#endif

int GetNumberOfComponent();
OMX_BUFFERHEADERTYPE *GetOMXBufferByAddr(SF_OMX_COMPONENT *pSfOMXComponent, OMX_U8 *pAddr);
OMX_ERRORTYPE StoreOMXBuffer(SF_OMX_COMPONENT *pSfOMXComponent, OMX_BUFFERHEADERTYPE *pBuffer);
OMX_ERRORTYPE ClearOMXBuffer(SF_OMX_COMPONENT *pSfOMXComponent, OMX_BUFFERHEADERTYPE *pBuffer);
OMX_U32 GetOMXBufferCount(SF_OMX_COMPONENT *pSfOMXComponent);

#ifdef __cplusplus
}
#endif

#endif
