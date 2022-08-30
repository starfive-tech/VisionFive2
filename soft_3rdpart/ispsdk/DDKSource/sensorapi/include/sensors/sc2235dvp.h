/**
  ******************************************************************************
  * @file sc2235dvp.h
  * @author  StarFive Isp Team
  * @version  V1.0
  * @date  08/12/2020
  * @brief SC2235 camera sensor header file
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


#ifndef __SC2235_DVP__
#define __SC2235_DVP__

#include <stdio.h>
#include <fcntl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <sys/param.h>
#include <sys/ioctl.h>  

#include "stf_include.h"
#include "sensorapi/sensorapi.h"
#include "sensorapi/sensor_phy.h"
#include "ci/ci_api_structs.h"

#include "sensor_name.h"


#ifdef __cplusplus
extern "C" {
#endif


//struct _Sensor_Functions_; // define in sensorapi/sensorapi.h


/**
 * @ingroup SENSOR_API
 */
STF_RESULT SC2235DVP_Create(
#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
    CI_CONNECTION *pCIConnection,
    STF_U8 u8IspIdx,
#endif //#if defined(VIRTUAL_IO_MAPPING) && !defined(USE_LINUX_SYSTEM_STARTAND_I2C)
    SENSOR_HANDLE **ppstHandle,
    STF_U8 u8Index
    );

/**
 * @brief Load registers from a file as the special mode SC2135_SPECIAL_MODE
 * @ingroup SENSOR_API
 *
 * This allow to specify registers for the SC2135 at run time instead of
 * compiling them in. It is not expected to be used on a final product but
 * is there to ease the mode integration.
 *
 * @warning If the provided file is invalid or does not contain all the 
 * needed registers for the getModeInfo() function to get the correct
 * information this function may not realise it
 *
 * File must be formatted to have pairs for hexadecimal values as 
 * 'offset value' as the parser will not cope with other characters.
 *
 * The parser will stop when fscanf cannot find couples of hexadecimal data
 * any more.
 *
 * If file contains more than 1 couple it will override the present values for
 * the special mode.
 * If the loaded data makes getModeInfo() return errors the registers will be
 * discarded.
 *
 * @warning this feature is not robust - SHOULD NOT BE USED in real systems.
 * 
 * @return STF_SUCCESS
 * @return STF_ERROR_NOT_INITIALISED if psSensorPhy is NULL (object not
 * initialised correctly)
 * @return STF_ERROR_INVALID_PARAMETERS if failed to open filename
 * @return STF_ERROR_FATAL if failed to read at least 2 registers
 * @return STF_ERROR_NOT_SUPPORTED if getModeInfo() failed with loaded values
 */


#ifdef __cplusplus
}
#endif


#endif //__SC2235_DVP__
