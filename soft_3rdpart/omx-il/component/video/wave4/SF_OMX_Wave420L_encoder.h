// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2022 StarFive Technology Co., Ltd.
 */
#ifndef SF_OMX_WAVE4_ENC
#define SF_OMX_WAVE4_ENC


#include "OMX_Component.h"
#include "OMX_Video.h"
#include "OMX_Index.h"
#include "OMX_IndexExt.h"
#include "SF_OMX_Core.h"
#include <sys/queue.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <pthread.h>
#include <errno.h>

#include "sf_queue.h"
#include "sf_thread.h"
#include "sf_semaphore.h"
#include "wave420l/sample/helper/main_helper.h"
#include "wave420l/vpuapi/vpuapi.h"


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

#define NUM_OF_PORTS 2

#define DEFAULT_FRAME_WIDTH 1920
#define DEFAULT_FRAME_HEIGHT 1080
#define DEFAULT_MJPEG_INPUT_BUFFER_SIZE (DEFAULT_FRAME_WIDTH * DEFAULT_FRAME_HEIGHT * 3)
#define DEFAULT_MJPEG_OUTPUT_BUFFER_SIZE (DEFAULT_FRAME_WIDTH * DEFAULT_FRAME_HEIGHT * 3)
#define DEFAULT_FRAMERATE 30
#define DEFAULT_GOP 30
#define DEFAULT_FRAME_FORMAT FORMAT_420

#define WAVE_DEFAULT_INPUT_BUF_NUMBER 8
#define WAVE_DEFAULT_OUTPUT_BUF_NUMBER 8
#define DEFAULT_BS_BUFF 1

typedef struct _SF_W420L_FUNCTIONS
{
    Int32 (*VPU_GetProductId)(Int32 coreIdx);
    RetCode (*VPU_GetVersionInfo)(Uint32 coreIdx, Uint32 *versionInfo, Uint32 *revision, Uint32 *productId);
    RetCode (*VPU_InitWithBitcode)(Uint32 coreIdx, const Uint16* code, Uint32 size);
    RetCode (*VPU_DeInit)(Uint32 coreIdx);
    Int32 (*GetEncOpenParam)(EncOpenParam *pEncOP, TestEncConfig *pEncConfig, ENC_CFG *pEncCfg);
    Int32 (*GetEncOpenParamDefault)(EncOpenParam *pEncOP, TestEncConfig *pEncConfig);
    RetCode (*VPU_EncOpen)(EncHandle* pHandle, EncOpenParam * pop);
    RetCode (*VPU_EncClose)(EncHandle handle);
    Int32 (*VPU_GetFrameBufSize)(Int32 coreIdx, Int32 stride, Int32 height, Int32 mapType, Int32 format, Int32 interleave, DRAMConfig *pDramCfg);
    RetCode (*VPU_EncRegisterFrameBuffer)(EncHandle handle, FrameBuffer* bufArray, Int32 num, Int32 stride, Int32 height, Int32 mapType);
    RetCode (*VPU_EncAllocateFrameBuffer)(EncHandle handle, FrameBufferAllocInfo info, FrameBuffer *frameBuffer);
    RetCode (*VPU_EncGiveCommand)(EncHandle handle, CodecCommand cmd, void* param);
    RetCode (*VPU_EncGetInitialInfo)(EncHandle handle, EncInitialInfo * info);
    RetCode (*VPU_SWReset)(Uint32 coreIdx, SWResetMode resetMode, void *pendingInst);
    RetCode (*VPU_EncStartOneFrame)(EncHandle handle, EncParam * param);
    Int32 (*VPU_WaitInterrupt)(Uint32 coreIdx, Int32 timeout);
    void (*VPU_ClearInterrupt)(Uint32 coreIdx);
    RetCode (*VPU_EncGetOutputInfo)(EncHandle handle, EncOutputInfo* info);

    int (*vdi_allocate_dma_memory)(unsigned long core_idx, vpu_buffer_t *vb);
    void (*vdi_free_dma_memory)(unsigned long core_idx, vpu_buffer_t *vb);

    YuvFeeder (*YuvFeeder_Create)(Uint32 type, const char* srcFilePath, YuvInfo yuvInfo);
    BOOL (*YuvFeeder_Feed)(YuvFeeder feeder, Uint32 coreIdx, FrameBuffer* fb, size_t picWidth, size_t picHeight, void* arg);
    BOOL (*YuvFeeder_Destroy)(YuvFeeder feeder);
    BitstreamReader (*BufferStreamReader_Create)(Uint32 type, EndianMode endian, EncHandle* handle);
    BOOL (*BufferStreamReader_Act)(BitstreamReader reader, PhysicalAddress bitstreamBuffer, Uint32 bitstreamBufferSize, Uint32 streamReadSize, Uint8* pBuffer,Comparator comparator);
    BOOL (*BufferStreamReader_Destroy)(BitstreamReader reader);
    Comparator (*Comparator_Create)(Uint32 type, char* goldenPath, ...);
    BOOL (*Comparator_Destroy)(Comparator  comp);
    BOOL (*Comparator_CheckEOF)(Comparator  comp);

    void (*DisplayEncodedInformation)(EncHandle handle, CodStd codec, Uint32 frameNo, EncOutputInfo* encodedInfo, ...);
    void (*PrintVpuVersionInfo)(Uint32 core_idx);
    Int32 (*CalcStride)(Uint32 width, Uint32 height, FrameBufferFormat format, BOOL cbcrInterleave, TiledMapType mapType, BOOL isVP9);
    Int32 (*GetPackedFormat) (Int32 srcBitDepth, Int32 packedType, Int32 p10bits,Int32 msb);
    Int32 (*LoadFirmware)(Int32 productId, Uint8** retFirmware, Uint32* retSizeInWord, const char* path);

    void (*SetMaxLogLevel)(int level);
    RetCode (*EnterLock)(Uint32 coreIdx);
    RetCode (*LeaveLock)(Uint32 coreIdx);
    void (*PrintVpuStatus)(Uint32 coreIdx, Uint32 productId);
}SF_W420L_FUNCTIONS;


