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
#include "main_helper.h"

#ifdef SUPPORT_FFMPEG
extern BitstreamWriterImpl containerWriter;
#endif /* SUPPORT_FFMPEG */

/* --------------------------------------------------------------------------
 * ES writer 
  --------------------------------------------------------------------------- */
typedef struct ESWriterContext {
    FILE*   fp;
} ESWriterContext;

static BOOL ESWriter_Create(BitstreamWriterImpl* impl, EncConfigParam* config, const char* path)
{
    FILE* fp;
    ESWriterContext* ctx = NULL;

    if ((fp=fopen(path, "wb")) == NULL) {
        JLOG(ERR, "<%s:%d> Failed to create file: %s\n", __FUNCTION__, __LINE__, path);
        return FALSE;
    }

    if (NULL == (ctx=(ESWriterContext*)malloc(sizeof(ESWriterContext)))) {
        JLOG(ERR, "<%s:%d> Failed to allocate memory(%d)\n", __FUNCTION__, __LINE__, sizeof(ESWriterContext));
        return FALSE;
    }
    ctx->fp = fp;
    impl->context = ctx;

    return TRUE;
}

static Uint32 ESWriter_Act(BitstreamWriterImpl* impl, Uint8* data, Uint32 size)
{
    ESWriterContext* ctx = (ESWriterContext*)impl->context;
    Uint32           nwrite = 0;

    nwrite = fwrite(data, 1, size, ctx->fp);
    fflush(ctx->fp);

    return nwrite;
}

//lint -e482 To avoid the balanced pair function rule: fopen - fclose
static BOOL ESWriter_Destroy(BitstreamWriterImpl* impl)
{
    ESWriterContext* ctx = (ESWriterContext*)impl->context;

    if (ctx->fp) fclose(ctx->fp);
    free(ctx);

    return TRUE;
}
//lint +e482

static BitstreamWriterImpl esWriter = {
    NULL,
    ESWriter_Create,
    ESWriter_Act,
    ESWriter_Destroy
};

#define DELAYED_BUFFER_SIZE             0x400000        /* 4MB */
typedef struct {
    BitstreamWriterImpl impl;
    Uint8*              delayedBuf;
    Uint32              delayedBufSize;
    Uint32              delayedBufOffset;
} AbstractBitstreamWriter;

BSWriter BitstreamWriter_Create(BSWriterType type, EncConfigParam* config, const char* path)
{
    AbstractBitstreamWriter* writer;
    BitstreamWriterImpl*     impl;

    switch (type) {
    case BSWRITER_ES: 
        impl = &esWriter; 
        break;
    case BSWRITER_CONTAINER:
#ifdef SUPPORT_FFMPEG
        impl = &containerWriter;
        break;
#endif /* SUPPORT_FFMPEG */
    default:
        JLOG(ERR, "<%s:%d> Not supported type: %d\n", __FUNCTION__, __LINE__, (Uint32)type);
        return NULL;
    }

    if ((writer=(AbstractBitstreamWriter*)malloc(sizeof(AbstractBitstreamWriter))) == NULL) {
        JLOG(ERR, "<%s:%d> Failed to allocate memory(%d)\n", __FUNCTION__, __LINE__, sizeof(AbstractBitstreamWriter));
        return NULL;
    }
    memset((void*)writer, 0x00, sizeof(AbstractBitstreamWriter));
    memcpy((void*)&writer->impl, impl, sizeof(BitstreamWriterImpl));

    if (FALSE == writer->impl.Create(&writer->impl, config, path)) {
        free(writer);
        writer = NULL;
    }
    else {
        writer->delayedBuf       = (Uint8*)malloc(DELAYED_BUFFER_SIZE);
        writer->delayedBufSize   = DELAYED_BUFFER_SIZE;
        writer->delayedBufOffset = 0;
    }

    return writer;
}

BOOL BitstreamWriter_Act(BSWriter writer, Uint8* data, Uint32 size, BOOL delayedWrite)
{
    AbstractBitstreamWriter* absWriter = (AbstractBitstreamWriter*)writer;

    if (TRUE == delayedWrite) {
        Uint32 offset = absWriter->delayedBufOffset;
        if ((size+offset) > absWriter->delayedBufSize) {
            JLOG(ERR, "<%s:%d> Delayed Buffer Full: bufferSize:%d, offset: %d, size: %d\n",
                 __FUNCTION__, __LINE__, absWriter->delayedBufSize, absWriter->delayedBufOffset, size);
            return FALSE;
        }
        else {
            Uint8* p = absWriter->delayedBuf + offset;
            memcpy(p, data, size);
            absWriter->delayedBufOffset += size;
            return TRUE;
        }
    }
    else {
        if (0 < absWriter->delayedBufOffset) {
            Uint32 totalSize = absWriter->delayedBufOffset + size;
            Uint8* p = (Uint8*)malloc(totalSize);
            BOOL   success;

            if (p == NULL) {
                JLOG(ERR, "<%s:%d> Failed to allocate memory(%d)\n", __FUNCTION__, __LINE__, totalSize);
                return FALSE;
            }
            memcpy(p, absWriter->delayedBuf, absWriter->delayedBufOffset);
            memcpy(p+absWriter->delayedBufOffset, data, size);
            success = absWriter->impl.Act(&absWriter->impl, p, totalSize);
            free(p);
            absWriter->delayedBufOffset = 0;
            return success;
        }
        else {
            return absWriter->impl.Act(&absWriter->impl, data, size);
        }
    }
}

void BitstreamWriter_Destroy(BSWriter writer)
{
    AbstractBitstreamWriter* absWriter = (AbstractBitstreamWriter*)writer;

    absWriter->impl.Destroy(&absWriter->impl);

    free(absWriter->delayedBuf);

    free(absWriter);
}

