/**
  ******************************************************************************
  * @file sensorapi.c
  * @author  StarFive Isp Team
  * @version  V1.0
  * @date  08/12/2020
  * @brief Implementation of platform sensor integration file
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


#include "stf_include.h"
#include "sensorapi/sensorapi.h"

#if defined(SENSOR_SC2235_DVP)
#include "sensors/sc2235dvp.h"
#endif //#if defined(SENSOR_SC2235_DVP)
#if defined(SENSOR_OV4689_MIPI)
#include "sensors/ov4689mipi.h"
#endif
#if defined(SENSOR_IMX219_MIPI)
#include "sensors/imx219mipi.h"
#endif

#define LOG_TAG "SENSOR"
#include "stf_common/userlog.h"


STF_RESULT (*InitialiseSensors[])(
#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
    CI_CONNECTION *pCIConnection,
    STF_U8 u8IspIdx,
#endif //#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
    SENSOR_HANDLE **ppstHandle,
    STF_U8 u8Index
    ) = {
#if defined(SENSOR_SC2235_DVP)
	SC2235DVP_Create,
#endif //#if defined(SENSOR_SC2235_DVP)
#if defined(SENSOR_OV4689_MIPI)
	OV4689MIPI_Create,
#endif
#if defined(SENSOR_IMX219_MIPI)
	IMX219MIPI_Create,
#endif
};

const char *Sensors[]={
#if defined(SENSOR_SC2235_DVP)
	SC2235DVP_SENSOR_INFO_NAME,
#endif //#if defined(SENSOR_SC2235_DVP)
#if defined(SENSOR_OV4689_MIPI)
	OV4689MIPI_SENSOR_INFO_NAME,
#endif
#if defined(SENSOR_IMX219_MIPI)
	IMX219MIPI_SENSOR_INFO_NAME,
#endif
	NULL,
};


/*
 * Sensor controls
 */

const STF_CHAR **Sensor_ListAll(
    STF_VOID
    )
{
	return Sensors;
}

STF_U8 Sensor_NumberSensors(
    STF_VOID
    )
{
    return (sizeof(InitialiseSensors) / sizeof(InitialiseSensors[0]));
}

STF_VOID Sensor_PrintAllModes(
#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
    CI_CONNECTION *pCIConnection,
    STF_U8 u8IspIdx,
#endif //#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
    FILE *f
    )
{
    const char **pszSensors = Sensor_ListAll();
    int s = 0;
    int ret;

    fprintf(f, "Available sensors in sensor API:\n");
    while (pszSensors[s]) {
        SENSOR_HANDLE *pstSensorHandle = NULL;

        if (STF_SUCCESS == Sensor_Initialise(
#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
            pCIConnection,
            u8IspIdx,
#endif //#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
            s,
            &pstSensorHandle,
            0
            )) {
            SENSOR_MODE mode = { 0, };
            SENSOR_INFO info = { 0, };
            int m = 0;

            ret = Sensor_GetInfo(pstSensorHandle, &info);

            fprintf(f, "\t%d: %s (v%s imager %d)\n",
                s,
                pszSensors[s],
                info.pszSensorVersion,
                (int)info.u8Imager
                );

            while (STF_SUCCESS == Sensor_GetMode(pstSensorHandle, m, &mode)) {
                STF_DOUBLE dRate = mode.dPixelRate * mode.u8BitDepth;
                char c = ' ';

                fprintf(f, "\t\tmode %2d: %5dx%5d @%.2f %ubit (total %dx%d "\
                    "mipi_lane=%u) exposure=(%u..%u) %s\n",
                    m,
                    mode.u16Width,
                    mode.u16Height,
                    mode.dFrameRate,
                    mode.u8BitDepth,
                    mode.u16HorizontalTotal,
                    mode.u16VerticalTotal,
                    mode.u8MipiLanes,
                    mode.u32ExposureMin,
                    mode.u32ExposureMax,
                    mode.u8SupportFlipping == SENSOR_FLIP_NONE
                        ? "flipping=none" :
                        (mode.u8SupportFlipping == SENSOR_FLIP_BOTH
                            ? "flipping=horizontal|vertical" :
                            (mode.u8SupportFlipping == SENSOR_FLIP_HORIZONTAL
                                ? "flipping=horizontal" : "flipping=vertical"
                            )
                        )
                    );
                fprintf(f, "\t\t         pixel rate %.4lf Mpx/s, ",
                    mode.dPixelRate
                    );
                if (1000000000 <= dRate) {
                    c = 'G';
                    dRate = dRate / 1000.0;
                } else {
                    c = 'M';
                }
                if (mode.u8MipiLanes > 0) {
                    fprintf(f, "bit rate %.4lf %cbits/s (per mipi lane)\n",
                        (dRate / mode.u8MipiLanes),
                        c
                        );
                } else {
                    fprintf(f, "bit rate %.4lf %cbits/s\n",
                        dRate,
                        c
                        );
                }
                m++;
            }
#ifdef SENSOR_AR330
            if (0 == strncmp(pszSensors[s], AR330_SENSOR_INFO_NAME,
                strlen(AR330_SENSOR_INFO_NAME))) {
                fprintf(f, "\t\tmode %d: special register load from file\n",
                    AR330_SPECIAL_MODE
                    );
            }
#endif
            Sensor_Destroy(pstSensorHandle);
        } else {
            printf("\t%d: %s - no modes display available\n",
                s,
                pszSensors[s]
                );
        }

        s++;
    }
}

