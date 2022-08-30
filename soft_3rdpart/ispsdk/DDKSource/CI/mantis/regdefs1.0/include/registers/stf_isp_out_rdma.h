/**
  ******************************************************************************
  * @file  stf_isp_out_rdma.h
  * @author  StarFive Isp Team
  * @version  V1.0
  * @date  01/06/2021
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
#ifndef __STF_ISP_OUT_RDMA_H__
#define __STF_ISP_OUT_RDMA_H__


#include "registers/stf_isp_rdma.h"


/* add ISP video output interface registers structure */
#define OUT_SS_SF_MAX                   (1 << 12)


typedef enum _EN_UV_ORDER {
    EN_UV_ORDER_UV = 0,                 // NV12.
    EN_UV_ORDER_VU,                     // NV21.
    //EN_UV_ORDER_MAX
} EN_UV_ORDER, *PEN_UV_ORDER;

typedef enum _EN_SCALING_MODE {
    EN_SCALING_MODE_SCALING_DOWN = 0,
    EN_SCALING_MODE_NO_SCALING = 2,
    //EN_SCALING_MODE_MAX
} EN_SCALING_MODE, *PEN_SCALING_MODE;


typedef struct _ST_OUT_UO_INFO {
    STF_U32 u32YAddr;
    STF_U32 u32UvAddr;
    STF_U16 u32Stride;
    EN_UV_ORDER eUvOrder;
    STF_U8 u8AxiId;
} ST_OUT_UO_INFO, *PST_OUT_UO_INFO;

typedef struct _ST_OUT_UO_INFO_PARAM {
    STF_U8 u8IspIdx;
    ST_OUT_UO_INFO stOutUoInfo;
} ST_OUT_UO_INFO_PARAM, *PST_OUT_UO_INFO_PARAM;

typedef struct _ST_OUT_SS_INFO {
    STF_U32 u32YAddr;
    STF_U32 u32UvAddr;
    STF_U16 u32Stride;
    EN_UV_ORDER eUvOrder;
    EN_SCALING_MODE eHorScalingMode;
    STF_U16 u16HorScalingFactor;
    EN_SCALING_MODE eVerScalingMode;
    STF_U16 u16VerScalingFactor;
    STF_U16 u16OutWidth;
    STF_U16 u16OutHeight;
    STF_U8 u8AxiId;
} ST_OUT_SS_INFO, *PST_OUT_SS_INFO;

typedef struct _ST_OUT_SS_INFO_PARAM {
    STF_U8 u8IspIdx;
    ST_OUT_SS_INFO stOutSsInfo;
} ST_OUT_SS_INFO_PARAM, *PST_OUT_SS_INFO_PARAM;

/* add ISP video output UO interface registers structure */
typedef struct _ST_OUT_UO_INFO_REG {
  #if defined(V4L2_DRIVER)
  #else
    REG_UOAY UoYAddr;               // 0x0A80, Unscaled Output Image Y Plane Start Address Register, 8-byte alignment.
    REG_UOAUV UoUvAddr;             // 0x0A84, Unscaled Output Image UV Plane Start Address Register, 8-byte alignment.
    REG_UOS UoStride;               // 0x0A88, Unscaled Output Image Stride Register, 8-byte(64bit) granularity.
  #endif //#if defined(V4L2_DRIVER)
    REG_UOPGM UoUvMode;             // 0x0A8C, Unscaled Output Pixel Coordinate Generator Mode Register.
    REG_UOAXI UoAxiId;              // 0x0A90, Unscaled Output AXI Control Register.
} ST_OUT_UO_INFO_REG, *PST_OUT_UO_INFO_REG;

#define ST_OUT_UO_INFO_REG_SIZE     (sizeof(ST_OUT_UO_INFO_REG))
#define ST_OUT_UO_INFO_REG_LEN      (ST_OUT_UO_INFO_REG_SIZE / sizeof(STF_U32))

