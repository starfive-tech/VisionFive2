// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2021 StarFive Technology Co., Ltd.
 */
#ifndef SF_OMX_VIDEO_COMMON
#define SF_OMX_VIDEO_COMMON

#include "OMX_Component.h"
#include "OMX_Video.h"
#include "OMX_VideoExt.h"
#include "OMX_Index.h"
#include "OMX_IndexExt.h"
#include "SF_OMX_Core.h"
#include "wave511/sample_v2/component/component.h"
#include "wave511/sample_v2/component_encoder/encoder_listener.h"
#include "wave511/sample_v2/component_decoder/decoder_listener.h"

#define INIT_SET_SIZE_VERSION(_struct_, _structType_) \
    do                                                \
    {                                                 \
        memset((_struct_), 0, sizeof(_structType_));  \
        (_struct_)->nSize = sizeof(_structType_);     \
        (_struct_)->nVersion.s.nVersionMajor = 1;     \
        (_struct_)->nVersion.s.nVersionMinor = 18;    \
        (_struct_)->nVersion.s.nRevision = 1;         \
        (_struct_)->nVersion.s.nStep = 0;             \
    } while (0)

#define DEFAULT_FRAME_WIDTH 3840
#define DEFAULT_FRAME_HEIGHT 2160
#define DEFAULT_VIDEO_INPUT_BUFFER_SIZE (DEFAULT_FRAME_WIDTH * DEFAULT_FRAME_HEIGHT) / 2
#define DEFAULT_VIDEO_OUTPUT_BUFFER_SIZE (DEFAULT_FRAME_WIDTH * DEFAULT_FRAME_HEIGHT * 3) / 2

