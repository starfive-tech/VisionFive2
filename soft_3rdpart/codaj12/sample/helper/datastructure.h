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
#ifndef __DATA_STRUCTURE_H__
#define __DATA_STRUCTURE_H__

#include "jputypes.h"
#include "platform.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
/************************************************************************/
/* Queue                                                                */
/************************************************************************/
typedef struct {
    void*   data;
} QueueData;

typedef struct {
    Uint8*      buffer;
    Uint32      size;
    Uint32      itemSize;
    Uint32      count;
    Uint32      front;
    Uint32      rear;
    JpuMutex    lock;
} Queue;

Queue* Queue_Create(
    Uint32    itemCount,
    Uint32    itemSize
    );

Queue* Queue_Create_With_Lock(
    Uint32    itemCount,
    Uint32    itemSize
    );

void Queue_Destroy(
    Queue*      queue
    );

/**
 * \brief       Enqueue with deep copy
 */
BOOL Queue_Enqueue(
    Queue*      queue,
    void*       data
    );

/**
 * \brief       Caller has responsibility for releasing the returned data
 */
void* Queue_Dequeue(
    Queue*      queue
    );

void Queue_Flush(
    Queue*      queue
    );

void* Queue_Peek(
    Queue*      queue
    );

Uint32 Queue_Get_Cnt(
    Queue*      queue
    );

/**
 * \brief       @dstQ is NULL, it allocates Queue structure and then copy from @srcQ.
 */
Queue* Queue_Copy(
    Queue*  dstQ,
    Queue*  srcQ
    );

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __DATA_STRUCTURE_H__ */


