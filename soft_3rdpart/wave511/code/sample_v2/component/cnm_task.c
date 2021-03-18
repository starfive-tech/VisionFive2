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

CNMTask CNMTaskCreate(void)
{
    CNMTaskContext* ctx = (CNMTaskContext*)osal_malloc(sizeof(CNMTaskContext));

    if (ctx == NULL) {
        VLOG(ERR, "%s:%d Failed to allocate a memory\n", __FUNCTION__, __LINE__);
        return NULL;
    }

    osal_memset((void*)ctx, 0x00, sizeof(CNMTaskContext));
    ctx->oneTimeRun = supportThread;

    return (CNMTask)ctx;
}

void CNMTaskDestroy(CNMTask task)
{
    CNMTaskContext* ctx = (CNMTaskContext*)task;
    Uint32          i;

    if (task == NULL) {
        VLOG(WARN, "%s:%d HANDLE is NULL\n", __FUNCTION__, __LINE__);
        return;
    }

    for (i=0; i<ctx->numComponents; i++) {
        ComponentRelease(ctx->componentList[i]);
    }

    for (i=0; i<ctx->numComponents; i++) {
        ComponentDestroy(ctx->componentList[i], NULL);
    }

    osal_free(task);

    return;
}

BOOL CNMTaskAdd(CNMTask task, Component component)
{
    CNMTaskContext* ctx = (CNMTaskContext*)task;
    Uint32 num = ctx->numComponents;

    if (ctx->numComponents == MAX_COMPONENTS_IN_TASK) {
        return FALSE;
    }

    ctx->componentList[num++] = (void*)component;
    ctx->numComponents = num;

    return TRUE;
}

BOOL CNMTaskRun(CNMTask task)
{
    CNMTaskContext* ctx = (CNMTaskContext*)task;
    Uint32          i;
    ComponentImpl*  com;
    BOOL            terminate      = FALSE;
    BOOL            stopComponents = FALSE;

    if (ctx->componentsConnected == FALSE) {
        for (i=0; i<ctx->numComponents-1; i++) {
            Component from = (Component)ctx->componentList[i];
            Component to   = (Component)ctx->componentList[i+1];
            if (ComponentSetupTunnel(from, to) == FALSE) {
                return FALSE;
            }
        }
        ctx->componentsConnected = TRUE;
    }

    while (terminate == FALSE) {
        terminate = TRUE;
        for (i=0; i<ctx->numComponents; i++) {
            if ((com=ctx->componentList[i]) == NULL) {
                ctx->stop = TRUE;
                break;
            }
            if (stopComponents == TRUE) {
                /* Failure! stop all components */
                ComponentStop(com);
            }
            else {
                if (com->terminate == FALSE) {
                    if (ComponentExecute(com) == COMPONENT_STATE_TERMINATED) {
                        stopComponents = (com->success == FALSE);
                        terminate     &= TRUE;
                    }
                    else {
                        terminate     &= FALSE;
                    }
                }
            }
        }
        if (ctx->oneTimeRun == TRUE) break;
        osal_msleep(1); // For cross-debugging on non-thread environment.
    }
    ctx->terminate = (terminate || stopComponents);

    return TRUE;
}

CNMTaskWaitState CNMTaskWait(CNMTask task)
{
    Uint32              i;
    Uint32              doneCount = 0;
    Int32               ret = CNM_TASK_ERROR;
    ComponentImpl*      com;
    CNMTaskContext*     ctx = (CNMTaskContext*)task;
    BOOL                stopComponents = FALSE;

    for (i=0; i<ctx->numComponents; i++) {
        if ((com=(Component)ctx->componentList[i]) != NULL) {
            if (stopComponents == TRUE) {
                ComponentStop(com);
                ret = ComponentWait(com);
            }
            else {
                if ((ret=ComponentWait(com)) == 0) {
                    stopComponents = (com->success == FALSE);
                }
            }
        }
        else {
            ret = 0;    // A component might be terminated in the creating step.
        }

        if (ret == 0) doneCount++;
    }

    return (doneCount == ctx->numComponents) ? CNM_TASK_DONE : CNM_TASK_RUNNING;
}

BOOL CNMTaskStop(CNMTask task)
{
    CNMTaskContext* ctx = (CNMTaskContext*)task;
    ComponentImpl*  com;
    Uint32          i;
    BOOL            success;

    if (task == NULL) 
        return TRUE;
    
    for (i=0; i<ctx->numComponents; i++) {
        if (ctx->componentList[i]) ComponentStop(ctx->componentList[i]);
    }

    success = TRUE;
    for (i=0; i<ctx->numComponents; i++) {
        if ((com=(ComponentImpl*)ctx->componentList[i]) != NULL) {
            if (com->success == FALSE) {
                VLOG(WARN, "%s:%d <%s> returns FALSE\n", __FUNCTION__, __LINE__, com->name);
            }
            success &= com->success;
        }
        else success &= FALSE;
    }

    return success;
}

BOOL CNMTaskIsTerminated(CNMTask task)
{
    CNMTaskContext* ctx = (CNMTaskContext*)task;
    
    return ctx->terminate;
}