typedef struct Message
{
    long msg_type;
    OMX_U32 msg_flag;
    OMX_BUFFERHEADERTYPE *pBuffer;
} Message;

typedef struct SF_OMX_CMD
{
    OMX_COMMANDTYPE Cmd;
    OMX_U32 nParam;
    OMX_PTR pCmdData;
}SF_OMX_CMD;

enum port_index
{
    OMX_INPUT_PORT_INDEX = 0,
    OMX_OUTPUT_PORT_INDEX = 1,
    OMX_PORT_MAX = 2,
};

typedef struct _SF_WAVE420L_IMPLEMEMT
{
    EncHandle           handle;
    SF_W420L_FUNCTIONS *functions;
    BSFeeder            feeder;
    vpu_buffer_t        vbSourceFrameBuf[ENC_SRC_BUF_NUM];
    vpu_buffer_t        vbReconFrameBuf[MAX_REG_FRAME];
    vpu_buffer_t        vbStream[MAX_REG_FRAME];
    FrameBuffer         fbSrc[ENC_SRC_BUF_NUM];
    FrameBuffer         fbRecon[MAX_REG_FRAME];
    Int32               regFrameBufCount;
    Int32               bsBufferCount;
    EncOpenParam        encOP;
    TestEncConfig       encConfig;
    EncInitialInfo      initialInfo;
    EncOutputInfo       outputInfo;
    Int32               instIdx;
    Int32               coreIdx;
    Uint32              tmpFramerate;
    Uint64              tmpCounter;
    SF_Queue            *EmptyQueue;
    SF_Queue            *FillQueue;
    SF_Queue            *CmdQueue;
    SF_Queue            *pauseQ;
    OMX_HANDLETYPE      pauseSemaphore;
    OMX_VIDEO_PARAM_HEVCTYPE HEVCComponent[2];

    THREAD_HANDLE_TYPE *pProcessThread;
    THREAD_HANDLE_TYPE *pCmdThread;
    OMX_BOOL bThreadRunning;
    OMX_BOOL bCmdRunning;
    OMX_BOOL bPause;
    OMX_STATETYPE currentState;
} SF_WAVE420L_IMPLEMEMT;


/* Warper functions marco */
#define Warp_VPU_GetProductId(Implement, coreIdx) \
    Implement->functions->VPU_GetProductId(coreIdx)

#define Warp_VPU_GetVersionInfo(Implement, coreIdx, pVersionInfo, pRevision, pProductId) \
    Implement->functions->VPU_GetVersionInfo(coreIdx, pVersionInfo, pRevision, pProductId)

#define Warp_VPU_InitWithBitcode(Implement, coreIdx, pCode, size) \
    Implement->functions->VPU_InitWithBitcode(coreIdx, pCode, size)

#define Warp_VPU_DeInit(Implement, coreIdx) \
    Implement->functions->VPU_DeInit(coreIdx)

#define Warp_GetEncOpenParam(Implement, pEncOP, pEncConfig, pEncCfg) \
    Implement->functions->GetEncOpenParam(pEncOP, pEncConfig, pEncCfg)

#define Warp_GetEncOpenParamDefault(Implement, pEncOP, pEncConfig) \
    Implement->functions->GetEncOpenParamDefault(pEncOP, pEncConfig)

#define Warp_VPU_EncOpen(Implement, pHandle, pop) \
    Implement->functions->VPU_EncOpen(pHandle, pop)

#define Warp_VPU_EncClose(Implement, handle) \
    Implement->functions->VPU_EncClose(handle)

#define Warp_VPU_GetFrameBufSize(Implement, coreIdx, stride, height, mapType, format, interleave, pDramCfg) \
    Implement->functions->VPU_GetFrameBufSize(coreIdx, stride, height, mapType, format, interleave, pDramCfg)

