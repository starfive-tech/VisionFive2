/**
  ******************************************************************************
  * @file  stfdrv_isp_sud_isp.h
  * @author  StarFive Isp Team
  * @version  V1.0
  * @date  01/05/2021
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
#ifndef __STFDRV_ISP_SUD_ISP_H__
#define __STFDRV_ISP_SUD_ISP_H__


#ifdef __cplusplus
extern "C" {
#endif


#include "stf_isp.h"
#include "ci/ci_api_structs.h"


/* ISP shadow update driver structure */


/* ISP shadow update driver interface */
//-----------------------------------------------------------------------------
extern
STF_RESULT STFDRV_ISP_SUD_ISP_SetReg(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    ST_ISP_SUD_ISP_RDMA *pstIspSudIspRdma
    );

//-----------------------------------------------------------------------------
extern
STF_RESULT STFDRV_ISP_SUD_ISP_SetShadowEnable(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    ST_ISP_SUD_ISP_RDMA *pstIspSudIspRdma
    );

#if defined(EXTEND_IOCTL_COMMAND_SUD_ISP)
// Since the shadow update enable bit cannot be read back,
// therefore disable below feature support.
#if defined(ENABLE_SET_SHADOW_MODE_SUPPORT)
extern
STF_RESULT STFDRV_ISP_SUD_ISP_SetShadowMode(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    ST_ISP_SUD_ISP_RDMA *pstIspSudIspRdma
    );

#endif //#if defined(ENABLE_SET_SHADOW_MODE_SUPPORT)
extern
STF_RESULT STFDRV_ISP_SUD_ISP_SetShadowImmediatelyAndEnable(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    STF_U8 u8IspIdx
    );

extern
STF_RESULT STFDRV_ISP_SUD_ISP_SetShadowNextVSyncAndEnable(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    STF_U8 u8IspIdx
    );

extern
STF_RESULT STFDRV_ISP_SUD_ISP_SetShadowImmediatelyAndTrigger(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    STF_U8 u8IspIdx
    );

extern
STF_RESULT STFDRV_ISP_SUD_ISP_SetShadowNextVSyncAndTrigger(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    STF_U8 u8IspIdx
    );

extern
STF_RESULT STFDRV_ISP_SUD_ISP_TriggerShadow(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    STF_U8 u8IspIdx
    );

#endif //#if defined(EXTEND_IOCTL_COMMAND_SUD_ISP)
//-----------------------------------------------------------------------------


#ifdef __cplusplus
}
#endif


#endif //__STFDRV_ISP_SUD_ISP_H__

