// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2022 StarFive Technology Co., Ltd.
 */
#include "sf_queue.h"
#include "SF_OMX_Core.h"

SF_Queue* SF_Queue_Create(
    OMX_U32      itemCount,
    OMX_U32      itemSize
    )
{
    SF_Queue* queue = NULL;
    OMX_U32 size  = itemCount*itemSize;

    if ((queue=(SF_Queue *)malloc(sizeof(SF_Queue))) == NULL)
        return NULL;
    queue->size   = itemCount;
    queue->itemSize = itemSize;
    queue->count  = 0;
    queue->front  = 0;
    queue->rear   = 0;
    queue->buffer = (OMX_U8*)malloc(size);
    if (pthread_mutex_init(&queue->lock, NULL) < 0) {
        free(queue);
        LOG(SF_LOG_ERR, "failed to pthread_mutex_init() errno(%d)\n", errno);
        return NULL;
    }
    if (pthread_cond_init(&queue->cond, NULL) < 0) {
        free(queue);
        LOG(SF_LOG_ERR, "failed to pthread_cond_init() errno(%d)\n", errno);
        return NULL;
    }

    return queue;
}

void SF_Queue_Destroy(
    SF_Queue*      queue
    )
{
    if (queue == NULL)
        return;

    if (queue->buffer)
        free(queue->buffer);
    free(queue);
}

OMX_ERRORTYPE SF_Queue_Enqueue(
    SF_Queue*      queue,
    void*       data
    )
{
    OMX_U8*      ptr;
    OMX_U32      offset;

    if (queue == NULL) return OMX_ErrorUndefined;

    /* SF_Queue is full */
    if (queue->count == queue->size)
        return OMX_ErrorOverflow;

    pthread_mutex_lock(&queue->lock);
    offset = queue->rear * queue->itemSize;

    ptr = &queue->buffer[offset];
    memcpy(ptr, data, queue->itemSize);
    queue->rear++;
    queue->rear %= queue->size;
    queue->count++;
    pthread_mutex_unlock(&queue->lock);
    pthread_cond_signal(&queue->cond);

    return OMX_ErrorNone;
}

void* SF_Queue_Dequeue(
    SF_Queue*      queue
    )
{
    void* data;
    OMX_U32   offset;

    if (queue == NULL)
        return NULL;
    /* SF_Queue is empty */
    if (queue->count == 0)
        return NULL;
    pthread_mutex_lock(&queue->lock);
    offset = queue->front * queue->itemSize;
    data   = (void*)&queue->buffer[offset];
    queue->front++;
    queue->front %= queue->size;
    queue->count--;
    pthread_mutex_unlock(&queue->lock);
    return data;
}

void* SF_Queue_Dequeue_Block(
    SF_Queue*      queue
    )
{
    void* data;
    OMX_U32   offset;

    if (queue == NULL)
        return NULL;
    pthread_mutex_lock(&queue->lock);
    /* SF_Queue is empty */
    while (queue->count == 0){
        pthread_cond_wait(&queue->cond,&queue->lock);
    }

    offset = queue->front * queue->itemSize;
    data   = (void*)&queue->buffer[offset];
    queue->front++;
    queue->front %= queue->size;
    queue->count--;
    pthread_mutex_unlock(&queue->lock);
    return data;
}

void SF_Queue_Flush(
    SF_Queue*      queue
    )
{
    if (queue == NULL)
        return;
    pthread_mutex_lock(&queue->lock);
    queue->count = 0;
    queue->front = 0;
    queue->rear  = 0;
    pthread_mutex_unlock(&queue->lock);
    return;
}

void* SF_Queue_Peek(
    SF_Queue*      queue
    )
{
    OMX_U32      offset;
    void*       temp;

    if (queue == NULL)
        return NULL;
    /* SF_Queue is empty */
    if (queue->count == 0)
        return NULL;
    pthread_mutex_lock(&queue->lock);
    offset = queue->front * queue->itemSize;
    temp = (void*)&queue->buffer[offset];
    pthread_mutex_unlock(&queue->lock);
    return  temp;
}

OMX_U32   SF_Queue_Get_Cnt(
    SF_Queue*      queue
    )
{
    OMX_U32      cnt;

    if (queue == NULL)
        return 0;
    pthread_mutex_lock(&queue->lock);
    cnt = queue->count;
    pthread_mutex_unlock(&queue->lock);
    return cnt;
}

SF_Queue* SF_Queue_Copy(
    SF_Queue*  dstQ,
    SF_Queue*  srcQ
    )
{
    SF_Queue*   queue = NULL;
    OMX_U32   bufferSize;

    if (dstQ == NULL) {
        if ((queue=(SF_Queue *)malloc(sizeof(SF_Queue))) == NULL)
            return NULL;
        memset((void*)queue, 0x00, sizeof(SF_Queue));
    }
    else {
        queue = dstQ;
    }

    bufferSize      = srcQ->size * srcQ->itemSize;
    queue->size     = srcQ->size;
    queue->itemSize = srcQ->itemSize;
    queue->count    = srcQ->count;
    queue->front    = srcQ->front;
    queue->rear     = srcQ->rear;
    if (queue->buffer) {
        free(queue->buffer);
    }
    queue->buffer   = (OMX_U8*)malloc(bufferSize);

    memcpy(queue->buffer, srcQ->buffer, bufferSize);

    return queue;
}