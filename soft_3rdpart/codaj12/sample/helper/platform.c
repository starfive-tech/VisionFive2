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
#include <errno.h>
#include <stdlib.h>
#include <time.h>
#include "jpulog.h"
#include "platform.h"
#include "config.h"

/*
 * Platform dependent functions
 */
#ifdef PLATFORM_WIN32
#include <windows.h>
JpuThread JpuThread_Create(
    JpuThreadRunner    runner,
    void*           arg
    )
{
    HANDLE thread;
    DWORD  threadId;

    thread = CreateThread(
        NULL,
        20*1024*1024,
        (LPTHREAD_START_ROUTINE)runner,
        arg,
        0,                              /* Creation Flag */
        &threadId);

    return (JpuThread)thread;
}

BOOL JpuThread_Join(
     JpuThread     thread
     )
{
    DWORD ret;
    HANDLE hJpuThread = (HANDLE)thread;

    if (hJpuThread == NULL)
        return FALSE;

    ret = WaitForSingleObject(hJpuThread, INFINITE);
    if (ret == WAIT_FAILED) {
        JLOG(ERR, "%s:%d WaitForSingleObject failed: ERROR=%d\n", __FUNCTION__, __LINE__, GetLastError());
        return FALSE;
    }
    return TRUE;
}

void MSleep(
    Uint32      ms
    )
{
    Sleep(ms);
}

/*
 * returns current time in ms unit
 */
Uint64 GetCurrentTimeInMs(
    void
    )
{
    return 0;
}
const __int64 DELTA_EPOCH_IN_MICROSECS= 11644473600000000;


int gettimeofday(struct timeval *tv/*in*/, struct timezone *tz/*in*/)
{
  FILETIME ft;
  Uint32 tmpres = 0;
  TIME_ZONE_INFORMATION tz_winapi;
  int rez=0;

   ZeroMemory(&ft,sizeof(ft));
   ZeroMemory(&tz_winapi,sizeof(tz_winapi));

    GetSystemTimeAsFileTime(&ft);

    tmpres = ft.dwHighDateTime;
    tmpres <<= 32;
    tmpres |= ft.dwLowDateTime;

    /*converting file time to unix epoch*/
    tmpres /= 10;  /*convert into microseconds*/
    tmpres -= (Uint32)DELTA_EPOCH_IN_MICROSECS;
    tv->tv_sec = (__int32)(tmpres*0.000001);
    tv->tv_usec =(tmpres%1000000);


    //_tzset(),don't work properly, so we use GetTimeZoneInformation
    rez=GetTimeZoneInformation(&tz_winapi);
    tz->tz_dsttime=(rez==2)?TRUE:FALSE;
    tz->tz_minuteswest = tz_winapi.Bias + ((rez==2)?tz_winapi.DaylightBias:0);

  return 0;
}


JpuMutex JpuMutex_Create(
    void
    )
{
    HANDLE mutexHandle;

    if ((mutexHandle=CreateMutex(NULL, FALSE, NULL)) == NULL) {
        JLOG(ERR, "%s:%d Failed to CreateJpuMutex: errno(%d)\n",
            __FUNCTION__, __LINE__, GetLastError());
    }

    return mutexHandle;
}

void JpuMutex_Destroy(
    JpuMutex   handle
    )
{
    if (handle == NULL) {
        JLOG(ERR, "%s:%d Invalid mutex handle\n", __FUNCTION__, __LINE__);
        return;
    }

    CloseHandle((HANDLE)handle);
}

