#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>

#include "vpuapifunc.h"
#include "main_helper.h"

#define MAX_FEEDING_SIZE        0x400000        /* 4MBytes */
#define DEFAULT_FEEDING_SIZE    0x20000         /* 128KBytes */

typedef struct FeederFixedContext {
    void*           address;
    Uint32          size;
    Uint32          offset;
    Uint32          feedingSize;
    BOOL            eos;
} FeederFixedContext;

void* BSFeederBuffer_Create(
    const char* path,
    CodStd      codecId
    )
{
    FeederFixedContext*  context=NULL;

    UNREFERENCED_PARAMETER(codecId);

    context = (FeederFixedContext*)osal_malloc(sizeof(FeederFixedContext));
    if (context == NULL) {
        VLOG(ERR, "%s:%d failed to allocate memory\n", __FUNCTION__, __LINE__);
        return NULL;
    }

    context->feedingSize = DEFAULT_FEEDING_SIZE;
    context->eos         = FALSE;
    context->offset = 0;
    return (void*)context;
}

BOOL BSFeederBuffer_Destroy(
    void* feeder
    )
{
    FeederFixedContext* context = (FeederFixedContext*)feeder;

    if (context == NULL) {
        VLOG(ERR, "%s:%d Null handle\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    osal_free(context);

    return TRUE;
}

void BSFeederBuffer_SetData(
    void*       feeder,
    char*       address,
    Uint32       size)
{
    FeederFixedContext*  context = (FeederFixedContext*)feeder;
    context->address = address;
    context->size = size;
}

void BSFeederBuffer_SetEos(void*       feeder)
{
    FeederFixedContext*  context = (FeederFixedContext*)feeder;
    context->eos = TRUE;
}

BOOL BSFeederBuffer_GetEos(void*       feeder)
{
    FeederFixedContext*  context = (FeederFixedContext*)feeder;
    return context->eos;
}

Int32 BSFeederBuffer_Act(
    void*       feeder,
    BSChunk*    chunk
    )
{
    FeederFixedContext*  context = (FeederFixedContext*)feeder;

    if (context == NULL) {
        VLOG(ERR, "%s:%d Null handle\n", __FUNCTION__, __LINE__);
        return 0;
    }
    // Due to memory performance, memset is temporarily commented
    // osal_memset(chunk->data, 0x00, chunk->size);
    if (context->size == 0) {
        chunk->eos = TRUE;
        return 0;
    }

    do {
        osal_memcpy(chunk->data, context->address, context->size);
    } while (FALSE);

    return context->size;
}

BOOL BSFeederBuffer_Rewind(
    void*       feeder
    )
{
    FeederFixedContext*  context = (FeederFixedContext*)feeder;

    context->eos = FALSE;

    return TRUE;
}

void BSFeederBuffer_SetFeedingSize(
    void*   feeder,
    Uint32  feedingSize
    )
{
    FeederFixedContext*  context = (FeederFixedContext*)feeder;
    if (feedingSize > 0) {
        context->feedingSize = feedingSize;
    }
}

