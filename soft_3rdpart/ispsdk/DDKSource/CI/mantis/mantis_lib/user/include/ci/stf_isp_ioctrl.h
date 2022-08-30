/**
  ******************************************************************************
  * @file  stf_isp_ioctrl.h
  * @author  StarFive Isp Team
  * @version  V1.0
  * @date  12/16/2021
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
#ifndef __STF_ISP_IOCTRL_H__
#define __STF_ISP_IOCTRL_H__


#ifdef __cplusplus
extern "C" {
#endif


#include <linux/types.h>
#include <linux/ioctl.h>
#include <linux/videodev2.h>
#include <stddef.h>

#include "stf_errors.h"
#include "ci/ci_api_structs.h" /* needed to know the size of structures */


#ifndef __user /* when building fake interface __user is not defined */
#define __user
#endif

#define ISP_IOC                                 ('V')


typedef enum _EN_ISP_IOCTL {
    EN_ISP_IOCTL_LOAD_FW = BASE_VIDIOC_PRIVATE + 1,
    EN_ISP_IOCTL_DMABUF_ALLOC,
    EN_ISP_IOCTL_DMABUF_FREE,
    EN_ISP_IOCTL_GET_HW_VER,
    EN_ISP_IOCTL_REG,
    EN_ISP_IOCTL_SHADOW_LOCK,
    EN_ISP_IOCTL_SHADOW_UNLOCK,
    EN_ISP_IOCTL_SHADOW_UNLOCK_N_TRIGGER,
    EN_ISP_IOCTL_SET_USER_CONFIG_ISP,
    EN_ISP_IOCTL_MAX
} EN_ISP_IOCTL, *PEN_ISP_IOCTL;


#define VIDIOC_STFISP_LOAD_FW \
    _IOW(ISP_IOC, EN_ISP_IOCTL_LOAD_FW, ST_FW_INFO)
#define VIDIOC_STFISP_DMABUF_ALLOC \
    _IOWR(ISP_IOC, EN_ISP_IOCTL_DMABUF_ALLOC, CI_DMA_MEM_PARAM)
#define VIDIOC_STFISP_DMABUF_FREE \
    _IOWR(ISP_IOC, EN_ISP_IOCTL_DMABUF_FREE, CI_DMA_MEM_PARAM)
#define VIDIOC_STFISP_GET_REG \
    _IOWR(ISP_IOC, EN_ISP_IOCTL_REG, ST_ISP_REG_PARAM)
#define VIDIOC_STFISP_SET_REG \
    _IOW(ISP_IOC, EN_ISP_IOCTL_REG, ST_ISP_REG_PARAM)
#define VIDIOC_STFISP_SHADOW_LOCK \
    _IO(ISP_IOC, EN_ISP_IOCTL_SHADOW_LOCK)
#define VIDIOC_STFISP_SHADOW_UNLOCK \
    _IO(ISP_IOC, EN_ISP_IOCTL_SHADOW_UNLOCK)
#define VIDIOC_STFISP_SHADOW_UNLOCK_N_TRIGGER \
    _IO(ISP_IOC, EN_ISP_IOCTL_SHADOW_UNLOCK_N_TRIGGER)
#define VIDIOC_STFISP_SET_USER_CONFIG_ISP \
    _IO(ISP_IOC, EN_ISP_IOCTL_SET_USER_CONFIG_ISP)


#ifdef __cplusplus
}
#endif


#endif /* __STF_ISP_IOCTRL_H__ */
