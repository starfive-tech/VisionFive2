/**
  ******************************************************************************
  * @file  stfdrv_isp_sat.h
  * @author  StarFive Isp Team
  * @version  V1.0
  * @date  12/14/2019
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
  */
#ifndef __STFDRV_ISP_SAT_H__
#define __STFDRV_ISP_SAT_H__


#ifdef __cplusplus
extern "C" {
#endif


#include "stf_isp.h"
#include "ci/ci_api_structs.h"


/* Brightness, contrast, hue and saturation driver structure */


/* Brightness, contrast, hue and saturation driver interface */
//-----------------------------------------------------------------------------
extern
STF_RESULT STFDRV_ISP_SAT_SetReg(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    ST_ISP_SAT_RDMA *pstIspSatRdma
    );

#if defined(EXTEND_IOCTL_COMMAND_SAT)
//-----------------------------------------------------------------------------
extern
STF_RESULT STFDRV_ISP_SAT_SetSaturationGain(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    ST_SAT_GAIN_PARAM *pstSatGain
    );

extern
STF_RESULT STFDRV_ISP_SAT_SetSaturationThreshold(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    ST_SAT_THRESHOLD_PARAM *pstSatThreshold
    );

extern
STF_RESULT STFDRV_ISP_SAT_SetSaturationUVOffset(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    ST_SAT_UV_OFFSET_PARAM *pstSatUvOffset
    );

extern
STF_RESULT STFDRV_ISP_SAT_SetSaturationHueFactor(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    ST_SAT_HUE_FACTOR_PARAM *pstSatHueFactor
    );

extern
STF_RESULT STFDRV_ISP_SAT_SetSaturationScalingCoefficient(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    STF_U8 u8IspIdx,
    STF_U8 u8ScalingFactor
    );

extern
STF_RESULT STFDRV_ISP_SAT_SetSaturationYAdjustment(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    ST_SAT_Y_ADJ_PARAM *pstIspSatYAdj
    );

#endif //#if defined(EXTEND_IOCTL_COMMAND_SAT)
//-----------------------------------------------------------------------------


#ifdef __cplusplus
}
#endif


#endif //__STFDRV_ISP_SAT_H__

