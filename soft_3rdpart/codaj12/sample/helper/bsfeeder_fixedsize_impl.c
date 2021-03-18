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
#include <time.h>
#include "main_helper.h"

#define DEFAULT_FEEDING_SIZE      0x800000         /* 8 MBytes*/

typedef struct FeederFixedContext {
    FILE*   fp;
    Uint32          feedingSize;
    BOOL    eos;
} FeederFixedContext;

void* BSFeederFixedSize_Create(const char* path)
{
    FILE*                fp = NULL;
    FeederFixedContext*  context=NULL;

    if ((fp=fopen(path, "rb")) == NULL) {
        JLOG(ERR, "%s:%d failed to open %s\n", __FUNCTION__, __LINE__, path);
        return NULL;
    }

    context = (FeederFixedContext*)malloc(sizeof(FeederFixedContext));
    if (context == NULL) {
        JLOG(ERR, "%s:%d failed to allocate memory\n", __FUNCTION__, __LINE__);
        fclose(fp);
        return NULL;
    }

    context->fp          = fp;
    context->feedingSize = DEFAULT_FEEDING_SIZE;
    context->eos         = FALSE;

    return (void*)context;
}

//lint -e482
BOOL BSFeederFixedSize_Destroy(void* feeder)
{
    FeederFixedContext* context = (FeederFixedContext*)feeder;

    if (context == NULL) {
        JLOG(ERR, "%s:%d Null handle\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    if (context->fp) 
        fclose(context->fp);

    free(context);

    return TRUE;
}
//int +e482

Int32 BSFeederFixedSize_Act(void* feeder, BSChunk* chunk)
{
    FeederFixedContext* context = (FeederFixedContext*)feeder;
    Uint32              nRead;
    Uint32              size;
    Uint32          feedingSize;

    if (context == NULL) {
        JLOG(ERR, "%s:%d Null handle\n", __FUNCTION__, __LINE__);
        return 0;
    }

    if (context->eos == TRUE) {
        chunk->eos = TRUE;
        return 0;
    }

    feedingSize = context->feedingSize;
    size = (chunk->size < feedingSize) ? chunk->size : feedingSize;
    do {
        nRead = fread(chunk->data, 1, size, context->fp);
        if ((Int32)nRead < 0) {
            JLOG(ERR, "%s:%d failed to read bitstream\n", __FUNCTION__, __LINE__);
            return 0;
        } 
        else if (nRead < size) {
            context->eos = TRUE;
            chunk->eos   = TRUE;
        }
    } while (FALSE);

    return nRead;
}


