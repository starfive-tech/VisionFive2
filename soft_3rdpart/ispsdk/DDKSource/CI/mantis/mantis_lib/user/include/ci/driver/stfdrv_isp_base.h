/**
  ******************************************************************************
  * @file  stfdrv_isp_base.h
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
#ifndef __STFDRV_ISP_BASE_H__
#define __STFDRV_ISP_BASE_H__


#ifdef __cplusplus
extern "C" {
#endif


#include "stf_isp.h"


/* ISP and CSI base driver structure */


/* ISP and CSI base driver interface */
//-----------------------------------------------------------------------------
#if defined(V4L2_DRIVER)
extern
STF_RESULT STFDRV_ISP_BASE_LoadFw(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    ST_FW_INFO *pstFwInfo
    );

#endif //#if defined(V4L2_DRIVER)
//-----------------------------------------------------------------------------
/**
 * @brief Read a register at a given offset
 *
 * @param[in] pConnection
 * @param[in] u32Method access method of register
 * @param[in] u32Offset offset indicated which register will be read
 * @param[in] u32Length length for indicated how much register will be read
 * @param[out] pu32Buffer where to store the result - should not be NULL
 *
 * @return STF_SUCCESS
 * @return STF_ERROR_INVALID_PARAMETER if pConnection or pResult is NULL or
 * eBank is an invalid value.
 * @return STF_ERROR_NOT_SUPPORTED if kernel module does not support debug
 * access
 * @return STF_ERROR_FATAL if kernel module returned an error.
 */
extern
STF_RESULT STFDRV_ISP_BASE_RegRead(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    STF_U32 u32Method,
    STF_U32 u32Offset,
    STF_U32 u32Length,
    STF_U32 *pu32Buffer
    );

/**
 * @brief Write a register at a given offset
 *
 * @param[in] pConnection
 * @param[in] u32Method access method of register
 * @param[in] u32Offset offset indicated which register will be write
 * @param[in] u32Length length for indicated how much register will be write
 * @param[in] pu32Buffer where to store the register value for the program
 *
 * @return STF_SUCCESS
 * @return STF_ERROR_INVALID_PARAMETER if pConnection or pResult is NULL or
 * eBank is an invalid value.
 * @return STF_ERROR_NOT_SUPPORTED if kernel module does not support debug
 * access
 */
extern
STF_RESULT STFDRV_ISP_BASE_RegWrite(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    STF_U32 u32Method,
    STF_U32 u32Offset,
    STF_U32 u32Length,
    STF_U32 *pu32Buffer
    );

//-----------------------------------------------------------------------------
/**
 * @brief Allocate a physical contiguous memory buffer
 *
 * @param[in] pConnection
 *
 * @return STF_SUCCESS
 * @return STF_ERROR_INVALID_PARAMETER if pConnection or pResult is NULL or
 * eBank is an invalid value.
 * @return STF_ERROR_NOT_SUPPORTED if kernel module does not support debug
 * access
 * @return STF_ERROR_FATAL if kernel module returned an error.
 */
