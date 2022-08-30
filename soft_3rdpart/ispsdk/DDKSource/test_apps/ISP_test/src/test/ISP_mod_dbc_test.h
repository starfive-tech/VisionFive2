/**
  ******************************************************************************
  * @file  ISP_mod_dbc_test.h
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
#ifndef __ISP_MOD_DBC_TEST_H__
#define __ISP_MOD_DBC_TEST_H__


#ifdef __cplusplus
extern "C" {
#endif


#include "stf_include.h"


/* ISPC DBC driver test structure */


/* ISPC DBC driver test variables */


/* ISPC DBC driver test interface */
//-----------------------------------------------------------------------------
extern
STF_RESULT Test_DBC_SetReg(
    STF_VOID
    );

#if defined(EXTEND_IOCTL_COMMAND_DBC)
//-----------------------------------------------------------------------------
extern
STF_RESULT Test_DBC_SetBadPixel(
    STF_VOID
    );

#endif //#if defined(EXTEND_IOCTL_COMMAND_DBC)
//-----------------------------------------------------------------------------
extern
STF_RESULT ISP_Mod_Dbc_Test(
    STF_VOID
    );

//-----------------------------------------------------------------------------


#ifdef __cplusplus
}
#endif


#endif //__ISP_MOD_DBC_TEST_H__

