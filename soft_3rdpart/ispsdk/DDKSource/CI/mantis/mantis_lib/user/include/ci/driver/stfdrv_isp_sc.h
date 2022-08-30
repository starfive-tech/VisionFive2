/**
  ******************************************************************************
  * @file  stfdrv_isp_sc.h
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
#ifndef __STFDRV_ISP_SC_H__
#define __STFDRV_ISP_SC_H__


#ifdef __cplusplus
extern "C" {
#endif


#include "stf_isp.h"
#include "ci/ci_api_structs.h"


/* statistics driver structure */


/* statistics driver interface */
//-----------------------------------------------------------------------------
extern
STF_RESULT STFDRV_ISP_SC_SetReg(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    ST_ISP_SC_RDMA *pstIspScRdma
    );

#if defined(EXTEND_IOCTL_COMMAND_SC)
extern
STF_RESULT STFDRV_ISP_SC_SetCfgReg(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    ST_ISP_SC_RDMA *pstIspScRdma
    );

extern
STF_RESULT STFDRV_ISP_SC_SetCropReg(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    ST_ISP_SC_RDMA *pstIspScRdma
    );

extern
STF_RESULT STFDRV_ISP_SC_SetDecReg(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    ST_ISP_SC_RDMA *pstIspScRdma
    );

extern
STF_RESULT STFDRV_ISP_SC_SetAeAfReg(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    ST_ISP_SC_RDMA *pstIspScRdma
    );

extern
STF_RESULT STFDRV_ISP_SC_SetAwbReg(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    ST_ISP_SC_RDMA *pstIspScRdma
    );

//-----------------------------------------------------------------------------
extern
STF_RESULT STFDRV_ISP_SC_SetDumpingAddress(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    STF_U8 u8IspIdx,
    STF_U32 u32Address
    );

extern
STF_RESULT STFDRV_ISP_SC_SetAxiId(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    STF_U8 u8IspIdx,
    STF_U8 u8AxiId
    );

extern
STF_RESULT STFDRV_ISP_SC_SetAeAfFrameCropping(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    ST_SC_FRAME_CROPPING *pstScFrameCropping
    );

extern
STF_RESULT STFDRV_ISP_SC_SetAeAfConfig(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    ST_SC_AE_AF_CFG_PARAM *pstScAeAfCfg
    );

extern
STF_RESULT STFDRV_ISP_SC_SetAeAfDecimation(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    ST_SC_DECIMATION *pstScDecimation
    );

extern
STF_RESULT STFDRV_ISP_SC_SetAfConfig(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    ST_SC_AF_CFG_PARAM *pstScAfCfg
    );

extern
STF_RESULT STFDRV_ISP_SC_SetAwbFrameCropping(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    ST_SC_FRAME_CROPPING *pstScFrameCropping
    );

extern
STF_RESULT STFDRV_ISP_SC_SetAwbConfig(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    ST_SC_AWB_CFG_PARAM *pstScAwbCfg
    );

extern
STF_RESULT STFDRV_ISP_SC_SetAwbDecimation(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    ST_SC_DECIMATION *pstScDecimation
    );

extern
STF_RESULT STFDRV_ISP_SC_SetAwbPixelSumRangeR(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    ST_SC_RANGE *pstScRange
    );

extern
STF_RESULT STFDRV_ISP_SC_SetAwbPixelSumRangeG(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    ST_SC_RANGE *pstScRange
    );

extern
STF_RESULT STFDRV_ISP_SC_SetAwbPixelSumRangeB(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    ST_SC_RANGE *pstScRange
    );

extern
STF_RESULT STFDRV_ISP_SC_SetAwbPixelSumRangeY(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    ST_SC_RANGE *pstScRange
    );

extern
STF_RESULT STFDRV_ISP_SC_SetAwbPixelSumRangeGR(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    ST_SC_RANGE *pstScRange
    );

extern
STF_RESULT STFDRV_ISP_SC_SetAwbPixelSumRangeGB(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    ST_SC_RANGE *pstScRange
    );

extern
STF_RESULT STFDRV_ISP_SC_SetAwbPixelSumRangeGRB(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    ST_SC_RANGE *pstScRange
    );

extern
STF_RESULT STFDRV_ISP_SC_SetAwbWeightSumRangeR(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    ST_SC_RANGE *pstScRange
    );

extern
STF_RESULT STFDRV_ISP_SC_SetAwbWeightSumRangeGr(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    ST_SC_RANGE *pstScRange
    );

extern
STF_RESULT STFDRV_ISP_SC_SetAwbWeightSumRangeGb(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    ST_SC_RANGE *pstScRange
    );

extern
STF_RESULT STFDRV_ISP_SC_SetAwbWeightSumRangeB(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    ST_SC_RANGE *pstScRange
    );

extern
STF_RESULT STFDRV_ISP_SC_SetAwbWeightTable(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    ST_SC_AWB_WT_TBL_PARAM *pstScAwbWeightingTable
    );

extern
STF_RESULT STFDRV_ISP_SC_SetAwbIntensityWeightCurve(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    ST_SC_AWB_INTSTY_WT_CRV_PARAM *pstScAwbIntensityWeightCurve
    );

#endif //#if defined(EXTEND_IOCTL_COMMAND_SC)
//-----------------------------------------------------------------------------
extern
STF_RESULT STFDRV_ISP_SC_SetDumpingResultControl(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    ST_SC_DUMP_RSLT_CTRL_PARAM *pstScDumpResultCtrl
    );

#if defined(EXTEND_IOCTL_COMMAND_SC)
extern
STF_U32 STFDRV_ISP_SC_GetResult(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    STF_U8 u8IspIdx
    );

#endif //#if defined(EXTEND_IOCTL_COMMAND_SC)
extern
STF_RESULT STFDRV_ISP_SC_GetResults(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    ST_SC_RSLTS_PARAM *pstScResults
    );

//-----------------------------------------------------------------------------


#ifdef __cplusplus
}
#endif


#endif //__STFDRV_ISP_SC_H__

