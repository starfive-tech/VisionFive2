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

#include "config.h"
#include "main_helper.h"

typedef struct {
    FILE*       fp;
} Context;

BOOL BinComparator_Create(
    ComparatorImpl* impl,
    char*           path
    )
{
    Context*    ctx;
    FILE*       fp;

    if ((fp=osal_fopen(path, "rb")) == NULL) {
        VLOG(ERR, "%s:%d failed to open bin file: %s\n", __FUNCTION__, __LINE__, path);
        return FALSE;
    }

    if ((ctx=(Context*)osal_malloc(sizeof(Context))) == NULL) {
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
    Context*    ctx = (Context*)impl->context;

    osal_fclose(ctx->fp);
    osal_free(ctx);

    return TRUE;
}

BOOL BinComparator_Compare(
    ComparatorImpl* impl,
    void*           data,
    Uint32          size
    )
{
    Uint8*      pBin = NULL;
    Context*    ctx = (Context*)impl->context;
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

        if ( impl->curIndex == 1 )//because of header
            VLOG(ERR, "MISMATCH WITH GOLDEN bin at header\n");
        else
            VLOG(ERR, "MISMATCH WITH GOLDEN bin at %d frame\n", impl->curIndex - 1);

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
 