typedef struct _SF_COMPONENT_FUNCTIONS
{
    Component (*ComponentCreate)(const char *componentName, CNMComponentConfig *componentParam);
    BOOL(*ComponentSetupTunnel)(Component fromComponent, Component toComponent);
    ComponentState (*ComponentExecute)(Component component);
    Int32 (*ComponentWait)(Component component);
    void (*ComponentStop)(Component component);
    void (*ComponentRelease)(Component component);
    BOOL(*ComponentChangeState)(Component component, Uint32 state);
    BOOL(*ComponentDestroy)(Component component, BOOL *ret);
    CNMComponentParamRet (*ComponentGetParameter)(Component from, Component to, GetParameterCMD commandType, void *data);
    CNMComponentParamRet (*ComponentSetParameter)(Component from, Component to, SetParameterCMD commandType, void *data);
    void (*ComponentNotifyListeners)(Component component, Uint64 event, void *data);
    BOOL(*ComponentRegisterListener)(Component component, Uint64 events, ComponentListenerFunc func, void *context);
    void (*ComponentPortCreate)(Port *port, Component owner, Uint32 depth, Uint32 size);
    void (*ComponentPortSetData)(Port *port, PortContainer *portData);
    PortContainer *(*ComponentPortPeekData)(Port *port);
    PortContainer *(*ComponentPortGetData)(Port *port);
    void *(*WaitBeforeComponentPortGetData)(Port *port);
    void (*ComponentPortWaitReadyStatus)(Port *port);
    void (*ComponentPortDestroy)(Port *port);
    BOOL(*ComponentPortHasInputData)(Port *port);
    Uint32 (*ComponentPortGetSize)(Port *port);
    void (*ComponentPortFlush)(Component component);
    ComponentState (*ComponentGetState)(Component component);
    BOOL(*ComponentParamReturnTest)(CNMComponentParamRet ret, BOOL *retry);
    // Listener
    BOOL(*SetupDecListenerContext)(DecListenerContext *ctx, CNMComponentConfig *config, Component renderer);
    BOOL(*SetupEncListenerContext)(EncListenerContext *ctx, CNMComponentConfig *config);
    void (*ClearDecListenerContext)(DecListenerContext *ctx);
    void (*HandleDecCompleteSeqEvent)(Component com, CNMComListenerDecCompleteSeq *param, DecListenerContext *ctx);
    void (*HandleDecRegisterFbEvent)(Component com, CNMComListenerDecRegisterFb *param, DecListenerContext *ctx);
    void (*HandleDecGetOutputEvent)(Component com, CNMComListenerDecDone *param, DecListenerContext *ctx);
    void (*HandleDecCloseEvent)(Component com, CNMComListenerDecClose *param, DecListenerContext *ctx);
    void (*ClearEncListenerContext)(EncListenerContext *ctx);
    void (*HandleEncFullEvent)(Component com, CNMComListenerEncFull *param, EncListenerContext *ctx);
    void (*HandleEncGetOutputEvent)(Component com, CNMComListenerEncDone *param, EncListenerContext *ctx);
    void (*HandleEncCompleteSeqEvent)(Component com, CNMComListenerEncCompleteSeq *param, EncListenerContext *ctx);
    void (*HandleEncGetEncCloseEvent)(Component com, CNMComListenerEncClose *param, EncListenerContext *ctx);
    void (*EncoderListener)(Component com, Uint64 event, void *data, void *context);
    void (*DecoderListener)(Component com, Uint64 event, void *data, void *context);
    // Helper
    void (*SetDefaultEncTestConfig)(TestEncConfig *testConfig);
    void (*SetDefaultDecTestConfig)(TestDecConfig *testConfig);
    Int32 (*LoadFirmware)(Int32 productId, Uint8 **retFirmware, Uint32 *retSizeInWord, const char *path);
    BOOL(*SetupEncoderOpenParam)(EncOpenParam *param, TestEncConfig *config, ENC_CFG *encCfg);
    RetCode (*SetUpDecoderOpenParam)(DecOpenParam *param, TestDecConfig *config);
    // VPU
    int (*VPU_GetProductId)(int coreIdx);
    BOOL(*Queue_Enqueue)(Queue *queue, void *data);
    Uint32 (*Queue_Get_Cnt)(Queue *queue);
    RetCode (*VPU_DecClrDispFlag)(DecHandle handle, int index);
    RetCode (*VPU_DecGetFrameBuffer)(DecHandle handle, int frameIdx, FrameBuffer* frameBuf);
    void (*Render_DecClrDispFlag)(void *context, int index);
    // VPU Log
    int (*InitLog)(void);
    void (*DeInitLog)(void);
    void (*SetMaxLogLevel)(int level);
    int (*GetMaxLogLevel)(void);
    // FrameBuffer
    void* (*AllocateFrameBuffer2)(ComponentImpl* com, Uint32 size);
    BOOL (*AttachDMABuffer)(ComponentImpl* com, Uint64 virtAddress, Uint32 size);
    void (*SetRenderTotalBufferNumber)(ComponentImpl* com, Uint32 number);
    void (*SetFeederTotalBufferNumber)(ComponentImpl* com, Uint32 number);
    void (*WaitForExecoderReady)(ComponentImpl *com);
} SF_COMPONENT_FUNCTIONS;

typedef struct _SF_WAVE5_IMPLEMEMT
{
    SF_COMPONENT_FUNCTIONS *functions;
    Component *hSFComponentExecoder;
    Component *hSFComponentFeeder;
    Component *hSFComponentRender;
    void *testConfig;
    CNMComponentConfig *config;
    void *lsnCtx;
    Uint16 *pusBitCode;
    CodStd bitFormat;
    OMX_VIDEO_PARAM_AVCTYPE AVCComponent[2];
    OMX_VIDEO_PARAM_HEVCTYPE HEVCComponent[2];
}SF_WAVE5_IMPLEMEMT;

#ifdef __cplusplus
extern "C"
{
#endif

    BOOL CheckEncTestConfig(TestEncConfig *testConfig);
    BOOL CheckDecTestConfig(TestDecConfig *testConfig);
    OMX_ERRORTYPE GetStateCommon(OMX_IN OMX_HANDLETYPE hComponent, OMX_OUT OMX_STATETYPE *pState);
    OMX_ERRORTYPE InitComponentStructorCommon(SF_OMX_COMPONENT *hComponent);
    OMX_ERRORTYPE ComponentClearCommon(SF_OMX_COMPONENT *hComponent);
    OMX_ERRORTYPE FlushBuffer(SF_OMX_COMPONENT *pSfOMXComponent, OMX_U32 nPort);
    SF_OMX_COMPONENT *GetSFOMXComponrntByComponent(Component *pComponent);
#ifdef __cplusplus
}
#endif

#endif
