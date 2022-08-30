/**
  ******************************************************************************
  * @file  ISP_mod_csi_in_test.c
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

#include "ci/ci_api_structs.h"
#include "ci/ci_api.h"

#include "ISPC/stflib_isp_base.h"

#define LOG_TAG "Mod_Csi_In_Test"
#include "stf_common/userlog.h"

#include "ISP_mod_csi_in_test.h"
#include "ISP_test.h"


/* ISPC CSI_IN driver test structure */


/* ISPC CSI_IN driver test variables */
static CI_CONNECTION *g_pstCiConnection = NULL;


/* ISPC CSI_IN driver test interface */
//-----------------------------------------------------------------------------
STF_RESULT Test_CSI_IN_SetReg(
    STF_VOID
    )
{
    int i;
    //STF_U8 u8Method;
    STF_U32 u32Offset;
    STF_U32 u32Length;
    STF_U32 u32Buffer[2 * 2];
    CI_CONNECTION *pstCiConnection = NULL;
    STF_RESULT ret = STF_SUCCESS;

    pstCiConnection = g_pstCiConnection;

    //=========================================================================
    u32Length = 2;
    u32Buffer[0 * 2] = ADDR_REG_SENSOR;
    u32Buffer[0 * 2 + 1] = 0;
    u32Buffer[1 * 2] = ADDR_REG_CFA;
    u32Buffer[1 * 2 + 1] = 0;
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
        const STF_CHAR szDvpSyncPol[][40] = {
            "Hsync negative, Vsync negative",
            "Hsync negative, Vsync positive",
            "Hsync positive, Vsync negative",
            "Hsync positive, Vsync positive",
        };
        const STF_CHAR szFirstPixel[][4] = {
            "R",
            "Gr",
            "Gb",
            "B",
        };
        const STF_U32 u32FirstPixel[4] = {
            0x0000EE44,
            0x0000BB11,
            0x000044EE,
            0x000011BB,
        };
        ST_ISP_CSI_IN_RDMA stIspCsiInRdma = {
            9,
            {
#if defined(SUPPORT_RDMA_FEATURE)
                { 0x00000000, 0x00000000 },
#endif //SUPPORT_RDMA_FEATURE
                0x00000008,
                0x000011BB,
#if defined(SUPPORT_RDMA_FEATURE)
                { 0x00000000, 0x00000000 },
#endif //SUPPORT_RDMA_FEATURE
            }
        };
        STF_U8 u8Temp;

        stIspCsiInRdma.stCsiInRdma.stRdmaStartCmd.Tag = MREG_WR;
        stIspCsiInRdma.stCsiInRdma.stRdmaStartCmd.RegAddr =
            ADDR_REG_SENSOR;
        stIspCsiInRdma.stCsiInRdma.stRdmaStartCmd.u32Param =
            ST_CSI_IN_REG_LEN;
        stIspCsiInRdma.stCsiInRdma.stRdmaEndCmd.Tag = DESP_END;

        stIspCsiInRdma.stCsiInRdma.stCsiInReg.Sensor.field.imager_sel =
            (rand() & 0x01);
        stIspCsiInRdma.stCsiInRdma.stCsiInReg.Sensor.field.itu656_en =
            (rand() & 0x01);
        stIspCsiInRdma.stCsiInRdma.stCsiInReg.Sensor.field.dvp_sync_pol =
            (rand() & 0x03);
        u8Temp = (rand() & 0x03);
        stIspCsiInRdma.stCsiInRdma.stCsiInReg.FmtCfg.value =
            u32FirstPixel[u8Temp];
        stIspCsiInRdma.u8IspIdx = 0;
        ret = STFDRV_ISP_CSI_IN_SetReg(
            pstCiConnection,
            &stIspCsiInRdma
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_CSI_IN_SetReg()\n",
            stIspCsiInRdma.u8IspIdx);
        LOG_INFO("Imager_Sel = %s\n",
            (stIspCsiInRdma.stCsiInRdma.stCsiInReg.Sensor.field.imager_sel)
                ? ("MIPI") : ("DVP")
            );
        LOG_INFO("ITU_656_En = %s\n",
            (stIspCsiInRdma.stCsiInRdma.stCsiInReg.Sensor.field.itu656_en)
                ? ("Enable") : ("Disable")
            );
        LOG_INFO("DVP_Sync_Pol = %d(%s)\n",
            stIspCsiInRdma.stCsiInRdma.stCsiInReg.Sensor.field.dvp_sync_pol,
            szDvpSyncPol[stIspCsiInRdma.stCsiInRdma.stCsiInReg.Sensor.field.dvp_sync_pol]
            );
        LOG_INFO("CSI First Pixel = 0x%08X([%d]%s)\n",
            stIspCsiInRdma.stCsiInRdma.stCsiInReg.FmtCfg.value,
            u8Temp,
            szFirstPixel[u8Temp]
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Length = 2;
    u32Buffer[0 * 2] = ADDR_REG_SENSOR;
    u32Buffer[0 * 2 + 1] = 0;
    u32Buffer[1 * 2] = ADDR_REG_CFA;
    u32Buffer[1 * 2 + 1] = 0;
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
        const STF_CHAR szDvpSyncPol[][40] = {
            "Hsync negative, Vsync negative",
            "Hsync negative, Vsync positive",
            "Hsync positive, Vsync negative",
            "Hsync positive, Vsync positive",
        };
        const STF_CHAR szFirstPixel[][4] = {
            "R",
            "Gr",
            "Gb",
            "B",
        };
        const STF_U32 u32FirstPixel[4] = {
            0x0000EE44,
            0x0000BB11,
            0x000044EE,
            0x000011BB,
        };
        ST_ISP_CSI_IN_RDMA stIspCsiInRdma = {
            0,
            {
#if defined(SUPPORT_RDMA_FEATURE)
                { 0x00000000, 0x00000000 },
#endif //SUPPORT_RDMA_FEATURE
                0x00000008,
                0x000011BB,
#if defined(SUPPORT_RDMA_FEATURE)
                { 0x00000000, 0x00000000 },
#endif //SUPPORT_RDMA_FEATURE
            }
        };
        STF_U8 u8Temp;

        stIspCsiInRdma.stCsiInRdma.stRdmaStartCmd.Tag = MREG_WR;
        stIspCsiInRdma.stCsiInRdma.stRdmaStartCmd.RegAddr =
            ADDR_REG_SENSOR;
        stIspCsiInRdma.stCsiInRdma.stRdmaStartCmd.u32Param =
            ST_CSI_IN_REG_LEN;
        stIspCsiInRdma.stCsiInRdma.stRdmaEndCmd.Tag = DESP_END;

        stIspCsiInRdma.stCsiInRdma.stCsiInReg.Sensor.field.imager_sel =
            (rand() & 0x01);
        stIspCsiInRdma.stCsiInRdma.stCsiInReg.Sensor.field.itu656_en =
            (rand() & 0x01);
        stIspCsiInRdma.stCsiInRdma.stCsiInReg.Sensor.field.dvp_sync_pol =
            (rand() & 0x03);
        u8Temp = (rand() & 0x03);
        stIspCsiInRdma.stCsiInRdma.stCsiInReg.FmtCfg.value =
            u32FirstPixel[u8Temp];
        stIspCsiInRdma.u8IspIdx = 0;
        ret = STFDRV_ISP_CSI_IN_SetReg(
            pstCiConnection,
            &stIspCsiInRdma
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_CSI_IN_SetReg()\n",
            stIspCsiInRdma.u8IspIdx);
        LOG_INFO("Imager_Sel = %s\n",
            (stIspCsiInRdma.stCsiInRdma.stCsiInReg.Sensor.field.imager_sel)
                ? ("MIPI") : ("DVP")
            );
        LOG_INFO("ITU_656_En = %s\n",
            (stIspCsiInRdma.stCsiInRdma.stCsiInReg.Sensor.field.itu656_en)
                ? ("Enable") : ("Disable")
            );
        LOG_INFO("DVP_Sync_Pol = %d(%s)\n",
            stIspCsiInRdma.stCsiInRdma.stCsiInReg.Sensor.field.dvp_sync_pol,
            szDvpSyncPol[stIspCsiInRdma.stCsiInRdma.stCsiInReg.Sensor.field.dvp_sync_pol]
            );
        LOG_INFO("CSI First Pixel = 0x%08X([%d]%s)\n",
            stIspCsiInRdma.stCsiInRdma.stCsiInReg.FmtCfg.value,
            u8Temp,
            szFirstPixel[u8Temp]
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Length = 2;
    u32Buffer[0 * 2] = ADDR_REG_SENSOR;
    u32Buffer[0 * 2 + 1] = 0;
    u32Buffer[1 * 2] = ADDR_REG_CFA;
    u32Buffer[1 * 2 + 1] = 0;
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

STF_RESULT Test_CSI_IN_SetLine(
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
    u32Offset = ADDR_REG_CSIINTS;
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
        STF_U16 u16Line;
        STF_U8 u8IspIdx = 0;

        u16Line = (rand() & 0x0FFF);
        u8IspIdx = 0;
        ret = STFDRV_ISP_CSI_IN_SetLine(
            pstCiConnection,
            u8IspIdx,
            u16Line
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_CSI_IN_SetLine() u16Line = %d(0x%04X)\n",
            u8IspIdx,
            u16Line,
            u16Line
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_CSIINTS;
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
        STF_U16 u16Line;
        STF_U8 u8IspIdx = 0;

        u16Line = 1080 - (1080 / 4);
        u8IspIdx = 0;
        ret = STFDRV_ISP_CSI_IN_SetLine(
            pstCiConnection,
            u8IspIdx,
            u16Line
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_CSI_IN_SetLine() u16Line = %d(0x%04X)\n",
            u8IspIdx,
            u16Line,
            u16Line
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_CSIINTS;
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

//-----------------------------------------------------------------------------
STF_RESULT ISP_Mod_Csi_In_Test(
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
        ret = Test_CSI_IN_SetReg(
            );
        LOG_INFO("Test Test_CSI_IN_SetReg command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the CSI_IN module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------
    //=========================================================================

    //=========================================================================
    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_CSI_IN_SetLine(
            );
        LOG_INFO("Test CSI_IN_SetLine command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the ISP_IN module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------
    //=========================================================================

    return ret;
}

//-----------------------------------------------------------------------------
