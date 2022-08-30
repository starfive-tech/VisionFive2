/**
  ******************************************************************************
  * @file  stf_isp_sud_rdma.h
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
#ifndef __STF_ISP_SUD_RDMA_H__
#define __STF_ISP_SUD_RDMA_H__


#include "registers/stf_isp_rdma.h"


/* add shadow update registers structure */
typedef enum _EN_SUD_MODE {
    EN_SUD_MODE_IMMEDIATELY = 0,
    EN_SUD_MODE_NEXT_VSYNC,
    //EN_SUD_MODE_MAX
} EN_SUD_MODE, *PEN_SUD_MODE;

typedef enum _EN_SUD_CTRL {
    EN_SUD_CTRL_DISABLE = 0,
    EN_SUD_CTRL_ENABLE,
    //EN_SUD_CTRL_MAX
} EN_SUD_CTRL, *PEN_SUD_CTRL;


typedef struct _ST_SUD_MODE_PARAM {
    STF_U8 u8IspIdx;
    EN_SUD_MODE enShadowMode;
} ST_SUD_MODE_PARAM, *PST_SUD_MODE_PARAM;

typedef struct _ST_SHADOW_PARAM {
    EN_SUD_MODE enShadowMode;                   /** 0: shadow load immediately on write 1 to UD, 1: shadow load at frame start, must write 1 to UD to arm the shadow load. */
    EN_SUD_CTRL enShadowEnable;                 /** Write 1 to trigger immediate shadow load when UDM=0, Write 1 to arm shadow load at frame start when UDM=1. */
} ST_SHADOW_PARAM, *PST_SHADOW_PARAM;

typedef struct _ST_SUD_SHADOW_PARAM {
    STF_U8 u8IspIdx;
    ST_SHADOW_PARAM stShadowParam;
} ST_SUD_SHADOW_PARAM, *PST_SUD_SHADOW_PARAM;


#endif //__STF_ISP_SUD_RDMA_H__
