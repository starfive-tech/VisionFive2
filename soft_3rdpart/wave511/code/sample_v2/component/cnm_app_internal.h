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

#ifndef __CNM_APP_INTERNAL_H__
#define __CNM_APP_INTERNAL_H__

#ifdef USE_FEEDING_METHOD_BUFFER
    #include "wave511/vpuapi/vputypes.h"
#else
    #include "vputypes.h"
#endif

#define MAX_TASKS_IN_APP            8
#define MAX_COMPONENTS_IN_TASK      6

extern BOOL supportThread;
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct CNMTaskContext {
    Uint32          numComponents;
    void*           componentList[MAX_COMPONENTS_IN_TASK];
    BOOL            stop;
    BOOL            oneTimeRun;
    BOOL            terminate;
    BOOL            componentsConnected;
} CNMTaskContext;

typedef struct {
    Uint32  numTasks;
    CNMTask taskList[MAX_TASKS_IN_APP];
} CNMAppContext;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __CNM_APP_INTERNAL_H__ */

