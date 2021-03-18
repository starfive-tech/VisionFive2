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

#include <string.h>
#include "component.h"

typedef enum {
    READER_STATE_OPEN,
    READER_STATE_READING,
} EncReaderState;

typedef struct {
    EncHandle       handle;
    Uint32          streamBufCount;
    Uint32          streamBufSize;
    vpu_buffer_t*   bsBuffer;
    Uint32          coreIdx;
    EndianMode      streamEndian;
    EncReaderState  state;
    char            bitstreamFileName[MAX_FILE_PATH];
    BitstreamReader bsReader;
    osal_file_t     fp;
    BOOL            ringBuffer;
    BOOL            ringBufferWrapEnable;
    Int32           productID;
} ReaderContext;

static CNMComponentParamRet GetParameterReader(ComponentImpl* from, ComponentImpl* com, GetParameterCMD commandType, void* data) 
{
    ReaderContext*           ctx    = (ReaderContext*)com->context;
    BOOL                     result = TRUE;
    ParamEncBitstreamBuffer* bsBuf  = NULL;
    PortContainer*          container;

    switch(commandType) {
    case GET_PARAM_COM_IS_CONTAINER_CONUSUMED:
        container = (PortContainer*)data;
        container->consumed = TRUE;
        break;
    case GET_PARAM_READER_BITSTREAM_BUF:
        if (ctx->bsBuffer == NULL) return CNM_COMPONENT_PARAM_NOT_READY;
        bsBuf      = (ParamEncBitstreamBuffer*)data;
        bsBuf->bs  = ctx->bsBuffer;
        bsBuf->num = ctx->streamBufCount;
        break;
    default:
        return CNM_COMPONENT_PARAM_NOT_FOUND;
    }

    return (result == TRUE) ? CNM_COMPONENT_PARAM_SUCCESS : CNM_COMPONENT_PARAM_FAILURE;
}

static CNMComponentParamRet SetParameterReader(ComponentImpl* from, ComponentImpl* com, SetParameterCMD commandType, void* data) 
{
    BOOL result = TRUE;

    switch(commandType) {
    default:
        result = FALSE;
        break;
    }

    return (result == TRUE) ? CNM_COMPONENT_PARAM_SUCCESS : CNM_COMPONENT_PARAM_FAILURE;
}

static BOOL ExecuteReader(ComponentImpl* com, PortContainer* in, PortContainer* out) 
{
    ReaderContext*          ctx     = (ReaderContext*)com->context;
    PortContainerES*        srcData = (PortContainerES*)in;
    BOOL                    success = TRUE;
    CNMComponentParamRet    ret;

    srcData->reuse = FALSE;

    switch (ctx->state) {
    case READER_STATE_OPEN:
        ret = ComponentGetParameter(com, com->srcPort.connectedComponent, GET_PARAM_ENC_HANDLE, &ctx->handle); 
        if (ComponentParamReturnTest(ret, &success) == FALSE) {
            return success;
        }
        if ( ctx->bitstreamFileName[0] != 0) {
            ctx->bsReader = BitstreamReader_Create(BUFFER_MODE_TYPE_LINEBUFFER, ctx->bitstreamFileName, ctx->streamEndian, &ctx->handle);
            if (ctx->bsReader == NULL) {
                VLOG(ERR, "%s:%d Failed to BitstreamReader_Create\n", __FUNCTION__, __LINE__);
                return FALSE;
            }
        }
    ctx->state = READER_STATE_READING;
        srcData->reuse  = TRUE;
        
        break;
    case READER_STATE_READING:
        if (srcData->size > 0 || 
            (ctx->ringBuffer == TRUE && ctx->ringBufferWrapEnable == FALSE && srcData->last == TRUE) ) {
            if ( ctx->ringBuffer == TRUE) {
                Uint8* buf = (Uint8*)osal_malloc(srcData->size);
                PhysicalAddress rd, paBsBufStart, paBsBufEnd;
                Int32           readSize;
                Uint32          room;

                rd = srcData->rdPtr;
                paBsBufStart = srcData->paBsBufStart;
                paBsBufEnd   = srcData->paBsBufEnd;
                readSize = srcData->size;

                if (ctx->ringBufferWrapEnable == TRUE ) {
                    if (ctx->bsReader != 0) {
                        if ((rd+readSize) > paBsBufEnd) {
                            room = paBsBufEnd - rd;
                            vdi_read_memory(ctx->coreIdx, rd, buf, room,  ctx->streamEndian);
                            vdi_read_memory(ctx->coreIdx, paBsBufStart, buf+room, (readSize-room), ctx->streamEndian);
                        } 
                        else {
                            vdi_read_memory(ctx->coreIdx, rd, buf, readSize, ctx->streamEndian);
                        }
                    }
                    VPU_EncUpdateBitstreamBuffer(ctx->handle, readSize);
                    if (ctx->fp) {
                        osal_fwrite(buf, readSize, 1, ctx->fp);
                    }
                }
                else { //ring=1, wrap=0
                    if (srcData->streamBufFull == TRUE || srcData->last == TRUE) { // read whole data at once and no UpdateBS
                        if (ctx->bsReader != 0) {
                            vdi_read_memory(ctx->coreIdx, rd, buf, readSize, ctx->streamEndian);
                        }
                        if (ctx->fp) {
                            osal_fwrite(buf, readSize, 1, ctx->fp);
                        }
                    }
                }
                if (srcData->streamBufFull == TRUE) {
                    if (ctx->ringBufferWrapEnable == FALSE) {
                        vdi_free_dma_memory(ctx->coreIdx, &srcData->buf, ENC_BS, ctx->handle->instIndex);
                        osal_memcpy(ctx->bsBuffer, &srcData->newBsBuf, sizeof(*ctx->bsBuffer));
                    }
                    srcData->buf.size = 0;
                }
                osal_free(buf);
            }
            else { /* line buffer mode */
                Uint8* buf = (Uint8*)osal_malloc(srcData->size);
                if (ctx->bsReader != 0) {
                    //buf = (Uint8*)osal_malloc(srcData->size);
                    BitstreamReader_Act(ctx->bsReader, srcData->buf.phys_addr, srcData->buf.size, srcData->size, NULL);

                    if(PRODUCT_ID_NOT_W_SERIES(ctx->productID) && TRUE == srcData->streamBufFull) {
                        VPU_ClearInterrupt(ctx->coreIdx);
                    }
                }
                if (srcData->streamBufFull == TRUE) {
                    srcData->buf.phys_addr = 0;
                }
                if (ctx->fp) {
                    osal_fwrite(buf, srcData->size, 1, ctx->fp);
                }
                if(buf)
                    osal_free(buf);
            }
            if (TRUE == srcData->streamBufFull) {
                srcData->streamBufFull = FALSE;
            }
        }

        srcData->consumed = TRUE;
        com->terminate    = srcData->last;
        break;
    default:
        success = FALSE;
        break;
    }

    return success;
}

