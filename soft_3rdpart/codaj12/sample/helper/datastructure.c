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

Queue* Queue_Create(
    Uint32      itemCount,
    Uint32      itemSize
    )
{
    Queue* queue = NULL;
    Uint32 size  = itemCount*itemSize;

    if ((queue=(Queue *)malloc(sizeof(Queue))) == NULL)
        return NULL;
    queue->size   = itemCount;
    queue->itemSize = itemSize;
    queue->count  = 0;
    queue->front  = 0;
    queue->rear   = 0;
    queue->buffer = (Uint8*)malloc(size);
    queue->lock = NULL;

    return queue;
}

Queue* Queue_Create_With_Lock(
    Uint32      itemCount,
    Uint32      itemSize
    )
{
    Queue* queue = NULL;
    Uint32 size  = itemCount*itemSize;

    if ((queue=(Queue *)malloc(sizeof(Queue))) == NULL)
        return NULL;
    queue->size   = itemCount;
    queue->itemSize = itemSize;
    queue->count  = 0;
    queue->front  = 0;
    queue->rear   = 0;
    queue->buffer = (Uint8*)malloc(size);
    queue->lock   = JpuMutex_Create();

    return queue;
}

void Queue_Destroy(
    Queue*      queue
    )
{
    if (queue == NULL)
        return;

    if (queue->buffer)
        free(queue->buffer);
    if (queue->lock)
        JpuMutex_Destroy(queue->lock);
    free(queue);
}

BOOL Queue_Enqueue(
    Queue*      queue,
    void*       data
    )
{
    Uint8*      ptr;
    Uint32      offset;

    if (queue == NULL) return FALSE;

    /* Queue is full */
    if (queue->count == queue->size)
        return -1;

    if (queue->lock)
        JpuMutex_Lock(queue->lock);
    offset = queue->rear * queue->itemSize;

    ptr = &queue->buffer[offset];
    memcpy(ptr, data, queue->itemSize);
    queue->rear++;
    queue->rear %= queue->size;
    queue->count++;
    if (queue->lock)
        JpuMutex_Unlock(queue->lock);

    return TRUE;
}

void* Queue_Dequeue(
    Queue*      queue
    )
{
    void* data;
    Uint32   offset;

    if (queue == NULL)
        return NULL;
    /* Queue is empty */
    if (queue->count == 0)
        return NULL;
    if (queue->lock)
        JpuMutex_Lock(queue->lock);
    offset = queue->front * queue->itemSize;
    data   = (void*)&queue->buffer[offset];
    queue->front++;
    queue->front %= queue->size;
    queue->count--;
    if (queue->lock)
        JpuMutex_Unlock(queue->lock);
    return data;
}

void Queue_Flush(
    Queue*      queue
    )
{
    if (queue == NULL)
        return;
    if (queue->lock)
        JpuMutex_Lock(queue->lock);
    queue->count = 0;
    queue->front = 0;
    queue->rear  = 0;
    if (queue->lock)
        JpuMutex_Unlock(queue->lock);
    return;
}

void* Queue_Peek(
    Queue*      queue
    )
{
    Uint32      offset;
    void*       temp;

    if (queue == NULL)
        return NULL;
    /* Queue is empty */
    if (queue->count == 0)
        return NULL;
    if (queue->lock)
        JpuMutex_Lock(queue->lock);
    offset = queue->front * queue->itemSize;
    temp = (void*)&queue->buffer[offset];
    if (queue->lock)
        JpuMutex_Unlock(queue->lock);
    return  temp;
}

Uint32   Queue_Get_Cnt(
    Queue*      queue
    )
{
    Uint32      cnt;

    if (queue == NULL)
        return 0;
    if (queue->lock)
        JpuMutex_Lock(queue->lock);
    cnt = queue->count;
    if (queue->lock)
        JpuMutex_Unlock(queue->lock);
    return cnt;
}

Queue* Queue_Copy(
    Queue*  dstQ,
    Queue*  srcQ
    )
{
    Queue*   queue = NULL;
    Uint32   bufferSize;

    if (dstQ == NULL) {
        if ((queue=(Queue *)malloc(sizeof(Queue))) == NULL)
            return NULL;
        memset((void*)queue, 0x00, sizeof(Queue));
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
    queue->buffer   = (Uint8*)malloc(bufferSize);

    memcpy(queue->buffer, srcQ->buffer, bufferSize);

    return queue;
}