typedef struct _ST_OUT_UO_REG {
  #if defined(V4L2_DRIVER)
  #else
    REG_UOAY UoYAddr;               // 0x0A80, Unscaled Output Image Y Plane Start Address Register, 8-byte alignment.
    REG_UOAUV UoUvAddr;             // 0x0A84, Unscaled Output Image UV Plane Start Address Register, 8-byte alignment.
    REG_UOS UoStride;               // 0x0A88, Unscaled Output Image Stride Register, 8-byte(64bit) granularity.
  #endif //#if defined(V4L2_DRIVER)
    REG_UOPGM UoUvMode;             // 0x0A8C, Unscaled Output Pixel Coordinate Generator Mode Register.
    REG_UOAXI UoAxiId;              // 0x0A90, Unscaled Output AXI Control Register.
} ST_OUT_UO_REG, *PST_OUT_UO_REG;

#define ST_OUT_UO_REG_SIZE      (sizeof(ST_OUT_UO_REG))
#define ST_OUT_UO_REG_LEN       (ST_OUT_UO_REG_SIZE / sizeof(STF_U32))

/* add ISP video output SS0 interface registers structure */
typedef struct _ST_OUT_SS0_INFO_REG {
    REG_UOPGM UoUvMode;             // 0x0A8C, Unscaled Output Pixel Coordinate Generator Mode Register.

  #if defined(V4L2_DRIVER)
  #else
    REG_SS0AY Ss0YAddr;             // 0x0A94, SS0 Output Image Y Plane Start Address Register, 8-byte alignment.
    REG_SS0AUV Ss0UvAddr;           // 0x0A98, SS0 Output Image UV Plane Start Address Register, 8-byte alignment.
  #endif //#if defined(V4L2_DRIVER)
    REG_SS0S Ss0Stride;             // 0x0A9C, SS0 Output Image Stride Register, 8-byte(64bit) granularity.
    REG_SS0HF Ss0HSF;               // 0x0AA0, SS0 H Scaling Factor Register.
    REG_SS0VF Ss0VSF;               // 0x0AA4, SS0 V Scaling Factor Register.
    REG_SS0IW Ss0Size;              // 0x0AA8, SS0 Output Image Size Register.

    REG_SSAXI SsAxiId;              // 0x0AC4, SS AXI Control Register.
} ST_OUT_SS0_INFO_REG, *PST_OUT_SS0_INFO_REG;

#define ST_OUT_SS0_INFO_REG_SIZE    (sizeof(ST_OUT_SS0_INFO_REG))
#define ST_OUT_SS0_INFO_REG_LEN     (ST_OUT_SS0_INFO_REG_SIZE / sizeof(STF_U32))

typedef struct _ST_OUT_SS0_REG {
  #if defined(V4L2_DRIVER)
  #else
    REG_SS0AY Ss0YAddr;             // 0x0A94, SS0 Output Image Y Plane Start Address Register, 8-byte alignment.
    REG_SS0AUV Ss0UvAddr;           // 0x0A98, SS0 Output Image UV Plane Start Address Register, 8-byte alignment.
  #endif //#if defined(V4L2_DRIVER)
    REG_SS0S Ss0Stride;             // 0x0A9C, SS0 Output Image Stride Register, 8-byte(64bit) granularity.
    REG_SS0HF Ss0HSF;               // 0x0AA0, SS0 H Scaling Factor Register.
    REG_SS0VF Ss0VSF;               // 0x0AA4, SS0 V Scaling Factor Register.
    REG_SS0IW Ss0Size;              // 0x0AA8, SS0 Output Image Size Register.
} ST_OUT_SS0_REG, *PST_OUT_SS0_REG;

#define ST_OUT_SS0_REG_SIZE     (sizeof(ST_OUT_SS0_REG))
#define ST_OUT_SS0_REG_LEN      (ST_OUT_SS0_REG_SIZE / sizeof(STF_U32))