static BOOL PrepareReader(ComponentImpl* com, BOOL* done)
{
    ReaderContext*       ctx = (ReaderContext*)com->context;
    Uint32               i;
    vpu_buffer_t*        bsBuffer;
    Uint32               num = ctx->streamBufCount;
 
    *done = FALSE;

    bsBuffer = (vpu_buffer_t*)osal_malloc(num*sizeof(vpu_buffer_t));
    for (i = 0; i < num; i++) {
        bsBuffer[i].size = ctx->streamBufSize;
        if (vdi_allocate_dma_memory(ctx->coreIdx, &bsBuffer[i], ENC_BS, /*ctx->handle->instIndex*/0) < 0) {
            VLOG(ERR, "%s:%d fail to allocate bitstream buffer\n", __FUNCTION__, __LINE__);
            osal_free(bsBuffer);
            return FALSE;
        }
    }
    ctx->bsBuffer   = bsBuffer;
    ctx->state      = READER_STATE_OPEN;

    *done = TRUE;

    return TRUE;
}

static void ReleaseReader(ComponentImpl* com)
{
    ReaderContext*  ctx = (ReaderContext*)com->context;
    Uint32          i   = 0;

    if (ctx->bsBuffer != NULL) {
        for (i = 0; i < ctx->streamBufCount ; i++) {
            if (ctx->bsBuffer[i].size)
                vdi_free_dma_memory(ctx->coreIdx, &ctx->bsBuffer[i], ENC_BS, 0);
        }
    }
}

static BOOL DestroyReader(ComponentImpl* com) 
{
    ReaderContext*  ctx = (ReaderContext*)com->context;

    if (ctx->bsBuffer != NULL) osal_free(ctx->bsBuffer);
    if (ctx->fp) osal_fclose(ctx->fp);
    osal_free(ctx);

    return TRUE;
}

static Component CreateReader(ComponentImpl* com, CNMComponentConfig* componentParam) 
{
    ReaderContext* ctx;

    com->context = osal_malloc(sizeof(ReaderContext));
    ctx = (ReaderContext*)com->context;
    osal_memset((void*)ctx, 0, sizeof(ReaderContext));

    strcpy(ctx->bitstreamFileName, componentParam->testEncConfig.bitstreamFileName);
    ctx->handle       = NULL;
    ctx->coreIdx      = componentParam->testEncConfig.coreIdx;
    ctx->productID    = componentParam->testEncConfig.productId;
    ctx->streamEndian = (EndianMode)componentParam->testEncConfig.stream_endian;

    ctx->streamBufCount = componentParam->encOpenParam.streamBufCount;
    ctx->streamBufSize  = componentParam->encOpenParam.streamBufSize;
    ctx->ringBuffer     = componentParam->encOpenParam.ringBufferEnable;
    ctx->ringBufferWrapEnable = componentParam->encOpenParam.ringBufferWrapEnable;

    return (Component)com;
}

ComponentImpl readerComponentImpl = {
    "reader",
    NULL,
    {0,},
    {0,},
    sizeof(PortContainer),
    5,
    CreateReader,
    GetParameterReader,
    SetParameterReader,
    PrepareReader,
    ExecuteReader,
    ReleaseReader,
    DestroyReader,
};

