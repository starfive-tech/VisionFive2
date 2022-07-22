//--=========================================================================--
//  This file is a part of VPU Reference API project
//-----------------------------------------------------------------------------
//
//       This confidential and proprietary software may be used only
//     as authorized by a licensing agreement from Chips&Media Inc.
//     In the event of publication, the following notice is applicable:
//
//            (C) COPYRIGHT CHIPS&MEDIA INC.
//                      ALL RIGHTS RESERVED
//
//       The entire notice above must be reproduced on all authorized
//       copies.
//
//--=========================================================================--
#include <stdio.h>
#include <stdarg.h>
#include <time.h>

#include "vpuapifunc.h"
#include "main_helper.h"

typedef struct {
    FeedingMethod   method;
    Uint8*          remainData;
    Uint32          remainDataSize;
    void*           actualFeeder;
    PhysicalAddress base;
    Uint32          size;
    Uint32          fillingMode;
    BOOL            eos;
    VpuThread       threadHandle;
    DecHandle       decHandle;
    EndianMode      endian;
    BSFeederHook    observer;
    void*           observerArg;
    BOOL            autoUpdate;     /* TRUE - Call VPU_DecUpdateBitstreamBuffer() in the bitstream feeder. default */
} BitstreamFeeder;

static void BitstreamFeeder_DummyObserver(
    void*   handle,
    void*   es,
    Uint32  size,
    void*   arg
    )
{
    UNREFERENCED_PARAMETER(handle);
    UNREFERENCED_PARAMETER(es);
    UNREFERENCED_PARAMETER(size);
    UNREFERENCED_PARAMETER(arg);
}

extern void* BSFeederFixedSize_Create(
    const char* path,
    Uint32    chunkSize
    );

extern BOOL BSFeederFixedSize_Destroy(
    BSFeeder feeder
    );

extern Int32 BSFeederFixedSize_Act(
    void*       feeder,
    BSChunk*    chunk
    );

extern Int32 BSFeederFixedSize_Rewind(
    void*       feeder
    );

extern void* BSFeederFrameSize_Create(
    const char* path,
    CodStd    codec,
    CodStd*   retCodecId,
    Uint32*   retMp4ClassId,
    Uint32*   retSeqWidth,
    Uint32*   retSeqHeight
    );

extern BOOL BSFeederFrameSize_Destroy(
    void*   feeder
    );

extern Int32 BSFeederFrameSize_Act(
    void*       feeder,
    BSChunk*    packet
    );

#ifdef SUPPORT_FFMPEG_DEMUX
extern BOOL BSFeederFrameSize_Rewind(
    void*       feeder
    );
#endif

extern void* BSFeederSizePlusEs_Create(
    const char* path
    );

extern BOOL BSFeederSizePlusEs_Destroy(
    void* feeder
    );

extern Int32 BSFeederSizePlusEs_Act(
    void*       feeder,
    BSChunk*    chunk
    );

extern Int32 BSFeederSizePlusEs_Rewind(
    void*       feeder
    );

