/**
 *******************************************************************************
  @file ci_alloc_info.h

  @brief Functions and structures to compute the allocation size for pixel
  formats

  @copyright StarFive Technology Co., Ltd. All Rights Reserved.

  @license Strictly Confidential.
    No part of this software, either material or conceptual may be copied or
    distributed, transmitted, transcribed, stored in a retrieval system or
    translated into any human or computer language in any form by any means,
    electronic, mechanical, manual or other-wise, or disclosed to third
    parties without the express written permission of
    Shanghai StarFive Technology Co., Ltd.

 ******************************************************************************/
#ifndef __CI_ALLOC_INFO_H__
#define __CI_ALLOC_INFO_H__


#include "stf_types.h"
#include "stf_common/pixel_format.h"


#ifdef __cplusplus
extern "C" {
#endif


// MANTIS_COMMON defined in pixel_format.h
/**
 * @defgroup CI_ALLOC CI Allocation information
 * @brief Allocation information (size and strides) per format
 * @ingroup MANTIS_COMMON
 * @{
 */
/*-----------------------------------------------------------------------------
 * Following elements are in CI_ALLOC documentation group
 *---------------------------------------------------------------------------*/

/**
 * @note total allocation = u32Stride*u32Height + u32CbStride*u32CbHeight
 */
struct CI_SIZEINFO
{
    enum pxlFormat eFmt;
    /** @brief buffer stride in Bytes (in YUV case is Y buffer stride) */
    STF_U32 u32Stride;
    /** @brief buffer height in lines (in YUV case is Y buffer height) */
    STF_U32 u32Height;

    /** @brief != 0 in YUV case only - combined Cb/Cr buffer stride in Bytes */
    STF_U32 u32CStride;
    /** @brief != 0 in YUV case only - combined Cb/Cr buffer height in lines */
    STF_U32 u32CHeight;
};

/**
 * @brief Access the compile time value for system memory alignment
 *
 * @return [bytes] data structures in system memory must be aligned to an
 * address which is a multiple of this
 */
STF_U32 CI_ALLOC_GetSysmemAlignment(void);

/**
 * @brief Get YUV buffer size information
 *
 * @param[in] pType format - it is not checked if it is YUV
 * @param[in] u32Width in pixels of the buffer
 * @param[in] u32Height in lines of the buffer
 * @param[out] pResult output information about the buffer
 *
 * @return STF_SUCCESS
 * @return STF_ERROR_NOT_SUPPORTED if selected tiling setup needed is not
 * possible
 */
STF_RESULT CI_ALLOC_YUVSizeInfo(
    const STF_BOOL8 bIsAlign_128,
    const PIXELTYPE *pType,
    STF_U32 u32Width,
    STF_U32 u32Height,
    struct CI_SIZEINFO *pResult
    );

/**
 * @brief Get RGB buffer size information
 *
 * @param[in] pType format - it is not checked if it is RGB
 * @note HDR insertion only supports @ref BGR_161616_64 and cannot be tiled
 * @param[in] u32Width in pixels of the buffer
 * @param[in] u32Height in lines of the buffer
 * @ref BGR_161616_64!
 * @param[out] pResult output information about the buffer
 *
 * @return STF_SUCCESS
 * @return STF_ERROR_NOT_SUPPORTED if selected tiling setup needed is not
 * possible
 *
 * @note also works for Bayer (RGGB) formats and HDR extraction (special RGB)
 */
STF_RESULT CI_ALLOC_RGBSizeInfo(
    const STF_BOOL8 bIsAlign_128,
    const PIXELTYPE *pType,
    STF_U32 u32Width,
    STF_U32 u32Height,
    struct CI_SIZEINFO *pResult
    );

/*-----------------------------------------------------------------------------
 * End of CI_ALLOC documentation group
 *---------------------------------------------------------------------------*/
/**
 * @}
 */


#ifdef __cplusplus
}
#endif


#endif // __CI_ALLOC_INFO_H__
