/**
  ******************************************************************************
  * @file  stf_isp_crop_rdma.h
  * @author  StarFive Isp Team
  * @version  V1.0
  * @date  03/22/2021
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
#ifndef __STF_ISP_CROP_RDMA_H__
#define __STF_ISP_CROP_RDMA_H__


#include "registers/stf_isp_rdma.h"


/* add CSI cropping filter registers structure */
typedef struct _ST_CAPTURE_WINDOW {
    STF_U8 u8IspIdx;
    ST_RECT stRect;
} ST_CAPTURE_WINDOW, *PST_CAPTURE_WINDOW;

typedef struct _ST_CROP_REG {
    REG_CAPTURE_WINDOW_START CapWinStart;           // 0x001C, input picture capture start cfg(x/y start location), each 16bit (cropping).
    REG_CAPTURE_WINDOW_END CapWinEnd;               // 0x0020, input picture capture end cfg(x/y end location), each 16bit (cropping).
} ST_CROP_REG, *PST_CROP_REG;

#define ST_CROP_REG_SIZE        (sizeof(ST_CROP_REG))
#define ST_CROP_REG_LEN         (ST_CROP_REG_SIZE / sizeof(STF_U32))

#pragma pack(push, 8)
typedef struct _ST_CROP_RDMA {
#if defined(SUPPORT_RDMA_FEATURE)
    ST_RDMA_CMD stRdmaStartCmd __attribute__ ((aligned (8)));
#endif //SUPPORT_RDMA_FEATURE
    ST_CROP_REG stCropReg __attribute__ ((aligned (8)));
#if defined(SUPPORT_RDMA_FEATURE)
    ST_RDMA_CMD stRdmaEndCmd __attribute__ ((aligned (8)));
#endif //SUPPORT_RDMA_FEATURE
} ST_CROP_RDMA, *PST_CROP_RDMA;

#define ST_CROP_RDMA_SIZE       (sizeof(ST_CROP_RDMA))
#define ST_CROP_RDMA_LEN        (ST_CROP_RDMA_SIZE / (sizeof(STF_U32) * 2))

typedef struct _ST_ISP_CROP_RDMA {
    STF_U8 u8IspIdx;
    ST_CROP_RDMA stCropRdma __attribute__ ((aligned (8)));
} ST_ISP_CROP_RDMA, *PST_ISP_CROP_RDMA;
#pragma pack(pop)


///* CSI cropping filter registers interface */
////-----------------------------------------------------------------------------
//extern
//STF_VOID STFHAL_ISP_CROP_SetReg(
//    STF_U8 u8IspIdx,
//    ST_CROP_RDMA *pstCropRdma
//    );
////-----------------------------------------------------------------------------
//extern
//STF_VOID STFHAL_ISP_CROP_SetCaptureWindow(
//    STF_U8 u8IspIdx,
//    ST_RECT *pstRect
//    );
////-----------------------------------------------------------------------------
//
//
#endif //__STF_ISP_CROP_RDMA_H__

