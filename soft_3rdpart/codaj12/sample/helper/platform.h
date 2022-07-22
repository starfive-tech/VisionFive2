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
#ifndef __JPU_PLATFORM_H__
#define __JPU_PLATFORM_H__

#ifdef USE_FEEDING_METHOD_BUFFER
    #include "codaj12/jpuapi/jputypes.h"
#else
    #include "jputypes.h"
#endif
/************************************************************************/
/* JpuMutex                                                                */
/************************************************************************/
typedef void*   JpuMutex;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
extern void MSleep(Uint32);

extern JpuMutex JpuMutex_Create(
    void
    );

extern void JpuMutex_Destroy(
    JpuMutex   handle
    );

extern BOOL JpuMutex_Lock(
    JpuMutex   handle
    );

extern BOOL JpuMutex_Unlock(
    JpuMutex   handle
    );

Uint32 GetRandom(
    Uint32   start,
    Uint32   end
    );
#ifdef __cplusplus
}
#endif /* __cplusplus */

/************************************************************************/
/* JpuThread                                                               */
/************************************************************************/
typedef void*   JpuThread;
typedef void(*JpuThreadRunner)(void*);

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

extern JpuThread JpuThread_Create(
    JpuThreadRunner    func,
    void*           arg
    );

extern BOOL JpuThread_Join(
    JpuThread thread
    );

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __JPU_PLATFORM_H__ */


