/**
  ******************************************************************************
  * @file  ISP_mod_crop_test.h
  * @author  StarFive Isp Team
  * @version  V1.0
  * @date  03/23/2019
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
#ifndef __ISP_MOD_CROP_TEST_H__
#define __ISP_MOD_CROP_TEST_H__


#ifdef __cplusplus
extern "C" {
#endif


#include "stf_include.h"


/* ISPC CROP driver test structure */


/* ISPC CROP driver test variables */


/* ISPC CROP driver test interface */
//-----------------------------------------------------------------------------
extern
STF_RESULT Test_CROP_SetReg(
    STF_VOID
    );

#if defined(EXTEND_IOCTL_COMMAND_CROP)
//-----------------------------------------------------------------------------
extern
STF_RESULT Test_CROP_SetCaptureWindow(
    STF_VOID
    );

#endif //#if defined(EXTEND_IOCTL_COMMAND_CROP)
//-----------------------------------------------------------------------------
extern
STF_RESULT ISP_Mod_Crop_Test(
    STF_VOID
    );

//-----------------------------------------------------------------------------


#ifdef __cplusplus
}
#endif


#endif //__ISP_MOD_CROP_TEST_H__

