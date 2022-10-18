// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2022 StarFive Technology Co., Ltd.
 */
#include "sf_thread.h"
#include "SF_OMX_Core.h"

OMX_ERRORTYPE CreateThread(THREAD_HANDLE_TYPE **threadHandle, OMX_PTR function_name, OMX_PTR argument)
{
    FunctionIn();

    int result = 0;
    int detach_ret = 0;
    THREAD_HANDLE_TYPE *thread;
    OMX_ERRORTYPE ret = OMX_ErrorNone;

    thread = malloc(sizeof(THREAD_HANDLE_TYPE));
    memset(thread, 0, sizeof(THREAD_HANDLE_TYPE));

    pthread_attr_init(&thread->attr);
    if (thread->stack_size != 0)
        pthread_attr_setstacksize(&thread->attr, thread->stack_size);

    /* set priority */
    if (thread->schedparam.sched_priority != 0)
        pthread_attr_setschedparam(&thread->attr, &thread->schedparam);

    detach_ret = pthread_attr_setdetachstate(&thread->attr, PTHREAD_CREATE_JOINABLE);
    if (detach_ret != 0)
    {
        free(thread);
        *threadHandle = NULL;
        ret = OMX_ErrorUndefined;
        goto EXIT;
    }

    result = pthread_create(&thread->pthread, &thread->attr, function_name, (void *)argument);
    /* pthread_setschedparam(thread->pthread, SCHED_RR, &thread->schedparam); */

    switch (result)
    {
    case 0:
        *threadHandle = thread;
        ret = OMX_ErrorNone;
        break;
    case EAGAIN:
        free(thread);
        *threadHandle = NULL;
        ret = OMX_ErrorInsufficientResources;
        break;
    default:
        free(thread);
        *threadHandle = NULL;
        ret = OMX_ErrorUndefined;
        break;
    }

EXIT:
    FunctionOut();

    return ret;
}

void ThreadExit(void *value_ptr)
{
    pthread_exit(value_ptr);
    return;
}