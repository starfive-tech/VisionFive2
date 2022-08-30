/**
  ******************************************************************************
  * @file  ISP_mod_cbar_test.c
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

#define LOG_TAG "Mod_CBar_Test"
#include "stf_common/userlog.h"

#include "ISP_mod_cbar_test.h"
#include "ISP_test.h"


/* ISPC CBAR driver test structure */


/* ISPC CBAR driver test variables */
static CI_CONNECTION *g_pstCiConnection = NULL;
static STF_CHAR g_szPattenType[][64] = {
    "Vertical bars",
    "Horizontal bars",
    "Vertical gradient",
    "Horizontal gradient",
};


/* ISPC CBAR driver test interface */
//-----------------------------------------------------------------------------
static STF_U32 Power(STF_U32 u32Base, STF_U32 u32Exponent)
{
    STF_U32 u32Result = 1;
    STF_U32 u32Idx;


    for (u32Idx = 0; u32Idx < u32Exponent; u32Idx++) {
        u32Result *= u32Base;
    }

    return u32Result;
}

STF_RESULT Test_CBAR_SetReg(
    STF_VOID
    )
{
    int i;
    //STF_U8 u8Method;
    STF_U32 u32Offset;
    STF_U32 u32Length;
    STF_U32 u32Buffer[ST_CBAR_REG_LEN];
    CI_CONNECTION *pstCiConnection = NULL;
    STF_RESULT ret = STF_SUCCESS;

    pstCiConnection = g_pstCiConnection;

    //=========================================================================
    u32Offset = ADDR_REG_CSICBFS;
    u32Length = ST_CBAR_REG_LEN;
    ret = STFLIB_ISP_RegReadSeries(
        TEST_ISP_DEVICE,
        u32Offset,
        u32Length,
        (STF_U32 *)&u32Buffer[0]
        );
    if (ret) {
        return ret;
    }
    for (i = 0; i < u32Length; i++) {
        LOG_INFO("Reg[0x%08X] <- 0x%08X\n", ADDR_REG_CSICBFS+i*4, u32Buffer[i]);
    }
    LOG_INFO("---------------------------------\n");
    //-------------------------------------------------------------------------
    {
        STF_U32 *pu32Temp;
        STF_U32 u32Idx;
        STF_U32 u32Cnt;
        ST_ISP_CBAR_RDMA stIspCBarRdma;

        stIspCBarRdma.stCBarRdma.stRdmaStartCmd.Tag = MREG_WR;
        stIspCBarRdma.stCBarRdma.stRdmaStartCmd.RegAddr =
            ADDR_REG_CSICBFS;
        stIspCBarRdma.stCBarRdma.stRdmaStartCmd.u32Param =
            ST_CBAR_REG_LEN;
        stIspCBarRdma.stCBarRdma.stRdmaEndCmd.Tag = DESP_END;

        u32Cnt = ST_CBAR_REG_LEN;
        pu32Temp = (STF_U32 *)&stIspCBarRdma.stCBarRdma.stCBarReg;
        for (u32Idx = 0; u32Idx < u32Cnt; u32Idx++) {
            switch (u32Idx) {
                case 0 ... 2:
                    pu32Temp[u32Idx] = (rand() & 0x1FFF)
                        | ((rand() & 0x1FFF) << 16);
                    break;

                case 3:
                    stIspCBarRdma.stCBarRdma.stCBarReg.Mode0.value = 0;
                    stIspCBarRdma.stCBarRdma.stCBarReg.Mode0.field.tg_en =
                        (rand() & 0x1);
                    stIspCBarRdma.stCBarRdma.stCBarReg.Mode0.field.dvp_en =
                        (rand() & 0x1);
                    stIspCBarRdma.stCBarRdma.stCBarReg.Mode0.field.mipi_en =
                        (rand() & 0x1);
                    stIspCBarRdma.stCBarRdma.stCBarReg.Mode0.field.hpolarity =
                        (rand() & 0x1);
                    stIspCBarRdma.stCBarRdma.stCBarReg.Mode0.field.vpolarity =
                        (rand() & 0x1);
                    //stIspCBarRdma.stCBarRdma.stCBarReg.Mode0.field.pattern =
                    //    (rand() & 0xF);
                    stIspCBarRdma.stCBarRdma.stCBarReg.Mode0.field.pattern =
                        (rand() & 0x3);
                    stIspCBarRdma.stCBarRdma.stCBarReg.Mode0.field.hwidth =
                        (rand() & 0xF);
                    stIspCBarRdma.stCBarRdma.stCBarReg.Mode0.field.vwidth =
                        (rand() & 0xF);
                    break;

                case 4:
                    pu32Temp[u32Idx] = (rand() & 0x00000FFF);
                    break;

                case 5 ... 12:
                    pu32Temp[u32Idx] = (rand() & 0x0FFF)
                        | ((rand() & 0x0FFF) << 16);
                    break;

                case 13 ... 20:
                    pu32Temp[u32Idx] = (rand() & 0x0FFF)
                        | ((rand() & 0x0FFF) << 16);
                    break;
            }
        }
        stIspCBarRdma.u8IspIdx = 0;
        ret = STFDRV_ISP_CBAR_SetReg(
            pstCiConnection,
            &stIspCBarRdma
            );
        if (ret) {
            return ret;
        }

        LOG_INFO("ISP_%d : STFDRV_ISP_CBAR_SetReg()\n", stIspCBarRdma.u8IspIdx);
        LOG_INFO("FrameSize - HTotal = 0x%04X, VTotal = 0x%04X\n",
            stIspCBarRdma.stCBarRdma.stCBarReg.FrameSize.field.htotal,
            stIspCBarRdma.stCBarRdma.stCBarReg.FrameSize.field.vtotal
            );
        LOG_INFO("FrameStart - HStart = 0x%04X, VStart = 0x%04X\n",
            stIspCBarRdma.stCBarRdma.stCBarReg.FrameStart.field.hstart,
            stIspCBarRdma.stCBarRdma.stCBarReg.FrameStart.field.vstart
            );
        LOG_INFO("FrameEnd - HENd = 0x%04X, VEnd = 0x%04X\n",
            stIspCBarRdma.stCBarRdma.stCBarReg.FrameEnd.field.hend,
            stIspCBarRdma.stCBarRdma.stCBarReg.FrameEnd.field.vend
            );
        LOG_INFO("Mode0 = 0x%08X\n",
            stIspCBarRdma.stCBarRdma.stCBarReg.Mode0.value
            );
        LOG_INFO("Mode0 - Timing Generator = 0x%01X(%s)\n",
            stIspCBarRdma.stCBarRdma.stCBarReg.Mode0.field.tg_en,
            ((stIspCBarRdma.stCBarRdma.stCBarReg.Mode0.field.tg_en)
                ? ("Enable") : ("Disable"))
            );
        LOG_INFO("Mode0 - DVP_Mux = 0x%01X(%s), MIPI_Mux = 0x%01X(%s)\n",
            stIspCBarRdma.stCBarRdma.stCBarReg.Mode0.field.dvp_en,
            ((stIspCBarRdma.stCBarRdma.stCBarReg.Mode0.field.dvp_en)
                ? ("Enable") : ("Disable")),
            stIspCBarRdma.stCBarRdma.stCBarReg.Mode0.field.mipi_en,
            ((stIspCBarRdma.stCBarRdma.stCBarReg.Mode0.field.mipi_en)
                ? ("Enable") : ("Disable"))
            );
        LOG_INFO("Mode0 - H_Polarity = 0x%01X(%s), V_Polarity = 0x%01X(%s)\n",
            stIspCBarRdma.stCBarRdma.stCBarReg.Mode0.field.hpolarity,
            ((stIspCBarRdma.stCBarRdma.stCBarReg.Mode0.field.hpolarity)
                ? ("Positive") : ("Negative")),
            stIspCBarRdma.stCBarRdma.stCBarReg.Mode0.field.vpolarity,
            ((stIspCBarRdma.stCBarRdma.stCBarReg.Mode0.field.vpolarity)
                ? ("Positive") : ("Negative"))
            );
        LOG_INFO("Mode0 - Pattern_Type = 0x%01X(%s)\n",
            stIspCBarRdma.stCBarRdma.stCBarReg.Mode0.field.pattern,
            g_szPattenType[stIspCBarRdma.stCBarRdma.stCBarReg.Mode0.field.pattern]
            );
        LOG_INFO("Mode0 - Horizontal bar width: = 0x%02X(%d => %d), "\
            "Vertical bar width = 0x%02X(%d => %d)\n",
            stIspCBarRdma.stCBarRdma.stCBarReg.Mode0.field.hwidth,
            stIspCBarRdma.stCBarRdma.stCBarReg.Mode0.field.hwidth,
            Power(2,
                (stIspCBarRdma.stCBarRdma.stCBarReg.Mode0.field.hwidth + 1)),
            stIspCBarRdma.stCBarRdma.stCBarReg.Mode0.field.vwidth,
            stIspCBarRdma.stCBarRdma.stCBarReg.Mode0.field.vwidth,
            Power(2,
                (stIspCBarRdma.stCBarRdma.stCBarReg.Mode0.field.vwidth + 1))
            );
        LOG_INFO("Mode1 = 0x%08X\n",
            stIspCBarRdma.stCBarRdma.stCBarReg.Mode1.value
            );
        LOG_INFO("Pixel[0-3] = 0x%03X, 0x%03X, 0x%03X, 0x%03X\n",
            stIspCBarRdma.stCBarRdma.stCBarReg.Pixel0_1.field.p0,
            stIspCBarRdma.stCBarRdma.stCBarReg.Pixel0_1.field.p1,
            stIspCBarRdma.stCBarRdma.stCBarReg.Pixel2_3.field.p2,
            stIspCBarRdma.stCBarRdma.stCBarReg.Pixel2_3.field.p3
            );
        LOG_INFO("Pixel[4-7] = 0x%03X, 0x%03X, 0x%03X, 0x%03X\n",
            stIspCBarRdma.stCBarRdma.stCBarReg.Pixel4_5.field.p4,
            stIspCBarRdma.stCBarRdma.stCBarReg.Pixel4_5.field.p5,
            stIspCBarRdma.stCBarRdma.stCBarReg.Pixel6_7.field.p6,
            stIspCBarRdma.stCBarRdma.stCBarReg.Pixel6_7.field.p7
            );
        LOG_INFO("Pixel[8-B] = 0x%03X, 0x%03X, 0x%03X, 0x%03X\n",
            stIspCBarRdma.stCBarRdma.stCBarReg.Pixel8_9.field.p8,
            stIspCBarRdma.stCBarRdma.stCBarReg.Pixel8_9.field.p9,
            stIspCBarRdma.stCBarRdma.stCBarReg.PixelA_B.field.pa,
            stIspCBarRdma.stCBarRdma.stCBarReg.PixelA_B.field.pb
            );
        LOG_INFO("Pixel[C-F] = 0x%03X, 0x%03X, 0x%03X, 0x%03X\n",
            stIspCBarRdma.stCBarRdma.stCBarReg.PixelC_D.field.pc,
            stIspCBarRdma.stCBarRdma.stCBarReg.PixelC_D.field.pd,
            stIspCBarRdma.stCBarRdma.stCBarReg.PixelE_F.field.pe,
            stIspCBarRdma.stCBarRdma.stCBarReg.PixelE_F.field.pf
            );
        LOG_INFO("Line 1 Pixel[0-3] = 0x%03X, 0x%03X, 0x%03X, 0x%03X\n",
            stIspCBarRdma.stCBarRdma.stCBarReg.Line1_Pixel0_1.field.p0,
            stIspCBarRdma.stCBarRdma.stCBarReg.Line1_Pixel0_1.field.p1,
            stIspCBarRdma.stCBarRdma.stCBarReg.Line1_Pixel2_3.field.p2,
            stIspCBarRdma.stCBarRdma.stCBarReg.Line1_Pixel2_3.field.p3
            );
        LOG_INFO("Line 1 Pixel[4-7] = 0x%03X, 0x%03X, 0x%03X, 0x%03X\n",
            stIspCBarRdma.stCBarRdma.stCBarReg.Line1_Pixel4_5.field.p4,
            stIspCBarRdma.stCBarRdma.stCBarReg.Line1_Pixel4_5.field.p5,
            stIspCBarRdma.stCBarRdma.stCBarReg.Line1_Pixel6_7.field.p6,
            stIspCBarRdma.stCBarRdma.stCBarReg.Line1_Pixel6_7.field.p7
            );
        LOG_INFO("Line 1 Pixel[8-B] = 0x%03X, 0x%03X, 0x%03X, 0x%03X\n",
            stIspCBarRdma.stCBarRdma.stCBarReg.Line1_Pixel8_9.field.p8,
            stIspCBarRdma.stCBarRdma.stCBarReg.Line1_Pixel8_9.field.p9,
            stIspCBarRdma.stCBarRdma.stCBarReg.Line1_PixelA_B.field.pa,
            stIspCBarRdma.stCBarRdma.stCBarReg.Line1_PixelA_B.field.pb
            );
        LOG_INFO("Line 1 Pixel[C-F] = 0x%03X, 0x%03X, 0x%03X, 0x%03X\n",
            stIspCBarRdma.stCBarRdma.stCBarReg.Line1_PixelC_D.field.pc,
            stIspCBarRdma.stCBarRdma.stCBarReg.Line1_PixelC_D.field.pd,
            stIspCBarRdma.stCBarRdma.stCBarReg.Line1_PixelE_F.field.pe,
            stIspCBarRdma.stCBarRdma.stCBarReg.Line1_PixelE_F.field.pf
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_CSICBFS;
    u32Length = ST_CBAR_REG_LEN;
    ret = STFLIB_ISP_RegReadSeries(
        TEST_ISP_DEVICE,
        u32Offset,
        u32Length,
        (STF_U32 *)&u32Buffer[0]
        );
    if (ret) {
        return ret;
    }
    for (i = 0; i < u32Length; i++) {
        LOG_INFO("Reg[0x%08X] = 0x%08X\n", ADDR_REG_CSICBFS+i*4, u32Buffer[i]);
    }
    LOG_INFO("---------------------------------\n");
    //=========================================================================

    return ret;
}

//-----------------------------------------------------------------------------
STF_RESULT ISP_Mod_Cbar_Test(
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
        ret = Test_CBAR_SetReg(
            );
        LOG_INFO("Test CBAR_SetReg command is %s, ret = %d(%s)\n",
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

    return ret;
}

//-----------------------------------------------------------------------------