STF_RESULT Sensor_Initialise(
#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
    CI_CONNECTION *pCIConnection,
    STF_U8 u8IspIdx,
#endif //#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
    STF_U16 u16Sensor,
    SENSOR_HANDLE **ppstHandle,
    STF_U8 u8Index
    )
{

    if (!ppstHandle) {
        LOG_ERROR("ppstHandle is NULL\n");
        return STF_ERROR_INVALID_PARAMETERS;
    }
    if (u16Sensor
        < (sizeof(InitialiseSensors) / sizeof(InitialiseSensors[0]))) {
        return InitialiseSensors[u16Sensor](
#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
            pCIConnection,
            u8IspIdx,
#endif //#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
            ppstHandle,
            u8Index
            );
    }

    LOG_ERROR("nSensor=%d supports %d sensors\n",
        u16Sensor,
        (sizeof(InitialiseSensors) / sizeof(InitialiseSensors[0]))
        );

    return STF_ERROR_INVALID_PARAMETERS;
}

STF_S8 Sensor_GetModeIdx(
    SENSOR_HANDLE *pstHandle,
    STF_U16 u16Width,
    STF_U16 u16Height
    )
{

    /* Check handle */
    if (!pstHandle) {
        LOG_ERROR("pstHandle is NULL\n");
        return -1;
    }

    /* Check function exists */
    if (pstHandle->GetModeIdx) {
        return (pstHandle->GetModeIdx)(pstHandle, u16Width, u16Height);

    }
    LOG_ERROR("GetModeIdx is not defined!\n");

    return -1;
}

STF_RESULT Sensor_GetMode(
    SENSOR_HANDLE *pstHandle,
    STF_U8 u8ModeIdx,
    SENSOR_MODE *pstModes
    )
{

    /* Check handle */
    if (!pstHandle) {
        LOG_ERROR("pstHandle is NULL\n");
        return STF_ERROR_INVALID_PARAMETERS;
    }

    /* Check parameters */
    if (!pstModes) {
        LOG_ERROR("psModes is NULL\n");
        return STF_ERROR_INVALID_PARAMETERS;
    }

    /* Check function exists */
    if (pstHandle->GetMode) {
        return (pstHandle->GetMode)(pstHandle, u8ModeIdx, pstModes);

    }
    LOG_ERROR("GetMode is not defined!\n");

    return STF_ERROR_FATAL;
}

