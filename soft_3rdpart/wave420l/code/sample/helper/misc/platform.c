//--=========================================================================--
//  This file is a part of VPU Reference API project
//-----------------------------------------------------------------------------
//
//       This confidential and proprietary software may be used only
//     as authorized by a licensing agreement from Chips&Media Inc.
//     In the event of publication, the following notice is applicable:
//
//            (C) COPYRIGHT CHIPS&MEDIA INC.
//                      ALL RIGHTS RESERVED
//
//       The entire notice above must be reproduced on all authorized
//       copies.
//
//--=========================================================================--

#include <errno.h>
#include <stdlib.h>
#include <time.h>
#include "main_helper.h"

/*
 * Platform dependent functions
 */
#ifdef PLATFORM_WIN32 
#include <windows.h>
VpuThread VpuThread_Create(
    VpuThreadRunner    runner,
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

    return (VpuThread)thread;
}

BOOL VpuThread_Join(
     VpuThread     thread
     )
{
    DWORD ret;
    HANDLE hVpuThread = (HANDLE)thread;

    if (hVpuThread == NULL)
        return FALSE;

    ret = WaitForSingleObject(hVpuThread, INFINITE);
    if (ret == WAIT_FAILED) {
        VLOG(ERR, "%s:%d WaitForSingleObject failed: ERROR=%d\n", __FUNCTION__, __LINE__, GetLastError());
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

VpuMutex VpuMutex_Create(
    void
    )
{
    HANDLE mutexHandle;
    
    if ((mutexHandle=CreateMutex(NULL, FALSE, NULL)) == NULL) {
        VLOG(ERR, "%s:%d Failed to CreateVpuMutex: errno(%d)\n", 
            __FUNCTION__, __LINE__, GetLastError());
    }

    return mutexHandle;
}

void VpuMutex_Destroy(
    VpuMutex   handle
    )
{
    if (handle == NULL) {
        VLOG(ERR, "%s:%d Invalid mutex handle\n", __FUNCTION__, __LINE__);
        return;
    }

    CloseHandle((HANDLE)handle);
}

BOOL VpuMutex_Lock(
    VpuMutex   handle
    )
{
    if (handle == NULL) {
        VLOG(ERR, "%s:%d Invalid mutex handle\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    if (WaitForSingleObject((HANDLE)handle, INFINITE) != WAIT_OBJECT_0) {
        VLOG(ERR, "%s:%d Faield to WaitForSingleObject. err(%d)\n", 
            __FUNCTION__, __LINE__, GetLastError());
        return FALSE;
    }

    return TRUE;
}

BOOL VpuMutex_Unlock(
    VpuMutex   handle
    )
{
    if (handle == NULL) {
        VLOG(ERR, "%s:%d Invalid mutex handle\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    ReleaseMutex((HANDLE)handle);

    return TRUE;
}

#elif defined(PLATFORM_LINUX)
#include <pthread.h>
#include <unistd.h>

typedef void *(*PTHREAD_START_ROUTINE)(void*);

VpuThread VpuThread_Create(
    VpuThreadRunner    runner,
    void*           arg
    )
{
    Int32   ret;
    VpuThread  thread = (pthread_t*)osal_malloc(sizeof(pthread_t));

    if ((ret=pthread_create((pthread_t*)thread, NULL, (PTHREAD_START_ROUTINE)runner, arg)) != 0) {
        osal_free(thread);
        VLOG(ERR, "Failed to pthread_create ret(%d)\n", ret);
        return NULL;
    }

    return thread;
}

BOOL VpuThread_Join(
     VpuThread     thread
     )
{
    Int32       ret;
    pthread_t   pthreadHandle;

    if (thread == NULL) {
        VLOG(ERR, "%s:%d invalid thread handle\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    pthreadHandle = *(pthread_t*)thread;

    if ((ret=pthread_join(pthreadHandle, NULL)) != 0) {
        VLOG(ERR, "%s:%d Failed to pthread_join ret(%d)\n", __FUNCTION__, __LINE__, ret);
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
} VpuMutexHandle;

VpuMutex VpuMutex_Create(
    void
    )
{
    VpuMutexHandle* handle = (VpuMutexHandle*)osal_malloc(sizeof(VpuMutexHandle));

    if (handle == NULL) {
        VLOG(ERR, "%s:%d failed to allocate memory\n", __FUNCTION__, __LINE__);
        return NULL;
    }

    if (pthread_mutex_init(&handle->lock, NULL) < 0) {
        osal_free(handle);
        VLOG(ERR, "%s:%d failed to pthread_mutex_init() errno(%d)\n", 
             __FUNCTION__, __LINE__, errno);
        return NULL;
    }

    return (VpuMutex)handle;
}

void VpuMutex_Destroy(
    VpuMutex   handle
    )
{
    if (handle == NULL) {
        VLOG(ERR, "%s:%d Invalid mutex handle\n", __FUNCTION__, __LINE__);
        return;
    }

    osal_free(handle);
}

BOOL VpuMutex_Lock(
    VpuMutex   handle
    )
{
    VpuMutexHandle* mutex = (VpuMutexHandle*)handle;

    if (mutex == NULL) {
        VLOG(ERR, "%s:%d Invalid mutex handle\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    pthread_mutex_lock(&mutex->lock);

    return TRUE;
}

BOOL VpuMutex_Unlock(
    VpuMutex   handle
    )
{
    VpuMutexHandle* mutex = (VpuMutexHandle*)handle;

    if (mutex == NULL) {
        VLOG(ERR, "%s:%d Invalid mutex handle\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    pthread_mutex_unlock(&mutex->lock);

    return TRUE;
}
#else
VpuThread VpuThread_Create(
    VpuThreadRunner    runner,
    void*           arg
    )
{
    UNREFERENCED_PARAMETER(runner);
    UNREFERENCED_PARAMETER(arg);

    VLOG(WARN, "%s not implemented yet\n", __FUNCTION__);

    return NULL;
}

BOOL VpuThread_Join(
     VpuThread     thread
     )
{
    UNREFERENCED_PARAMETER(thread);

    VLOG(WARN, "%s not implemented yet\n", __FUNCTION__);

    return FALSE;
}

void MSleep(
    Uint32      ms
    )
{
    UNREFERENCED_PARAMETER(ms);
}

VpuMutex VpuMutex_Create(
    void
    )
{
    void* ctx;
    VLOG(WARN, "%s not implemented yet\n", __FUNCTION__);

    ctx = osal_malloc(sizeof(Int32));

    return (VpuMutex)ctx;
}

void VpuMutex_Destroy(
    VpuMutex   handle
    )
{
    UNREFERENCED_PARAMETER(handle);

    VLOG(WARN, "%s not implemented yet\n", __FUNCTION__);

    osal_free((void*)handle);
}

BOOL VpuMutex_Lock(
    VpuMutex   handle
    )
{
    UNREFERENCED_PARAMETER(handle);

    return TRUE;
}

BOOL VpuMutex_Unlock(
    VpuMutex   handle
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
        VLOG(INFO, "======= RANDOM SEED: %08x ======\n", randomSeed);
        srand(randomSeed);
    }

    if (range == 0) {
        VLOG(ERR, "%s:%d RANGE IS 0\n", __FUNCTION__, __LINE__);
        return 0;
    }
    else {
        return ((rand()%range) + start);
    }
}

