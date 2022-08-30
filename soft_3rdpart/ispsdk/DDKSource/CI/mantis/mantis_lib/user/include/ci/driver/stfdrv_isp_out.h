/**
  ******************************************************************************
  * @file  stfdrv_isp_out.h
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
#ifndef __STFDRV_ISP_OUT_H__
#define __STFDRV_ISP_OUT_H__


#ifdef __cplusplus
extern "C" {
#endif


#include "stf_isp.h"
#include "ci/ci_api_structs.h"


/* ISP video output driver structure */


/* ISP video output driver interface */
//-----------------------------------------------------------------------------
extern
STF_RESULT STFDRV_ISP_OUT_SetReg(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    ST_ISP_OUT_RDMA *pstIspOutRdma
    );

#if defined(EXTEND_IOCTL_COMMAND_OUT)
//-----------------------------------------------------------------------------
extern
STF_RESULT STFDRV_ISP_OUT_SetUoInfo(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    ST_OUT_UO_INFO_PARAM *pstIspOutUoInfo
    );

extern
STF_RESULT STFDRV_ISP_OUT_SetSs0Info(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    ST_OUT_SS_INFO_PARAM *pstIspOutSs0Info
    );

extern
STF_RESULT STFDRV_ISP_OUT_SetSs1Info(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    ST_OUT_SS_INFO_PARAM *pstIspOutSs1Info
    );

#endif //#if defined(EXTEND_IOCTL_COMMAND_OUT)
//-----------------------------------------------------------------------------

/* ISP video output UO driver interface */
//-----------------------------------------------------------------------------
extern
STF_RESULT STFDRV_ISP_OUT_UO_SetReg(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    ST_ISP_OUT_UO_RDMA *pstIspOutUoRdma
    );

#if defined(EXTEND_IOCTL_COMMAND_OUT_UO)
//-----------------------------------------------------------------------------

#endif //#if defined(EXTEND_IOCTL_COMMAND_OUT_UO)
//-----------------------------------------------------------------------------

/* ISP video output SS0 driver interface */
//-----------------------------------------------------------------------------
extern
STF_RESULT STFDRV_ISP_OUT_SS0_SetReg(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    ST_ISP_OUT_SS0_RDMA *pstIspOutSS0Rdma
    );

#if defined(EXTEND_IOCTL_COMMAND_OUT_SS0)
//-----------------------------------------------------------------------------

#endif //#if defined(EXTEND_IOCTL_COMMAND_OUT_SS0)
//-----------------------------------------------------------------------------

/* ISP video output SS1 driver interface */
//-----------------------------------------------------------------------------
extern
STF_RESULT STFDRV_ISP_OUT_SS1_SetReg(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    ST_ISP_OUT_SS1_RDMA *pstIspOutSs1Rdma
    );

#if defined(EXTEND_IOCTL_COMMAND_OUT_SS1)
//-----------------------------------------------------------------------------

#endif //#if defined(EXTEND_IOCTL_COMMAND_OUT_SS1)
//-----------------------------------------------------------------------------


#ifdef __cplusplus
}
#endif


#endif //__STFDRV_ISP_OUT_H__