STF_RESULT Sensor_GetState(
    SENSOR_HANDLE *pstHandle,
    SENSOR_STATUS *pstStatus
    )
{

    /* Check handle */
    if (!pstHandle) {
        LOG_ERROR("pstHandle is NULL\n");
        return STF_ERROR_INVALID_PARAMETERS;
    }

    /* Check parameters */
    if (!pstStatus) {
        LOG_ERROR("pstStatus is NULL\n");
        return STF_ERROR_INVALID_PARAMETERS;
    }

    /* Check function exists */
    if (pstHandle->GetState) {
        return (pstHandle->GetState)(pstHandle, pstStatus);
    }
    LOG_ERROR("GetState is not defined!\n");

    return STF_ERROR_FATAL;
}

STF_RESULT Sensor_GetInterfaceInfo(
    SENSOR_HANDLE *pstHandle,
    SENSOR_INTFC *pstInterface
    )
{

    /* Check handle */
    if (!pstHandle) {
        LOG_ERROR("pstHandle is NULL\n");
        return STF_ERROR_INVALID_PARAMETERS;
    }

    /* Check function exists */
    if (pstHandle->GetInterfaceInfo) {
        return (pstHandle->GetInterfaceInfo)(pstHandle, pstInterface);
    }
    LOG_ERROR("GetInterfaceInfo is not defined!\n");

    return STF_ERROR_FATAL;
}

STF_RESULT Sensor_SetMode(
    SENSOR_HANDLE *pstHandle,
    STF_U8 u8ModeIdx,
    STF_U8 u8Flipping
    )
{

    /* Check handle */
    if (!pstHandle) {
        LOG_ERROR("pstHandle is NULL\n");
        return STF_ERROR_INVALID_PARAMETERS;
    }

    /* Check function exists */
    if (pstHandle->SetMode) {
        return (pstHandle->SetMode)(pstHandle, u8ModeIdx, u8Flipping);
    }
    LOG_ERROR("SetMode is not defined!\n");

    return STF_ERROR_FATAL;
}

STF_RESULT Sensor_Enable(
    SENSOR_HANDLE *pstHandle
    )
{

    /* Check handle */
    if (!pstHandle) {
        LOG_ERROR("pstHandle is NULL\n");
        return STF_ERROR_INVALID_PARAMETERS;
    }

    /* Check function exists */
    if (pstHandle->Enable) {
        return (pstHandle->Enable)(pstHandle);
    }
    LOG_ERROR("Enable is not defined!\n");

    return STF_ERROR_FATAL;
}

STF_RESULT Sensor_Disable(
    SENSOR_HANDLE *pstHandle
    )
{

    /* Check handle */
    if (!pstHandle) {
        LOG_ERROR("pstHandle is NULL\n");
        return STF_ERROR_INVALID_PARAMETERS;
    }

    /* Check function exists */
    if (pstHandle->Disable) {
        return (pstHandle->Disable)(pstHandle);
    }
    LOG_ERROR("Disable is not defined!\n");

    return STF_ERROR_FATAL;
}

STF_RESULT Sensor_Destroy(
    SENSOR_HANDLE *pstHandle
    )
{

    /* Check handle */
    if (!pstHandle) {
        LOG_ERROR("pstHandle is NULL\n");
        return STF_ERROR_INVALID_PARAMETERS;
    }

    /* Check function exists */
    if (pstHandle->Destroy) {
        return (pstHandle->Destroy)(pstHandle);
    }
    LOG_ERROR("Destroy is not defined!\n");

    return STF_ERROR_FATAL;
}