/* add ISP video output SS1 interface registers structure */
typedef struct _ST_OUT_SS1_INFO_REG {
    REG_UOPGM UoUvMode;             // 0x0A8C, Unscaled Output Pixel Coordinate Generator Mode Register.

  #if defined(V4L2_DRIVER)
  #else
    REG_SS1AY Ss1YAddr;             // 0x0AAC, SS1 Output Image Y Plane Start Address Register, 8-byte alignment.
    REG_SS1AUV Ss1UvAddr;           // 0x0AB0, SS1 Output Image UV Plane Start Address Register, 8-byte alignment.
  #endif //#if defined(V4L2_DRIVER)
    REG_SS1S Ss1Stride;             // 0x0AB4, SS1 Output Image Stride Register, 8-byte(64bit) granularity.
    REG_SS1HF Ss1HSF;               // 0x0AB8, SS1 H Scaling Factor Register.
    REG_SS1VF Ss1VSF;               // 0x0ABC, SS1 V Scaling Factor Register.
    REG_SS1IW Ss1Size;              // 0x0AC0, SS1 Output Image Size Register.

    REG_SSAXI SsAxiId;              // 0x0AC4, SS AXI Control Register.
} ST_OUT_SS1_INFO_REG, *PST_OUT_SS1_INFO_REG;

#define ST_OUT_SS1_INFO_REG_SIZE    (sizeof(ST_OUT_SS1_INFO_REG))
#define ST_OUT_SS1_INFO_REG_LEN     (ST_OUT_SS1_INFO_REG_SIZE / sizeof(STF_U32))

typedef struct _ST_OUT_SS1_REG {
  #if defined(V4L2_DRIVER)
  #else
    REG_SS1AY Ss1YAddr;             // 0x0AAC, SS1 Output Image Y Plane Start Address Register, 8-byte alignment.
    REG_SS1AUV Ss1UvAddr;           // 0x0AB0, SS1 Output Image UV Plane Start Address Register, 8-byte alignment.
  #endif //#if defined(V4L2_DRIVER)
    REG_SS1S Ss1Stride;             // 0x0AB4, SS1 Output Image Stride Register, 8-byte(64bit) granularity.
    REG_SS1HF Ss1HSF;               // 0x0AB8, SS1 H Scaling Factor Register.
    REG_SS1VF Ss1VSF;               // 0x0ABC, SS1 V Scaling Factor Register.
    REG_SS1IW Ss1Size;              // 0x0AC0, SS1 Output Image Size Register.
} ST_OUT_SS1_REG, *PST_OUT_SS1_REG;

#define ST_OUT_SS1_REG_SIZE     (sizeof(ST_OUT_SS1_REG))
#define ST_OUT_SS1_REG_LEN      (ST_OUT_SS1_REG_SIZE / sizeof(STF_U32))

typedef struct _ST_OUT_SS_AXI_REG {
    REG_SSAXI SsAxiId;              // 0x0AC4, SS AXI Control Register.
} ST_OUT_SS_AXI_REG, *PST_OUT_SS_AXI_REG;

#define ST_OUT_SS_AXI_REG_SIZE  (sizeof(ST_OUT_SS_AXI_REG))
#define ST_OUT_SS_AXI_REG_LEN   (ST_OUT_SS_AXI_REG_SIZE / sizeof(STF_U32))

