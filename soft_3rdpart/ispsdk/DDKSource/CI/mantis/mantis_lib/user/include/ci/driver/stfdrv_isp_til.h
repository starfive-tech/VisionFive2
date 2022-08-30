/**
  ******************************************************************************
  * @file  stfdrv_isp_til.h
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
#ifndef __STFDRV_ISP_TIL_H__
#define __STFDRV_ISP_TIL_H__


#ifdef __cplusplus
extern "C" {
#endif


#include "stf_isp.h"
#include "ci/ci_api_structs.h"


/* tiling input/output image driver structure */


/* tiling input/output image driver interface */
//-----------------------------------------------------------------------------
extern
STF_RESULT STFDRV_ISP_TIL_SetReg(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    ST_ISP_TIL_RDMA *pstIspTilRdma
    );

extern
STF_RESULT STFDRV_ISP_TIL_GetAllBusyStatus(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    STF_U8 u8IspIdx
    );

extern
STF_RESULT STFDRV_ISP_TIL_1_SetWindowReg(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    ST_ISP_TIL_RDMA *pstIspTilRdma
    );

extern
STF_RESULT STFDRV_ISP_TIL_1_SetConfigReg(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    ST_ISP_TIL_RDMA *pstIspTilRdma
    );

extern
STF_RESULT STFDRV_ISP_TIL_1_SetReadReg(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    ST_ISP_TIL_RDMA *pstIspTilRdma
    );

extern
STF_RESULT STFDRV_ISP_TIL_1_SetWriteReg(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    ST_ISP_TIL_RDMA *pstIspTilRdma
    );

extern
STF_U32 STFDRV_ISP_TIL_1_GetBusyStatus(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    STF_U8 u8IspIdx
    );

#if defined(EXTEND_IOCTL_COMMAND_TIL)
extern
STF_BOOL8 STFDRV_ISP_TIL_1_IsReadBusy(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    STF_U8 u8IspIdx
    );

extern
STF_BOOL8 STFDRV_ISP_TIL_1_IsWriteBusy(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    STF_U8 u8IspIdx
    );

#endif //#if defined(EXTEND_IOCTL_COMMAND_TIL)
#if defined(EXTEND_IOCTL_COMMAND_TIL)
//-----------------------------------------------------------------------------
extern
STF_RESULT STFDRV_ISP_TIL_1_SetWindowSize(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    ST_ISP_TIL_RDMA *pstIspTilRdma
    );

extern
STF_RESULT STFDRV_ISP_TIL_1_SetReadBufAddr(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    ST_ISP_TIL_RDMA *pstIspTilRdma
    );

extern
STF_RESULT STFDRV_ISP_TIL_1_SetReadBufStride(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    ST_ISP_TIL_RDMA *pstIspTilRdma
    );

extern
STF_RESULT STFDRV_ISP_TIL_1_SetReadDataPath(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    ST_ISP_TIL_RDMA *pstIspTilRdma
    );

extern
STF_RESULT STFDRV_ISP_TIL_1_SetReadNv21(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    ST_ISP_TIL_RDMA *pstIspTilRdma
    );

extern
STF_RESULT STFDRV_ISP_TIL_1_SetReadDataFormat(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    ST_ISP_TIL_RDMA *pstIspTilRdma
    );

extern
STF_RESULT STFDRV_ISP_TIL_1_SetReadFlexibleBits(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    ST_ISP_TIL_RDMA *pstIspTilRdma
    );

extern
STF_RESULT STFDRV_ISP_TIL_1_SetReadIndependentRvi(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    ST_ISP_TIL_RDMA *pstIspTilRdma
    );

extern
STF_RESULT STFDRV_ISP_TIL_1_SetReadLineBufOff(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    ST_ISP_TIL_RDMA *pstIspTilRdma
    );

extern
STF_RESULT STFDRV_ISP_TIL_1_SetReadAxiId(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    ST_ISP_TIL_RDMA *pstIspTilRdma
    );

extern
STF_RESULT STFDRV_ISP_TIL_1_SetReadEnable(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    ST_ISP_TIL_RDMA *pstIspTilRdma
    );

extern
STF_RESULT STFDRV_ISP_TIL_1_SetWriteBufAddr(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    ST_ISP_TIL_RDMA *pstIspTilRdma
    );

extern
STF_RESULT STFDRV_ISP_TIL_1_SetWriteBufStride(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    ST_ISP_TIL_RDMA *pstIspTilRdma
    );

extern
STF_RESULT STFDRV_ISP_TIL_1_SetWriteDataPath(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    ST_ISP_TIL_RDMA *pstIspTilRdma
    );

extern
STF_RESULT STFDRV_ISP_TIL_1_SetWriteNv21(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    ST_ISP_TIL_RDMA *pstIspTilRdma
    );

extern
STF_RESULT STFDRV_ISP_TIL_1_SetWriteDataFormat(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    ST_ISP_TIL_RDMA *pstIspTilRdma
    );

extern
STF_RESULT STFDRV_ISP_TIL_1_SetWriteFlexibleBits(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    ST_ISP_TIL_RDMA *pstIspTilRdma
    );

extern
STF_RESULT STFDRV_ISP_TIL_1_SetWriteAxiId(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    ST_ISP_TIL_RDMA *pstIspTilRdma
    );

extern
STF_RESULT STFDRV_ISP_TIL_1_SetWriteEnable(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    ST_ISP_TIL_RDMA *pstIspTilRdma
    );

extern
STF_RESULT STFDRV_ISP_TIL_1_SetIspRaw10InDW(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    ST_ISP_TIL_RDMA *pstIspTilRdma
    );

extern
STF_RESULT STFDRV_ISP_TIL_1_SetLoopbackMode(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    ST_ISP_TIL_RDMA *pstIspTilRdma
    );

extern
STF_RESULT STFDRV_ISP_TIL_1_SetDmaMode(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    ST_ISP_TIL_RDMA *pstIspTilRdma
    );

#endif //#if defined(EXTEND_IOCTL_COMMAND_TIL)
//-----------------------------------------------------------------------------


#ifdef __cplusplus
}
#endif


#endif //__STFDRV_ISP_TIL_H__

