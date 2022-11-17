/**
  ******************************************************************************
  * @file  stf_isp_ctrl.c
  * @author  StarFive Isp Team
  * @version  V1.0
  * @date  02/07/2022
  * @brief
  ******************************************************************************
  * @copy
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STARFIVE SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2019 Shanghai StarFive</center></h2>
  */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <getopt.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <malloc.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <asm/types.h>
#include <jpeglib.h>
#include <libv4l2.h>
#include <signal.h>
#include <stdint.h>
#include <inttypes.h>
#include <linux/fb.h>
#include <stdbool.h>
#include <pthread.h>
#include <sys/stat.h>

#include <linux/media.h>
#include <linux/types.h>
#include <linux/v4l2-mediabus.h>
#include <linux/v4l2-subdev.h>
#include <linux/videodev2.h>

#include "mediactl.h"

#include "stf_include.h"
#include "ci/ci_api_structs.h"
#include "ci/ci_api.h"

#include "ISPC/stflib_isp_cfg.h"
#include "ISPC/stflib_isp_base.h"
#include "ISPC/stflib_isp_device.h"
#include "ISPC/stflib_isp_pipeline.h"
#include "ISPC/stflib_isp_shot.h"
#include "ISPC/stflib_isp_sensor.h"
//#include "ISPC/stflib_isp_camera.h"
//#include "ISPC/stflib_mod_config.h"

#include "Controls/stfctl_isp_ae.h"
#include "Controls/stfctl_isp_awb.h"

#include "Modules/stfmod_isp_awb.h"
#include "Modules/stfmod_isp_sc.h"
#if defined(SUPPORT_DUMP_MODULE)
#include "Modules/stfmod_isp_dump.h"
#endif //#if defined(SUPPORT_DUMP_MODULE)
#include "Modules/stfmod_isp_gmargb.h"
#include "Modules/stfmod_isp_yhist.h"
#if defined(ONE_OUT_MODULE)
#include "Modules/stfmod_isp_out.h"
#else
#include "Modules/stfmod_isp_out.h"
#include "Modules/stfmod_isp_out_uo.h"
#include "Modules/stfmod_isp_out_ss0.h"
#include "Modules/stfmod_isp_out_ss1.h"
#endif //#if defined(ONE_OUT_MODULE
#include "Modules/stfmod_isp_til.h"
#include "Modules/stfmod_isp_til_1_rd.h"
#include "Modules/stfmod_isp_til_1_wr.h"
#if defined(SUPPORT_VIC_2_0)
#include "Modules/stfmod_isp_til_2_rd.h"
#include "Modules/stfmod_isp_til_2_wr.h"
#endif //#if defined(SUPPORT_VIC_2_0)

#define LOG_TAG "STF_ISP_CTRL"
#include "stf_common/userlog.h"

#include "stf_isp_ctrl.h"


#if 0
#define THREAD_LOG_ERROR LOG_ERROR
#define THREAD_LOG_WARNING LOG_WARNING
#define THREAD_LOG_INFO LOG_INFO
#define THREAD_LOG_DEBUG LOG_DEBUG
#else
#define THREAD_LOG_ERROR(strings, arg...)
#define THREAD_LOG_WARNING(strings, arg...)
#define THREAD_LOG_INFO(strings, arg...)
#define THREAD_LOG_DEBUG(strings, arg...)
#endif
#if 0
#define V_LOG_ERROR LOG_ERROR
#define V_LOG_WARNING LOG_WARNING
#define V_LOG_INFO LOG_INFO
//#define V_LOG_DEBUG LOG_DEBUG
#define V_LOG_DEBUG LOG_INFO
#else
#define V_LOG_ERROR(strings, arg...)
#define V_LOG_WARNING(strings, arg...)
#define V_LOG_INFO(strings, arg...)
#define V_LOG_DEBUG(strings, arg...)
#endif

#define STF_ISP_CTRL_CFG_DEF        ("stf_isp_ctrl.cfg")
#define STF_ISP_CTRL_SENSOR_DEF     ("imx219mipi")
#define STF_ISP_CTRL_SENSOR_IF_DEF  (EN_SNSR_IF_CSI_0)
#define STF_ISP_CTRL_SENSOR_ISP_DEF (EN_ISP_DEV_IDX_0)


//-------------------------------------
#define TEST_START_ISP_DEMO_KEY     's'
#define TEST_STOP_ISP_DEMO_KEY      'x'
#define TEST_HELP_KEY               '?'
#define EXIT_KEY                    '.'


typedef enum _EN_ISP_MAIN_STEP {
    EN_ISP_MAIN_STEP_NONE = 0,
    EN_ISP_MAIN_STEP_BUFFER_INIT,
    EN_ISP_MAIN_STEP_PREPARE_STOP,
    EN_ISP_MAIN_STEP_STOP,
    EN_ISP_MAIN_STEP_PREPARE_CAPTURE,
    EN_ISP_MAIN_STEP_CAPTURE,
    EN_ISP_MAIN_STEP_MAX
} EN_ISP_MAIN_STEP, *PEN_ISP_MAIN_STEP;


typedef struct _VDEV_DET_THREAD_DATA {
    STF_BOOL8 bExit;
    STF_U32 u32SourceCount;
    pthread_mutex_t lock;
} VDEV_DET_THREAD_DATA, *PVDEV_DET_THREAD_DATA;

typedef struct _ISP_MAIN_THREAD_DATA {
    STF_BOOL8 bExit;
    STF_BOOL8 bStart;
    STF_BOOL8 bCapture;
    EN_ISP_MAIN_STEP enIspMainStep;
    pthread_mutex_t lock;
} ISP_MAIN_THREAD_DATA, *PISP_MAIN_THREAD_DATA;


/* ISP Demo variables */
//-------------------------------------------------------------------------
#if 0
static VDEV_DET_THREAD_DATA g_stVDevDetThread[EN_ISP_DEV_IDX_MAX] = {
    { .bExit = STF_FALSE, .lock = PTHREAD_MUTEX_INITIALIZER },
#if defined(CONFIG_STF_DUAL_ISP)
    { .bExit = STF_FALSE, .lock = PTHREAD_MUTEX_INITIALIZER }
#endif //#if defined(CONFIG_STF_DUAL_ISP)
};
#else
static VDEV_DET_THREAD_DATA g_stVDevDetThread[EN_ISP_DEV_IDX_MAX];
#endif //#if 0
static pthread_t g_VideoDetectThreadId[EN_ISP_DEV_IDX_MAX] = {
    -1,
#if defined(CONFIG_STF_DUAL_ISP)
    -1
#endif //#if defined(CONFIG_STF_DUAL_ISP)
};
#if 0
static ISP_MAIN_THREAD_DATA g_stIspMainThreadData = {
    .bExit = STF_FALSE,
    .bCapture = STF_FALSE,
    .enIspMainStep = EN_ISP_MAIN_STEP_NONE,
    .lock = PTHREAD_MUTEX_INITIALIZER
};
#else
static ISP_MAIN_THREAD_DATA g_stIspMainThreadData;
#endif //#if 0
//-------------------------------------------------------------------------
#if defined(WAITING_FOR_LATER_TURN_ON)
static CI_CONNECTION *g_pstCiConnection = NULL;
static ST_SENSOR g_stSensor;
static ST_PIPELINE g_stPipeline;
static ST_CD_SYSTEM g_stCdSystem;
//-------------------------------------------------------------------------
static ST_SHOT_INFO *g_pstShotInfo = NULL;
//-------------------------------------------------------------------------
static STF_CHAR g_szIspMainStep[][64] = {
    "EN_ISP_MAIN_STEP_NONE",
    "EN_ISP_MAIN_STEP_BUFFER_INIT",
    "EN_ISP_MAIN_STEP_PREPARE_STOP",
    "EN_ISP_MAIN_STEP_STOP",
    "EN_ISP_MAIN_STEP_PREPARE_CAPTURE",
    "EN_ISP_MAIN_STEP_CAPTURE",
    "EN_ISP_MAIN_STEP_MAX"
};
#endif //#if defined(WAITING_FOR_LATER_TURN_ON)
//-------------------------------------------------------------------------


/*
 * other functions
 */

//-----------------------------------------------------------------------------
void StopAllThread(int sig_id)
{
    STF_INT nIdx;

    THREAD_LOG_INFO("\nReceived the ctrl-c command!\n");
    for (nIdx = EN_ISP_DEV_IDX_0; EN_ISP_DEV_IDX_MAX > nIdx; nIdx++) {
        if (-1 != g_VideoDetectThreadId[nIdx]) {
            STFAPI_VDEV_StopDetectThread(nIdx);
        }
    }
    pthread_mutex_lock(&(g_stIspMainThreadData.lock));
    g_stIspMainThreadData.bExit = STF_TRUE;
    pthread_mutex_unlock(&(g_stIspMainThreadData.lock));
}

STF_RESULT InstallSIGINTHandler(
    STF_VOID
    )
{
    struct sigaction stSigAction;

    STF_MEMSET(&stSigAction, 0x00, sizeof(struct sigaction));
    stSigAction.sa_handler = StopAllThread;
    if (sigaction(SIGINT, &stSigAction, 0) != 0) {
        LOG_ERROR("Could not install SIGINT handler! Error: %d, %s\n",
            errno, strerror(errno));
        return STF_ERROR_FATAL;
    }

    return STF_SUCCESS;
}

static STF_VOID *DetectVideoDeviceSourceChange(
    STF_VOID *pvArg
    )
{
    ST_CI_DEVICE stIspDev;
    struct v4l2_event stEvent;
    struct timeval stTimeVal = { 2, 0 };
    fd_set stReadSet;
    fd_set stWriteSet;
    fd_set stExceptSet;
    STF_INT *pnIspIdx;
    STF_INT nIspIdx;
    STF_BOOL8 bExit = STF_FALSE;
    STF_RESULT Ret;
    STF_RESULT *pRet = NULL;

    pRet = STF_MALLOC(sizeof(STF_RESULT));
    *pRet = STF_SUCCESS;

    // Get the ISP device number from the argument.
    pnIspIdx = (STF_INT *)pvArg;
    nIspIdx = *pnIspIdx;
    STF_FREE(pvArg);
    if ((EN_ISP_DEV_IDX_0 > nIspIdx)
#if defined(CONFIG_STF_DUAL_ISP)
        || (EN_ISP_DEV_IDX_1 < nIspIdx)
#endif //#if defined(CONFIG_STF_DUAL_ISP)
        ) {
        LOG_ERROR("Out of support ISP index, nIspIdx = %d\n", nIspIdx);
#if 0
        pthread_exit(0);
#else
        *pRet = STF_ERROR_INVALID_PARAMETERS;
        pthread_exit((STF_VOID *)pRet);
#endif //#if 0
    }

    // Open the ISP device.
    THREAD_LOG_INFO("Open the ISP device %d(\'%s\')\n",
        nIspIdx, g_szDeviceName[g_enIspDevId[nIspIdx]]);
    if (STF_SUCCESS != (Ret = STFLIB_ISP_DEVICE_StructInitialize(&stIspDev,
        g_enIspDevId[nIspIdx], STF_FALSE, EN_MEM_TYPE_NONE, 0, 0))) {
        LOG_ERROR("Cannot open '%s': %d, %s\n",
            g_szDeviceName[g_enIspDevId[nIspIdx]],
            errno, strerror(errno));
#if 0
        pthread_exit(0);
#else
        *pRet = STF_ERROR_FAILURE;
        pthread_exit((STF_VOID *)pRet);
#endif //#if 0
    }

    // Subscribe the event.
    THREAD_LOG_INFO("The ISP device \'%s\' detect thread %d "\
        "subscribe the event!\n", g_szDeviceName[g_enIspDevId[nIspIdx]],
        nIspIdx);
    if (STF_SUCCESS != (Ret = stIspDev.SubscribeEvent(&stIspDev, -1))) {
        LOG_ERROR("Call VIDIOC_SUBSCRIBE_EVENT failed, Ret = %d\n", Ret);
        if (STF_SUCCESS != (Ret = STFLIB_ISP_DEVICE_StructUninitialize(
            &stIspDev))) {
            LOG_ERROR("Cannot close '%s': %d, %s\n",
                g_szDeviceName[g_enIspDevId[nIspIdx]],
                errno, strerror(errno));
        }
#if 0
        pthread_exit(0);
#else
        *pRet = STF_ERROR_FAILURE;
        pthread_exit((STF_VOID *)pRet);
#endif //#if 0
    }

    while (STF_FALSE == bExit) {
        stTimeVal.tv_sec = 2;
        FD_ZERO(&stReadSet);
        FD_SET(stIspDev.GetFileHandle(&stIspDev), &stReadSet);
        FD_ZERO(&stWriteSet);
        FD_SET(stIspDev.GetFileHandle(&stIspDev), &stWriteSet);
        FD_ZERO(&stExceptSet);
        FD_SET(stIspDev.GetFileHandle(&stIspDev), &stExceptSet);
        Ret = select(stIspDev.GetFileHandle(&stIspDev) + 1,
            &stReadSet, &stWriteSet, &stExceptSet, &stTimeVal);
        if (0 > Ret) {
            LOG_ERROR("Get event failed!\n");
        } else if (0 == Ret) {
            //THREAD_LOG_INFO("Get event timeout!\n");
        } else {
            //THREAD_LOG_INFO("Get event!\n");
            while (STF_SUCCESS == (Ret = stIspDev.DequeueEvent(
                &stIspDev,
                &stEvent
                ))) {
                THREAD_LOG_INFO("DQEvent type = %d\n", stEvent.type);
                if (V4L2_EVENT_SOURCE_CHANGE == stEvent.type) {
                    break;
                }
            }
            if ((STF_SUCCESS == Ret)
                && (V4L2_EVENT_SOURCE_CHANGE == stEvent.type)) {
                pthread_mutex_lock(&(g_stVDevDetThread[nIspIdx].lock));
                g_stVDevDetThread[nIspIdx].u32SourceCount =
                        stEvent.u.src_change.changes;
                pthread_mutex_unlock(&(g_stVDevDetThread[nIspIdx].lock));
#if 0
                THREAD_LOG_INFO("DQEvent type = %d, changes = %d\n",
                    stEvent.type, stEvent.u.src_change.changes);
#else
                LOG_INFO("DQEvent type = %d, changes = %d\n",
                    stEvent.type, stEvent.u.src_change.changes);
#endif //#if 0
            }
        }
        pthread_mutex_lock(&(g_stVDevDetThread[nIspIdx].lock));
        bExit = g_stVDevDetThread[nIspIdx].bExit;
        pthread_mutex_unlock(&(g_stVDevDetThread[nIspIdx].lock));
        if (STF_TRUE == bExit) {
            THREAD_LOG_INFO("Received the thread exit command!\n");
        }
    }

    THREAD_LOG_INFO("Prepare close the ISP device \'%s\' detect thread %d!\n",
        g_szDeviceName[g_enIspDevId[nIspIdx]], nIspIdx);

    // Un-subscribe the event.
    THREAD_LOG_INFO("The ISP device \'%s\' detect thread %d "\
        "un-subscribe the event!\n", g_szDeviceName[g_enIspDevId[nIspIdx]],
        nIspIdx);
    if (STF_SUCCESS != (Ret = stIspDev.UnsubscribeEvent(&stIspDev, -1))) {
        LOG_ERROR("Call VIDIOC_UNSUBSCRIBE_EVENT failed, Ret = %d\n", Ret);
        *pRet = STF_ERROR_FAILURE;
    }

    THREAD_LOG_INFO("Close the ISP device \'%s\'\n",
        g_szDeviceName[g_enIspDevId[nIspIdx]]);
    if (STF_SUCCESS != (Ret = STFLIB_ISP_DEVICE_StructUninitialize(
        &stIspDev))) {
        LOG_ERROR("Cannot close '%s': %d, %s\n",
            g_szDeviceName[g_enIspDevId[nIspIdx]],
            errno, strerror(errno));
        *pRet = STF_ERROR_FAILURE;
    }

#if 0
    pthread_exit(0);
#else
    pthread_exit((STF_VOID *)pRet);
#endif //#if 0
}