typedef struct _ST_OUT_REG {
  #if defined(V4L2_DRIVER)
  #else
    REG_UOAY UoYAddr;               // 0x0A80, Unscaled Output Image Y Plane Start Address Register, 8-byte alignment.
    REG_UOAUV UoUvAddr;             // 0x0A84, Unscaled Output Image UV Plane Start Address Register, 8-byte alignment.
  #endif //#if defined(V4L2_DRIVER)
    REG_UOS UoStride;               // 0x0A88, Unscaled Output Image Stride Register, 8-byte(64bit) granularity.
    REG_UOPGM UoUvMode;             // 0x0A8C, Unscaled Output Pixel Coordinate Generator Mode Register.
    REG_UOAXI UoAxiId;              // 0x0A90, Unscaled Output AXI Control Register.

  #if defined(V4L2_DRIVER)
  #else
    REG_SS0AY Ss0YAddr;             // 0x0A94, SS0 Output Image Y Plane Start Address Register, 8-byte alignment.
    REG_SS0AUV Ss0UvAddr;           // 0x0A98, SS0 Output Image UV Plane Start Address Register, 8-byte alignment.
  #endif //#if defined(V4L2_DRIVER)
    REG_SS0S Ss0Stride;             // 0x0A9C, SS0 Output Image Stride Register, 8-byte(64bit) granularity.
    REG_SS0HF Ss0HSF;               // 0x0AA0, SS0 H Scaling Factor Register.
    REG_SS0VF Ss0VSF;               // 0x0AA4, SS0 V Scaling Factor Register.
    REG_SS0IW Ss0Size;              // 0x0AA8, SS0 Output Image Size Register.

  #if defined(V4L2_DRIVER)
  #else
    REG_SS1AY Ss1YAddr;             // 0x0AAC, SS1 Output Image Y Plane Start Address Register, 8-byte alignment.
    REG_SS1AUV Ss1UvAddr;           // 0x0AB0, SS1 Output Image UV Plane Start Address Register, 8-byte alignment.
  #endif //#if defined(V4L2_DRIVER)
    REG_SS1S Ss1Stride;             // 0x0AB4, SS1 Output Image Stride Register, 8-byte(64bit) granularity.
    REG_SS1HF Ss1HSF;               // 0x0AB8, SS1 H Scaling Factor Register.
    REG_SS1VF Ss1VSF;               // 0x0ABC, SS1 V Scaling Factor Register.
    REG_SS1IW Ss1Size;              // 0x0AC0, SS1 Output Image Size Register.

    REG_SSAXI SsAxiId;              // 0x0AC4, SS AXI Control Register.
} ST_OUT_REG, *PST_OUT_REG;

#define ST_OUT_REG_SIZE         (sizeof(ST_OUT_REG))
#define ST_OUT_REG_LEN          (ST_OUT_REG_SIZE / sizeof(STF_U32))

#pragma pack(push, 8)
typedef struct _ST_OUT_RDMA {
#if defined(SUPPORT_RDMA_FEATURE)
    ST_RDMA_CMD stRdmaStartCmd __attribute__ ((aligned (8)));
#endif //SUPPORT_RDMA_FEATURE
    ST_OUT_UO_REG stOutUoReg __attribute__ ((aligned (8)));
#if defined(SUPPORT_RDMA_FEATURE)
    ST_RDMA_CMD stRdmaSs0Cmd __attribute__ ((aligned (8)));
#endif //SUPPORT_RDMA_FEATURE
    ST_OUT_SS0_REG stOutSs0Reg __attribute__ ((aligned (8)));
#if defined(SUPPORT_RDMA_FEATURE)
    ST_RDMA_CMD stRdmaSs1Cmd __attribute__ ((aligned (8)));
#endif //SUPPORT_RDMA_FEATURE
    ST_OUT_SS1_REG stOutSs1Reg __attribute__ ((aligned (8)));
#if defined(SUPPORT_RDMA_FEATURE)
    ST_RDMA_CMD stRdmaSsAxiCmd __attribute__ ((aligned (8)));
#endif //SUPPORT_RDMA_FEATURE
    ST_OUT_SS_AXI_REG stOutSsAxiReg __attribute__ ((aligned (8)));
#if defined(SUPPORT_RDMA_FEATURE)
    ST_RDMA_CMD stRdmaEndCmd __attribute__ ((aligned (8)));
#endif //SUPPORT_RDMA_FEATURE
} ST_OUT_RDMA, *PST_OUT_RDMA;

#define ST_OUT_RDMA_SIZE        (sizeof(ST_OUT_RDMA))
#define ST_OUT_RDMA_LEN         (ST_OUT_RDMA_SIZE / (sizeof(STF_U32) * 2))

