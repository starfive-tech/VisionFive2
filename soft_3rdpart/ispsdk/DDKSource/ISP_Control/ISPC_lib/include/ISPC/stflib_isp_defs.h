/**
  ******************************************************************************
  * @file stflib_isp_defs.h
  * @author  StarFive Isp Team
  * @version  V1.0
  * @date  05/26/2020
  * @brief
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


#ifndef __STFLIB_ISP_DEFS__
#define __STFLIB_ISP_DEFS__


#ifdef __cplusplus
extern "C" {
#endif


#ifdef WIN32
#define NOMINMAX
#endif
#include <stddef.h>
#include <stdbool.h>
#include <math.h>
#ifdef USE_MATH_NEON
#include <mneon.h>
#endif
#include <limits.h>
#include "stf_include.h"
#include "stf_common/pixel_format.h"


/** creates a lot of comparison errors if using std::min */
#define ISPC_MIN(a, b) (((a) < (b)) ? (a) : (b))
/** creates a lot of comparison errors if using std::max */
#define ISPC_MAX(a, b) (((a) > (b)) ? (a) : (b))

/** @brief clip a value between 2 bounds */
#if 0
#define CLIP(value_type, bound_type, value, bound1, bound2)                         \
(                                                                                   \
    value_type value_clip(value_type value, bound_type bound1, bound_type bound2) { \
        if (bound2 < bound1) {                                                      \
            const bound_type tmp = bound1;                                          \
            bound1 = bound2;                                                        \
            bound2 = tmp;                                                           \
        }                                                                           \
        return ISPC_MIN(ISPC_MAX(bound1, value), bound2);                           \
    }                                                                               \
)
#elif 0
#define CLIP(value, bound1, bound2)                         \
({                                                          \
    (bound2 < bound1) ?                                     \
        (ISPC_MIN(ISPC_MAX(bound2, value), bound1)) :       \
        (ISPC_MIN(ISPC_MAX(bound1, value), bound2));        \
})
#elif 1
#define CLIP(value, bound1, bound2)                         \
({                                                          \
    typeof(value) __value = value;                          \
    typeof(value) __bound1 = bound1;                        \
    typeof(value) __bound2 = bound2;                        \
    if (__bound2 > __bound1) {                              \
        typeof(value) temp = __bound1;                      \
        __bound1 = __bound2;                                \
        __bound2 = temp;                                    \
    }                                                       \
    ISPC_MIN(ISPC_MAX(bound1, value), bound2);              \
})
#endif

// Some versions of VisualStudio don't have log2()
#ifdef USE_MATH_NEON
inline double LOG2(double val) { return ((double)log10f_neon((float)(val))) / ((double)log10f_neon(2.0f)); }
#else
inline double LOG2(double val) { return log10(val) / log10(2.0); }
#endif


/**
 * @brief context status definitions
 */
typedef enum __EN_CTX_STATUS {
    ISPC_Ctx_UNINIT = 0, /**< @brief Context not initialited */
    ISPC_Ctx_INIT, /**< @brief Context initialited */
    ISPC_Ctx_SETUP, /**< @brief Pipeline has been setup */
    ISPC_Ctx_READY, /**< @brief Pipeline is ready for capture */
    ISPC_Ctx_ERROR /**< @brief Pipeline is in error state */
} EN_CTX_STATUS, *PEN_CTX_STATUS;

typedef enum _EN_SCALER_RECT_TYPE {
    SCALER_RECT_CLIP = 0,
    SCALER_RECT_CROP,
    SCALER_RECT_SIZE
} EN_SCALER_RECT_TYPE, *PEN_SCALER_RECT_TYPE;


/**
 * @brief Global setup parameters that may be used in several places of the
 * pipeline.
 */
typedef struct _ST_GLOBAL_SETUP
{
    STF_U32 CFA_WIDTH;
    STF_U32 CFA_HEIGHT;

   /**
    * @name Sensor information
    * @{
    */

    /** @brief Input image width */
    STF_U32 u32Sensor_Width;
    /** @brief Input image height */
    STF_U32 u32Sensor_Height;
    /** @brief Input image bitdepth */
    STF_U32 u32Sensor_BitDepth;
    /** @brief IIF setup, imager ID */
    STF_U32 u32Sensor_Imager;
    /** @brief sensor gain - may change regularly */
    STF_DOUBLE dSensor_Gain;
    STF_U32 u32Sensor_WellDepth;
    /** @brief [enum] Layout of the CFA cells. One of:RGGB,GRBG,GBRG,BGGR. */
    eMOSAIC enSensor_BayerFormat;
    STF_DOUBLE dSensor_ReadNoise;

    /**
     * @}
     * @name Image size information
     * @note Managed by IIF module
     * @{
     */
    /* should it be removed because managed by IIF? or should IIF become
     * part of global? */

    /**
     * @brief Image width after decimation and cropping
     *
     * @note Derived from result of ModuleIIF::setup()
     */
    STF_U32 u32ImageWidth;
    /**
     * @brief Image height after decimation and cropping
     *
     * @note Derived from result of ModuleIIF::setup()
     */
    STF_U32 u32ImageHeight;

    /**
     * @}
     * @name Pipeline output width and height
     * @note Output size after the scaler
     * @{
     */

    //=== Godspeed === Add new memory/buffer type support here.
    /** @brief UO output format. */
    ePxlFormat enUoPixelFormat;
    /** @brief UO output width. */
    STF_U16 u16UoWidth;
    /** @brief UO output height. */
    STF_U16 u16UoHeight;

    /** @brief Ss0 output format. */
    ePxlFormat enSs0PixelFormat;
    /** @brief SS0 output width.  */
    STF_U16 u16Ss0Width;
    /** @brief SS0 output height. */
    STF_U16 u16Ss0Height;

    /** @brief Ss1 output format. */
    ePxlFormat enSs1PixelFormat;
    /** @brief SS1 output width.  */
    STF_U16 u16Ss1Width;
    /** @brief SS1 output height. */
    STF_U16 u16Ss1Height;

    /** @brief DUMP output format. */
    ePxlFormat enDumpPixelFormat;
    /** @brief DUMP output width. */
    STF_U16 u16DumpWidth;
    /** @brief DUMP output height. */
    STF_U16 u16DumpHeight;

    /** @brief TIL_1_RD input format. */
    ePxlFormat enTiling_1_RdPixelFormat;
    /** @brief TIL_1_RD input width. */
    STF_U16 u16Tiling_1_RdWidth;
    /** @brief TIL_1_RD input height. */
    STF_U16 u16Tiling_1_RdHeight;

    /** @brief TIL_1_WR output format. */
    ePxlFormat enTiling_1_WrPixelFormat;
    /** @brief TIL_1_WR output width. */
    STF_U16 u16Tiling_1_WrWidth;
    /** @brief TIL_1_WR output height. */
    STF_U16 u16Tiling_1_WrHeight;

    /**
     * @}
     * @name Other global parameters
     * @{
     */

    /**
     * @}
     */
} ST_GLOBAL_SETUP, *PST_GLOBAL_SETUP;


#ifdef __cplusplus
}
#endif


#endif //__STFLIB_ISP_DEFS__
