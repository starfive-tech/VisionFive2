/**
  ******************************************************************************
  * @file  ISP_mod_til_test.h
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
#ifndef __ISP_MOD_TIL_TEST_H__
#define __ISP_MOD_TIL_TEST_H__


#ifdef __cplusplus
extern "C" {
#endif


#include "stf_include.h"


/* ISPC TIL driver test structure */


/* ISPC TIL driver test variables */


/* ISPC TIL driver test interface */
//-----------------------------------------------------------------------------
extern
STF_RESULT Test_TIL_SetReg(
    STF_VOID
    );

extern
STF_RESULT Test_TIL_GetAllBusyStatus(
    STF_VOID
    );

extern
STF_RESULT Test_TIL_1_SetWindowReg(
    STF_VOID
    );

extern
STF_RESULT Test_TIL_1_SetConfigReg(
    STF_VOID
    );

extern
STF_RESULT Test_TIL_1_SetReadReg(
    STF_VOID
    );

extern
STF_RESULT Test_TIL_1_SetWriteReg(
    STF_VOID
    );

extern
STF_RESULT Test_TIL_1_GetBusyStatus(
    STF_VOID
    );

#if defined(EXTEND_IOCTL_COMMAND_TIL)
extern
STF_RESULT Test_TIL_1_IsReadBusy(
    STF_VOID
    );

extern
STF_RESULT Test_TIL_1_IsWriteBusy(
    STF_VOID
    );

#endif //#if defined(EXTEND_IOCTL_COMMAND_TIL)
#if defined(EXTEND_IOCTL_COMMAND_TIL)
//-----------------------------------------------------------------------------
extern
STF_RESULT Test_TIL_1_SetWindowSize(
    STF_VOID
    );

extern
STF_RESULT Test_TIL_1_SetReadBufAddr(
    STF_VOID
    );

extern
STF_RESULT Test_TIL_1_SetReadBufStride(
    STF_VOID
    );

extern
STF_RESULT Test_TIL_1_SetReadDataPath(
    STF_VOID
    );

extern
STF_RESULT Test_TIL_1_SetReadNv21(
    STF_VOID
    );

extern
STF_RESULT Test_TIL_1_SetReadDataFormat(
    STF_VOID
    );

extern
STF_RESULT Test_TIL_1_SetReadFlexibleBits(
    STF_VOID
    );

extern
STF_RESULT Test_TIL_1_SetReadIndependentRvi(
    STF_VOID
    );

extern
STF_RESULT Test_TIL_1_SetReadLineBufOff(
    STF_VOID
    );

extern
STF_RESULT Test_TIL_1_SetReadAxiId(
    STF_VOID
    );

extern
STF_RESULT Test_TIL_1_SetReadEnable(
    STF_VOID
    );

extern
STF_RESULT Test_TIL_1_SetWriteBufAddr(
    STF_VOID
    );

extern
STF_RESULT Test_TIL_1_SetWriteBufStride(
    STF_VOID
    );

extern
STF_RESULT Test_TIL_1_SetWriteDataPath(
    STF_VOID
    );

extern
STF_RESULT Test_TIL_1_SetWriteNv21(
    STF_VOID
    );

extern
STF_RESULT Test_TIL_1_SetWriteDataFormat(
    STF_VOID
    );

extern
STF_RESULT Test_TIL_1_SetWriteFlexibleBits(
    STF_VOID
    );

extern
STF_RESULT Test_TIL_1_SetWriteAxiId(
    STF_VOID
    );

extern
STF_RESULT Test_TIL_1_SetReadEnable(
    STF_VOID
    );

extern
STF_RESULT Test_TIL_1_SetIspRaw10InDW(
    STF_VOID
    );

extern
STF_RESULT Test_TIL_1_SetLoopbackMode(
    STF_VOID
    );

extern
STF_RESULT Test_TIL_1_SetDmaMode(
    STF_VOID
    );

#endif //#if defined(EXTEND_IOCTL_COMMAND_TIL)
//-----------------------------------------------------------------------------
extern
STF_RESULT ISP_Mod_Til_Test(
    STF_VOID
    );

//-----------------------------------------------------------------------------


#ifdef __cplusplus
}
#endif


#endif //__ISP_MOD_TIL_TEST_H__

