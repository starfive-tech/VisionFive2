/*
 * Copyright (c) 2019, Chips&Media
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

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
    Uint32          coreIdx;
    Uint32          room;
    Uint32          fillingMode;
    BOOL            eos;
    DecHandle       decHandle;
    EndianMode      endian;
    BSFeederHook    observer;
    void*           observerArg;
    vpu_buffer_t    bitBuffer;
    int             sequenceHeaderSize;
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
    CodStd      codecId
    );

extern BOOL BSFeederFixedSize_Destroy(
    void* feeder
    );

extern Int32 BSFeederFixedSize_Act(
    void*       feeder,
    BSChunk*    chunk
    );

extern BOOL BSFeederFixedSize_Rewind(
    void*       feeder
    );

extern void BSFeederFixedSize_SetFeedingSize(
    void*   feeder,
    Uint32  feedingSize
    );

extern void* BSFeederFrameSize_Create(
    const char* path,
    CodStd      codec
    );

extern BOOL BSFeederFrameSize_Destroy(
    void*   feeder
    );

extern Int32 BSFeederFrameSize_Act(
    void*       feeder,
    BSChunk*    packet
    );

extern BOOL BSFeederFrameSize_Rewind(
    void*       feeder
    );

extern void* BSFeederSizePlusEs_Create(
    const char* path,
    CodStd      codecId
    );

extern BOOL BSFeederSizePlusEs_Destroy(
    void* feeder
    );

extern Int32 BSFeederSizePlusEs_Act(
    void*       feeder,
    BSChunk*    chunk
    );

extern BOOL BSFeederSizePlusEs_Rewind(
    void*       feeder
    );

extern Int32 BSFeederFrameSize_GetStandard(
    void*       feeder
    );

/**
* Abstract Bitstream Feeader Functions
*/
void* BitstreamFeeder_Create(
    Uint32          coreIdx,
    const char*     path,
    CodStd          codecId,
    FeedingMethod   method,
    EndianMode      endian
    )
{
    /*lint -esym(438, ap) */
    BitstreamFeeder*    handle = NULL;
    void*               feeder = NULL;

    switch (method) {
    case FEEDING_METHOD_FIXED_SIZE:
        feeder = BSFeederFixedSize_Create(path, codecId);
        break;
    case FEEDING_METHOD_FRAME_SIZE:
        feeder = BSFeederFrameSize_Create(path, codecId);
        break;
    case FEEDING_METHOD_SIZE_PLUS_ES:
        feeder = BSFeederSizePlusEs_Create(path, codecId);
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
        handle->fillingMode  = (method == FEEDING_METHOD_FIXED_SIZE) ? BSF_FILLING_RINGBUFFER : BSF_FILLING_LINEBUFFER;
        handle->eos          = FALSE;
        handle->observer     = (BSFeederHook)BitstreamFeeder_DummyObserver;
        handle->observerArg  = NULL;
        handle->endian       = endian;
        handle->coreIdx      = coreIdx;
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

BOOL BitstreamFeeder_SetFeedingSize(
    BSFeeder    feeder,
    Uint32      size
    )
{
    BitstreamFeeder* bsf = (BitstreamFeeder*)feeder;
    BOOL             ret = FALSE;

    if (bsf->method == FEEDING_METHOD_FIXED_SIZE) {
        BSFeederFixedSize_SetFeedingSize(bsf->actualFeeder, size);
        ret = TRUE;
    }

    return ret;
}

Uint32 BitstreamFeeder_Act(
    BSFeeder            feeder,
    vpu_buffer_t*       bsBuffer,
    PhysicalAddress     wrPtr,
    Uint32              room,
    PhysicalAddress*    newWrPtr
    )
{
    BitstreamFeeder* bsf = (BitstreamFeeder*)feeder;
    Int32            feedingSize = 0;
    BSChunk          chunk = {0};
    EndianMode       endian;

    if (bsf == NULL) {
        VLOG(ERR, "%s:%d Null handle\n", __FUNCTION__, __LINE__);
        return 0;
    }

    endian = bsf->endian;

    if (bsf->remainData == NULL) {
        chunk.size = bsBuffer->size;
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
        Uint32          rightSize=0, leftSize=feedingSize;
        PhysicalAddress base = bsBuffer->phys_addr;
        Uint32          size = bsBuffer->size;

        if ((Int32)room < feedingSize) {
            bsf->remainData     = chunk.data;
            bsf->remainDataSize = feedingSize;
            return 0;
        }

        if (bsf->fillingMode == BSF_FILLING_RINGBUFFER || bsf->fillingMode == BSF_FILLING_RINGBUFFER_WITH_ENDFLAG) {
            if ((wrPtr+feedingSize) >= (base+size)) {
                PhysicalAddress endAddr = base+size;
                rightSize = endAddr-wrPtr;
                leftSize  = (wrPtr+feedingSize) - endAddr;
                if (rightSize > 0) {
                    VpuWriteMem(bsf->coreIdx, wrPtr, (unsigned char*)chunk.data, rightSize, (int)endian);
                    VLOG(TRACE, "<%s:%d> WRPTR: %p SIZE: %x\n", __FUNCTION__, __LINE__, wrPtr, rightSize);
                }
                wrPtr = base;
            }
        }

        VpuWriteMem(bsf->coreIdx, wrPtr, (unsigned char*)chunk.data+rightSize, leftSize, (int)endian);
        *newWrPtr = wrPtr + leftSize;
    }

    bsf->eos = chunk.eos;
    bsf->sequenceHeaderSize = chunk.seqHeaderSize;
    osal_free(chunk.data);

    bsf->remainData     = NULL;
    bsf->remainDataSize = 0;

    return feedingSize;
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

Uint32 BitstreamFeeder_GetSeqHeaderSize(
	BSFeeder    feeder
	)
{
	BitstreamFeeder* bsf = (BitstreamFeeder*)feeder;

	if (bsf == NULL) {
		VLOG(ERR, "%s:%d Null handle\n", __FUNCTION__, __LINE__);
		return FALSE;
	}
	return bsf->sequenceHeaderSize;
}

Int32 BitstreamFeeder_GetStandard(
    BSFeeder    feeder
    )
{
    BitstreamFeeder* bsf = (BitstreamFeeder*)feeder;
    Int32 standard = -1;

    if (bsf == NULL) {
        VLOG(ERR, "%s:%d Null handle\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    switch (bsf->method) {
    case FEEDING_METHOD_FRAME_SIZE:
        standard = BSFeederFrameSize_GetStandard(bsf->actualFeeder);
        break;
    case FEEDING_METHOD_FIXED_SIZE:
    case FEEDING_METHOD_SIZE_PLUS_ES:
    default:
        VLOG(ERR, "%s:%d Invalid return value (%d),  method(%d)\n", __FUNCTION__, __LINE__, -1, bsf->method);
        break;
    }
    return standard;
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
}

BOOL BitstreamFeeder_Destroy(
    BSFeeder feeder
    )
{
    BitstreamFeeder* bsf = (BitstreamFeeder*)feeder;

    if (bsf == NULL) {
        return FALSE;
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
    case FEEDING_METHOD_FRAME_SIZE:
        success = BSFeederFrameSize_Rewind(bsf->actualFeeder);
        break;
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