typedef struct _ST_ISP_OUT_RDMA {
    STF_U8 u8IspIdx;
    ST_OUT_RDMA stOutRdma __attribute__ ((aligned (8)));
} ST_ISP_OUT_RDMA, *PST_ISP_OUT_RDMA;
#pragma pack(pop)

#pragma pack(push, 8)
typedef struct _ST_OUT_UO_RDMA {
#if defined(SUPPORT_RDMA_FEATURE)
    ST_RDMA_CMD stRdmaStartCmd __attribute__ ((aligned (8)));
#endif //SUPPORT_RDMA_FEATURE
    ST_OUT_UO_REG stOutUoReg __attribute__ ((aligned (8)));
#if defined(SUPPORT_RDMA_FEATURE)
    ST_RDMA_CMD stRdmaEndCmd __attribute__ ((aligned (8)));
#endif //SUPPORT_RDMA_FEATURE
} ST_OUT_UO_RDMA, *PST_OUT_UO_RDMA;

#define ST_OUT_UO_RDMA_SIZE     (sizeof(ST_OUT_UO_RDMA))
#define ST_OUT_UO_RDMA_LEN      (ST_OUT_UO_RDMA_SIZE / (sizeof(STF_U32) * 2))

typedef struct _ST_ISP_OUT_UO_RDMA {
    STF_U8 u8IspIdx;
    ST_OUT_UO_RDMA stOutUoRdma __attribute__ ((aligned (8)));
} ST_ISP_OUT_UO_RDMA, *PST_ISP_OUT_UO_RDMA;
#pragma pack(pop)

#pragma pack(push, 8)
typedef struct _ST_OUT_SS0_RDMA {
#if defined(SUPPORT_RDMA_FEATURE)
    ST_RDMA_CMD stRdmaStartCmd __attribute__ ((aligned (8)));
#endif //SUPPORT_RDMA_FEATURE
    REG_UOPGM stUvMode __attribute__ ((aligned (8)));   // 0x0A8C, Unscaled Output Pixel Coordinate Generator Mode Register.
#if defined(SUPPORT_RDMA_FEATURE)
    ST_RDMA_CMD stRdmaOutSs0Cmd __attribute__ ((aligned (8)));
#endif //SUPPORT_RDMA_FEATURE
    ST_OUT_SS0_REG stOutSs0Reg __attribute__ ((aligned (8)));
#if defined(SUPPORT_RDMA_FEATURE)
    ST_RDMA_CMD stRdmaSsxAxiCmd __attribute__ ((aligned (8)));
#endif //SUPPORT_RDMA_FEATURE
    REG_SSAXI stSsAxiId __attribute__ ((aligned (8)));  // 0x0AC4, SS AXI Control Register.
#if defined(SUPPORT_RDMA_FEATURE)
    ST_RDMA_CMD stRdmaEndCmd __attribute__ ((aligned (8)));
#endif //SUPPORT_RDMA_FEATURE
} ST_OUT_SS0_RDMA, *PST_OUT_SS0_RDMA;

#define ST_OUT_SS0_RDMA_SIZE    (sizeof(ST_OUT_SS0_RDMA))
#define ST_OUT_SS0_RDMA_LEN     (ST_OUT_SS0_RDMA_SIZE / (sizeof(STF_U32) * 2))

typedef struct _ST_ISP_OUT_SS0_RDMA {
    STF_U8 u8IspIdx;
    ST_OUT_SS0_RDMA stOutSs0Rdma __attribute__ ((aligned (8)));
} ST_ISP_OUT_SS0_RDMA, *PST_ISP_OUT_SS0_RDMA;
#pragma pack(pop)