STF_INT STFAPI_VDEV_StartDetectThread(
    EN_ISP_DEV_IDX enIspIdx
    )
{
    pthread_attr_t stThreadAttr;
    STF_INT nIspIdx = enIspIdx;
    STF_INT nSysRet = STF_SUCCESS;

    THREAD_LOG_INFO("<---\n");
    THREAD_LOG_INFO("Trying to start video device detect thread %d\n", nIspIdx);
    if ((EN_ISP_DEV_IDX_0 > nIspIdx)
#if defined(CONFIG_STF_DUAL_ISP)
        || (EN_ISP_DEV_IDX_1 < nIspIdx)
#endif //#if defined(CONFIG_STF_DUAL_ISP)
        ) {
        LOG_ERROR("Out of support ISP index, nIspIdx = %d\n", nIspIdx);
        THREAD_LOG_INFO("--->\n");
        return -EINVAL;
    }

    if (-1 == g_VideoDetectThreadId[nIspIdx]) {
        STF_INT *pnIspIndex = STF_MALLOC(sizeof(STF_INT));

        g_stVDevDetThread[nIspIdx].bExit = STF_FALSE;
        g_stVDevDetThread[nIspIdx].u32SourceCount = 0;
        pthread_mutex_init(&(g_stVDevDetThread[nIspIdx].lock), NULL);
        pthread_attr_init(&stThreadAttr);

        *pnIspIndex = nIspIdx;
        nSysRet = pthread_create(
            &g_VideoDetectThreadId[nIspIdx],
            &stThreadAttr,
            DetectVideoDeviceSourceChange,
            pnIspIndex
            );
        if (nSysRet) {
            LOG_ERROR("pthread_create is failed, nSysRet = %d(%s)\n",
                nSysRet,
                strerror(nSysRet)
                );
        } else {
            THREAD_LOG_INFO("The video device \'%s\' detect thread is start, "\
                "ThreadId = %d(0x%08X)\n",
                g_szDeviceName[g_enIspDevId[nIspIdx]],
                g_VideoDetectThreadId[nIspIdx],
                g_VideoDetectThreadId[nIspIdx]
                );
        }

        pthread_attr_destroy(&stThreadAttr);
    } else {
        THREAD_LOG_INFO("The video device \'%s\' detect thread "\
            "has been started already! ThreadId = %d(0x%08X)\n",
            g_VideoDetectThreadId[nIspIdx],
            g_VideoDetectThreadId[nIspIdx]
            );
    }
    THREAD_LOG_INFO("--->\n");

    return nSysRet;
}

STF_INT STFAPI_VDEV_StopDetectThread(
    EN_ISP_DEV_IDX enIspIdx
    )
{
    STF_INT nIspIdx = enIspIdx;
    STF_INT nSysRet = STF_SUCCESS;
    STF_VOID *pvRet = NULL;

    THREAD_LOG_INFO("<---\n");
    if ((EN_ISP_DEV_IDX_0 > nIspIdx)
#if defined(CONFIG_STF_DUAL_ISP)
        || (EN_ISP_DEV_IDX_1 < nIspIdx)
#endif //#if defined(CONFIG_STF_DUAL_ISP)
        ) {
        LOG_ERROR("Out of support ISP index, nIspIdx = %d\n", nIspIdx);
        THREAD_LOG_INFO("--->\n");
        return -EINVAL;
    }

    if (-1 == g_VideoDetectThreadId[nIspIdx]) {
        LOG_ERROR("The video device \'%s\' detect thread has not been "\
            "started! ThreadId = %d(0x%08X)\n",
            g_szDeviceName[g_enIspDevId[nIspIdx]],
            g_VideoDetectThreadId[nIspIdx],
            g_VideoDetectThreadId[nIspIdx]
            );
    } else {
        THREAD_LOG_INFO("The video device \'%s\' detect thread will be stop, "\
            "ThreadId = %d(0x%08X)\n",
            g_szDeviceName[g_enIspDevId[nIspIdx]],
            g_VideoDetectThreadId[nIspIdx],
            g_VideoDetectThreadId[nIspIdx]
            );
        THREAD_LOG_INFO("Issue a exit flag to the video device \'%s\' "\
            "detect thread %d!\n", g_szDeviceName[g_enIspDevId[nIspIdx]],
            nIspIdx);
        pthread_mutex_lock(&(g_stVDevDetThread[nIspIdx].lock));
        g_stVDevDetThread[nIspIdx].bExit = STF_TRUE;
        pthread_mutex_unlock(&(g_stVDevDetThread[nIspIdx].lock));

#if 0
        THREAD_LOG_INFO("Sleep 5 micro-second!\n");
        usleep(5);
#endif //#if 0
        THREAD_LOG_INFO("Waiting video device \'%s\' detect thread %d exit!\n",
            g_szDeviceName[g_enIspDevId[nIspIdx]], nIspIdx);
        nSysRet = pthread_join(g_VideoDetectThreadId[nIspIdx], &pvRet);
        THREAD_LOG_INFO("Call pthread_join() function back!\n");
        if (nSysRet) {
            LOG_ERROR("pthread_join is failed, nSysRet = %d(%s)\n",
                nSysRet,
                strerror(nSysRet)
                );
        }
        if (pvRet) {
            THREAD_LOG_INFO("The video device \'%s\' detect thread %d is stop,"\
                " pRet = %d(%s)\n",
                g_szDeviceName[g_enIspDevId[nIspIdx]],
                nIspIdx,
                *(STF_RESULT *)pvRet,
                STF_STR_ERROR(*(STF_RESULT *)pvRet)
                );
            STF_FREE(pvRet);
        } else {
            LOG_ERROR("The video device \'%s\' detect thread %d is stop, "\
                "but the ret is NULL pointer!\n",
                g_szDeviceName[g_enIspDevId[nIspIdx]], nIspIdx);
        }
        THREAD_LOG_INFO("Destory the mutex!\n");
        pthread_mutex_destroy(&(g_stVDevDetThread[nIspIdx].lock));
        g_VideoDetectThreadId[nIspIdx] = -1;
    }
    THREAD_LOG_INFO("--->\n");

    return nSysRet;
}

STF_RESULT WriteFile(
    STF_CHAR *pszFilename,
    STF_VOID *pvBuffer,
    STF_U32 u32Size
    )
{
    FILE *fpOutput;

    if (NULL == (fpOutput = fopen(pszFilename, "w+"))) {
        LOG_ERROR("can't open \'%s\' file!!!\n", pszFilename);
        return STF_ERROR_FATAL;
    }

    fwrite(pvBuffer, u32Size, 1, fpOutput);
    fclose(fpOutput);

    return STF_SUCCESS ;
}

#if defined(WAITING_FOR_LATER_TURN_ON)
static STF_RESULT LoadTestDataIntoMemBuf(
    STF_CHAR *pszFilename,
    CI_MEM_PARAM *pstMemParam
    )
{
    STF_INT iFileId;
    struct stat stStat;
    STF_U32 u32FileSize;
    STF_S32 s32Size;
    STF_RESULT ret = STF_SUCCESS;

    if ((!pszFilename) || (!pstMemParam) || (!pstMemParam->pBuffer)) {
        LOG_ERROR("The pszFilename,  pstMemParam or pstMemParam->pBuffer is NULL!\n");
        return STF_ERROR_INVALID_PARAMETERS;
    }

    if (0 > (iFileId = open(pszFilename, O_RDONLY, 0666))) {
        LOG_ERROR("Failed to open \"%s\" image file!\n",
            pszFilename
            );
        return STF_ERROR_FAILURE;
    }

    stat(pszFilename, &stStat);
    u32FileSize = stStat.st_size;

    if (u32FileSize > pstMemParam->u32BufSize) {
        LOG_ERROR("The \"%s\" file size is large than the \"%s\n memory buffer size!\n",
            pszFilename,
            pstMemParam->stSysMem.szMemName
            );
        return STF_ERROR_FAILURE;
    }

    if (0 > (s32Size = read(iFileId, pstMemParam->pBuffer, u32FileSize))) {
        LOG_ERROR("Failed to load the \"%s\" file into the \"%s\n memory buffer!\n",
            pszFilename,
            pstMemParam->stSysMem.szMemName
            );
        close(iFileId);
        return STF_ERROR_FAILURE;
    }

    close(iFileId);

    return ret;
}

//-----------------------------------------------------------------------------
static STF_RESULT ResetIsp(
    STF_U8 u8IspIdx
    )
{
    ST_ISP_MODULES_EN_PARAM stIspModulesEnableParam;
    STF_RESULT Ret = STF_SUCCESS;

    DEBUG("Disable all of modules (include CSI and ISP).\n");
    //=========================================================================
    stIspModulesEnableParam.stModulesEnableParam.u64Modules =
        (EN_KRN_MOD_ALL | EN_KRN_MOD_CSI | EN_KRN_MOD_ISP);
    stIspModulesEnableParam.stModulesEnableParam.bEnable = STF_FALSE;
    stIspModulesEnableParam.u8IspIdx = u8IspIdx;
    if (STF_SUCCESS != (Ret = STFLIB_ISP_SetModulesEnable(
        &stIspModulesEnableParam
        ))) {
        LOG_ERROR("Failed to disable all of modules (include CSI and ISP).\n");
        return STF_ERROR_FATAL;
    }
    //=========================================================================

    DEBUG("Clear all of interrupt.\n");
    //=========================================================================
    if (STF_SUCCESS != (Ret = STFDRV_ISP_BASE_SetInterruptClear(
#if defined(VIRTUAL_IO_MAPPING)
        g_pstCiConnection,
#endif //VIRTUAL_IO_MAPPING
        u8IspIdx,
        EN_INT_ALL
        ))) {
        LOG_ERROR("Failed to disable all of modules (include CSI and ISP).\n");
        return STF_ERROR_FATAL;
    }

    DEBUG("Reset the ISP.\n");
    //=========================================================================
    if (STF_SUCCESS != (Ret = STFDRV_ISP_BASE_IspReset(
#if defined(VIRTUAL_IO_MAPPING)
        g_pstCiConnection,
#endif //VIRTUAL_IO_MAPPING
        u8IspIdx
        ))) {
        LOG_ERROR("Failed to reset the ISP %d.\n", u8IspIdx);
        return STF_ERROR_FATAL;
    }

    return Ret;
}