STF_RESULT Sensor_GetInfo(
    SENSOR_HANDLE *pstHandle,
    SENSOR_INFO *pstInfo
    )
{

    /* Check handle */
    if (!pstHandle) {
        LOG_ERROR("pstHandle is NULL\n");
        return STF_ERROR_INVALID_PARAMETERS;
    }

    /* Check parameters */
    if (!pstInfo) {
        LOG_ERROR("pstInfo is NULL\n");
        return STF_ERROR_INVALID_PARAMETERS;
    }

    /* Check function exists */
    if (pstHandle->GetInfo) {
        STF_RESULT Ret;

        Ret = Sensor_GetState(pstHandle, &(pstInfo->stStatus));
        if (Ret) {
            LOG_ERROR("failed to get sensor's state\n");
            return Ret;
        }

        Ret = (pstHandle->GetInfo)(pstHandle, pstInfo);
        if (Ret) {
            LOG_ERROR("failed to get sensor's info\n");
            return Ret;
        } else {
            LOG_DEBUG("original_mosaic=%d flip=0x%x enabled_mosaic=%d\n",
                (int)pstInfo->enBayerOriginal,
                pstInfo->stStatus.u8Flipping,
                pstInfo->enBayerEnabled);
        }

        Ret = Sensor_GetMode(
            pstHandle,
            pstInfo->stStatus.u8CurrentMode,
            &(pstInfo->stMode)
            );
        if (Ret) {
            LOG_ERROR("failed to get sensor's mode\n");
            return Ret;
        }

        return STF_SUCCESS;
    }
    LOG_ERROR("GetInfo is not defined!\n");

    return STF_ERROR_FATAL;
}

/*
 * Register access controls
 */

STF_RESULT Sensor_GetRegister(
    SENSOR_HANDLE *pstHandle,
    STF_U16 u16RegAddr,
    STF_U16 *pu16RegVal,
    STF_U8 u8Context
    )
{

    /* Check handle */
    if (!pstHandle) {
        LOG_ERROR("pstHandle is NULL\n");
        return STF_ERROR_INVALID_PARAMETERS;
    }

    /* Check parameters */
    if (!pu16RegVal) {
        LOG_ERROR("pu16RegVal is NULL\n");
        return STF_ERROR_INVALID_PARAMETERS;
    }

    /* Check function exists */
    if (pstHandle->GetRegister) {
        return (pstHandle->GetRegister)(pstHandle, u16RegAddr, pu16RegVal, u8Context);
    }
    LOG_ERROR("GetRegister is not defined!\n");

    return STF_ERROR_FATAL;
}

STF_RESULT Sensor_SetRegister(
    SENSOR_HANDLE *pstHandle,
    STF_U16 u16RegAddr,
    STF_U16 u16RegVal,
    STF_U8 u8Context
    )
{

    /* Check handle */
    if (!pstHandle) {
        LOG_ERROR("pstHandle is NULL\n");
        return STF_ERROR_INVALID_PARAMETERS;
    }

    /* Check function exists */
    if (pstHandle->SetRegister) {
        return (pstHandle->SetRegister)(pstHandle, u16RegAddr, u16RegVal, u8Context);
    }
    LOG_ERROR("SetRegister is not defined!\n");

    return STF_ERROR_FATAL;
}

/*
 * Exposure and gain controls
 */

STF_RESULT Sensor_GetGainRange(
    SENSOR_HANDLE *pstHandle,
    STF_DOUBLE *pdMin,
    STF_DOUBLE *pdMax,
    STF_U8 *pu8Contexts
    )
{

    /* Check handle */
    if (!pstHandle) {
        LOG_ERROR("pstHandle is NULL\n");
        return STF_ERROR_INVALID_PARAMETERS;
    }

    /* Check parameters */
    if (!pdMin || !pdMax || !pu8Contexts) {
        LOG_ERROR("pdMin, pdMax or pu8Contexts is NULL\n");
        return STF_ERROR_INVALID_PARAMETERS;
    }

    /* Check function exists */
    if (pstHandle->GetGainRange) {
        return (pstHandle->GetGainRange)(pstHandle, pdMin, pdMax, pu8Contexts);
    }
    LOG_ERROR("GetGainRange is not defined!\n");

    return STF_ERROR_FATAL;
}

