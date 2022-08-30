/**
  ******************************************************************************
  * @file  ISP_mod_dbc_test.c
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

#define LOG_TAG "Mod_Dbc_Test"
#include "stf_common/userlog.h"

#include "ISP_mod_dbc_test.h"
#include "ISP_test.h"


/* ISPC DBC driver test structure */


/* ISPC DBC driver test variables */
static CI_CONNECTION *g_pstCiConnection = NULL;


/* ISPC DBC driver test interface */
//-----------------------------------------------------------------------------
STF_RESULT Test_DBC_SetReg(
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
    u32Offset = ADDR_REG_IDBC;
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
        STF_U16 *pu16Temp = NULL;
        STF_U32 u32Idx;
        STF_U32 u32Cnt;
        ST_ISP_DBC_RDMA stIspDbcRdma = {
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

        stIspDbcRdma.stDbcRdma.stRdmaStartCmd.Tag = MREG_WR;
        stIspDbcRdma.stDbcRdma.stRdmaStartCmd.RegAddr = ADDR_REG_IDBC;
        stIspDbcRdma.stDbcRdma.stRdmaStartCmd.u32Param = ST_DBC_REG_LEN;
        stIspDbcRdma.stDbcRdma.stRdmaEndCmd.Tag = DESP_END;

        u32Cnt = 2;
        pu16Temp = (STF_U16 *)&stIspDbcRdma.stDbcRdma.stDbcReg;
        for (u32Idx = 0; u32Idx < u32Cnt; u32Idx++) {
            pu16Temp[u32Idx] = (rand() & 0x03FF);
        }
        stIspDbcRdma.u8IspIdx = 0;
        ret = STFDRV_ISP_DBC_SetReg(
            pstCiConnection,
            &stIspDbcRdma
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_DBC_SetReg()\n", stIspDbcRdma.u8IspIdx);
        LOG_INFO("badxt = 0x%04X\n",
            stIspDbcRdma.stDbcRdma.stDbcReg.Threshold.field.badxt
            );
        LOG_INFO("badgt = 0x%04X\n",
            stIspDbcRdma.stDbcRdma.stDbcReg.Threshold.field.badgt
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_IDBC;
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

#if defined(EXTEND_IOCTL_COMMAND_DBC)
//-----------------------------------------------------------------------------
STF_RESULT Test_DBC_SetBadPixel(
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
    u32Offset = ADDR_REG_IDBC;
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
        STF_U16 *pu16Temp = NULL;
        STF_U32 u32Idx;
        STF_U32 u32Cnt;
        ST_ISP_DBC_BAD_PXL_THRESHOLD_PARAM stIspDbcBadPixelThreshold;

        u32Cnt = 2;
        pu16Temp =
            (STF_U16 *)&stIspDbcBadPixelThreshold.stDbcBadPixelThreshold.u16Xt;
        for (u32Idx = 0; u32Idx < u32Cnt; u32Idx++) {
            pu16Temp[u32Idx] = (rand() & 0x03FF);
        }
        stIspDbcBadPixelThreshold.u8IspIdx = 0;
        ret = STFDRV_ISP_DBC_SetBadPixel(
            pstCiConnection,
            &stIspDbcBadPixelThreshold
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_DBC_SetBadPixel()\n",
            stIspDbcBadPixelThreshold.u8IspIdx);
        LOG_INFO("u16Xt = 0x%04X\n",
            stIspDbcBadPixelThreshold.stDbcBadPixelThreshold.u16Xt
            );
        LOG_INFO("u16Gt = 0x%04X\n",
            stIspDbcBadPixelThreshold.stDbcBadPixelThreshold.u16Gt
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_IDBC;
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

#endif //#if defined(EXTEND_IOCTL_COMMAND_DBC)
//-----------------------------------------------------------------------------
STF_RESULT ISP_Mod_Dbc_Test(
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
        ret = Test_DBC_SetReg(
            );
        LOG_INFO("Test DBC_SetReg command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the DBC module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------
    //=========================================================================
#if defined(EXTEND_IOCTL_COMMAND_DBC)

    //=========================================================================
    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_DBC_SetBadPixel(
            );
        LOG_INFO("Test DBC_SetBadPixel command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the DBC module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------
    //=========================================================================
#endif //#if defined(EXTEND_IOCTL_COMMAND_DBC)

    return ret;
}

//-----------------------------------------------------------------------------
