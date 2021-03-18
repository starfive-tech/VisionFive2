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

#include "config.h"
#include "main_helper.h"
#if defined(PLATFORM_LINUX) || defined(PLATFORM_QNX)
#include <sys/types.h>
#include <sys/stat.h>
#endif
#ifdef PLATFORM_NON_OS
#ifdef LIB_C_STUB
#include <sys/types.h>
#include <sys/stat.h>
#endif
#endif

typedef struct {
    osal_file_t fp;
    Uint32      width;
    Uint32      height;
    Uint32      frameSize;
    BOOL        cbcrInterleave;
    FrameBufferFormat format;
    char        *path;
    Uint32      isVp9;
    Uint8*      lastYuv;
} YuvCompContext;

static Uint32 Calculate(
    YuvCompContext*    ctx
    )
{
    Uint32      lumaSize   = 0;
    Uint32      chromaSize = 0;
    Uint32      frameSize  = 0;
    Uint32      frames = 0;
    Uint32      width  = ctx->width;
    Uint32      height = ctx->height;
    Uint64      endPos = 0;
    BOOL        cbcrInterleave = ctx->cbcrInterleave;
    FrameBufferFormat format = ctx->format;
#if defined(PLATFORM_LINUX) || defined(PLATFORM_QNX)
    struct   stat  file_info;
#endif
#ifdef PLATFORM_NON_OS
#ifdef LIB_C_STUB
    struct   stat  file_info;
#endif
#endif

    lumaSize = width * height;
    switch (format) {
    case FORMAT_400:
        chromaSize = 0;
        break;
    case FORMAT_YUYV:
    case FORMAT_YVYU:
    case FORMAT_UYVY:
    case FORMAT_VYUY:
        chromaSize = lumaSize;
        break;
    case FORMAT_420:
        chromaSize = lumaSize / 2;
        break;
    case FORMAT_422:
    case FORMAT_224:
        chromaSize = lumaSize;
        break;
    case FORMAT_444:
        chromaSize = lumaSize * 2;
        break;
    case FORMAT_420_P10_16BIT_LSB:
    case FORMAT_420_P10_16BIT_MSB:
        lumaSize  *= 2;
        chromaSize = lumaSize/2;
        break;
    case FORMAT_420_P10_32BIT_LSB:
    case FORMAT_420_P10_32BIT_MSB:
#ifdef DUMP_YUV_WHOLE_DATA
        if (cbcrInterleave) {
            lumaSize   = ((VPU_ALIGN16(width)+11)/12*16) * height;
            chromaSize = VPU_ALIGN16(((VPU_ALIGN16(width/2))*2+11)/12*16) * height / 2;
        }
        else {
            lumaSize   = ((VPU_ALIGN16(width)+11)/12*16) * height;
            chromaSize = VPU_ALIGN16(((VPU_ALIGN16(width/2))+11)/12*16) * height / 2 *2;
        }
#else
        if (cbcrInterleave) {
            lumaSize   = ((width+2)/3*4) * height;
            chromaSize = ((width+2)/3*4) * height / 2;
        }
        else {
            lumaSize   = (width+2)/3*4 * height;
            chromaSize = (width/2+2)/3*4 * height / 2 *2;
        }
#endif
        break;
    case FORMAT_422_P10_16BIT_MSB:
    case FORMAT_422_P10_16BIT_LSB:
        lumaSize  *= 2;
        chromaSize = lumaSize;
        break;
    case FORMAT_422_P10_32BIT_LSB:
    case FORMAT_422_P10_32BIT_MSB:
        {
        Uint32 twice = cbcrInterleave ? 2 : 1;
        if (ctx->isVp9 == TRUE) {
            lumaSize   = VPU_ALIGN32(((width+11)/12)*16);
            chromaSize = (((width/2)+11)*twice/12)*16;
        }
        else {
            width = VPU_ALIGN32(width);
            lumaSize   = ((VPU_ALIGN16(width)+11)/12)*16;
            chromaSize = ((VPU_ALIGN16(width/2)+11)*twice/12)*16;
            if ((chromaSize*2) > lumaSize) {
                lumaSize = chromaSize * 2;
            }
        }
        if (cbcrInterleave == TRUE) {
            lumaSize = MAX(lumaSize, chromaSize);
        }
        }
        break;
    default:
        VLOG(ERR, "%s:%d Invalid format: %d\n", __FILE__, __LINE__, format);
    }
    frameSize = lumaSize + chromaSize;

#ifdef PLATFORM_WIN32
#if (_MSC_VER == 1200)
    osal_fseek(ctx->fp, 0, SEEK_END);
    endPos = ftell(ctx->fp);
    osal_fseek(ctx->fp, 0, SEEK_SET);
#else
    _fseeki64((FILE*)ctx->fp, 0LL, SEEK_END);
    endPos = _ftelli64((FILE*)ctx->fp);
    _fseeki64((FILE*)ctx->fp, 0LL, SEEK_SET);
#endif
#else
    stat( ctx->path, &file_info);
    endPos = file_info.st_size;
#endif

    frames = (Uint32)(endPos / frameSize);

    if (endPos % frameSize) {
        VLOG(ERR, "%s:%d Mismatch - file_size: %llu frameSize: %d\n",
            __FUNCTION__, __LINE__, endPos, frameSize);
    }
    ctx->frameSize  = frameSize;

    return frames;
}

