/**
  ******************************************************************************
  * @file  ISP_mod_ctc_test.c
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

#define LOG_TAG "Mod_Ctc_Test"
#include "stf_common/userlog.h"

#include "ISP_mod_ctc_test.h"
#include "ISP_test.h"


/* ISPC CTC driver test structure */


/* ISPC CTC driver test variables */
static CI_CONNECTION *g_pstCiConnection = NULL;


/* ISPC CTC driver test interface */
//-----------------------------------------------------------------------------
STF_RESULT Test_CTC_SetReg(
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
    u32Offset = ADDR_REG_ICTC;
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
        ST_ISP_CTC_RDMA stIspCtcRdma = {
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

        stIspCtcRdma.stCtcRdma.stRdmaStartCmd.Tag = MREG_WR;
        stIspCtcRdma.stCtcRdma.stRdmaStartCmd.RegAddr = ADDR_REG_ICTC;
        stIspCtcRdma.stCtcRdma.stRdmaStartCmd.u32Param = ST_CTC_REG_LEN;
        stIspCtcRdma.stCtcRdma.stRdmaEndCmd.Tag = DESP_END;

        stIspCtcRdma.stCtcRdma.stCtcReg.Mode_Threshold.field.mingt =
            (rand() & 0x03FF);
        stIspCtcRdma.stCtcRdma.stCtcReg.Mode_Threshold.field.maxgt =
            (rand() & 0x03FF);
        stIspCtcRdma.stCtcRdma.stCtcReg.Mode_Threshold.field.gf_mode =
            (rand() & 0x03);
        stIspCtcRdma.u8IspIdx = 0;
        ret = STFDRV_ISP_CTC_SetReg(
            pstCiConnection,
            &stIspCtcRdma
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_CTC_SetReg()\n", stIspCtcRdma.u8IspIdx);
        LOG_INFO("mingt = 0x%04X\n",
            stIspCtcRdma.stCtcRdma.stCtcReg.Mode_Threshold.field.mingt
            );
        LOG_INFO("maxgt = 0x%04X\n",
            stIspCtcRdma.stCtcRdma.stCtcReg.Mode_Threshold.field.maxgt
            );
        LOG_INFO("gf_mode = 0x%01X\n",
            stIspCtcRdma.stCtcRdma.stCtcReg.Mode_Threshold.field.gf_mode
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_ICTC;
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

#if defined(EXTEND_IOCTL_COMMAND_CTC)
//-----------------------------------------------------------------------------
STF_RESULT Test_CTC_SetMode(
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
    u32Offset = ADDR_REG_ICTC;
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
        EN_CTC_MODE enCtcMode = EN_CTC_MODE_5X5_NEIGHBORHOOD;
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        ret = STFDRV_ISP_CTC_SetMode(
            pstCiConnection,
            u8IspIdx,
            enCtcMode
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_CTC_SetMode()\n", u8IspIdx);
        LOG_INFO("enCtcMode = 0x%01X\n",
            enCtcMode
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_ICTC;
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
        EN_CTC_MODE enCtcMode = EN_CTC_MODE_DETAIL_AVERAGE_N_SMOOTH_CENTER;
        STF_U8 u8IspIdx = 0;

        u8IspIdx = 0;
        ret = STFDRV_ISP_CTC_SetMode(
            pstCiConnection,
            u8IspIdx,
            enCtcMode
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_CTC_SetMode()\n", u8IspIdx);
        LOG_INFO("enCtcMode = 0x%01X\n",
            enCtcMode
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_ICTC;
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

STF_RESULT Test_CTC_SetThreshold(
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
    u32Offset = ADDR_REG_ICTC;
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
        ST_ISP_CTC_THRESHOLD_PARAM stIspCtcThreshold;

        u32Cnt = 2;
        pu16Temp = (STF_U16 *)&stIspCtcThreshold.stCtcThreshold.u16Min;
        for (u32Idx = 0; u32Idx < u32Cnt; u32Idx++) {
            pu16Temp[u32Idx] = (rand() & 0x03FF);
        }
        stIspCtcThreshold.u8IspIdx = 0;
        ret = STFDRV_ISP_CTC_SetThreshold(
            pstCiConnection,
            &stIspCtcThreshold
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_CTC_SetThreshold()\n",
            stIspCtcThreshold.u8IspIdx);
        LOG_INFO("u16Min = 0x%04X\n",
            stIspCtcThreshold.stCtcThreshold.u16Min
            );
        LOG_INFO("u16Max = 0x%04X\n",
            stIspCtcThreshold.stCtcThreshold.u16Max
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_ICTC;
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

#endif //#if defined(EXTEND_IOCTL_COMMAND_CTC)
//-----------------------------------------------------------------------------
STF_RESULT ISP_Mod_Ctc_Test(
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
        ret = Test_CTC_SetReg(
            );
        LOG_INFO("Test CTC_SetReg command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the CTC module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------
    //=========================================================================
#if defined(EXTEND_IOCTL_COMMAND_CTC)

    //=========================================================================
    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_CTC_SetMode(
            );
        LOG_INFO("Test CTC_SetMode command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the CTC module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_CTC_SetThreshold(
            );
        LOG_INFO("Test CTC_SetThreshold command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the CTC module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------
    //=========================================================================
#endif //#if defined(EXTEND_IOCTL_COMMAND_CTC)

    return ret;
}

//-----------------------------------------------------------------------------
