/**
  ******************************************************************************
  * @file  ISP_mod_sc_test.h
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
#ifndef __ISP_MOD_SC_TEST_H__
#define __ISP_MOD_SC_TEST_H__


#ifdef __cplusplus
extern "C" {
#endif


#include "stf_include.h"


/* ISPC SC driver test structure */


/* ISPC SC driver test variables */


/* ISPC SC driver test interface */
//-----------------------------------------------------------------------------
extern
STF_RESULT Test_SC_SetReg(
    STF_VOID
    );

#if defined(EXTEND_IOCTL_COMMAND_SC)
extern
STF_RESULT Test_SC_SetCfgReg(
    STF_VOID
    );

extern
STF_RESULT Test_SC_SetCropReg(
    STF_VOID
    );

extern
STF_RESULT Test_SC_SetDecReg(
    STF_VOID
    );

extern
STF_RESULT Test_SC_SetAeAfReg(
    STF_VOID
    );

extern
STF_RESULT Test_SC_SetAwbReg(
    STF_VOID
    );

//-----------------------------------------------------------------------------
extern
STF_RESULT Test_SC_SetDumpingAddress(
    STF_VOID
    );

extern
STF_RESULT Test_SC_SetAxiId(
    STF_VOID
    );

extern
STF_RESULT Test_SC_SetAeAfFrameCropping(
    STF_VOID
    );

extern
STF_RESULT Test_SC_SetAeAfConfig(
    STF_VOID
    );

extern
STF_RESULT Test_SC_SetAeAfDecimation(
    STF_VOID
    );

extern
STF_RESULT Test_SC_SetAfConfig(
    STF_VOID
    );

extern
STF_RESULT Test_SC_SetAwbFrameCropping(
    STF_VOID
    );

extern
STF_RESULT Test_SC_SetAwbConfig(
    STF_VOID
    );

extern
STF_RESULT Test_SC_SetAwbDecimation(
    STF_VOID
    );

extern
STF_RESULT Test_SC_SetAwbPixelSumRangeR(
    STF_VOID
    );

extern
STF_RESULT Test_SC_SetAwbPixelSumRangeG(
    STF_VOID
    );

extern
STF_RESULT Test_SC_SetAwbPixelSumRangeB(
    STF_VOID
    );

extern
STF_RESULT Test_SC_SetAwbPixelSumRangeY(
    STF_VOID
    );

extern
STF_RESULT Test_SC_SetAwbPixelSumRangeGR(
    STF_VOID
    );

extern
STF_RESULT Test_SC_SetAwbPixelSumRangeGB(
    STF_VOID
    );

extern
STF_RESULT Test_SC_SetAwbPixelSumRangeGRB(
    STF_VOID
    );

extern
STF_RESULT Test_SC_SetAwbWeightSumRangeR(
    STF_VOID
    );

extern
STF_RESULT Test_SC_SetAwbWeightSumRangeGr(
    STF_VOID
    );

extern
STF_RESULT Test_SC_SetAwbWeightSumRangeGb(
    STF_VOID
    );

extern
STF_RESULT Test_SC_SetAwbWeightSumRangeB(
    STF_VOID
    );

extern
STF_RESULT Test_SC_SetAwbWeightTable(
    STF_VOID
    );

extern
STF_RESULT Test_SC_SetAwbIntensityWeightCurve(
    STF_VOID
    );

#endif //#if defined(EXTEND_IOCTL_COMMAND_SC)
//-----------------------------------------------------------------------------
extern
STF_RESULT Test_SC_SetDumpingResultControl(
    STF_VOID
    );

#if defined(EXTEND_IOCTL_COMMAND_SC)
extern
STF_RESULT Test_SC_GetResult(
    STF_VOID
    );

#endif //#if defined(EXTEND_IOCTL_COMMAND_SC)
extern
STF_RESULT Test_SC_SC_GetResults(
    STF_VOID
    );

//-----------------------------------------------------------------------------
extern
STF_RESULT ISP_Mod_Sc_Test(
    STF_VOID
    );

//-----------------------------------------------------------------------------


#ifdef __cplusplus
}
#endif


#endif //__ISP_MOD_SC_TEST_H__

