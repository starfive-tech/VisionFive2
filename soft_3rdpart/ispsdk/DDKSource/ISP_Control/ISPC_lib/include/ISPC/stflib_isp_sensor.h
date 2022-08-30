/**
  ******************************************************************************
  * @file stflib_isp_sensor.h
  * @author  StarFive Isp Team
  * @version  V1.0
  * @date  08/26/2020
  * @brief Declaration of ISPC::Sensor
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
  ******************************************************************************/


#ifndef __STFLIB_ISP_SENSOR_H__
#define __STFLIB_ISP_SENSOR_H__


#ifdef __cplusplus
extern "C" {
#endif


#include "stf_include.h"
#include "ci/ci_api_structs.h"

#include "stf_common/pixel_format.h"
#include "sensorapi/sensorapi.h"
#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
#include "ISPC/stflib_isp_device.h"
#endif //#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)


typedef enum _EN_SENSOR_STATE {
    SENSOR_ERROR = 0,                           /**< @brief Sensor is in error estate. */
    SENSOR_INITIALIZED,                         /**< @brief Sensor initialization is complete. */
    SENSOR_ENABLED,                             /**< @brief Sensor is enabled for capture. */
    SENSOR_CONFIGURED,                          /**< @brief Sensor is configured. */
} EN_SENSOR_STATE, *PEN_SENSOR_STATE;


typedef struct _ST_SENSOR ST_SENSOR;


typedef struct _ST_SENSOR {
    SENSOR_HANDLE *pstSensorHandle;             /** @brief Sensor handle that controls the camera */
    EN_SENSOR_STATE enState;                    /** @brief Sensor running state */

    STF_U16 u16Width;                           /** @brief in pixels, Range: 0 - 16384. */
    STF_U16 u16Height;                          /** @brief in lines, Range: 0 - 16384. */
    STF_U8 u8Imager;                            /** @brief imager id */
    STF_U16 u16VerticalTotal;                   /** @brief Number of lines including blanking, Range: 0 - 16383 */
    enum MOSAICType enBayerFormat;              /** @brief Mosaic of the sensor */
    STF_U8 u8SensorContexts;                    /** @brief contexts */
    STF_U8 u8BitDepth;                          /** @brief Data size of the sensor, Range: 8 - 16. */
    STF_DOUBLE dFrameRate;                      /** @brief Sensor default frame rate, Range: 1.0 - 255.0. */
    STF_DOUBLE dCurrentFPS;                     /** @brief Frame rate programmed in the sensor, Range: 1.0 - 255.0. */
    STF_U32 u32WellDepth;                       /** Range: 0 - 65535 */
    STF_DOUBLE dReadNoise;                      /** Range: 0.0 - 100.0 */
    STF_DOUBLE dAperture;
    STF_U16 u16FocalLength;

    STF_U32 u32ProgrammedExposure;              /** @brief Exposure time programmed in the sensor (microseconds), Range: 0.0 - 5000.0. */
    STF_DOUBLE dProgrammedGain;                 /** @brief Gain programmed in the sensor, Range: 0.0 - 128.0. */
    STF_U16 u16ProgrammedFocus;                 /** @brief Focus distance set in the sensor */
    STF_U32 u32MinExposure;                     /** @brief Minumum exposure times (microseconds) */
    STF_U32 u32MaxExposure;                     /** @brief Maximum exposure times (microseconds) */
    STF_U16 u16MinFocus;                        /** @brief Minimum focus distance (milimetre) */
    STF_U16 u16MaxFocus;                        /** @brief Maximum focus distance (milimetre) */
    STF_DOUBLE dMinGain;                        /** @brief Minimum exposure gain */
    STF_DOUBLE dMaxGain;                        /** @brief Maximum exposure gain */
    STF_BOOL8 bFocusSupported;                  /** @brief variable focus support status */

    /**
     * @brief Get sensor state name
     *
     * @param s Sensor state
     */
    const STF_CHAR* (*StateName)(EN_SENSOR_STATE enState);

    /**
     * @brief Populate a list with the available sensors from Sensor API
     *
     * @param [out] sensorNames appends name to the list
     */
    STF_VOID (*GetSensorNames)(STF_U8 *pu8Count, STF_CHAR szSensorName[][64]);

    /**
     * @brief Get sensor ID from a name
     *
     * @param name of the sensor in Sensor API
     *
     * @return positive identifier or negative number if sensor not found
     */
    STF_S8 (*GetSensorId)(const char *SensorName);

    /**
     * @brief to centralise the init of the constructor so that children
     * classes can initialise the object as if constructor has been called
     * with a sensor id
     */
#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
    STF_RESULT (*Init)(CI_CONNECTION *pCIConnection, STF_U8 u8IspIdx, ST_SENSOR *pstSensor, STF_U8 u8SensorId, STF_U8 u8Index);
#else
    STF_RESULT (*Init)(ST_SENSOR *pstSensor, STF_U8 u8SensorId, STF_U8 u8Index);
#endif //#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)

    /**
     * @brief Destroy a sensor using given identifier.
     */
    STF_RESULT (*Destroy)(ST_SENSOR *pstSensor);

    /**
     * @brief Access to sensors API handle
     *
     * Mainly to be able to call extended functions
     */
    SENSOR_HANDLE *(*GetHandle)(ST_SENSOR *pstSensor);

    /**
     * @brief Get the sensor state.
     */
    EN_SENSOR_STATE (*GetState)(ST_SENSOR *pstSensor);

    /**
     * @brief Get sensor information
     *
     * @param[out] sMode
     */
    STF_RESULT (*GetInfo)(ST_SENSOR *pstSensor, SENSOR_INFO *pstSensorInfo);

    /**
     * @brief Get sensor mode index
     *
     * @param u16Width the width of mode for the mode to check.
     * @param u16Height the height of mode for the mode to check.
     *
     * @return positive mode index or negative number if mode is not found
     */
    STF_S8 (*GetModeIdx)(ST_SENSOR *pstSensor, STF_U16 u16Width, STF_U16 u16Height);

    /**
     * @brief Get some mode information (including if flipping is supported)
     *
     * @param mode for the sensor
     * @param[out] sMode
     */
    STF_RESULT (*GetMode)(ST_SENSOR *pstSensor, STF_U8 u8ModeIdx, SENSOR_MODE *pstModes);

    /**
     * @brief Set the mode of the sensor
     */
    STF_RESULT (*SetMode)(ST_SENSOR *pstSensor, STF_U8 u8ModeIdx, STF_U8 u8Flipping);

    /**
     * @brief  Get the sensor's connection data bus and I2C hardware information.
     *
     * @param[out] pstInterface
     */
    STF_RESULT (*GetInterfaceInfo)(ST_SENSOR *pstSensor, SENSOR_INTFC *pstInterface);

    /**
     * @brief Configure the sensor with a selected mode.
     */
    STF_RESULT (*Configure)(ST_SENSOR *pstSensor, STF_U8 u8ModeIdx, STF_U8 u8Flipping);

    /**
     * @brief Access to the Sensor API insert function (which may not be
     * available on all sensors!)
     */
    STF_RESULT (*Insert)(ST_SENSOR *pstSensor);

    /**
     * @brief Access to the Sensor API waitProcessed function (which may not
     * be available on all sensors!)
     */
    STF_RESULT (*WaitProcessed)(ST_SENSOR *pstSensor);

    /**
     * @brief start transmission of the data from the sensor.
     */
    STF_RESULT (*Enable)(ST_SENSOR *pstSensor);

    /**
     * @brief stop transmission of the data from the sensor.
     */
    STF_RESULT (*Disable)(ST_SENSOR *pstSensor);

    /**
     * @brief Reset the sensor.
     */
    STF_RESULT (*Reset)(ST_SENSOR *pstSensor);

    /**
     * @brief Get status value in the sensor.
     */
    STF_RESULT (*GetStatus)(ST_SENSOR *pstSensor, SENSOR_STATUS *pstStatus);

    /**
     * @brief Get the sensor register value.
     *
     * @param u16RegAddr the sensor register address.
     * @param pu16RegValue point to read back value from the sensor.
     */
    STF_RESULT (*GetReg)(ST_SENSOR *pstSensor, STF_U16 u16RegAddr, STF_U16 *pu16RegValue);

    /**
     * @brief Program the value to the register of sensor.
     *
     * @param u16RegAddr the sensor register address.
     * @param u16RegValue the program value to the sensor register.
     */
    STF_RESULT (*SetReg)(ST_SENSOR *pstSensor, STF_U16 u16RegAddr, STF_U16 u16RegValue);

    /**
     * @brief Get flip and mirror value in the sensor.
     */
    STF_RESULT (*GetFlipMirror)(ST_SENSOR *pstSensor, STF_U8 *pu8Flag);

#ifdef SIFIVE_ISP_TUNING
    /**
     * @brief Set flip and mirror value in the sensor.
     */
    STF_RESULT (*SetFlipMirror)(ST_SENSOR *pstSensor, STF_U8 u8Flag);

#endif //SIFIVE_ISP_TUNING
    /**
     * @brief Get the fixed FPS value in the sensor.
     */
    STF_RESULT (*GetFixedFPS)(ST_SENSOR *pstSensor, STF_U16 *pu16FixedFps);

    /**
     * @brief Set the FPS value the sensor.
     */
    STF_RESULT (*SetFPS)(ST_SENSOR *pstSensor, STF_DOUBLE dFps);

    /**
     * @brief Get the snapahot resolution value in the sensor.
     */
    STF_RESULT (*GetSnapShotResolution)(ST_SENSOR *pstSensor, ST_RES_LIST *pstResList);

    /**
     * @brief Set the resolution value the sensor.
     */
    STF_RESULT (*SetResolution)(ST_SENSOR *pstSensor, STF_U16 u16ImgW, STF_U16 u16ImgH);

    /**
     * @brief Get minimum exposure time value programmable in the sensor
     * (microseconds).
     */
    STF_U32 (*GetMinExposure)(ST_SENSOR *pstSensor);

    /**
     * @brief Get maximum exposure time value programmable in the sensor
     * (microseconds).
     */
    STF_U32 (*GetMaxExposure)(ST_SENSOR *pstSensor);

    /**
     * @brief Get minimum and maximum exposure time value programmable in the sensor
     * (microseconds).
     */
    STF_RESULT (*GetExposureRange)(ST_SENSOR *pstSensor, STF_U32 *pu32MinExposure, STF_U32 *pu32MaxExposure);

    /**
     * @brief Get the exposure time programmed in the sensor (microseconds)
     */
    STF_U32 (*GetExposure)(ST_SENSOR *pstSensor);

    /**
     * @brief Program the exposure time in the sensor (microseconds).
     *
     * @param u32Exposure Exposure time (microseconds)
     */
    STF_RESULT (*SetExposure)(ST_SENSOR *pstSensor, STF_U32 u32Exposure);

    /**
     * @brief Get minimum gain value programmable in the sensor.
     */
    STF_DOUBLE (*GetMinGain)(ST_SENSOR *pstSensor);

#ifdef SIFIVE_ISP_TUNING
    /**
     * @brief Set the minimum gain value programmable in the sensor.
     */
    STF_RESULT (*SetMinGain)(ST_SENSOR *pstSensor, STF_DOUBLE dMinGain);

#endif //SIFIVE_ISP_TUNING
    /**
     * @brief Get maximum gain value programmable in the sensor.
     */
    STF_DOUBLE (*GetMaxGain)(ST_SENSOR *pstSensor);

#ifdef SIFIVE_ISP_TUNING
    /**
     * @brief Set the maximum gain value programmable in the sensor.
     */
    STF_RESULT (*SetMaxGain)(ST_SENSOR *pstSensor, STF_DOUBLE dMaxGain);

#endif //SIFIVE_ISP_TUNING
    /**
     * @brief Get minimum and maximum gain value programmable in the sensor.
     */
    STF_RESULT (*GetGainRange)(ST_SENSOR *pstSensor, STF_DOUBLE *pdMinGain, STF_DOUBLE *pdMaxGain);

    /**
     * @brief Set the minimum and maximum gain value programmable in the sensor according to the ISO value.
     */
    STF_RESULT (*SetIsoLimit)(ST_SENSOR *pstSensor, STF_U8 u8IsoLimit);

    /**
     * @brief Get the gain value programmed in the sensor.
     */
    STF_DOUBLE (*GetGain)(ST_SENSOR *pstSensor);

    /**
     * @brief Set the gain value in the sensor.
     */
    STF_RESULT (*SetGain)(ST_SENSOR *pstSensor, STF_DOUBLE dGain);

    /**
     * @brief Program the exposure time (microseconds) and gain in the sensor.
     *
     * @param u32Exposure Exposure time (microseconds)
     * @param dGain gain value
     */
    STF_RESULT (*SetExposureAndGain)(ST_SENSOR *pstSensor, STF_U32 u32Exposure, STF_DOUBLE dGain);

    /**
     * @brief Get state of driver support for variable focus.
     */
    STF_BOOL8 (*GetFocusSupported)(ST_SENSOR *pstSensor);

    /**
     * @brief Get minimum focus distance programmable in the sensor in
     * milimiters.
     */
    STF_U16 (*GetMinFocus)(ST_SENSOR *pstSensor);

    /**
     * @brief Get maximum focus distance programmable in the sensor in
     * milimiters.
     */
    STF_U16 (*GetMaxFocus)(ST_SENSOR *pstSensor);

    /**
     * @brief Get the focus distance currently programmed in the sensor in
     * milimiters
     *
     * @return Focus distance currently set in the sensor
     */
    STF_U16 (*GetFocusDistance)(ST_SENSOR *pstSensor);

    /**
     * @brief Set focus distance in the sensor in milimiters
     *
     * @param focusDistance Focus distance to be set in the sensor
     */
    STF_RESULT (*SetFocusDistance)(ST_SENSOR *pstSensor, STF_U16 u16FocusDistance);
} ST_SENSOR, *PST_SENSOR;


extern
const STF_CHAR* STFLIB_ISP_SENSOR_StateName(
    EN_SENSOR_STATE enState
    );

extern
STF_VOID STFLIB_ISP_SENSOR_GetSensorNames(
    STF_U8 *pu8Count,
    STF_CHAR szSensorName[][64]
    );

extern
STF_S8 STFLIB_ISP_SENSOR_GetSensorId(
    const STF_CHAR *pszSensorName
    );

extern
STF_RESULT STFLIB_ISP_SENSOR_Init(
#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
    CI_CONNECTION *pCIConnection,
    STF_U8 u8IspIdx,
#endif //#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
    ST_SENSOR *pstSensor,
    STF_U8 u8SensorId,
    STF_U8 u8Index
    );

extern
STF_RESULT STFLIB_ISP_SENSOR_Destroy(
    ST_SENSOR *pstSensor
    );

extern
SENSOR_HANDLE * STFLIB_ISP_SENSOR_GetHandle(
    ST_SENSOR *pstSensor
    );

extern
EN_SENSOR_STATE STFLIB_ISP_SENSOR_GetState(
    ST_SENSOR *pstSensor
    );

extern
STF_RESULT STFLIB_ISP_SENSOR_GetInfo(
    ST_SENSOR *pstSensor,
    SENSOR_INFO *pstSensorInfo
    );

extern
STF_S8 STFLIB_ISP_SENSOR_GetModeIdx(
    ST_SENSOR *pstSensor,
    STF_U16 u16Width,
    STF_U16 u16Height
    );

extern
STF_RESULT STFLIB_ISP_SENSOR_GetMode(
    ST_SENSOR *pstSensor,
    STF_U8 u8ModeIdx,
    SENSOR_MODE *pstModes
    );

extern
STF_RESULT STFLIB_ISP_SENSOR_SetMode(
    ST_SENSOR *pstSensor,
    STF_U8 u8ModeIdx,
    STF_U8 u8Flipping
    );

extern
STF_RESULT STFLIB_ISP_SENSOR_GetInterfaceInfo(
    ST_SENSOR *pstSensor,
    SENSOR_INTFC *pstInterface
    );

extern
STF_RESULT STFLIB_ISP_SENSOR_Configure(
    ST_SENSOR *pstSensor,
    STF_U8 u8ModeIdx,
    STF_U8 u8Flipping
    );

extern
STF_RESULT STFLIB_ISP_SENSOR_Insert(
    ST_SENSOR *pstSensor
    );

extern
STF_RESULT STFLIB_ISP_SENSOR_WaitProcessed(
    ST_SENSOR *pstSensor
    );

extern
STF_RESULT STFLIB_ISP_SENSOR_Enable(
    ST_SENSOR *pstSensor
    );

extern
STF_RESULT STFLIB_ISP_SENSOR_Disable(
    ST_SENSOR *pstSensor
    );

extern
STF_RESULT STFLIB_ISP_SENSOR_Reset(
    ST_SENSOR *pstSensor
    );

extern
STF_RESULT STFLIB_ISP_SENSOR_GetStatus(
    ST_SENSOR *pstSensor,
    SENSOR_STATUS *pstStatus
    );

extern
STF_RESULT STFLIB_ISP_SENSOR_GetReg(
    ST_SENSOR *pstSensor,
    STF_U16 u16RegAddr,
    STF_U16 *pu16RegValue
    );

extern
STF_RESULT STFLIB_ISP_SENSOR_SetReg(
    ST_SENSOR *pstSensor,
    STF_U16 u16RegAddr,
    STF_U16 u16RegValue
    );

extern
STF_RESULT STFLIB_ISP_SENSOR_GetFlipMirror(
    ST_SENSOR *pstSensor,
    STF_U8 *pu8Flag
    );

#ifdef SIFIVE_ISP_TUNING
extern
STF_RESULT STFLIB_ISP_SENSOR_SetFlipMirror(
    ST_SENSOR *pstSensor,
    STF_U8 u8Flag
    );

#endif //SIFIVE_ISP_TUNING
extern
STF_RESULT STFLIB_ISP_SENSOR_GetFixedFPS(
    ST_SENSOR *pstSensor,
    STF_U16 *pu16FixedFps
    );

extern
STF_RESULT STFLIB_ISP_SENSOR_SetFPS(
    ST_SENSOR *pstSensor,
    STF_DOUBLE dFps
    );

extern
STF_RESULT STFLIB_ISP_SENSOR_GetSnapShotResolution(
    ST_SENSOR *pstSensor,
    ST_RES_LIST *pstResList
    );

extern
STF_RESULT STFLIB_ISP_SENSOR_SetResolution(
    ST_SENSOR *pstSensor,
    STF_U16 u16ImgW,
    STF_U16 u16ImgH
    );

extern
STF_U32 STFLIB_ISP_SENSOR_GetMinExposure(
    ST_SENSOR *pstSensor
    );

extern
STF_U32 STFLIB_ISP_SENSOR_GetMaxExposure(
    ST_SENSOR *pstSensor
    );

extern
STF_RESULT STFLIB_ISP_SENSOR_GetExposureRange(
    ST_SENSOR *pstSensor,
    STF_U32 *pu32MinExposure,
    STF_U32 *pu32MaxExposure
    );

extern
STF_U32 STFLIB_ISP_SENSOR_GetExposure(
    ST_SENSOR *pstSensor
    );

extern
STF_RESULT STFLIB_ISP_SENSOR_SetExposure(
    ST_SENSOR *pstSensor,
    STF_U32 u32Exposure
    );

extern
STF_DOUBLE STFLIB_ISP_SENSOR_GetMinGain(
    ST_SENSOR *pstSensor
    );

#ifdef SIFIVE_ISP_TUNING
extern
STF_RESULT STFLIB_ISP_SENSOR_SetMinGain(
    ST_SENSOR *pstSensor,
    STF_DOUBLE dMinGain
    );

#endif //SIFIVE_ISP_TUNING
extern
STF_DOUBLE STFLIB_ISP_SENSOR_GetMaxGain(
    ST_SENSOR *pstSensor
    );

#ifdef SIFIVE_ISP_TUNING
extern
STF_RESULT STFLIB_ISP_SENSOR_SetMaxGain(
    ST_SENSOR *pstSensor,
    STF_DOUBLE dMaxGain
    );

#endif //SIFIVE_ISP_TUNING
extern
STF_RESULT STFLIB_ISP_SENSOR_GetGainRange(
    ST_SENSOR *pstSensor,
    STF_DOUBLE *pdMinGain,
    STF_DOUBLE *pdMaxGain
    );

extern
STF_RESULT STFLIB_ISP_SENSOR_SetIsoLimit(
    ST_SENSOR *pstSensor,
    STF_U8 u8IsoLimit
    );

extern
STF_DOUBLE STFLIB_ISP_SENSOR_GetGain(
    ST_SENSOR *pstSensor
    );

extern
STF_RESULT STFLIB_ISP_SENSOR_SetGain(
    ST_SENSOR *pstSensor,
    STF_DOUBLE dGain
    );

extern
STF_RESULT STFLIB_ISP_SENSOR_SetExposureAndGain(
    ST_SENSOR *pstSensor,
    STF_U32 u32Exposure,
    STF_DOUBLE dGain
    );

extern
STF_BOOL8 STFLIB_ISP_SENSOR_GetFocusSupported(
    ST_SENSOR *pstSensor
    );

extern
STF_U16 STFLIB_ISP_SENSOR_GetMinFocus(
    ST_SENSOR *pstSensor
    );

extern
STF_U16 STFLIB_ISP_SENSOR_GetMaxFocus(
    ST_SENSOR *pstSensor
    );

extern
STF_U16 STFLIB_ISP_SENSOR_GetFocusDistance(
    ST_SENSOR *pstSensor
    );

extern
STF_RESULT STFLIB_ISP_SENSOR_SetFocusDistance(
    ST_SENSOR *pstSensor,
    STF_U16 u16FocusDistance
    );

/**
 * @brief Create a sensor using given identifier.
 *
 * Sensor is initialised in this function.
 * Mode should be selected with configure() and sensor can then be started
 * using enabled()
 *
 * @param sensorId
 */
extern
STF_RESULT STFLIB_ISP_SENSOR_SturctInitialize(
    ST_SENSOR *pstSensor,
#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
    CI_CONNECTION *pCIConnection,
    STF_U8 u8IspIdx,
#endif //#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
    STF_U8 u8SensorId,
    STF_U8 u8Index
    );

/**
 * @brief Destroy a sensor using given identifier.
 */
extern
STF_RESULT STFLIB_ISP_SENSOR_SturctUninitialize(
    ST_SENSOR *pstSensor
    );


#ifdef __cplusplus
}
#endif


#endif //__STFLIB_ISP_SENSOR_H__
