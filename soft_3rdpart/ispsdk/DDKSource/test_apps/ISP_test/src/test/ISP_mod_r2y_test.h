/**
  ******************************************************************************
  * @file  ISP_mod_r2y_test.h
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
#ifndef __ISP_MOD_R2Y_TEST_H__
#define __ISP_MOD_R2Y_TEST_H__


#ifdef __cplusplus
extern "C" {
#endif


#include "stf_include.h"


/* ISPC R2Y driver test structure */


/* ISPC R2Y driver test variables */


/* ISPC R2Y driver test interface */
//-----------------------------------------------------------------------------
extern
STF_RESULT Test_R2Y_SetReg(
    STF_VOID
    );

#if defined(EXTEND_IOCTL_COMMAND_R2Y)
//-----------------------------------------------------------------------------
extern
STF_RESULT Test_R2Y_SetColorMatrix(
    STF_VOID
    );

#endif //#if defined(EXTEND_IOCTL_COMMAND_R2Y)
//-----------------------------------------------------------------------------
extern
STF_RESULT ISP_Mod_R2y_Test(
    STF_VOID
    );

//-----------------------------------------------------------------------------


#ifdef __cplusplus
}
#endif


#endif //__ISP_MOD_R2Y_TEST_H__

