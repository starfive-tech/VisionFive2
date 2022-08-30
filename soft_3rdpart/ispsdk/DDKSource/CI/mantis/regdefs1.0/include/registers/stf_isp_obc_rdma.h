/**
  ******************************************************************************
  * @file  stf_isp_obc_rdma.h
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
#ifndef __STF_ISP_OBC_RDMA_H__
#define __STF_ISP_OBC_RDMA_H__


#include "registers/stf_isp_rdma.h"


/* add optical black compensation registers structure */
typedef struct _ST_OBC_CFG_REG {
    REG_OBC_CFG Region;             // 0x0034, OBC(Optical Black Compensation) window height(2^HEIGHT)/OBC window width(2^WIDTH),
} ST_OBC_CFG_REG, *PST_OBC_CFG_REG;

#define ST_OBC_CFG_REG_SIZE     (sizeof(ST_OBC_CFG_REG))
#define ST_OBC_CFG_REG_LEN      (ST_OBC_CFG_REG_SIZE / sizeof(STF_U32))

typedef struct _ST_OBC_GAIN_REG {
    REG_OBCG_CFG_0 RGain;           // 0x02E0, each symbol has four points, each point has one 8bit [0.2.6] gain register.
    REG_OBCG_CFG_1 GrGain;          // 0x02E4, each symbol has four points, each point has one 8bit [0.2.6] gain register.
    REG_OBCG_CFG_2 GbGain;          // 0x02E8, each symbol has four points, each point has one 8bit [0.2.6] gain register.
    REG_OBCG_CFG_3 BGain;           // 0x02EC, each symbol has four points, each point has one 8bit [0.2.6] gain register.
} ST_OBC_GAIN_REG, *PST_OBC_GAIN_REG;
#define ST_OBC_GAIN_REG_SIZE    (sizeof(ST_OBC_GAIN_REG))
#define ST_OBC_GAIN_REG_LEN     (ST_OBC_GAIN_REG_SIZE / sizeof(STF_U32))

typedef struct _ST_OBC_OFST_REG {
    REG_OBCO_CFG_0 ROffset;         // 0x02F0, each symbol has four points, each point has one 8bit offset register.
    REG_OBCO_CFG_1 GrOffset;        // 0x02F4, each symbol has four points, each point has one 8bit offset register.
    REG_OBCO_CFG_2 GbOffset;        // 0x02F8, each symbol has four points, each point has one 8bit offset register.
    REG_OBCO_CFG_3 BOffset;         // 0x02FC, each symbol has four points, each point has one 8bit offset register.
} ST_OBC_OFST_REG, *PST_OBC_OFST_REG;
#define ST_OBC_OFST_REG_SIZE    (sizeof(ST_OBC_OFST_REG))
#define ST_OBC_OFST_REG_LEN     (ST_OBC_OFST_REG_SIZE / sizeof(STF_U32))

typedef struct _ST_OBC_REG {
    REG_OBC_CFG Region;             // 0x0034, OBC(Optical Black Compensation) window height(2^HEIGHT)/OBC window width(2^WIDTH),
    //STF_U32 u32Reserved[?];         // Added reserved variables to let ST_OBC_REG address is contiguous.
    //-------------------------------------------------------------------------
    ST_OBC_GAIN_REG stGain;         // 0x02E0 - 0x02EF, each symbol has four point, each point has one 8bit [0.2.6] gain register.
    ST_OBC_OFST_REG stOffset;       // 0x02F0 - 0x02FF, each symbol has four points, each point has one 8bit offset register.
} ST_OBC_REG, *PST_OBC_REG;

#define ST_OBC_REG_SIZE         (sizeof(ST_OBC_REG))
#define ST_OBC_REG_LEN          (ST_OBC_REG_SIZE / sizeof(STF_U32))

#pragma pack(push, 8)
typedef struct _ST_OBC_RDMA {
#if defined(SUPPORT_RDMA_FEATURE)
    ST_RDMA_CMD stRdmaStartCmd __attribute__ ((aligned (8)));
#endif //SUPPORT_RDMA_FEATURE
    ST_OBC_CFG_REG stObcCfgReg __attribute__ ((aligned (8)));
#if defined(SUPPORT_RDMA_FEATURE)
    ST_RDMA_CMD stRdmaGainCmd __attribute__ ((aligned (8)));
#endif //SUPPORT_RDMA_FEATURE
    ST_OBC_GAIN_REG stObcGainReg __attribute__ ((aligned (8)));
#if defined(SUPPORT_RDMA_FEATURE)
    ST_RDMA_CMD stRdmaOfstCmd __attribute__ ((aligned (8)));
#endif //SUPPORT_RDMA_FEATURE
    ST_OBC_OFST_REG stObcOfstReg __attribute__ ((aligned (8)));
#if defined(SUPPORT_RDMA_FEATURE)
    ST_RDMA_CMD stRdmaEndCmd __attribute__ ((aligned (8)));
#endif //SUPPORT_RDMA_FEATURE
} ST_OBC_RDMA, *PST_OBC_RDMA;

#define ST_OBC_RDMA_SIZE        (sizeof(ST_OBC_RDMA))
#define ST_OBC_RDMA_LEN         (ST_OBC_RDMA_SIZE / (sizeof(STF_U32) * 2))
#define ST_OBC_CFG_RDMA_SIZE    (sizeof(ST_RDMA_CMD) + (((sizeof(ST_OBC_CFG_REG) + 7) / 8) * 8))
#define ST_OBC_CFG_RDMA_LEN     (ST_OBC_CFG_RDMA_SIZE / (sizeof(STF_U32) * 2))
#define ST_OBC_GAIN_RDMA_SIZE   (sizeof(ST_RDMA_CMD) + (((sizeof(ST_OBC_GAIN_REG) + 7) / 8) * 8))
#define ST_OBC_GAIN_RDMA_LEN    (ST_OBC_GAIN_RDMA_SIZE / (sizeof(STF_U32) * 2))
#define ST_OBC_OFST_RDMA_SIZE   (sizeof(ST_RDMA_CMD) + (((sizeof(ST_OBC_OFST_REG) + 7) / 8) * 8))
#define ST_OBC_OFST_RDMA_LEN    (ST_OBC_OFST_RDMA_SIZE / (sizeof(STF_U32) * 2))

typedef struct _ST_ISP_OBC_RDMA {
    STF_U8 u8IspIdx;
    ST_OBC_RDMA stObcRdma __attribute__ ((aligned (8)));
} ST_ISP_OBC_RDMA, *PST_ISP_OBC_RDMA;
#pragma pack(pop)


///* optical black compensation registers interface */
////-----------------------------------------------------------------------------
//extern
//STF_VOID STFHAL_ISP_OBC_SetReg(
//    STF_U8 u8IspIdx,
//    ST_OBC_RDMA *pstObcRdma
//    );
////-----------------------------------------------------------------------------
//extern
//STF_VOID STFHAL_ISP_OBC_SetRegion(
//    STF_U8 u8IspIdx,
//    ST_OBC_RDMA *pstObcRdma
//    );
//extern
//STF_VOID STFHAL_ISP_OBC_SetGain(
//    STF_U8 u8IspIdx,
//    ST_OBC_RDMA *pstObcRdma
//    );
//extern
//STF_VOID STFHAL_ISP_OBC_SetOffset(
//    STF_U8 u8IspIdx,
//    ST_OBC_RDMA *pstObcRdma
//    );
////-----------------------------------------------------------------------------
//
//
#endif //__STF_ISP_OBC_RDMA_H__
