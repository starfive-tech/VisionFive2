// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2022 StarFive Technology Co., Ltd.
 */
#ifndef SF_SEMAPHORE_H
#define SF_SEMAPHORE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include "OMX_Types.h"
#include "OMX_Core.h"

OMX_ERRORTYPE SF_SemaphoreCreate(OMX_HANDLETYPE *semaphoreHandle);
OMX_ERRORTYPE SF_SemaphoreTerminate(OMX_HANDLETYPE semaphoreHandle);
OMX_ERRORTYPE SF_SemaphoreWait(OMX_HANDLETYPE semaphoreHandle);
OMX_ERRORTYPE SF_SemaphorePost(OMX_HANDLETYPE semaphoreHandle);
OMX_ERRORTYPE SF_Set_SemaphoreCount(OMX_HANDLETYPE semaphoreHandle, OMX_S32 val);
OMX_ERRORTYPE SF_Get_SemaphoreCount(OMX_HANDLETYPE semaphoreHandle, OMX_S32 *val);

#endif //SF_SEMAPHORE_H