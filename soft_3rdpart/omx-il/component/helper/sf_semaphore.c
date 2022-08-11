// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2022 StarFive Technology Co., Ltd.
 */
#include "sf_semaphore.h"
#include "SF_OMX_Core.h"

OMX_ERRORTYPE SF_SemaphoreCreate(OMX_HANDLETYPE *semaphoreHandle)
{
    sem_t *sema;

    sema = (sem_t *)malloc(sizeof(sem_t));
    if (!sema)
        return OMX_ErrorInsufficientResources;

    if (sem_init(sema, 0, 0) != 0) {
        free(sema);
        return OMX_ErrorUndefined;
    }

    *semaphoreHandle = (OMX_HANDLETYPE)sema;

    LOG(SF_LOG_INFO,"SF_SemaphoreCreate %p\r\n", sema);
    return OMX_ErrorNone;
}

OMX_ERRORTYPE SF_SemaphoreTerminate(OMX_HANDLETYPE semaphoreHandle)
{
    sem_t *sema = (sem_t *)semaphoreHandle;

    if (sema == NULL)
        return OMX_ErrorBadParameter;

    if (sem_destroy(sema) != 0)
        return OMX_ErrorUndefined;

    free(sema);
    return OMX_ErrorNone;
}

OMX_ERRORTYPE SF_SemaphoreWait(OMX_HANDLETYPE semaphoreHandle)
{
    sem_t *sema = (sem_t *)semaphoreHandle;
    FunctionIn();
    LOG(SF_LOG_INFO,"SF_SemaphoreWait %p\r\n", semaphoreHandle);

    if (sema == NULL)
        return OMX_ErrorBadParameter;

    if (sem_wait(sema) != 0)
        return OMX_ErrorUndefined;

    FunctionOut();

    return OMX_ErrorNone;
}

OMX_ERRORTYPE SF_SemaphorePost(OMX_HANDLETYPE semaphoreHandle)
{
    sem_t *sema = (sem_t *)semaphoreHandle;

    FunctionIn();
    LOG(SF_LOG_INFO,"SF_SemaphorePost %p\r\n", semaphoreHandle);

    if (sema == NULL)
        return OMX_ErrorBadParameter;

    if (sem_post(sema) != 0)
        return OMX_ErrorUndefined;

    FunctionOut();

    return OMX_ErrorNone;
}

OMX_ERRORTYPE SF_Set_SemaphoreCount(OMX_HANDLETYPE semaphoreHandle, OMX_S32 val)
{
    sem_t *sema = (sem_t *)semaphoreHandle;

    if (sema == NULL)
        return OMX_ErrorBadParameter;

    if (sem_init(sema, 0, val) != 0)
        return OMX_ErrorUndefined;

    return OMX_ErrorNone;
}

OMX_ERRORTYPE SF_Get_SemaphoreCount(OMX_HANDLETYPE semaphoreHandle, OMX_S32 *val)
{
    sem_t *sema = (sem_t *)semaphoreHandle;
    int semaVal = 0;

    if (sema == NULL)
        return OMX_ErrorBadParameter;

    if (sem_getvalue(sema, &semaVal) != 0)
        return OMX_ErrorUndefined;

    *val = (OMX_S32)semaVal;

    return OMX_ErrorNone;
}