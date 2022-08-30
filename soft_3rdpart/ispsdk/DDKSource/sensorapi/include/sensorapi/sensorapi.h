/**
  ******************************************************************************
  * @file sensorapi.h
  * @author  StarFive Isp Team
  * @version  V1.0
  * @date  08/12/2020
  * @brief Header describing the API for controlling sensors
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


#ifndef __SENSOR_API_H__
#define __SENSOR_API_H__


#ifdef __cplusplus /* If this is a C++ compiler, use C linkage */
extern "C" {
#endif


#include "stf_include.h"
#include <stdio.h> // FILE*
#include "stf_common/pixel_format.h"
#include "ci/ci_api.h"


/**
 * @defgroup SENSOR_API Sensor Interface: API documentation
 * @brief Documentation of the Sensor Interface user-side API.
 * 
 * The Sensor API is intended to provide an abstraction of common sensor setup
 * and control functions so that the STF AAA functions and camera HAL can
 * easily be re-targeted at other camera sensors. 
 *
 * To see how to add a new sensor look at the @ref SENSOR_GUIDE
 * @{
 */
/*-----------------------------------------------------------------------------
 * Following elements are in the SENSOR_API documentation module
 *---------------------------------------------------------------------------*/

/**
 * @brief Handle to the functions of the sensor
 */
typedef struct _SENSOR_FUNCS SENSOR_HANDLE;


/** @brief maximum size of the string for the sensor info name string */
#define SENSOR_INFO_NAME_MAX 64
/** @brief maximum size of the string for the sensor version string */
#define SENSOR_INFO_VERSION_MAX 64


typedef enum _EN_EXPO_GAIN_METHOD {
    EN_EXPO_GAIN_METHOD_SAME_TIME = 0,
    EN_EXPO_GAIN_METHOD_1ST_EXPO_2ND_GAIN,
    EN_EXPO_GAIN_METHOD_1ST_GAIN_2ND_EXPO,
} EN_EXPO_GAIN_METHOD, *PEN_EXPO_GAIN_METHOD;
#define EN_EXPO_GAIN_METHOD_MAX     (EN_EXPO_GAIN_METHOD_1ST_GAIN_2ND_EXPO + 1)

typedef enum _SENSOR_FLIPPING {
    SENSOR_FLIP_NONE = 0,
    /** @brief Horizontal mirroring of the image */
    SENSOR_FLIP_HORIZONTAL = 1, 
    /** @brief Vertical mirroring of the image */
    SENSOR_FLIP_VERTICAL = (SENSOR_FLIP_HORIZONTAL << 1),
    /** @brief Horizontal and vertical mirroring of the image */
    SENSOR_FLIP_BOTH = (SENSOR_FLIP_HORIZONTAL | SENSOR_FLIP_VERTICAL),
} SENSOR_FLIPPING, *PSENSOR_FLIPPING;

/**
 * @brief sensor states
 *
 * When the sensor is Uninitialised you can do nothing other than initialise 
 * it, this will return a handle that you use in subsequent calls.
 * User should never see an Uninitialised sensor as while it is in this state 
 * they do not have a handle to it.
 *
 * In Idle you destroy the sensor, you can also adjust parameters such as 
 * setting the mode, exposure, gain and focus, and change mode.
 *
 * When running you can disable the sensor or adjust flipping, focus ,exposure 
 * and gain. you cannot change mode.
 */
typedef enum _SENSOR_STATE {
    /**
     * @brief Sensor has not been initialised should never see this as it 
     * should always be either STATE_IDLE or STATE_RUNNING
     */
    SENSOR_STATE_UNINITIALISED = 0,
    /**
     * @brief Sensor has been initialised and is idle
     */
    SENSOR_STATE_IDLE,
    /**
     * @brief Sensor has been initialised and is running
     */
    SENSOR_STATE_RUNNING
} SENSOR_STATE, *PSENSOR_STATE;

/**
 * @brief Status of the sensor (state + selected mode + selected flipping)
 */
typedef struct _SENSOR_STATUS {
    /**
     * @brief Mode number that sensor is currently running in, this is an index
     * to the list of modes return by the GetSensorMode call
     */
    STF_U8 u8CurrentMode;
    /**
     * @brief Selected flipping (composed with values from @ref SENSOR_FLIPPING)
     */
    STF_U8 u8Flipping;
    STF_DOUBLE dCurrentFps;
    /**
     * @brief State of the sensor
     */
    SENSOR_STATE enState;
} SENSOR_STATUS, *PSENSOR_STATUS;

/**
 * @brief Contains information about a sensor mode
 */
typedef struct _SENSOR_MODE
{
    /**
     * @brief Mode's bitdepth
     */
    STF_U8 u8BitDepth;
    /**
     * @brief width of output in pixels
     */
    STF_U16 u16Width;
    /**
     * @brief height of output in pixels
     */
    STF_U16 u16Height;
    /**
     * @brief frame-rate in Hz
     */
    STF_DOUBLE dFrameRate;
    /**
     * @brief pixel rate in Mpx/s
     *
     * This can usually be computed from
     * ui16HorizontalTotal*ui16VerticalTotal*flFrameRate but may vary for
     * some sensor
     */
    STF_DOUBLE dPixelRate;
    /**
     * @brief Horizontal total size (including blanking) in pixels
     *
     * Uesed by the system to compute the bit-rate
     */
    STF_U16 u16HorizontalTotal;
    /**
     * @brief Vertical total size (including blanking) in pixels
     *
     * Used by the system to allow it to pass the information to the flicker 
     * detection
     */
    STF_U16 u16VerticalTotal;
    /**
     * @brief Supports flipping when enabling
     */
    STF_U8 u8SupportFlipping;

    STF_U32 u32ExposureMin;
    STF_U32 u32ExposureMax;
    STF_DOUBLE dExposureMin;
    
    STF_U8 u8MipiLanes;
} SENSOR_MODE, *PSENSOR_MODE;

typedef struct _SENSOR_INTFC
{
    STF_U8 u8DataInterface;     /** Indicates which data interface will be used. 0: DVP, 1: MIPI. */
    STF_U8 u8I2cChannel;        /** Indicates which I2C channel will be used. Range: 0 - 3. */
    STF_U8 u8I2cSlaveAddrbit;   /** Indicates the slave address bit. 0: 7 bits, 1: 10 bits. */
    STF_U8 u8I2cRegAddrBit;     /** Indicates the sensor register address bit. 0: 8 bits, 1: 16 bits. */
    STF_U8 u8I2cRegDataBit;     /** Indicates the sensor register data bit. 0: 8 bits, 1: 16 bits. */
    STF_U16 u16I2cSlaveAddr;    /** Indicates the slave address of sensor. */
    STF_S16 s16PowerGpioId;
    STF_U16 u16PowerGpioInitLevel;
    STF_S16 s16ResetGpioId;
    STF_U16 u16ResetGpioInitLevel;
    STF_U16 u16ResetIntervalTime;
    STF_U8 u8SensorPolarity;
} SENSOR_INTFC, *PSENSOR_INTFC;

/**
 * @brief Sensor + status + mode info
 */
typedef struct _SENSOR_INFO
{
    /**
     * @brief original CFA filter layout (before flipping)
     *
     * @ref eBayerEnabled for currently enabled bayer format
     *
     * @note SHOULD NOT be affected by mode selection
     */
    enum MOSAICType enBayerOriginal;

    /**
     * @brief enabled CFA filter layout (including flipping)
     *
     * This is filled up by sensor's function as sensors may have different
     * patterns if flipping is enabled or not
     *
     * Should be derived from @ref SENSOR_INFO::eBayerOriginal and
     * @ref SENSOR_STATUS::ui8Flipping
     */
    enum MOSAICType enBayerEnabled;

    /** 
     * @brief a Text name for the sensor for easy identification
     */
    char pszSensorName[SENSOR_INFO_NAME_MAX]; 
    /**
     * @brief to be used where multiple versions of the sensor exists
     */
    char pszSensorVersion[SENSOR_INFO_VERSION_MAX];

    /**
     * @brief Aperture of the attached lens
     */
    STF_DOUBLE dNumber;

    /**
     * @brief Focal length of the lens (does not take into account any 
     * magnification due to cropping)
     */
    STF_U16 u16FocalLength;

    /**
     * @brief Number of electrons the sensor wells can take before clipping
     */
    STF_U32 u32WellDepth;

    /**
     * @brief noise of the sensor in electrons (standard deviation)
     */
    STF_DOUBLE dReadNoise;

    /**
     * @brief gasket used by the imager
     */
    STF_U8 u8Imager;

    /**
     * @brief Is this a back or front facing camera. 
     *
     * To be used by application layer to determine horizontal/vertical flip 
     * orientations
     */
    STF_BOOL8 bBackFacing;

    /**
     * @brief Current status of the sensor
     *
     * @note Filled by @ref Sensor_GetInfo() before calling sensor's function 
     * - no need to re-implement in sensor
     */
    SENSOR_STATUS stStatus;

    /**
     * @brief Information about current mode
     *
     * @note Filled by @ref Sensor_GetInfo() before calling sensor's function 
     * - no need to re-implement in sensor
     */
    SENSOR_MODE stMode;

//    /**
//     * @brief Information about hardware connection data bus and I2C information.
//     */
//    SENSOR_INTFC stInterface;

    STF_U32 u32ModeCount;

    EN_EXPO_GAIN_METHOD enExposureGainMethod;
} SENSOR_INFO, *PSENSOR_INFO;

/* Parameter handling
The main parameters that we are interested in handling are Gain, Exposure, Sensor orientation and focus.
In order to allow an extensible system that can cope with other potential parameters we shall use a mechanism similar to the OMX extension system

So calls to will fill in the address of the param name (we would expect to use common ones for common parameters e.g. GlobalGain, but this
way will allo a tweaking application to require very little specific knowledge) it will also return the max,min and current values for the configuration.
The return value wivoid Sensor_PrintAllModes(FILE *f)ll be the index for parameters that are known, and -1 when the parameter does not exist (i.e. we have reached the end of the list)
This function can only be called in the idle state or above*/
/* Using a simple naming scheme will allow an application to group related parameters together e.g.
"Invert.Horizontal" and "Invert.Vertical" could be used*/

/**
 * @brief Interface functions for the sensor device.
 *
 * This should be populated by the sensor driver to provide an interface that
 * can cope with multiple sensors implemented by different vendors whilst 
 * providing a consistent point of entry
 *
 * This structure should be populate at the creation time of a sensor.
 *
 * All function must be implemented.
 * If they are marked as optional it is allowed to always have an erroneous
 * return such as STF_ERROR_NOT_SUPPORTED
 */
typedef struct _SENSOR_FUNCS
{
    /**
     * @name Controls and information
     * @{
     */

    /**
     * @brief Get the mode index of the sensor
     *
     * @param hHandle the sensor
     * @param u16Width the width of mode for the mode to check.
     * @param u16Height the height of of mode for the mode to check.
     * corresponding to the index.
     *
     * @return positive mode index or negative number if mode is not found
     */
    STF_S8 (*GetModeIdx)(
        SENSOR_HANDLE *pstHandle,
        STF_U16 u16Width,
        STF_U16 u16Height
        );
    /**
     * @brief Fill in a structure detailing the modes the sensor supports
     *
     * @param hHandle the sensor
     * @param nIndex the index for the mode to check.
     * @param[out] psModes pointer to a Sensor_Mode structure listing the mode
     * corresponding to the index.
     *
     * @return STF_SUCCESS if a mode with the corresponding index was found
     * @return STF_ERROR_VALUE_OUT_OF_RANGE if the mode index does not
     * exist
     */
    STF_RESULT (*GetMode)(
        SENSOR_HANDLE *pstHandle,
        STF_U8 u8ModeIdx,
        SENSOR_MODE *pstModes
        );
    /**
     * @brief Get the current state of the sensor
     *
     * @param hHandle the sensor
     * @param[out] psStatus pointer to a SENSOR_STATUS structure to be filled 
     * in by the call
     *
     * @return STF_SUCCESS
     */
    STF_RESULT (*GetState)(
        SENSOR_HANDLE *pstHandle,
        SENSOR_STATUS *pstStatus
        );
    /**
     * @brief Get the sensor's connection data bus and I2C hardware information.
     *
     * @param hHandle the sensor
     * @param[out] pstInterface pointer to a SENSOR_INTFC structure to be filled
     * in by the call
     *
     * @return STF_SUCCESS
     */
    STF_RESULT (*GetInterfaceInfo)(
        SENSOR_HANDLE *pstHandle,
        SENSOR_INTFC *pstInterface
        );
    /**
     * @brief Set the mode of the sensor
     *
     * A sensor mode should be a combination of size and frame-rate
     *
     * @param hHandle the sensor
     * @param nMode index into the sensor mode list returned by the 
     * GetSensorModes function
     * @param ui8Flipping Flip the sensor image horizontal readout
     * direction (mirror mode) or in vertical readout direction (use 
     * combinaison of @ref SENSOR_FLIPPING)
     *
     * @return STF_SUCCESS
     * @return STF_ERROR_VALUE_OUT_OF_RANGE if nMode is not a valid mode
     * @return STF_ERROR_NOT_SUPPORTED if selected flipping isn't available
     * @return STF_ERROR_UNEXPECTED_STATE if sensor already enabled
     */
    STF_RESULT (*SetMode)(
        SENSOR_HANDLE *pstHandle,
        STF_U8 u8ModeIdx,
        STF_U8 u8Flipping
        );
    /**
     * @brief Enable the sensor
     *
     * The difference between configured (done with @ref SENSOR_FUNCS::SetMode)
     * and enabled is that a configured sensor may not be sending data yet 
     * while an enabled one should be sending data.
     *
     * @param hHandle the sensor
     *
     * @return STF_SUCCESS
     * @return STF_ERROR_UNEXPECTED_STATE if the device cannot be enabled
     * (either it is already enabled or not in correct state to enable)
     */
    STF_RESULT (*Enable)(
        SENSOR_HANDLE *pstHandle
        );
    /**
     * @brief Disable the sensor
     *
     * The sensor should stop sending data but not be destroyed (i.e. 
     * @ref SENSOR_FUNCS::Enable should make it start sending data again)
     *
     * @param hHandle the sensor
     *
     * @return STF_SUCCESS
     * @return STF_ERROR_UNEXPECTED_STATE if the device cannot be disabled
     * (either it is already disabled or not in correct state to disable)
     */
    STF_RESULT (*Disable)(
        SENSOR_HANDLE *pstHandle
        );
    /**
     * @brief Release and de-initialise sensor
     *
     * @param hHandle the sensor
     *
     * @return STF_SUCCESS
     * @return STF_ERROR_UNEXPECTED_STATE if the device cannot be destroyed
     * (it should be IDLE to be destroyable)
     */
    STF_RESULT (*Destroy)(
        SENSOR_HANDLE *pstHandle
        );

    /**
     * @brief Get information about the sensor - dependent on what was chosen 
     * with @ref SENSOR_FUNCS::SetMode
     * 
     * See @ref SENSOR_INFO members description to know what this function
     * should populate.
     *
     * @param hHandle the sensor
     * @param[out] return info structure
     *
     * @return STF_SUCCESS
     */
    STF_RESULT (*GetInfo)(
        SENSOR_HANDLE *pstHandle,
        SENSOR_INFO *pstInfo
        );

    /**
     * @}
     * @name Register access controls
     * @{
     */

    /**
     * @brief Read the sensor register value
     *
     * @param hHandle the sensor
     * @param u16RegAddr the register address
     * @param[out] pu16RegVal pointer to the read value of register
     * @param ui8Context the register context that this setting refers to
     *
     * @return STF_SUCCESS
     * @return STF_ERROR_VALUE_OUT_OF_RANGE if ui8Context is not a valid
     * context
     */
    STF_RESULT (*GetRegister)(
        SENSOR_HANDLE *pstHandle,
        STF_U16 u16RegAddr,
        STF_U16 *pu16RegVal,
        STF_U8 u8Context
        );
    /**
     * @brief Set the sensor register value
     *
     * @param hHandle the sensor
     * @param u16RegAddr the register address
     * @param u16RegVal the register value
     * @param ui8Context the register context that this setting change will
     * apply to
     *
     * @return STF_SUCCESS
     * @return STF_ERROR_VALUE_OUT_OF_RANGE if ui8Context is not a valid
     * context
     */
    STF_RESULT (*SetRegister)(
        SENSOR_HANDLE *pstHandle,
        STF_U16 u16RegAddr,
        STF_U16 u16RegVal,
        STF_U8 u8Context
        );

    /**
     * @}
     * @name Gains and exposure
     * @{
     */

    /**
     * @brief Get the range of the gain settings the sensor can support
     *
     * @param hHandle the sensor
     * @param[out] pflMin pointer to minimum gain setting
     * @param[out] pflMax pointer to maximum gain setting
     * @param[out] pui8Contexts the number of separate gains
     *
     * @note This range can include both analogue and digital gain. 
     * The adjustment should apply analogue first and if the required setting
     * needs some extra then apply digital gain.
     *
     * @return STF_SUCCESS
     * @return STF_ERROR_UNEXPECTED_STATE if setMode() has not been called
     */
    STF_RESULT (*GetGainRange)(
        SENSOR_HANDLE *pstHandle,
        STF_DOUBLE *pdMin,
        STF_DOUBLE *pdMax,
        STF_U8 *pu8Contexts
        );
    /**
     * @brief Get the current gain setting for the sensor
     *
     * @param hHandle the sensor
     * @param[out] pflCurrent Pointer to receive the current gain setting
     * @param ui8Context the gain context that this setting refers to
     *
     * @return STF_SUCCESS
     * @return STF_ERROR_VALUE_OUT_OF_RANGE if ui8Context is not a valid
     * context
     * @return STF_ERROR_UNEXPECTED_STATE if the state is not valid to get
     * gains
     */
    STF_RESULT (*GetCurrentGain)(
        SENSOR_HANDLE *pstHandle,
        STF_DOUBLE *pdCurrentGain,
        STF_U8 u8Context
        );
    /**
     * @brief Set the current gain setting for the sensor
     *
     * @param hHandle the sensor
     * @param flGain The new setting to use
     * @param ui8Context the gain context that this setting refers to
     *
     * @return STF_SUCCESS
     * @return STF_ERROR_VALUE_OUT_OF_RANGE if ui8Context is not a valid
     * context
     * @return STF_ERROR_UNEXPECTED_STATE if the state is not valid to change
     * gains
     */
    STF_RESULT (*SetGain)(
        SENSOR_HANDLE *pstHandle,
        STF_DOUBLE dGain,
        STF_U8 u8Context
        );
    /**
     * @brief Get the range of exposures the sensor can use
     *
     * @param hHandle the sensor
     * @param[out] pui32Min the shortest exposure (measured in microseconds)
     * @param[out] pui32Max the longest exposure the sensor will allow
     * @param[out] pui8Contexts the number of separate exposures
     *
     * @note If an exposure is longer than the frame period this can cause the 
     * frame-rate to slow down
     *
     * @return STF_SUCCESS
     * @return STF_ERROR_UNEXPECTED_STATE if the state is not valid to get
     * exposure ranges
     */
    STF_RESULT (*GetExposureRange)(
        SENSOR_HANDLE *pstHandle,
        STF_U32 *pu32Min,
        STF_U32 *pu32Max,
        STF_U8 *pu8Contexts
        );
    /**
     * @brief Query the sensor for the current exposure period
     *
     * @param hHandle the sensor
     * @param[out] pui32Exposure pointer to hold the queried exposure value
     * @param ui8Context the exposure context that this setting refers to
     *
     * @return STF_SUCCESS
     * @return STF_ERROR_VALUE_OUT_OF_RANGE if ui8Context is not a valid
     * context
     */
    STF_RESULT (*GetExposure)(
        SENSOR_HANDLE *pstHandle,
        STF_U32 *pu32Exposure,
        STF_U8 u8Context
        );
    /**
     * @brief Set the sensor to the requested exposure period
     *
     * @param hHandle the sensor
     * @param ui32Exposure target exposure period in microseconds.
     * @param ui8Context the exposure context that this setting change will 
     * apply to
     *
     * @return STF_SUCCESS
     * @return STF_ERROR_VALUE_OUT_OF_RANGE if ui8Context is not a valid
     * context
     */
    STF_RESULT (*SetExposure)(
        SENSOR_HANDLE *pstHandle,
        STF_U32 u32Exposure,
        STF_U8 u8Context
        );

    /**
     * @}
     * @name Focus control
     * @{
     */

    /**
     * @brief [optional] Get the range of allowed focus positions
     *
     * @param hHandle the sensor
     * @param[out] pui16Min the minimum focus position in mm
     * @param[out] pui16Max the maximum focus position in mm
     *
     * @return STF_SUCCESS
     */
    STF_RESULT (*GetFocusRange)(
        SENSOR_HANDLE *pstHandle,
        STF_U16 *pu16Min,
        STF_U16 *pu16Max
        );
    /**
     * @brief [optional] Query the current focus position
     *
     * @param hHandle the sensor
     * @param[out] pui16Current the current focus position in mm
     *
     * @return STF_SUCCESS
     */
    STF_RESULT (*GetCurrentFocus)(
        SENSOR_HANDLE *pstHandle,
        STF_U16 *pu16CurrentFocus
        );
    /**
     * @brief [optional] Set the sensor to focus at the selected position
     *
     * @param hHandle the sensor
     * @param ui16Focus the target focus position that the sensor should set in
     * mm
     *
     * @return STF_SUCCESS
     */
    STF_RESULT (*SetFocus)(
        SENSOR_HANDLE *pstHandle,
        STF_U16 u16Focus
        );

    /**
     * @}
     * @name Miscellaneous controls
     * @{
     */

    /**
     * @brief [optional] Set the sensor to the requested exposure period
     *
     * @param hHandle the sensor
     * @param bAlwaysOn If set, the flash is always enabled, if false then the
     * flash will be enabled for i16Frames, after a delay of i16FrameDelay 
     * frames
     * @param i16FrameDelay delay between this call and the enabling of the 
     * flash
     * @param i16Frames number of frames for flash to be active
     * @param ui16FlashPulseWidth Pulse width of flash pulse for Xenon flashes,
     * measured in pixel clock cycles.
     *
     * @return STF_SUCCESS
     */
    STF_RESULT (*ConfigureFlash)(
        SENSOR_HANDLE *pstHandle,
        STF_BOOL8 bAlwaysOn,
        STF_S16 s16FrameDelay,
        STF_S16 s16Frames,
        STF_U16 u16FlashPulseWidth
        );

    /**
     * @brief [optional] Used to control insertion point of the sensor.
     *
     * When call a frame should be inserted into the sensor.
     * The mechanism of setting the source of the data has to be handled using
     * extended parameters and is specific per sensor.
     *
     * Optionally after an insert a sensor can require
     * @ref SENSOR_FUNCS::WaitProcessed to be called to wait for the frame
     * to have been fully processed.
     *
     * @param hHandle the sensor
     *
     * @return STF_SUCCESS
     */
    STF_RESULT (*Insert)(
        SENSOR_HANDLE *pstHandle
        );

    /**
     * @brief [optional] Used to wait for an inserted frame to have been
     * fully processed and sent to the ISP.
     *
     * @param hHandle the sensor
     *
     * @return STF_SUCCESS
     */
    STF_RESULT (*WaitProcessed)(
        SENSOR_HANDLE *pstHandle
        );

    STF_RESULT (*SetFlipMirror)(
        SENSOR_HANDLE *pstHandle,
        STF_U8 u8Flag
        );

    STF_RESULT (*GetFixedFPS)(
        SENSOR_HANDLE *pstHandle,
        STF_U16 *py16Fixed
        );

    STF_RESULT (*SetFPS)(
        SENSOR_HANDLE *pstHandle,
        STF_DOUBLE dFps
        );

    STF_RESULT (*SetExposureAndGain)(
        SENSOR_HANDLE *pstHandle,
        STF_U32 u32Exposure,
        STF_DOUBLE dGain,
        STF_U8 u8Context
        );

    STF_RESULT (*SetResolution)(
        SENSOR_HANDLE *pstHandle,
        STF_U16 u16ImgW,
        STF_U16 U16ImgH
        );

    STF_RESULT (*GetSnapRes)(
        SENSOR_HANDLE *pstHandle,
        ST_RES_LIST *pstReslist
        );

    STF_RESULT (*Reset)(
        SENSOR_HANDLE *pstHandle
        );

    /**
     * @}
     */
} SENSOR_FUNCS, *PSENSOR_FUNCS;


/**
 * @name Sensor controls
 * @brief The main controls of the sensor
 * @{
 */

/**
 * @brief Access to a list of sensor names
 *
 * The index into the list is the sensor index
 */
extern
const STF_CHAR **Sensor_ListAll(
    STF_VOID
    );

/**
 * @brief Access the number of available sensors
 */
extern
STF_U8 Sensor_NumberSensors(
    STF_VOID
    );

STF_VOID Sensor_PrintAllModes(
#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
    CI_CONNECTION *pCIConnection,
    STF_U8 u8IspIdx,
#endif //#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
    FILE *f
    );

/**
 * @brief Initialise a sensor
 *
 * @note causes a sensor to transition from Uninitialised to Idle
 *
 * @param nSensor The index of the sensor to initialise
 * @param phHandle The handle of the sensor to initialise
 *
 * @return STF_SUCCESS
 * @return STF_ERROR_INVALID_PARAMETERS if phHandle is NULL or nSensor is too
 * big an idea to be supported
 */
extern
STF_RESULT Sensor_Initialise(
#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
    CI_CONNECTION *pCIConnection,
    STF_U8 u8IspIdx,
#endif //#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
    STF_U16 u16Sensor,
    SENSOR_HANDLE **ppstHandle,
    STF_U8 u8Index
    );

/**
 * @see Delegates to SENSOR_FUNCS::GetModeIdx
 * @copydoc SENSOR_FUNCS::GetModeIdx
 *
 * @return return positive mode index or negative number if mode is not found
 */
extern
STF_S8 Sensor_GetModeIdx(
    SENSOR_HANDLE *pstHandle,
    STF_U16 u16Width,
    STF_U16 u16Height
    );

/**
 * @see Delegates to SENSOR_FUNCS::GetMode
 * @copydoc SENSOR_FUNCS::GetMode
 *
 * @return STF_ERROR_INVALID_PARAMETERS if hHandle or psModes is NULL
 * @return STF_ERROR_FATAL if GetMode function is not available for this
 * sensor
 */
extern
STF_RESULT Sensor_GetMode(
    SENSOR_HANDLE *pstHandle,
    STF_U8 u8ModeIdx,
    SENSOR_MODE *pstModes
    );

/**
 * @see Delegates to SENSOR_FUNCS::GetState
 * @copydoc SENSOR_FUNCS::GetState
 *
 * @return STF_ERROR_INVALID_PARAMETERS if hHandle or psStatus is NULL
 * @return STF_ERROR_FATAL if GetState function is not available for this
 * sensor
 */
extern
STF_RESULT Sensor_GetState(
    SENSOR_HANDLE *pstHandle,
    SENSOR_STATUS *pstStatus
    );

/**
 * @see Delegates to SENSOR_FUNCS::GetInterfaceInfo
 * @copydoc SENSOR_FUNCS::GetInterfaceInfo
 *
 * @return STF_ERROR_INVALID_PARAMETERS if hHandle or psStatus is NULL
 * @return STF_ERROR_FATAL if GetInterfaceInfo function is not available for this
 * sensor
 */
extern
STF_RESULT Sensor_GetInterfaceInfo(
    SENSOR_HANDLE *pstHandle,
    SENSOR_INTFC *pstInterface
    );

/**
 * @see Delegates to SENSOR_FUNCS::SetMode
 * @copydoc SENSOR_FUNCS::SetMode

 * @return STF_ERROR_INVALID_PARAMETERS if hHandle is NULL
 * @return STF_ERROR_FATAL if SetMode function is not available for this sensor
 */
extern
STF_RESULT Sensor_SetMode(
    SENSOR_HANDLE *pstHandle,
    STF_U8 u8ModeIdx,
    STF_U8 u8Flipping
    );

/**
 * @see Delegates to SENSOR_FUNCS::Enable
 * @copydoc SENSOR_FUNCS::Enable
 *
 * @return STF_ERROR_INVALID_PARAMETERS if hHandle is NULL
 * @return STF_ERROR_FATAL if Enable function is not available for this sensor
 */
extern
STF_RESULT Sensor_Enable(
    SENSOR_HANDLE *pstHandle
    );

/**
 * @see Delegates to SENSOR_FUNCS::Disable
 * @copydoc SENSOR_FUNCS::Disable
 *
 * @return STF_ERROR_INVALID_PARAMETERS if hHandle is NULL
 * @return STF_ERROR_FATAL if Disable function is not available for this sensor
 */
extern
STF_RESULT Sensor_Disable(
    SENSOR_HANDLE *pstHandle
    );

/**
 * @see Delegates to SENSOR_FUNCS::Destroy
 * @copydoc SENSOR_FUNCS::Destroy
 *
 * @return STF_ERROR_INVALID_PARAMETERS if hHandle is NULL
 * @return STF_ERROR_FATAL if Destroy function is not available for this sensor
 */
extern
STF_RESULT Sensor_Destroy(
    SENSOR_HANDLE *pstHandle
    );

/**
 * @brief Get information about the sensor
 *
 * @param hHandle the sensor
 * @param[out] psInfo info structure
 *
 * @return STF_SUCCESS
 * @return STF_ERROR_INVALID_PARAMETERS if hHandle or psInfo is NULL
 * @return STF_ERROR_FATAL if GetInfo function is not available for this sensor
 */
extern
STF_RESULT Sensor_GetInfo(
    SENSOR_HANDLE *pstHandle,
    SENSOR_INFO *pstInfo
    );

/**
 * @}
 */
/*-----------------------------------------------------------------------------
 * End of the sensor controls
 *---------------------------------------------------------------------------*/ 
/**
 * @name Register access controls
 * @brief Register access controls for the selected mode
 * @{
 */

/**
 * @see Delegates to SENSOR_FUNCS::GetRegister
 * @copydoc SENSOR_FUNCS::GetRegister
 *
 * @return STF_ERROR_INVALID_PARAMETERS if hHandle or pui32Exposure is NULL
 * @return STF_ERROR_FATAL if GetExposure function is not available for this
 * sensor
 */
extern
STF_RESULT Sensor_GetRegister(
    SENSOR_HANDLE *pstHandle,
    STF_U16 u16RegAddr,
    STF_U16 *pu16RegVal,
    STF_U8 u8Context
    );

/**
 * @see Delegates to SENSOR_FUNCS::SetRegister
 * @copydoc SENSOR_FUNCS::SetRegister
 *
 * @return STF_ERROR_INVALID_PARAMETERS if hHandle is NULL
 * @return STF_ERROR_FATAL if SetExposure function is not available for this
 * sensor
 */
extern
STF_RESULT Sensor_SetRegister(
    SENSOR_HANDLE *pstHandle,
    STF_U16 u16RegAddr,
    STF_U16 u16RegVal,
    STF_U8 u8Context
    );

/**
 * @}
 */
/*-----------------------------------------------------------------------------
 * End of the register access controls
 *---------------------------------------------------------------------------*/
/**
 * @name Exposure and gain controls
 * @brief Exposure and gain controls for the selected mode
 * @{
 */ 

/**
 * @see Delegates to SENSOR_FUNCS::GetGainRange
 * @copydoc SENSOR_FUNCS::GetGainRange
 *
 * @return STF_ERROR_INVALID_PARAMETERS if hHandle, pflMin, pflMax or
 * puiContexts is NULL
 * @return STF_ERROR_FATAL if GetGainRange function is not available for this
 * sensor
 */
extern
STF_RESULT Sensor_GetGainRange(
    SENSOR_HANDLE *pstHandle,
    STF_DOUBLE *pdMin,
    STF_DOUBLE *pdMax,
    STF_U8 *pu8Contexts
    );

/**
 * @see Delegates to SENSOR_FUNCS::GetCurrentGain
 * @copydoc SENSOR_FUNCS::GetCurrentGain
 *
 * @return STF_ERROR_INVALID_PARAMETERS if hHandle or pdCurrentGain is NULL
 * @return STF_ERROR_FATAL if GetCurrentGain function is not available for this
 * sensor
 */
extern
STF_RESULT Sensor_GetCurrentGain(
    SENSOR_HANDLE *pstHandle,
    STF_DOUBLE *pdCurrentGain,
    STF_U8 u8Context
    );

/**
 * @see Delegates to SENSOR_FUNCS::SetGain
 * @copydoc SENSOR_FUNCS::SetGain
 *
 * @return STF_ERROR_INVALID_PARAMETERS if hHandle is NULL
 * @return STF_ERROR_FATAL if SetGain function is not available for this sensor
 */
extern
STF_RESULT Sensor_SetGain(
    SENSOR_HANDLE *pstHandle,
    STF_DOUBLE dGain,
    STF_U8 u8Context
    );

/**
 * @see Delegates to SENSOR_FUNCS::GetExposureRange
 * @copydoc SENSOR_FUNCS::GetExposureRange
 *
 * @return STF_ERROR_INVALID_PARAMETERS if hHandle, pui32Min, pui32Max or
 * pui8Contexts is NULL
 * @return STF_ERROR_FATAL if GetExposureRange function is not available for
 * this sensor
 */
extern
STF_RESULT Sensor_GetExposureRange(
    SENSOR_HANDLE *pstHandle,
    STF_U32 *pu32Min,
    STF_U32 *pu32Max,
    STF_U8 *pu8Contexts
    );

/**
 * @see Delegates to SENSOR_FUNCS::GetExposure
 * @copydoc SENSOR_FUNCS::GetExposure
 *
 * @return STF_ERROR_INVALID_PARAMETERS if hHandle or pui32Exposure is NULL
 * @return STF_ERROR_FATAL if GetExposure function is not available for this
 * sensor
 */
extern
STF_RESULT Sensor_GetExposure(
    SENSOR_HANDLE *pstHandle,
    STF_U32 *pu32Exposure,
    STF_U8 u8Context
    );

/**
 * @see Delegates to SENSOR_FUNCS::SetExposure
 * @copydoc SENSOR_FUNCS::SetExposure
 *
 * @return STF_ERROR_INVALID_PARAMETERS if hHandle is NULL
 * @return STF_ERROR_FATAL if SetExposure function is not available for this
 * sensor
 */
extern
STF_RESULT Sensor_SetExposure(
    SENSOR_HANDLE *pstHandle,
    STF_U32 u32Exposure,
    STF_U8 u8Context
    );

/**
 * @}
 */
/*-----------------------------------------------------------------------------
 * End of the exposure/gain controls
 *---------------------------------------------------------------------------*/ 
/**
 * @name Focus controls
 * @brief Optional control of the focus of the sensor
 * @{
 */

/**
 * @see Delegates to SENSOR_FUNCS::GetFocusRange
 * @copydoc SENSOR_FUNCS::GetFocusRange
 *
 * @return STF_ERROR_INVALID_PARAMETERS if hHandle, pui16Min or pui16Max is
 * NULL
 * @return STF_ERROR_NOT_SUPPORTED if GetFocusRange function is not available
 * for this sensor as it is an optional implementation
 */
extern
STF_RESULT Sensor_GetFocusRange(
    SENSOR_HANDLE *pstHandle,
    STF_U16 *pu16Min,
    STF_U16 *pu16Max
    );

/**
 * @see Delegates to SENSOR_FUNCS::GetCurrentFocus
 * @copydoc SENSOR_FUNCS::GetCurrentFocus
 *
 * @return STF_ERROR_INVALID_PARAMETERS if hHandle or pui16Current is NULL
 * @return STF_ERROR_NOT_SUPPORTED if GetCurrentFocus function is not available
 * for this sensor as it is an optional implementation
 */
extern
STF_RESULT Sensor_GetCurrentFocus(
    SENSOR_HANDLE *pstHandle,
    STF_U16 *pu16CurrentFocus
    );

/**
 * @see Delegates to SENSOR_FUNCS::SetFocus
 * @copydoc SENSOR_FUNCS::SetFocus
 *
 * @return STF_ERROR_INVALID_PARAMETERS if hHandle is NULL
 * @return STF_ERROR_NOT_SUPPORTED if SetFocus function is not available for
 * this sensor as it is an optional implementation
 */
extern
STF_RESULT Sensor_SetFocus(
    SENSOR_HANDLE *pstHandle,
    STF_U16 u16Focus
    );

/**
 * @}
 */
/*-----------------------------------------------------------------------------
 * End of the focus controls
 *---------------------------------------------------------------------------*/
/**
 * @name Miscellaneous controls
 * @{
 */ 

/**
 * @see Delegates to SENSOR_FUNCS::ConfigureFlash
 * @copydoc SENSOR_FUNCS::ConfigureFlash
 *
 * @return STF_ERROR_INVALID_PARAMETERS if hHandle is NULL
 * @return STF_ERROR_NOT_SUPPORTED if ConfigureFlash function is not available
 * for this sensor as it is an optional implementation
 */
extern
STF_RESULT Sensor_ConfigureFlash(
    SENSOR_HANDLE *pstHandle,
    STF_BOOL8 bAlwaysOn,
    STF_S16 s16FrameDelay,
    STF_S16 s16Frames,
    STF_U16 u16FlashPulseWidth
    );

/**
 * @see Delegates to SENSOR_FUNCS::Insert
 * @copydoc SENSOR_FUNCS::Insert
 *
 * @return STF_ERROR_INVALID_PARAMETERS if hHandle is NULL
 * @return STF_ERROR_NOT_SUPPORTED if Insert function is not available for this
 * sensor as it is an optional implementation
 */
extern
STF_RESULT Sensor_Insert(
    SENSOR_HANDLE *pstHandle
    );

/**
 * @see Delegates to SENSOR_FUNCS::WaitProcessed
 * @copydoc SENSOR_FUNCS::WaitProcessed
 *
 * @return STF_ERROR_INVALID_PARAMETERS if hHandle is NULL
 * @return STF_ERROR_NOT_SUPPORTED if WaitProcessed function is not available
 * for this sensor as it is an optional implementation
 */
extern
STF_RESULT Sensor_WaitProcessed(
    SENSOR_HANDLE *pstHandle
    );

extern
STF_RESULT Sensor_SetFlipMirror(
    SENSOR_HANDLE *pstHandle,
    STF_U8 u8Flag
    );

extern
STF_RESULT Sensor_GetFixedFPS(
    SENSOR_HANDLE *pstHandle,
    STF_U16 *pu16FixedFps
    );

extern
STF_RESULT Sensor_SetFPS(
    SENSOR_HANDLE *pstHandle,
    STF_DOUBLE dFps
    );

extern
STF_RESULT Sensor_SetExposureAndGain(
    SENSOR_HANDLE *pstHandle,
    STF_U32 u32Exposure,
    STF_DOUBLE dGain,
    STF_U8 u8Context
    );

extern
STF_RESULT Sensor_SetResolution(
    SENSOR_HANDLE *pstHandle,
    STF_U16 u16ImgW,
    STF_U16 U16ImgH
    );

extern
STF_RESULT Sensor_GetSnapRes(
    SENSOR_HANDLE *pstHandle,
    ST_RES_LIST *pstReslist
    );

extern
STF_RESULT Sensor_Reset(
    SENSOR_HANDLE *pstHandle
    );

/**
 * @}
 */
/*-----------------------------------------------------------------------------
 * End of the miscellaneous controls
 *---------------------------------------------------------------------------*/ 
/**
 * @}
 */
/*-----------------------------------------------------------------------------
 * End of the SENSOR_API documentation module
 *---------------------------------------------------------------------------*/

/**
 * @page SENSOR_GUIDE Sensor Module Implementation Guide

 The Sensor API is intended to be easy to add custom sensors to. 
 The API defines a set of functions that a sensor driver should support to allow the Mantis AAA algorithms and applications to interact with the sensor to produce a correctly exposed image of the required format.

 # Sensor API

To add a sensor add an entry providing the sensor name to the Sensors array in mantis_dev_sensors.c and add a pointer to its Initialise function.


e.g.

    STF_RESULT (*InitialiseSensors[])(SENSOR_HANDLE **ppstHandle) =
    {
        DummyCamCreate,
        DGCamCreate,
        AR330CamCreate,
        P401CamCreate,
        NewCamCreate
    };

    char *Sensors[]={
        "Dummy Camera",
        "Data Generator",
        "AR330 Camera",
        "P401 Camera",
        "MyNewCam"
    };

The initialise function should fill in a /a SENSOR_FUNCS   structure to point to the functions used to control the sensor, and return this as a handle that the user can use to call the sensor control functions. In order to keep some state it is recommended to place this at the start of a larger camera context structure

e.g.

 @code typedef struct _newcam_struct
    {
      SENSOR_FUNCS sFuncs;
      STF_U16 u16CurrentMode;
      STF_BOOL8 bEnabled;
      STF_U32 u32ExposureMax;
      STF_U32 u32Exposure;
      STF_U32 u32ExposureMin;
      STF_U8 u8MipiLanes;
      SENSOR_I2C * psI2C;
      STF_DOUBLE dGain;
    }NEWCAM_STRUCT;

    STF_RESULT NewCamCreate(SENSOR_HANDLE **ppstHandle)
    {
        MYNEWCAM_STRUCT *psCam;

        psCam=(MYNEWCAMCAM_STRUCT *)STF_CALLOC(sizeof(MYNEWCAMCAM_STRUCT), 1);
        if(!psCam)
            return STF_ERROR_MALLOC_FAILED;

        *phHandle=(void*)psCam;
        psCam->sFuncs.GetSensorModes=MYNEWCAM_GetSensorModes;
        psCam->sFuncs.GetSensorState=MYNEWCAM_GetSensorState;
        psCam->sFuncs.SetSensorMode=MYNEWCAM_SetSensorMode;
        psCam->sFuncs.EnableSensor=MYNEWCAM_EnableSensor;
        psCam->sFuncs.DisableSensor=MYNEWCAM_DisableSensor;
        psCam->sFuncs.DestroySensor=MYNEWCAM_DestroySensor;

        //psCam->sFuncs.GetCurrentFocus stays NULL so is Unsupported;
        //psCam->sFuncs.GetFocusRange stays NULL so is Unsupported;
        //psCam->sFuncs.SetFocus stays NULL so is Unsupported;

        psCam->sFuncs.GetCurrentGain=MYNEWCAM_GetGain;
        psCam->sFuncs.GetGainRange=MYNEWCAM_GetGainRange;
        psCam->sFuncs.SetGain=MYNEWCAM_SetGain;

        psCam->sFuncs.SetExposure=MYNEWCAM_SetExposure;
        psCam->sFuncs.GetExposureRange=MYNEWCAM_GetExposureRange;
        psCam->sFuncs.GetExposure=MYNEWCAM_GetExposure;

        psCam->nEnabled=0;
        psCam->ui16CurrentMode=0;

        psCam->psI2C= SensorI2CInit(0,0x20,0x21,400*1000,STF_TRUE);

        LOG_INFO("Sensor initialised\n");
        return STF_SUCCESS;
    }
@endcode

The Initialise function will probably need to get a handle to the I2C subsytem as shown above, by calling the SensorI2CInit() function.
It can then use this to call the SensorI2CWrite8(), SensorI2CWrite16(),SensorI2CRead8(),SensorI2CRead16() functions and also to call the DoGasket() function which must be called just before a sensor is enabled


Other issues which may need to be taken into account.
If the SOC has an external gasket then this will need to be configured. In this case the doGasket function should be modified to do whatever is required




*/
/*-------------------------------------------------------------------------
 * End of the SENSOR_GUIDE documentation module
 *------------------------------------------------------------------------*/


#ifdef __cplusplus /* If this is a C++ compiler, end C linkage */
}
#endif


#endif // __SENSOR_API_H__
