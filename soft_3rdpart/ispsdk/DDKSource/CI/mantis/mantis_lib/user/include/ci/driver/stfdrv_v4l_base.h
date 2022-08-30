/**
  ******************************************************************************
  * @file  stfdrv_v4l_base.h
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
#ifndef __STFDRV_V4L_BASE_H__
#define __STFDRV_V4L_BASE_H__


#ifdef __cplusplus
extern "C" {
#endif


#include "stf_isp.h"


/* V4L2 base driver structure */


/* V4L2 base driver interface */
//-----------------------------------------------------------------------------
extern
STF_RESULT STFDRV_V4L_BASE_IOCTL(
    CI_CONNECTION *pConnection,
    STF_U32 u32Cmd,
    STF_VOID *pvParam
    );

extern
STF_VOID *STFDRV_V4L_BASE_MMAP(
    CI_CONNECTION *pConnection,
    STF_U32 u32Offset,
    STF_U32 u32Length
    );

extern
STF_RESULT STFDRV_V4L_BASE_MUNMAP(
    CI_CONNECTION *pConnection,
    STF_VOID *pvBuf,
    STF_U32 u32Length
    );

//-----------------------------------------------------------------------------


#ifdef __cplusplus
}
#endif


#endif //__STFDRV_V4L_BASE_H__

