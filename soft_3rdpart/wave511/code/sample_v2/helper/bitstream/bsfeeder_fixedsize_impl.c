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
#include <stdlib.h>
#include <time.h>
#include <errno.h>

#include "vpuapifunc.h"
#include "main_helper.h"

#define MAX_FEEDING_SIZE        0x400000        /* 4MBytes */
#define DEFAULT_FEEDING_SIZE    0x20000         /* 128KBytes */

typedef struct FeederFixedContext {
    osal_file_t*    fp;
    Uint32          feedingSize;
    BOOL            eos;
} FeederFixedContext;

void* BSFeederFixedSize_Create(
    const char* path,
    CodStd      codecId
    )
{
    osal_file_t*    fp = NULL;
    FeederFixedContext*  context=NULL;

    UNREFERENCED_PARAMETER(codecId);

    if ((fp=osal_fopen(path, "rb")) == NULL) {
        VLOG(ERR, "%s:%d failed to open %s\n", __FUNCTION__, __LINE__, path);
        return NULL;
    }

    context = (FeederFixedContext*)osal_malloc(sizeof(FeederFixedContext));
    if (context == NULL) {
        VLOG(ERR, "%s:%d failed to allocate memory\n", __FUNCTION__, __LINE__);
        osal_fclose(fp);
        return NULL;
    }

    context->fp          = fp;
    context->feedingSize = DEFAULT_FEEDING_SIZE;
    context->eos         = FALSE;

    return (void*)context;
}

BOOL BSFeederFixedSize_Destroy(
    void* feeder
    )
{
    FeederFixedContext* context = (FeederFixedContext*)feeder;

    if (context == NULL) {
        VLOG(ERR, "%s:%d Null handle\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    if (context->fp) 
        osal_fclose(context->fp);

    osal_free(context);

    return TRUE;
}

Int32 BSFeederFixedSize_Act(
    void*       feeder,
    BSChunk*    chunk
    )
{
    FeederFixedContext*  context = (FeederFixedContext*)feeder;
    size_t          nRead;
    Uint32          size;
    Uint32          feedingSize;

    if (context == NULL) {
        VLOG(ERR, "%s:%d Null handle\n", __FUNCTION__, __LINE__);
        return 0;
    }

    if (context->eos == TRUE) {
        chunk->eos = TRUE;
        return 0;
    }

    feedingSize = context->feedingSize;
    if (feedingSize == 0) {
        Uint32  KB = 1024;
        BOOL    probability10;

        srand((Uint32)time(NULL));
        feedingSize   = rand() % MAX_FEEDING_SIZE;
        probability10 = (BOOL)((feedingSize%100) < 10);
        if (feedingSize < KB) {
            if (probability10 == FALSE) 
                feedingSize *= 100;
        }
    }

    size = (chunk->size < feedingSize) ? chunk->size : feedingSize;

    do {
        nRead = osal_fread(chunk->data, 1, size, context->fp);
        if ((Int32)nRead < 0) {
            VLOG(ERR, "%s:%d failed to read bitstream(errno: %d)\n", __FUNCTION__, __LINE__, errno);
            return 0;
        } 
        else if (nRead < size) {
            context->eos = TRUE;
            chunk->eos   = TRUE;
        }
    } while (FALSE);

    return nRead;
}

BOOL BSFeederFixedSize_Rewind(
    void*       feeder
    )
{
    FeederFixedContext*  context = (FeederFixedContext*)feeder;
    Int32           ret;

    if ((ret=osal_fseek(context->fp, 0, SEEK_SET)) != 0) {
        VLOG(ERR, "%s:%d Failed to fseek(ret: %d)\n", __FUNCTION__, __LINE__, ret);
        return FALSE;
    }
    context->eos = FALSE;

    return TRUE;
}

void BSFeederFixedSize_SetFeedingSize(
    void*   feeder,
    Uint32  feedingSize
    )
{
    FeederFixedContext*  context = (FeederFixedContext*)feeder;
    if (feedingSize > 0) {
        context->feedingSize = feedingSize;
    }
}

