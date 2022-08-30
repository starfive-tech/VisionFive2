/**
*******************************************************************************
 @file ci_modules_structs.h

 @brief definitions of the different modules HW configuration available to the
 outside world

 @copyright StarFive Technology Co., Ltd. All Rights Reserved.

 @license Strictly Confidential.
   No part of this software, either material or conceptual may be copied or
   distributed, transmitted, transcribed, stored in a retrieval system or
   translated into any human or computer language in any form by any means,
   electronic, mechanical, manual or other-wise, or disclosed to third
   parties without the express written permission of
   Shanghai StarFive Technology Co., Ltd.

******************************************************************************/
#ifndef CI_MODULES_STRUCTS_H_
#define CI_MODULES_STRUCTS_H_

#include "stf_types.h"
#include "stf_common/pixel_format.h"  // for bayer format

// defines some array sizes and if some blocks are available or not
#include "mantis_hw_info.h"  // NOLINT

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @ingroup CI_API_MOD
 * @{
 */
/*-----------------------------------------------------------------------------
 * Following elements are in the CI_API documentation module
 *---------------------------------------------------------------------------*/

/** @brief Fixed size of the CFA */
#define CI_CFA_WIDTH 2
/** @brief Fixed size of the CFA */
#define CI_CFA_HEIGHT 2

typedef struct CI_MODULE_IIF
{
    /**
     * @brief size of the imager (in CFA)
     *
     * @note Register: IMGR_OUT_SIZE:IMGR_OUT_ROWS_CFA and IMGR_OUT_COLS_CFA
     */
    STF_U16 u16ImagerSize[2];
#ifdef UNUSED_CODE_AND_VARIABLE

    /**
     * @brief Horizontal and vertical decimation of the image from the
     * imager (in CFA) - 0 means no decimation
     *
     * @note Register: IMGR_IMAGE_DECIMATION:IMGR_COMP_SKIP_X,
     * IMGR_IMAGE_DECIMATION:IMGR_COMP_SKIP_Y
     */
    STF_U16 u16ImagerDecimation[2];
#endif //UNUSED_CODE_AND_VARIABLE
} CI_MODULE_IIF;


/**
 * @}
 */
/*-----------------------------------------------------------------------------
 * End of the CI_API documentation module
 *---------------------------------------------------------------------------*/

#ifdef __cplusplus
}
#endif

#endif /* CI_MODULES_STRUCTS_H_ */