STF_RESULT Sensor_GetCurrentGain(
    SENSOR_HANDLE *pstHandle,
    STF_DOUBLE *pdCurrentGain,
    STF_U8 u8Context
    )
{

    /* Check handle */
    if (!pstHandle) {
        LOG_ERROR("pstHandle is NULL\n");
        return STF_ERROR_INVALID_PARAMETERS;
    }

    /* Check parameters */
    if (!pdCurrentGain) {
        LOG_ERROR("pdCurrentGain is NULL\n");
        return STF_ERROR_INVALID_PARAMETERS;
    }

    /* Check function exists */
    if (pstHandle->GetCurrentGain) {
        return (pstHandle->GetCurrentGain)(pstHandle, pdCurrentGain, u8Context);
    }
    LOG_ERROR("GetCurrentGain is not defined!\n");

    return STF_ERROR_FATAL;
}

STF_RESULT Sensor_SetGain(
    SENSOR_HANDLE *pstHandle,
    STF_DOUBLE dGain,
    STF_U8 u8Context
    )
{

    /* Check handle */
    if (!pstHandle) {
        LOG_ERROR("pstHandle is NULL\n");
        return STF_ERROR_INVALID_PARAMETERS;
    }

    /* could also check that gain fits in min/max for the mode */

    /* Check function exists */
    if (pstHandle->SetGain) {
        return (pstHandle->SetGain)(pstHandle, dGain, u8Context);
    }
    LOG_ERROR("SetGain is not defined!\n");

    return STF_ERROR_FATAL;
}

STF_RESULT Sensor_GetExposureRange(
    SENSOR_HANDLE *pstHandle,
    STF_U32 *pu32Min,
    STF_U32 *pu32Max,
    STF_U8 *pu8Contexts
    )
{

    /* Check handle */
    if (!pstHandle) {
        LOG_ERROR("pstHandle is NULL\n");
        return STF_ERROR_INVALID_PARAMETERS;
    }

    /* Check parameters */
    if (!pu32Min || !pu32Max || !pu8Contexts) {
        LOG_ERROR("pu32Min, pu32Max or pu8Contexts is NULL\n");
        return STF_ERROR_INVALID_PARAMETERS;
    }

    /* Check function exists */
    if (pstHandle->GetExposureRange) {
        return (pstHandle->GetExposureRange)(
            pstHandle, pu32Min, pu32Max, pu8Contexts);
    }
    LOG_ERROR("GetExposureRange is not defined!\n");

    return STF_ERROR_FATAL;
}

STF_RESULT Sensor_GetExposure(
    SENSOR_HANDLE *pstHandle,
    STF_U32 *pu32Exposure,
    STF_U8 u8Context
    )
{

    /* Check handle */
    if (!pstHandle) {
        LOG_ERROR("pstHandle is NULL\n");
        return STF_ERROR_INVALID_PARAMETERS;
    }

    /* Check parameters */
    if (!pu32Exposure) {
        LOG_ERROR("pu32Exposure is NULL\n");
        return STF_ERROR_INVALID_PARAMETERS;
    }

    /* Check function exists */
    if (pstHandle->GetExposure) {
        return (pstHandle->GetExposure)(pstHandle, pu32Exposure, u8Context);
    }
    LOG_ERROR("GetExposure is not defined!\n");

    return STF_ERROR_FATAL;
}

STF_RESULT Sensor_SetExposure(
    SENSOR_HANDLE *pstHandle,
    STF_U32 u32Exposure,
    STF_U8 u8Context
    )
{

    /* Check handle */
    if (!pstHandle) {
        LOG_ERROR("pstHandle is NULL\n");
        return STF_ERROR_INVALID_PARAMETERS;
    }

    /* could also check that given exposure fits in min/max for the mode */

    /* Check function exists */
    if (pstHandle->SetExposure) {
        return (pstHandle->SetExposure)(pstHandle, u32Exposure, u8Context);
    }
    LOG_ERROR("SetExposure is not defined!\n");

    return STF_ERROR_FATAL;
}

/*
 * Focus control
 */

