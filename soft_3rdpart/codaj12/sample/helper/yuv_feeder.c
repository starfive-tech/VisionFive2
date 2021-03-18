/*
 * Copyright (c) 2018, Chips&Media
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
#include <string.h>
#include <assert.h>
#include "yuv_feeder.h"
#include "platform.h"
#include "jpulog.h"

#define FB_QUEUE_SIZE           10

typedef struct {
    FILE*   fp;
    Uint32  frameSize;
    Uint32  lumaSize;
    Uint32  chromaSize;
    Uint32  lumaLineWidth;
    Uint32  lumaHeight;
    Uint32  chromaLineWidth;
    Uint32  chromaHeight;
    Uint8*  pYuv;
    Uint32  fbEndian;
    Uint32  currentRow;
} IYuvContext;

static void DefaultListener(
    YuvFeederListenerArg* arg
    )
{
    UNREFERENCED_PARAMETER(arg);
}

static void CalcYuvFrameSize(
    YuvAttr*    attr,
    IYuvContext* ctx
    )
{
    Uint32 lSize = 0, cSize = 0, divc=1;
    Uint32 Bpp   = (attr->bpp+7)>>3;
    Uint32 divw, divh;

    divw = divh = 1;
    switch (attr->format) {
    case FORMAT_400:
        /* No chroma data */
        divw = divh = 0;
        break;
    case FORMAT_420:
        divw = 2;
        divh = 2;
        break;
    case FORMAT_422:
        divw = 2;
        divh = 1;
        break;
    case FORMAT_440:
        divw = 1;
        divh = 2;
        break;
    case FORMAT_444:
        break;
    default:
        JLOG(WARN, "%s:%d NOT SUPPORTED YUV FORMAT: %d\n", __FUNCTION__, __LINE__, attr->format);
        break;
    }
    divc                 = divw * divh;
    lSize                = attr->width * attr->height * Bpp;
    cSize                = divc == 0 ? 0 : lSize / divc;
    ctx->frameSize       = lSize + 2*cSize;
    ctx->lumaSize        = lSize;
    ctx->chromaSize      = cSize;
    ctx->lumaLineWidth   = attr->width * Bpp;
    ctx->lumaHeight      = attr->height;
    ctx->chromaLineWidth = divw == 0 ? 0 : attr->width * Bpp / divw;
    ctx->chromaHeight    = divh == 0 ? 0 : attr->height / divh;

    if (attr->packedFormat != PACKED_FORMAT_NONE) {
        if (attr->packedFormat == PACKED_FORMAT_444) {
            ctx->lumaLineWidth *= 3;
        }
        else {
            ctx->lumaLineWidth *= 2;
        }
        ctx->chromaLineWidth = 0;
        ctx->chromaHeight    = 0;
    }
    else {
        if (attr->chromaInterleaved != CBCR_SEPARATED) {
            ctx->chromaLineWidth *= 2;
        }
    }
}


static void CopyYuvData(PhysicalAddress fbAddr, Uint32 fbStride, Uint8* data, Uint32 dataStride, Uint32 dataHeight, Uint32 endian)
{
    PhysicalAddress addr    = fbAddr;
    Uint8*          pData   = data;
    Uint8*          newData = NULL;
    Uint32          i;

    if (dataStride < fbStride) {
        if (NULL == (newData=(Uint8*)malloc(fbStride))) {
            JLOG(ERR, "<%s> failed to allocate memory\n", __FUNCTION__);
            return;
        }
        memset(newData, 0x00, fbStride);
    }

    for (i=0; i<dataHeight; i++) {
        if (NULL != newData) {
            memcpy(newData, pData, dataStride);
            jdi_write_memory(addr, newData, fbStride, endian);
        }
        else {
            jdi_write_memory(addr, pData, fbStride, endian);
        }
        pData += dataStride;
        addr  += fbStride;
    }

    if (newData) free(newData);
}

/* @return  height of picture on success, -1 on failure
 */
