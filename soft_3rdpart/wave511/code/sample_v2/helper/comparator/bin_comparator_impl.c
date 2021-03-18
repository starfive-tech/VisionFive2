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

typedef struct {
    FILE*       fp;
} binCompContext;

BOOL BinComparator_Create(
    ComparatorImpl* impl,
    char*           path
    )
{
    binCompContext*    ctx;
    FILE*       fp;

    if ((fp=osal_fopen(path, "rb")) == NULL) {
        VLOG(ERR, "%s:%d failed to open bin file: %s\n", __FUNCTION__, __LINE__, path);
        return FALSE;
    }

    if ((ctx=(binCompContext*)osal_malloc(sizeof(binCompContext))) == NULL) {
        osal_fclose(fp);
        return FALSE;
    }

    ctx->fp        = fp;
    impl->context  = ctx;
    impl->eof      = FALSE;

    return TRUE;
}

BOOL BinComparator_Destroy(
    ComparatorImpl*  impl
    )
{
    binCompContext*    ctx = (binCompContext*)impl->context;

    osal_fclose(ctx->fp);
    osal_free(ctx);

    return TRUE;
}

BOOL BinComparator_Compare(
    ComparatorImpl* impl,
    void*           data,
    PhysicalAddress size
    )
{
    Uint8*      pBin = NULL;
    binCompContext*    ctx = (binCompContext*)impl->context;
    BOOL        match = FALSE;

    pBin = (Uint8*)osal_malloc(size);
        
    osal_fread(pBin, size, 1, ctx->fp);

    if (IsEndOfFile(ctx->fp) == TRUE)
        impl->eof = TRUE;
    else
        impl->numOfFrames++;

    match = (osal_memcmp(data, (void*)pBin, size) == 0 ? TRUE : FALSE);
    if (match == FALSE) {
        FILE* fpGolden;
        FILE* fpOutput;
        char tmp[200];


        sprintf(tmp, "./golden_%s_%05d.bin", GetBasename(impl->filename), impl->curIndex-1);
        if ((fpGolden=osal_fopen(tmp, "wb")) == NULL) {
            VLOG(ERR, "Faild to create %s\n", tmp);
            osal_free(pBin);
            return FALSE;
        }
        VLOG(ERR, "Saving... Golden Bin at %s\n", tmp);
        osal_fwrite(pBin, size, 1, fpGolden);
        osal_fclose(fpGolden);

        sprintf(tmp, "./encoded_%s_%05d.bin", GetBasename(impl->filename), impl->curIndex-1);
        if ((fpOutput=osal_fopen(tmp, "wb")) == NULL) {
            VLOG(ERR, "Faild to create %s\n", tmp);
            osal_free(pBin);
            return FALSE;
        }
        VLOG(ERR, "Saving... encoded Bin at %s\n", tmp);
        osal_fwrite(data, size, 1, fpOutput);
        osal_fclose(fpOutput);
    }

    osal_free(pBin);

    return match;
}

BOOL BinComparator_Configure(
    ComparatorImpl*     impl,
    ComparatorConfType  type,
    void*               val
    )
{
    UNREFERENCED_PARAMETER(impl);
    UNREFERENCED_PARAMETER(type);
    UNREFERENCED_PARAMETER(val);
    return FALSE;
}

ComparatorImpl binComparatorImpl = {
    NULL,
    NULL,
    0,
    0,
    BinComparator_Create,
    BinComparator_Destroy,
    BinComparator_Compare,
    BinComparator_Configure,
    FALSE,
};
 
