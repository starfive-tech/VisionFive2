/**
  ******************************************************************************
  * @file  stf_isp_lccf_rdma.h
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
#ifndef __STF_ISP_LCCF_RDMA_H__
#define __STF_ISP_LCCF_RDMA_H__


#include "registers/stf_isp_rdma.h"


/* add lens correction cos4 registers structure */
typedef enum _EN_LCCF_CHN {
    EN_LCCF_CHN_R = 0,
    EN_LCCF_CHN_GR,
    EN_LCCF_CHN_GB,
    EN_LCCF_CHN_B,
    EN_LCCF_CHN_MAX
} EN_LCCF_CHN, *PEN_LCCF_CHN;


typedef struct _ST_LCCF_CENTER_PARAM {
    STF_U8 u8IspIdx;
    ST_POINT stPoint;
} ST_LCCF_CENTER_PARAM, *PST_LCCF_CENTER_PARAM;

typedef struct _ST_LCCF_RADIUS_PARAM {
    STF_U8 u8IspIdx;
    STF_U8 u8Radius;
} ST_LCCF_RADIUS_PARAM, *PST_LCCF_RADIUS_PARAM;

typedef struct _ST_LCCF_FACTOR_PARAM {
    STF_U16 au16F1[EN_LCCF_CHN_MAX];    // R/Gr/Gb/B channel factor 1 of LCCF module.
    STF_U16 au16F2[EN_LCCF_CHN_MAX];    // R/Gr/Gb/B channel factor 2 of LCCF module.
} ST_LCCF_FACTOR_PARAM, *PST_LCCF_FACTOR_PARAM;

typedef struct _ST_ISP_LCCF_FACTOR_PARAM {
    STF_U8 u8IspIdx;
    ST_LCCF_FACTOR_PARAM stLccfFactor;
} ST_ISP_LCCF_FACTOR_PARAM, *PST_ISP_LCCF_FACTOR_PARAM;

typedef struct _ST_LCCF_CTR_REG {
    REG_LCCF_CFG_0 Center;          // 0x0050, X/Y distance from capture window(each 15bits).
} ST_LCCF_CTR_REG, *PST_LCCF_CTR_REG;

#define ST_LCCF_CTR_REG_SIZE    (sizeof(ST_LCCF_CTR_REG))
#define ST_LCCF_CTR_REG_LEN     (ST_LCCF_CTR_REG_SIZE / sizeof(STF_U32))

typedef struct _ST_LCCF_RAD_REG {
    REG_LCCF_CFG_1 Radius;          // 0x0058, LCCF maximum distance register.
} ST_LCCF_RAD_REG, *PST_LCCF_RAD_REG;

#define ST_LCCF_RAD_REG_SIZE    (sizeof(ST_LCCF_RAD_REG))
#define ST_LCCF_RAD_REG_LEN     (ST_LCCF_RAD_REG_SIZE / sizeof(STF_U32))

typedef struct _ST_LCCF_CFG_REG {
    REG_LCCF_CFG_0 Center;          // 0x0050, X/Y distance from capture window(each 15bits).
    STF_U32 u32Reserved_0054[1];    // Added reserved variables to let ST_LCCF_REG address is contiguous.
    REG_LCCF_CFG_1 Radius;          // 0x0058, LCCF maximum distance register.
} ST_LCCF_CFG_REG, *PST_LCCF_CFG_REG;

#define ST_LCCF_CFG_REG_SIZE    (sizeof(ST_LCCF_CFG_REG))
#define ST_LCCF_CFG_REG_LEN     (ST_LCCF_CFG_REG_SIZE / sizeof(STF_U32))

typedef struct _ST_LCCF_FACTOR_REG {
    REG_LCCF_CFG_2 RFactor;         // 0x00E0, LCCF F parameter for symbol 0 register.
    REG_LCCF_CFG_3 GrFactor;        // 0x00E4, LCCF F parameter for symbol 1 register.
    REG_LCCF_CFG_4 GbFactor;        // 0x00E8, LCCF F parameter for symbol 2 register.
    REG_LCCF_CFG_5 BFactor;         // 0x00EC, LCCF F parameter for symbol 3 register.
} ST_LCCF_FACTOR_REG, *PST_LCCF_FACTOR_REG;

#define ST_LCCF_FACTOR_REG_SIZE (sizeof(ST_LCCF_FACTOR_REG))
#define ST_LCCF_FACTOR_REG_LEN  (ST_LCCF_FACTOR_REG_SIZE / sizeof(STF_U32))

typedef struct _ST_LCCF_REG {
    REG_LCCF_CFG_0 Center;          // 0x0050, X/Y distance from capture window(each 15bits).
    STF_U32 u32Reserved_0054[1];    // Added reserved variables to let ST_LCCF_REG address is contiguous.
    REG_LCCF_CFG_1 Radius;          // 0x0058, LCCF maximum distance register.
    //STF_U32 u32Reserved[?];         // Added reserved variables to let ST_LCCF_REG address is contiguous.
    //-------------------------------------------------------------------------
    REG_LCCF_CFG_2 RFactor;         // 0x00E0, LCCF F parameter for symbol 0 register.
    REG_LCCF_CFG_3 GrFactor;        // 0x00E4, LCCF F parameter for symbol 1 register.
    REG_LCCF_CFG_4 GbFactor;        // 0x00E8, LCCF F parameter for symbol 2 register.
    REG_LCCF_CFG_5 BFactor;         // 0x00EC, LCCF F parameter for symbol 3 register.
} ST_LCCF_REG, *PST_LCCF_REG;