static Int32 LoadFrameFromFile(
    IYuvContext*    ctx,
    FrameBuffer*    fb,
    YuvAttr         attr,
    Uint32          endian
    )
{
    Uint32  nread;
    BOOL    success = TRUE;
    Uint8*  pY;
    Uint8*  pU;
    Uint8*  pV;

    if ((nread=fread(ctx->pYuv, 1, ctx->frameSize, ctx->fp)) != ctx->frameSize) {
        JLOG(WARN, "%s:%d INSUFFICIENT FRAME DATA!!!(%d)\n", __FUNCTION__, __LINE__, ctx->frameSize);
        success = FALSE;
    }


    pY = ctx->pYuv;
    switch (attr.format) {
    case FORMAT_400:
        CopyYuvData(fb->bufY, fb->stride, pY, ctx->lumaLineWidth, ctx->lumaHeight, endian);
        /* No chroma data */
        break;
    case FORMAT_420:
    case FORMAT_422:
    case FORMAT_440:
    case FORMAT_444:
        if (attr.packedFormat == PACKED_FORMAT_NONE) {
            CopyYuvData(fb->bufY, fb->stride, pY, ctx->lumaLineWidth, ctx->lumaHeight, endian);
            pU = pY + ctx->lumaSize;
            if (attr.chromaInterleaved == CBCR_SEPARATED) {
                CopyYuvData(fb->bufCb, fb->strideC, pU, ctx->chromaLineWidth, ctx->chromaHeight, endian);
                pV = pU + ctx->chromaSize;
                CopyYuvData(fb->bufCr, fb->strideC, pV, ctx->chromaLineWidth, ctx->chromaHeight, endian);
            }
            else {
                CopyYuvData(fb->bufCb, fb->strideC, pU, ctx->chromaLineWidth, ctx->chromaHeight, endian);
            }
        }
        else {
            CopyYuvData(fb->bufY, fb->stride, pY, ctx->lumaLineWidth, ctx->lumaHeight, endian);
        }
        break;
    default:
        JLOG(ERR, "%s:%d NOT SUPPORTED YUV FORMAT:%d\n", __FUNCTION__, __LINE__, attr.format);
        success = FALSE;
        break;
    }

    return success == FALSE ? -1 : attr.height;
}

/* @return  It returns the num of rows so far.
 */
static Int32 LoadSliceFromFile(
    IYuvContext*    ctx,
    FrameBuffer*    fb,
    YuvAttr         attr,
    Uint32          endian
    )
{
    Uint32  nread;
    BOOL    success = TRUE;
    Uint8*  pY;
    Uint8*  pU;
    Uint8*  pV;
    Uint32  lumaOffset, chromaOffset;

    if (ctx->currentRow == attr.height) {
        ctx->currentRow = 0;
    }

    if (ctx->currentRow == 0) {
        if ((nread=fread(ctx->pYuv, 1, ctx->frameSize, ctx->fp)) != ctx->frameSize) {
            JLOG(WARN, "%s:%d INSUFFICIENT FRAME DATA!!!\n", __FUNCTION__, __LINE__);
            success = FALSE;
        }
    }

    lumaOffset = ctx->currentRow * ctx->lumaLineWidth;
    pY = ctx->pYuv;
    switch (attr.format) {
    case FORMAT_400:
        jdi_write_memory(fb->bufY+lumaOffset, pY+lumaOffset, ctx->lumaLineWidth, endian);
        ctx->currentRow++;
        /* No chroma data */
        break;
    case FORMAT_420:
    case FORMAT_422:
    case FORMAT_440:
    case FORMAT_444:
        if (attr.packedFormat == PACKED_FORMAT_NONE) {
            Uint32 bytesPerPixel = (attr.format == FORMAT_422) ? 2 : 3;
            Uint32 offset        = ctx->currentRow * ctx->lumaLineWidth * bytesPerPixel;
            Uint32 lineWidth     = ctx->lumaLineWidth * bytesPerPixel;
            jdi_write_memory(fb->bufY+offset, pY+offset, lineWidth, endian);
            ctx->currentRow++;
        }
        else {
            Uint32 currentChromaRow = (attr.format == FORMAT_444) ? ctx->currentRow : ctx->currentRow/2;

            jdi_write_memory(fb->bufY+lumaOffset, pY+lumaOffset, ctx->lumaLineWidth, endian);
            if (attr.format != FORMAT_444) {
                jdi_write_memory(fb->bufY+lumaOffset+ctx->lumaLineWidth, pY+lumaOffset+ctx->lumaLineWidth, ctx->lumaLineWidth, endian);
            }
            pU = pY + ctx->lumaSize;
            if (attr.chromaInterleaved == TRUE) {
                chromaOffset = currentChromaRow * ctx->chromaLineWidth * 2;
                jdi_write_memory(fb->bufCb+chromaOffset, pU+chromaOffset, 2*ctx->chromaLineWidth, endian);
            }
            else {
                chromaOffset = currentChromaRow * ctx->chromaLineWidth;
                jdi_write_memory(fb->bufCb+chromaOffset, pU+chromaOffset, ctx->chromaLineWidth, endian);
                pV = pU + ctx->chromaSize;
                jdi_write_memory(fb->bufCr+chromaOffset, pV+chromaOffset, ctx->chromaLineWidth, endian);
            }
            ctx->currentRow += attr.format == FORMAT_444 ? 1 : 2;
        }
        break;
    default:
        JLOG(ERR, "%s:%d NOT SUPPORTED YUV YUV_FMT:%d\n", __FUNCTION__, __LINE__);
        success = FALSE;
        break;
    }

    return success == FALSE ? -1 : ctx->currentRow;
}