#define Warp_VPU_EncRegisterFrameBuffer(Implement, handle, bufArray, num, stride, height, mapType)\
    Implement->functions->VPU_EncRegisterFrameBuffer(handle, bufArray, num, stride, height, mapType)

#define Warp_VPU_EncAllocateFrameBuffer(Implement, handle, info, pframeBuffer) \
    Implement->functions->VPU_EncAllocateFrameBuffer(handle, info, pframeBuffer)

#define Warp_VPU_EncGiveCommand(Implement, handle, cmd, param) \
    Implement->functions->VPU_EncGiveCommand(handle, cmd, param)

#define Warp_VPU_EncGetInitialInfo(Implement, handle, pInfo) \
    Implement->functions->VPU_EncGetInitialInfo(handle, pInfo)

#define Warp_VPU_SWReset(Implement, coreIdx, resetMode, pendingInst) \
    Implement->functions->VPU_SWReset(coreIdx, resetMode, pendingInst)

#define Warp_VPU_EncStartOneFrame(Implement, handle, param) \
    Implement->functions->VPU_EncStartOneFrame(handle, param)

#define Warp_VPU_WaitInterrupt(Implement, coreIdx, timeout) \
    Implement->functions->VPU_WaitInterrupt(coreIdx, timeout)

#define Warp_VPU_ClearInterrupt(Implement, coreIdx) \
    Implement->functions->VPU_ClearInterrupt(coreIdx)

#define Warp_VPU_EncGetOutputInfo(Implement, handle, info) \
    Implement->functions->VPU_EncGetOutputInfo(handle, info)

#define Warp_vdi_allocate_dma_memory(Implement, core_idx, vb) \
    Implement->functions->vdi_allocate_dma_memory(core_idx, vb)

#define Warp_vdi_free_dma_memory(Implement, core_idx, vb) \
    Implement->functions->vdi_free_dma_memory(core_idx, vb)

#define Warp_YuvFeeder_Create(Implement, type, srcFilePath, yuvInfo) \
    Implement->functions->YuvFeeder_Create(type, srcFilePath, yuvInfo)

#define Warp_YuvFeeder_Feed(Implement, feeder, coreIdx, fb, picWidth, picHeight, arg) \
    Implement->functions->YuvFeeder_Feed(feeder, coreIdx, fb, picWidth, picHeight, arg)

#define Warp_YuvFeeder_Destroy(Implement, feeder) \
    Implement->functions->YuvFeeder_Destroy(feeder)

#define Warp_BufferStreamReader_Create(Implement, type, endian, handle) \
    Implement->functions->BufferStreamReader_Create(type, endian, handle)

#define Warp_BufferStreamReader_Act(Implement, reader, bitstreamBuffer, bitstreamBufferSize, streamReadSize, pBuffer, comparator) \
    Implement->functions->BufferStreamReader_Act(reader, bitstreamBuffer, bitstreamBufferSize, streamReadSize, pBuffer, comparator)

#define Warp_BufferStreamReader_Destroy(Implement, reader) \
    Implement->functions->BufferStreamReader_Destroy(reader)

#define Warp_Comparator_Create(Implement, type, goldenPath, ...) \
    Implement->functions->Comparator_Create(type, goldenPath, __VA_ARGS__)

#define Warp_Comparator_Destroy(Implement, comp) \
    Implement->functions->Comparator_Destroy(comp)

#define Warp_Comparator_CheckEOF(Implement, comp) \
    Implement->functions->Comparator_CheckEOF(comp)

#define Warp_DisplayEncodedInformation(Implement, handle, codec, frameNo, encodedInfo, ...) \
    Implement->functions->DisplayEncodedInformation(handle, codec, frameNo, encodedInfo, __VA_ARGS__)

#define Warp_PrintVpuVersionInfo(Implement, core_idx) \
    Implement->functions->PrintVpuVersionInfo(core_idx)

#define Warp_CalcStride(Implement, width, height, format, cbcrInterleave, mapType, isVP9) \
    Implement->functions->CalcStride(width, height, format, cbcrInterleave, mapType, isVP9)

#define Warp_LoadFirmware(Implement, productId, retFirmware, retSizeInWord, path) \
    Implement->functions->LoadFirmware(productId, retFirmware, retSizeInWord, path)

#define Warp_SetMaxLogLevel(Implement, level) \
    Implement->functions->SetMaxLogLevel(level)

#define Warp_EnterLock(Implement, coreIdx) \
    Implement->functions->EnterLock(coreIdx)

#define Warp_LeaveLock(Implement, coreIdx) \
    Implement->functions->LeaveLock(coreIdx)

#define Warp_PrintVpuStatus(Implement, coreIdx, productId) \
    Implement->functions->PrintVpuStatus(coreIdx, productId)

#endif //SF_OMX_WAVE4_ENC
