/**
*******************************************************************************
 @file module_config.h

 @brief ISP module configuration helper

 @copyright StarFive Technology Co., Ltd. All Rights Reserved.

 @license Strictly Confidential.
   No part of this software, either material or conceptual may be copied or
   distributed, transmitted, transcribed, stored in a retrieval system or
   translated into any human or computer language in any form by any means,
   electronic, mechanical, manual or other-wise, or disclosed to third
   parties without the express written permission of
   Shanghai StarFive Technology Co., Ltd.

******************************************************************************/
#ifndef MODULES_CONFIG_H_
#define MODULES_CONFIG_H_


#include "stf_types.h"
#include "ci/ci_modules_structs.h"
#include "ci/ci_api_structs.h"

#include "mc/mc_structs.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup MC Module Configuration (MC)
 * @ingroup CI_API
 * @brief Helping the configuration of the CI pipeline
 * @{
 */
/*-----------------------------------------------------------------------------
 * Following elements are in the MC documentation module
 *---------------------------------------------------------------------------*/


/**
 * @name Configuration functions
 * @brief Initialise the modules to have a default ready to use CI pipeline
 * @{
 */
/*-----------------------------------------------------------------------------
 * Following elements are in the Configuration functions group
 *---------------------------------------------------------------------------*/

/** @warning Only does a memset of 0 */

// statistics

/**
 * @name Extraction functions
 * @brief Convert the statistics blob information into structures (meta-data)
 * @{
 */
/*-----------------------------------------------------------------------------
 * Following elements are in the Extraction functions group
 *---------------------------------------------------------------------------*/

//=== Godspeed === Add new memory/buffer type support here.
extern
STF_RESULT MC_ScDumpExtract(const STF_VOID *blob, STF_VOID *pScDump);
extern
STF_RESULT MC_YHistExtract(const STF_VOID *blob, STF_VOID *pYHist);

/**
 * @}
 */
/*-----------------------------------------------------------------------------
 * End of the Configuration functions group
 *---------------------------------------------------------------------------*/

/**
 * @name Pipeline functions
 * @brief Initialise and convert a whole pipeline
 * @{
 */
/*-----------------------------------------------------------------------------
 * Following elements are in the Conversion functions group
 *---------------------------------------------------------------------------*/

/**
 * @}
 */
/*-----------------------------------------------------------------------------
 * End of the Configuration functions group
 *---------------------------------------------------------------------------*/

/**
 * @name Conversion functions
 * @brief Convert the logical level information into the register information
 * expected by CI
 * @{
 */
/*-----------------------------------------------------------------------------
 * Following elements are in the Conversion functions group
 *---------------------------------------------------------------------------*/

// statistics

/**
 * @}
 */
/*-----------------------------------------------------------------------------
 * End of the Conversion functions group
 *---------------------------------------------------------------------------*/

/**
 * @}
 */
/*-----------------------------------------------------------------------------
 * End of the MC documentation module
 *---------------------------------------------------------------------------*/

#ifdef __cplusplus
}
#endif

#endif /* MODULES_CONFIG_H_ */