/**
* Abstract Bitstream Feeader Functions 
*/
void* BitstreamFeeder_Create(
    const char*     path,
    FeedingMethod   method,
    PhysicalAddress base,
    Uint32          size,
    ...
    )
{
    /*lint -esym(438, ap) */
    va_list             ap;
    Uint32              chunkSize;
    BitstreamFeeder*    handle = NULL;
    void*               feeder = NULL;
    CodStd              codecId;
    CodStd*             retCodecId;
    Uint32*             retMp4ClassId;
    Uint32*             retSeqWidth;
    Uint32*             retSeqHeight;

    switch (method) {
    case FEEDING_METHOD_FIXED_SIZE:
        va_start(ap, size);
        chunkSize = va_arg(ap, int);
        va_end(ap);
        feeder = BSFeederFixedSize_Create(path, chunkSize);
        break;
    case FEEDING_METHOD_FRAME_SIZE:
        va_start(ap, size);
        codecId       = (CodStd)va_arg(ap, Int32);
        retCodecId    = (CodStd*)va_arg(ap, CodStd*);
        retMp4ClassId = (Uint32*)va_arg(ap, Uint32*);
        retSeqWidth   = (Uint32*)va_arg(ap, Uint32*);
        retSeqHeight  = (Uint32*)va_arg(ap, Uint32*);
        va_end(ap);
        feeder = BSFeederFrameSize_Create(path, codecId, retCodecId, retMp4ClassId, retSeqWidth, retSeqHeight);
        break;
    case FEEDING_METHOD_SIZE_PLUS_ES:
        feeder = BSFeederSizePlusEs_Create(path);
        break;
    default:
        feeder = NULL;
        break;
    }

    if (feeder != NULL) {
        if ((handle=(BitstreamFeeder*)osal_malloc(sizeof(BitstreamFeeder))) == NULL) {
            VLOG(ERR, "%s:%d Failed to allocate memory\n", __FUNCTION__, __LINE__);
            return NULL;
        }
        handle->actualFeeder = feeder;
        handle->method       = method;
        handle->remainData   = NULL;
        handle->remainDataSize = 0;
        handle->base         = base;
        handle->size         = size;
        handle->fillingMode  = (method == FEEDING_METHOD_FIXED_SIZE) ? BSF_FILLING_RINGBUFFER : BSF_FILLING_LINEBUFFER;
        handle->threadHandle = NULL;
        handle->eos          = FALSE;
        handle->observer     = (BSFeederHook)BitstreamFeeder_DummyObserver;
        handle->observerArg  = NULL;
        handle->autoUpdate   = TRUE;
    }

    return handle;
    /*lint +esym(438, ap) */
}

void BitstreamFeeder_SetFillMode(
    BSFeeder    feeder,
    Uint32      mode
    )
{
    BitstreamFeeder* bsf = (BitstreamFeeder*)feeder;

    switch (mode) {
    case BSF_FILLING_AUTO:
        bsf->fillingMode  = (bsf->method == FEEDING_METHOD_FIXED_SIZE) ? BSF_FILLING_RINGBUFFER : BSF_FILLING_LINEBUFFER;
        break;
    case BSF_FILLING_RINGBUFFER:
    case BSF_FILLING_LINEBUFFER:
    case BSF_FILLING_RINGBUFFER_WITH_ENDFLAG:
        bsf->fillingMode = mode;
        break;
    default:
        VLOG(INFO, "%s Not supported mode %d\n", __FUNCTION__, mode);
        break;
    }
}

