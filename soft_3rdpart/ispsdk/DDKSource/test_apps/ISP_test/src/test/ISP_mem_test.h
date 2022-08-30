/**
  ******************************************************************************
  * @file  ISP_mem_test.h
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
#ifndef __ISP_Mem_TEST_H__
#define __ISP_Mem_TEST_H__


#ifdef __cplusplus
extern "C" {
#endif


#include "stf_include.h"


/* ISPC Memory driver test structure */


/* ISPC Memory driver test variables */


/* ISPC Memory driver test interface */
//-----------------------------------------------------------------------------
//=== Godspeed === Add new memory/buffer type support here.
extern
STF_RESULT Test_UoMemAlloc(
    STF_VOID
    );

extern
STF_RESULT Test_UoMemFree(
    STF_VOID
    );

extern
STF_RESULT Test_Ss0MemAlloc(
    STF_VOID
    );

extern
STF_RESULT Test_Ss0MemFree(
    STF_VOID
    );

extern
STF_RESULT Test_Ss1MemAlloc(
    STF_VOID
    );

extern
STF_RESULT Test_Ss1MemFree(
    STF_VOID
    );

extern
STF_RESULT Test_DumpMemAlloc(
    STF_VOID
    );

extern
STF_RESULT Test_DumpMemFree(
    STF_VOID
    );

extern
STF_RESULT Test_Tiling_1_WrMemAlloc(
    STF_VOID
    );

extern
STF_RESULT Test_Tiling_1_WrMemFree(
    STF_VOID
    );

extern
STF_RESULT Test_ScDumpMemAlloc(
    STF_VOID
    );

extern
STF_RESULT Test_ScDumpMemFree(
    STF_VOID
    );

extern
STF_RESULT Test_YHistMemAlloc(
    STF_VOID
    );

extern
STF_RESULT Test_YHistMemFree(
    STF_VOID
    );

extern
STF_RESULT Test_GenMemMemAlloc(
    STF_VOID
    );

extern
STF_RESULT Test_GenMemMemFree(
    STF_VOID
    );

extern
STF_RESULT Test_GenMemAlign128MemAlloc(
    STF_VOID
    );

extern
STF_RESULT Test_GenMemAlign128MemFree(
    STF_VOID
    );

//-----------------------------------------------------------------------------
extern
STF_RESULT ISP_Mem_Test(
    STF_VOID
    );

//-----------------------------------------------------------------------------


#ifdef __cplusplus
}
#endif


#endif //__ISP_MEM_TEST_H__