BOOL JpuMutex_Lock(
    JpuMutex   handle
    )
{
    if (handle == NULL) {
        JLOG(ERR, "%s:%d Invalid mutex handle\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    if (WaitForSingleObject((HANDLE)handle, INFINITE) != WAIT_OBJECT_0) {
        JLOG(ERR, "%s:%d Faield to WaitForSingleObject. err(%d)\n",
            __FUNCTION__, __LINE__, GetLastError());
        return FALSE;
    }

    return TRUE;
}

BOOL JpuMutex_Unlock(
    JpuMutex   handle
    )
{
    if (handle == NULL) {
        JLOG(ERR, "%s:%d Invalid mutex handle\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    ReleaseMutex((HANDLE)handle);

    return TRUE;
}

#elif defined(PLATFORM_LINUX)
#include <pthread.h>
#include <unistd.h>

typedef void *(*PTHREAD_START_ROUTINE)(void*);

JpuThread JpuThread_Create(
    JpuThreadRunner    runner,
    void*           arg
    )
{
    Int32   ret;
    JpuThread  thread = (pthread_t*)malloc(sizeof(pthread_t));

    if ((ret=pthread_create((pthread_t*)thread, NULL, (PTHREAD_START_ROUTINE)runner, arg)) != 0) {
        free(thread);
        JLOG(ERR, "Failed to pthread_create ret(%d)\n", ret);
        return NULL;
    }

    return thread;
}

BOOL JpuThread_Join(
     JpuThread     thread
     )
{
    Int32       ret;
    pthread_t   pthreadHandle;

    if (thread == NULL) {
        JLOG(ERR, "%s:%d invalid thread handle\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    pthreadHandle = *(pthread_t*)thread;

    if ((ret=pthread_join(pthreadHandle, NULL)) != 0) {
        JLOG(ERR, "%s:%d Failed to pthread_join ret(%d)\n", __FUNCTION__, __LINE__, ret);
        return FALSE;
    }

    return TRUE;
}

void MSleep(
    Uint32      ms
    )
{
    usleep(ms*1000);
}

typedef struct {
    pthread_mutex_t lock;
} JpuMutexHandle;

JpuMutex JpuMutex_Create(
    void
    )
{
    JpuMutexHandle* handle = (JpuMutexHandle*)malloc(sizeof(JpuMutexHandle));

    if (handle == NULL) {
        JLOG(ERR, "%s:%d failed to allocate memory\n", __FUNCTION__, __LINE__);
        return NULL;
    }

    if (pthread_mutex_init(&handle->lock, NULL) < 0) {
        free(handle);
        JLOG(ERR, "%s:%d failed to pthread_mutex_init() errno(%d)\n",
             __FUNCTION__, __LINE__, errno);
        return NULL;
    }

    return (JpuMutex)handle;
}

void JpuMutex_Destroy(
    JpuMutex   handle
    )
{
    if (handle == NULL) {
        JLOG(ERR, "%s:%d Invalid mutex handle\n", __FUNCTION__, __LINE__);
        return;
    }

    free(handle);
}

//lint -save -e454
BOOL JpuMutex_Lock(
    JpuMutex   handle
    )
{
    JpuMutexHandle* mutex = (JpuMutexHandle*)handle;

    if (mutex == NULL) {
        JLOG(ERR, "%s:%d Invalid mutex handle\n", __FUNCTION__, __LINE__);
        return FALSE;
    }
    pthread_mutex_lock(&mutex->lock);

    return TRUE;
}
//lint -restore

//lint -save -e455
BOOL JpuMutex_Unlock(
    JpuMutex   handle
    )
{
    JpuMutexHandle* mutex = (JpuMutexHandle*)handle;

    if (mutex == NULL) {
        JLOG(ERR, "%s:%d Invalid mutex handle\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    pthread_mutex_unlock(&mutex->lock);

    return TRUE;
}
//lint -restore
#else
JpuThread JpuThread_Create(
    JpuThreadRunner    runner,
    void*           arg
    )
{
    UNREFERENCED_PARAMETER(runner);
    UNREFERENCED_PARAMETER(arg);

    JLOG(WARN, "%s not implemented yet\n", __FUNCTION__);

    return NULL;
}

BOOL JpuThread_Join(
     JpuThread     thread
     )
{
    UNREFERENCED_PARAMETER(thread);

    JLOG(WARN, "%s not implemented yet\n", __FUNCTION__);

    return FALSE;
}

void MSleep(
    Uint32      ms
    )
{
    UNREFERENCED_PARAMETER(ms);
}

JpuMutex JpuMutex_Create(
    void
    )
{
    void* ctx;
    JLOG(WARN, "%s not implemented yet\n", __FUNCTION__);

    ctx = malloc(sizeof(Int32));

    return (JpuMutex)ctx;
}

void JpuMutex_Destroy(
    JpuMutex   handle
    )
{
    UNREFERENCED_PARAMETER(handle);

    JLOG(WARN, "%s not implemented yet\n", __FUNCTION__);

    free((void*)handle);
}

BOOL JpuMutex_Lock(
    JpuMutex   handle
    )
{
    UNREFERENCED_PARAMETER(handle);

    return TRUE;
}

BOOL JpuMutex_Unlock(
    JpuMutex   handle
    )
{
    UNREFERENCED_PARAMETER(handle);

    return TRUE;
}
#endif /* WINDOWS PLATFORM */

Uint32 randomSeed;
Uint32 GetRandom(
    Uint32   start,
    Uint32   end
    )
{
    Uint32   range = end-start+1;

    if (randomSeed == 0) {
        randomSeed = (Uint32)time(NULL);
        JLOG(INFO, "======= RANDOM SEED: %08x ======\n", randomSeed);
        srand(randomSeed);
    }

    if (range == 0) {
        JLOG(ERR, "%s:%d RANGE IS 0\n", __FUNCTION__, __LINE__);
        return 0;
    }
    else {
        return (((Uint32)rand()%range) + start);
    }
}