#endif //#if defined(WAITING_FOR_LATER_TURN_ON)
static STF_RESULT EnableIspModulesAndControls(
    ST_PIPELINE *pstPipeline
    )
{
    STF_RESULT Ret = STF_SUCCESS;

    if (!pstPipeline) {
        LOG_ERROR("Invalid pstPipeline parameter.\n");
        return STF_ERROR_INVALID_ARG;
    }

    V_LOG_INFO("Enable modules.\n");
    //=========================================================================
#if defined(ENABLE_COLOR_BAR)
    if (STF_SUCCESS != (Ret = pstPipeline->ModuleEnable(
        pstPipeline,
        EN_MODULE_ID_CBAR,
        STF_TRUE
        ))) {
        LOG_WARNING("Failed to enable the module \"CBAR\"!\n");
    }
#endif //#if defined(ENABLE_COLOR_BAR)
#if !defined(DISABLE_CSI_MODULES)
  #if defined(V4L2_DRIVER)
  #else
    if (STF_SUCCESS != (Ret = pstPipeline->ModuleEnable(
        pstPipeline,
        EN_MODULE_ID_DEC,
        STF_TRUE
        ))) {
        LOG_WARNING("Failed to enable the module \"DEC\"!\n");
    }
  #endif //#if defined(V4L2_DRIVER)
    if (STF_SUCCESS != (Ret = pstPipeline->ModuleEnable(
        pstPipeline,
        EN_MODULE_ID_OBC,
        STF_TRUE
        ))) {
        LOG_WARNING("Failed to enable the module \"OBC\"!\n");
    }
  #if defined(SUPPORT_LCBQ_MODULE) && !defined(ENABLE_COLOR_BAR)
    #if defined(V4L2_DRIVER)
    #else
    if (STF_SUCCESS != (Ret = pstPipeline->ModuleEnable(
        pstPipeline,
        EN_MODULE_ID_LCBQ,
        STF_TRUE
        ))) {
        LOG_WARNING("Failed to enable the module \"LCBQ\"!\n");
    }
    #endif //#if defined(V4L2_DRIVER)
  #endif //#if defined(SUPPORT_LCBQ_MODULE) && !defined(ENABLE_COLOR_BAR)
    if (STF_SUCCESS != (Ret = pstPipeline->ModuleEnable(
        pstPipeline,
        EN_MODULE_ID_OECF,
        STF_TRUE
        ))) {
        LOG_WARNING("Failed to enable the module \"OECF\"!\n");
    }
  #if defined(SUPPORT_OECFHM_MODULE) && !defined(ENABLE_COLOR_BAR)
    #if defined(V4L2_DRIVER)
    #else
    if (STF_SUCCESS != (Ret = pstPipeline->ModuleEnable(
        pstPipeline,
        EN_MODULE_ID_OECFHM,
        STF_TRUE
        ))) {
        LOG_WARNING("Failed to enable the module \"OECFHM\"!\n");
    }
    #endif //#if defined(V4L2_DRIVER)
  #endif //#if defined(SUPPORT_OECFHM_MODULE) && !defined(ENABLE_COLOR_BAR)
    if (STF_SUCCESS != (Ret = pstPipeline->ModuleEnable(
        pstPipeline,
        EN_MODULE_ID_LCCF,
        STF_TRUE
        ))) {
        LOG_WARNING("Failed to enable the module \"LCCF\"!\n");
    }
    if (STF_SUCCESS != (Ret = pstPipeline->ModuleEnable(
        pstPipeline,
        EN_MODULE_ID_AWB,
        STF_TRUE
        ))) {
        LOG_WARNING("Failed to enable the module \"AWB\"!\n");
    }
#endif //#if !defined(DISABLE_CSI_MODULES)
    if (STF_SUCCESS != (Ret = pstPipeline->ModuleEnable(
        pstPipeline,
        EN_MODULE_ID_DBC,
        STF_TRUE
        ))) {
        LOG_WARNING("Failed to enable the module \"DBC\"!\n");
    }
    if (STF_SUCCESS != (Ret = pstPipeline->ModuleEnable(
        pstPipeline,
        EN_MODULE_ID_CTC,
        STF_TRUE
        ))) {
        LOG_WARNING("Failed to enable the module \"CTC\"!\n");
    }
    if (STF_SUCCESS != (Ret = pstPipeline->ModuleEnable(
        pstPipeline,
        EN_MODULE_ID_CFA,
        STF_TRUE
        ))) {
        LOG_WARNING("Failed to enable the module \"CFA\"!\n");
    }
    if (STF_SUCCESS != (Ret = pstPipeline->ModuleEnable(
        pstPipeline,
        EN_MODULE_ID_CAR,
        STF_TRUE
        ))) {
        LOG_WARNING("Failed to enable the module \"CAR\"!\n");
    }
    if (STF_SUCCESS != (Ret = pstPipeline->ModuleEnable(
        pstPipeline,
        EN_MODULE_ID_CCM,
        STF_TRUE
        ))) {
        LOG_WARNING("Failed to enable the module \"CCM\"!\n");
    }
    if (STF_SUCCESS != (Ret = pstPipeline->ModuleEnable(
        pstPipeline,
        EN_MODULE_ID_GMARGB,
        STF_TRUE
        ))) {
        LOG_WARNING("Failed to enable the module \"GMARGB\"!\n");
    }
    if (STF_SUCCESS != (Ret = pstPipeline->ModuleEnable(
        pstPipeline,
        EN_MODULE_ID_R2Y,
        STF_TRUE
        ))) {
        LOG_WARNING("Failed to enable the module \"R2Y\"!\n");
    }
    if (STF_SUCCESS != (Ret = pstPipeline->ModuleEnable(
        pstPipeline,
        EN_MODULE_ID_YCRV,
        STF_TRUE
        ))) {
        LOG_WARNING("Failed to enable the module \"YCRV\"!\n");
    }
    if (STF_SUCCESS != (Ret = pstPipeline->ModuleEnable(
        pstPipeline,
        EN_MODULE_ID_SHRP,
        STF_TRUE
        ))) {
        LOG_WARNING("Failed to enable the module \"SHRP\"!\n");
    }
    if (STF_SUCCESS != (Ret = pstPipeline->ModuleEnable(
        pstPipeline,
        EN_MODULE_ID_DNYUV,
        STF_TRUE
        ))) {
        LOG_WARNING("Failed to enable the module \"DNYUV\"!\n");
    }
    if (STF_SUCCESS != (Ret = pstPipeline->ModuleEnable(
        pstPipeline,
        EN_MODULE_ID_SAT,
        STF_TRUE
        ))) {
        LOG_WARNING("Failed to enable the module \"SAT\"!\n");
    }
    if (STF_SUCCESS != (Ret = pstPipeline->ModuleEnable(
        pstPipeline,
        EN_MODULE_ID_SC,
        STF_TRUE
        ))) {
        LOG_WARNING("Failed to enable the module \"SC\"!\n");
    }
    if (STF_SUCCESS != (Ret = pstPipeline->ModuleEnable(
        pstPipeline,
        EN_MODULE_ID_YHIST,
        STF_TRUE
        ))) {
        LOG_WARNING("Failed to enable the module \"YHIST\"!\n");
    }
#if defined(ONE_OUT_MODULE)
    if (STF_SUCCESS != (Ret = STFMOD_ISP_OUT_EnableUo(
        &pstPipeline->stIspCtx,
        STF_TRUE
        ))) {
        LOG_WARNING("Failed to enable the module \"OUT_UO\"!\n");
    }
  #if 0
    if (STF_SUCCESS != (Ret = STFMOD_ISP_OUT_EnableSs0(
        &pstPipeline->stIspCtx,
        STF_TRUE
        ))) {
        LOG_WARNING("Failed to enable the module \"OUT_SS0\"!\n");
    }
    if (STF_SUCCESS != (Ret = STFMOD_ISP_OUT_EnableSs1(
        &pstPipeline->stIspCtx,
        STF_TRUE
        ))) {
        LOG_WARNING("Failed to enable the module \"OUT_SS1\"!\n");
    }
  #endif
#else
    if (STF_SUCCESS != (Ret = pstPipeline->ModuleEnable(
        pstPipeline,
        EN_MODULE_ID_OUT_UO,
        STF_TRUE
        ))) {
        LOG_WARNING("Failed to enable the module \"OUT_UO\"!\n");
    }
  #if 0
    if (STF_SUCCESS != (Ret = pstPipeline->ModuleEnable(
        pstPipeline,
        EN_MODULE_ID_OUT_SS0,
        STF_TRUE
        ))) {
        LOG_WARNING("Failed to enable the module \"OUT_SS0\"!\n");
    }
    if (STF_SUCCESS != (Ret = pstPipeline->ModuleEnable(
        pstPipeline,
        EN_MODULE_ID_OUT_SS1,
        STF_TRUE
        ))) {
        LOG_WARNING("Failed to enable the module \"OUT_SS1\"!\n");
    }
  #endif
#endif //#if defined(ONE_OUT_MODULE)
#if defined(SUPPORT_DUMP_MODULE)
    if (STF_SUCCESS != (Ret = pstPipeline->ModuleEnable(
        pstPipeline,
        EN_MODULE_ID_DUMP,
        STF_TRUE
        ))) {
        LOG_WARNING("Failed to enable the module \"DUMP\"!\n");
    }
#endif //#if defined(SUPPORT_DUMP_MODULE)
#if defined(ENABLE_TIL_1_RD)
  #if defined(V4L2_DRIVER)
  #else
    if (STF_SUCCESS != (Ret = pstPipeline->ModuleEnable(
        pstPipeline,
        EN_MODULE_ID_TIL_1_RD,
        STF_TRUE
        ))) {
        LOG_WARNING("Failed to enable the module \"TIL_1_RD\"!\n");
    }
  #endif //#if defined(V4L2_DRIVER)
#endif //#if defined(ENABLE_TIL_1_RD)
#if defined(ENABLE_TIL_1_WR)
  #if defined(V4L2_DRIVER)
  #else
    if (STF_SUCCESS != (Ret = pstPipeline->ModuleEnable(
        pstPipeline,
        EN_MODULE_ID_TIL_1_WR,
        STF_TRUE
        ))) {
        LOG_WARNING("Failed to enable the module \"TIL_1_WR\"!\n");
    }
  #endif //#if defined(V4L2_DRIVER)
#endif //#if defined(ENABLE_TIL_1_WR)
#if defined(SUPPORT_VIC_2_0)
  #if defined(ENABLE_TIL_2_RD)
    if (STF_SUCCESS != (Ret = pstPipeline->ModuleEnable(
        pstPipeline,
        EN_MODULE_ID_TIL_2_RD,
        STF_TRUE
        ))) {
        LOG_WARNING("Failed to enable the module \"TIL_2_RD\"!\n");
    }
  #endif //#if defined(ENABLE_TIL_2_RD)
  #if defined(ENABLE_TIL_2_WR)
    if (STF_SUCCESS != (Ret = pstPipeline->ModuleEnable(
        pstPipeline,
        EN_MODULE_ID_TIL_2_WR,
        STF_TRUE
        ))) {
        LOG_WARNING("Failed to enable the module \"TIL_2_WR\"!\n");
    }
  #endif //#if defined(ENABLE_TIL_2_WR)
#endif //#if defined(SUPPORT_VIC_2_0)

    V_LOG_INFO("Enable controls.\n");
    //=========================================================================
#if defined(ENABLE_COLOR_BAR) || defined(DISABLE_CSI_MODULES) || defined(ENABLE_TIL_1_RD) || defined(ENABLE_TIL_2_RD)
#else
    if (STF_SUCCESS != (Ret = pstPipeline->ControlEnable(
        pstPipeline,
        EN_CONTROL_ID_AE,
        STF_TRUE
        ))) {
        LOG_WARNING("Failed to enable the control \"AE\"!\n");
    }
    if (STF_SUCCESS != (Ret = pstPipeline->ControlEnable(
        pstPipeline,
        EN_CONTROL_ID_AWB,
        STF_TRUE
        ))) {
        LOG_WARNING("Failed to enable the control \"AWB\"!\n");
    }
    if (STF_SUCCESS != (Ret = pstPipeline->ControlEnable(
        pstPipeline,
        EN_CONTROL_ID_LCCF,
        STF_TRUE
        ))) {
        LOG_WARNING("Failed to enable the control \"LCCF\"!\n");
    }
#endif //#if defined(ENABLE_COLOR_BAR) || defined(DISABLE_CSI_MODULES) || defined(ENABLE_TIL_1_RD) || defined(ENABLE_TIL_2_RD)
#if defined(ENABLE_COLOR_BAR) || defined(ENABLE_TIL_1_RD) || defined(ENABLE_TIL_2_RD)
#else
    if (STF_SUCCESS != (Ret = pstPipeline->ControlEnable(
        pstPipeline,
        EN_CONTROL_ID_CCM,
        STF_TRUE
        ))) {
        LOG_WARNING("Failed to enable the control \"CCM\"!\n");
    }
    if (STF_SUCCESS != (Ret = pstPipeline->ControlEnable(
        pstPipeline,
        EN_CONTROL_ID_YCRV,
        STF_TRUE
        ))) {
        LOG_WARNING("Failed to enable the control \"YCRV\"!\n");
    }
    if (STF_SUCCESS != (Ret = pstPipeline->ControlEnable(
        pstPipeline,
        EN_CONTROL_ID_SHRP,
        STF_TRUE
        ))) {
        LOG_WARNING("Failed to enable the control \"SHRP\"!\n");
    }
    if (STF_SUCCESS != (Ret = pstPipeline->ControlEnable(
        pstPipeline,
        EN_CONTROL_ID_DNYUV,
        STF_TRUE
        ))) {
        LOG_WARNING("Failed to enable the control \"DNYUV\"!\n");
    }
    if (STF_SUCCESS != (Ret = pstPipeline->ControlEnable(
        pstPipeline,
        EN_CONTROL_ID_SAT,
        STF_TRUE
        ))) {
        LOG_WARNING("Failed to enable the control \"SAT\"!\n");
    }
#endif //#if defined(ENABLE_COLOR_BAR) || defined(ENABLE_TIL_1_RD) || defined(ENABLE_TIL_2_RD)

    return STF_SUCCESS;
}

static STF_RESULT RegisterEnableModulesAndControlsToPipeline(
    ST_PIPELINE *pstPipeline
    )
{
    STF_RESULT Ret = STF_SUCCESS;

    if (!pstPipeline) {
        LOG_ERROR("Invalid pstPipeline parameter.\n");
        return STF_ERROR_INVALID_ARG;
    }

    V_LOG_INFO("Unregister enable modules and controls from pipeline.\n");
    //=========================================================================
    if (STF_SUCCESS != (Ret = pstPipeline->UnregisterAllOfModulesAndControls(
        pstPipeline
        ))) {
        LOG_WARNING("Failed to Unregister enable modules and controls!\n");
    }

    V_LOG_INFO("Register enable modules and controls to pipeline.\n");
    //=========================================================================
    if (STF_SUCCESS != (Ret = pstPipeline->RegisterEnabledModulesAndControls(
        pstPipeline
        ))) {
        LOG_WARNING("Failed to register enable modules and controls!\n");
    }

    V_LOG_INFO("Pipeline setup RDMA chain.\n");
    //=========================================================================
    if (STF_SUCCESS != (Ret = pstPipeline->SetNextRdma(
        pstPipeline
        ))) {
        LOG_WARNING("Failed to setup the RDMA chain!\n");
    }

    return STF_SUCCESS;
}

#if defined(WAITING_FOR_LATER_TURN_ON)
static STF_RESULT CdSystemtInitial(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pstCiConnection,
#endif //VIRTUAL_IO_MAPPING
    ST_CD_SYSTEM *pstCdSystem,
    ST_SENSOR *pstSensor,
    STF_U8 u8IspIdx,
    STF_U8 u8SensorMode
    )
{
    STF_RESULT Ret = STF_SUCCESS;

#if defined(VIRTUAL_IO_MAPPING)
    if (!pstCiConnection) {
        LOG_ERROR("Invalid pstCiConnection parameter.\n");
        return STF_ERROR_INVALID_ARG;
    }

#endif //VIRTUAL_IO_MAPPING
    if (!pstCdSystem) {
        LOG_ERROR("Invalid pstCdSystem parameter.\n");
        return STF_ERROR_INVALID_ARG;
    }

    if (!pstSensor) {
        LOG_ERROR("Invalid pstSensor parameter.\n");
        return STF_ERROR_INVALID_ARG;
    }

    if (STF_SUCCESS != (Ret = STFLIB_ISP_CD_SYSTEM_StructInitialize(
        pstCdSystem,
#if defined(VIRTUAL_IO_MAPPING)
        pstCiConnection,
#endif //VIRTUAL_IO_MAPPING
        pstSensor,
        u8SensorMode,
        u8IspIdx
        ))) {
        LOG_ERROR("Failed to initialize ST_CD_SYSTEM structure and "\
            "initialize the cd_system!!!\n");
        return STF_ERROR_FAILURE;
    }
    //-------------------------------------------------------------------------
    if (STF_SUCCESS != (Ret = pstCdSystem->CamObjCreate(
        pstCdSystem,
        u8IspIdx
        ))) {
        LOG_ERROR("Failed to create the camera object!!!\n");
        return STF_ERROR_FAILURE;
    }
    //-------------------------------------------------------------------------
    if (STF_SUCCESS != (Ret = pstCdSystem->CamObjSetPower(
        pstCdSystem,
        STF_TRUE
        ))) {
        LOG_ERROR("Failed to setup the camera object power on!!!\n");
        return STF_ERROR_FAILURE;
    }
    //-------------------------------------------------------------------------
#if !defined(SENSOR_OV4689_MIPI)
    if (STF_SUCCESS != (Ret = pstCdSystem->CamObjStartI2cFastMode(
        pstCdSystem,
        EN_I2C_SPEED_400K
        ))) {
        LOG_ERROR("Failed to start I2C fast mode for the camera object!!!\n");
        return STF_ERROR_FAILURE;
    }
    //-------------------------------------------------------------------------
#endif //#if !defined(SENSOR_OV4689_MIPI)
    if (STF_SUCCESS != (Ret = pstCdSystem->ChnObjCreate(
        pstCdSystem,
        u8IspIdx
        ))) {
        LOG_ERROR("Failed to create the channel object!!!\n");
        return STF_ERROR_FAILURE;
    }
    //-------------------------------------------------------------------------
    if (STF_SUCCESS != (Ret = pstCdSystem->ChnObjSetVideoSource(
        pstCdSystem,
        u8IspIdx
        ))) {
        LOG_ERROR("Failed to setup the video source to "\
            "the channel object!!!\n");
        return STF_ERROR_FAILURE;
    }
    //-------------------------------------------------------------------------

    return STF_SUCCESS;
}

