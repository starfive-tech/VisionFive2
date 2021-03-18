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
#include <time.h>
#include "component.h"

#define STREAM_BUF_SIZE_HEVC                (10*1024*1024)  // max bitstream size(HEVC:10MB,VP9:not specified)
#define STREAM_BUF_SIZE_VP9                 (20*1024*1024)  // max bitstream size(HEVC:10MB,VP9:not specified)
#define STREAM_BUF_SIZE                     0x700000
#define FEEDING_SIZE                        0x20000

typedef struct {
    TestDecConfig   config;
    vpu_buffer_t*   bsBuffer;
    Uint32          numBuffers;
    BSFeeder        feeder;
    PhysicalAddress nextWrPtr;
    Uint32          bsMargin;
    Uint32          loopCount;
    Uint32          dropCount;
    DecHandle       handle;
} FeederContext;

static BOOL IsStartCode(Uint8* pData)
{
    // find 00 00 00 01
    if (pData[0] == 0x00 && pData[1] == 0x00 && pData[2] == 0x00 && pData[3] == 0x01)
        return TRUE;
    else
        return FALSE;
}

static BOOL FindSPSAndPPS(Uint8* pData, Uint32 size)
{
    Uint32 k;
    Uint32 nalUnitType = 0;

    for (k = 0; k < size; k += 4) {
        if (IsStartCode((pData + k)) == TRUE) {
            nalUnitType = (pData[k+4] >> 1) & 0x3F;
            if (nalUnitType == 33 || nalUnitType == 34) {
                VLOG(ERR, "Find SPS/PPS : %d\n", nalUnitType);
                return TRUE;
            }
        }
    }
    return FALSE;
}

static void InjectErrorEvery188(BSFeeder bsf, void* data, Uint32 size, void* arg)
{
    Uint8*      pData = (Uint8*)data;
    Uint8       val;
    Uint32      pos;
    Uint32      i, nchunk, blockSize;

    UNREFERENCED_PARAMETER(bsf);

    if (randomSeed == 0) {
        randomSeed = (Uint32)time(NULL);
        srand(randomSeed);
        VLOG(INFO, "RANDOM SEED: %d\n", randomSeed);
    }

    if (size == 0) {
        return;
    }

    blockSize = (size < 188) ? size : 188;

    nchunk = (size + blockSize -1) / blockSize;
    for (i=0; i<nchunk; i++) {
        pos = GetRandom(0, blockSize-1);
        pData = ((Uint8*)data + i*blockSize);
        if (FindSPSAndPPS(pData, blockSize) == FALSE) {
            /* size is 188 */
            val = pData[pos];
            val = ~val;
            pData[pos] = val;
            pos = (pos+1) % blockSize;
        }
        else {
            // if find SPS or PPS, skip current chunk and next chunk.
            i++;
        }
    }
    //VLOG(TRACE, "CREATE ERROR BIT!!!\n");
}

static CNMComponentParamRet GetParameterFeeder(ComponentImpl* from, ComponentImpl* com, GetParameterCMD commandType, void* data)
{
    FeederContext*              ctx     = (FeederContext*)com->context;
    ParamDecBitstreamBuffer*    bsBuf   = NULL;
    PortContainer*              container;

    switch(commandType) {
    case GET_PARAM_COM_IS_CONTAINER_CONUSUMED:
        // This query command is sent from the comonponent core.
        // If input data are consumed in sequence, it should return TRUE through PortContainer::consumed.
        container = (PortContainer*)data;
        container->consumed = TRUE;
        break;
    case GET_PARAM_FEEDER_BITSTREAM_BUF:
        if (ctx->bsBuffer == NULL) return CNM_COMPONENT_PARAM_NOT_READY;
        bsBuf = (ParamDecBitstreamBuffer*)data;
        bsBuf->num = ctx->numBuffers;
        bsBuf->bs  = ctx->bsBuffer;
        break;
    case GET_PARAM_FEEDER_EOS:
        if (ctx->feeder == NULL) return CNM_COMPONENT_PARAM_NOT_READY;
        *(BOOL*)data = BitstreamFeeder_IsEos(ctx->feeder);
        break;
    default:
        return CNM_COMPONENT_PARAM_NOT_FOUND;
    }

    return CNM_COMPONENT_PARAM_SUCCESS;
}

