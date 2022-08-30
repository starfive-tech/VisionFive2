/**
  ******************************************************************************
  * @file  stf_isp_rdma.h
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
#ifndef __STF_ISP_RDMA_H__
#define __STF_ISP_RDMA_H__


/* add RDMA registers structure */
typedef enum _ST_RDMA_TAG {
    SREG_WR = 0xA0,                 // Single register write.
    MREG_WR = 0xA1,                 // Multiple registers write.
    DESP_LNK = 0xA2,                // Link descriptor.
    DESP_SINT = 0xA3,               // Soft interrupt & blank.
    DESP_END = 0xAF,                // End tag.
} EN_ST_RDMA_TAG;


typedef struct _ST_RDMA_CMD {
    STF_U32 u32Param;               // register value or length.
    union {
        STF_U32 u32Value;
        struct
        {
            STF_U32 RegAddr : 24;   // register address offset.
            STF_U32 Tag     : 8;    // RDMA command.
        }/* Cmd*/;
    }/* u32Cmd*/;
} ST_RDMA_CMD, *PST_RDMA_CMD;

typedef struct _ST_RDMA_REG {
    REG_RDMA_ENR rdma_enr;          // 0x0C00, RDMA enable register. */
    REG_RDMA_SACKR rdma_sackr;      // 0x0C04, RDMA Software sint ack register. */
    REG_RDMA_DADDR rdma_daddr;      // 0x0C08, RDMA Descriptor Start Address register. */
    REG_RDMA_CADDR rdma_caddr;      // 0x0C0C, RDMA Descriptor Current Address register (Read-only). */
    REG_RDMA_STSR rdma_stsr;        // 0x0C10, RDMA Status Register (Read-only). */
} ST_RDMA_REG, *PST_RDMA_REG;

#define ST_RDMA_REG_SIZE        (sizeof(ST_RDMA_REG))
#define ST_RDMA_REG_LEN         (ST_RDMA_REG_SIZE / sizeof(STF_U32))


///* RDMA registers interface */
////-----------------------------------------------------------------------------
//extern STF_S32 STFHAL_ISP_RDMA_Enable(STF_BOOL8 bEnable);
//extern STF_S32 STFHAL_ISP_RDMA_Trigger(STF_VOID);
//extern STF_S32 STFHAL_ISP_RDMA_SetSoftIntAck(STF_VOID);
//extern STF_S32 STFHAL_ISP_RDMA_SetStartAddress(STF_U32 u32StartAddress);
//extern STF_S32 STFHAL_ISP_RDMA_SetStartAddressAndTrigger(STF_U32 u32StartAddress);
//extern STF_S32 STFHAL_ISP_RDMA_GetCurrectAddress(STF_U32 *pu32Address);
//extern STF_S32 STFHAL_ISP_RDMA_GetStatus(REG_RDMA_STSR *pstRdmaStatus);
////-----------------------------------------------------------------------------
//
//
#endif //__STF_ISP_RDMA_H__