#define ST_LCCF_REG_SIZE        (sizeof(ST_LCCF_REG))
#define ST_LCCF_REG_LEN         (ST_LCCF_REG_SIZE / sizeof(STF_U32))

#pragma pack(push, 8)
typedef struct _ST_LCCF_RDMA {
#if defined(SUPPORT_RDMA_FEATURE)
    ST_RDMA_CMD stRdmaStartCmd __attribute__ ((aligned (8)));
#endif //SUPPORT_RDMA_FEATURE
    ST_LCCF_CTR_REG stLccfCtrReg __attribute__ ((aligned (8)));
#if defined(SUPPORT_RDMA_FEATURE)
    ST_RDMA_CMD stRdmaRadCmd __attribute__ ((aligned (8)));
#endif //SUPPORT_RDMA_FEATURE
    ST_LCCF_RAD_REG stLccfRadReg __attribute__ ((aligned (8)));
#if defined(SUPPORT_RDMA_FEATURE)
    ST_RDMA_CMD stRdmaFactorCmd __attribute__ ((aligned (8)));
#endif //SUPPORT_RDMA_FEATURE
    ST_LCCF_FACTOR_REG stLccfFactorReg __attribute__ ((aligned (8)));
#if defined(SUPPORT_RDMA_FEATURE)
    ST_RDMA_CMD stRdmaEndCmd __attribute__ ((aligned (8)));
#endif //SUPPORT_RDMA_FEATURE
} ST_LCCF_RDMA, *PST_LCCF_RDMA;

#define ST_LCCF_RDMA_SIZE           (sizeof(ST_LCCF_RDMA))
#define ST_LCCF_RDMA_LEN            (ST_LCCF_RDMA_SIZE / (sizeof(STF_U32) * 2))
#define ST_LCCF_CTR_RDMA_SIZE       (sizeof(ST_RDMA_CMD) + (((sizeof(ST_LCCF_CTR_REG) + 7) / 8) * 8))
#define ST_LCCF_CTR_RDMA_LEN        (ST_LCCF_CTR_RDMA_SIZE / (sizeof(STF_U32) * 2))
#define ST_LCCF_RAD_RDMA_SIZE       (sizeof(ST_RDMA_CMD) + (((sizeof(ST_LCCF_RAD_REG) + 7) / 8) * 8))
#define ST_LCCF_RAD_RDMA_LEN        (ST_LCCF_RAD_RDMA_SIZE / (sizeof(STF_U32) * 2))
#define ST_LCCF_CFG_RDMA_SIZE       (sizeof(ST_RDMA_CMD) + (((sizeof(ST_LCCF_CFG_REG) + 7) / 8) * 8))
#define ST_LCCF_CFG_RDMA_LEN        (ST_LCCF_CFG_RDMA_SIZE / (sizeof(STF_U32) * 2))
#define ST_LCCF_FACTOR_RDMA_SIZE    (sizeof(ST_RDMA_CMD) + (((sizeof(ST_LCCF_FACTOR_REG) + 7) / 8) * 8))
#define ST_LCCF_FACTOR_RDMA_LEN     (ST_LCCF_FACTOR_RDMA_SIZE / (sizeof(STF_U32) * 2))

typedef struct _ST_ISP_LCCF_RDMA {
    STF_U8 u8IspIdx;
    ST_LCCF_RDMA stLccfRdma __attribute__ ((aligned (8)));
} ST_ISP_LCCF_RDMA, *PST_ISP_LCCF_RDMA;
#pragma pack(pop)


///* lens correction cos4 registers interface */
////-----------------------------------------------------------------------------
//extern
//STF_VOID STFHAL_ISP_LCCF_SetReg(
//    STF_U8 u8IspIdx,
//    ST_LCCF_RDMA *pstLccfRdma
//    );
////-----------------------------------------------------------------------------
//extern
//STF_VOID STFHAL_ISP_LCCF_SetCenter(
//    STF_U8 u8IspIdx,
//    STF_U16 u16X,
//    STF_U16 u16Y
//    );
//extern
//STF_VOID STFHAL_ISP_LCCF_SetRadius(
//    STF_U8 u8IspIdx,
//    STF_U8 u8Radius
//    );
//extern
//STF_VOID STFHAL_ISP_LCCF_SetFactor(
//    STF_U8 u8IspIdx,
//    STF_U16 au16F1[],
//    STF_U16 au16F2[]
//    );
////-----------------------------------------------------------------------------
//
//
#endif //__STF_ISP_LCCF_RDMA_H__
