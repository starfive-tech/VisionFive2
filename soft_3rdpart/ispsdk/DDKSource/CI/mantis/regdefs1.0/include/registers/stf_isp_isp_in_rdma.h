/**
  ******************************************************************************
  * @file  stf_isp_isp_in_rdma.h
  * @author  StarFive Isp Team
  * @version  V1.0
  * @date  03/24/2021
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
#ifndef __STF_ISP_ISP_IN_RDMA_H__
#define __STF_ISP_ISP_IN_RDMA_H__


#include "registers/stf_isp_rdma.h"


/* add ISP input registers structure */
typedef enum _EN_ISP_1ST_PXL {
    EN_ISP_1ST_PXL_R = 0,
    EN_ISP_1ST_PXL_B = 1,
    EN_ISP_1ST_PXL_GR = 2,
    EN_ISP_1ST_PXL_GB = 3,
    //EN_ISP_1ST_PXL_MAX
} EN_ISP_1ST_PXL, *PEN_ISP_1ST_PXL;

typedef enum _EN_ISP_MULTI_FRAME_MODE {
    EN_ISP_MULTI_FRAME_MODE_DISABLE = 0,
    EN_ISP_MULTI_FRAME_MODE_ENABLE,
    //EN_ISP_MULTI_FRAME_MODE_MAX
} EN_ISP_MULTI_FRAME_MODE, *PEN_ISP_MULTI_FRAME_MODE;


typedef struct _ST_ISP_MULTI_FRAME_MODE {
    STF_U8 u8IspIdx;
    EN_ISP_MULTI_FRAME_MODE enIspMultiFrameMode;
} ST_ISP_MULTI_FRAME_MODE, *PST_ISP_MULTI_FRAME_MODE;

typedef struct _ST_ISP_FIRST_PIXEL {
    STF_U8 u8IspIdx;
    EN_ISP_1ST_PXL enFirstPixel;
} ST_ISP_FIRST_PIXEL, *PST_ISP_FIRST_PIXEL;

typedef struct _ST_CAPTURE_SIZE {
    STF_U8 u8IspIdx;
    ST_SIZE stSize;
} ST_CAPTURE_SIZE, *PST_CAPTURE_SIZE;

typedef struct _ST_ISP_IN_REG {
    //REG_ISPCTRL0 IspCtrl0;                          // 0x0A00, ISP Control Register 0.
    //REG_ISPCTRL1 IspCtrl1;                          // 0x0A08, ISP Control Register 1.
    //REG_IFSZ IspCaptureSize;                        // 0x0A0C, isp pipeline X/Y size, each 13bits(after decimation)
} ST_ISP_IN_REG, *PST_ISP_IN_REG;

#define ST_ISP_IN_REG_SIZE      (sizeof(ST_ISP_IN_REG))
#define ST_ISP_IN_REG_LEN       (ST_ISP_IN_REG_SIZE / sizeof(STF_U32))

#pragma pack(push, 8)
typedef struct _ST_ISP_IN_RDMA {
#if defined(SUPPORT_RDMA_FEATURE)
    ST_RDMA_CMD stRdmaStartCmd __attribute__ ((aligned (8)));
#endif //SUPPORT_RDMA_FEATURE
    ST_ISP_IN_REG stIspInReg __attribute__ ((aligned (8)));
#if defined(SUPPORT_RDMA_FEATURE)
    ST_RDMA_CMD stRdmaEndCmd __attribute__ ((aligned (8)));
#endif //SUPPORT_RDMA_FEATURE
} ST_ISP_IN_RDMA, *PST_ISP_IN_RDMA;

#define ST_ISP_IN_RDMA_SIZE     (sizeof(ST_ISP_IN_RDMA))
#define ST_ISP_IN_RDMA_LEN      (ST_ISP_IN_RDMA_SIZE / (sizeof(STF_U32) * 2))

typedef struct _ST_ISP_ISP_IN_RDMA {
    STF_U8 u8IspIdx;
    ST_ISP_IN_RDMA stIspInRdma __attribute__ ((aligned (8)));
} ST_ISP_ISP_IN_RDMA, *PST_ISP_ISP_IN_RDMA;
#pragma pack(pop)


///* ISP input registers interface */
////-----------------------------------------------------------------------------
//extern
//STF_VOID STFHAL_ISP_ISP_IN_SetReg(
//    STF_U8 u8IspIdx,
//    ST_ISP_IN_RDMA *pstIspInRdma
//    );
////-----------------------------------------------------------------------------
//extern
//STF_VOID STFHAL_ISP_ISP_IN_SetMultipleFrame(
//    STF_U8 u8IspIdx,
//    EN_ISP_MULTI_FRAME_MODE enIspMultiFrameMode
//    );
//extern
//STF_VOID STFHAL_ISP_ISP_IN_SetFirstPixel(
//    STF_U8 u8IspIdx,
//    EN_ISP_1ST_PXL enFirstPixel
//    );
//extern
//STF_VOID STFHAL_ISP_ISP_IN_SetCaptureSize(
//    STF_U8 u8IspIdx,
//    ST_SIZE stCaptureSize
//    );
////-----------------------------------------------------------------------------
//
//
#endif //__STF_ISP_ISP_IN_RDMA_H__