#endif //#if defined(WAITING_FOR_LATER_TURN_ON)
static STF_VOID PrintUsage(
    STF_VOID
    )
{

    printf("\nRuntime key controls:\n");
    printf("\t=== Demo Program ===\n");
    printf("------------------------------\n");
    printf("%c Start the ISP capture demo\n", TEST_START_ISP_DEMO_KEY);
    printf("%c Stop the ISP capture demo\n", TEST_STOP_ISP_DEMO_KEY);
    printf("%c Print the key controls menu\n", TEST_HELP_KEY);
    printf("%c Exit\n", EXIT_KEY);

    printf("\n");
}

static STF_VOID EnableRawMode(
    struct termios *pstOriginalTerminalAttribute
    )
{
    struct termios stNewTerminalAttribute;

    LOG_DEBUG("Enable Console RAW mode\n");

    /* set the terminal to raw mode */
    tcgetattr(fileno(stdin), pstOriginalTerminalAttribute);
    STF_MEMCPY(
        &stNewTerminalAttribute,
        pstOriginalTerminalAttribute,
        sizeof(struct termios)
        );
    stNewTerminalAttribute.c_lflag &= ~(ECHO|ICANON);
    stNewTerminalAttribute.c_cc[VTIME] = 0;
    stNewTerminalAttribute.c_cc[VMIN] = 0;

    tcsetattr(fileno(stdin), TCSANOW, &stNewTerminalAttribute);
}

static STF_VOID DisableRawMode(
    struct termios *pstOriginalTerminalAttribute
    )
{

    LOG_DEBUG("Disable Console RAW mode\n");

    tcsetattr(fileno(stdin), TCSANOW, pstOriginalTerminalAttribute);
}

static STF_CHAR GetChar(
    STF_VOID
    )
{
    static char line[2];

    if (read(0, line, 1)) {
        return line[0];
    }

    return -1;
}

static STF_VOID PrintOptions(
    FILE* fp,
    int argc,
    char** argv
    )
{

#if defined(CONFIG_STF_DUAL_ISP)
    fprintf(fp,
        "Usage: %s [options]\n\n"
        "Options:\n"
        "-h | --help          Print this message\n"
        "-m | --sensor0       Sensor 0 name\n"
        "                         sc2235dvp\n"
        "                         ov4689mipi\n"
        "                         imx219mipi\n"
        "                         default is imx219mipi\n"
        "-n | --sensor1       Sensor 1 name\n"
        "                         sc2235dvp\n"
        "                         ov4689mipi\n"
        "                         imx219mipi\n"
        "-i | --dvp           Set the DVP interface source\n"
        "                         0: Sensor 0\n"
        "                         1: Sensor 1\n"
        "-j | --csi0          Set the CSI 0 interface source\n"
        "                         0: Sensor 0\n"
        "                         1: Sensor 1\n"
        "                         default is 0\n"
        "-k | --csi1          Set the CSI 1 interface source\n"
        "                         0: Sensor 0\n"
        "                         1: Sensor 1\n"
        "-a | --isp0          Set the ISP 0 source\n"
        "                         0: DVP\n"
        "                         1: CSI 0\n"
        "                         2: CSI 1\n"
        "                         default is 1\n"
        "-b | --isp1          Set the ISP 1 source\n"
        "                         0: DVP\n"
        "                         1: CSI 0\n"
        "                         2: CSI 1\n"
        "-c | --cfg           Set the config file\n"
        "\n",
        argv[0]
        );
#else
    fprintf(fp,
        "Usage: %s [options]\n\n"
        "Options:\n"
        "-h | --help          Print this message\n"
        "-m | --sensor0       Sensor 0 name\n"
        "                         sc2235dvp\n"
        "                         ov4689mipi\n"
        "                         imx219mipi\n"
        "                         default is imx219mipi\n"
        "-n | --sensor1       Sensor 1 name\n"
        "                         sc2235dvp\n"
        "                         ov4689mipi\n"
        "                         imx219mipi\n"
        "-i | --dvp           Set the DVP interface source\n"
        "                         0: Sensor 0\n"
        "                         1: Sensor 1\n"
        "-j | --csi0          Set the CSI 0 interface source\n"
        "                         0: Sensor 0\n"
        "                         1: Sensor 1\n"
        "                         default is 0\n"
        "-k | --csi1          Set the CSI 1 interface source\n"
        "                         0: Sensor 0\n"
        "                         1: Sensor 1\n"
        "-a | --isp0          Set the ISP 0 source\n"
        "                         0: DVP\n"
        "                         1: CSI 0\n"
        "                         2: CSI 1\n"
        "                         default is 1\n"
        "-c | --cfg           Set the config file\n"
        "\n",
        argv[0]
        );
#endif //#if defined(CONFIG_STF_DUAL_ISP)
}

static STF_VOID ParseOptions(
    int argc,
    char **argv,
    ST_ISP_PARAMS *pstParams,
    STF_CHAR *pszConfigFile
    )
{
    //-------------------------------------------------------------------------
    const STF_CHAR szShortOptions[] = "hm:n:i:j:k:a:b:c:";
    const struct option stLongOptions[] = {
        { "help",       no_argument,        NULL,   'h' },
        { "sensor0",    required_argument,  NULL,   'm' },
        { "sensor1",    required_argument,  NULL,   'n' },
        { "dvp",        required_argument,  NULL,   'i' },
        { "csi0",       required_argument,  NULL,   'j' },
        { "csi1",       required_argument,  NULL,   'k' },
        { "isp0",       required_argument,  NULL,   'a' },
#if defined(CONFIG_STF_DUAL_ISP)
        { "isp1",       required_argument,  NULL,   'b' },
#endif //#if defined(CONFIG_STF_DUAL_ISP)
        { "cfg",        required_argument,  NULL,   'c' },
        { 0,            0,                  0,      0   }
    };
    //-------------------------------------------------------------------------
    int nIndex = 0;
    int nOpt = 0;
    int value = 0;

    STF_ASSERT(argv);           // null pointer forbidden
    STF_ASSERT(pstParams);      // null pointer forbidden
    STF_ASSERT(pszConfigFile);  // null pointer forbidden

    while (-1 != (nOpt = getopt_long(argc, argv, szShortOptions, stLongOptions, &nIndex))) {
        switch (nOpt) {
            case 0: /* getopt_long() flag */
                break;

            case 'h':
                PrintOptions(stdout, argc, argv);
                exit(EXIT_SUCCESS);
                break;

            case 'm':
                strncpy(pstParams->szSensor[0], optarg, STF_ISP_PARAM_LEN);
                break;

            case 'n':
                strncpy(pstParams->szSensor[1], optarg, STF_ISP_PARAM_LEN);
                break;

            case 'i':
                pstParams->nSensorInterface[0] = atoi(optarg);
                break;

            case 'j':
                pstParams->nSensorInterface[1] = atoi(optarg);
                break;

            case 'k':
                pstParams->nSensorInterface[2] = atoi(optarg);
                break;

            case 'a':
                pstParams->nISP[0] = atoi(optarg);
                break;

#if defined(CONFIG_STF_DUAL_ISP)
            case 'b':
                pstParams->nISP[1] = atoi(optarg);
                break;

#endif //#if defined(CONFIG_STF_DUAL_ISP)
            case 'c':
                strncpy(pszConfigFile, optarg, STF_ISP_PARAM_LEN);
                break;

            default:
                PrintOptions(stdout, argc, argv);
                exit(EXIT_SUCCESS);
                break;
        }
    }
}

static STF_RESULT ExtraIspSettingInitializeForEachDifferentSensor(
    ST_PIPELINE *pstPipeline,
    STF_U16 u16SensorId
    )
{
    ST_ISP_MOD_GMARGB *pstModGmaRgb = NULL;
    ST_ISP_MOD_SC *pstModSc = NULL;
    ST_ISP_CTL_AE *pstCtlAe = NULL;
    STF_RESULT Ret = STF_SUCCESS;

    if (!pstPipeline) {
        LOG_ERROR("pstPipeline = %p is NULL!\n",
            pstPipeline
            );
        return STF_ERROR_INVALID_ARG;
    }

    STF_ASSERT(EN_SENSOR_ID_MAX > u16SensorId);

    // Configure extra ISP setting for each different sensor.
    //=========================================================================
    V_LOG_DEBUG("Configure extra ISP setting for each different sensor.\n");
    switch (u16SensorId) {
        case EN_SENSOR_ID_SC2235_DVP:
            // sc2235dvp sensor.
            break;

        case EN_SENSOR_ID_OV4689_MIPI:
            // ov4689mipi sensor.
            break;

        case EN_SENSOR_ID_IMX219_MIPI:
            // imx219mipi sensor.
            pstModGmaRgb = (ST_ISP_MOD_GMARGB *)STFLIB_ISP_GetModule(
                &pstPipeline->stIspCtx,
                EN_MODULE_ID_GMARGB
                );
            if (pstModGmaRgb) {
                STFMOD_ISP_GMARGB_SetGammaFactor(
                    pstModGmaRgb,
                    1.9
                    );
            }
            pstModSc = (ST_ISP_MOD_SC *)STFLIB_ISP_GetModule(
                &pstPipeline->stIspCtx,
                EN_MODULE_ID_SC
                );
            if (pstModSc) {
                const STF_U8 u8AwbCW[13][13] = {
#if 0
                    //0.00  0.25  0.50  0.75  1.00  1.25  1.50  1.75  2.00  2.25  2.50  2.75  3.00
                    //------------------------------------------------------------------------------
                    {    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0 },   // 0.00
                    {    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0 },   // 0.25
                    {    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0 },   // 0.50
                    {    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0 },   // 0.75
                    {    0,    0,    0,    0,    8,    8,    8,    8,    8,    8,    8,    8,    8 },   // 1.00
                    {    0,    0,    0,    0,    8,   15,   15,   15,   15,    8,    8,    8,    8 },   // 1.25
                    {    0,    0,    0,    0,    8,   15,   15,   15,   15,    8,    8,    8,    8 },   // 1.50
                    {    0,    0,    0,    0,    8,   15,   15,   15,   15,    8,    8,    8,    8 },   // 1.75
                    {    0,    0,    0,    0,    8,   15,   15,   15,   15,    8,    8,    8,    8 },   // 2.00
                    {    0,    0,    0,    0,    8,   15,   15,   15,   15,    8,    8,    8,    8 },   // 2.25
                    {    0,    0,    0,    0,    8,    8,    8,    8,    8,    8,    8,    8,    8 },   // 2.50
                    {    0,    0,    0,    0,    8,    8,    8,    8,    8,    8,    8,    8,    8 },   // 2.75
                    {    0,    0,    0,    0,    8,    8,    8,    8,    8,    8,    8,    8,    8 },   // 3.00
                    //------------------------------------------------------------------------------
#else
                    //0.00  0.25  0.50  0.75  1.00  1.25  1.50  1.75  2.00  2.25  2.50  2.75  3.00
                    //------------------------------------------------------------------------------
                    {   15,   15,   15,   15,   15,   15,   15,   15,   15,   15,   15,   15,   15 },   // 0.00
                    {   15,   15,   15,   15,   15,   15,   15,   15,   15,   15,   15,   15,   15 },   // 0.25
                    {   15,   15,   15,   15,   15,   15,   15,   15,   15,   15,   15,   15,   15 },   // 0.50
                    {   15,   15,   15,   15,   15,   15,   15,   15,   15,   15,   15,   15,   15 },   // 0.75
                    {   15,   15,   15,   15,   15,   15,   15,   15,   15,   15,   15,   15,   15 },   // 1.00
                    {   15,   15,   15,   15,   15,   15,   15,   15,   15,   15,   15,   15,   15 },   // 1.25
                    {   15,   15,   15,   15,   15,   15,   15,   15,   15,   15,   15,   15,   15 },   // 1.50
                    {   15,   15,   15,   15,   15,   15,   15,   15,   15,   15,   15,   15,   15 },   // 1.75
                    {   15,   15,   15,   15,   15,   15,   15,   15,   15,   15,   15,   15,   15 },   // 2.00
                    {   15,   15,   15,   15,   15,   15,   15,   15,   15,   15,   15,   15,   15 },   // 2.25
                    {   15,   15,   15,   15,   15,   15,   15,   15,   15,   15,   15,   15,   15 },   // 2.50
                    {   15,   15,   15,   15,   15,   15,   15,   15,   15,   15,   15,   15,   15 },   // 2.75
                    {   15,   15,   15,   15,   15,   15,   15,   15,   15,   15,   15,   15,   15 },   // 3.00
                    //------------------------------------------------------------------------------
#endif //#if 0
                };
                STFMOD_ISP_SC_SetAwbWeightingTable(
                    pstModSc,
                    (STF_U8 (*)[13])u8AwbCW
                    );
            }
            pstCtlAe = (ST_ISP_CTL_AE *)STFLIB_ISP_GetControl(
                &pstPipeline->stIspCtx,
                EN_CONTROL_ID_AE
                );
            if (pstCtlAe) {
                const ST_EXPO_LOCK ST_EXPO_LOCK_50HZ_DEF[] = {
                    {  10000,  10000 },     // 10ms sensor exposure time.
                    {  20000,  20000 },     // 20ms sensor exposure time.
                    {  30000,  30000 },     // 30ms(30fps) sensor exposure time.
                    {  40000,  30000 },     // 30ms(30fps) sensor exposure time. AGC starts at  40000/30000= 1.33.
                    {  96000,  30000 },     // 30ms(30fps) sensor exposure time, AGC starts at  96000/30000= 3.2.
                    { 120000,  30000 },     // 30ms(30fps) sensor exposure time, AGC starts at 120000/30000= 4.00.
                    { 150000,  30000 },     // 30ms(30fps) sensor exposure time, AGC starts at 150000/30000= 5.00.
                    { 180000,  30000 },     // 30ms(30fps) sensor exposure time, AGC starts at 180000/30000= 6.00.
                    { 210000,  30000 },     // 30ms(30fps) sensor exposure time, AGC starts at 210000/30000= 7.00.
                    { 240000,  30000 },     // 30ms(30fps) sensor exposure time, AGC starts at 240000/30000= 8.00.
                    { 270000,  30000 },     // 30ms(30fps) sensor exposure time, AGC starts at 270000/30000= 9.00.
                    { 300000,  30000 },     // 30ms(30fps) sensor exposure time, AGC starts at 300000/30000=10.00.
                };
                const ST_EXPO_LOCK ST_EXPO_LOCK_60HZ_DEF[] = {
                    {   8333,   8333 },     // 8.333ms sensor exposure time
                    {  16667,  16667 },     // 16.667ms sensor exposure time
                    {  25000,  25000 },     // 25ms sensor exposure time
                    {  33333,  33333 },     // 33.333ms(30fps) sensor exposure time
                    {  41667,  33333 },     // 33.333ms(30fps) sensor exposure time. AGC starts at  41667/33333= 1.25.
                    {  66666,  33333 },     // 33.333ms(30fps) sensor exposure time, AGC starts at  66666/33333= 2.00.
                    {  99999,  33333 },     // 33.333ms(30fps) sensor exposure time, AGC starts at  99999/33333= 3.00.
                    { 133333,  33333 },     // 33.333ms(30fps) sensor exposure time, AGC starts at 133333/33333= 4.00.
                    { 142856,  33333 },     // 33.333ms(30fps) sensor exposure time, AGC starts at 142856/33333= 4.29.
                    { 160000,  33333 },     // 33.333ms(30fps) sensor exposure time, AGC starts at 160000/33333= 4.80.
                    { 166666,  33333 },     // 33.333ms(30fps) sensor exposure time, AGC starts at 166666/33333= 5.00.
                    { 200000,  33333 },     // 33.333ms(30fps) sensor exposure time, AGC starts at 200000/33333= 6.00.
                    { 230000,  33333 },     // 33.333ms(30fps) sensor exposure time, AGC starts at 230000/33333= 6.90.
                    { 270000,  33333 },     // 33.333ms(30fps) sensor exposure time, AGC starts at 270000/33333= 8.10.
                    { 300000,  33333 },     // 33.333ms(30fps) sensor exposure time, AGC starts at 300000/33333= 9.00.
                    { 340000,  33333 },     // 33.333ms(30fps) sensor exposure time, AGC starts at 340000/33333=10.20.
                };

                STFCTL_ISP_AE_SetTargetBrightness(
                    pstCtlAe,
                    80
                    );
                STFCTL_ISP_AE_SetExposureGainStep(
                    pstCtlAe,
                    0x0108
                    );
                STFCTL_ISP_AE_SetExposureLockTable(
                    pstCtlAe,
                    EN_EXPO_LOCK_FREQ_50,
                    (ST_EXPO_LOCK *)ST_EXPO_LOCK_50HZ_DEF,
                    sizeof(ST_EXPO_LOCK_50HZ_DEF) / sizeof(ST_EXPO_LOCK)
                    );
                STFCTL_ISP_AE_SetExposureLockTable(
                    pstCtlAe,
                    EN_EXPO_LOCK_FREQ_60,
                    (ST_EXPO_LOCK *)ST_EXPO_LOCK_60HZ_DEF,
                    sizeof(ST_EXPO_LOCK_60HZ_DEF) / sizeof(ST_EXPO_LOCK)
                    );
            }
            break;
    }
    //=========================================================================

ExtraIspSettingInitializeForEachDifferentSensor_Failed:

    return Ret;
}