static Uint32 FeedBitstream(
    BSFeeder        feeder,
    vpu_buffer_t*   buffer
    )
{
    BitstreamFeeder* bsf = (BitstreamFeeder*)feeder;
    Int32            feedingSize = 0;
    BSChunk          chunk = {0};
    PhysicalAddress  rdPtr, wrPtr;
    Uint32           room;
    DecHandle        decHandle;
    EndianMode       endian;
    
    if (bsf == NULL) {
        VLOG(ERR, "%s:%d Null handle\n", __FUNCTION__, __LINE__);
        return 0;
    }

    decHandle = bsf->decHandle;
    endian    = bsf->endian;

    if (bsf->remainData == NULL) {
        chunk.size = bsf->size; 
        chunk.data = osal_malloc(chunk.size);
        chunk.eos  = FALSE;
        if (chunk.data == NULL) {
            VLOG(ERR, "%s:%d failed to allocate memory\n", __FUNCTION__, __LINE__);
            return 0;
        }
        switch (bsf->method) {
        case FEEDING_METHOD_FIXED_SIZE:
            feedingSize = BSFeederFixedSize_Act(bsf->actualFeeder, &chunk);
            break;
        case FEEDING_METHOD_FRAME_SIZE:
            feedingSize = BSFeederFrameSize_Act(bsf->actualFeeder, &chunk);
            break;
        case FEEDING_METHOD_SIZE_PLUS_ES:
            feedingSize = BSFeederSizePlusEs_Act(bsf->actualFeeder, &chunk);
            break;
        default:
            VLOG(ERR, "%s:%d Invalid method(%d)\n", __FUNCTION__, __LINE__, bsf->method);
            osal_free(chunk.data);
            return 0;
        }
    }
    else {
        chunk.data  = bsf->remainData;
        feedingSize = bsf->remainDataSize;
    }

    bsf->observer((void*)bsf, chunk.data, feedingSize, bsf->observerArg);

    if (feedingSize < 0) {
        VLOG(ERR, "feeding size is negative value: %d\n", feedingSize);
        osal_free(chunk.data);
        return 0;
    }

    if (feedingSize > 0) {
        Uint32 coreIdx = VPU_HANDLE_CORE_INDEX(decHandle);
        Uint32 rightSize=0, leftSize=feedingSize;

        if (buffer == NULL) {
            VPU_DecGetBitstreamBuffer(decHandle, &rdPtr, &wrPtr, &room);
        }
        else {
            rdPtr = wrPtr = buffer->phys_addr;
            room  = buffer->size;
        }
        if ((Int32)room < feedingSize) {
            bsf->remainData     = chunk.data;
            bsf->remainDataSize = feedingSize;
            return 0;
        }

        if (bsf->fillingMode == BSF_FILLING_RINGBUFFER || bsf->fillingMode == BSF_FILLING_RINGBUFFER_WITH_ENDFLAG) {
            if ((wrPtr+feedingSize) >= (bsf->base+bsf->size)) {
                PhysicalAddress endAddr = bsf->base+bsf->size;
                rightSize = endAddr-wrPtr;
                leftSize  = (wrPtr+feedingSize) - endAddr;
                if (rightSize > 0) {
                    VpuWriteMem(coreIdx, wrPtr, (unsigned char*)chunk.data, rightSize, (int)endian);
                }
                wrPtr     = bsf->base;
            }
        }
        
        VpuWriteMem(coreIdx, wrPtr, (unsigned char*)chunk.data+rightSize, leftSize, (int)endian);
    }

    if (bsf->autoUpdate == TRUE) {
        /* If feedingSize is equal to zero then VPU will be ready to terminate current sequence. */
        VPU_DecUpdateBitstreamBuffer(decHandle, feedingSize);
        if (chunk.eos == TRUE || bsf->fillingMode == BSF_FILLING_RINGBUFFER_WITH_ENDFLAG) {
            VPU_DecUpdateBitstreamBuffer(decHandle, STREAM_END_SET_FLAG);
        }
    }
    bsf->eos = chunk.eos;
    osal_free(chunk.data);

    bsf->remainData     = NULL;
    bsf->remainDataSize = 0;

    return feedingSize;
}

static void FeederThread(
    void* arg
    )
{
    BitstreamFeeder* bsf = (BitstreamFeeder*)arg;

    while (bsf->eos == FALSE) {
        MSleep(10);
        FeedBitstream(bsf, NULL);
    }
}

Uint32 BitstreamFeeder_Act(
    DecHandle       decHandle,
    BSFeeder        feeder,
    EndianMode      endian,
    vpu_buffer_t*   buffer
    )
{
    BitstreamFeeder* bsf = (BitstreamFeeder*)feeder;

    bsf->decHandle = decHandle;
    bsf->endian    = endian;
    return FeedBitstream(feeder, buffer);
}

/* \param   sync    FALSE: The thread fill bitstream into a bitstream buffer continously
 *                  TRUE:  The thread start to fill bitstream into a bitstream buffer just one time.
 *                         Client MUST call BitstreamFeeder_Run() with fourth parameter TRUE for every
 *                         decoding sequence.
 */
BOOL BitstreamFeeder_Run(
    DecHandle   decHandle,
    BSFeeder    feeder,
    EndianMode  endian
    )
{
    BitstreamFeeder*    bsf = (BitstreamFeeder*)feeder;
    VpuThreadRunner     runner = (VpuThreadRunner)FeederThread;

    bsf->decHandle = decHandle;
    bsf->endian    = endian;

    bsf->threadHandle = VpuThread_Create(runner, (void*)bsf);
    if (bsf->threadHandle == NULL) {
        return FALSE;
    }

    return TRUE;
}

