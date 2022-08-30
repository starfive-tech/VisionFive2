/**
  ******************************************************************************
  * @file  ISP_mod_cfa_ahd_test.c
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

#define LOG_TAG "Mod_Cfa_Test"
#include "stf_common/userlog.h"

#include "ISP_mod_cfa_ahd_test.h"
#include "ISP_test.h"


/* ISPC CFA driver test structure */


/* ISPC CFA driver test variables */
static CI_CONNECTION *g_pstCiConnection = NULL;


/* ISPC CFA driver test interface */
//-----------------------------------------------------------------------------
STF_RESULT Test_CFA_SetReg(
    STF_VOID
    )
{
    int i;
    //STF_U8 u8Method;
    STF_U32 u32Offset;
    //STF_U32 u32Length;
    STF_U32 u32Buffer[1];
    CI_CONNECTION *pstCiConnection = NULL;
    STF_RESULT ret = STF_SUCCESS;

    pstCiConnection = g_pstCiConnection;

    //=========================================================================
    u32Offset = ADDR_REG_ICFAM;
    ret = STFLIB_ISP_RegReadOne(
        TEST_ISP_DEVICE,
        u32Offset,
        (STF_U32 *)&u32Buffer[0]
        );
    if (ret) {
        return ret;
    }
    LOG_INFO("Reg[0x%08X] = 0x%08X\n", u32Offset, u32Buffer[0]);
    LOG_INFO("---------------------------------\n");
    //-------------------------------------------------------------------------
    {
        ST_ISP_CFA_RDMA stIspCfaRdma = {
            0,
            {
#if defined(SUPPORT_RDMA_FEATURE)
                { 0x00000000, 0x00000000 },
#endif //SUPPORT_RDMA_FEATURE
                {
                    0x00000000,
                },
#if defined(SUPPORT_RDMA_FEATURE)
                { 0x00000000, 0x00000000 },
#endif //SUPPORT_RDMA_FEATURE
            }
        };

        stIspCfaRdma.stCfaRdma.stRdmaStartCmd.Tag = MREG_WR;
        stIspCfaRdma.stCfaRdma.stRdmaStartCmd.RegAddr = ADDR_REG_ICFAM;
        stIspCfaRdma.stCfaRdma.stRdmaStartCmd.u32Param = ST_CFA_REG_LEN;
        stIspCfaRdma.stCfaRdma.stRdmaEndCmd.Tag = DESP_END;

        stIspCfaRdma.stCfaRdma.stCfaReg.Mode.field.hv_width = (rand() & 0x0F);
        stIspCfaRdma.stCfaRdma.stCfaReg.Mode.field.cross_cov = (rand() & 0x03);
        stIspCfaRdma.u8IspIdx = 0;
        ret = STFDRV_ISP_CFA_SetReg(
            pstCiConnection,
            &stIspCfaRdma
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_CFA_SetReg()\n", stIspCfaRdma.u8IspIdx);
        LOG_INFO("hv_width = 0x%01X\n",
            stIspCfaRdma.stCfaRdma.stCfaReg.Mode.field.hv_width
            );
        LOG_INFO("cross_cov = 0x%01X\n",
            stIspCfaRdma.stCfaRdma.stCfaReg.Mode.field.cross_cov
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_ICFAM;
    ret = STFLIB_ISP_RegReadOne(
        TEST_ISP_DEVICE,
        u32Offset,
        (STF_U32 *)&u32Buffer[0]
        );
    if (ret) {
        return ret;
    }
    LOG_INFO("Reg[0x%08X] = 0x%08X\n", u32Offset, u32Buffer[0]);
    LOG_INFO("---------------------------------\n");
    //=========================================================================

    return ret;
}

#if defined(EXTEND_IOCTL_COMMAND_CFA)
//-----------------------------------------------------------------------------
STF_RESULT Test_CFA_SetMode(
    STF_VOID
    )
{
    //int i;
    //STF_U8 u8Method;
    STF_U32 u32Offset;
    //STF_U32 u32Length;
    STF_U32 u32Buffer[1];
    CI_CONNECTION *pstCiConnection = NULL;
    STF_RESULT ret = STF_SUCCESS;

    pstCiConnection = g_pstCiConnection;

    //=========================================================================
    u32Offset = ADDR_REG_ICFAM;
    ret = STFLIB_ISP_RegReadOne(
        TEST_ISP_DEVICE,
        u32Offset,
        (STF_U32 *)&u32Buffer[0]
        );
    if (ret) {
        return ret;
    }
    LOG_INFO("Reg[0x%08X] = 0x%08X\n", u32Offset, u32Buffer[0]);
    LOG_INFO("---------------------------------\n");
    //-------------------------------------------------------------------------
    {
        ST_ISP_CFA_MODE_PARAM stIspCfaMode;

        stIspCfaMode.stCfaMode.u8HvWidth = (rand() & 0x0F);
        stIspCfaMode.stCfaMode.u8CrossCovariance = (rand() & 0x03);
        stIspCfaMode.u8IspIdx = 0;
        ret = STFDRV_ISP_CFA_SetMode(
            pstCiConnection,
            &stIspCfaMode
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_CFA_SetMode()\n", stIspCfaMode.u8IspIdx);
        LOG_INFO("u8HvWidth = 0x%01X\n",
            stIspCfaMode.stCfaMode.u8HvWidth
            );
        LOG_INFO("u8CrossCovariance = 0x%01X\n",
            stIspCfaMode.stCfaMode.u8CrossCovariance
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_ICFAM;
    ret = STFLIB_ISP_RegReadOne(
        TEST_ISP_DEVICE,
        u32Offset,
        (STF_U32 *)&u32Buffer[0]
        );
    if (ret) {
        return ret;
    }
    LOG_INFO("Reg[0x%08X] = 0x%08X\n", u32Offset, u32Buffer[0]);
    LOG_INFO("---------------------------------\n");
   //=========================================================================

    return ret;
}

#endif //#if defined(EXTEND_IOCTL_COMMAND_CFA)
//-----------------------------------------------------------------------------
STF_RESULT ISP_Mod_Cfa_Test(
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
        ret = Test_CFA_SetReg(
            );
        LOG_INFO("Test CFA_SetReg command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the CFA module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------
    //=========================================================================
#if defined(EXTEND_IOCTL_COMMAND_CFA)

    //=========================================================================
    //-------------------------------------------------------------------------
    {
        ret = Test_CFA_SetMode(
            );
        LOG_INFO("Test CFA_SetMode command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the CFA module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------
    //=========================================================================
#endif //#if defined(EXTEND_IOCTL_COMMAND_CFA)

    return ret;
}

//-----------------------------------------------------------------------------
