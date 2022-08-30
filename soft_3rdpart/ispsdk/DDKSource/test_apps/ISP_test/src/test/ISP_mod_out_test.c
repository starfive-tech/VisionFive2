/**
  ******************************************************************************
  * @file  ISP_mod_out_test.c
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

#define LOG_TAG "Mod_Out_Test"
#include "stf_common/userlog.h"

#include "ISP_mod_out_test.h"
#include "ISP_test.h"


/* ISPC OUT driver test structure */


/* ISPC OUT driver test variables */
static CI_CONNECTION *g_pstCiConnection = NULL;


/* ISPC OUT driver test interface */
//-----------------------------------------------------------------------------
STF_RESULT Test_OUT_SetReg(
    STF_VOID
    )
{
    int i;
    //STF_U8 u8Method;
    STF_U32 u32Offset;
    STF_U32 u32Length;
    STF_U32 u32Buffer[ST_OUT_REG_LEN + 6];
    CI_CONNECTION *pstCiConnection = NULL;
    STF_RESULT ret = STF_SUCCESS;

    pstCiConnection = g_pstCiConnection;

    //=========================================================================
    u32Offset = ADDR_REG_UOAY;
    u32Length = ST_OUT_REG_LEN + 6;
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
        LOG_INFO("Reg[0x%08X] = 0x%08X\n", ADDR_REG_UOAY+i*4, u32Buffer[i]);
    }
    LOG_INFO("---------------------------------\n");
    //-------------------------------------------------------------------------
    {
        STF_U8 *pu8Temp = NULL;
        STF_U32 u32Idx;
        STF_U32 u32Cnt;
        ST_ISP_OUT_RDMA stIspOutRdma;

        stIspOutRdma.stOutRdma.stRdmaStartCmd.Tag = MREG_WR;
#if defined(V4L2_DRIVER)
        stIspOutRdma.stOutRdma.stRdmaStartCmd.RegAddr = ADDR_REG_UOPGM;
#else
        stIspOutRdma.stOutRdma.stRdmaStartCmd.RegAddr = ADDR_REG_UOAY;
#endif //#if defined(V4L2_DRIVER)
        stIspOutRdma.stOutRdma.stRdmaStartCmd.u32Param = ST_OUT_UO_REG_LEN;
        stIspOutRdma.stOutRdma.stRdmaSs0Cmd.Tag = MREG_WR;
#if defined(V4L2_DRIVER)
        stIspOutRdma.stOutRdma.stRdmaSs0Cmd.RegAddr = ADDR_REG_SS0S;
#else
        stIspOutRdma.stOutRdma.stRdmaSs0Cmd.RegAddr = ADDR_REG_SS0AY;
#endif //#if defined(V4L2_DRIVER)
        stIspOutRdma.stOutRdma.stRdmaSs0Cmd.u32Param = ST_OUT_SS0_REG_LEN;
        stIspOutRdma.stOutRdma.stRdmaSs1Cmd.Tag = MREG_WR;
#if defined(V4L2_DRIVER)
        stIspOutRdma.stOutRdma.stRdmaSs1Cmd.RegAddr = ADDR_REG_SS1S;
#else
        stIspOutRdma.stOutRdma.stRdmaSs1Cmd.RegAddr = ADDR_REG_SS1AY;
#endif //#if defined(V4L2_DRIVER)
        stIspOutRdma.stOutRdma.stRdmaSs1Cmd.u32Param = ST_OUT_SS1_REG_LEN;
        stIspOutRdma.stOutRdma.stRdmaSsAxiCmd.Tag = MREG_WR;
        stIspOutRdma.stOutRdma.stRdmaSsAxiCmd.RegAddr = ADDR_REG_SSAXI;
        stIspOutRdma.stOutRdma.stRdmaSsAxiCmd.u32Param = ST_OUT_SS_AXI_REG_LEN;
        stIspOutRdma.stOutRdma.stRdmaEndCmd.Tag = DESP_END;

#if defined(V4L2_DRIVER)
#else
        stIspOutRdma.stOutRdma.stOutUoReg.UoYAddr.value =
            (rand() & 0xFFFFFFF8);
        stIspOutRdma.stOutRdma.stOutUoReg.UoUvAddr.value =
            (rand() & 0xFFFFFFF8);
        stIspOutRdma.stOutRdma.stOutUoReg.UoStride.value =
            (rand() & 0x00001FF8);
#endif //#if defined(V4L2_DRIVER)
        stIspOutRdma.stOutRdma.stOutUoReg.UoUvMode.value =
            ((rand() & 0x01) << 5);
        stIspOutRdma.stOutRdma.stOutUoReg.UoAxiId.value =
            ((rand() & 0xFF) << 8);
#if defined(V4L2_DRIVER)
#else
        stIspOutRdma.stOutRdma.stOutSs0Reg.Ss0YAddr.value =
            (rand() & 0xFFFFFFF8);
        stIspOutRdma.stOutRdma.stOutSs0Reg.Ss0UvAddr.value =
            (rand() & 0xFFFFFFF8);
#endif //#if defined(V4L2_DRIVER)
        stIspOutRdma.stOutRdma.stOutSs0Reg.Ss0Stride.value =
            (rand() & 0x00001FF8);
        stIspOutRdma.stOutRdma.stOutSs0Reg.Ss0HSF.value = (rand() & 0x02)
            | ((rand() & 0xFFF) << 16);
        stIspOutRdma.stOutRdma.stOutSs0Reg.Ss0VSF.value = (rand() & 0x02)
            | ((rand() & 0xFFF) << 16);
        stIspOutRdma.stOutRdma.stOutSs0Reg.Ss0Size.value = (rand() & 0x1FFF)
            | ((rand() & 0x1FFF) << 16);
#if defined(V4L2_DRIVER)
#else
        stIspOutRdma.stOutRdma.stOutSs1Reg.Ss1YAddr.value =
            (rand() & 0xFFFFFFF8);
        stIspOutRdma.stOutRdma.stOutSs1Reg.Ss1UvAddr.value =
            (rand() & 0xFFFFFFF8);
#endif //#if defined(V4L2_DRIVER)
        stIspOutRdma.stOutRdma.stOutSs1Reg.Ss1Stride.value =
            (rand() & 0x00001FF8);
        stIspOutRdma.stOutRdma.stOutSs1Reg.Ss1HSF.value = (rand() & 0x02)
            | ((rand() & 0xFFF) << 16);
        stIspOutRdma.stOutRdma.stOutSs1Reg.Ss1VSF.value = (rand() & 0x02)
            | ((rand() & 0xFFF) << 16);
        stIspOutRdma.stOutRdma.stOutSs1Reg.Ss1Size.value = (rand() & 0x1FFF)
            | ((rand() & 0x1FFF) << 16);
        stIspOutRdma.stOutRdma.stOutSsAxiReg.SsAxiId.value = (rand() & 0xFF)
            | ((rand() & 0xFF) << 8);
        stIspOutRdma.u8IspIdx = 0;
        ret = STFDRV_ISP_OUT_SetReg(
            pstCiConnection,
            &stIspOutRdma
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_OUT_SetReg()\n", stIspOutRdma.u8IspIdx);
#if defined(V4L2_DRIVER)
        //LOG_INFO("Uo - Stride =  = 0x%04X\n",
        //    stIspOutRdma.stOutRdma.stOutUoReg.UoStride.value
        //    );
#else
        LOG_INFO("Uo - YAddr = 0x%08X, UvAddr = 0x%08X, Stride =  = 0x%04X\n",
            stIspOutRdma.stOutRdma.stOutUoReg.UoYAddr.value,
            stIspOutRdma.stOutRdma.stOutUoReg.UoUvAddr.value,
            stIspOutRdma.stOutRdma.stOutUoReg.UoStride.value
            );
#endif //#if defined(V4L2_DRIVER)
        LOG_INFO("Uo - UvMode = 0x%01X(%s), AxiId = 0x%02X\n",
            stIspOutRdma.stOutRdma.stOutUoReg.UoUvMode.field.nv21,
            ((stIspOutRdma.stOutRdma.stOutUoReg.UoUvMode.field.nv21)
                ? ("NV21") : ("NV12")),
            stIspOutRdma.stOutRdma.stOutUoReg.UoAxiId.field.refwid
            );
        LOG_INFO("--------------------\n");
#if defined(V4L2_DRIVER)
        LOG_INFO("Ss0 -  Stride =  = 0x%04X\n",
            stIspOutRdma.stOutRdma.stOutSs0Reg.Ss0Stride.value
            );
#else
        LOG_INFO("Ss0 - YAddr = 0x%08X, UvAddr = 0x%08X, Stride =  = 0x%04X\n",
            stIspOutRdma.stOutRdma.stOutSs0Reg.Ss0YAddr.value,
            stIspOutRdma.stOutRdma.stOutSs0Reg.Ss0UvAddr.value,
            stIspOutRdma.stOutRdma.stOutSs0Reg.Ss0Stride.value
            );
#endif //#if defined(V4L2_DRIVER)
        LOG_INFO("Ss0 - Horizontal Scaling - Mode = 0x%01X(%s), "\
            "Factor = 0x%04X\n",
            stIspOutRdma.stOutRdma.stOutSs0Reg.Ss0HSF.field.sm,
            ((0 == stIspOutRdma.stOutRdma.stOutSs0Reg.Ss0HSF.field.sm)
                ? ("Scale Down") : ("No Scaling")),
            stIspOutRdma.stOutRdma.stOutSs0Reg.Ss0HSF.field.sf
            );
        LOG_INFO("Ss0 - Vertical Scaling - Mode = 0x%01X(%s), "\
            "Factor = 0x%04X\n",
            stIspOutRdma.stOutRdma.stOutSs0Reg.Ss0VSF.field.sm,
            ((0 == stIspOutRdma.stOutRdma.stOutSs0Reg.Ss0VSF.field.sm)
                ? ("Scale Down") : ("No Scaling")),
            stIspOutRdma.stOutRdma.stOutSs0Reg.Ss0VSF.field.sf
            );
        LOG_INFO("Ss0 - Output size - Width = 0x%04X(%d), "\
            "Height = 0x%04X(%d)\n",
            stIspOutRdma.stOutRdma.stOutSs0Reg.Ss0Size.field.w,
            stIspOutRdma.stOutRdma.stOutSs0Reg.Ss0Size.field.w,
            stIspOutRdma.stOutRdma.stOutSs0Reg.Ss0Size.field.h,
            stIspOutRdma.stOutRdma.stOutSs0Reg.Ss0Size.field.h
            );
        LOG_INFO("Ss0 - AxiId = 0x%02X\n",
            stIspOutRdma.stOutRdma.stOutSsAxiReg.SsAxiId.field.ss0wid
            );
        LOG_INFO("--------------------\n");
#if defined(V4L2_DRIVER)
        LOG_INFO("Ss1 - Stride =  = 0x%04X\n",
            stIspOutRdma.stOutRdma.stOutSs1Reg.Ss1Stride.value
            );
#else
        LOG_INFO("Ss1 - YAddr = 0x%08X, UvAddr = 0x%08X, Stride =  = 0x%04X\n",
            stIspOutRdma.stOutRdma.stOutSs1Reg.Ss1YAddr.value,
            stIspOutRdma.stOutRdma.stOutSs1Reg.Ss1UvAddr.value,
            stIspOutRdma.stOutRdma.stOutSs1Reg.Ss1Stride.value
            );
#endif //#if defined(V4L2_DRIVER)
        LOG_INFO("Ss1 - Horizontal Scaling - Mode = 0x%01X(%s), "\
            "Factor = 0x%04X\n",
            stIspOutRdma.stOutRdma.stOutSs1Reg.Ss1HSF.field.sm,
            ((0 == stIspOutRdma.stOutRdma.stOutSs1Reg.Ss1HSF.field.sm)
                ? ("Scale Down") : ("No Scaling")),
            stIspOutRdma.stOutRdma.stOutSs1Reg.Ss1HSF.field.sf
            );
        LOG_INFO("Ss1 - Vertical Scaling - Mode = 0x%01X(%s), "\
            "Factor = 0x%04X\n",
            stIspOutRdma.stOutRdma.stOutSs1Reg.Ss1VSF.field.sm,
            ((0 == stIspOutRdma.stOutRdma.stOutSs1Reg.Ss1VSF.field.sm)
                ? ("Scale Down") : ("No Scaling")),
            stIspOutRdma.stOutRdma.stOutSs1Reg.Ss1VSF.field.sf
            );
        LOG_INFO("Ss1 - Output size - Width = 0x%04X(%d), "\
            "Height = 0x%04X(%d)\n",
            stIspOutRdma.stOutRdma.stOutSs1Reg.Ss1Size.field.w,
            stIspOutRdma.stOutRdma.stOutSs1Reg.Ss1Size.field.w,
            stIspOutRdma.stOutRdma.stOutSs1Reg.Ss1Size.field.h,
            stIspOutRdma.stOutRdma.stOutSs1Reg.Ss1Size.field.h
            );
        LOG_INFO("Ss1 - AxiId = 0x%02X\n",
            stIspOutRdma.stOutRdma.stOutSsAxiReg.SsAxiId.field.ss1wid
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_UOAY;
    u32Length = ST_OUT_REG_LEN + 6;
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
        LOG_INFO("Reg[0x%08X] = 0x%08X\n", ADDR_REG_UOAY+i*4, u32Buffer[i]);
    }
    LOG_INFO("---------------------------------\n");
    //=========================================================================

    return ret;
}

#if defined(EXTEND_IOCTL_COMMAND_OUT)
//-----------------------------------------------------------------------------
STF_RESULT Test_OUT_SetUoInfo(
    STF_VOID
    )
{
    int i;
    //STF_U8 u8Method;
    //STF_U32 u32Offset;
    STF_U32 u32Length;
    STF_U32 u32Buffer[(ST_OUT_UO_INFO_REG_LEN) * 2];
    CI_CONNECTION *pstCiConnection = NULL;
    STF_RESULT ret = STF_SUCCESS;

    pstCiConnection = g_pstCiConnection;

    //=========================================================================
    u32Length = ST_OUT_UO_INFO_REG_LEN;
    for (i = 0; i < u32Length; i++) {
#if defined(V4L2_DRIVER)
        u32Buffer[i * 2] = ADDR_REG_UOPGM + (i * 4);
#else
        u32Buffer[i * 2] = ADDR_REG_UOAY + (i * 4);
#endif //#if defined(V4L2_DRIVER)
        u32Buffer[i * 2 + 1] = 0;
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
        STF_U8 *pu8Temp = NULL;
        STF_U32 u32Idx;
        STF_U32 u32Cnt;
        ST_OUT_UO_INFO_PARAM stIspOutUoInfo;

#if defined(V4L2_DRIVER)
#else
        stIspOutUoInfo.stOutUoInfo.u32YAddr = (rand() & 0xFFFFFFF8);
        stIspOutUoInfo.stOutUoInfo.u32UvAddr = (rand() & 0xFFFFFFF8);
        stIspOutUoInfo.stOutUoInfo.u32Stride = (rand() & 0x00001FF8);
#endif //#if defined(V4L2_DRIVER)
        stIspOutUoInfo.stOutUoInfo.eUvOrder = ((rand() & 0x01)
            ? (EN_UV_ORDER_VU) : (EN_UV_ORDER_UV));
        stIspOutUoInfo.stOutUoInfo.u8AxiId = (rand() & 0xFF);
        stIspOutUoInfo.u8IspIdx = 0;
        ret = STFDRV_ISP_OUT_SetUoInfo(
            pstCiConnection,
            &stIspOutUoInfo
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_OUT_SetUoReg()\n",
            stIspOutUoInfo.u8IspIdx);
#if defined(V4L2_DRIVER)
        //LOG_INFO("Uo - Stride =  = 0x%04X\n",
        //    stIspOutUoInfo.stOutUoInfo.u32Stride
        //    );
#else
        LOG_INFO("Uo - YAddr = 0x%08X, UvAddr = 0x%08X, Stride =  = 0x%04X\n",
            stIspOutUoInfo.stOutUoInfo.u32YAddr,
            stIspOutUoInfo.stOutUoInfo.u32UvAddr,
            stIspOutUoInfo.stOutUoInfo.u32Stride
            );
#endif //#if defined(V4L2_DRIVER)
        LOG_INFO("Uo - UvMode = 0x%01X(%s), AxiId = 0x%02X\n",
            stIspOutUoInfo.stOutUoInfo.eUvOrder,
            ((stIspOutUoInfo.stOutUoInfo.eUvOrder) ? ("NV21") : ("NV12")),
            stIspOutUoInfo.stOutUoInfo.u8AxiId
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Length = ST_OUT_UO_INFO_REG_LEN;
    for (i = 0; i < u32Length; i++) {
#if defined(V4L2_DRIVER)
        u32Buffer[i * 2] = ADDR_REG_UOPGM + (i * 4);
#else
        u32Buffer[i * 2] = ADDR_REG_UOAY + (i * 4);
#endif //#if defined(V4L2_DRIVER)
        u32Buffer[i * 2 + 1] = 0;
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

STF_RESULT Test_OUT_SetSs0Info(
    STF_VOID
    )
{
    int i;
    //STF_U8 u8Method;
    //STF_U32 u32Offset;
    STF_U32 u32Length;
    STF_U32 u32Buffer[(ST_OUT_SS0_INFO_REG_LEN) * 2];
    CI_CONNECTION *pstCiConnection = NULL;
    STF_RESULT ret = STF_SUCCESS;

    pstCiConnection = g_pstCiConnection;

    //=========================================================================
    u32Length = ST_OUT_SS0_INFO_REG_LEN;
    for (i = 0; i < u32Length; i++) {
#if defined(V4L2_DRIVER)
        if (0 == i) {
            u32Buffer[i * 2] = ADDR_REG_UOPGM;
        } else if (5 == i) {
            u32Buffer[i * 2] = ADDR_REG_SSAXI;
        } else {
            u32Buffer[i * 2] = ADDR_REG_SS0S + ((i - 1) * 4);
        }
#else
        if (0 == i) {
            u32Buffer[i * 2] = ADDR_REG_UOPGM;
        } else if (7 == i) {
            u32Buffer[i * 2] = ADDR_REG_SSAXI;
        } else {
            u32Buffer[i * 2] = ADDR_REG_SS0AY + ((i - 1) * 4);
        }
#endif //#if defined(V4L2_DRIVER)
        u32Buffer[i * 2 + 1] = 0;
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
        STF_U8 *pu8Temp = NULL;
        STF_U32 u32Idx;
        STF_U32 u32Cnt;
        ST_OUT_SS_INFO_PARAM stIspOutSs0Info;

#if defined(V4L2_DRIVER)
#else
        stIspOutSs0Info.stOutSsInfo.u32YAddr = (rand() & 0xFFFFFFF8);
        stIspOutSs0Info.stOutSsInfo.u32UvAddr = (rand() & 0xFFFFFFF8);
#endif //#if defined(V4L2_DRIVER)
        stIspOutSs0Info.stOutSsInfo.u32Stride = (rand() & 0x00001FF8);
        stIspOutSs0Info.stOutSsInfo.eUvOrder = ((rand() & 0x01)
            ? (EN_UV_ORDER_VU) : (EN_UV_ORDER_UV));
        stIspOutSs0Info.stOutSsInfo.eHorScalingMode = ((rand() & 0x02)
            ? (EN_SCALING_MODE_NO_SCALING) : (EN_SCALING_MODE_SCALING_DOWN));;
        stIspOutSs0Info.stOutSsInfo.u16HorScalingFactor = (rand() & 0xFFF);
        stIspOutSs0Info.stOutSsInfo.eVerScalingMode = ((rand() & 0x02)
            ? (EN_SCALING_MODE_NO_SCALING) : (EN_SCALING_MODE_SCALING_DOWN));;
        stIspOutSs0Info.stOutSsInfo.u16VerScalingFactor = (rand() & 0xFFF);
        stIspOutSs0Info.stOutSsInfo.u16OutWidth = (rand() & 0x1FFF);
        stIspOutSs0Info.stOutSsInfo.u16OutHeight = (rand() & 0x1FFF);
        stIspOutSs0Info.stOutSsInfo.u8AxiId = (rand() & 0xFF);
        stIspOutSs0Info.u8IspIdx = 0;
        ret = STFDRV_ISP_OUT_SetSs0Info(
            pstCiConnection,
            &stIspOutSs0Info
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_OUT_SetSs0Reg()\n",
            stIspOutSs0Info.u8IspIdx);
        LOG_INFO("Ss0 - UvMode = 0x%01X(%s)\n",
            stIspOutSs0Info.stOutSsInfo.eUvOrder,
            ((stIspOutSs0Info.stOutSsInfo.eUvOrder) ? ("NV21") : ("NV12"))
            );
#if defined(V4L2_DRIVER)
        LOG_INFO("Ss0 - Stride =  = 0x%04X\n",
            stIspOutSs0Info.stOutSsInfo.u32Stride
            );
#else
        LOG_INFO("Ss0 - YAddr = 0x%08X, UvAddr = 0x%08X, Stride =  = 0x%04X\n",
            stIspOutSs0Info.stOutSsInfo.u32YAddr,
            stIspOutSs0Info.stOutSsInfo.u32UvAddr,
            stIspOutSs0Info.stOutSsInfo.u32Stride
            );
#endif //#if defined(V4L2_DRIVER)
        LOG_INFO("Ss0 - Horizontal Scaling - Mode = 0x%01X(%s), "\
            "Factor = 0x%04X\n",
            stIspOutSs0Info.stOutSsInfo.eHorScalingMode,
            ((0 == stIspOutSs0Info.stOutSsInfo.eHorScalingMode)
                ? ("Scale Down") : ("No Scaling")),
            stIspOutSs0Info.stOutSsInfo.u16HorScalingFactor
            );
        LOG_INFO("Ss0 - Vertical Scaling - Mode = 0x%01X(%s), "\
            "Factor = 0x%04X\n",
            stIspOutSs0Info.stOutSsInfo.eVerScalingMode,
            ((0 == stIspOutSs0Info.stOutSsInfo.eVerScalingMode)
                ? ("Scale Down") : ("No Scaling")),
            stIspOutSs0Info.stOutSsInfo.u16VerScalingFactor
            );
        LOG_INFO("Ss0 - Output size - Width = 0x%04X(%d), "\
            "Height = 0x%04X(%d)\n",
            stIspOutSs0Info.stOutSsInfo.u16OutWidth,
            stIspOutSs0Info.stOutSsInfo.u16OutWidth,
            stIspOutSs0Info.stOutSsInfo.u16OutHeight,
            stIspOutSs0Info.stOutSsInfo.u16OutHeight
            );
        LOG_INFO("Ss0 - AxiId = 0x%02X\n",
            stIspOutSs0Info.stOutSsInfo.u8AxiId
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Length = ST_OUT_SS0_INFO_REG_LEN;
    for (i = 0; i < u32Length; i++) {
#if defined(V4L2_DRIVER)
        if (0 == i) {
            u32Buffer[i * 2] = ADDR_REG_UOPGM;
        } else if (5 == i) {
            u32Buffer[i * 2] = ADDR_REG_SSAXI;
        } else {
            u32Buffer[i * 2] = ADDR_REG_SS0S + ((i - 1) * 4);
        }
#else
        if (0 == i) {
            u32Buffer[i * 2] = ADDR_REG_UOPGM;
        } else if (7 == i) {
            u32Buffer[i * 2] = ADDR_REG_SSAXI;
        } else {
            u32Buffer[i * 2] = ADDR_REG_SS0AY + ((i - 1) * 4);
        }
#endif //#if defined(V4L2_DRIVER)
        u32Buffer[i * 2 + 1] = 0;
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

STF_RESULT Test_OUT_SetSs1Info(
    STF_VOID
    )
{
    int i;
    //STF_U8 u8Method;
    //STF_U32 u32Offset;
    STF_U32 u32Length;
    STF_U32 u32Buffer[(ST_OUT_SS1_INFO_REG_LEN) * 2];
    CI_CONNECTION *pstCiConnection = NULL;
    STF_RESULT ret = STF_SUCCESS;

    pstCiConnection = g_pstCiConnection;

    //=========================================================================
    u32Length = ST_OUT_SS1_INFO_REG_LEN;
    for (i = 0; i < u32Length; i++) {
#if defined(V4L2_DRIVER)
        if (0 == i) {
            u32Buffer[i * 2] = ADDR_REG_UOPGM;
        } else if (5 == i) {
            u32Buffer[i * 2] = ADDR_REG_SSAXI;
        } else {
            u32Buffer[i * 2] = ADDR_REG_SS1S + ((i - 1) * 4);
        }
#else
        if (0 == i) {
            u32Buffer[i * 2] = ADDR_REG_UOPGM;
        } else if (7 == i) {
            u32Buffer[i * 2] = ADDR_REG_SSAXI;
        } else {
            u32Buffer[i * 2] = ADDR_REG_SS1AY + ((i - 1) * 4);
        }
#endif //#if defined(V4L2_DRIVER)
        u32Buffer[i * 2 + 1] = 0;
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
        STF_U8 *pu8Temp = NULL;
        STF_U32 u32Idx;
        STF_U32 u32Cnt;
        ST_OUT_SS_INFO_PARAM stIspOutSs1Info;

#if defined(V4L2_DRIVER)
#else
        stIspOutSs1Info.stOutSsInfo.u32YAddr = (rand() & 0xFFFFFFF8);
        stIspOutSs1Info.stOutSsInfo.u32UvAddr = (rand() & 0xFFFFFFF8);
#endif //#if defined(V4L2_DRIVER)
        stIspOutSs1Info.stOutSsInfo.u32Stride = (rand() & 0x00001FF8);
        stIspOutSs1Info.stOutSsInfo.eUvOrder = ((rand() & 0x01)
            ? (EN_UV_ORDER_VU) : (EN_UV_ORDER_UV));
        stIspOutSs1Info.stOutSsInfo.eHorScalingMode = ((rand() & 0x02)
            ? (EN_SCALING_MODE_NO_SCALING) : (EN_SCALING_MODE_SCALING_DOWN));;
        stIspOutSs1Info.stOutSsInfo.u16HorScalingFactor = (rand() & 0xFFF);
        stIspOutSs1Info.stOutSsInfo.eVerScalingMode = ((rand() & 0x02)
            ? (EN_SCALING_MODE_NO_SCALING) : (EN_SCALING_MODE_SCALING_DOWN));;
        stIspOutSs1Info.stOutSsInfo.u16VerScalingFactor = (rand() & 0xFFF);
        stIspOutSs1Info.stOutSsInfo.u16OutWidth = (rand() & 0x1FFF);
        stIspOutSs1Info.stOutSsInfo.u16OutHeight = (rand() & 0x1FFF);
        stIspOutSs1Info.stOutSsInfo.u8AxiId = (rand() & 0xFF);
        stIspOutSs1Info.u8IspIdx = 0;
        ret = STFDRV_ISP_OUT_SetSs1Info(
            pstCiConnection,
            &stIspOutSs1Info
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_OUT_SetSs1Reg()\n",
            stIspOutSs1Info.u8IspIdx);
        LOG_INFO("Ss1 - UvMode = 0x%01X(%s)\n",
            stIspOutSs1Info.stOutSsInfo.eUvOrder,
            ((stIspOutSs1Info.stOutSsInfo.eUvOrder) ? ("NV21") : ("NV12"))
            );
#if defined(V4L2_DRIVER)
        LOG_INFO("Ss1 - Stride =  = 0x%04X\n",
            stIspOutSs1Info.stOutSsInfo.u32Stride
            );
#else
        LOG_INFO("Ss1 - YAddr = 0x%08X, UvAddr = 0x%08X, Stride =  = 0x%04X\n",
            stIspOutSs1Info.stOutSsInfo.u32YAddr,
            stIspOutSs1Info.stOutSsInfo.u32UvAddr,
            stIspOutSs1Info.stOutSsInfo.u32Stride
            );
#endif //#if defined(V4L2_DRIVER)
        LOG_INFO("Ss1 - Horizontal Scaling - Mode = 0x%01X(%s), "\
            "Factor = 0x%04X\n",
            stIspOutSs1Info.stOutSsInfo.eHorScalingMode,
            ((0 == stIspOutSs1Info.stOutSsInfo.eHorScalingMode)
                ? ("Scale Down") : ("No Scaling")),
            stIspOutSs1Info.stOutSsInfo.u16HorScalingFactor
            );
        LOG_INFO("Ss1 - Vertical Scaling - Mode = 0x%01X(%s), "\
            "Factor = 0x%04X\n",
            stIspOutSs1Info.stOutSsInfo.eVerScalingMode,
            ((0 == stIspOutSs1Info.stOutSsInfo.eVerScalingMode)
                ? ("Scale Down") : ("No Scaling")),
            stIspOutSs1Info.stOutSsInfo.u16VerScalingFactor
            );
        LOG_INFO("Ss1 - Output size - Width = 0x%04X(%d), "\
            "Height = 0x%04X(%d)\n",
            stIspOutSs1Info.stOutSsInfo.u16OutWidth,
            stIspOutSs1Info.stOutSsInfo.u16OutWidth,
            stIspOutSs1Info.stOutSsInfo.u16OutHeight,
            stIspOutSs1Info.stOutSsInfo.u16OutHeight
            );
        LOG_INFO("Ss1 - AxiId = 0x%02X\n",
            stIspOutSs1Info.stOutSsInfo.u8AxiId
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Length = ST_OUT_SS1_INFO_REG_LEN;
    for (i = 0; i < u32Length; i++) {
#if defined(V4L2_DRIVER)
        if (0 == i) {
            u32Buffer[i * 2] = ADDR_REG_UOPGM;
        } else if (5 == i) {
            u32Buffer[i * 2] = ADDR_REG_SSAXI;
        } else {
            u32Buffer[i * 2] = ADDR_REG_SS1S + ((i - 1) * 4);
        }
#else
        if (0 == i) {
            u32Buffer[i * 2] = ADDR_REG_UOPGM;
        } else if (7 == i) {
            u32Buffer[i * 2] = ADDR_REG_SSAXI;
        } else {
            u32Buffer[i * 2] = ADDR_REG_SS1AY + ((i - 1) * 4);
        }
#endif //#if defined(V4L2_DRIVER)
        u32Buffer[i * 2 + 1] = 0;
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

#endif //#if defined(EXTEND_IOCTL_COMMAND_OUT)
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
STF_RESULT Test_OUT_UO_SetReg(
    STF_VOID
    )
{
    int i;
    //STF_U8 u8Method;
    STF_U32 u32Offset;
    STF_U32 u32Length;
    STF_U32 u32Buffer[ST_OUT_UO_REG_LEN];
    CI_CONNECTION *pstCiConnection = NULL;
    STF_RESULT ret = STF_SUCCESS;

    pstCiConnection = g_pstCiConnection;

    //=========================================================================
#if defined(V4L2_DRIVER)
    u32Offset = ADDR_REG_UOPGM;
#else
    u32Offset = ADDR_REG_UOAY;
#endif //#if defined(V4L2_DRIVER)
    u32Length = ST_OUT_UO_REG_LEN;
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
        LOG_INFO("Reg[0x%08X] = 0x%08X\n", u32Offset+i*4, u32Buffer[i]);
    }
    LOG_INFO("---------------------------------\n");
    //-------------------------------------------------------------------------
    {
        STF_U8 *pu8Temp = NULL;
        STF_U32 u32Idx;
        STF_U32 u32Cnt;
        ST_ISP_OUT_UO_RDMA stIspOutUoRdma;

        stIspOutUoRdma.stOutUoRdma.stRdmaStartCmd.Tag = MREG_WR;
#if defined(V4L2_DRIVER)
        stIspOutUoRdma.stOutUoRdma.stRdmaStartCmd.RegAddr = ADDR_REG_UOPGM;
#else
        stIspOutUoRdma.stOutUoRdma.stRdmaStartCmd.RegAddr = ADDR_REG_UOAY;
#endif //#if defined(V4L2_DRIVER)
        stIspOutUoRdma.stOutUoRdma.stRdmaStartCmd.u32Param = ST_OUT_UO_REG_LEN;
        stIspOutUoRdma.stOutUoRdma.stRdmaEndCmd.Tag = DESP_END;

#if defined(V4L2_DRIVER)
#else
        stIspOutUoRdma.stOutUoRdma.stOutUoReg.UoYAddr.value = (rand() & 0xFFFFFFF8);
        stIspOutUoRdma.stOutUoRdma.stOutUoReg.UoUvAddr.value = (rand() & 0xFFFFFFF8);
        stIspOutUoRdma.stOutUoRdma.stOutUoReg.UoStride.value = (rand() & 0x00001FF8);
#endif //#if defined(V4L2_DRIVER)
        stIspOutUoRdma.stOutUoRdma.stOutUoReg.UoUvMode.value = ((rand() & 0x01) << 5);
        stIspOutUoRdma.stOutUoRdma.stOutUoReg.UoAxiId.value = ((rand() & 0xFF) << 8);
        stIspOutUoRdma.u8IspIdx = 0;
        ret = STFDRV_ISP_OUT_UO_SetReg(
            pstCiConnection,
            &stIspOutUoRdma
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_OUT_UO_SetReg()\n",
            stIspOutUoRdma.u8IspIdx);
#if defined(V4L2_DRIVER)
        //LOG_INFO("Uo - Stride =  = 0x%04X\n",
        //    stIspOutUoRdma.stOutUoRdma.stOutUoReg.UoStride.value
        //    );
#else
        LOG_INFO("Uo - YAddr = 0x%08X, UvAddr = 0x%08X, Stride =  = 0x%04X\n",
            stIspOutUoRdma.stOutUoRdma.stOutUoReg.UoYAddr.value,
            stIspOutUoRdma.stOutUoRdma.stOutUoReg.UoUvAddr.value,
            stIspOutUoRdma.stOutUoRdma.stOutUoReg.UoStride.value
            );
#endif //#if defined(V4L2_DRIVER)
        LOG_INFO("Uo - UvMode = 0x%01X(%s), AxiId = 0x%02X\n",
            stIspOutUoRdma.stOutUoRdma.stOutUoReg.UoUvMode.field.nv21,
            ((stIspOutUoRdma.stOutUoRdma.stOutUoReg.UoUvMode.field.nv21)
                ? ("NV21") : ("NV12")),
            stIspOutUoRdma.stOutUoRdma.stOutUoReg.UoAxiId.field.refwid
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
#if defined(V4L2_DRIVER)
    u32Offset = ADDR_REG_UOPGM;
#else
    u32Offset = ADDR_REG_UOAY;
#endif //#if defined(V4L2_DRIVER)
    u32Length = ST_OUT_UO_REG_LEN;
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
        LOG_INFO("Reg[0x%08X] = 0x%08X\n", u32Offset+i*4, u32Buffer[i]);
    }
    LOG_INFO("---------------------------------\n");
    //=========================================================================

    return ret;
}

#if defined(EXTEND_IOCTL_COMMAND_OUT_UO)
//-----------------------------------------------------------------------------

#endif //#if defined(EXTEND_IOCTL_COMMAND_OUT_UO)
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
STF_RESULT Test_OUT_SS0_SetReg(
    STF_VOID
    )
{
    int i;
    //STF_U8 u8Method;
    STF_U32 u32Offset;
    STF_U32 u32Length;
    STF_U32 u32Buffer[ST_OUT_SS0_REG_LEN];
    CI_CONNECTION *pstCiConnection = NULL;
    STF_RESULT ret = STF_SUCCESS;

    pstCiConnection = g_pstCiConnection;

    //=========================================================================
    u32Offset = ADDR_REG_UOPGM;
    ret = STFLIB_ISP_RegReadOne(
        TEST_ISP_DEVICE,
        u32Offset,
        (STF_U32 *)&u32Buffer[0]
        );
    if (ret) {
        return ret;
    }
    LOG_INFO("Reg[0x%08X] = 0x%08X\n", u32Offset, u32Buffer[0]);
#if defined(V4L2_DRIVER)
    u32Offset = ADDR_REG_SS0S;
#else
    u32Offset = ADDR_REG_SS0AY;
#endif //#if defined(V4L2_DRIVER)
    u32Length = ST_OUT_SS0_REG_LEN;
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
        LOG_INFO("Reg[0x%08X] = 0x%08X\n", u32Offset+i*4, u32Buffer[i]);
    }
    u32Offset = ADDR_REG_SSAXI;
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
        ST_ISP_OUT_SS0_RDMA stIspOutSs0Rdma;
        REG_SSAXI SsAxi = { 0 };

        ret = STFLIB_ISP_RegReadOne(
            TEST_ISP_DEVICE,
            ADDR_REG_SSAXI,
            (STF_U32 *)&SsAxi
            );
        if (ret) {
            return ret;
        }

        stIspOutSs0Rdma.stOutSs0Rdma.stRdmaStartCmd.Tag = MREG_WR;
        stIspOutSs0Rdma.stOutSs0Rdma.stRdmaStartCmd.RegAddr = ADDR_REG_UOPGM;
        stIspOutSs0Rdma.stOutSs0Rdma.stRdmaStartCmd.u32Param = 1;
        stIspOutSs0Rdma.stOutSs0Rdma.stRdmaOutSs0Cmd.Tag = MREG_WR;
#if defined(V4L2_DRIVER)
        stIspOutSs0Rdma.stOutSs0Rdma.stRdmaOutSs0Cmd.RegAddr = ADDR_REG_SS0S;
#else
        stIspOutSs0Rdma.stOutSs0Rdma.stRdmaOutSs0Cmd.RegAddr = ADDR_REG_SS0AY;
#endif //#if defined(V4L2_DRIVER)
        stIspOutSs0Rdma.stOutSs0Rdma.stRdmaOutSs0Cmd.u32Param =
            ST_OUT_SS0_REG_LEN;
        stIspOutSs0Rdma.stOutSs0Rdma.stRdmaSsxAxiCmd.Tag = MREG_WR;
        stIspOutSs0Rdma.stOutSs0Rdma.stRdmaSsxAxiCmd.RegAddr = ADDR_REG_SSAXI;
        stIspOutSs0Rdma.stOutSs0Rdma.stRdmaSsxAxiCmd.u32Param = 1;
        stIspOutSs0Rdma.stOutSs0Rdma.stRdmaEndCmd.Tag = DESP_END;

        stIspOutSs0Rdma.stOutSs0Rdma.stUvMode.value = ((rand() & 0x01) << 5);
#if defined(V4L2_DRIVER)
#else
        stIspOutSs0Rdma.stOutSs0Rdma.stOutSs0Reg.Ss0YAddr.value =
            (rand() & 0xFFFFFFF8);
        stIspOutSs0Rdma.stOutSs0Rdma.stOutSs0Reg.Ss0UvAddr.value =
            (rand() & 0xFFFFFFF8);
#endif //#if defined(V4L2_DRIVER)
        stIspOutSs0Rdma.stOutSs0Rdma.stOutSs0Reg.Ss0Stride.value =
            (rand() & 0x00001FF8);
        stIspOutSs0Rdma.stOutSs0Rdma.stOutSs0Reg.Ss0HSF.value = (rand() & 0x02)
            | ((rand() & 0xFFF) << 16);
        stIspOutSs0Rdma.stOutSs0Rdma.stOutSs0Reg.Ss0VSF.value = (rand() & 0x02)
            | ((rand() & 0xFFF) << 16);
        stIspOutSs0Rdma.stOutSs0Rdma.stOutSs0Reg.Ss0Size.value =
            (rand() & 0x1FFF) | ((rand() & 0x1FFF) << 16);
        stIspOutSs0Rdma.stOutSs0Rdma.stSsAxiId.value = 0;
        stIspOutSs0Rdma.stOutSs0Rdma.stSsAxiId.field.ss0wid = (rand() & 0xFF);
        stIspOutSs0Rdma.stOutSs0Rdma.stSsAxiId.field.ss1wid =
            SsAxi.field.ss1wid;
        stIspOutSs0Rdma.u8IspIdx = 0;
        ret = STFDRV_ISP_OUT_SS0_SetReg(
            pstCiConnection,
            &stIspOutSs0Rdma
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_OUT_SS0_SetReg()\n",
            stIspOutSs0Rdma.u8IspIdx);
        LOG_INFO("Ss0 - UvMode = 0x%01X(%s)\n",
            stIspOutSs0Rdma.stOutSs0Rdma.stUvMode.field.nv21,
            ((stIspOutSs0Rdma.stOutSs0Rdma.stUvMode.field.nv21)
                ? ("NV21") : ("NV12"))
            );
#if defined(V4L2_DRIVER)
        LOG_INFO("Ss0 - Stride =  = 0x%04X\n",
            stIspOutSs0Rdma.stOutSs0Rdma.stOutSs0Reg.Ss0Stride.value
            );
#else
        LOG_INFO("Ss0 - YAddr = 0x%08X, UvAddr = 0x%08X, Stride =  = 0x%04X\n",
            stIspOutSs0Rdma.stOutSs0Rdma.stOutSs0Reg.Ss0YAddr.value,
            stIspOutSs0Rdma.stOutSs0Rdma.stOutSs0Reg.Ss0UvAddr.value,
            stIspOutSs0Rdma.stOutSs0Rdma.stOutSs0Reg.Ss0Stride.value
            );
#endif //#if defined(V4L2_DRIVER)
        LOG_INFO("Ss0 - Horizontal Scaling - Mode = 0x%01X(%s), "\
            "Factor = 0x%04X\n",
            stIspOutSs0Rdma.stOutSs0Rdma.stOutSs0Reg.Ss0HSF.field.sm,
            ((0 == stIspOutSs0Rdma.stOutSs0Rdma.stOutSs0Reg.Ss0HSF.field.sm)
                ? ("Scale Down") : ("No Scaling")),
            stIspOutSs0Rdma.stOutSs0Rdma.stOutSs0Reg.Ss0HSF.field.sf
            );
        LOG_INFO("Ss0 - Vertical Scaling - Mode = 0x%01X(%s), "\
            "Factor = 0x%04X\n",
            stIspOutSs0Rdma.stOutSs0Rdma.stOutSs0Reg.Ss0VSF.field.sm,
            ((0 == stIspOutSs0Rdma.stOutSs0Rdma.stOutSs0Reg.Ss0VSF.field.sm)
                ? ("Scale Down") : ("No Scaling")),
            stIspOutSs0Rdma.stOutSs0Rdma.stOutSs0Reg.Ss0VSF.field.sf
            );
        LOG_INFO("Ss0 - Output size - Width = 0x%04X(%d), "\
            "Height = 0x%04X(%d)\n",
            stIspOutSs0Rdma.stOutSs0Rdma.stOutSs0Reg.Ss0Size.field.w,
            stIspOutSs0Rdma.stOutSs0Rdma.stOutSs0Reg.Ss0Size.field.w,
            stIspOutSs0Rdma.stOutSs0Rdma.stOutSs0Reg.Ss0Size.field.h,
            stIspOutSs0Rdma.stOutSs0Rdma.stOutSs0Reg.Ss0Size.field.h
            );
        LOG_INFO("Ss0 - AxiId = 0x%02X\n",
            stIspOutSs0Rdma.stOutSs0Rdma.stSsAxiId.field.ss0wid
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_UOPGM;
    ret = STFLIB_ISP_RegReadOne(
        TEST_ISP_DEVICE,
        u32Offset,
        (STF_U32 *)&u32Buffer[0]
        );
    if (ret) {
        return ret;
    }
    LOG_INFO("Reg[0x%08X] = 0x%08X\n", u32Offset, u32Buffer[0]);
#if defined(V4L2_DRIVER)
    u32Offset = ADDR_REG_SS0S;
#else
    u32Offset = ADDR_REG_SS0AY;
#endif //#if defined(V4L2_DRIVER)
    u32Length = ST_OUT_SS0_REG_LEN;
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
        LOG_INFO("Reg[0x%08X] = 0x%08X\n", u32Offset+i*4, u32Buffer[i]);
    }
    u32Offset = ADDR_REG_SSAXI;
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

#if defined(EXTEND_IOCTL_COMMAND_OUT_SS0)
//-----------------------------------------------------------------------------

#endif //#if defined(EXTEND_IOCTL_COMMAND_OUT_SS0)
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
STF_RESULT Test_OUT_SS1_SetReg(
    STF_VOID
    )
{
    int i;
    //STF_U8 u8Method;
    STF_U32 u32Offset;
    STF_U32 u32Length;
    STF_U32 u32Buffer[ST_OUT_SS1_REG_LEN];
    CI_CONNECTION *pstCiConnection = NULL;
    STF_RESULT ret = STF_SUCCESS;

    pstCiConnection = g_pstCiConnection;

    //=========================================================================
    u32Offset = ADDR_REG_UOPGM;
    ret = STFLIB_ISP_RegReadOne(
        TEST_ISP_DEVICE,
        u32Offset,
        (STF_U32 *)&u32Buffer[0]
        );
    if (ret) {
        return ret;
    }
    LOG_INFO("Reg[0x%08X] = 0x%08X\n", u32Offset, u32Buffer[0]);
#if defined(V4L2_DRIVER)
    u32Offset = ADDR_REG_SS1S;
#else
    u32Offset = ADDR_REG_SS1AY;
#endif //#if defined(V4L2_DRIVER)
    u32Length = ST_OUT_SS1_REG_LEN;
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
        LOG_INFO("Reg[0x%08X] = 0x%08X\n", u32Offset+i*4, u32Buffer[i]);
    }
    u32Offset = ADDR_REG_SSAXI;
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
        ST_ISP_OUT_SS1_RDMA stIspOutSs1Rdma;
        REG_SSAXI SsAxi = { 0 };

        ret = STFLIB_ISP_RegReadOne(
            TEST_ISP_DEVICE,
            ADDR_REG_SSAXI,
            (STF_U32 *)&SsAxi
            );
        if (ret) {
            return ret;
        }

        stIspOutSs1Rdma.stOutSs1Rdma.stRdmaStartCmd.Tag = MREG_WR;
        stIspOutSs1Rdma.stOutSs1Rdma.stRdmaStartCmd.RegAddr = ADDR_REG_UOPGM;
        stIspOutSs1Rdma.stOutSs1Rdma.stRdmaStartCmd.u32Param = 1;
        stIspOutSs1Rdma.stOutSs1Rdma.stRdmaOutSs1Cmd.Tag = MREG_WR;
#if defined(V4L2_DRIVER)
        stIspOutSs1Rdma.stOutSs1Rdma.stRdmaOutSs1Cmd.RegAddr = ADDR_REG_SS1S;
#else
        stIspOutSs1Rdma.stOutSs1Rdma.stRdmaOutSs1Cmd.RegAddr = ADDR_REG_SS1AY;
#endif //#if defined(V4L2_DRIVER)
        stIspOutSs1Rdma.stOutSs1Rdma.stRdmaOutSs1Cmd.u32Param =
            ST_OUT_SS1_REG_LEN;
        stIspOutSs1Rdma.stOutSs1Rdma.stRdmaSsxAxiCmd.Tag = MREG_WR;
        stIspOutSs1Rdma.stOutSs1Rdma.stRdmaSsxAxiCmd.RegAddr = ADDR_REG_SSAXI;
        stIspOutSs1Rdma.stOutSs1Rdma.stRdmaSsxAxiCmd.u32Param = 1;
        stIspOutSs1Rdma.stOutSs1Rdma.stRdmaEndCmd.Tag = DESP_END;

        stIspOutSs1Rdma.stOutSs1Rdma.stUvMode.value = ((rand() & 0x01) << 5);
#if defined(V4L2_DRIVER)
#else
        stIspOutSs1Rdma.stOutSs1Rdma.stOutSs1Reg.Ss1YAddr.value =
            (rand() & 0xFFFFFFF8);
        stIspOutSs1Rdma.stOutSs1Rdma.stOutSs1Reg.Ss1UvAddr.value =
            (rand() & 0xFFFFFFF8);
#endif //#if defined(V4L2_DRIVER)
        stIspOutSs1Rdma.stOutSs1Rdma.stOutSs1Reg.Ss1Stride.value =
            (rand() & 0x00001FF8);
        stIspOutSs1Rdma.stOutSs1Rdma.stOutSs1Reg.Ss1HSF.value = (rand() & 0x02)
            | ((rand() & 0xFFF) << 16);
        stIspOutSs1Rdma.stOutSs1Rdma.stOutSs1Reg.Ss1VSF.value = (rand() & 0x02)
            | ((rand() & 0xFFF) << 16);
        stIspOutSs1Rdma.stOutSs1Rdma.stOutSs1Reg.Ss1Size.value =
            (rand() & 0x1FFF) | ((rand() & 0x1FFF) << 16);
        stIspOutSs1Rdma.stOutSs1Rdma.stSsAxiId.value = 0;
        stIspOutSs1Rdma.stOutSs1Rdma.stSsAxiId.field.ss0wid =
            SsAxi.field.ss0wid;
        stIspOutSs1Rdma.stOutSs1Rdma.stSsAxiId.field.ss1wid = (rand() & 0xFF);

        stIspOutSs1Rdma.u8IspIdx = 0;
        ret = STFDRV_ISP_OUT_SS1_SetReg(
            pstCiConnection,
            &stIspOutSs1Rdma
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_OUT_SS1_SetReg()\n",
            stIspOutSs1Rdma.u8IspIdx);
        LOG_INFO("Ss1 - UvMode = 0x%01X(%s)\n",
            stIspOutSs1Rdma.stOutSs1Rdma.stUvMode.field.nv21,
            ((stIspOutSs1Rdma.stOutSs1Rdma.stUvMode.field.nv21)
                ? ("NV21") : ("NV12"))
            );
#if defined(V4L2_DRIVER)
        LOG_INFO("Ss1 - Stride =  = 0x%04X\n",
            stIspOutSs1Rdma.stOutSs1Rdma.stOutSs1Reg.Ss1Stride.value
            );
#else
        LOG_INFO("Ss1 - YAddr = 0x%08X, UvAddr = 0x%08X, Stride =  = 0x%04X\n",
            stIspOutSs1Rdma.stOutSs1Rdma.stOutSs1Reg.Ss1YAddr.value,
            stIspOutSs1Rdma.stOutSs1Rdma.stOutSs1Reg.Ss1UvAddr.value,
            stIspOutSs1Rdma.stOutSs1Rdma.stOutSs1Reg.Ss1Stride.value
            );
#endif //#if defined(V4L2_DRIVER)
        LOG_INFO("Ss1 - Horizontal Scaling - Mode = 0x%01X(%s), "\
            "Factor = 0x%04X\n",
            stIspOutSs1Rdma.stOutSs1Rdma.stOutSs1Reg.Ss1HSF.field.sm,
            ((0 == stIspOutSs1Rdma.stOutSs1Rdma.stOutSs1Reg.Ss1HSF.field.sm)
                ? ("Scale Down") : ("No Scaling")),
            stIspOutSs1Rdma.stOutSs1Rdma.stOutSs1Reg.Ss1HSF.field.sf
            );
        LOG_INFO("Ss1 - Vertical Scaling - Mode = 0x%01X(%s), "\
            "Factor = 0x%04X\n",
            stIspOutSs1Rdma.stOutSs1Rdma.stOutSs1Reg.Ss1VSF.field.sm,
            ((0 == stIspOutSs1Rdma.stOutSs1Rdma.stOutSs1Reg.Ss1VSF.field.sm)
                ? ("Scale Down") : ("No Scaling")),
            stIspOutSs1Rdma.stOutSs1Rdma.stOutSs1Reg.Ss1VSF.field.sf
            );
        LOG_INFO("Ss1 - Output size - Width = 0x%04X(%d), "\
            "Height = 0x%04X(%d)\n",
            stIspOutSs1Rdma.stOutSs1Rdma.stOutSs1Reg.Ss1Size.field.w,
            stIspOutSs1Rdma.stOutSs1Rdma.stOutSs1Reg.Ss1Size.field.w,
            stIspOutSs1Rdma.stOutSs1Rdma.stOutSs1Reg.Ss1Size.field.h,
            stIspOutSs1Rdma.stOutSs1Rdma.stOutSs1Reg.Ss1Size.field.h
            );
        LOG_INFO("Ss1 - AxiId = 0x%02X\n",
            stIspOutSs1Rdma.stOutSs1Rdma.stSsAxiId.field.ss1wid
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_UOPGM;
    ret = STFLIB_ISP_RegReadOne(
        TEST_ISP_DEVICE,
        u32Offset,
        (STF_U32 *)&u32Buffer[0]
        );
    if (ret) {
        return ret;
    }
    LOG_INFO("Reg[0x%08X] = 0x%08X\n", u32Offset, u32Buffer[0]);
#if defined(V4L2_DRIVER)
    u32Offset = ADDR_REG_SS1S;
#else
    u32Offset = ADDR_REG_SS1AY;
#endif //#if defined(V4L2_DRIVER)
    u32Length = ST_OUT_SS1_REG_LEN;
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
        LOG_INFO("Reg[0x%08X] = 0x%08X\n", u32Offset+i*4, u32Buffer[i]);
    }
    u32Offset = ADDR_REG_SSAXI;
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

#if defined(EXTEND_IOCTL_COMMAND_OUT_SS1)
//-----------------------------------------------------------------------------

#endif //#if defined(EXTEND_IOCTL_COMMAND_OUT_SS1)
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
STF_RESULT ISP_Mod_Out_Test(
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
        ret = Test_OUT_SetReg(
            );
        LOG_INFO("Test OUT_SetReg command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the OUT module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------
    //=========================================================================
#if defined(EXTEND_IOCTL_COMMAND_OUT)

    //=========================================================================
    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_OUT_SetUoInfo(
            );
        LOG_INFO("Test OUT_SetUoInfo command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the OUT module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_OUT_SetSs0Info(
            );
        LOG_INFO("Test OUT_SetSs0Info command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the OUT module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_OUT_SetSs1Info(
            );
        LOG_INFO("Test OUT_SetSs1Info command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the OUT module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------
    //=========================================================================
#endif //#if defined(EXTEND_IOCTL_COMMAND_OUT)

    //=========================================================================
    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_OUT_UO_SetReg(
            );
        LOG_INFO("Test OUT_UO_SetReg command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the OUT_UO module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------
    //=========================================================================
#if defined(EXTEND_IOCTL_COMMAND_OUT_UO)

    //=========================================================================
    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------
    //=========================================================================
#endif //#if defined(EXTEND_IOCTL_COMMAND_OUT_UO)

    //=========================================================================
    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_OUT_SS0_SetReg(
            );
        LOG_INFO("Test OUT_SS0_SetReg command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the OUT_SS0 module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------
    //=========================================================================
#if defined(EXTEND_IOCTL_COMMAND_OUT_SS0)

    //=========================================================================
    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------
    //=========================================================================
#endif //#if defined(EXTEND_IOCTL_COMMAND_OUT_SS0)

    //=========================================================================
    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_OUT_SS1_SetReg(
            );
        LOG_INFO("Test OUT_SS1_SetReg command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the OUT_SS1 module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------
    //=========================================================================
#if defined(EXTEND_IOCTL_COMMAND_OUT_SS1)

    //=========================================================================
    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------
    //=========================================================================
#endif //#if defined(EXTEND_IOCTL_COMMAND_OUT_SS1)

    return ret;
}

//-----------------------------------------------------------------------------
