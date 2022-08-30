/**
  ******************************************************************************
  * @file  ISP_mod_sud_test.h
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
#ifndef __ISP_MOD_SUD_TEST_H__
#define __ISP_MOD_SUD_TEST_H__


#ifdef __cplusplus
extern "C" {
#endif


#include "stf_include.h"


/* ISPC SUD driver test structure */


/* ISPC SUD driver test variables */


/* ISPC SUD driver test interface */
//-----------------------------------------------------------------------------
extern
STF_RESULT Test_SUD_SetReg(
    STF_VOID
    );

//-----------------------------------------------------------------------------
extern
STF_RESULT Test_SUD_SetCsiReg(
    STF_VOID
    );

extern
STF_RESULT Test_SUD_SetCsiShadowEnable(
    STF_VOID
    );

#if defined(EXTEND_IOCTL_COMMAND_SUD_CSI)
// Since the shadow update enable bit cannot be read back,
// therefore disable below feature support.
#if defined(ENABLE_SET_SHADOW_MODE_SUPPORT)
extern
STF_RESULT Test_SUD_SetCsiShadowMode(
    STF_VOID
    );

#endif //#if defined(ENABLE_SET_SHADOW_MODE_SUPPORT)
extern
STF_RESULT Test_SUD_SetCsiShadowImmediatelyAndEnable(
    STF_VOID
    );

extern
STF_RESULT Test_SUD_SetCsiShadowNextVSyncAndEnable(
    STF_VOID
    );

extern
STF_RESULT Test_SUD_SetCsiShadowImmediatelyAndTrigger(
    STF_VOID
    );

extern
STF_RESULT Test_SUD_SetCsiShadowNextVSyncAndTrigger(
    STF_VOID
    );

extern
STF_RESULT Test_SUD_TriggerCsiShadow(
    STF_VOID
    );

#endif //#if defined(EXTEND_IOCTL_COMMAND_SUD_CSI)
extern
STF_RESULT Test_SUD_SetIspReg(
    STF_VOID
    );

extern
STF_RESULT Test_SUD_SetIspShadowEnable(
    STF_VOID
    );

#if defined(EXTEND_IOCTL_COMMAND_SUD_ISP)
// Since the shadow update enable bit cannot be read back,
// therefore disable below feature support.
#if defined(ENABLE_SET_SHADOW_MODE_SUPPORT)
extern
STF_RESULT Test_SUD_SetIspShadowMode(
    STF_VOID
    );

#endif //#if defined(ENABLE_SET_SHADOW_MODE_SUPPORT)
extern
STF_RESULT Test_SUD_SetIspShadowImmediatelyAndEnable(
    STF_VOID
    );

extern
STF_RESULT Test_SUD_SetIspShadowNextVSyncAndEnable(
    STF_VOID
    );

extern
STF_RESULT Test_SUD_SetIspShadowImmediatelyAndTrigger(
    STF_VOID
    );

extern
STF_RESULT Test_SUD_SetIspShadowNextVSyncAndTrigger(
    STF_VOID
    );

extern
STF_RESULT Test_SUD_TriggerIspShadow(
    STF_VOID
    );

#endif //#if defined(EXTEND_IOCTL_COMMAND_SUD_ISP)
extern
STF_RESULT Test_SUD_IspShadowEnableBitTest(
    STF_VOID
    );

//-----------------------------------------------------------------------------
extern
STF_RESULT ISP_Mod_Sud_Test(
    STF_VOID
    );

//-----------------------------------------------------------------------------


#ifdef __cplusplus
}
#endif


#endif //__ISP_MOD_SUD_TEST_H__