BOOL YUVComparator_Create(
    ComparatorImpl* impl,
    char*           path
    )
{
    YuvCompContext*        ctx;
    osal_file_t*    fp;

    if ((fp=osal_fopen(path, "rb")) == NULL) {
        VLOG(ERR, "%s:%d failed to open yuv file: %s\n", __FUNCTION__, __LINE__, path);
        return FALSE;
    }

    if ((ctx=(YuvCompContext*)osal_malloc(sizeof(YuvCompContext))) == NULL) {
        osal_fclose(fp);
        return FALSE;
    }
    osal_memset((void*)ctx, 0x00, sizeof(YuvCompContext));

    ctx->fp        = fp;
    ctx->path      = path;
    ctx->lastYuv   = NULL;
    impl->context  = ctx;
    impl->eof      = FALSE;

    return TRUE;
}

BOOL YUVComparator_Destroy(
    ComparatorImpl*  impl
    )
{
    YuvCompContext*    ctx = (YuvCompContext*)impl->context;

    osal_fclose(ctx->fp);
    if (ctx->lastYuv) 
        osal_free(ctx->lastYuv);
    osal_free(ctx);

    return TRUE;
}

BOOL YUVComparator_Compare(
    ComparatorImpl* impl,
    void*           data,
    PhysicalAddress size
    )
{
    Uint8*      pYuv = NULL;
    YuvCompContext*    ctx = (YuvCompContext*)impl->context;
    BOOL        match = FALSE;

    if (data == (void *)COMPARATOR_SKIP) {
        int fpos;
        fpos = osal_ftell(ctx->fp);
        osal_fseek(ctx->fp, fpos+size, SEEK_SET);

        if (IsEndOfFile(ctx->fp) == TRUE)
            impl->eof = TRUE;

        return TRUE;
    }

    if (impl->usePrevDataOneTime == TRUE) {
        pYuv = ctx->lastYuv;
        impl->usePrevDataOneTime = FALSE;
    }
    else {
        if (ctx->lastYuv) {
            osal_free(ctx->lastYuv);
            ctx->lastYuv = NULL;
        }

        pYuv = (Uint8*)osal_malloc(size);
        osal_fread(pYuv, 1, size, ctx->fp);
        ctx->lastYuv = pYuv;
    }

    if (IsEndOfFile(ctx->fp) == TRUE)
        impl->eof = TRUE;

    match = (osal_memcmp(data, (void*)pYuv, size) == 0 ? TRUE : FALSE);
    if (match == FALSE) {
        osal_file_t* fpGolden;
        osal_file_t* fpOutput;
        char tmp[200];

        VLOG(ERR, "MISMATCH WITH GOLDEN YUV at %d frame\n", impl->curIndex);
        sprintf(tmp, "./golden.yuv");
        if ((fpGolden=osal_fopen(tmp, "wb")) == NULL) {
            VLOG(ERR, "Faild to create golden.yuv\n");
            osal_free(pYuv);
            return FALSE;
        }
        VLOG(ERR, "Saving... Golden YUV at %s\n", tmp);

        sprintf(tmp, "./decoded.yuv");
        osal_fwrite(pYuv, 1, size, fpGolden);
        osal_fclose(fpGolden);
        if ((fpOutput=osal_fopen(tmp, "wb")) == NULL) {
            VLOG(ERR, "Faild to create golden.yuv\n");
            osal_free(pYuv);
            return FALSE;
        }
        VLOG(ERR, "Saving... decoded YUV at %s\n", tmp);
        osal_fwrite(data, 1, size, fpOutput);
        osal_fclose(fpOutput);
    }

    return match;
}

BOOL YUVComparator_Configure(
    ComparatorImpl*     impl,
    ComparatorConfType  type,
    void*               val
    )
{
    PictureInfo*        yuv = NULL;
    YuvCompContext*            ctx = (YuvCompContext*)impl->context;
    BOOL                ret = TRUE;

    switch (type) {
    case COMPARATOR_CONF_SET_PICINFO:
        yuv = (PictureInfo*)val;
        ctx->width  = yuv->width;
        ctx->height = yuv->height;
        ctx->format = yuv->format;
        ctx->cbcrInterleave = yuv->cbcrInterleave;
        //can not calculate a sequence changed YUV
        impl->numOfFrames   = Calculate(ctx);
        break;
    default:
        ret = FALSE;
        break;
    }
    return ret;
}

BOOL YUVComparator_Rewind(
    ComparatorImpl*     impl
    )
{
    YuvCompContext*    ctx = (YuvCompContext*)impl->context;
    Int32       ret;

    if ((ret=osal_fseek(ctx->fp, 0, SEEK_SET)) != 0) {
        VLOG(ERR, "%s:%d failed to osal_fseek(ret:%d)\n", __FUNCTION__, __LINE__, ret);
        return FALSE;
    }

    impl->eof = FALSE;

    return TRUE;
}

ComparatorImpl yuvComparatorImpl = {
    NULL,
    NULL,
    0,
    0,
    YUVComparator_Create,
    YUVComparator_Destroy,
    YUVComparator_Compare,
    YUVComparator_Configure,
    YUVComparator_Rewind,
    FALSE,
};
 