STF_RESULT Sensor_GetFocusRange(
    SENSOR_HANDLE *pstHandle,
    STF_U16 *pu16Min,
    STF_U16 *pu16Max
    )
{

    /* Check handle */
    if (!pstHandle) {
        LOG_ERROR("pstHandle is NULL\n");
        return STF_ERROR_INVALID_PARAMETERS;
    }

    /* Check parameters */
    if (!pu16Min || !pu16Max) {
        LOG_ERROR("pu16Min or pu16Max is NULL\n");
        return STF_ERROR_INVALID_PARAMETERS;
    }

    /* Check function exists - optional implementation */
    if (pstHandle->GetFocusRange) {
        return (pstHandle->GetFocusRange)(pstHandle, pu16Min, pu16Max);
    }

    return STF_ERROR_NOT_SUPPORTED;
}


STF_RESULT Sensor_GetCurrentFocus(
    SENSOR_HANDLE *pstHandle,
    STF_U16 *pu16CurrentFocus
    )
{

    /* Check handle */
    if (!pstHandle) {
        LOG_ERROR("pstHandle is NULL\n");
        return STF_ERROR_INVALID_PARAMETERS;
    }

    /* Check parameters */
    if (!pu16CurrentFocus) {
        LOG_ERROR("pu16CurrentFocus is NULL\n");
        return STF_ERROR_INVALID_PARAMETERS;
    }

    /* Check function exists - optional implementation */
    if (pstHandle->GetCurrentFocus) {
        return (pstHandle->GetCurrentFocus)(pstHandle, pu16CurrentFocus);
    }

    return STF_ERROR_NOT_SUPPORTED;
}


STF_RESULT Sensor_SetFocus(
    SENSOR_HANDLE *pstHandle,
    STF_U16 u16Focus
    )
{

    /* Check handle */
    if (!pstHandle) {
        LOG_ERROR("pstHandle is NULL\n");
        return STF_ERROR_INVALID_PARAMETERS;
    }

    /* could also check that focus fits in min/max for the mode */

    /* Check function exists - optional implementation */
    if (pstHandle->SetFocus) {
        return (pstHandle->SetFocus)(pstHandle, u16Focus);
    }

    return STF_ERROR_NOT_SUPPORTED;
}

/*
 * miscellaneous controls
 */

STF_RESULT Sensor_ConfigureFlash(
    SENSOR_HANDLE *pstHandle,
    STF_BOOL8 bAlwaysOn,
    STF_S16 s16FrameDelay,
    STF_S16 s16Frames,
    STF_U16 u16FlashPulseWidth
    )
{

    /* Check handle */
    if (!pstHandle) {
        LOG_ERROR("pstHandle is NULL\n");
        return STF_ERROR_INVALID_PARAMETERS;
    }

    /* Check function exists - optional implementation */
    if (pstHandle->ConfigureFlash) {
        return (pstHandle->ConfigureFlash)(
            pstHandle, bAlwaysOn, s16FrameDelay, s16Frames, u16FlashPulseWidth);
    }

    return STF_ERROR_NOT_SUPPORTED;
}

STF_RESULT Sensor_Insert(
    SENSOR_HANDLE *pstHandle
    )
{

    /* Check handle */
    if (!pstHandle) {
        LOG_ERROR("pstHandle is NULL\n");
        return STF_ERROR_INVALID_PARAMETERS;
    }

    /* Check function exists - optional implementation */
    if (pstHandle->Insert) {
        return (pstHandle->Insert)(pstHandle);
    }

    return STF_ERROR_NOT_SUPPORTED;
}

STF_RESULT Sensor_WaitProcessed(
    SENSOR_HANDLE *pstHandle
    )
{

    /* Check handle */
    if (!pstHandle) {
        LOG_ERROR("pstHandle is NULL\n");
        return STF_ERROR_INVALID_PARAMETERS;
    }

    /* Check function exists - optional implementation */
    if (pstHandle->WaitProcessed) {
        return (pstHandle->WaitProcessed)(pstHandle);
    }

    return STF_ERROR_NOT_SUPPORTED;
}

