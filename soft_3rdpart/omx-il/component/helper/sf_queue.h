// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2022 StarFive Technology Co., Ltd.
 */
#ifndef SF_QUEUE_H
#define SF_QUEUE_H

#include <stdint.h>
#include <errno.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include "OMX_Types.h"
#include "OMX_Core.h"

typedef struct {
    OMX_U8*      buffer;
    OMX_U32      size;
    OMX_U32      itemSize;
    OMX_U32      count;
    OMX_U32      front;
    OMX_U32      rear;
    pthread_mutex_t    lock;
    pthread_cond_t     cond;
} SF_Queue;


SF_Queue* SF_Queue_Create(OMX_U32 itemCount, OMX_U32 itemSize);
void SF_Queue_Destroy(SF_Queue* queue);
OMX_ERRORTYPE SF_Queue_Enqueue(SF_Queue* queue, void* data);
void* SF_Queue_Dequeue(SF_Queue* queue);
void* SF_Queue_Dequeue_Block(SF_Queue* queue);
void SF_Queue_Flush(SF_Queue* queue);
void* SF_Queue_Peek(SF_Queue* queue);
OMX_U32   SF_Queue_Get_Cnt(SF_Queue* queue);
SF_Queue* SF_Queue_Copy(SF_Queue* dstQ, SF_Queue* srcQ);

#endif //SF_QUEUE_H