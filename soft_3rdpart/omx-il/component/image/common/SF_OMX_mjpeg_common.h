// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2021 StarFive Technology Co., Ltd.
 */
#ifndef SF_OMX_MJPEG_COMMON
#define SF_OMX_MJPEG_COMMON

#include "OMX_Component.h"
#include "OMX_Image.h"
#include "OMX_Index.h"
#include "OMX_IndexExt.h"
#include "SF_OMX_Core.h"
#include "sf_queue.h"
#include "sf_thread.h"
#include "sf_semaphore.h"
#include "codaj12/jpuapi/jpuapi.h"
#include "codaj12/jpuapi/jpuapifunc.h"
#include "codaj12/sample/main_helper.h"
#include <sys/queue.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <pthread.h>
#include <errno.h>

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

#define DEFAULT_FRAME_WIDTH 1920
#define DEFAULT_FRAME_HEIGHT 1080
#define DEFAULT_MJPEG_INPUT_BUFFER_SIZE (DEFAULT_FRAME_WIDTH * DEFAULT_FRAME_HEIGHT * 3)
#define DEFAULT_MJPEG_OUTPUT_BUFFER_SIZE (DEFAULT_FRAME_WIDTH * DEFAULT_FRAME_HEIGHT * 3)
#define NUM_FRAME_BUF MAX_FRAME
#define DEFAULT_FRAME_FORMAT FORMAT_420

#define CODAJ12_OUTPUT_BUF_NUMBER 4
#define CODAJ12_INPUT_BUF_NUMBER 1

#define INPUT_PORT_SUPPORTFORMAT_NUM_MAX    1

#define MCA_MAX_INDEX 1

typedef struct _SF_CODAJ12_FUNCTIONS
{
    JpgRet (*JPU_Init)(void);
    JpgRet (*JPU_DecOpen)(JpgDecHandle *pHandle, JpgDecOpenParam *pop);
    JpgRet (*JPU_DecGetInitialInfo)(JpgDecHandle handle, JpgDecInitialInfo *info);
    JpgRet (*JPU_DecRegisterFrameBuffer2)(JpgDecHandle handle, FrameBuffer *bufArray, int stride);
    JpgRet (*JPU_DecRegisterFrameBuffer)(JpgDecHandle handle, FrameBuffer * bufArray, int num, int stride);
    JpgRet (*JPU_DecGiveCommand)(JpgDecHandle handle, JpgCommand cmd, void *param);
    JpgRet (*JPU_DecStartOneFrameBySerialNum)(JpgDecHandle handle, JpgDecParam *param,int bufferIndex);
    JpgRet (*JPU_DecStartOneFrame)(JpgDecHandle handle, JpgDecParam *param);
    JpgRet (*JPU_DecGetOutputInfo)(JpgDecHandle handle, JpgDecOutputInfo *info);
    JpgRet (*JPU_SWReset)(JpgHandle handle);
    JpgRet (*JPU_DecSetRdPtrEx)(JpgDecHandle handle, PhysicalAddress addr, BOOL updateWrPtr);
    JpgRet (*JPU_DecClose)(JpgDecHandle handle);
    Int32 (*JPU_WaitInterrupt)(JpgHandle handle, int timeout);
    FrameBuffer *(*JPU_GetFrameBufPool)(JpgDecHandle handle);
    void (*JPU_ClrStatus)(JpgHandle handle, Uint32 val);
    void (*JPU_DeInit)(void);

    void (*BSFeederBuffer_SetData)(void *feeder, char *address, Uint32 size);
    Uint32 (*BitstreamFeeder_SetData)(BSFeeder feeder, void *data, Uint32 size);
    BSFeeder (*BitstreamFeeder_Create)(const char *path, FeedingMethod method, EndianMode endian);
    Uint32 (*BitstreamFeeder_Act)(BSFeeder feeder, JpgDecHandle handle, jpu_buffer_t *bsBuffer);
    BOOL(*BitstreamFeeder_Destroy)
    (BSFeeder feeder);

    int (*jdi_allocate_dma_memory)(jpu_buffer_t *vb);
    void (*jdi_free_dma_memory)(jpu_buffer_t *vb);

    void *(*AllocateOneFrameBuffer)
    // (Uint32 instIdx, Uint32 size, Uint32 *bufferIndex);
    (Uint32 instIdx, FrameFormat subsample, CbCrInterLeave cbcrIntlv,
     PackedFormat packed, Uint32 rotation, BOOL scalerOn, Uint32 width, Uint32 height, Uint32 bitDepth, Uint32 *bufferIndex);
    int (*SaveYuvImageHelper)(Uint8* pYuv, FrameBuffer* fb, CbCrInterLeave interLeave, PackedFormat packed,
                        Uint32 picWidth, Uint32 picHeight, Uint32 bitDepth);
    int (*SaveYuvImageHelperDma)(Uint8* pYuv, FrameBuffer* fb, CbCrInterLeave interLeave, PackedFormat packed,
                        Uint32 picWidth, Uint32 picHeight, Uint32 bitDepth);
    int (*FreeOneFrameBuffer)(Uint32 instIdx, void* virt_addr);
    void (*FreeFrameBuffer)(int instIdx);
    FRAME_BUF *(*GetFrameBuffer)(int instIdx, int idx);
    Uint32 (*GetFrameBufferCount)(int instIdx);
    BOOL (*AttachOneFrameBuffer)(Uint32 instIdx, FrameFormat subsample, CbCrInterLeave cbcrIntlv, PackedFormat packed,
                         Uint32 rotation, BOOL scalerOn, Uint32 width, Uint32 height, Uint32 bitDepth,
                         void *virtAddress, Uint32 size, Uint32 *bufferIndex);
    BOOL (*AllocateFrameBuffer)(Uint32 instIdx, FrameFormat subsample, CbCrInterLeave cbcrIntlv, PackedFormat packed,
                         Uint32 rotation, BOOL scalerOn, Uint32 width, Uint32 height, Uint32 bitDepth, Uint32 num);
    BOOL (*UpdateFrameBuffers)(Uint32 instIdx, Uint32 num, FRAME_BUF *frameBuf);

    // JPU Log
    void (*SetMaxLogLevel)(int level);
} SF_CODAJ12_FUNCTIONS;