STF_RESULT Sensor_SetFlipMirror(
    SENSOR_HANDLE *pstHandle,
    STF_U8 u8Flag
    )
{

    /* Check handle */
    if (!pstHandle) {
        LOG_ERROR("pstHandle is NULL\n");
        return STF_ERROR_INVALID_PARAMETERS;
    }

    /* Check function exists - optional implementation */
    if (pstHandle->SetFlipMirror) {
        return (pstHandle->SetFlipMirror)(pstHandle, u8Flag);
    }

    return STF_ERROR_NOT_SUPPORTED;
}

STF_RESULT Sensor_GetFixedFPS(
    SENSOR_HANDLE *pstHandle,
    STF_U16 *pu16FixedFps
    )
{

	if (!pstHandle) {
		LOG_ERROR("pstHandle is NULL\n");
		return STF_ERROR_INVALID_PARAMETERS;
	}

	/* Check function exists - optional implementation */
	if (pstHandle->GetFixedFPS) {
		return (pstHandle->GetFixedFPS)(pstHandle, pu16FixedFps);
	}

	return STF_ERROR_NOT_SUPPORTED;
}

STF_RESULT Sensor_SetFPS(
    SENSOR_HANDLE *pstHandle,
    STF_DOUBLE dFps
    )
{

	 /* Check handle */
	if (!pstHandle) {
		LOG_ERROR("pstHandle is NULL\n");
		return STF_ERROR_INVALID_PARAMETERS;
	}

	/* Check function exists - optional implementation */
	if (pstHandle->SetFPS) {
		return (pstHandle->SetFPS)(pstHandle, dFps);
	}

	return STF_ERROR_NOT_SUPPORTED;
}

STF_RESULT Sensor_SetExposureAndGain(
    SENSOR_HANDLE *pstHandle,
    STF_U32 u32Exposure,
    STF_DOUBLE dGain,
    STF_U8 u8Context
    )
{

    /* Check handle */
    if (!pstHandle) {
        LOG_ERROR("pstHandle is NULL\n");
        return STF_ERROR_INVALID_PARAMETERS;
    }

    /* Check function exists - optional implementation */
    if (pstHandle->SetExposureAndGain) {
        return (pstHandle->SetExposureAndGain)(
            pstHandle, u32Exposure, dGain, u8Context);
    }

    return STF_ERROR_NOT_SUPPORTED;
}

STF_RESULT Sensor_SetResolution(
    SENSOR_HANDLE *pstHandle,
    STF_U16 u16ImgW,
    STF_U16 U16ImgH
    )
{

	if (!pstHandle) {
		LOG_ERROR("pstHandle is NULL\n");
		return STF_ERROR_INVALID_PARAMETERS;
	}

	/* Check function exists - optional implementation */
	if (pstHandle->SetResolution) {
		return (pstHandle->SetResolution)(pstHandle, u16ImgW, U16ImgH);
	}

	return STF_ERROR_NOT_SUPPORTED;
}

STF_RESULT Sensor_GetSnapRes(
    SENSOR_HANDLE *pstHandle,
    ST_RES_LIST *pstReslist
    )
{

	if (!pstHandle) {
		LOG_ERROR("pstHandle is NULL\n");
		return STF_ERROR_INVALID_PARAMETERS;
	}

	if (pstHandle->GetSnapRes) {
		return (pstHandle->GetSnapRes)(pstHandle, pstReslist);
	}

	return STF_ERROR_NOT_SUPPORTED;
}

STF_RESULT Sensor_Reset(
    SENSOR_HANDLE *pstHandle
    )
{

    /* Check handle */
    if (!pstHandle) {
        LOG_ERROR("pstHandle is NULL\n");
        return STF_ERROR_INVALID_PARAMETERS;
    }

    /* Check function exists */
    if (pstHandle->Reset) {
        return (pstHandle->Reset)(pstHandle);
    }
    LOG_ERROR(" Sensor Reset is not defined!\n");

    return STF_ERROR_FATAL;
}
