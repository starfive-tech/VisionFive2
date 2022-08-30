/**
*******************************************************************************
 @file mc_structs.h

 @brief

 @copyright StarFive Technology Co., Ltd. All Rights Reserved.

 @license Strictly Confidential.
   No part of this software, either material or conceptual may be copied or
   distributed, transmitted, transcribed, stored in a retrieval system or
   translated into any human or computer language in any form by any means,
   electronic, mechanical, manual or other-wise, or disclosed to third
   parties without the express written permission of
   Shanghai StarFive Technology Co., Ltd.

******************************************************************************/
#ifndef MC_STRUCTS_H_
#define MC_STRUCTS_H_

#include "stf_common/pixel_format.h"
#include "ci/ci_api_structs.h"
#include "ci/ci_modules_structs.h"


#ifdef __cplusplus
extern "C" {
#endif

/**
 * @ingroup MC
 * @{
 */
/*-----------------------------------------------------------------------------
 * Following elements are in the MC documentation module
 *---------------------------------------------------------------------------*/

/**
 * @note Can be changed from double to float to change the precision of
 * computations
 */
#define MC_FLOAT double

/**
 * @name Main modules
 * @brief Modules that are not configuring statistics output
 * @{
 */
/*-----------------------------------------------------------------------------
 * Following elements are in the Main modules group
 *---------------------------------------------------------------------------*/


/**
 * @}
 */
/*-----------------------------------------------------------------------------
 * End of the Main modules group
 *---------------------------------------------------------------------------*/

/**
 * @name Statistics configuration
 * @brief Statistics configuration modules
 * @{
 */
/*-----------------------------------------------------------------------------
 * Following elements are in the Statistics modules group
 *---------------------------------------------------------------------------*/


/**
 * @}
 */
/*-----------------------------------------------------------------------------
 * End of the Statistics modules group
 *---------------------------------------------------------------------------*/

// no name group for pipeline as it's alone

//=== Godspeed === Add new memory/buffer type support here.
typedef struct MC_PIPELINE
{
    /**
     * given when initialising the structure - represents HW configuration
     */
    const CI_HWINFO *pHWInfo;

    /**
     * @brief can only be a YUV format
     *
     * @warning does not differentiate between YUV and YVU: HW configuration
     * is not changed by MC to match UV/VU order
     */
    ePxlFormat enUoOutput;
    /** @brief can only be a YUV format */
    ePxlFormat enSs0Output;
    /** @brief can only be a YUV format */
    ePxlFormat enSs1Output;
    /** @brief can only be Bayer RAW */
    ePxlFormat enDumpOutput;
    /** @brief can only be RGB/YUV/Bayer Raw format */
    ePxlFormat enTiling_1_RdInput;
    /** @brief can only be RGB/YUV/Bayer Raw format */
    ePxlFormat enTiling_1_WrOutput;

    STF_BOOL8 bEnableTimestamp;

    /**
     * @name Main modules
     * @{
     */


    /**
     * @}
     * @name Statistics modules
     * @{
     */


    /**
     * @}
     */
} MC_PIPELINE;

/**
 * @name Statistics output
 * @{
 */
/*-----------------------------------------------------------------------------
 * Following elements are in the statistics output group
 *---------------------------------------------------------------------------*/

//=== Godspeed === Add new memory/buffer type support here.
typedef struct MC_STATS_TIMESTAMP
{
    /**
     * @brief Start frame received at the input of the imager interface,
     * provided by HW
     */
    STF_U32 u32StartFrameIn;
    /**
     * @brief End of frame received at the input of the imager interface,
     * provided by HW
     */
    STF_U32 u32EndFrameIn;
    /**
     * @brief Start of frame sent out of the imager interface,
     * provided by HW
     */
    STF_U32 u32StartFrameOut;
    /**
     * @brief End frame sent out of the Uo memory interface,
     * provided by HW
     */
    STF_U32 u32EndFrameUoOut;
    /**
     * @brief End frame sent out to the Ss0 memory interface,
     * provided by HW
     */
    STF_U32 u32EndFrameSs0Out;
    /**
     * @brief End frame sent out to the Ss1 memory interface,
     * provided by HW
     */
    STF_U32 u32EndFrameSs1Out;
    /**
     * @brief End frame sent out to the Dump memory interface,
     * provided by HW
     */
    STF_U32 u32EndFrameDumpOut;
    /**
     * @brief End frame sent out to the Tiling 1 Read memory interface,
     * provided by HW
     */
    STF_U32 u32EndFrameTiling_1_RdIn;
    /**
     * @brief End frame sent out to the Tiling 1 Write memory interface,
     * provided by HW
     */
    STF_U32 u32EndFrameTiling_1_WrOut;
    /**
     * @brief Read by drivers when interrupt is serviced
     */
    STF_U32 u32InterruptServiced;
#ifdef SIFIVE_ISP

    STF_U64 u64CurSystemTS;
#endif //SIFIVE_ISP
} MC_STATS_TIMESTAMP;


/**
 * @}
 */
/*-----------------------------------------------------------------------------
 * End of the Statistics output group
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

#endif /* MC_STRUCTS_H_ */
