/**
 *******************************************************************************
  @file ci_api.h

  @brief User-side driver functions

  @copyright StarFive Technology Co., Ltd. All Rights Reserved.

  @license Strictly Confidential.
    No part of this software, either material or conceptual may be copied or
    distributed, transmitted, transcribed, stored in a retrieval system or
    translated into any human or computer language in any form by any means,
    electronic, mechanical, manual or other-wise, or disclosed to third
    parties without the express written permission of
    Shanghai StarFive Technology Co., Ltd.

 ******************************************************************************/
#ifndef __CI_API_H__
#define __CI_API_H__


#include "stf_types.h"
#include "stf_defs.h"

#include "ci/ci_api_structs.h"
#include "ci/driver/stfdrv_gen_base.h"
#include "ci/driver/stfdrv_isp_base.h"
#include "ci/driver/stfdrv_isp_sud_csi.h"
#include "ci/driver/stfdrv_isp_sud_isp.h"
#include "ci/driver/stfdrv_isp_csi_in.h"
#include "ci/driver/stfdrv_isp_cbar.h"
#include "ci/driver/stfdrv_isp_crop.h"
#include "ci/driver/stfdrv_isp_dc.h"
#include "ci/driver/stfdrv_isp_dec.h"
#include "ci/driver/stfdrv_isp_oba.h"
#include "ci/driver/stfdrv_isp_obc.h"
#include "ci/driver/stfdrv_isp_lcbq.h"
#include "ci/driver/stfdrv_isp_sc.h"
#include "ci/driver/stfdrv_isp_dump.h"
#include "ci/driver/stfdrv_isp_isp_in.h"
#include "ci/driver/stfdrv_isp_dbc.h"
#include "ci/driver/stfdrv_isp_ctc.h"
#include "ci/driver/stfdrv_isp_oecf.h"
#include "ci/driver/stfdrv_isp_oecfhm.h"
#include "ci/driver/stfdrv_isp_lccf.h"
#include "ci/driver/stfdrv_isp_awb.h"
#include "ci/driver/stfdrv_isp_cfa_ahd.h"
#include "ci/driver/stfdrv_isp_car.h"
#include "ci/driver/stfdrv_isp_ccm.h"
#include "ci/driver/stfdrv_isp_gmargb.h"
#include "ci/driver/stfdrv_isp_r2y.h"
#include "ci/driver/stfdrv_isp_yhist.h"
#include "ci/driver/stfdrv_isp_ycrv.h"
#include "ci/driver/stfdrv_isp_shrp.h"
#include "ci/driver/stfdrv_isp_dnyuv.h"
#include "ci/driver/stfdrv_isp_sat.h"
#include "ci/driver/stfdrv_isp_out.h"
#include "ci/driver/stfdrv_isp_til.h"
#include "ci/driver/stfdrv_isp_buf.h"
#if defined(V4L2_DRIVER)
#include "ci/driver/stfdrv_v4l_base.h"
#endif //#if defined(V4L2_DRIVER)


#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup CI_API Capture Interface: library documentation
 * @brief Documentation of the Capture Interface user-side driver.
 *
 * Other pages that may be useful:
 * @li @ref KRN_CI_API
 * @li @ref MC
 */
/*-----------------------------------------------------------------------------
 * Following elements are in the CI_API documentation module
 *---------------------------------------------------------------------------*/

/**
 * @defgroup CI_API_DRIVER Driver functions (CI_Driver)
 * @ingroup CI_API
 * @brief Manage the connection and global interaction with the kernel-side
 * driver
 * @{
 */
/*-----------------------------------------------------------------------------
 * Following elements are in the Connection functions group
 *---------------------------------------------------------------------------*/

/**
 * @brief Initialise the drivers and return a handle to the connection
 *
 * @note Does not fail if the driver is already initialise. It only returns a
 * handle to the driver.
 *
 * @warning If the CI is compiled as a fake kernel module the internal function
 * KRN_CI_DriverCreate() should be called prior to that function.
 *
 * @param[in,out] ppConnection returned pointer to a capture interface handle
 *
 * @return STF_SUCCESS
 * @return STF_ERROR_INVALID_PARAMETERS if ppConnection is NULL
 * @return STF_ERROR_ALREADY_INITIALISED if the connection is already
 * established
 * @return STF_ERROR_FATAL if size check failed
 */
STF_RESULT CI_DriverInit(
    CI_CONNECTION **ppConnection,
    STF_CHAR *szDeviceName,
    STF_BOOL8 bIsVideoDevice,
    STF_BOOL8 bIsDrmDevice,
    STF_BOOL8 bIsDrmDeviceAllocDmaBuf
    );

/**
 * @brief Free the connection
 *
 * @warning If the CI is compiled as a fake kernel module the internal function
 * KRN_CI_DriverDestroy() should be called after that function.
 *
 * @return STF_SUCCESS
 * @return STF_ERROR_INVALID_PARAMETERS if pConnection is NULL
 */
STF_RESULT CI_DriverFinalise(
    CI_CONNECTION *pConnection
    );

STF_RESULT CI_DriverFileCreate(
    CI_ION_CONNECTION **ppIonConnection,
    STF_INT nFileDesc
    );

int CI_DriverFileGetHandle(
    CI_ION_CONNECTION *pIonConnection
    );

STF_RESULT CI_DriverFileRelease(
    CI_ION_CONNECTION *pIonConnection
    );


/**
 * @}
 */
/*-----------------------------------------------------------------------------
 * End of the Driver functions group
 *---------------------------------------------------------------------------*/
/**
 * @defgroup CI_API_PIPELINE Pipeline functions
 * @ingroup CI_API
 * @brief Pipeline configuration and capture controls
 */
/**
 * @name Pipeline configuration functions (CI_Pipeline)
 * @ingroup CI_API_PIPELINE
 * @brief Functions to manipulate the pipeline configuration
 * @{
 */
/*-----------------------------------------------------------------------------
 * Following elements are in the CI_Pipeline configuration functions group
 *---------------------------------------------------------------------------*/

/**
 * @brief Extract the statistics configuration from the shot's load structure
 *
 * @warning The shot has to be acquired and not released yet
 *
 * @param[in] pShot
 * @param[out] pConfig configuration of the stats modules from load structure
 * values and enabled outputs
 *
 * @return STF_SUCCESS
 * @return STF_ERROR_NOT_INITIALISED if pShot was not acquired beforehand
 */
STF_RESULT CI_PipelineExtractConfig(const CI_SHOT *pShot,
    CI_PIPELINE_STATS *pConfig);

/**
 * @}
 */
/*-----------------------------------------------------------------------------
 * End of the CI_Pipeline capture functions group
 *---------------------------------------------------------------------------*/

/**
 * @defgroup CI_API_GASKET Gasket functions (CI_Gasket)
 * @ingroup CI_API
 * @brief Manage the Gasket
 * @{
 */
/*-----------------------------------------------------------------------------
 * Following elements are in the Gasket functions group
 *---------------------------------------------------------------------------*/

STF_RESULT CI_GasketInit(CI_GASKET *pGasket);

/**
 * @}
 */
 /*----------------------------------------------------------------------------
 * End of the Gasket functions group
 *---------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
 * End of the CI_API documentation module
 *---------------------------------------------------------------------------*/


#ifdef __cplusplus
}
#endif


#endif /* __CI_API_H__ */