static CNMComponentParamRet SetParameterFeeder(ComponentImpl* from, ComponentImpl* com, SetParameterCMD commandType, void* data)
{
    CNMComponentParamRet ret = CNM_COMPONENT_PARAM_SUCCESS;
    FeederContext*       ctx = (FeederContext*)com->context;

    switch(commandType) {
    case SET_PARAM_COM_PAUSE:
        com->pause = *(BOOL*)data;
        break;
    case SET_PARAM_FEEDER_START_INJECT_ERROR:
        BitstreamFeeder_SetHook(ctx->feeder, InjectErrorEvery188, NULL);
        break;
    case SET_PARAM_FEEDER_STOP_INJECT_ERROR:
        BitstreamFeeder_SetHook(ctx->feeder, NULL, NULL);
        break;
    case SET_PARAM_FEEDER_RESET:
        ctx->nextWrPtr = ctx->bsBuffer[0].phys_addr;
        ctx->dropCount = 2;     /* Drop remaining error injected packets */
        break;
    default:
        ret = CNM_COMPONENT_PARAM_NOT_FOUND;
        break;
    }

    return ret;
}

static BOOL PrepareFeeder(ComponentImpl* com, BOOL *done)
{
    FeederContext*  ctx    = (FeederContext*)com->context;
    TestDecConfig*  config = &ctx->config;
    Uint32          i;
    vpu_buffer_t*   bsBuffer;

    bsBuffer = (vpu_buffer_t*)osal_malloc(com->numSinkPortQueue*sizeof(vpu_buffer_t));
    for (i=0; i<ctx->numBuffers; i++) {
        bsBuffer[i].size = config->bsSize;
        if (vdi_allocate_dma_memory(config->coreIdx, &bsBuffer[i], DEC_BS, 0) < 0) {    // can't know instIdx now.
            VLOG(ERR, "%s:%d fail to allocate bitstream buffer\n", __FUNCTION__, __LINE__);
            return FALSE;
        }
    }

    if (config->bitstreamMode == BS_MODE_INTERRUPT) {
        ctx->nextWrPtr = bsBuffer[0].phys_addr;
    }
    else {
        Queue* srcQ  = com->sinkPort.inputQ;
        Queue* tempQ = Queue_Copy(NULL, srcQ);
        PortContainerES* out;

        // Flush all data.
        Queue_Flush(srcQ);

        for (i=0; i<ctx->numBuffers; i++) {
            out = (PortContainerES*)Queue_Dequeue(tempQ);
            out->buf = bsBuffer[i];
            Queue_Enqueue(srcQ, (void*)out);
        }
        Queue_Destroy(tempQ);
    }
    ctx->bsBuffer = bsBuffer;
    *done = TRUE;

    return TRUE;
}

static BOOL ExecuteFeeder(ComponentImpl* com, PortContainer* in, PortContainer* out)
{
    FeederContext*          ctx       = (FeederContext*)com->context;
    vpu_buffer_t*           bsBuffer  = ctx->bsBuffer;
    BSFeeder                bsFeeder  = ctx->feeder;
    TestDecConfig*          config    = &ctx->config;
    PortContainerES*        container = (PortContainerES*)out;
    PhysicalAddress         wrPtr;
    Int32                   room;
    BOOL                    eos         = FALSE;


    if (com->pause) return TRUE;

    eos = BitstreamFeeder_IsEos(bsFeeder);

    if (config->bitstreamMode == BS_MODE_PIC_END) {
        bsBuffer = &container->buf;
        wrPtr    = bsBuffer->phys_addr;
        room     = bsBuffer->size;
        container->size = BitstreamFeeder_Act(bsFeeder, bsBuffer, wrPtr, (Uint32)room, &ctx->nextWrPtr);
    }
    else {
        ParamDecBitstreamBufPos bsPos;
        BOOL                    success;
        if (eos == FALSE) {
            CNMComponentParamRet ret = ComponentGetParameter(com, com->sinkPort.connectedComponent, GET_PARAM_DEC_BITSTREAM_BUF_POS, (void*)&bsPos);
            if (ComponentParamReturnTest(ret, &success) == FALSE) {
                if (success == FALSE) VLOG(ERR, "%s:%d return FALSE\n", __FUNCTION__, __LINE__);
                return success;
            }
            bsBuffer = &bsBuffer[0];

            wrPtr = (ctx->nextWrPtr > 0) ? ctx->nextWrPtr :  bsPos.wrPtr;

            if (wrPtr >= bsPos.rdPtr) {
                room  = (bsBuffer->phys_addr + bsBuffer->size) - wrPtr;
                room += (bsPos.rdPtr - bsBuffer->phys_addr - 1) - ctx->bsMargin;
            }
            else {
                room = bsPos.rdPtr - wrPtr - ctx->bsMargin - 1;
            }
            if (room < 0) room = 0;
            container->size = BitstreamFeeder_Act(bsFeeder, bsBuffer, wrPtr, (Uint32)room, &ctx->nextWrPtr);
            if (ctx->dropCount > 0) {
                container->size = 0;
                ctx->dropCount--;
                ctx->nextWrPtr = wrPtr;
            }
        }
        else {
            if (ctx->loopCount > 0) {
                eos = FALSE;
                BitstreamFeeder_Rewind(bsFeeder);
                ctx->loopCount--;
            }
            container->size = 0;
        }
    }

    container->last  = (config->streamEndFlag == TRUE) ? TRUE : eos;
    container->buf   = *bsBuffer;
    container->reuse = (container->size == 0 && container->last == FALSE);

    /* To stop sending an element stream.
       A user will clear pause state when he gets the stream-end flag form the VPU */

    com->pause = config->streamEndFlag;

    return TRUE;
}