extern
STF_RESULT STFDRV_ISP_BASE_MemoryAlloc(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
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

/**
 * @brief Free a physical contiguous memory buffer
 *
 * @param[in] pConnection
 *
 * @return STF_SUCCESS
 * @return STF_ERROR_INVALID_PARAMETER if pConnection or pResult is NULL or
 * eBank is an invalid value.
 * @return STF_ERROR_NOT_SUPPORTED if kernel module does not support debug
 * access
 */
extern
STF_RESULT STFDRV_ISP_BASE_MemoryFree(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    CI_MEM_PARAM *pstMemParam
    );

//-----------------------------------------------------------------------------
/**
 * @brief Lock the CSI and ISP shadow update right.
 *
 * @param[in] pConnection
 *
 * @return STF_SUCCESS
 * @return STF_ERROR_INVALID_PARAMETER if pConnection or pResult is NULL or
 * eBank is an invalid value.
 * @return STF_ERROR_NOT_SUPPORTED if kernel module does not support debug
 * access
 */
extern
STF_RESULT STFDRV_ISP_BASE_ShadowLock(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection
#endif //VIRTUAL_IO_MAPPING
    );

/**
 * @brief Unlock the CSI and ISP shadow update right.
 *
 * @param[in] pConnection
 *
 * @return STF_SUCCESS
 * @return STF_ERROR_INVALID_PARAMETER if pConnection or pResult is NULL or
 * eBank is an invalid value.
 * @return STF_ERROR_NOT_SUPPORTED if kernel module does not support debug
 * access
 */
extern
STF_RESULT STFDRV_ISP_BASE_ShadowUnlock(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection
#endif //VIRTUAL_IO_MAPPING
    );

/**
 * @brief Unlock the CSI and ISP shadow update right and trigger CSI and ISP shadow.
 *
 * @param[in] pConnection
 *
 * @return STF_SUCCESS
 * @return STF_ERROR_INVALID_PARAMETER if pConnection or pResult is NULL or
 * eBank is an invalid value.
 * @return STF_ERROR_NOT_SUPPORTED if kernel module does not support debug
 * access
 */
extern
STF_RESULT STFDRV_ISP_BASE_ShadowUnlockAndTrigger(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection
#endif //VIRTUAL_IO_MAPPING
    );

//-----------------------------------------------------------------------------
/**
 * @brief Notify the kernel driver that the user application has initialized
 *        the ISP.
 *
 * @param[in] pConnection
 *
 * @return STF_SUCCESS
 * @return STF_ERROR_INVALID_PARAMETER if pConnection or pResult is NULL or
 * eBank is an invalid value.
 * @return STF_ERROR_NOT_SUPPORTED if kernel module does not support debug
 * access
 */
extern
STF_RESULT STFDRV_ISP_BASE_SetUserConfigIsp(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection
#endif //VIRTUAL_IO_MAPPING
    );

//-----------------------------------------------------------------------------
extern
STF_RESULT STFDRV_ISP_BASE_SetCsiEnable(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    STF_U8 u8IspIdx,
    STF_BOOL8 bEnable
    );

#if defined(EXTEND_IOCTL_COMMAND_BASE)
extern
STF_BOOL8 STFDRV_ISP_BASE_IsCSIBusy(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    STF_U8 u8IspIdx
    );

extern
STF_BOOL8 STFDRV_ISP_BASE_IsScdDone(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    STF_U8 u8IspIdx
    );

extern
STF_BOOL8 STFDRV_ISP_BASE_IsScdError(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    STF_U8 u8IspIdx
    );

extern
STF_BOOL8 STFDRV_ISP_BASE_IsItu656Field(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    STF_U8 u8IspIdx
    );

extern
STF_BOOL8 STFDRV_ISP_BASE_IsItu656Error(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    STF_U8 u8IspIdx
    );

#endif //#if defined(EXTEND_IOCTL_COMMAND_BASE)
#if 1
extern
STF_U32 STFDRV_ISP_BASE_GetCsiAllStatus(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    STF_U8 u8IspIdx
    );
#else
extern
STF_RESULT STFDRV_ISP_BASE_GetCsiAllStatus(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    ST_ISP_BASE_U32_STATUS *pstIspCsiAllStatus
    );
#endif

//-----------------------------------------------------------------------------
extern
STF_U16 STFDRV_ISP_BASE_GetCSIVCount(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    STF_U8 u8IspIdx
    );

extern
STF_U16 STFDRV_ISP_BASE_GetCSIHCount(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    STF_U8 u8IspIdx
    );

//-----------------------------------------------------------------------------
extern
STF_U16 STFDRV_ISP_BASE_GetCsiLineIntLine(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    STF_U8 u8IspIdx
    );

extern
STF_RESULT STFDRV_ISP_BASE_SetCsiLineIntLine(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    STF_U8 u8IspIdx,
    STF_U16 u16Line
    );

//-----------------------------------------------------------------------------
#if defined(ISP_MODULE_CLOCK_GATE_CONTROL)
extern
STF_RESULT STFDRV_ISP_BASE_SetModulesClockGateControlEnable(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    ST_ISP_MODULES_EN_PARAM *pstIspModulesEnableParam
    );

//-----------------------------------------------------------------------------
#endif //ISP_MODULE_CLOCK_GATE_CONTROL
extern
STF_RESULT STFDRV_ISP_BASE_SetModuleEnable(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
#if defined(SUPPORT_MULTI_ISP)
    STF_U8 u8IspIdx,
#endif //SUPPORT_MULTI_ISP
    STF_U64 u64Module,
    STF_BOOL8 bEnable
    );

extern
STF_RESULT STFDRV_ISP_BASE_SetModulesEnable(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    ST_ISP_MODULES_EN_PARAM *pstIspModulesEnableParam
    );

//-----------------------------------------------------------------------------
extern
STF_RESULT STFDRV_ISP_BASE_SetSensorParameter(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    STF_U8 u8IspIdx,
    EN_IMAGER_SEL enImagerSel,
    EN_DVP_ITU656 enItu656,
    EN_DVP_SYNC_POLARITY enPolarity
    );

extern
STF_U32 STFDRV_ISP_BASE_GetSensorRawFormat(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    STF_U8 u8IspIdx
    );

extern
STF_RESULT STFDRV_ISP_BASE_SetSensorRawFormat(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    STF_U8 u8IspIdx,
    EN_CFA_RAW_FMT enRawFormat
    );

//-----------------------------------------------------------------------------
extern
ST_SIZE STFDRV_ISP_BASE_GetIspCaptureSize(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    STF_U8 u8IspIdx
    );

extern
STF_RESULT STFDRV_ISP_BASE_SetCsiCaptureWindowAndIspCaptureSize(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    ST_CAPTURE_WINDOW *pstCsiCaptureWindow
    );

//-----------------------------------------------------------------------------
extern
STF_RESULT STFDRV_ISP_BASE_SetIspFirstFixel(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    STF_U8 u8IspIdx,
    EN_ISP_1ST_PXL enFirstPixel
    );

extern
STF_RESULT STFDRV_ISP_BASE_SetIspMultiFrameMode(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    STF_U8 u8IspIdx,
    EN_ISP_MULTI_FRAME_MODE enIspMultiFrameMode
    );

extern
STF_RESULT STFDRV_ISP_BASE_SetIspAndCsiBaseParameters(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    ST_ISP_CSI_BASE_PARAMETERS *pstIspCsiBaseParameters
    );

extern
STF_RESULT STFDRV_ISP_BASE_SetIspEnable(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    STF_U8 u8IspIdx,
    STF_BOOL8 bEnable
    );

extern
STF_RESULT STFDRV_ISP_BASE_SetIspReset(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    STF_U8 u8IspIdx,
    STF_BOOL8 bReset
    );

extern
STF_RESULT STFDRV_ISP_BASE_IspReset(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    STF_U8 u8IspIdx
    );

extern
STF_RESULT STFDRV_ISP_BASE_IspResetAndEnable(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    STF_U8 u8IspIdx
    );

//#if defined(SUPPORT_VIC_2_0)
extern
ST_HW_VERSION STFDRV_ISP_BASE_GetIspHwVersion(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    STF_U8 u8IspIdx
    );

//#endif //#if defined(SUPPORT_VIC_2_0)
//-----------------------------------------------------------------------------
#if defined(EXTEND_IOCTL_COMMAND_BASE)
extern
STF_BOOL8 STFDRV_ISP_BASE_IsIspBusy(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    STF_U8 u8IspIdx
    );

extern
STF_BOOL8 STFDRV_ISP_BASE_IsUoBusy(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    STF_U8 u8IspIdx
    );

extern
STF_BOOL8 STFDRV_ISP_BASE_IsSs0Busy(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    STF_U8 u8IspIdx
    );

extern
STF_BOOL8 STFDRV_ISP_BASE_IsSs1Busy(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    STF_U8 u8IspIdx
    );

extern
STF_BOOL8 STFDRV_ISP_BASE_IsOutBusy(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    STF_U8 u8IspIdx
    );

extern
STF_BOOL8 STFDRV_ISP_BASE_IsIspOrOutBusy(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    STF_U8 u8IspIdx
    );

extern
STF_BOOL8 STFDRV_ISP_BASE_IsCsiBufferOverrun(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    STF_U8 u8IspIdx
    );

#endif //#if defined(EXTEND_IOCTL_COMMAND_BASE)
extern
STF_U32 STFDRV_ISP_BASE_GetIspAllStatus(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    STF_U8 u8IspIdx
    );

//-----------------------------------------------------------------------------
#if defined(EXTEND_IOCTL_COMMAND_BASE)
extern
STF_RESULT STFDRV_ISP_BASE_SetInterrupt(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    STF_U8 u8IspIdx,
    EN_ISP_INTERRUPT enInterrupt
    );

#endif //#if defined(EXTEND_IOCTL_COMMAND_BASE)
extern
STF_RESULT STFDRV_ISP_BASE_SetInterruptClear(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    STF_U8 u8IspIdx,
    EN_ISP_INTERRUPT enInterrupt
    );

#if defined(EXTEND_IOCTL_COMMAND_BASE)
extern
STF_BOOL8 STFDRV_ISP_BASE_IsIspDoneInterrupt(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    STF_U8 u8IspIdx
    );

extern
STF_BOOL8 STFDRV_ISP_BASE_IsCsiDoneInterrupt(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    STF_U8 u8IspIdx
    );

extern
STF_BOOL8 STFDRV_ISP_BASE_IsScDoneInterrupt(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    STF_U8 u8IspIdx
    );

extern
STF_BOOL8 STFDRV_ISP_BASE_IsCsiLineInterrupt(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    STF_U8 u8IspIdx
    );

#endif //#if defined(EXTEND_IOCTL_COMMAND_BASE)
extern
STF_U32 STFDRV_ISP_BASE_GetInterruptStatus(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    STF_U8 u8IspIdx
    );

#if defined(EXTEND_IOCTL_COMMAND_BASE)
//-----------------------------------------------------------------------------
// Since the shadow update enable bit cannot be read back,
// therefore disable below feature support.
#if defined(ENABLE_SET_SHADOW_MODE_SUPPORT)
extern
STF_RESULT STFDRV_ISP_BASE_SetCsiShadowMode(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    STF_U8 u8IspIdx,
    EN_SUD_MODE enShadowMode
    );

#endif //#if defined(ENABLE_SET_SHADOW_MODE_SUPPORT)
extern
STF_RESULT STFDRV_ISP_BASE_SetCsiShadow(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    ST_SUD_SHADOW_PARAM *pstSudShadowParam
    );

extern
STF_RESULT STFDRV_ISP_BASE_SetCsiShadowImmediatelyAndEnable(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    STF_U8 *pu8IspIdx
    );

extern
STF_RESULT STFDRV_ISP_BASE_SetCsiShadowNextVSyncAndEnable(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    STF_U8 *pu8IspIdx
    );

extern
STF_RESULT STFDRV_ISP_BASE_SetCsiShadowImmediatelyAndTrigger(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    STF_U8 *pu8IspIdx
    );

extern
STF_RESULT STFDRV_ISP_BASE_SetCsiShadowNextVSyncAndTrigger(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    STF_U8 *pu8IspIdx
    );

extern
STF_RESULT STFDRV_ISP_BASE_TriggerCsiShadow(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    STF_U8 *pu8IspIdx
    );

#endif //#if defined(EXTEND_IOCTL_COMMAND_BASE)
#if defined(EXTEND_IOCTL_COMMAND_BASE)
//-----------------------------------------------------------------------------
// Since the shadow update enable bit cannot be read back,
// therefore disable below feature support.
#if defined(ENABLE_SET_SHADOW_MODE_SUPPORT)
extern
STF_RESULT STFDRV_ISP_BASE_SetIspShadowMode(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    STF_U8 u8IspIdx,
    EN_SUD_MODE enShadowMode
    );

#endif //#if defined(ENABLE_SET_SHADOW_MODE_SUPPORT)
extern
STF_RESULT STFDRV_ISP_BASE_SetIspShadow(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    ST_SUD_SHADOW_PARAM *pstSudShadowParam
    );

extern
STF_RESULT STFDRV_ISP_BASE_SetIspShadowImmediatelyAndEnable(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    STF_U8 *pu8IspIdx
    );

extern
STF_RESULT STFDRV_ISP_BASE_SetIspShadowNextVSyncAndEnable(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    STF_U8 *pu8IspIdx
    );

extern
STF_RESULT STFDRV_ISP_BASE_SetIspShadowImmediatelyAndTrigger(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    STF_U8 *pu8IspIdx
    );

extern
STF_RESULT STFDRV_ISP_BASE_SetIspShadowNextVSyncAndTrigger(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    STF_U8 *pu8IspIdx
    );

extern
STF_RESULT STFDRV_ISP_BASE_TriggerIspShadow(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    STF_U8 *pu8IspIdx
    );

#endif //#if defined(EXTEND_IOCTL_COMMAND_BASE)
//-----------------------------------------------------------------------------
extern
STF_RESULT STFDRV_ISP_BASE_CsiAndIspStart(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    STF_U8 *pu8IspIdx
    );

extern
STF_RESULT STFDRV_ISP_BASE_CsiAndIspStop(
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pConnection,
#endif //VIRTUAL_IO_MAPPING
    STF_U8 *pu8IspIdx
    );

//-----------------------------------------------------------------------------


#ifdef __cplusplus
}
#endif


#endif //__STFDRV_ISP_BASE_H__

