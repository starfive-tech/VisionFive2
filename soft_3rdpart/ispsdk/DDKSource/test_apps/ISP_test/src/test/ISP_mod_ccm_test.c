/**
  ******************************************************************************
  * @file  ISP_mod_ccm_test.c
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

#include "ci/ci_api_structs.h"
#include "ci/ci_api.h"

#include "ISPC/stflib_isp_base.h"

#define LOG_TAG "Mod_Ccm_Test"
#include "stf_common/userlog.h"

#include "ISP_mod_ccm_test.h"
#include "ISP_test.h"


/* ISPC CCM driver test structure */


/* ISPC CCM driver test variables */
static CI_CONNECTION *g_pstCiConnection = NULL;


/* ISPC CCM driver test interface */
//-----------------------------------------------------------------------------
STF_RESULT Test_CCM_SetReg(
    STF_VOID
    )
{
    int i;
    //STF_U8 u8Method;
    //STF_U32 u32Offset;
    STF_U32 u32Length;
    STF_U32 u32Buffer[26 * 2];
    CI_CONNECTION *pstCiConnection = NULL;
    STF_RESULT ret = STF_SUCCESS;

    pstCiConnection = g_pstCiConnection;

    //=========================================================================
    u32Length = 26;
    for (i = 0; i < u32Length; i++) {
        u32Buffer[0 + i * 2] = ADDR_REG_ICAMD_0 + (i * 4);
        u32Buffer[0 + i * 2 + 1] = 0;
    }
    ret = STFLIB_ISP_RegReadByTable(
        TEST_ISP_DEVICE,
        u32Length,
        (ST_REG_TBL *)&u32Buffer[0]
        );
    if (ret) {
        return ret;
    }
    for (i = 0; i < u32Length; i++) {
        LOG_INFO("Reg[0x%08X] = 0x%08X\n",
            u32Buffer[i * 2], u32Buffer[i * 2 + 1]);
    }
    LOG_INFO("---------------------------------\n");
    //-------------------------------------------------------------------------
    {
        STF_U32 *pu32Temp;
        STF_U32 u32Idx;
        STF_U32 u32Cnt;
        ST_ISP_CCM_RDMA stIspCcmRdma;

        stIspCcmRdma.stCcmRdma.stRdmaStartCmd.Tag = MREG_WR;
        stIspCcmRdma.stCcmRdma.stRdmaStartCmd.RegAddr = ADDR_REG_ICAMD_0;
        stIspCcmRdma.stCcmRdma.stRdmaStartCmd.u32Param = ST_CCM_REG_LEN;
        stIspCcmRdma.stCcmRdma.stRdmaEndCmd.Tag = DESP_END;

        u32Cnt = 26;
        pu32Temp = (STF_U32 *)&stIspCcmRdma.stCcmRdma.stCcmReg;
        for (u32Idx = 0; u32Idx < u32Cnt; u32Idx++) {
            if (0 == u32Idx) {
                pu32Temp[u32Idx] =
                    ((rand() & 0x07FF) | ((rand() & 0x0F) << 16));
            } else {
                pu32Temp[u32Idx] = (rand() & 0x07FF);
            }
        }
        stIspCcmRdma.u8IspIdx = 0;
        ret = STFDRV_ISP_CCM_SetReg(
            pstCiConnection,
            &stIspCcmRdma
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_CCM_SetReg()\n", stIspCcmRdma.u8IspIdx);
        LOG_INFO("dnrm = 0x%01X\n",
            stIspCcmRdma.stCcmRdma.stCcmReg.MatrixSlope0_Dnrm.field.dnrm
            );
        pu32Temp =
            (STF_U32 *)&stIspCcmRdma.stCcmRdma.stCcmReg.MatrixSlope0_Dnrm;
        u32Cnt = 9;
        for (u32Idx = 0; u32Idx < u32Cnt; u32Idx++) {
            if (0 == u32Idx) {
                LOG_INFO("MatrixSlope[%d] = 0x%04X\n",
                    u32Idx,
                    (pu32Temp[u32Idx] & 0xFFFF)
                    );
            } else {
                LOG_INFO("MatrixSlope[%d] = 0x%04X\n",
                    u32Idx,
                    pu32Temp[u32Idx]
                    );
            }
        }
        pu32Temp = (STF_U32 *)&stIspCcmRdma.stCcmRdma.stCcmReg.OffsetSlope0;
        u32Cnt = 3;
        for (u32Idx = 0; u32Idx < u32Cnt; u32Idx++) {
            LOG_INFO("OffsetSlope[%d] = 0x%04X\n",
                u32Idx,
                pu32Temp[u32Idx]
                );
        }

        pu32Temp = (STF_U32 *)&stIspCcmRdma.stCcmRdma.stCcmReg.SmLowMatrix0;
        u32Cnt = 9;
        for (u32Idx = 0; u32Idx < u32Cnt; u32Idx++) {
            LOG_INFO("SmLowMatrix[%d] = 0x%04X\n",
                u32Idx,
                pu32Temp[u32Idx]
                );
        }
        pu32Temp = (STF_U32 *)&stIspCcmRdma.stCcmRdma.stCcmReg.SmLowOffset0;
        u32Cnt = 3;
        for (u32Idx = 0; u32Idx < u32Cnt; u32Idx++) {
            LOG_INFO("SmLowOffset[%d] = 0x%04X\n",
                u32Idx,
                pu32Temp[u32Idx]
                );
        }
        LOG_INFO("Distance = 0x%04X\n",
            stIspCcmRdma.stCcmRdma.stCcmReg.Distance.value
            );
        LOG_INFO("SmLow = 0x%04X\n",
            stIspCcmRdma.stCcmRdma.stCcmReg.SmLow.value
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Length = 26;
    for (i = 0; i < u32Length; i++) {
        u32Buffer[0 + i * 2] = ADDR_REG_ICAMD_0 + (i * 4);
        u32Buffer[0 + i * 2 + 1] = 0;
    }
    ret = STFLIB_ISP_RegReadByTable(
        TEST_ISP_DEVICE,
        u32Length,
        (ST_REG_TBL *)&u32Buffer[0]
        );
    if (ret) {
        return ret;
    }
    for (i = 0; i < u32Length; i++) {
        LOG_INFO("Reg[0x%08X] = 0x%08X\n",
            u32Buffer[i * 2], u32Buffer[i * 2 + 1]);
    }
    LOG_INFO("---------------------------------\n");
    //=========================================================================

    return ret;
}

#if defined(EXTEND_IOCTL_COMMAND_CCM)
//-----------------------------------------------------------------------------

#endif //#if defined(EXTEND_IOCTL_COMMAND_CCM)
//-----------------------------------------------------------------------------
STF_RESULT ISP_Mod_Ccm_Test(
    STF_VOID
    )
{
    STF_RESULT ret = STF_SUCCESS;

    g_pstCiConnection = (CI_CONNECTION *)STFLIB_ISP_BASE_GetConnection(
        TEST_ISP_DEVICE
        );

    //=========================================================================
    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_CCM_SetReg(
            );
        LOG_INFO("Test CCM_SetReg command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the CCM module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------
    //=========================================================================
#if defined(EXTEND_IOCTL_COMMAND_CCM)

    //=========================================================================
    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------
    //=========================================================================
#endif //#if defined(EXTEND_IOCTL_COMMAND_CCM)

    return ret;
}

//-----------------------------------------------------------------------------