#pragma pack(push, 8)
typedef struct _ST_OUT_SS1_RDMA {
#if defined(SUPPORT_RDMA_FEATURE)
    ST_RDMA_CMD stRdmaStartCmd __attribute__ ((aligned (8)));
#endif //SUPPORT_RDMA_FEATURE
    REG_UOPGM stUvMode __attribute__ ((aligned (8)));   // 0x0A8C, Unscaled Output Pixel Coordinate Generator Mode Register.
#if defined(SUPPORT_RDMA_FEATURE)
    ST_RDMA_CMD stRdmaOutSs1Cmd __attribute__ ((aligned (8)));
#endif //SUPPORT_RDMA_FEATURE
    ST_OUT_SS1_REG stOutSs1Reg __attribute__ ((aligned (8)));
#if defined(SUPPORT_RDMA_FEATURE)
    ST_RDMA_CMD stRdmaSsxAxiCmd __attribute__ ((aligned (8)));
#endif //SUPPORT_RDMA_FEATURE
    REG_SSAXI stSsAxiId __attribute__ ((aligned (8)));  // 0x0AC4, SS AXI Control Register.
#if defined(SUPPORT_RDMA_FEATURE)
    ST_RDMA_CMD stRdmaEndCmd __attribute__ ((aligned (8)));
#endif //SUPPORT_RDMA_FEATURE
} ST_OUT_SS1_RDMA, *PST_OUT_SS1_RDMA;

#define ST_OUT_SS1_RDMA_SIZE    (sizeof(ST_OUT_SS1_RDMA))
#define ST_OUT_SS1_RDMA_LEN     (ST_OUT_SS1_RDMA_SIZE / (sizeof(STF_U32) * 2))

typedef struct _ST_ISP_OUT_SS1_RDMA {
    STF_U8 u8IspIdx;
    ST_OUT_SS1_RDMA stOutSs1Rdma __attribute__ ((aligned (8)));
} ST_ISP_OUT_SS1_RDMA, *PST_ISP_OUT_SS1_RDMA;
#pragma pack(pop)


///* ISP video output interface registers interface */
////-----------------------------------------------------------------------------
//extern
//STF_VOID STFHAL_ISP_OUT_SetReg(
//    STF_U8 u8IspIdx,
//    ST_OUT_RDMA *pstOutRdma
//    );
////-----------------------------------------------------------------------------
//extern
//STF_VOID STFHAL_ISP_OUT_SetUoInfo(
//    STF_U8 u8IspIdx,
//    ST_OUT_UO_INFO *pstOutUoInfo
//    );
//extern
//STF_VOID STFHAL_ISP_OUT_SetSs0Info(
//    STF_U8 u8IspIdx,
//    ST_OUT_SS_INFO *pstOutSs0Info
//    );
//extern
//STF_VOID STFHAL_ISP_OUT_SetSs1Info(
//    STF_U8 u8IspIdx,
//    ST_OUT_SS_INFO *pstOutSs1Info
//    );
////-----------------------------------------------------------------------------
//
///* ISP video output UO interface registers interface */
////-----------------------------------------------------------------------------
//extern
//STF_VOID STFHAL_ISP_OUT_UO_SetReg(
//    STF_U8 u8IspIdx,
//    ST_OUT_UO_RDMA *pstOutUoRdma
//    );
////-----------------------------------------------------------------------------
//
///* ISP video output SS0 interface registers interface */
////-----------------------------------------------------------------------------
//extern
//STF_VOID STFHAL_ISP_OUT_SS0_SetReg(
//    STF_U8 u8IspIdx,
//    ST_OUT_SS0_RDMA *pstOutSs0Rdma
//    );
////-----------------------------------------------------------------------------
//
///* ISP video output SS1 interface registers interface */
////-----------------------------------------------------------------------------
//extern
//STF_VOID STFHAL_ISP_OUT_SS1_SetReg(
//    STF_U8 u8IspIdx,
//    ST_OUT_SS1_RDMA *pstOutSs1Rdma
//    );
////-----------------------------------------------------------------------------
//
//
#endif //__STF_ISP_OUT_RDMA_H__
