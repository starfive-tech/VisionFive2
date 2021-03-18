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

#include "vdi_osal.h"
#include "cnm_app.h"
#include "cnm_app_internal.h"

STATIC CNMAppContext appCtx;
Int32 CnmErrorStatus = 0;
void CNMAppInit(void)
{
    osal_memset((void*)&appCtx, 0x00, sizeof(CNMAppContext));

    osal_init_keyboard();
}

BOOL CNMAppAdd(CNMTask task)
{
    CNMTaskContext* taskCtx = (CNMTaskContext*)task;
    if (appCtx.numTasks >= MAX_TASKS_IN_APP) {
        return FALSE;
    }

    taskCtx->oneTimeRun = TRUE;
    appCtx.taskList[appCtx.numTasks++] = task;
    return TRUE;
}

BOOL CNMAppRun(void)
{
    CNMTask             task;
    Uint32              i;
    BOOL                success;
    Uint32              runningTask;

    success     = TRUE;
    runningTask = appCtx.numTasks;
    for (i=0; i<appCtx.numTasks; i++) {
        task=(CNMTask)appCtx.taskList[i];
        if ( FALSE == CNMTaskRun((CNMTask)task))
            return FALSE;
    }

    while (0 < runningTask) {
        for (i=0; i<appCtx.numTasks; i++) {
            if (NULL == (task=(CNMTask)appCtx.taskList[i])) continue;

            if (FALSE == supportThread) {
                if (FALSE == CNMTaskRun((CNMTask)task)) break;
            }

            if (CNM_TASK_RUNNING != CNMTaskWait(task)) {
                success &= CNMTaskStop(task);
                CNMTaskDestroy(task);
                appCtx.taskList[i] = NULL;
                runningTask--;
            }
        }
    }

    osal_memset((void*)&appCtx, 0x00, sizeof(CNMAppContext));

    osal_close_keyboard();

    return (0 == CnmErrorStatus) ? success : FALSE;
}

void CNMAppStop(void)
{
    Uint32 i;

    for (i=0; i<appCtx.numTasks; i++) {
        CNMTaskStop(appCtx.taskList[i]);
    }
}

void CNMErrorSet(Int32 val)
{
   CnmErrorStatus = val;
}

Int32 CNMErrorGet(void)
{
    return CnmErrorStatus;
}

