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
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>

#include "vpuapifunc.h"
#include "main_helper.h"

#define MAX_FEEDING_SIZE        0x400000        /* 4MBytes */

typedef struct struct_ReaderConext {
    osal_file_t fp;
    Uint32      feedingSize;
    BOOL        eos;
} ReaderContext;

void* BSFeederSizePlusEs_Create(
    const char* path
    )
{
    osal_file_t     fp = NULL;
    ReaderContext*  context=NULL;

    if ((fp=osal_fopen(path, "rb")) == NULL) {
        VLOG(ERR, "%s:%d failed to open %s\n", __FUNCTION__, __LINE__, path);
        return NULL;
    }

    context = (ReaderContext*)osal_malloc(sizeof(ReaderContext));
    if (context == NULL) {
        VLOG(ERR, "%s:%d failed to allocate memory\n", __FUNCTION__, __LINE__);
        osal_fclose(fp);
        return NULL;
    }

    context->fp          = fp;
    context->feedingSize = 0;
    context->eos         = FALSE;

    return (void*)context;
}

BOOL BSFeederSizePlusEs_Destroy(
    void* feeder
    )
{
    ReaderContext* context = (ReaderContext*)feeder;

    if (context == NULL) {
        VLOG(ERR, "%s:%d Null handle\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    if (context->fp) 
        osal_fclose(context->fp);

    osal_free(context);

    return TRUE;
}

Int32 BSFeederSizePlusEs_Act(
    void*       feeder,
    BSChunk*    chunk
    )
{
    ReaderContext*  context = (ReaderContext*)feeder;
    size_t          nRead;
    Uint32          chunkSize = 0;

    if (context == NULL) {
        VLOG(ERR, "%s:%d Null handle\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    if (context->eos == TRUE) {
        return 0;
    }

    osal_fread(&chunkSize, 1, 4, context->fp);

    nRead = osal_fread(chunk->data, 1, chunkSize, context->fp);
    if (nRead < 0) {
        VLOG(ERR, "%s:%d failed to read bitstream(errno: %d)\n", __FUNCTION__, __LINE__, errno);
        return 0;
    } 
    else if (nRead < chunkSize) {
        context->eos = TRUE;
    }
    chunk->size = chunkSize;

    return nRead;
}

BOOL BSFeederSizePlusEs_Rewind(
    void*       feeder
    )
{
    ReaderContext*  context = (ReaderContext*)feeder;
    Int32           ret;

    if ((ret=osal_fseek(context->fp, 0, SEEK_SET)) != 0) {
        VLOG(ERR, "%s:%d failed osal_fseek(ret:%d)\n", __FUNCTION__, __LINE__, ret);
        return FALSE;
    }

    return TRUE;
}

