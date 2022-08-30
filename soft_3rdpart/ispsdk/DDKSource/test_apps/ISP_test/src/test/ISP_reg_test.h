/**
  ******************************************************************************
  * @file  ISP_reg_test.h
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
#ifndef __ISP_REG_TEST_H__
#define __ISP_REG_TEST_H__


#ifdef __cplusplus
extern "C" {
#endif


#include "stf_include.h"


/* ISPC Register driver test structure */


/* ISPC Register driver test variables */


/* ISPC Register driver test interface */
//-----------------------------------------------------------------------------
extern
STF_RESULT Test_ReadOne(
    STF_VOID
    );

extern
STF_RESULT Test_ReadSeries(
    STF_VOID
    );

extern
STF_RESULT Test_ReadModule(
    STF_VOID
    );

extern
STF_RESULT Test_ReadByTable(
    STF_VOID
    );

extern
STF_RESULT Test_ReadByTable2(
    STF_VOID
    );

extern
STF_RESULT Test_ReadByTable3(
    STF_VOID
    );

//-----------------------------------------------------------------------------
extern
STF_RESULT Test_WriteOne(
    STF_VOID
    );

extern
STF_RESULT Test_WriteSeries(
    STF_VOID
    );

extern
STF_RESULT Test_WriteModule(
    STF_VOID
    );

extern
STF_RESULT Test_WriteByTable(
    STF_VOID
    );

extern
STF_RESULT Test_WriteByTable2(
    STF_VOID
    );

extern
STF_RESULT Test_WriteByTable3(
    STF_VOID
    );

extern
STF_RESULT Test_WriteBySimplePack(
    STF_VOID
    );

extern
STF_RESULT Test_WriteBySoftRdma(
    STF_VOID
    );

//-----------------------------------------------------------------------------
extern
STF_RESULT ISP_Reg_Test(
    STF_VOID
    );

//-----------------------------------------------------------------------------


#ifdef __cplusplus
}
#endif


#endif //__ISP_REG_TEST_H__