static STF_RESULT ExtraSeneorSettingInitialize(
    ST_SENSOR *pstSensor,
    STF_U16 u16SensorId,
    STF_CHAR szSensorName[]
    )
{
    STF_U16 u16RegAddr = 0;
    STF_U16 u16RegValue = 0;
    STF_U16 u16BlcValue = 0;
    STF_RESULT Ret = STF_SUCCESS;

    if (!pstSensor) {
        LOG_ERROR("pstSensor = %p is NULL!\n",
            pstSensor
            );
        return STF_ERROR_INVALID_ARG;
    }

    STF_ASSERT(EN_SENSOR_ID_MAX > u16SensorId);

    // Set the sensor BLC value and mode as auto.
    //=========================================================================
    V_LOG_DEBUG("Set the sensor BLC mode as auto.\n");
    switch (u16SensorId) {
        case EN_SENSOR_ID_SC2235_DVP:
            // sc2235dvp sensor.
            break;

        case EN_SENSOR_ID_OV4689_MIPI:
            // ov4689mipi sensor.
            break;

        case EN_SENSOR_ID_IMX219_MIPI:
            // imx219mipi sensor.
            break;
    }
    //=========================================================================

    return STF_SUCCESS;

ExtraSeneorSettingInitialize_Failed:

    return Ret;
}

