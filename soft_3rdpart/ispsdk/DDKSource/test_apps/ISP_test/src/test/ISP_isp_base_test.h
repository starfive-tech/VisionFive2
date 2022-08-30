/**
  ******************************************************************************
  * @file  ISP_isp_base_test.h
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
#ifndef __ISP_ISP_BASE_TEST_H__
#define __ISP_ISP_BASE_TEST_H__


#ifdef __cplusplus
extern "C" {
#endif


#include "stf_include.h"


/* ISPC ISP base driver test structure */


/* ISPC ISP base driver test variables */


/* ISPC ISP base driver test interface */
//-----------------------------------------------------------------------------
extern
STF_RESULT Test_SetCSIEnable(
    STF_VOID
    );

#if defined(EXTEND_IOCTL_COMMAND_BASE)
extern
STF_RESULT Test_IsCSIBusy(
    STF_VOID
    );

extern
STF_RESULT Test_IsScdDone(
    STF_VOID
    );

extern
STF_RESULT Test_IsScdError(
    STF_VOID
    );

extern
STF_RESULT Test_IsItu656Field(
    STF_VOID
    );

extern
STF_RESULT Test_IsItu656Error(
    STF_VOID
    );

#endif //#if defined(EXTEND_IOCTL_COMMAND_BASE)
extern
STF_RESULT Test_GetCsiAllStatus(
    STF_VOID
    );

//-----------------------------------------------------------------------------
extern
STF_RESULT Test_GetCSIVCount(
    STF_VOID
    );

extern
STF_RESULT Test_GetCSIHCount(
    STF_VOID
    );

extern
STF_RESULT Test_SetCsiLineIntLine(
    STF_VOID
    );

extern
STF_RESULT Test_GetCsiLineIntLine(
    STF_VOID
    );

//-----------------------------------------------------------------------------
extern
STF_RESULT Test_SetModuleEnable(
    STF_VOID
    );

extern
STF_RESULT Test_SetModulesEnable(
    STF_VOID
    );

//-----------------------------------------------------------------------------
extern
STF_RESULT Test_SetSensorParameter(
    STF_VOID
    );

extern
STF_RESULT Test_GetSensorRawFormat(
    STF_VOID
    );

extern
STF_RESULT Test_SetSensorRawFormat(
    STF_VOID
    );

//-----------------------------------------------------------------------------
extern
STF_RESULT Test_GetIspCaptureSize(
    STF_VOID
    );

extern
STF_RESULT Test_SetCsiCaptureWindowAndIspCaptureSize(
    STF_VOID
    );

//-----------------------------------------------------------------------------
extern
STF_RESULT Test_SetIspFirstFixel(
    STF_VOID
    );

extern
STF_RESULT Test_SetIspMultiFrameMode(
    STF_VOID
    );

extern
STF_RESULT Test_SetIspAndCsiBaseParameters(
    STF_VOID
    );

extern
STF_RESULT Test_SetIspEnable(
    STF_VOID
    );

extern
STF_RESULT Test_SetIspReset(
    STF_VOID
    );

extern
STF_RESULT Test_IspReset(
    STF_VOID
    );

extern
STF_RESULT Test_IspResetAndEnable(
    STF_VOID
    );

//-----------------------------------------------------------------------------
#if defined(EXTEND_IOCTL_COMMAND_BASE)
extern
STF_RESULT Test_IsIsp_Out_IspOrOutBusy(
    STF_VOID
    );

extern
STF_RESULT Test_IsIsp_Uo_Out_IspOrOutBusy(
    STF_VOID
    );

extern
STF_RESULT Test_IsIsp_Ss0_Out_IspOrOutBusy(
    STF_VOID
    );

extern
STF_RESULT Test_IsIsp_Ss1_Out_IspOrOutBusy(
    STF_VOID
    );

extern
STF_RESULT Test_IsCsiBufferOverrun(
    STF_VOID
    );

#endif //#if defined(EXTEND_IOCTL_COMMAND_BASE)
extern
STF_RESULT Test_GetIspAllStatus(
    STF_VOID
    );

//-----------------------------------------------------------------------------
#if defined(EXTEND_IOCTL_COMMAND_BASE)
extern
STF_RESULT Test_IsIspDoneInterrupt(
    STF_VOID
    );

extern
STF_RESULT Test_IsCsiDoneInterrupt(
    STF_VOID
    );

extern
STF_RESULT Test_IsScDoneInterrupt(
    STF_VOID
    );

extern
STF_RESULT Test_IsCsiLineInterrupt(
    STF_VOID
    );

extern
STF_RESULT Test_SetInterrupt(
    STF_VOID
    );

#endif //#if defined(EXTEND_IOCTL_COMMAND_BASE)
extern
STF_RESULT Test_SetInterruptClear(
    STF_VOID
    );

extern
STF_RESULT Test_GetInterruptStatus(
    STF_VOID
    );

#if defined(EXTEND_IOCTL_COMMAND_BASE)
//-----------------------------------------------------------------------------
// Since the shadow update enable bit cannot be read back,
// therefore disable below feature support.
#if defined(ENABLE_SET_SHADOW_MODE_SUPPORT)
extern
STF_RESULT Test_SetCsiShadowMode(
    STF_VOID
    );

#endif //#if defined(ENABLE_SET_SHADOW_MODE_SUPPORT)
extern
STF_RESULT Test_SetCsiShadow(
    STF_VOID
    );

extern
STF_RESULT Test_SetCsiShadowImmediatelyAndEnable(
    STF_VOID
    );

extern
STF_RESULT Test_SetCsiShadowNextVSyncAndEnable(
    STF_VOID
    );

extern
STF_RESULT Test_SetCsiShadowImmediatelyAndTrigger(
    STF_VOID
    );

extern
STF_RESULT Test_SetCsiShadowNextVSyncAndTrigger(
    STF_VOID
    );

extern
STF_RESULT Test_TriggerCsiShadow(
    STF_VOID
    );

#endif //#if defined(EXTEND_IOCTL_COMMAND_BASE)
#if defined(EXTEND_IOCTL_COMMAND_BASE)
//-----------------------------------------------------------------------------
// Since the shadow update enable bit cannot be read back,
// therefore disable below feature support.
#if defined(ENABLE_SET_SHADOW_MODE_SUPPORT)
extern
STF_RESULT Test_SetIspShadowMode(
    STF_VOID
    );

#endif //#if defined(ENABLE_SET_SHADOW_MODE_SUPPORT)
extern
STF_RESULT Test_SetIspShadow(
    STF_VOID
    );

extern
STF_RESULT Test_SetIspShadowImmediatelyAndEnable(
    STF_VOID
    );

extern
STF_RESULT Test_SetIspShadowNextVSyncAndEnable(
    STF_VOID
    );

extern
STF_RESULT Test_SetIspShadowImmediatelyAndTrigger(
    STF_VOID
    );

extern
STF_RESULT Test_SetIspShadowNextVSyncAndTrigger(
    STF_VOID
    );

extern
STF_RESULT Test_TriggerIspShadow(
    STF_VOID
    );

#endif //#if defined(EXTEND_IOCTL_COMMAND_BASE)
//-----------------------------------------------------------------------------
extern
STF_RESULT Test_CsiAndIspStop(
    STF_VOID
    );

extern
STF_RESULT Test_CsiAndIspStart(
    STF_VOID
    );

//-----------------------------------------------------------------------------
extern
STF_RESULT ISP_Isp_Base_Test(
    STF_VOID
    );

//-----------------------------------------------------------------------------


#ifdef __cplusplus
}
#endif


#endif //__ISP_ISP_BASE_TEST_H__

