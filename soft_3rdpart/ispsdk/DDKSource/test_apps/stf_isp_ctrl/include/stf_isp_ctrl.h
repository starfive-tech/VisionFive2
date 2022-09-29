/**
  ******************************************************************************
  * @file  stf_isp_ctrl.h
  * @author  StarFive Isp Team
  * @version  V1.0
  * @date  02/07/2022
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
#ifndef __ISP_DEMO_H__
#define __ISP_DEMO_H__


#ifdef __cplusplus
extern "C" {
#endif


#include <termios.h>


#define CAPTURE_WIDTH               (1920)
#define CAPTURE_HEIGHT              (1080)


extern
STF_INT STFAPI_VDEV_StartDetectThread(
    EN_ISP_DEV_IDX enIspIdx
    );
extern
STF_INT STFAPI_VDEV_StopDetectThread(
    EN_ISP_DEV_IDX enIspIdx
    );
//extern
//STF_INT Run(
//    STF_INT argc,
//    STF_CHAR *argv[]
//    );


#ifdef __cplusplus
}
#endif


#endif //__ISP_DEMO_H__
