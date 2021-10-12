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

#include <errno.h>
#include "main_helper.h"
#include "../misc/skip.h"

typedef struct {
    osal_file_t     fp;
    Uint32          md5Size;
    Uint32          prevMd5[12];
} Context;


BOOL MD5Comparator_Create(
    ComparatorImpl* impl,
    char*           path
    )
{
    Context*        ctx;
    osal_file_t     fp;
    Uint32        temp;

    if ((fp=osal_fopen(path, "r")) == NULL) {
        VLOG(ERR, "%s:%d failed to open md5 file: %s, errno(%d)\n", __FUNCTION__, __LINE__, path, errno);
        return FALSE;
    }

    if ((ctx=(Context*)osal_malloc(sizeof(Context))) == NULL) {
        osal_fclose(fp);
        return FALSE;
    }

    while (!osal_feof(fp)) {
        if (osal_fscanf((FILE*)fp, "%08x", &temp) < 1) break;
        impl->numOfFrames++;
    }

    osal_fseek(fp, 0, SEEK_SET);

    ctx->fp       = fp;
    ctx->md5Size  = 12;
    impl->context = ctx;
    impl->eof     = FALSE;

    return TRUE;
}

BOOL MD5Comparator_Destroy(
    ComparatorImpl*  impl
    )
{
    Context*    ctx = (Context*)impl->context;

    osal_fclose(ctx->fp);
    osal_free(ctx);

    return TRUE;
}

BOOL MD5Comparator_Compare(
    ComparatorImpl* impl,
    void*           data,
    Uint32          size
    )
{
    Context*    ctx = (Context*)impl->context;
    BOOL        match = TRUE;
    Uint32      md5[12];
    Uint32      index;
    Uint32*     decodedMD5 = (Uint32*)data;

    if (ctx->md5Size != size) {
        VLOG(ERR, "%s:%d different MD5 size\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    do {
        osal_memset((void*)md5, 0x00, sizeof(md5));
        if (impl->usePrevDataOneTime == TRUE) {
            impl->usePrevDataOneTime = FALSE;
            osal_memcpy(md5, ctx->prevMd5, ctx->md5Size*sizeof(md5[0]));
        }
        else {
            for (index=0; index<ctx->md5Size; index++) {
                /* FIXME: osal_osal_fscanf has problem on Windows.
                */
                osal_fscanf((FILE*)ctx->fp, "%08x", &md5[index]);

                if (IsEndOfFile(ctx->fp) == TRUE) {
                    impl->eof = TRUE;
                    break;
                }
            }
        }

        if (data == NULL)
            return FALSE;

        match = TRUE;
        for (index=0; index<size; index++) {
            if (md5[index] != decodedMD5[index]) {
                match = FALSE;
                break;
            }
        }
    } while (impl->enableScanMode == TRUE && match == FALSE && impl->eof == FALSE);

    osal_memcpy(ctx->prevMd5, md5, ctx->md5Size*sizeof(md5[0]));

    if (match == FALSE ) {
        VLOG(ERR, "MISMATCH WITH GOLDEN MD5 at %d frame\n", impl->curIndex);
        VLOG(ERR, "GOLDEN         DECODED\n"
                  "-----------------------\n");
        for (index=0; index<size; index++) 
            VLOG(ERR, "%08x       %08x \n", md5[index], decodedMD5[index]);
    }

    return match;
}

BOOL MD5Comparator_Configure(
    ComparatorImpl*     impl,
    ComparatorConfType  type,
    void*               val
    )
{
    Context*    ctx = (Context*)impl->context;
    BOOL        ret = TRUE;

    switch (type) {
    case COMPARATOR_CONF_SET_GOLDEN_DATA_SIZE:
        ctx->md5Size = *(Uint32*)val;
        impl->numOfFrames /= ctx->md5Size;
        break;
    default:
        ret = FALSE;
        break;
    }

    return ret;
}

BOOL MD5Comparator_Rewind(
    ComparatorImpl*     impl
    )
{
    Context*    ctx = (Context*)impl->context;
    Int32       ret;

    if ((ret=osal_fseek(ctx->fp, 0, SEEK_SET)) != 0) {
        VLOG(ERR, "%s:%d Failed to osal_fseek(ret: %d)\n", __FUNCTION__, __LINE__, ret);
        return FALSE;
    }

    return TRUE;
}

ComparatorImpl md5ComparatorImpl = {
    NULL,
    NULL,
    0,
    0,
    MD5Comparator_Create,
    MD5Comparator_Destroy,
    MD5Comparator_Compare,
    MD5Comparator_Configure,
    MD5Comparator_Rewind,
};
 
