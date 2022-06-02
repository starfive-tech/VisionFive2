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

#ifndef __CNM_APP_H__
#define __CNM_APP_H__


#ifdef USE_FEEDING_METHOD_BUFFER
    #include "wave511/vpuapi/vputypes.h"
    #include "wave511/sample_v2/component/component.h"
#else
    #include "vputypes.h"
    #include "component.h"
#endif
typedef void*       CNMTask;

typedef enum {
    CNM_TASK_DONE,
    CNM_TASK_RUNNING,
    CNM_TASK_ERROR
} CNMTaskWaitState;

typedef void (*CNMTaskListener)(CNMTask task, void* context);

typedef struct CNMAppConfig {
    char            fwpath[256];
} CNMAppConfig;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

void    CNMAppInit(void);
BOOL    CNMAppAdd(CNMTask task);
BOOL    CNMAppRun(void);
void    CNMAppStop(void);

CNMTask CNMTaskCreate(void);
void    CNMTaskDestroy(CNMTask task);
BOOL    CNMTaskAdd(CNMTask task, Component component);
BOOL    CNMTaskRun(CNMTask task);
CNMTaskWaitState CNMTaskWait(CNMTask task);
BOOL    CNMTaskStop(CNMTask task);
BOOL    CNMTaskIsTerminated(CNMTask task);

enum {
    CNM_ERROR_NONE,
    CNM_ERROR_FAILURE,
    CNM_ERROR_HANGUP,
};
void CNMErrorSet(Int32 val);
Int32 CNMErrorGet();

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __CNM_APP_H__ */