static STF_INT IspCtrl(
    STF_U8 u8IspIdx,
    STF_CHAR *pszSensorName
    )
{
    //-------------------------------------------------------------------------
    ST_CI_DEVICE stDevUo0;
    ST_CI_DEVICE *pstScDumpYHistDevice = NULL;
    struct v4l2_format stFormat;
    //-------------------------------------------------------------------------
    fd_set stReadSet;
    struct timeval stTimeVal = { 1, 0 };
    STF_S32 s32BufCount;
    //STF_U32 u32BufIndex[PIPELINE_IMG_BUF_MAX_TOTAL] = { 0 };
    ST_VDO_BUF_INFO stVideoBufferInfo[PIPELINE_IMG_BUF_MAX_TOTAL] = { 0 };
    //-------------------------------------------------------------------------
    CI_CONNECTION *pstCiConnection = NULL;
    //-------------------------------------------------------------------------
    STF_CHAR szSensorName[256];
    STF_U16 u16SensorId = 0;
    STF_U8 u8SensorIdx = 0;
    STF_U8 u8SensorMode = 0;
    STF_U8 u8SensorFlipping = SENSOR_FLIP_NONE;
    ST_SIZE stCaptureSize;
    //-------------------------------------------------------------------------
    ST_SENSOR stSensor;
    SENSOR_INFO stSensorInfo;
    SENSOR_INTFC stSensorInterfaceInfo;
    //-------------------------------------------------------------------------
    ST_PIPELINE stPipeline;
    //-------------------------------------------------------------------------
    ST_SHOT_INFO *pstShotInfo = NULL;
    //-------------------------------------------------------------------------
    STF_S8 s8ScDumpCount = 0;
    STF_BOOL8 bIsScDumpForAe = STF_FALSE;
    //-------------------------------------------------------------------------
    STF_BOOL8 bExit = STF_FALSE;
    STF_BOOL8 bLoopActive = STF_TRUE;
    STF_BOOL8 bCapture = STF_FALSE;
    STF_BOOL8 bFirstTime = STF_TRUE;
    STF_BOOL8 bIspEnable = STF_FALSE;
    STF_BOOL8 bSensorEnable = STF_FALSE;
    //-------------------------------------------------------------------------
    STF_U8 u8IspMainStep = EN_ISP_MAIN_STEP_NONE;
    //-------------------------------------------------------------------------
    STF_U32 u32Counter = 0;
    STF_CHAR szFilename[512] = "";
    //-------------------------------------------------------------------------
    STF_CHAR szIspBinParamFilename[512] = { 0 };
    struct stat stStat;
    STF_BOOL8 bFileExist = STF_FALSE;
    //-------------------------------------------------------------------------
    STF_RESULT Ret = STF_SUCCESS;
    //-------------------------------------------------------------------------

    STF_ASSERT(pszSensorName);  // null pointer forbidden

    LOG_INFO("=== Enter ISP control loop ===\n");

    strncpy(szSensorName, pszSensorName, sizeof(szSensorName));
    //u16SensorId = 0;                            // Select SC2235 DVP sensor.
    u8SensorIdx = 0;                            // Select index of configure file of sensor.
    u8SensorMode = 0;                           // Select mode 0 1080p 25fps.
    u8SensorFlipping = SENSOR_FLIP_NONE;        // Set sensor flip mode none.
    //stCaptureSize.u16Cx = CAPTURE_WIDTH;
    //stCaptureSize.u16Cy = CAPTURE_HEIGHT;
    //=========================================================================

    // Get the video device output size.
    //=========================================================================
#if 1
    {
        STF_BOOL8 bSuccessful = STF_FALSE;

        V_LOG_DEBUG("Open the UO video device!\n");
        if (STF_SUCCESS != (Ret = STFLIB_ISP_DEVICE_StructInitialize(
            &stDevUo0,
  #if defined(CONFIG_STF_DUAL_ISP)
            (EN_ISP_DEV_IDX_0 == u8IspIdx)
                ? (EN_DEV_ID_ISP_0_UO) : (EN_DEV_ID_ISP_1_UO),
  #else
            EN_DEV_ID_ISP_0_UO,
  #endif //#if defined(CONFIG_STF_DUAL_ISP)
            STF_FALSE,
            EN_MEM_TYPE_NONE,
            0,
            0
            ))) {
            LOG_ERROR("Cannot open \'%s\': %d, %s\n",
                stDevUo0.szDeviceName, errno, strerror(errno));
            goto ISP_CTRL_FAILURE_EXIT;
        }
        //---------------------------------------------------------------------
        V_LOG_DEBUG("Get the UO video device format!\n");
        STF_MEMSET(&stFormat, 0, sizeof(struct v4l2_format));
        stFormat.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        stFormat.fmt.pix.field = V4L2_FIELD_ANY;
        if (STF_SUCCESS != (Ret = stDevUo0.GetFormat(&stDevUo0, &stFormat))) {
            LOG_ERROR("Failed to get the \'%s\' device format, Error: %d, %s\n",
                stDevUo0.szDeviceName, errno, strerror(errno));
        } else {
            stCaptureSize.u16Cx = stFormat.fmt.pix.width;
            stCaptureSize.u16Cy = stFormat.fmt.pix.height;
            bSuccessful = STF_TRUE;
            V_LOG_INFO("The \'%s\' device capture size is %dx%d\n",
                stDevUo0.szDeviceName, stCaptureSize.u16Cx,
                stCaptureSize.u16Cy);
        }
        //---------------------------------------------------------------------
        V_LOG_DEBUG("Close the UO video device!\n");
        // check if opening was successfull
        if (STF_SUCCESS != (Ret = STFLIB_ISP_DEVICE_StructUninitialize(
            &stDevUo0))) {
            LOG_ERROR("Failed to close \'%s\', Error: %d, %s\n",
                stDevUo0.szDeviceName, errno, strerror(errno));
            goto ISP_CTRL_FAILURE_EXIT;
        }
        if (STF_FALSE == bSuccessful) {
            goto ISP_CTRL_FAILURE_EXIT;
        }
    }
#else
    stCaptureSize.u16Cx = CAPTURE_WIDTH;
    stCaptureSize.u16Cy = CAPTURE_HEIGHT;
#endif //#if 0
    //=========================================================================

    // Get the connection.
    //=========================================================================
    V_LOG_DEBUG("Connected to the ISP kernel driver!\n");
    Ret = STFLIB_ISP_BASE_Connection(u8IspIdx);
    if (Ret) {
        LOG_ERROR("Failed to connected to the ISP driver\n");
        Ret = STF_ERROR_DEVICE_NOT_FOUND;
        goto ISP_CTRL_FAILURE_EXIT;
    } else {
        pstCiConnection = (CI_CONNECTION *)STFLIB_ISP_BASE_GetConnection(
            u8IspIdx
            );
        V_LOG_DEBUG("ISP driver is connected, connections = 0x%016llX\n",
            pstCiConnection
            );
    }
    //=========================================================================

    // Get the sensor ID.
    //=========================================================================
    V_LOG_DEBUG("Get the sensor index.\n");
    if (-1 == (u16SensorId = STFLIB_ISP_SENSOR_GetSensorId(
        szSensorName
        ))) {
        LOG_ERROR("Failed to get the sensor Id of \"%s\"!!!\n", szSensorName);
        Ret = STF_ERROR_DEVICE_NOT_FOUND;
        goto ISP_CTRL_FAILURE_DISCONNECTION;
    }
    V_LOG_INFO("The \"%s\" ID = %d!!!\n", szSensorName, u16SensorId);
    //=========================================================================

    // ST_SENSOR structure initialize.
    //=========================================================================
    V_LOG_DEBUG("Configure the sensor structure.\n");
    if (STF_SUCCESS != (Ret = STFLIB_ISP_SENSOR_SturctInitialize(
        &stSensor,
#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
        pstCiConnection,
        u8IspIdx,
#endif //#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
        u16SensorId,
        u8SensorIdx
        ))) {
        LOG_ERROR("Failed to initialize ST_SENSOR structure!!!\n");
        Ret = STF_ERROR_FAILURE;
        goto ISP_CTRL_FAILURE_DISCONNECTION;
    }

    //-------------------------------------------------------------------------
    V_LOG_DEBUG("Get the sensor state.\n");
    if (SENSOR_ERROR == stSensor.GetState(&stSensor)) {
        LOG_ERROR("Error can't get camera sensor , "\
            "please insmod sensor driver\n");
        Ret = STF_ERROR_FAILURE;
        goto ISP_CTRL_FAILURE_SENSOR_STRUCT_UNINIT;
    }
    //=========================================================================

    // Get sensor ID.
    //=========================================================================
    V_LOG_DEBUG("Get the sensor ID.\n");
    {
        STF_U16 u16SensorID = 0;
        STF_U16 u16RegAddr = 0;
        STF_U16 u16RegValue = 0;

        switch (u16SensorId) {
            case 0:
                // sc2235dvp sensor.
                u16RegAddr = 0x3107;
                break;

            case 1:
                // ov4689mipi sensor.
                u16RegAddr = 0x300A;
                break;

            case 2:
                // imx219mipi sensor.
                u16RegAddr = 0x0000;
                break;

            default:
                STF_ASSERT(u16SensorId < 3);
                break;
        }
        if (STF_SUCCESS != (Ret = stSensor.GetReg(&stSensor, u16RegAddr,
            &u16RegValue))) {
            LOG_ERROR("Failed to get sensor's register [0x%04X] value!!!\n",
                u16RegAddr);
        }
        u16SensorID = (u16RegValue << 8);
        u16RegAddr += 1;
        if (STF_SUCCESS != (Ret = stSensor.GetReg(&stSensor, u16RegAddr,
            &u16RegValue))) {
            LOG_ERROR("Failed to get sensor's register [0x%04X] value!!!\n",
                u16RegAddr);
        }
        u16SensorID |= (u16RegValue & 0x00FF);
        V_LOG_INFO("Get the sensor \'%s\' ID = 0x%04X\n", szSensorName,
            u16SensorID);
    }
    //=========================================================================

    // Get sensor information.
    //=========================================================================
    V_LOG_DEBUG("Get the sensor information.\n");
    if (STF_SUCCESS != (Ret = stSensor.GetInfo(&stSensor, &stSensorInfo))) {
        LOG_ERROR("Failed to get sensor's information!!!\n");
        goto ISP_CTRL_FAILURE_SENSOR_STRUCT_UNINIT;
    }
    //=========================================================================

    // Get sensor interface information.
    //=========================================================================
    V_LOG_DEBUG("Get the sensor interface information.\n");
    if (STF_SUCCESS != (Ret = stSensor.GetInterfaceInfo(&stSensor,
        &stSensorInterfaceInfo))) {
        LOG_ERROR("Failed to get sensor's interface information!!!\n");
        goto ISP_CTRL_FAILURE_SENSOR_STRUCT_UNINIT;
    }
    //=========================================================================

    // Initialize sensor registers.
    //=========================================================================
    V_LOG_DEBUG("Initialize the sensor.\n");
    switch (u16SensorId) {
        case 0:
            // sc2235dvp sensor.
            u8SensorFlipping = SENSOR_FLIP_NONE;
            break;

        case 1:
            // ov4689mipi sensor.
            u8SensorFlipping = SENSOR_FLIP_VERTICAL;
            break;

        case 2:
            // imx219mipi sensor.
            u8SensorFlipping = SENSOR_FLIP_NONE;
            break;

        default:
            STF_ASSERT(u16SensorId < 3);
            break;
    }
    u8SensorMode = stSensor.GetModeIdx(
        &stSensor, stCaptureSize.u16Cx, stCaptureSize.u16Cy);
    if (-1 == u8SensorMode) {
        u8SensorMode = 0;
    }
    if (0 <= u8SensorMode) {
        if (STF_SUCCESS != (Ret = stSensor.Configure(&stSensor, u8SensorMode,
            u8SensorFlipping))) {
            LOG_ERROR("Unable to configure the sensor for mode %d with "\
                "flipping 0x%x\n", u8SensorMode, u8SensorFlipping);
            goto ISP_CTRL_FAILURE_SENSOR_STRUCT_UNINIT;
        }
    } else {
        LOG_ERROR("The sensor mode is less than the 0!!!\n");
        goto ISP_CTRL_FAILURE_SENSOR_STRUCT_UNINIT;
    }
    //=========================================================================

    // ST_PIPELINE structure initialize.
    //=========================================================================
    V_LOG_DEBUG("Configure the pipeline structure.\n");
    if (STF_SUCCESS != (Ret = STFLIB_ISP_PIPELINE_StructInitialize(
        &stPipeline,
        u8IspIdx,
#if defined(VIRTUAL_IO_MAPPING)
        pstCiConnection,
#endif //VIRTUAL_IO_MAPPING
        &stSensor,
#if defined(ENABLE_DMABUF_METHOD)
        EN_MEM_TYPE_DMA,
#else
        EN_MEM_TYPE_MMAP,
#endif //#if defined(ENABLE_DMABUF_METHOD)
        stCaptureSize,
        STF_TRUE
        ))) {
        LOG_ERROR("Failed to initialize ST_PIPELINE structure!!!\n");
        goto ISP_CTRL_FAILURE_SENSOR_STRUCT_UNINIT;
    }
    //=========================================================================

    // Register ths pipeline configuration into kernel driver.
    //=========================================================================
    V_LOG_DEBUG("Register pipeline to ISP device.\n");
    if (STF_SUCCESS != (Ret = stPipeline.Register(&stPipeline))) {
        LOG_ERROR("Failed to register the pipeline configuration "\
            "into kernel driver!!!\n");
        goto ISP_CTRL_FAILURE_PIPELINE_STRUCT_UNINIT;
    }
    //=========================================================================

    // SC_DUMP_Y_HIST video device initialize.
    //=========================================================================
    {
        STF_U32 u32PixelFormat;

        V_LOG_DEBUG("Get the \"SC_DUMP_YHIST\" video device handle.\n");
        if (NULL == (pstScDumpYHistDevice = STFLIB_ISP_GetVideoDevice(
            stPipeline.GetIspContext(&stPipeline),
            EN_ISP_PORT_ID_SC_DUMP_YHIST
            ))) {
            LOG_ERROR("Failed to get the \"SC_DUMP_YHIST\" video device "\
                "handle.\n");
            goto ISP_CTRL_FAILURE_PIPELINE_STRUCT_UNINIT;
        }

        V_LOG_DEBUG("SC_DUMP_Y_HIST video device initialize.\n");
#if 0
        if (12 == stSensorInfo.stMode.u8BitDepth) {
            u32PixelFormat = g_u32BayerFormatDef[stSensorInfo.enBayerOriginal - 1 + BAYER_FORMAT_MAX];
        } else {
            u32PixelFormat = g_u32BayerFormatDef[stSensorInfo.enBayerOriginal - 1];
        }
#else
        u32PixelFormat = g_u32BayerFormatDef[stSensorInfo.enBayerOriginal - 1 + BAYER_FORMAT_MAX];
#endif //#if 0
        if (STF_SUCCESS != (Ret = pstScDumpYHistDevice->InitDevice(
            pstScDumpYHistDevice,
            u32PixelFormat
            ))) {
            LOG_ERROR("Failed to initialize the SC_DUMP_Y_HIST video "\
                "device!!!\n");
            goto ISP_CTRL_FAILURE_PIPELINE_STRUCT_UNINIT;
        }
    }
    //=========================================================================

    // Enable modules and controls to pipeline.
    //=========================================================================
    V_LOG_DEBUG("Enable modules and controls to pipeline.\n");
    if (STF_SUCCESS != (Ret = EnableIspModulesAndControls(&stPipeline))) {
        LOG_ERROR("failed to call EnableIspModulesAndControls function!\n");
        goto ISP_CTRL_FAILURE_PIPELINE_STRUCT_UNINIT;
    }
    //g_stPipeline.PrintRegisteredModulesAndControls(&g_stPipeline);
    //=========================================================================

    // ISP load binary parameters.
    //=========================================================================
    bFileExist = STF_FALSE;
    sprintf(szIspBinParamFilename, "/root/.isp_setting/IspSetting_%dx%d.ybn",
        stPipeline.stIspCtx.stImgSize.u16Cx,
        stPipeline.stIspCtx.stImgSize.u16Cy
        );
    bFileExist = (0 == stat(szIspBinParamFilename, &stStat))
        ? STF_TRUE : STF_FALSE;
    if (STF_FALSE == bFileExist) {
        sprintf(szIspBinParamFilename, "/root/ISP/IspSetting_%s_%dx%d.ybn",
            szSensorName,
            stPipeline.stIspCtx.stImgSize.u16Cx,
            stPipeline.stIspCtx.stImgSize.u16Cy
            );
        bFileExist = (0 == stat(szIspBinParamFilename, &stStat))
            ? STF_TRUE : STF_FALSE;
    }
    if (STF_FALSE == bFileExist) {
        LOG_ERROR("===== Doesn't found the %dx%d resolution setting, "\
            "Trying to use the %dx%d resolution setting. =====\n",
            stPipeline.stIspCtx.stImgSize.u16Cx,
            stPipeline.stIspCtx.stImgSize.u16Cy,
            CAPTURE_WIDTH,
            CAPTURE_HEIGHT
            );

        sprintf(szIspBinParamFilename, "/root/ISP/IspSetting_%s_%dx%d.ybn",
            szSensorName,
            CAPTURE_WIDTH,
            CAPTURE_HEIGHT
            );
        bFileExist = (0 == stat(szIspBinParamFilename, &stStat))
            ? STF_TRUE : STF_FALSE;
    }
    Ret = STF_SUCCESS;
    if (bFileExist) {
        V_LOG_DEBUG("ISP load binary parameters: %s.\n", szIspBinParamFilename);
        if (STF_SUCCESS != (Ret = stPipeline.LoadBinParam(
            &stPipeline,
            szIspBinParamFilename
            ))) {
            LOG_ERROR("Failed to the \'%s\' binary parameters file!\n",
                szIspBinParamFilename);
            //*pRet = STF_ERROR_FAILURE;
            //goto Isp_Main_Thread_Failed_Destory_Pipeline_Structure;
        }
    } else {
        LOG_ERROR("===== No matching ISP settings file found!!! =====\n");
    }

#if 1
    if ((STF_FALSE == bFileExist) || (STF_SUCCESS != Ret)) {
        // Configure extra ISP setting initialize for each different sensor.
        //=====================================================================
        V_LOG_DEBUG("Configure extra ISP setting initialize for each "\
            "different sensor.\n");
        if (STF_SUCCESS != (Ret = ExtraIspSettingInitializeForEachDifferentSensor(
            &stPipeline,
            u16SensorId
            ))) {
            LOG_ERROR("Failed to configure extra ISP setting for "\
                "each different sensor!!!\n");
            //*pRet = STF_ERROR_FAILURE;
            //goto Isp_Main_Thread_Failed_Destory_Pipeline_Structure;
        }
        //=====================================================================
    }

#endif //#if 1
    // ISP load binary calibration data.
    //=========================================================================
    V_LOG_DEBUG("ISP load binary calibration data.\n");
    if (STF_SUCCESS != (Ret = stPipeline.LoadBinClbrt(
        &stPipeline
        ))) {
        LOG_ERROR("Failed to call LoadBinClbrt() function!\n");
        //*pRet = STF_ERROR_FAILURE;
        //goto Isp_Main_Thread_Failed_Destory_Pipeline_Structure;
    }

    // Register modules and controls to pipeline.
    //=========================================================================
    V_LOG_DEBUG("Register modules and controls to pipeline.\n");
    if (STF_SUCCESS != (Ret = RegisterEnableModulesAndControlsToPipeline(
        &stPipeline
        ))) {
        LOG_ERROR("failed to call RegisterEnableModulesAndControlsToPipeline "\
            "function!\n");
        goto ISP_CTRL_FAILURE_PIPELINE_STRUCT_UNINIT;
    }
    //g_stPipeline.PrintRegisteredModulesAndControls(&g_stPipeline);
    //=========================================================================

    // Program all of CSI and ISP registers value at first time.
    //=========================================================================
    V_LOG_DEBUG("Setup all of the modules setting to ISP device.\n");
    if (STF_SUCCESS != (Ret = stPipeline.SetupAll(&stPipeline))) {
        LOG_ERROR("Failed to program all of CSI and ISP registers value!!!\n");
        goto ISP_CTRL_FAILURE_PIPELINE_STRUCT_UNINIT;
    }
    //=========================================================================

    //LOG_INFO("ISP load binary parameters: %s.\n", g_szIspBinParamFilename);
    //=========================================================================

    //LOG_INFO("ISP load binary calibration data.\n");
    //=========================================================================

    // Allocate needs memory.
    //=========================================================================
    V_LOG_DEBUG("Allocate needs memory.\n");
    //STFLIB_ISP_MemDebugInfoEnable(STF_ENABLE);
    {
        STF_U8 u8ByfferType = 0;

        //=== Godspeed === Add new memory/buffer type support here.
        if (STF_SUCCESS != (Ret = stPipeline.BufAllocate(
            &stPipeline,
#if defined(PIPELINE_BUF_PUSH_POP_DISABLE)
            1,
#else
            (PIPELINE_IMG_BUF_MAX - 1),
#endif //#if defined(PIPELINE_BUF_PUSH_POP_DISABLE)
            u8ByfferType,
            STF_TRUE,
            STF_TRUE,
            NULL
            ))) {
            LOG_ERROR("Failed to allocate needs memory!!!\n");
            goto ISP_CTRL_FAILURE_PIPELINE_STRUCT_UNINIT;
        }
    }
    //STFLIB_ISP_MemDebugInfoEnable(STF_DISABLE);
    //=========================================================================

#if 0
    // Check allocated ScDump and YHist buffer information.
    //=========================================================================
    V_LOG_DEBUG("Check allocated ScDump and YHist buffer information.\n");
    {
        ST_SHOT_INFO *pstShotInfo = NULL;
        CI_MEM_PARAM *pstMemParam = NULL;
        STF_U32 u32Idx;
        STF_U32 u32Count = 0;

        u32Count = stPipeline.ShotGetAllocatedCount(&stPipeline);
        V_LOG_DEBUG("================================\n");
        V_LOG_DEBUG("Check ScDump buffer information by shot.\n");
        for (u32Idx = 0; u32Idx < u32Count; u32Idx++) {
            if (STF_SUCCESS != (Ret = stPipeline.ShotGetDirectly(
                &stPipeline,
                u32Idx,
                &pstShotInfo
                ))) {
                LOG_ERROR("Failed to get the shot information\n");
                goto ISP_CTRL_FAILURE_MEM_FREE;
            }
            if (NULL != pstShotInfo) {
                LOG_INFO("[%02d] - pBuffer = %p\n",
                    u32Idx,
                    pstShotInfo->stScDumpMem.pBuffer
                    );
            }
        }
        V_LOG_DEBUG("--------------------------------\n");
        V_LOG_DEBUG("Check YHist buffer information by shot.\n");
        for (u32Idx = 0; u32Idx < u32Count; u32Idx++) {
            if (STF_SUCCESS != (Ret = stPipeline.ShotGetDirectly(
                &stPipeline,
                u32Idx,
                &pstShotInfo
                ))) {
                LOG_ERROR("Failed to get the shot information\n");
                goto ISP_CTRL_FAILURE_MEM_FREE;
            }
            if (NULL != pstShotInfo) {
                LOG_INFO("[%02d] - pBuffer = %p\n",
                    u32Idx,
                    pstShotInfo->stYHistMem.pBuffer
                    );
            }
        }
        V_LOG_DEBUG("================================\n");
        V_LOG_DEBUG("Check ScDump buffer information by mem.\n");
        for (u32Idx = 0; u32Idx < u32Count; u32Idx++) {
            if (STF_SUCCESS != (Ret = stPipeline.BufGetDirectly(
                &stPipeline,
                u32Idx,
                EN_PIPELINE_BUF_ID_SC_DUMP,
                &pstMemParam
                ))) {
                LOG_ERROR("Failed to get the buffer information\n");
                goto ISP_CTRL_FAILURE_MEM_FREE;
            }
            if (NULL != pstMemParam) {
                LOG_INFO("[%02d] - pBuffer = %p\n",
                    u32Idx,
                    pstMemParam->pBuffer
                    );
            }
        }
        V_LOG_DEBUG("--------------------------------\n");
        V_LOG_DEBUG("Check YHist buffer information by mem.\n");
        for (u32Idx = 0; u32Idx < u32Count; u32Idx++) {
            if (STF_SUCCESS != (Ret = stPipeline.BufGetDirectly(
                &stPipeline,
                u32Idx,
                EN_PIPELINE_BUF_ID_YHIST,
                &pstMemParam
                ))) {
                LOG_ERROR("Failed to get the buffer information\n");
                goto ISP_CTRL_FAILURE_MEM_FREE;
            }
            if (NULL != pstMemParam) {
                LOG_INFO("[%02d] - pBuffer = %p\n",
                    u32Idx,
                    pstMemParam->pBuffer
                    );
            }
        }
        V_LOG_DEBUG("================================\n");
    }
    //=========================================================================

#endif //#if 0
    // Configure the pipeline state as ready.
    //=========================================================================
    V_LOG_DEBUG("Configure the pipeline state as ready.\n");
    if (STF_SUCCESS != (Ret = stPipeline.SetPipelineReady(
        &stPipeline
        ))) {
        LOG_ERROR("Failed to set pipeline state as ready!\n");
        Ret = STF_ERROR_FAILURE;
        goto ISP_CTRL_FAILURE_MEM_FREE;
    }
    //=========================================================================

    // Configure the contiguous capture mode as false.
    //=========================================================================
    V_LOG_DEBUG("Configure the contiguous capture mode as false.\n");
    if (STF_SUCCESS != (Ret = stPipeline.ShotSetContiguousCaptureMode(
        &stPipeline,
        STF_FALSE,
#if defined(SUPPORT_DUMP_MODULE)
        CI_TYPE_DUMP,
#else
        CI_TYPE_TILING_1_WR,
#endif //#if defined(SUPPORT_DUMP_MODULE)
        -1
        ))) {
        LOG_ERROR("Failed to set contiguous capture mode!\n");
        Ret = STF_ERROR_FAILURE;
        goto ISP_CTRL_FAILURE_MEM_FREE;
    }
    //=========================================================================

    // Enable the sensor.
    //=========================================================================
    V_LOG_DEBUG("Enable the sensor.\n");
    if (STF_SUCCESS != (Ret = stSensor.Enable(&stSensor))) {
        LOG_ERROR("Failed to enable sensor!\n");
        Ret = STF_ERROR_FAILURE;
        goto ISP_CTRL_FAILURE_MEM_FREE;
    } else {
        bSensorEnable = STF_TRUE;
    }

    // Configure the pipeline into active capture state.
    //=========================================================================
    V_LOG_DEBUG("Configure the pipeline into active capture state.\n");
    if (STF_SUCCESS != (Ret = stPipeline.SetActiveCapture(
        &stPipeline
        ))) {
        LOG_ERROR("Failed to set active capture!\n");
        Ret = STF_ERROR_FAILURE;
        goto ISP_CTRL_FAILURE_MEM_FREE;
    }
    //=========================================================================

    // Reset the frame count.
    //=========================================================================
    V_LOG_DEBUG("Reset the frame count.\n");
    if (STF_SUCCESS != (Ret = stPipeline.ResetFrameCount(
        &stPipeline
        ))) {
        LOG_ERROR("Failed to reset the ISP frame count!\n");
        Ret = STF_ERROR_FAILURE;
        goto ISP_CTRL_FAILURE_MEM_FREE;
    }
    //=========================================================================

#if 1
    // Configure extra sensor setting for each different sensor.
    //=========================================================================
    V_LOG_DEBUG("Configure extra sensor setting for each different sensor.\n");
    if (STF_SUCCESS != (Ret = ExtraSeneorSettingInitialize(
        &stSensor,
        u16SensorId,
        szSensorName
        ))) {
        LOG_ERROR("Failed to configure extra senosr setting for "\
            "each different sensor!!!\n");
        //*pRet = STF_ERROR_FAILURE;
        //goto Isp_Main_Thread_Failed_Buffer_Free;
    }
    //=========================================================================

#endif //#if 1
    //=========================================================================
    srand(time(0));
    usleep(100*1000);
#if 0
    PrintUsage();
#endif //#if 0
    //=========================================================================

    // Get sensor information.
    //=========================================================================
    V_LOG_DEBUG("Get the sensor information.\n");
    if (STF_SUCCESS != (Ret = stSensor.GetInfo(&stSensor, &stSensorInfo))) {
        LOG_ERROR("Failed to get sensor's information!!!\n");
        goto ISP_CTRL_FAILURE_SENSOR_DISABLE;
    }
    //=========================================================================

    //=========================================================================
    bLoopActive = STF_TRUE;
    bCapture = STF_FALSE;
    bIspEnable = STF_FALSE;
    bSensorEnable = STF_FALSE;
    //=========================================================================

    //=========================================================================
    s8ScDumpCount = 0;
    bIsScDumpForAe = STF_FALSE;
    //=========================================================================

    //=========================================================================
    pstShotInfo = NULL;
    u8IspMainStep = EN_ISP_MAIN_STEP_STOP;
    //=========================================================================

    V_LOG_DEBUG("Enter the 3A control loop.\n");
    //=========================================================================
    u32Counter = 0;
    //=========================================================================
    bExit = STF_FALSE;
    //=========================================================================
    while ((STF_FALSE == bExit) && (bLoopActive)) {
        STF_CHAR chReadKey;

        if (bFirstTime) {
            V_LOG_DEBUG("Enter the while loop.\n");
            bFirstTime = STF_FALSE;
        }

        FD_ZERO(&stReadSet);
        FD_SET(
            pstScDumpYHistDevice->GetFileHandle(pstScDumpYHistDevice),
            &stReadSet
            );

        /* Timeout. */
        stTimeVal.tv_sec = 1;
        stTimeVal.tv_usec = 0;

        Ret = select(
            pstScDumpYHistDevice->GetFileHandle(pstScDumpYHistDevice) + 1,
            &stReadSet,
            NULL,
            NULL,
            &stTimeVal
            );
        if (0 > Ret) {
            if (EINTR == errno) {
                continue;
            }
            LOG_ERROR("Get event failed!\n");
            goto ISP_CTRL_FAILURE_SC_DUMP_YHIST_STREAM_OFF;
        } else if (0 == Ret) {
            V_LOG_INFO("Get event timeout!\n");
        } else {
            s32BufCount = -1;
            Ret = pstScDumpYHistDevice->DequeueBuf_2(
                pstScDumpYHistDevice,
                &s32BufCount,
                &stVideoBufferInfo[0]
                );
            if (0 < s32BufCount) {
                CI_MEM_PARAM *pstMem = NULL;
                static STF_U32 u32BufferSize = HW_CI_ScDumpSize()
                    + HW_CI_YHistSize();
                STF_U32 u32Idx;

                for (u32Idx = 0; u32Idx < s32BufCount; u32Idx++) {
                    //V_LOG_INFO("Received buffer index = %d, bIsScDumpForAe = %s\n",
                    //    stVideoBufferInfo[u32Idx].u8BufIdx,
                    //    (stVideoBufferInfo[u32Idx].bIsScDumpForAe)
                    //        ? ("True") : ("False")
                    //    );
                    // Get the received shot pointer.
                    //=========================================================
                    if (STF_SUCCESS != (Ret = stPipeline.ShotGetDirectly(
                        &stPipeline,
                        stVideoBufferInfo[u32Idx].u8BufIdx,
                        &pstShotInfo
                        ))) {
                        LOG_ERROR("Failed to get the shot information\n");
                        goto ISP_CTRL_FAILURE_MEM_FREE;
                    }

                    // Check the SC extract point.
                    //=========================================================
                    if (bIsScDumpForAe
                        != stVideoBufferInfo[u32Idx].bIsScDumpForAe) {
                        bIsScDumpForAe =
                            stVideoBufferInfo[u32Idx].bIsScDumpForAe;
                        s8ScDumpCount = 0;
                    }
                    s8ScDumpCount++;
                    pstShotInfo->bIsScDumpForAe = bIsScDumpForAe;
                    pstShotInfo->u32ScDumpCount = s8ScDumpCount;

                    // Read the Y histogram results into the YHist memory
                    // for the Y curve.
                    //=========================================================
                    pstMem = pstShotInfo->pstYHistMem;
                    if (0 < pstMem->stSysMem.uiAllocated) {
                        Ret = STFDRV_ISP_YHIST_GetAccResult2(
#if defined(VIRTUAL_IO_MAPPING)
                            pstCiConnection,
#endif //VIRTUAL_IO_MAPPING
                            u8IspIdx,
                            pstMem->pBuffer
                            );
                        if (Ret) {
                            LOG_ERROR("Failed to read Y histogram results.\n");
                        }
                    }

                    // Assign this shot information to completed for 3A control.
                    //=========================================================
                    if (STF_SUCCESS != (Ret = stPipeline.ShotAssignToCompleted(
                        &stPipeline,
                        pstShotInfo
                        ))) {
                        LOG_ERROR("Failed to assign this shot information "\
                            "to completed!\n");
                        goto ISP_CTRL_FAILURE_SC_DUMP_YHIST_STREAM_OFF;
                    }

                    u32Counter++;
                    // Calculate registered controls.
                    //===================================================
                    //LOG_INFO("Calculate registered controls.\n");
                    if (STF_SUCCESS != (
                        Ret = stPipeline.CalculateAllRequestedControls(
                            &stPipeline))
                        ) {
                        LOG_ERROR("failed to call g_stPipeline."\
                            "CalculateAllRequestedControls function!\n");
                        goto ISP_CTRL_FAILURE_SC_DUMP_YHIST_STREAM_OFF;
                    }
                    //stPipeline.PrintRegisteredModulesAndControls(&stPipeline);
                    //===================================================

                    // Update registered modules.
                    //=======================================================
                    //LOG_INFO("Update registered modules.\n");
                    if (STF_SUCCESS != (Ret = stPipeline.UpdateAllRequestedModules(&stPipeline))) {
                        LOG_ERROR("failed to call stPipeline.UpdateAllRequestedModules function!\n");
                        goto ISP_CTRL_FAILURE_SC_DUMP_YHIST_STREAM_OFF;
                    }
                    //stPipeline.PrintRegisteredModulesAndControls(&stPipeline);
                    //=======================================================

                    Ret = pstScDumpYHistDevice->EnqueueBuf_2(
                        pstScDumpYHistDevice,
                        1,
                        &stVideoBufferInfo[u32Idx]
                        );
                    if (STF_SUCCESS != Ret) {
                        LOG_ERROR("Failed to enqueue ScDump and YHist buffer "\
                            "into the \"SC_DUMP_YHIST\" video device.\n");
                        goto ISP_CTRL_FAILURE_SC_DUMP_YHIST_STREAM_OFF;
                    }
                }
            }
        }

        pthread_mutex_lock(&(g_stIspMainThreadData.lock));
        bExit = g_stIspMainThreadData.bExit;
        pthread_mutex_unlock(&(g_stIspMainThreadData.lock));
        if (STF_FALSE == bExit) {
            pthread_mutex_lock(&(g_stVDevDetThread[u8IspIdx].lock));
            bExit = (1 == g_stVDevDetThread[u8IspIdx].u32SourceCount)
                ? (STF_TRUE) : (STF_FALSE);
            pthread_mutex_unlock(&(g_stVDevDetThread[u8IspIdx].lock));
        }
        usleep(5);
    }
    V_LOG_DEBUG("Exit the 3A control loop.\n");

ISP_CTRL_FAILURE_SC_DUMP_YHIST_STREAM_OFF:
    V_LOG_DEBUG("Configure the pipeline into inactive capture state.\n");
    if (STF_SUCCESS != (Ret = stPipeline.ClearActiveCapture(
        &stPipeline
        ))) {
        LOG_ERROR("Failed to clear active capture state!\n");
    }
ISP_CTRL_FAILURE_SENSOR_DISABLE:
    V_LOG_DEBUG("Disable the sensor.\n");
    usleep(1000 * 200);
    if (STF_SUCCESS != (Ret = stSensor.Disable(&stSensor))) {
        LOG_ERROR("Failed to disable the sensor.\n");
    }
ISP_CTRL_FAILURE_MEM_FREE:
    //u8IspMainStep = EN_ISP_MAIN_STEP_NONE;
ISP_CTRL_FAILURE_PIPELINE_STRUCT_UNINIT:
    V_LOG_DEBUG("Destroy the pipeline object!\n");
    if (STF_SUCCESS != (Ret = STFLIB_ISP_PIPELINE_StructUninitialize(
        &stPipeline))) {
        LOG_ERROR("Failed to uninitialize ST_PIPELINE structure!!!\n");
    }
ISP_CTRL_FAILURE_SENSOR_STRUCT_UNINIT:
    V_LOG_DEBUG("Destroy the sensor object!\n");
    if (STF_SUCCESS != (Ret = stSensor.Destroy(&stSensor))) {
        LOG_ERROR("Failed to uninitialize ST_SENSOR structure!!!\n");
    }
ISP_CTRL_FAILURE_DISCONNECTION:
    V_LOG_DEBUG("Disconnected from the ISP kernel driver!\n");
    if (!pstCiConnection) {
        if (STF_SUCCESS != (Ret = STFLIB_ISP_BASE_Disconnection(u8IspIdx))) {
            LOG_ERROR("Failed to disconnected to the ISP driver\n");
        } else {
            pstCiConnection = NULL;
        }
    }
ISP_CTRL_FAILURE_EXIT:
    LOG_INFO("=== Exit ISP control loop ===\n");

    return Ret;
}