static BOOL IYuvFeeder_Create(
    AbstractYuvFeeder* feeder,
    const char*        path
    )
{
    IYuvContext*    ctx;
    FILE*           fp;

    if ((fp=fopen(path, "rb")) == NULL) {
        JLOG(ERR, "%s:%d failed to open yuv file: %s\n", __FUNCTION__, __LINE__, path);
        return FALSE;
    }

    if ((ctx=(IYuvContext*)malloc(sizeof(IYuvContext))) == NULL) {
        fclose(fp);
        return FALSE;
    }

    memset(ctx, 0, sizeof(IYuvContext));

    ctx->fp = fp;
    CalcYuvFrameSize(&feeder->attr, ctx);
    ctx->pYuv      = malloc(ctx->frameSize);
    feeder->impl->context  = (void*)ctx;

    return TRUE;
}

static BOOL IYuvFeeder_Destroy(
    AbstractYuvFeeder* feeder
    )
{
    IYuvContext*    ctx;

    if (feeder == NULL) {
        return FALSE;
    }
    ctx = (IYuvContext*)feeder->impl->context;
    if (ctx->fp != NULL) {
        fclose(ctx->fp); //lint !e482
    }
    if (ctx->pYuv != NULL) {
        free(ctx->pYuv);
    }
    free(ctx);

    return FALSE;
}

static Int32 IYuvFeeder_Feed(
    AbstractYuvFeeder* feeder,
    FrameBuffer*       fb,
    Uint32             endian
    )
{
    if (feeder->sliceHeight > 0) {
        return LoadSliceFromFile((IYuvContext*)feeder->impl->context, fb, feeder->attr, endian);
    }
    else {
        return LoadFrameFromFile((IYuvContext*)feeder->impl->context, fb, feeder->attr, endian);
    }
}

static BOOL IYuvFeeder_Configure(
    AbstractYuvFeeder* feeder,
    Uint32             cmd,
    void*              arg
    )
{
    UNREFERENCED_PARAMETER(feeder);
    UNREFERENCED_PARAMETER(cmd);
    UNREFERENCED_PARAMETER(arg);

    return FALSE;
}

static YuvFeederImpl IYuvFeederImpl = {
    NULL,
    IYuvFeeder_Create,
    IYuvFeeder_Feed,
    IYuvFeeder_Destroy,
    IYuvFeeder_Configure
};

static Int32 FeedYuv(
    AbstractYuvFeeder*  feeder,
    FrameBuffer*        fb
    )
{
    YuvFeederImpl* impl = feeder->impl;

    return impl->Feed(feeder, fb, feeder->fbEndian);
}

static void NotifyPictureDone(
    AbstractYuvFeeder* absFeeder,
    Int32              lines
    )
{
    YuvFeederListenerArg larg;
    BOOL                 doNotify = FALSE;

    // Notify to the client when errors occur or slice buffer is filled.
    doNotify = (BOOL)(lines <= 0 || (lines%(Int32)absFeeder->sliceHeight) == 0);

    if (doNotify) {
        larg.currentRow = lines<=0 ? absFeeder->attr.height : lines;
        larg.context    = absFeeder->sliceNotiCtx;
        larg.height     = absFeeder->attr.height;
        absFeeder->listener(&larg);
    }

    return;
}