BOOL BitstreamFeeder_IsEos(
    BSFeeder    feeder
    )
{
    BitstreamFeeder* bsf = (BitstreamFeeder*)feeder;

    if (bsf == NULL) {
        VLOG(ERR, "%s:%d Null handle\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    return bsf->eos;
}

void BitstreamFeeder_Stop(
    BSFeeder    feeder
    )
{
    BitstreamFeeder* bsf = (BitstreamFeeder*)feeder;

    if (bsf == NULL) {
        VLOG(ERR, "%s:%d Null handle\n", __FUNCTION__, __LINE__);
        return;
    }

    if (bsf->threadHandle != NULL) {
        bsf->eos = TRUE;
        VpuThread_Join(bsf->threadHandle);
        bsf->threadHandle = NULL;
    }
}

BOOL BitstreamFeeder_Destroy(
    BSFeeder feeder
    )
{
    BitstreamFeeder* bsf = (BitstreamFeeder*)feeder;

    if (bsf == NULL) {
        VLOG(ERR, "%s:%d Null handle\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    if (bsf->threadHandle) {
        bsf->eos = TRUE;
        VpuThread_Join(bsf->threadHandle);
        bsf->threadHandle = NULL;
    }

    switch (bsf->method) {
    case FEEDING_METHOD_FIXED_SIZE:
        BSFeederFixedSize_Destroy(bsf->actualFeeder);
        break;
    case FEEDING_METHOD_FRAME_SIZE:
        BSFeederFrameSize_Destroy(bsf->actualFeeder);
        break;
    case FEEDING_METHOD_SIZE_PLUS_ES:
        BSFeederSizePlusEs_Destroy(bsf->actualFeeder);
        break;
    default:
        VLOG(ERR, "%s:%d Invalid method(%d)\n", __FUNCTION__, __LINE__, bsf->method);
        break;
    }

    if (bsf->remainData) {
        osal_free(bsf->remainData);
    }

    osal_free(bsf);

    return TRUE;
}

BOOL BitstreamFeeder_Rewind(
    BSFeeder feeder
    )
{
    BitstreamFeeder*    bsf = (BitstreamFeeder*)feeder;
    BOOL                success = FALSE;

    if (bsf == NULL) {
        VLOG(ERR, "%s:%d handle is NULL\n", __FUNCTION__, __LINE__);
        return success;
    }

    switch (bsf->method) {
    case FEEDING_METHOD_FIXED_SIZE:
        success = BSFeederFixedSize_Rewind(bsf->actualFeeder);
        break;
#ifdef SUPPORT_FFMPEG_DEMUX
    case FEEDING_METHOD_FRAME_SIZE:
        success = BSFeederFrameSize_Rewind(bsf->actualFeeder);
        break;
#endif
    case FEEDING_METHOD_SIZE_PLUS_ES:
        success = BSFeederSizePlusEs_Rewind(bsf->actualFeeder);
        break;
    default:
        VLOG(ERR, "%s:%d Invalid method(%d)\n", __FUNCTION__, __LINE__, bsf->method);
        break;
    }

    if (success == TRUE) {
        bsf->eos = FALSE;
    }

    return success;
}

BOOL BitstreamFeeder_SetHook(
    BSFeeder        feeder,
    BSFeederHook    hookFunc,
    void*           hookArg
    )
{
    BitstreamFeeder*    bsf = (BitstreamFeeder*)feeder;

    if (bsf == NULL) {
        VLOG(ERR, "%s:%d handle is NULL\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    if (hookFunc == NULL) {
        bsf->observer = (BSFeederHook)BitstreamFeeder_DummyObserver;
    }
    else {
        bsf->observer = hookFunc;
    }
    bsf->observerArg  = hookArg;

    return TRUE;
}

BOOL BitstreamFeeder_SetAutoUpdate(
    BSFeeder        feeder,
    BOOL            onoff
    )
{
    BitstreamFeeder*    bsf = (BitstreamFeeder*)feeder;

    if (bsf == NULL) {
        VLOG(ERR, "%s:%d handle is NULL\n", __FUNCTION__, __LINE__);
        return FALSE;
    }
    bsf->autoUpdate = onoff;

    return TRUE;
}
 