static void ReleaseFeeder(ComponentImpl* com)
{
    FeederContext* ctx    = (FeederContext*)com->context;
    TestDecConfig* config = &ctx->config;
    Uint32 i = 0;

    if (NULL != ctx->bsBuffer) {
        for (i = 0; i < ctx->numBuffers; i++) {
            vdi_free_dma_memory(config->coreIdx, &ctx->bsBuffer[i], DEC_BS, 0);
        }
    }
}

static BOOL DestroyFeeder(ComponentImpl* com) {
    FeederContext* ctx = (FeederContext*)com->context;

    if (ctx->bsBuffer != NULL) osal_free(ctx->bsBuffer);

    BitstreamFeeder_Destroy(ctx->feeder);

    osal_free(ctx);

    return TRUE;
}

static Component CreateFeeder(ComponentImpl* com, CNMComponentConfig* comConfig)
{
    FeederContext*  ctx;
    BSFeeder        feeder;
    TestDecConfig*  config;

    com->context = osal_malloc(sizeof(FeederContext));
    ctx = (FeederContext*)com->context;
    osal_memset((void*)ctx, 0, sizeof(FeederContext));

    config = &ctx->config;
    osal_memcpy((void*)config, (void*)&comConfig->testDecConfig, sizeof(TestDecConfig));

    if (PRODUCT_ID_W_SERIES(config->productId)) {
        ctx->numBuffers = (config->bitstreamMode == BS_MODE_PIC_END) ? com->numSinkPortQueue: 1;
    } else {
        ctx->numBuffers = 1;
    }

    if (config->bsSize == 0) {
        if(PRODUCT_ID_W_SERIES(config->productId)) {
            config->bsSize = (config->bitFormat == STD_VP9) ? STREAM_BUF_SIZE_VP9 : STREAM_BUF_SIZE_HEVC;
        } else {
            config->bsSize = STREAM_BUF_SIZE;
        }
    }

    ctx->bsMargin  = PRODUCT_ID_W_SERIES(config->productId) ? 0 : 2048; /* 2048 = GBU_SIZE x 2 */
    ctx->loopCount = (config->loopCount > 0) ? (config->loopCount-1) : 0;

    if (PRODUCT_ID_NOT_W_SERIES(config->productId)) {
        if (BS_MODE_PIC_END == config->bitstreamMode || STD_THO == config->bitFormat) {
            config->feedingMode = FEEDING_METHOD_FRAME_SIZE;
        }
    }

    if ((feeder=BitstreamFeeder_Create(config->coreIdx, config->inputPath, config->bitFormat, config->feedingMode, config->streamEndian)) == NULL) {
        return NULL;
    }

    BitstreamFeeder_SetFeedingSize(feeder, config->feedingSize);

#ifdef SUPPORT_FFMPEG_DEMUX
#endif /* SUPPORT_FFMPEG_DEMUX */
    if (config->streamEndFlag == TRUE) {
        BitstreamFeeder_SetFillMode(feeder, BSF_FILLING_RINGBUFFER_WITH_ENDFLAG);
    }
    else {
        BitstreamFeeder_SetFillMode(feeder, (config->bitstreamMode == BS_MODE_PIC_END) ? BSF_FILLING_LINEBUFFER : BSF_FILLING_RINGBUFFER);
    }

    if (config->errorInject == TRUE) {
        BitstreamFeeder_SetHook(feeder, InjectErrorEvery188, NULL);
    }


    ctx->feeder = feeder;

    return (Component)com;
}

ComponentImpl feederComponentImpl = {
    "feeder",
    NULL,
    {0,},
    {0,},
    sizeof(PortContainerES),
    5,
    CreateFeeder,
    GetParameterFeeder,
    SetParameterFeeder,
    PrepareFeeder,
    ExecuteFeeder,
    ReleaseFeeder,
    DestroyFeeder
};

