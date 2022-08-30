/**
  ******************************************************************************
  * @file  stflib_mod_config.h
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
  ******************************************************************************/


#ifndef __STFLIB_MOD_CONFIG_H__
#define __STFLIB_MOD_CONFIG_H__


#ifdef __cplusplus
extern "C" {
#endif


#include "stf_include.h"
#include "ci/ci_api_structs.h"
#include "ISPC/stflib_isp_base.h"


/* Each modules configuration library structure */


/* Each modules configuration library variables */


/* Each modules configuration library interface */
//-----------------------------------------------------------------------------
extern
STF_RESULT STFLIB_SUD_Config(
    STF_U8 u8IspIdx
    );

extern
STF_RESULT STFLIB_CBAR_Config(
    STF_U8 u8IspIdx
    );

extern
STF_RESULT STFLIB_DC_Config(
    STF_U8 u8IspIdx
    );

extern
STF_RESULT STFLIB_DEC_Config(
    STF_U8 u8IspIdx
    );

extern
STF_RESULT STFLIB_OBC_Config(
    STF_U8 u8IspIdx
    );

extern
STF_RESULT STFLIB_DBC_Config(
    STF_U8 u8IspIdx
    );

extern
STF_RESULT STFLIB_CTC_Config(
    STF_U8 u8IspIdx
    );

extern
STF_RESULT STFLIB_LCBQ_Config(
    STF_U8 u8IspIdx
    );

extern
STF_RESULT STFLIB_OECF_Config(
    STF_U8 u8IspIdx
    );

extern
STF_RESULT STFLIB_OECFHM_Config(
    STF_U8 u8IspIdx
    );

extern
STF_RESULT STFLIB_LCCF_CONFIG(
    STF_U8 u8IspIdx
    );

extern
STF_RESULT STFLIB_AWB_Config(
    STF_U8 u8IspIdx
    );

extern
STF_RESULT STFLIB_CFA_Config(
    STF_U8 u8IspIdx
    );

extern
STF_RESULT STFLIB_CAR_Config(
    STF_U8 u8IspIdx
    );

extern
STF_RESULT STFLIB_CCM_Config(
    STF_U8 u8IspIdx
    );

extern
STF_RESULT STFLIB_GMARGB_Config(
    STF_U8 u8IspIdx
    );

extern
STF_RESULT STFLIB_R2Y_Config(
    STF_U8 u8IspIdx
    );

extern
STF_RESULT STFLIB_YCRV_Config(
    STF_U8 u8IspIdx
    );

extern
STF_RESULT STFLIB_SHRP_Config(
    STF_U8 u8IspIdx
    );

extern
STF_RESULT STFLIB_DNYUV_Config(
    STF_U8 u8IspIdx
    );

extern
STF_RESULT STFLIB_SAT_Config(
    STF_U8 u8IspIdx
    );

extern
STF_RESULT STFLIB_OBA_Config(
    STF_U8 u8IspIdx
    );

extern
STF_RESULT STFLIB_SC_Config(
    ST_MEMS_INFO *pstMemsInfo
    );

extern
STF_RESULT STFLIB_YHIST_Config(
    ST_MEMS_INFO *pstMemsInfo
    );

extern
STF_RESULT STFLIB_DUMP_Config(
    ST_MEMS_INFO *pstMemsInfo
    );

extern
STF_RESULT STFLIB_OUT_Config(
    ST_MEMS_INFO *pstMemsInfo
    );

extern
STF_RESULT STFLIB_TIL_Config(
    ST_MEMS_INFO *pstMemsInfo
    );

//-----------------------------------------------------------------------------
extern
STF_RESULT STFLIB_ISP_CSI_Config(
    STF_U8 u8IspIdx,
    ST_SIZE stSize
    );

//-----------------------------------------------------------------------------
extern
STF_RESULT STFLIB_Mod_Config(
    ST_MEMS_INFO *pstMemsInfo
    );

//-----------------------------------------------------------------------------


#ifdef __cplusplus
}
#endif


#endif //__STFLIB_MOD_CONFIG_H__
