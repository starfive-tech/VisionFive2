/**
  ******************************************************************************
  * @file  stflib_isp_base.h
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
  ******************************************************************************/


#ifndef __STFLIB_ISP_BASE_H__
#define __STFLIB_ISP_BASE_H__


#ifdef __cplusplus
extern "C" {
#endif


#include "stf_include.h"
#include "ci/ci_api_structs.h"
#if defined(V4L2_DRIVER)
#include "ISPC/stflib_isp_sensor.h"
#endif //#if defined(V4L2_DRIVER)


typedef enum _EN_DEV_ID EN_DEV_ID;


typedef struct _ST_CI_DEVICE ST_CI_DEVICE;


/* ISP base library structure */
//=== Godspeed === Add new memory/buffer type support here.
typedef struct _ST_MEMS_INFO {
    STF_U8 u8IspIdx;
    ST_SIZE stImgSize;
    CI_MEM_PARAM *pstUoMem;
    CI_MEM_PARAM *pstSs0Mem;
    CI_MEM_PARAM *pstSs1Mem;
    CI_MEM_PARAM *pstDumpMem;
    CI_MEM_PARAM *pstTiling_1_RdMem;
    CI_MEM_PARAM *pstTiling_1_WrMem;
    CI_MEM_PARAM *pstScDumpMem;
    CI_MEM_PARAM *pstYHistMem;
} ST_MEMS_INFO, *PST_MEMS_INFO;


/* ISP base library variables */
extern const STF_U64 g_u64ModuleMask[];
extern const STF_CHAR g_szModuleMask[][20];
extern const STF_CHAR g_szAllocBufferType[][32];
extern const STF_CHAR g_szFormatTypes[][20];
extern const STF_CHAR g_szPackedStart[][20];


/* ISP base library interface */
//-----------------------------------------------------------------------------
extern
STF_RESULT STFLIB_ISP_BASE_AssignIspDevicePointer(
    ST_CI_DEVICE *pstDevice,
    EN_DEV_ID enDevId
    );

extern
STF_RESULT STFLIB_ISP_BASE_RemoveIspDevicePointer(
    EN_DEV_ID enDevId
    );

extern
const STF_BOOL8 STFLIB_ISP_BASE_IsIspDeviceOpen(
    EN_DEV_ID enDevId
    );

extern
STF_RESULT STFLIB_ISP_BASE_Connection(
    STF_U8 u8IspIdx
    );

extern
STF_RESULT STFLIB_ISP_BASE_Disconnection(
    STF_U8 u8IspIdx
    );

extern
const STF_BOOL8 STFLIB_ISP_BASE_IsConnected(
    STF_U8 u8IspIdx
    );

extern
const STF_S32 STFLIB_ISP_BASE_GetConnections(
    STF_U8 u8IspIdx
    );

extern
const CI_CONNECTION *STFLIB_ISP_BASE_GetConnection(
    STF_U8 u8IspIdx
    );

extern
const int STFLIB_ISP_BASE_GetFileHandle(
    STF_U8 u8IspIdx
    );

extern
STF_RESULT STFLIB_ISP_BASE_DisconnectionAll(
    STF_U8 u8IspIdx
    );

//-----------------------------------------------------------------------------
extern
STF_VOID STFLIB_ISP_RegDebugInfoEnable(
    STF_BOOL8 bRegDebugInfoEnable
    );

extern
STF_RESULT STFLIB_ISP_RegReadOne(
    STF_U8 u8IspIdx,
    STF_U32 u32RegAddr,
    STF_U32 *pu32RegBuf
    );

extern
STF_RESULT STFLIB_ISP_RegWriteOne(
    STF_U8 u8IspIdx,
    STF_U32 u32RegAddr,
    STF_U32 *pu32RegBuf
    );

extern
STF_RESULT STFLIB_ISP_RegReadSeries(
    STF_U8 u8IspIdx,
    STF_U32 u32RegAddr,
    STF_U32 u32Items,
    STF_U32 *pu32RegBuf
    );

extern
STF_RESULT STFLIB_ISP_RegWriteSeries(
    STF_U8 u8IspIdx,
    STF_U32 u32RegAddr,
    STF_U32 u32Items,
    STF_U32 *pu32RegBuf
    );

extern
STF_RESULT STFLIB_ISP_RegReadModule(
    STF_U8 u8IspIdx,
    STF_U32 u32RegAddr,
    STF_U32 u32Items,
    STF_U32 *pu32RegBuf
    );

extern
STF_RESULT STFLIB_ISP_RegWriteModule(
    STF_U8 u8IspIdx,
    STF_U32 u32RegAddr,
    STF_U32 u32Items,
    STF_U32 *pu32RegBuf
    );

extern
STF_RESULT STFLIB_ISP_RegReadByTable(
    STF_U8 u8IspIdx,
    STF_U32 u32Items,
    ST_REG_TBL *pstRegBuf
    );

extern
STF_RESULT STFLIB_ISP_RegWriteByTable(
    STF_U8 u8IspIdx,
    STF_U32 u32Items,
    ST_REG_TBL *pstRegBuf
    );

extern
STF_RESULT STFLIB_ISP_RegReadByTable2(
    STF_U8 u8IspIdx,
    STF_U32 u32Items,
    ST_REG_TBL_2 *pstRegBuf
    );

extern
STF_RESULT STFLIB_ISP_RegWriteByTable2(
    STF_U8 u8IspIdx,
    STF_U32 u32Items,
    ST_REG_TBL_2 *pstRegBuf
    );

extern
STF_RESULT STFLIB_ISP_RegReadByTable3(
    STF_U8 u8IspIdx,
    STF_U32 u32Items,
    ST_REG_TBL_3 *pstRegBuf
    );

extern
STF_RESULT STFLIB_ISP_RegWriteByTable3(
    STF_U8 u8IspIdx,
    STF_U32 u32Items,
    ST_REG_TBL_3 *pstRegBuf
    );

extern
STF_RESULT STFLIB_ISP_RegWriteBySimplePack(
    STF_U8 u8IspIdx,
    STF_U32 u32Items,
    ST_RDMA_CMD *pstRegBuf
    );

extern
STF_RESULT STFLIB_ISP_RegWriteBySoftRdma(
    STF_U8 u8IspIdx,
    STF_U32 u32PhysBufAddr
    );

//-----------------------------------------------------------------------------
#if 0
extern
const STF_CHAR* FormatString(
    ePxlFormat ePxl
    );

extern
const STF_CHAR* MosaicString(
    enum MOSAICType mos
    );

#endif
extern
STF_VOID ParseCiMemParamInfo(
    CI_MEM_PARAM *pstCiMemParam
    );

extern
STF_VOID STFLIB_ISP_MemDebugInfoEnable(
    STF_BOOL8 bMemDebugInfoEnable
    );

extern
STF_RESULT STFLIB_ISP_MemAlloc(
    STF_U8 u8IspIdx,
    CI_MEM_PARAM *pstMemParam,
    STF_U32 u32Width,
    STF_U32 u32Height,
    CI_BUFFTYPE enAllocBufferType,
    eFORMAT_TYPES enFormatType,
    ePxlFormat enPixelFormat,
    eMOSAIC enMosaic,
    STF_U32 u32Size,
    STF_CHAR *szMemName
    );

extern
STF_RESULT STFLIB_ISP_MemAlloc2(
    STF_U8 u8IspIdx,
    CI_MEM_PARAM **ppstMemParam,
    STF_U32 u32Width,
    STF_U32 u32Height,
    CI_BUFFTYPE enAllocBufferType,
    eFORMAT_TYPES enFormatType,
    ePxlFormat enPixelFormat,
    eMOSAIC enMosaic,
    STF_U32 u32Size,
    STF_CHAR *szMemName
    );

extern
STF_RESULT STFLIB_ISP_MemFree(
    STF_U8 u8IspIdx,
    CI_MEM_PARAM *pstMemParam
    );

extern
STF_RESULT STFLIB_ISP_MemFree2(
    STF_U8 u8IspIdx,
    CI_MEM_PARAM **ppstMemParam
    );

//-----------------------------------------------------------------------------
extern
STF_RESULT STFLIB_ISP_SetModulesEnable(
    ST_ISP_MODULES_EN_PARAM *pstIspModulesEnableParam
    );

extern
STF_RESULT STFLIB_ISP_CsiAndIspStart(
    STF_U8 u8IspIdx
    );

extern
STF_RESULT STFLIB_ISP_CsiAndIspStop(
    STF_U8 u8IspIdx
    );

//-----------------------------------------------------------------------------
extern
STF_U32 OutScalingFactotCal(
    STF_U32 u32SrcSize,
    STF_U32 u32DstSize
    );

extern
STF_U32 OutScalingDstSizeCal(
    STF_U32 u32SrcSize,
    STF_U32 u32SaclingFactor
    );

//-----------------------------------------------------------------------------
extern
STF_RESULT AWB_SetWbGain(
    STF_U8 u8IspIdx,
    STF_U8 u8WbGainIdx
    );

extern
STF_RESULT R2Y_SetMatrix(
    STF_U8 u8IspIdx,
    STF_U8 u8MatrixIdx
    );

//-----------------------------------------------------------------------------
extern
STF_RESULT SUD_SetCsiShadowEnable(
    STF_U8 u8IspIdx
    );

extern
STF_RESULT SUD_SetCsiShadowDisable(
    STF_U8 u8IspIdx
    );

extern
STF_RESULT SUD_SetIspShadowEnable(
    STF_U8 u8IspIdx
    );

extern
STF_RESULT SUD_SetIspShadowDisable(
    STF_U8 u8IspIdx
    );

//-----------------------------------------------------------------------------
extern
STF_RESULT SC_PrintScDumpBuffer(
    STF_U32 *pu32Buffer
    );

extern
STF_RESULT YHist_PrintYHistBuffer(
    STF_U32 *pu32Buffer
    );

//-----------------------------------------------------------------------------
extern
STF_RESULT ISP_Check_Isp_Param_Value(
    STF_U8 u8IspIdx
    );

extern
STF_RESULT ISP_Check_Sensor_Param_Value(
#if defined(V4L2_DRIVER)
    ST_SENSOR *pstSensor,
#endif //#if defined(V4L2_DRIVER)
    const STF_U16 *pau16SensorParam,
    STF_U32 u32Length
    );

extern
STF_RESULT ISP_Check_Register_Value(
    STF_U8 u8IspIdx
    );

//-----------------------------------------------------------------------------


#ifdef __cplusplus
}
#endif


#endif //__STFLIB_ISP_BASE_H__