static void YuvFeederThread(
    void* arg
    )
{
    AbstractYuvFeeder* absFeeder = (AbstractYuvFeeder*)arg;
    FrameBuffer* fb;
    Int32        lines;
    BOOL         done = FALSE;

    while (absFeeder->threadStop == FALSE) {
        if ((fb=Queue_Dequeue(absFeeder->fbQueue)) == NULL) {
            MSleep(10);
            continue;
        }

        done = FALSE;
        while (done == FALSE) {
            if ((lines=FeedYuv(absFeeder, fb)) == absFeeder->attr.height) {
                done = TRUE;
            }
            NotifyPictureDone(absFeeder, lines);
            if (lines <= 0)
                break; // Error
        }
    }
}

/*lint -esym(438, ap) */
YuvFeeder YuvFeeder_Create(
    YuvFeederMode         mode,
    const char*           path,
    YuvAttr               attr,
    Uint32                fbEndian,
    YuvFeederListener     listener
    )
{
    AbstractYuvFeeder *feeder;
    YuvFeederImpl     *impl;
    BOOL              success = FALSE;

    if (path == NULL) {
        JLOG(ERR, "%s:%d src path is NULL\n", __FUNCTION__, __LINE__);
        return NULL;
    }

    if ((impl=malloc(sizeof(YuvFeederImpl))) == NULL) {
        return NULL;
    }
    memcpy((void*)impl, (void*)&IYuvFeederImpl, sizeof(YuvFeederImpl));

    if ((feeder=(AbstractYuvFeeder*)calloc(1, sizeof(AbstractYuvFeeder))) == NULL) {
        free(impl);
        return NULL;
    }
    feeder->impl        = impl;
    feeder->thread      = NULL;
    feeder->threadStop  = FALSE;
    feeder->listener    = NULL;
    feeder->fbEndian    = fbEndian;
    feeder->fbQueue     = NULL;
    feeder->attr        = attr;


    success = impl->Create(feeder, path);

    if (success == FALSE)
        return NULL;

    if (mode == YUV_FEEDER_MODE_THREAD) {
        feeder->thread   = (void*)JpuThread_Create((JpuThreadRunner)YuvFeederThread, (void*)feeder);
        feeder->listener = (listener == NULL) ? DefaultListener : listener;
        feeder->fbQueue  = Queue_Create_With_Lock(FB_QUEUE_SIZE, sizeof(FrameBuffer));
    }

    return feeder;
}
/*lint +esym(438, ap) */

BOOL YuvFeeder_Destroy(
    YuvFeeder feeder
    )
{
    YuvFeederImpl*     impl = NULL;
    AbstractYuvFeeder* absFeeder =  (AbstractYuvFeeder*)feeder;

    if (absFeeder == NULL) {
        JLOG(ERR, "%s:%d Invalid handle\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    impl = absFeeder->impl;

    if (absFeeder->thread) {
        absFeeder->threadStop = TRUE;
        JpuThread_Join((JpuThread)absFeeder->thread);
    }

    impl->Destroy(absFeeder);
    free(impl);
    free(feeder);

    return TRUE;
}

BOOL YuvFeeder_Feed(
    YuvFeeder       feeder,
    FrameBuffer*    fb
    )
{
    AbstractYuvFeeder* absFeeder = (AbstractYuvFeeder*)feeder;

    if (absFeeder == NULL) {
        JLOG(ERR, "%s:%d Invalid handle\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    if (absFeeder->thread == NULL) {
        return FeedYuv(absFeeder, fb);
    }
    else {
        return Queue_Enqueue(absFeeder->fbQueue, fb);
    }
}

BOOL YuvFeeder_Configure(
    YuvFeeder       feeder,
    YuvFeederCmd    cmd,
    void*           arg
    )
{
    AbstractYuvFeeder* absFeeder = (AbstractYuvFeeder*)feeder;
    BOOL               success = TRUE;
    SliceNotifyParam*  sliceParam;

    if (absFeeder == NULL) {
        JLOG(ERR, "%s:%d Invalid handle\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    switch (cmd) {
    case YUV_FEEDER_CMD_SET_SLICE_NOTIFY:
        if (absFeeder->thread == NULL) {
            JLOG(ERR, "The YuvFeeder is not thread mode\n");
            break;
        }
        sliceParam = (SliceNotifyParam*)arg;
        absFeeder->sliceHeight  = sliceParam->rows;
        absFeeder->sliceNotiCtx = sliceParam->arg;
        break;
    default:
        success = FALSE;
        break;
    }

    return success;
}

