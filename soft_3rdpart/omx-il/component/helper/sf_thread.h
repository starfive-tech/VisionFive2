// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2022 StarFive Technology Co., Ltd.
 */
#ifndef SF_THREAD_H
#define SF_THREAD_H

#include <stdint.h>
#include <errno.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include "OMX_Types.h"
#include "OMX_Core.h"

typedef struct _THREAD_HANDLE_TYPE
{
    pthread_t          pthread;
    pthread_attr_t     attr;
    struct sched_param schedparam;
    int                stack_size;
} THREAD_HANDLE_TYPE;


OMX_ERRORTYPE CreateThread(THREAD_HANDLE_TYPE **threadHandle, OMX_PTR function_name, OMX_PTR argument);
void ThreadExit(void *value_ptr);

#endif //SF_THREAD_H