int main(
    int argc,
    char *argv[]
    )
{
    //-------------------------------------------------------------------------
    struct termios stOriginalTerminalAttribute;
    STF_BOOL8 bExit = STF_FALSE;
    STF_U32 u32SourceCount[EN_ISP_DEV_IDX_MAX] = { 0 };
    STF_BOOL8 bStart = STF_FALSE;
    int nRet;
    STF_INT nIspIdx = EN_ISP_DEV_IDX_0;
    //-------------------------------------------------------------------------
    STF_INT nIdx;
    ST_CI_DEVICE stIspDev[EN_ISP_DEV_IDX_MAX];
    //-------------------------------------------------------------------------
    STF_CHAR szConfigFile[STF_ISP_PARAM_LEN] = {
#if 0
        STF_ISP_CTRL_CFG_DEF
#else
        ""
#endif //#if 0
    };
    ST_ISP_CFG_PARAMS stIspCtrlCfgParams = {
        13,
        {
            { "Sensor0", "", EN_ISP_CFG_PARAM_TYPE_STR },
            { "Sensor1", "", EN_ISP_CFG_PARAM_TYPE_STR },
            { "DVP", "", EN_ISP_CFG_PARAM_TYPE_INT },
            { "CSI0", "", EN_ISP_CFG_PARAM_TYPE_INT },
            { "CSI1", "", EN_ISP_CFG_PARAM_TYPE_INT },
            { "ISP0", "", EN_ISP_CFG_PARAM_TYPE_INT },
            { "ISP1", "", EN_ISP_CFG_PARAM_TYPE_INT },
            { "ISP0_Width", "", EN_ISP_CFG_PARAM_TYPE_INT },
            { "ISP1_Width", "", EN_ISP_CFG_PARAM_TYPE_INT },
            { "ISP0_Height", "", EN_ISP_CFG_PARAM_TYPE_INT },
            { "ISP1_Height", "", EN_ISP_CFG_PARAM_TYPE_INT },
            { "ISP0_AutoDetect", "", EN_ISP_CFG_PARAM_TYPE_BOOL },
            { "ISP1_AutoDetect", "", EN_ISP_CFG_PARAM_TYPE_BOOL },
        },
    };
    ST_ISP_PARAMS stIspCtrlParams = {
        { "", "" },
        { -1, -1, -1 },
        { -1, -1 },
        STF_ISP_CTRL_CFG_DEF,
        { -1, -1 },
        { -1, -1 },
        { STF_FALSE, STF_FALSE },
    };
    STF_INT nSensor = -1;
    STF_INT nSensorInterface = -1;
    //-------------------------------------------------------------------------
    STF_RESULT Ret = STF_SUCCESS;
    //-------------------------------------------------------------------------

    EnableRawMode(&stOriginalTerminalAttribute);

    g_stIspMainThreadData.bExit = STF_FALSE;
    g_stIspMainThreadData.bCapture = STF_FALSE;
    g_stIspMainThreadData.enIspMainStep = EN_ISP_MAIN_STEP_NONE;
    pthread_mutex_init(&(g_stIspMainThreadData.lock), NULL);

    if (STF_SUCCESS!= InstallSIGINTHandler()) {
        LOG_ERROR("Exit StarFive ISP control program!\n");
        return EXIT_FAILURE;
    }

    STF_MEMSET(&stIspDev[0], 0x00, sizeof(stIspDev));

    // Configure ISP/Sensor interface/Sensor select.
    //=========================================================================
    V_LOG_DEBUG("Configure ISP/Sensor interface/Sensor select.\n");
    //-------------------------------------------------------------------------
    STFLIB_ISP_CFG_ParseConfig(
        stIspCtrlParams.szConfigFile,
        &stIspCtrlCfgParams,
        &stIspCtrlParams
        );
    //-------------------------------------------------------------------------
    ParseOptions(argc, argv, &stIspCtrlParams, &szConfigFile[0]);
    if (strlen(szConfigFile)) {
        strncpy(
            stIspCtrlParams.szConfigFile,
            szConfigFile,
            STF_ISP_PARAM_LEN
            );
        STFLIB_ISP_CFG_ParseConfig(
            stIspCtrlParams.szConfigFile,
            &stIspCtrlCfgParams,
            &stIspCtrlParams
            );
    }
    //-------------------------------------------------------------------------
    if ((0 == strlen(stIspCtrlParams.szSensor[0]))
        && (0 == strlen(stIspCtrlParams.szSensor[1]))) {
        strncpy(
            stIspCtrlParams.szSensor[0],
            STF_ISP_CTRL_SENSOR_DEF,
            STF_ISP_PARAM_LEN
            );
    }
    if ((-1 == stIspCtrlParams.nSensorInterface[0])
        && (-1 == stIspCtrlParams.nSensorInterface[1])
        && (-1 == stIspCtrlParams.nSensorInterface[2])
        ) {
        if (strlen(stIspCtrlParams.szSensor[0])) {
            for (nIdx = 0; nIdx < EN_SENSOR_ID_MAX; nIdx++) {
                if (0 == strcmp(
                    g_szSENSOR_NAME[nIdx],
                    stIspCtrlParams.szSensor[0]
                    )) {
                    stIspCtrlParams.nSensorInterface[STF_ISP_CTRL_SENSOR_IF_DEF] =
                        0;
                    break;
                }
            }
        } else if (strlen(stIspCtrlParams.szSensor[1])) {
            for (nIdx = 0; nIdx < EN_SENSOR_ID_MAX; nIdx++) {
                if (0 == strcmp(
                    g_szSENSOR_NAME[nIdx],
                    stIspCtrlParams.szSensor[1]
                    )) {
                    stIspCtrlParams.nSensorInterface[STF_ISP_CTRL_SENSOR_IF_DEF] =
                        1;
                    break;
                }
            }
        }
    }
    if ((-1 == stIspCtrlParams.nSensorInterface[0])
        && (-1 == stIspCtrlParams.nSensorInterface[1])
        && (-1 == stIspCtrlParams.nSensorInterface[2])
        ) {
        LOG_ERROR("No sensor assigned to the sensor interface!!!\n");
        exit(EXIT_FAILURE);
    }
    if ((-1 == stIspCtrlParams.nISP[0])
#if defined(CONFIG_STF_DUAL_ISP)
        && (-1 == stIspCtrlParams.nISP[1])
#endif //#if defined(CONFIG_STF_DUAL_ISP)
        ) {
        if (-1 != stIspCtrlParams.nSensorInterface[0]) {
            stIspCtrlParams.nISP[STF_ISP_CTRL_SENSOR_ISP_DEF] = EN_SNSR_IF_DVP;
        } else if (-1 != stIspCtrlParams.nSensorInterface[1]) {
            stIspCtrlParams.nISP[STF_ISP_CTRL_SENSOR_ISP_DEF] = EN_SNSR_IF_CSI_0;
        } else if (-1 != stIspCtrlParams.nSensorInterface[2]) {
            stIspCtrlParams.nISP[STF_ISP_CTRL_SENSOR_ISP_DEF] = EN_SNSR_IF_CSI_1;
        }
    }
    if ((-1 == stIspCtrlParams.nISP[0])
#if defined(CONFIG_STF_DUAL_ISP)
        && (-1 == stIspCtrlParams.nISP[1])
#endif //#if defined(CONFIG_STF_DUAL_ISP)
        ) {
        LOG_ERROR("No sensor interface assigned to the ISP!!!\n");
        exit(EXIT_FAILURE);
    }
    //-------------------------------------------------------------------------
#if defined(CONFIG_STF_DUAL_ISP)
    if (-1 != stIspCtrlParams.nISP[0]) {
        nIspIdx = EN_ISP_DEV_IDX_0;
    } else if (-1 != stIspCtrlParams.nISP[1]) {
        nIspIdx = EN_ISP_DEV_IDX_1;
    } else {
        nIspIdx = EN_ISP_DEV_IDX_0;
    }
#else
    nIspIdx = EN_ISP_DEV_IDX_0;
#endif //#if defined(CONFIG_STF_DUAL_ISP)
    //-------------------------------------------------------------------------
#if 0
    LOG_INFO("nIspIdx = %d\n", nIspIdx);
    DumpConfigParam(&stIspCtrlParams);
    //-------------------------------------------------------------------------
#endif

    // Generate the device table.
    V_LOG_DEBUG("Generate the device table.\n");
    if (STF_SUCCESS != (Ret = STFLIB_ISP_DEVICE_GenerateDeviceTable(
        NULL,
        NULL
        ))) {
        LOG_ERROR("Failed to generate the video device table!!!\n");
        exit(EXIT_FAILURE);
    }

    // Setup the video device links.
    {
        ST_CI_DEVICE stDevice;
        EN_DEV_ID enDevId;
        STF_INT nIdx;

        for (nIdx = 0; nIdx < EN_ISP_PORT_ID_MAX; nIdx++) {
            if (EN_ISP_PORT_ID_SC_DUMP_YHIST != nIdx) {
                continue;
            }
            STF_MEMCLR(&stDevice, sizeof(ST_CI_DEVICE));
#if defined(OLD_ISP_VIDEO_DEVICE_NO_ORDER)
            enDevId = g_nIspVideoDeviceIndex[nIdx] + nIspIdx;
#else
            enDevId = g_nIspVideoDeviceIndex[nIdx]
                + (nIspIdx * EN_ISP_PORT_IDX_MAX);
#endif //#if defined(OLD_ISP_VIDEO_DEVICE_NO_ORDER)
            //LOG_INFO("Tryinig to call device structure initialize\n");
            if (STF_SUCCESS != STFLIB_ISP_DEVICE_StructInitialize(
                &stDevice,
                enDevId,
                STF_FALSE,
                EN_MEM_TYPE_NONE,
                0,
                0
                )) {
                LOG_ERROR("Cannot open the ISP %d \'%s\' video device, "\
                    "Error: %d(0x%08X), %s\n",
                    nIspIdx,
                    g_szDeviceName[enDevId],
                    errno,
                    errno,
                    strerror(errno)
                    );
                continue;
            }

            //LOG_INFO("Tryinig to call device ProcessLinks_2() function\n");
            if (STF_SUCCESS != stDevice.ProcessLinks_2(
                &stDevice,
                stIspCtrlParams.nISP[nIspIdx]
                )) {
                LOG_ERROR("Failed to setup the ISP %d \'%s\' video device "\
                    "video device links, Error: %d(0x%08X), %s\n",
                    nIspIdx,
                    g_szDeviceName[enDevId],
                    errno,
                    errno,
                    strerror(errno)
                    );
            }

            //LOG_INFO("Tryinig to call device SetFormat() function\n");
            if (STF_SUCCESS != stDevice.SetFormat(
                &stDevice,
                CAPTURE_WIDTH,
                CAPTURE_HEIGHT,
                g_u32IspPortPixelFormatDef[nIdx]
                )) {
                LOG_ERROR("Failed to setup the ISP %d \'%s\' video device "\
                    "video device format 0x%08X(%s), Error: %d(0x%08X), %s\n",
                    nIspIdx,
                    g_szDeviceName[enDevId],
                    g_u32IspPortPixelFormatDef[nIdx],
                    g_szIspPortPixelFormatDef[nIdx],
                    errno,
                    errno,
                    strerror(errno)
                    );
            }

            //LOG_INFO("Tryinig to call device structure un-initialize\n");
            if (STF_SUCCESS != STFLIB_ISP_DEVICE_StructUninitialize(
                &stDevice
                )) {
                LOG_ERROR("Cannot close the ISP %d \'%s\' video device, "\
                    "Error: %d(0x%08X), %s\n",
                    nIspIdx,
                    g_szDeviceName[enDevId],
                    errno,
                    errno,
                    strerror(errno)
                    );
            }
        }
    }

    // Open the ISP device.
    V_LOG_DEBUG("Open the ISP device %d.\n", nIspIdx);
    if (STF_SUCCESS != (Ret = STFLIB_ISP_DEVICE_StructInitialize(
        &stIspDev[nIspIdx], g_enIspDevId[nIspIdx], STF_FALSE, EN_MEM_TYPE_NONE, 0, 0))) {
        LOG_ERROR("Cannot open '%s': %d, %s\n",
            g_szDeviceName[g_enIspDevId[nIspIdx]],
            errno, strerror(errno));
        goto main_failed_close_isp_device;
    }

    if (STF_SUCCESS != (nRet = STFAPI_VDEV_StartDetectThread(
        nIspIdx))) {
        LOG_ERROR("Failed to start video device detect thread %d!\n",
            nIspIdx);
        goto main_failed_close_isp_device;
    }

    bExit = STF_FALSE;
    while (STF_FALSE == bExit) {
        pthread_mutex_lock(&(g_stVDevDetThread[nIspIdx].lock));
        u32SourceCount[nIspIdx] = g_stVDevDetThread[nIspIdx].u32SourceCount;
        pthread_mutex_unlock(&(g_stVDevDetThread[nIspIdx].lock));
        pthread_mutex_lock(&(g_stIspMainThreadData.lock));
        bExit = g_stIspMainThreadData.bExit;
        pthread_mutex_unlock(&(g_stIspMainThreadData.lock));
        if (STF_FALSE == bExit) {
            if ((u32SourceCount[nIspIdx]) || (bStart)) {
                if (bStart) {
                    //nRet = Run(argc, argv);
                } else {
                    //---------------------------------------------------------
                    // Re-Generate the device table.
                    if (STF_SUCCESS != (Ret = STFLIB_ISP_DEVICE_GenerateDeviceTable(
                        NULL, NULL))) {
                        LOG_ERROR("Failed to generate the device table!!!\n");
                        //exit(EXIT_FAILURE);
                        goto main_failed_close_isp_device;
                    }
                    //---------------------------------------------------------
                    nSensorInterface = stIspCtrlParams.nISP[nIspIdx];
                    nSensor = stIspCtrlParams.nSensorInterface[nSensorInterface];
                    //---------------------------------------------------------
                    nRet = IspCtrl(nIspIdx, stIspCtrlParams.szSensor[nSensor]);
                    //---------------------------------------------------------
                }
            } else {
#if 0
                fprintf(stderr, ".");
                usleep(5 * 1000);
#else
                usleep(100);
#endif //#if 0
            }
        }
    }

main_failed_close_isp_device:
    for (nIdx = EN_ISP_DEV_IDX_0; EN_ISP_DEV_IDX_MAX > nIdx; nIdx++) {
        if (-1 != g_VideoDetectThreadId[nIdx]) {
            STFAPI_VDEV_StopDetectThread(nIdx);
        }
    }
    for (nIdx = 0; nIdx < EN_ISP_DEV_IDX_MAX; nIdx++) {
        if (stIspDev[nIdx].pstCiConnection) {
            V_LOG_DEBUG("Close the ISP device %d!\n", nIdx);
            if (STF_SUCCESS != (Ret = STFLIB_ISP_DEVICE_StructUninitialize(
                &stIspDev[nIdx]))) {
                LOG_ERROR("Cannot close '%s': %d, %s\n",
                    g_szDeviceName[g_enIspDevId[nIdx]],
                    errno, strerror(errno));
            }
        }
    }
main_failed:
    pthread_mutex_destroy(&(g_stIspMainThreadData.lock));

    DisableRawMode(&stOriginalTerminalAttribute);

    return EXIT_SUCCESS;
}
