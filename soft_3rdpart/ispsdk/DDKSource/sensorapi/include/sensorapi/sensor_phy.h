/**
  ******************************************************************************
  * @file sensor_phy.h
  * @author  StarFive Isp Team
  * @version  V1.0
  * @date  08/12/2020
  * @brief Header describing the interface of imager's PHY and VIC gasket
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


#ifndef __SENSOR_PHY__
#define __SENSOR_PHY__


#ifdef __cplusplus
extern "C" {
#endif


#include "stf_types.h"


struct CI_GASKET; /* defined in ci_api_struct.h */
struct CI_CONNECTION; /* defined in ci_api_struct.h */


/**
 * @defgroup SENSOR_API_PHY Sensor PHY control
 * @ingroup SENSOR_API
 * @brief Control of the imager's PHY
 */
/*-----------------------------------------------------------------------------
 * Following elements are in the PHY documentation module
 *---------------------------------------------------------------------------*/
 
/**
 * @brief Control of the PHY
 *
 * @note May need to be re-implemented to support the actual customer's PHY
 */ 
typedef struct _sensor_phy_
{
#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
    STF_BOOL8 bHasCall_CI_DriverInit;
    STF_U8 u8IspIdx;
    /**
     * @brief Connection to the low level driver to access gasket reservation
     * functionalities
     */
    struct CI_CONNECTION *psConnection;
#endif //#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
    /**
     * @brief Gasket object used for reservation of the V2500 Gasket
     */
	struct CI_GASKET *psGasket;
} SENSOR_PHY, *PSENSOR_PHY;


/**
 * @brief Initialise the Gasket hardware
 */
SENSOR_PHY * SensorPhyInit(
#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
    CI_CONNECTION *pCIConnection,
    STF_U8 u8IspIdx
#else
    STF_VOID
#endif //#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
    );

/**
 * @brief Close the access to the I2Cdevice
 *
 * @param psSensorPhy the pointer to the sensor device to close
 */
STF_VOID SensorPhyDeinit(
    SENSOR_PHY *pstSensorPhy
    );

/**
 * @brief Turn on/off the imager gasket
 *
 * @param psSensorPhy The Internal Sensor structure being operated on.
 * @param bEnable flag for enable/disable
 * @param uiMipiLanes number of MIPI lanes to enable. Set to 0 for parallel 
 * imagers
 * @param ui8SensorNum value to write to the SENSOR_SELECT register of the PHY
 */
STF_RESULT SensorPhyCtrl(
    SENSOR_PHY *pstSensorPhy,
    STF_BOOL8 bEnable,
    STF_U8 u8MipiLanes,
    STF_U8 u8SensorNum
    );
		
/**
 * @brief Change bayer format
 *
 * @param psSensorPhy the pointer to the sensor device to close
 * @param bayerFmt the bayer format to switch to
 *
 * Add by feng.qu@infotm.com
 */
STF_RESULT SensorPhyBayerFormat(
    SENSOR_PHY *pstSensorPhy,
    enum MOSAICType bayerFmt
    );

/**
 * @brief discover the i2c device
 */
STF_RESULT FindI2cDev(
#ifdef SIFIVE_ISP
    STF_CHAR *szI2cDevName,
    STF_U32 u32Len,
    STF_U32 u32Addr,
    const STF_CHAR *i2c_adaptor
#else
    STF_CHAR *szI2cDevName,
    STF_U32 u32Len
#endif //SIFIVE_ISP
    );

/**
 * @}
 */
/*-----------------------------------------------------------------------------
 * End of PHY documentation module
 *---------------------------------------------------------------------------*/


#ifdef __cplusplus
}
#endif 


#endif // __SENSOR_PHY__
