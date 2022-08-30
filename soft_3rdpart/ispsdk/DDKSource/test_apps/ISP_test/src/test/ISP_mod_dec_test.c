/**
  ******************************************************************************
  * @file  ISP_mod_dec_test.c
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

#define LOG_TAG "Mod_Dec_Test"
#include "stf_common/userlog.h"

#include "ISP_mod_dec_test.h"
#include "ISP_test.h"


/* ISPC DEC driver test structure */


/* ISPC DEC driver test variables */
static CI_CONNECTION *g_pstCiConnection = NULL;


/* ISPC DEC driver test interface */
//-----------------------------------------------------------------------------
STF_RESULT Test_DEC_SetReg(
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
    u32Offset = ADDR_REG_DEC_CFG;
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
        STF_U8 *pu8Temp = NULL;
        STF_U32 u32Idx;
        STF_U32 u32Cnt;
        ST_ISP_DEC_RDMA stIspDecRdma = {
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

        stIspDecRdma.stDecRdma.stRdmaStartCmd.Tag = MREG_WR;
        stIspDecRdma.stDecRdma.stRdmaStartCmd.RegAddr =
            ADDR_REG_DEC_CFG;
        stIspDecRdma.stDecRdma.stRdmaStartCmd.u32Param =
            ST_DEC_REG_LEN;
        stIspDecRdma.stDecRdma.stRdmaEndCmd.Tag = DESP_END;

        u32Cnt = 4;
        pu8Temp = (STF_U8 *)&stIspDecRdma.stDecRdma.stDecReg;
        for (u32Idx = 0; u32Idx < u32Cnt; u32Idx++) {
            pu8Temp[u32Idx] = (rand() & 0x1F);
        }
        stIspDecRdma.u8IspIdx = 0;
        ret = STFDRV_ISP_DEC_SetReg(
            pstCiConnection,
            &stIspDecRdma
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_DEC_SetReg()\n",
            stIspDecRdma.u8IspIdx);
        LOG_INFO("hperiod = 0x%02X\n",
            stIspDecRdma.stDecRdma.stDecReg.Cfg.field.hperiod
            );
        LOG_INFO("hkeep = 0x%02X\n",
            stIspDecRdma.stDecRdma.stDecReg.Cfg.field.hkeep
            );
        LOG_INFO("vperiod = 0x%02X\n",
            stIspDecRdma.stDecRdma.stDecReg.Cfg.field.vperiod
            );
        LOG_INFO("vkeep = 0x%02X\n",
            stIspDecRdma.stDecRdma.stDecReg.Cfg.field.vkeep
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_DEC_CFG;
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

#if defined(EXTEND_IOCTL_COMMAND_DEC)
//-----------------------------------------------------------------------------
STF_RESULT Test_DEC_SetDecimation(
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
    u32Offset = ADDR_REG_DEC_CFG;
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
        STF_U8 *pu8Temp = NULL;
        STF_U32 u32Idx;
        STF_U32 u32Cnt;
        ST_DEC_DECIMATION_PARAM stDecDecimation;

        u32Cnt = 4;
        pu8Temp = (STF_U8 *)&stDecDecimation.stDec;
        for (u32Idx = 0; u32Idx < u32Cnt; u32Idx++) {
            pu8Temp[u32Idx] = (rand() & 0x1F);
        }
        stDecDecimation.u8IspIdx = 0;
        ret = STFDRV_ISP_DEC_SetDecimation(
            pstCiConnection,
            &stDecDecimation
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_DEC_SetDecimation()\n",
            stDecDecimation.u8IspIdx);
        LOG_INFO("u8Hperiod = 0x%02X\n",
            stDecDecimation.stDec.u8HPeriod
            );
        LOG_INFO("u8Hkeep = 0x%02X\n",
            stDecDecimation.stDec.u8HKeep
            );
        LOG_INFO("u8Vperiod = 0x%02X\n",
            stDecDecimation.stDec.u8VPeriod
            );
        LOG_INFO("u8Vkeep = 0x%02X\n",
            stDecDecimation.stDec.u8VKeep
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_DEC_CFG;
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

#endif //#if defined(EXTEND_IOCTL_COMMAND_DEC)
//-----------------------------------------------------------------------------
STF_RESULT ISP_Mod_Dec_Test(
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
        ret = Test_DEC_SetReg(
            );
        LOG_INFO("Test DEC_SetReg command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the DEC module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------
    //=========================================================================
#if defined(EXTEND_IOCTL_COMMAND_DEC)

    //=========================================================================
    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_DEC_SetDecimation(
            );
        LOG_INFO("Test DEC_SetDecimation command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the DEC module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------
    //=========================================================================
#endif //#if defined(EXTEND_IOCTL_COMMAND_DEC)

    return ret;
}

//-----------------------------------------------------------------------------
