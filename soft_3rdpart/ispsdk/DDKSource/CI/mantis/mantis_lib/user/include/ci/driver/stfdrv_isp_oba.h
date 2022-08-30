/**
  ******************************************************************************
  * @file  stfdrv_isp_oba.h
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
#ifndef __STFDRV_ISP_OBA_H__
#define __STFDRV_ISP_OBA_H__


#ifdef __cplusplus
extern "C" {
#endif


#include "stf_isp.h"
#include "ci/ci_api_structs.h"


/* optical black statistics driver structure */


/* optical black statistics driver interface */
//-----------------------------------------------------------------------------
extern
STF_RESULT STFDRV_ISP_OBA_SetReg(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    ST_ISP_OBA_RDMA *pstIspObaRdma
    );

extern
ST_OBA_ACC STFDRV_ISP_OBA_GetAcc(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    STF_U8 u8IspIdx
    );

#if defined(EXTEND_IOCTL_COMMAND_OBA)
//-----------------------------------------------------------------------------
extern
STF_RESULT STFDRV_ISP_OBA_SetBlackCropping(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    ST_ISP_OBA_CROP_PARAM *pstIspObaCrop
    );

extern
STF_RESULT STFDRV_ISP_OBA_GetAccumulation(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    ST_ISP_OBA_ACCUMULATION_PARAM *pstIspObaAccumulation
    );

#endif //#if defined(EXTEND_IOCTL_COMMAND_OBA)
//-----------------------------------------------------------------------------


#ifdef __cplusplus
}
#endif


#endif //__STFDRV_ISP_DEC_H__