typedef struct Message
{
    long msg_type;
    OMX_U32 msg_flag;
    OMX_BUFFERHEADERTYPE *pBuffer;
} Message;

typedef struct _SF_CODAJ12_IMPLEMEMT
{
    DecConfigParam *config;
    SF_CODAJ12_FUNCTIONS *functions;
    BSFeeder feeder;
    jpu_buffer_t vbStream;
    JpgDecOpenParam decOP;
    JpgDecHandle handle;
    JpgDecInitialInfo initialInfo;
    Int32 instIdx;
    FrameBuffer frameBuf[NUM_FRAME_BUF];
    FRAME_BUF frame[NUM_FRAME_BUF];
    OMX_S32 sInputMessageQueue;
    OMX_S32 sOutputMessageQueue;
    OMX_S32 sBufferDoneQueue;
    THREAD_HANDLE_TYPE *pProcessThread;
    OMX_BOOL bThreadRunning;
    OMX_STATETYPE currentState;
    FrameFormat frameFormat;
    OMX_BOOL allocBufFlag;
    OMX_BOOL gotEos;
    SF_Queue *CmdQueue;
    SF_Queue *inPauseQ;
    SF_Queue *outPauseQ;

    OMX_HANDLETYPE pauseOutSemaphore;
    THREAD_HANDLE_TYPE *pCmdThread;
    OMX_BOOL bCmdRunning;
    OMX_BOOL bPause;
} SF_CODAJ12_IMPLEMEMT;

enum port_index
{
    OMX_INPUT_PORT_INDEX = 0,
    OMX_OUTPUT_PORT_INDEX = 1,
    OMX_PORT_MAX = 2,
};

#ifdef __cplusplus
extern "C"
{
#endif

OMX_ERRORTYPE GetStateMjpegCommon(OMX_IN OMX_HANDLETYPE hComponent, OMX_OUT OMX_STATETYPE *pState);
OMX_ERRORTYPE InitMjpegStructorCommon(SF_OMX_COMPONENT *hComponent);
OMX_BOOL AttachOutputBuffer(SF_OMX_COMPONENT *pSfOMXComponent, OMX_U8* pBuffer, OMX_U32 nSizeBytes);
OMX_U8* AllocateOutputBuffer(SF_OMX_COMPONENT *pSfOMXComponent, OMX_U32 nSizeBytes);
void ThreadExit(void *value_ptr);
void CodaJ12FlushBuffer(SF_OMX_COMPONENT *pSfOMXComponent, OMX_U32 nPortNumber);

#ifdef __cplusplus
}
#endif

#endif
