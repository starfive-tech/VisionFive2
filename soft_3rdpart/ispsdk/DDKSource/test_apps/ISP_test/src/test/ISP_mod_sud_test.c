/**
  ******************************************************************************
  * @file  ISP_mod_sud_test.c
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

#include <unistd.h>

#include "ci/ci_api_structs.h"
#include "ci/ci_api.h"

#include "ISPC/stflib_isp_base.h"

#define LOG_TAG "Mod_sud_Test"
#include "stf_common/userlog.h"

#include "ISP_mod_sud_test.h"
#include "ISP_test.h"


/* ISPC SUD driver test structure */


/* ISPC SUD driver test variables */
static CI_CONNECTION *g_pstCiConnection = NULL;


/* ISPC SUD driver test interface */
//-----------------------------------------------------------------------------
STF_RESULT Test_SUD_SetReg(
    STF_VOID
    )
{
    int i;
    //STF_U8 u8Method;
    //STF_U32 u32Offset;
    STF_U32 u32Length;
    STF_U32 u32Buffer[2 * 2];
    CI_CONNECTION *pstCiConnection = NULL;
    STF_RESULT ret = STF_SUCCESS;

    pstCiConnection = g_pstCiConnection;

    //=========================================================================
    u32Length = 2;
    for (i = 0; i < u32Length; i++) {
        if (0 == i) {
            u32Buffer[i * 2] = ADDR_REG_CSIINTS + (i * 4);
        } else {
            u32Buffer[i * 2] = ADDR_REG_IESHD + ((i - 1) * 4);
        }
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
        //STF_U32 *pu32Temp = NULL;
        //STF_U32 u32Idx;
        //STF_U32 u32Cnt;
        ST_ISP_SUD_CSI_RDMA stIspSudCsiRdma;
        ST_ISP_SUD_ISP_RDMA stIspSudIspRdma;

        stIspSudCsiRdma.stSudCsiRdma.stRdmaStartCmd.Tag = MREG_WR;
        stIspSudCsiRdma.stSudCsiRdma.stRdmaStartCmd.RegAddr =
            ADDR_REG_CSIINTS;
        stIspSudCsiRdma.stSudCsiRdma.stRdmaStartCmd.u32Param =
            ST_SUD_CSI_REG_LEN;
        stIspSudCsiRdma.stSudCsiRdma.stRdmaEndCmd.Tag = DESP_END;

        stIspSudCsiRdma.stSudCsiRdma.stSudCsiReg.CsiShadowCtrl.value =
            ((rand() & 0x01) << 16) | ((rand() & 0x01) << 17);
        stIspSudCsiRdma.u8IspIdx = 0;
        ret = STFDRV_ISP_SUD_CSI_SetReg(
            pstCiConnection,
            &stIspSudCsiRdma
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_SUD_CSI_SetReg()\n",
            stIspSudCsiRdma.u8IspIdx);
        LOG_INFO("CSI shadow control - Enable = 0x%01X(%s), "\
            "Mode = 0x%01X(%s)\n",
            stIspSudCsiRdma.stSudCsiRdma.stSudCsiReg.CsiShadowCtrl.field.ud,
            ((stIspSudCsiRdma.stSudCsiRdma.stSudCsiReg.CsiShadowCtrl.field.ud)
                ? ("Enable") : ("Disable")),
            stIspSudCsiRdma.stSudCsiRdma.stSudCsiReg.CsiShadowCtrl.field.udm,
            ((stIspSudCsiRdma.stSudCsiRdma.stSudCsiReg.CsiShadowCtrl.field.udm)
                ? ("Next VSync") : ("Immediately"))
            );
        stIspSudIspRdma.stSudIspRdma.stRdmaStartCmd.Tag = MREG_WR;
        stIspSudIspRdma.stSudIspRdma.stRdmaStartCmd.RegAddr =
            ADDR_REG_IESHD;
        stIspSudIspRdma.stSudIspRdma.stRdmaStartCmd.u32Param =
            ST_SUD_ISP_REG_LEN;
        stIspSudIspRdma.stSudIspRdma.stRdmaEndCmd.Tag = DESP_END;

        stIspSudIspRdma.stSudIspRdma.stSudIspReg.IspShadowCtrl.value =
            (rand() & 0x01) | ((rand() & 0x01) << 1);
        stIspSudIspRdma.u8IspIdx = 0;
        ret = STFDRV_ISP_SUD_ISP_SetReg(
            pstCiConnection,
            &stIspSudIspRdma
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_SUD_ISP_SetReg()\n",
            stIspSudIspRdma.u8IspIdx);
        LOG_INFO("ISP shadow control - Enable = 0x%01X(%s), "\
            "Mode = 0x%01X(%s)\n",
            stIspSudIspRdma.stSudIspRdma.stSudIspReg.IspShadowCtrl.field.ud,
            ((stIspSudIspRdma.stSudIspRdma.stSudIspReg.IspShadowCtrl.field.ud)
                ? ("Enable") : ("Disable")),
            stIspSudIspRdma.stSudIspRdma.stSudIspReg.IspShadowCtrl.field.udm,
            ((stIspSudIspRdma.stSudIspRdma.stSudIspReg.IspShadowCtrl.field.udm)
                ? ("Next VSync") : ("Immediately"))
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Length = 2;
    for (i = 0; i < u32Length; i++) {
        if (0 == i) {
            u32Buffer[i * 2] = ADDR_REG_CSIINTS + (i * 4);
        } else {
            u32Buffer[i * 2] = ADDR_REG_IESHD + ((i - 1) * 4);
        }
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

//-----------------------------------------------------------------------------
STF_RESULT Test_SUD_SetCsiReg(
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
        //STF_U32 *pu32Temp = NULL;
        //STF_U32 u32Idx;
        //STF_U32 u32Cnt;
        ST_ISP_SUD_CSI_RDMA stIspSudCsiRdma;

        stIspSudCsiRdma.stSudCsiRdma.stRdmaStartCmd.Tag = MREG_WR;
        stIspSudCsiRdma.stSudCsiRdma.stRdmaStartCmd.RegAddr =
            ADDR_REG_CSIINTS;
        stIspSudCsiRdma.stSudCsiRdma.stRdmaStartCmd.u32Param =
            ST_SUD_CSI_REG_LEN;
        stIspSudCsiRdma.stSudCsiRdma.stRdmaEndCmd.Tag = DESP_END;

        stIspSudCsiRdma.stSudCsiRdma.stSudCsiReg.CsiShadowCtrl.value =
            ((rand() & 0x01) << 16) | ((rand() & 0x01) << 17);
        stIspSudCsiRdma.u8IspIdx = 0;
        ret = STFDRV_ISP_SUD_CSI_SetReg(
            pstCiConnection,
            &stIspSudCsiRdma
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_SUD_CSI_SetReg()\n",
            stIspSudCsiRdma.u8IspIdx);
        LOG_INFO("CSI shadow control - Enable = 0x%01X(%s), "\
            "Mode = 0x%01X(%s)\n",
            stIspSudCsiRdma.stSudCsiRdma.stSudCsiReg.CsiShadowCtrl.field.ud,
            ((stIspSudCsiRdma.stSudCsiRdma.stSudCsiReg.CsiShadowCtrl.field.ud)
                ? ("Enable") : ("Disable")),
            stIspSudCsiRdma.stSudCsiRdma.stSudCsiReg.CsiShadowCtrl.field.udm,
            ((stIspSudCsiRdma.stSudCsiRdma.stSudCsiReg.CsiShadowCtrl.field.udm)
                ? ("Next VSync") : ("Immediately"))
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

STF_RESULT Test_SUD_SetCsiShadowEnable(
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
        //STF_U32 *pu32Temp = NULL;
        //STF_U32 u32Idx;
        //STF_U32 u32Cnt;
        ST_ISP_SUD_CSI_RDMA stIspSudCsiRdma;

        stIspSudCsiRdma.stSudCsiRdma.stSudCsiReg.CsiShadowCtrl.value =
            ((rand() & 0x01) << 16) | ((rand() & 0x01) << 17);
        stIspSudCsiRdma.u8IspIdx = 0;
        ret = STFDRV_ISP_SUD_CSI_SetShadowEnable(
            pstCiConnection,
            &stIspSudCsiRdma
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_SUD_CSI_SetShadowEnable()\n",
            stIspSudCsiRdma.u8IspIdx);
        LOG_INFO("CSI shadow control - Enable = 0x%01X(%s)\n",
            stIspSudCsiRdma.stSudCsiRdma.stSudCsiReg.CsiShadowCtrl.field.ud,
            ((stIspSudCsiRdma.stSudCsiRdma.stSudCsiReg.CsiShadowCtrl.field.ud)
                ? ("Enable") : ("Disable"))
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

#if defined(EXTEND_IOCTL_COMMAND_SUD_CSI)
// Since the shadow update enable bit cannot be read back,
// therefore disable below feature support.
#if defined(ENABLE_SET_SHADOW_MODE_SUPPORT)
STF_RESULT Test_SUD_SetCsiShadowMode(
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
        //STF_U32 *pu32Temp = NULL;
        //STF_U32 u32Idx;
        //STF_U32 u32Cnt;
        ST_ISP_SUD_CSI_RDMA stIspSudCsiRdma;

        stIspSudCsiRdma.stSudCsiRdma.stSudCsiReg.CsiShadowCtrl.value =
            ((rand() & 0x01) << 16) | ((rand() & 0x01) << 17);
        stIspSudCsiRdma.u8IspIdx = 0;
        ret = STFDRV_ISP_SUD_CSI_SetShadowMode(
            pstCiConnection,
            &stIspSudCsiRdma
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_SUD_CSI_SetShadowMode()\n",
            stIspSudCsiRdma.u8IspIdx);
        LOG_INFO("CSI shadow control -  Mode = 0x%01X(%s)\n",
            stIspSudCsiRdma.stSudCsiRdma.stSudCsiReg.CsiShadowCtrl.field.udm,
            ((stIspSudCsiRdma.stSudCsiRdma.stSudCsiReg.CsiShadowCtrl.field.udm)
                ? ("Next VSync") : ("Immediately"))
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

#endif //#if defined(ENABLE_SET_SHADOW_MODE_SUPPORT)
STF_RESULT Test_SUD_SetCsiShadowImmediatelyAndEnable(
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
        //STF_U32 *pu32Temp = NULL;
        //STF_U32 u32Idx;
        //STF_U32 u32Cnt;
        //ST_ISP_SUD_CSI_RDMA stIspSudCsiRdma;
        STF_U8 u8IspIdx;

        u8IspIdx = 0;
        ret = STFDRV_ISP_SUD_CSI_SetShadowImmediatelyAndEnable(
            pstCiConnection,
            u8IspIdx
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : "\
            "STFDRV_ISP_SUD_CSI_SetShadowImmediatelyAndEnable()\n", u8IspIdx);
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

STF_RESULT Test_SUD_SetCsiShadowNextVSyncAndEnable(
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
        //STF_U32 *pu32Temp = NULL;
        //STF_U32 u32Idx;
        //STF_U32 u32Cnt;
        //ST_ISP_SUD_CSI_RDMA stIspSudCsiRdma;
        STF_U8 u8IspIdx;

        u8IspIdx = 0;
        ret = STFDRV_ISP_SUD_CSI_SetShadowNextVSyncAndEnable(
            pstCiConnection,
            u8IspIdx
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : "\
            "STFDRV_ISP_SUD_CSI_SetShadowNextVSyncAndEnable()\n", u8IspIdx);
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

STF_RESULT Test_SUD_SetCsiShadowImmediatelyAndTrigger(
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
        //STF_U32 *pu32Temp = NULL;
        //STF_U32 u32Idx;
        //STF_U32 u32Cnt;
        //ST_ISP_SUD_CSI_RDMA stIspSudCsiRdma;
        STF_U8 u8IspIdx;

        u8IspIdx = 0;
        ret = STFDRV_ISP_SUD_CSI_SetShadowImmediatelyAndTrigger(
            pstCiConnection,
            u8IspIdx
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : "\
            "STFDRV_ISP_SUD_CSI_SetShadowImmediatelyAndTrigger()\n", u8IspIdx);
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

STF_RESULT Test_SUD_SetCsiShadowNextVSyncAndTrigger(
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
        //STF_U32 *pu32Temp = NULL;
        //STF_U32 u32Idx;
        //STF_U32 u32Cnt;
        //ST_ISP_SUD_CSI_RDMA stIspSudCsiRdma;
        STF_U8 u8IspIdx;

        u8IspIdx = 0;
        ret = STFDRV_ISP_SUD_CSI_SetShadowNextVSyncAndTrigger(
            pstCiConnection,
            u8IspIdx
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : "\
            "STFDRV_ISP_SUD_CSI_SetShadowNextVSyncAndTrigger()\n", u8IspIdx);
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

STF_RESULT Test_SUD_TriggerCsiShadow(
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
        //STF_U32 *pu32Temp = NULL;
        //STF_U32 u32Idx;
        //STF_U32 u32Cnt;
        //ST_ISP_SUD_CSI_RDMA stIspSudCsiRdma;
        STF_U8 u8IspIdx;

        u8IspIdx = 0;
        ret = STFDRV_ISP_SUD_CSI_TriggerShadow(
            pstCiConnection,
            u8IspIdx
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_SUD_CSI_TriggerShadow()\n", u8IspIdx);
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

#endif //#if defined(EXTEND_IOCTL_COMMAND_SUD_CSI)
STF_RESULT Test_SUD_SetIspReg(
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
    u32Offset = ADDR_REG_IESHD;
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
        //STF_U32 *pu32Temp = NULL;
        //STF_U32 u32Idx;
        //STF_U32 u32Cnt;
        ST_ISP_SUD_ISP_RDMA stIspSudIspRdma;

        stIspSudIspRdma.stSudIspRdma.stRdmaStartCmd.Tag = MREG_WR;
        stIspSudIspRdma.stSudIspRdma.stRdmaStartCmd.RegAddr =
            ADDR_REG_IESHD;
        stIspSudIspRdma.stSudIspRdma.stRdmaStartCmd.u32Param =
            ST_SUD_ISP_REG_LEN;
        stIspSudIspRdma.stSudIspRdma.stRdmaEndCmd.Tag = DESP_END;

        stIspSudIspRdma.stSudIspRdma.stSudIspReg.IspShadowCtrl.value =
            (rand() & 0x01) | ((rand() & 0x01) << 1);
        stIspSudIspRdma.u8IspIdx = 0;
        ret = STFDRV_ISP_SUD_ISP_SetReg(
            pstCiConnection,
            &stIspSudIspRdma
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_SUD_ISP_SetReg()\n",
            stIspSudIspRdma.u8IspIdx);
        LOG_INFO("ISP shadow control - Enable = 0x%01X(%s), "\
            "Mode = 0x%01X(%s)\n",
            stIspSudIspRdma.stSudIspRdma.stSudIspReg.IspShadowCtrl.field.ud,
            ((stIspSudIspRdma.stSudIspRdma.stSudIspReg.IspShadowCtrl.field.ud)
                ? ("Enable") : ("Disable")),
            stIspSudIspRdma.stSudIspRdma.stSudIspReg.IspShadowCtrl.field.udm,
            ((stIspSudIspRdma.stSudIspRdma.stSudIspReg.IspShadowCtrl.field.udm)
                ? ("Next VSync") : ("Immediately"))
            );
        LOG_INFO("---------------------------------\n");
        usleep(2000000);
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_IESHD;
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

STF_RESULT Test_SUD_SetIspShadowEnable(
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
    u32Offset = ADDR_REG_IESHD;
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
        //STF_U32 *pu32Temp = NULL;
        //STF_U32 u32Idx;
        //STF_U32 u32Cnt;
        ST_ISP_SUD_ISP_RDMA stIspSudIspRdma;

        stIspSudIspRdma.stSudIspRdma.stSudIspReg.IspShadowCtrl.value =
            (rand() & 0x01) | ((rand() & 0x01) << 1);
        stIspSudIspRdma.u8IspIdx = 0;
        ret = STFDRV_ISP_SUD_ISP_SetShadowEnable(
            pstCiConnection,
            &stIspSudIspRdma
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_SUD_ISP_SetShadowEnable()\n",
            stIspSudIspRdma.u8IspIdx);
        LOG_INFO("ISP shadow control - Enable = 0x%01X(%s)\n",
            stIspSudIspRdma.stSudIspRdma.stSudIspReg.IspShadowCtrl.field.ud,
            ((stIspSudIspRdma.stSudIspRdma.stSudIspReg.IspShadowCtrl.field.ud)
                ? ("Enable") : ("Disable"))
            );
        LOG_INFO("---------------------------------\n");
        usleep(2000000);
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_IESHD;
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

#if defined(EXTEND_IOCTL_COMMAND_SUD_ISP)
// Since the shadow update enable bit cannot be read back,
// therefore disable below feature support.
#if defined(ENABLE_SET_SHADOW_MODE_SUPPORT)
STF_RESULT Test_SUD_SetIspShadowMode(
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
    u32Offset = ADDR_REG_IESHD;
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
        //STF_U32 *pu32Temp = NULL;
        //STF_U32 u32Idx;
        //STF_U32 u32Cnt;
        ST_ISP_SUD_ISP_RDMA stIspSudIspRdma;

        stIspSudIspRdma.stSudIspRdma.stSudIspReg.IspShadowCtrl.value =
            (rand() & 0x01) | ((rand() & 0x01) << 1);
        stIspSudIspRdma.u8IspIdx = 0;
        ret = STFDRV_ISP_SUD_ISP_SetShadowMode(
            pstCiConnection,
            &stIspSudIspRdma
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_SUD_ISP_SetShadowMode()\n",
            stIspSudIspRdma.u8IspIdx);
        LOG_INFO("ISP shadow control - Mode = 0x%01X(%s)\n",
            stIspSudIspRdma.stSudIspRdma.stSudIspReg.IspShadowCtrl.field.udm,
            ((stIspSudIspRdma.stSudIspRdma.stSudIspReg.IspShadowCtrl.field.udm)
                ? ("Next VSync") : ("Immediately"))
            );
        LOG_INFO("---------------------------------\n");
        usleep(2000000);
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_IESHD;
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

#endif //#if defined(ENABLE_SET_SHADOW_MODE_SUPPORT)
STF_RESULT Test_SUD_SetIspShadowImmediatelyAndEnable(
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
    u32Offset = ADDR_REG_IESHD;
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
        //STF_U32 *pu32Temp = NULL;
        //STF_U32 u32Idx;
        //STF_U32 u32Cnt;
        //ST_ISP_SUD_ISP_RDMA stIspSudIspRdma;
        STF_U8 u8IspIdx;

        u8IspIdx = 0;
        ret = STFDRV_ISP_SUD_ISP_SetShadowImmediatelyAndEnable(
            pstCiConnection,
            u8IspIdx
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : "\
            "STFDRV_ISP_SUD_ISP_SetShadowImmediatelyAndEnable()\n", u8IspIdx);
        LOG_INFO("---------------------------------\n");
        usleep(2000000);
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_IESHD;
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

STF_RESULT Test_SUD_SetIspShadowNextVSyncAndEnable(
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
    u32Offset = ADDR_REG_IESHD;
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
        //STF_U32 *pu32Temp = NULL;
        //STF_U32 u32Idx;
        //STF_U32 u32Cnt;
        //ST_ISP_SUD_ISP_RDMA stIspSudIspRdma;
        STF_U8 u8IspIdx;

        u8IspIdx = 0;
        ret = STFDRV_ISP_SUD_ISP_SetShadowNextVSyncAndEnable(
            pstCiConnection,
            u8IspIdx
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : "\
            "STFDRV_ISP_SUD_ISP_SetShadowNextVSyncAndEnable()\n", u8IspIdx);
        LOG_INFO("---------------------------------\n");
        usleep(2000000);
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_IESHD;
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

STF_RESULT Test_SUD_SetIspShadowImmediatelyAndTrigger(
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
    u32Offset = ADDR_REG_IESHD;
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
        //STF_U32 *pu32Temp = NULL;
        //STF_U32 u32Idx;
        //STF_U32 u32Cnt;
        //ST_ISP_SUD_ISP_RDMA stIspSudIspRdma;
        STF_U8 u8IspIdx;

        u8IspIdx = 0;
        ret = STFDRV_ISP_SUD_ISP_SetShadowImmediatelyAndTrigger(
            pstCiConnection,
            u8IspIdx
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : "\
            "STFDRV_ISP_SUD_ISP_SetShadowImmediatelyAndTrigger()\n", u8IspIdx);
        LOG_INFO("---------------------------------\n");
        usleep(2000000);
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_IESHD;
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

STF_RESULT Test_SUD_SetIspShadowNextVSyncAndTrigger(
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
    u32Offset = ADDR_REG_IESHD;
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
        //STF_U32 *pu32Temp = NULL;
        //STF_U32 u32Idx;
        //STF_U32 u32Cnt;
        //ST_ISP_SUD_ISP_RDMA stIspSudIspRdma;
        STF_U8 u8IspIdx;

        u8IspIdx = 0;
        ret = STFDRV_ISP_SUD_ISP_SetShadowNextVSyncAndTrigger(
            pstCiConnection,
            u8IspIdx
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : "\
            "STFDRV_ISP_SUD_ISP_SetShadowNextVSyncAndTrigger()\n", u8IspIdx);
        LOG_INFO("---------------------------------\n");
        usleep(2000000);
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_IESHD;
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

STF_RESULT Test_SUD_TriggerIspShadow(
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
    u32Offset = ADDR_REG_IESHD;
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
        //STF_U32 *pu32Temp = NULL;
        //STF_U32 u32Idx;
        //STF_U32 u32Cnt;
        //ST_ISP_SUD_ISP_RDMA stIspSudIspRdma;
        STF_U8 u8IspIdx;

        u8IspIdx = 0;
        ret = STFDRV_ISP_SUD_ISP_TriggerShadow(
            pstCiConnection,
            u8IspIdx
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_SUD_ISP_TriggerShadow()\n", u8IspIdx);
        LOG_INFO("---------------------------------\n");
        usleep(2000000);
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_IESHD;
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

#endif //#if defined(EXTEND_IOCTL_COMMAND_SUD_ISP)
STF_RESULT Test_SUD_IspShadowEnableBitTest(
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
    u32Offset = ADDR_REG_IESHD;
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
    for (i = 0; i < 4; i++) {
        //---------------------------------------------------------------------
        //STF_U32 *pu32Temp = NULL;
        //STF_U32 u32Idx;
        //STF_U32 u32Cnt;
        ST_ISP_SUD_ISP_RDMA stIspSudIspRdma;

        stIspSudIspRdma.stSudIspRdma.stSudIspReg.IspShadowCtrl.field.ud =
            STF_DISABLE;
        stIspSudIspRdma.stSudIspRdma.stSudIspReg.IspShadowCtrl.field.udm =
            (rand() & 0x01);  // Don't care this bit result.
        stIspSudIspRdma.u8IspIdx = 0;
        ret = STFDRV_ISP_SUD_ISP_SetShadowEnable(
            pstCiConnection,
            &stIspSudIspRdma
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_SUD_ISP_SetShadowEnable()\n",
            stIspSudIspRdma.u8IspIdx);
        LOG_INFO("ISP shadow control - Enable = 0x%01X(%s)\n",
            stIspSudIspRdma.stSudIspRdma.stSudIspReg.IspShadowCtrl.field.ud,
            ((stIspSudIspRdma.stSudIspRdma.stSudIspReg.IspShadowCtrl.field.ud)
                ? ("Enable") : ("Disable"))
            );
        LOG_INFO("---------------------------------\n");
        usleep(2000000);
        //---------------------------------------------------------------------
        u32Offset = ADDR_REG_IESHD;
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
        //---------------------------------------------------------------------
    }
    //-------------------------------------------------------------------------
    for (i = 0; i < 4; i++) {
        //---------------------------------------------------------------------
        //STF_U32 *pu32Temp = NULL;
        //STF_U32 u32Idx;
        //STF_U32 u32Cnt;
        ST_ISP_SUD_ISP_RDMA stIspSudIspRdma;

        stIspSudIspRdma.stSudIspRdma.stSudIspReg.IspShadowCtrl.field.ud =
            STF_ENABLE;
        stIspSudIspRdma.stSudIspRdma.stSudIspReg.IspShadowCtrl.field.udm =
            (rand() & 0x01);  // Don't care this bit result.
        stIspSudIspRdma.u8IspIdx = 0;
        ret = STFDRV_ISP_SUD_ISP_SetShadowEnable(
            pstCiConnection,
            &stIspSudIspRdma
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_SUD_ISP_SetShadowEnable()\n",
            stIspSudIspRdma.u8IspIdx);
        LOG_INFO("ISP shadow control - Enable = 0x%01X(%s)\n",
            stIspSudIspRdma.stSudIspRdma.stSudIspReg.IspShadowCtrl.field.ud,
            ((stIspSudIspRdma.stSudIspRdma.stSudIspReg.IspShadowCtrl.field.ud)
                ? ("Enable") : ("Disable"))
            );
        LOG_INFO("---------------------------------\n");
        usleep(2000000);
        //---------------------------------------------------------------------
        u32Offset = ADDR_REG_IESHD;
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
        //---------------------------------------------------------------------
    }
    //-------------------------------------------------------------------------
    {
        //STF_U32 *pu32Temp = NULL;
        //STF_U32 u32Idx;
        //STF_U32 u32Cnt;
        ST_ISP_SUD_ISP_RDMA stIspSudIspRdma;

        stIspSudIspRdma.stSudIspRdma.stSudIspReg.IspShadowCtrl.field.ud =
            STF_ENABLE;
        stIspSudIspRdma.stSudIspRdma.stSudIspReg.IspShadowCtrl.field.udm =
            (rand() & 0x01);  // Don't care this bit result.
        stIspSudIspRdma.u8IspIdx = 0;
        ret = STFDRV_ISP_SUD_ISP_SetShadowEnable(
            pstCiConnection,
            &stIspSudIspRdma
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_SUD_ISP_SetShadowEnable()\n",
            stIspSudIspRdma.u8IspIdx);
        LOG_INFO("ISP shadow control - Enable = 0x%01X(%s)\n",
            stIspSudIspRdma.stSudIspRdma.stSudIspReg.IspShadowCtrl.field.ud,
            ((stIspSudIspRdma.stSudIspRdma.stSudIspReg.IspShadowCtrl.field.ud)
                ? ("Enable") : ("Disable"))
            );
        LOG_INFO("---------------------------------\n");
        usleep(2000000);
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_IESHD;
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
        //STF_U32 *pu32Temp = NULL;
        //STF_U32 u32Idx;
        //STF_U32 u32Cnt;
        ST_ISP_SUD_ISP_RDMA stIspSudIspRdma;

        stIspSudIspRdma.stSudIspRdma.stSudIspReg.IspShadowCtrl.field.ud =
            STF_DISABLE;
        stIspSudIspRdma.stSudIspRdma.stSudIspReg.IspShadowCtrl.field.udm =
            (rand() & 0x01);  // Don't care this bit result.
        stIspSudIspRdma.u8IspIdx = 0;
        ret = STFDRV_ISP_SUD_ISP_SetShadowEnable(
            pstCiConnection,
            &stIspSudIspRdma
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_SUD_ISP_SetShadowEnable()\n",
            stIspSudIspRdma.u8IspIdx);
        LOG_INFO("ISP shadow control - Enable = 0x%01X(%s)\n",
            stIspSudIspRdma.stSudIspRdma.stSudIspReg.IspShadowCtrl.field.ud,
            ((stIspSudIspRdma.stSudIspRdma.stSudIspReg.IspShadowCtrl.field.ud)
                ? ("Enable") : ("Disable"))
            );
        LOG_INFO("---------------------------------\n");
        usleep(2000000);
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_IESHD;
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
        //STF_U32 *pu32Temp = NULL;
        //STF_U32 u32Idx;
        //STF_U32 u32Cnt;
        ST_ISP_SUD_ISP_RDMA stIspSudIspRdma;

        stIspSudIspRdma.stSudIspRdma.stSudIspReg.IspShadowCtrl.field.ud =
            STF_DISABLE;
        stIspSudIspRdma.stSudIspRdma.stSudIspReg.IspShadowCtrl.field.udm =
            (rand() & 0x01);  // Don't care this bit result.
        stIspSudIspRdma.u8IspIdx = 0;
        ret = STFDRV_ISP_SUD_ISP_SetShadowEnable(
            pstCiConnection,
            &stIspSudIspRdma
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_SUD_ISP_SetShadowEnable()\n",
            stIspSudIspRdma.u8IspIdx);
        LOG_INFO("ISP shadow control - Enable = 0x%01X(%s)\n",
            stIspSudIspRdma.stSudIspRdma.stSudIspReg.IspShadowCtrl.field.ud,
            ((stIspSudIspRdma.stSudIspRdma.stSudIspReg.IspShadowCtrl.field.ud)
                ? ("Enable") : ("Disable"))
            );
        LOG_INFO("---------------------------------\n");
        usleep(2000000);
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_IESHD;
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
        //STF_U32 *pu32Temp = NULL;
        //STF_U32 u32Idx;
        //STF_U32 u32Cnt;
        ST_ISP_SUD_ISP_RDMA stIspSudIspRdma;

        stIspSudIspRdma.stSudIspRdma.stSudIspReg.IspShadowCtrl.field.ud =
            STF_ENABLE;
        stIspSudIspRdma.stSudIspRdma.stSudIspReg.IspShadowCtrl.field.udm =
            (rand() & 0x01);  // Don't care this bit result.
        stIspSudIspRdma.u8IspIdx = 0;
        ret = STFDRV_ISP_SUD_ISP_SetShadowEnable(
            pstCiConnection,
            &stIspSudIspRdma
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_SUD_ISP_SetShadowEnable()\n",
            stIspSudIspRdma.u8IspIdx);
        LOG_INFO("ISP shadow control - Enable = 0x%01X(%s)\n",
            stIspSudIspRdma.stSudIspRdma.stSudIspReg.IspShadowCtrl.field.ud,
            ((stIspSudIspRdma.stSudIspRdma.stSudIspReg.IspShadowCtrl.field.ud)
                ? ("Enable") : ("Disable"))
            );
        LOG_INFO("---------------------------------\n");
        usleep(2000000);
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_IESHD;
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
        //STF_U32 *pu32Temp = NULL;
        //STF_U32 u32Idx;
        //STF_U32 u32Cnt;
        ST_ISP_SUD_ISP_RDMA stIspSudIspRdma;

        stIspSudIspRdma.stSudIspRdma.stSudIspReg.IspShadowCtrl.field.ud =
            STF_ENABLE;
        stIspSudIspRdma.stSudIspRdma.stSudIspReg.IspShadowCtrl.field.udm =
            (rand() & 0x01);  // Don't care this bit result.
        stIspSudIspRdma.u8IspIdx = 0;
        ret = STFDRV_ISP_SUD_ISP_SetShadowEnable(
            pstCiConnection,
            &stIspSudIspRdma
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_SUD_ISP_SetShadowEnable()\n",
            stIspSudIspRdma.u8IspIdx);
        LOG_INFO("ISP shadow control - Enable = 0x%01X(%s)\n",
            stIspSudIspRdma.stSudIspRdma.stSudIspReg.IspShadowCtrl.field.ud,
            ((stIspSudIspRdma.stSudIspRdma.stSudIspReg.IspShadowCtrl.field.ud)
                ? ("Enable") : ("Disable"))
            );
        LOG_INFO("---------------------------------\n");
        usleep(2000000);
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_IESHD;
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
        //STF_U32 *pu32Temp = NULL;
        //STF_U32 u32Idx;
        //STF_U32 u32Cnt;
        ST_ISP_SUD_ISP_RDMA stIspSudIspRdma;

        stIspSudIspRdma.stSudIspRdma.stSudIspReg.IspShadowCtrl.field.ud =
            STF_DISABLE;
        stIspSudIspRdma.stSudIspRdma.stSudIspReg.IspShadowCtrl.field.udm =
            (rand() & 0x01);  // Don't care this bit result.
        stIspSudIspRdma.u8IspIdx = 0;
        ret = STFDRV_ISP_SUD_ISP_SetShadowEnable(
            pstCiConnection,
            &stIspSudIspRdma
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_SUD_ISP_SetShadowEnable()\n",
            stIspSudIspRdma.u8IspIdx);
        LOG_INFO("ISP shadow control - Enable = 0x%01X(%s)\n",
            stIspSudIspRdma.stSudIspRdma.stSudIspReg.IspShadowCtrl.field.ud,
            ((stIspSudIspRdma.stSudIspRdma.stSudIspReg.IspShadowCtrl.field.ud)
                ? ("Enable") : ("Disable"))
            );
        LOG_INFO("---------------------------------\n");
        usleep(2000000);
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_IESHD;
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
        //STF_U32 *pu32Temp = NULL;
        //STF_U32 u32Idx;
        //STF_U32 u32Cnt;
        ST_ISP_SUD_ISP_RDMA stIspSudIspRdma;

        stIspSudIspRdma.stSudIspRdma.stSudIspReg.IspShadowCtrl.field.ud =
            STF_DISABLE;
        stIspSudIspRdma.stSudIspRdma.stSudIspReg.IspShadowCtrl.field.udm =
            (rand() & 0x01);  // Don't care this bit result.
        stIspSudIspRdma.u8IspIdx = 0;
        ret = STFDRV_ISP_SUD_ISP_SetShadowEnable(
            pstCiConnection,
            &stIspSudIspRdma
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_SUD_ISP_SetShadowEnable()\n",
            stIspSudIspRdma.u8IspIdx);
        LOG_INFO("ISP shadow control - Enable = 0x%01X(%s)\n",
            stIspSudIspRdma.stSudIspRdma.stSudIspReg.IspShadowCtrl.field.ud,
            ((stIspSudIspRdma.stSudIspRdma.stSudIspReg.IspShadowCtrl.field.ud)
                ? ("Enable") : ("Disable"))
            );
        LOG_INFO("---------------------------------\n");
        usleep(2000000);
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_IESHD;
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
        //STF_U32 *pu32Temp = NULL;
        //STF_U32 u32Idx;
        //STF_U32 u32Cnt;
        ST_ISP_SUD_ISP_RDMA stIspSudIspRdma;

        stIspSudIspRdma.stSudIspRdma.stSudIspReg.IspShadowCtrl.field.ud =
            STF_DISABLE;
        stIspSudIspRdma.stSudIspRdma.stSudIspReg.IspShadowCtrl.field.udm =
            (rand() & 0x01);  // Don't care this bit result.
        stIspSudIspRdma.u8IspIdx = 0;
        ret = STFDRV_ISP_SUD_ISP_SetShadowEnable(
            pstCiConnection,
            &stIspSudIspRdma
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_SUD_ISP_SetShadowEnable()\n",
            stIspSudIspRdma.u8IspIdx);
        LOG_INFO("ISP shadow control - Enable = 0x%01X(%s)\n",
            stIspSudIspRdma.stSudIspRdma.stSudIspReg.IspShadowCtrl.field.ud,
            ((stIspSudIspRdma.stSudIspRdma.stSudIspReg.IspShadowCtrl.field.ud)
                ? ("Enable") : ("Disable"))
            );
        LOG_INFO("---------------------------------\n");
        usleep(2000000);
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_IESHD;
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
        //STF_U32 *pu32Temp = NULL;
        //STF_U32 u32Idx;
        //STF_U32 u32Cnt;
        ST_ISP_SUD_ISP_RDMA stIspSudIspRdma;

        stIspSudIspRdma.stSudIspRdma.stSudIspReg.IspShadowCtrl.field.ud =
            STF_ENABLE;
        stIspSudIspRdma.stSudIspRdma.stSudIspReg.IspShadowCtrl.field.udm =
            (rand() & 0x01);  // Don't care this bit result.
        stIspSudIspRdma.u8IspIdx = 0;
        ret = STFDRV_ISP_SUD_ISP_SetShadowEnable(
            pstCiConnection,
            &stIspSudIspRdma
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_SUD_ISP_SetShadowEnable()\n",
            stIspSudIspRdma.u8IspIdx);
        LOG_INFO("ISP shadow control - Enable = 0x%01X(%s)\n",
            stIspSudIspRdma.stSudIspRdma.stSudIspReg.IspShadowCtrl.field.ud,
            ((stIspSudIspRdma.stSudIspRdma.stSudIspReg.IspShadowCtrl.field.ud)
                ? ("Enable") : ("Disable"))
            );
        LOG_INFO("---------------------------------\n");
        usleep(2000000);
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_IESHD;
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
        //STF_U32 *pu32Temp = NULL;
        //STF_U32 u32Idx;
        //STF_U32 u32Cnt;
        ST_ISP_SUD_ISP_RDMA stIspSudIspRdma;

        stIspSudIspRdma.stSudIspRdma.stSudIspReg.IspShadowCtrl.field.ud =
            STF_DISABLE;
        stIspSudIspRdma.stSudIspRdma.stSudIspReg.IspShadowCtrl.field.udm =
            (rand() & 0x01);  // Don't care this bit result.
        stIspSudIspRdma.u8IspIdx = 0;
        ret = STFDRV_ISP_SUD_ISP_SetShadowEnable(
            pstCiConnection,
            &stIspSudIspRdma
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_SUD_ISP_SetShadowEnable()\n",
            stIspSudIspRdma.u8IspIdx);
        LOG_INFO("ISP shadow control - Enable = 0x%01X(%s)\n",
            stIspSudIspRdma.stSudIspRdma.stSudIspReg.IspShadowCtrl.field.ud,
            ((stIspSudIspRdma.stSudIspRdma.stSudIspReg.IspShadowCtrl.field.ud)
                ? ("Enable") : ("Disable"))
            );
        LOG_INFO("---------------------------------\n");
        usleep(2000000);
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_IESHD;
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
STF_RESULT ISP_Mod_Sud_Test(
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
        ret = Test_SUD_SetReg(
            );
        LOG_INFO("Test SUD_SetReg command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the SUD module command test.\n");
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
        ret = Test_SUD_SetCsiReg(
            );
        LOG_INFO("Test SUD_SetCsiReg command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the SUD module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_SUD_SetCsiShadowEnable(
            );
        LOG_INFO("Test SUD_SetCsiShadowEnable command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the SUD module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

#if defined(EXTEND_IOCTL_COMMAND_SUD_CSI)
    // Since the shadow update enable bit cannot be read back,
    // therefore disable below feature support.
#if defined(ENABLE_SET_SHADOW_MODE_SUPPORT)
    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_SUD_SetCsiShadowMode(
            );
        LOG_INFO("Test SUD_SetCsiShadowMode command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the SUD module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

#endif //#if defined(ENABLE_SET_SHADOW_MODE_SUPPORT)
    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_SUD_SetCsiShadowImmediatelyAndEnable(
            );
        LOG_INFO("Test SUD_SetCsiShadowImmediatelyAndEnable command is %s, "\
            "ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the SUD module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_SUD_SetCsiShadowNextVSyncAndEnable(
            );
        LOG_INFO("Test SUD_SetCsiShadowNextVSyncAndEnable command is %s, "\
            "ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the SUD module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_SUD_SetCsiShadowImmediatelyAndTrigger(
            );
        LOG_INFO("Test SUD_SetCsiShadowImmediatelyAndTrigger command is %s, "\
            "ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the SUD module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_SUD_SetCsiShadowNextVSyncAndTrigger(
            );
        LOG_INFO("Test SUD_SetCsiShadowNextVSyncAndTrigger command is %s, "\
            "ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the SUD module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_SUD_TriggerCsiShadow(
            );
        LOG_INFO("Test SUD_TriggerCsiShadow command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the SUD module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

#endif //#if defined(EXTEND_IOCTL_COMMAND_SUD_CSI)
    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_SUD_SetIspReg(
            );
        LOG_INFO("Test SUD_SetIspReg command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the SUD module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_SUD_SetIspShadowEnable(
            );
        LOG_INFO("Test SUD_SetIspShadowEnable command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the SUD module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

#if defined(EXTEND_IOCTL_COMMAND_SUD_ISP)
    // Since the shadow update enable bit cannot be read back,
    // therefore disable below feature support.
#if defined(ENABLE_SET_SHADOW_MODE_SUPPORT)
    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_SUD_SetIspShadowMode(
            );
        LOG_INFO("Test SUD_SetIspShadowMode command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the SUD module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

#endif //#if defined(ENABLE_SET_SHADOW_MODE_SUPPORT)
    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_SUD_SetIspShadowImmediatelyAndEnable(
            );
        LOG_INFO("Test SUD_SetIspShadowImmediatelyAndEnable command is %s, "\
            "ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the SUD module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_SUD_SetIspShadowNextVSyncAndEnable(
            );
        LOG_INFO("Test SUD_SetIspShadowNextVSyncAndEnable command is %s, "\
            "ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the SUD module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_SUD_SetIspShadowImmediatelyAndTrigger(
            );
        LOG_INFO("Test SUD_SetIspShadowImmediatelyAndTrigger command is %s, "\
            "ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the SUD module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_SUD_SetIspShadowNextVSyncAndTrigger(
            );
        LOG_INFO("Test SUD_SetIspShadowNextVSyncAndTrigger command is %s, "\
            "ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the SUD module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_SUD_TriggerIspShadow(
            );
        LOG_INFO("Test SUD_TriggerIspShadow command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the SUD module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

#endif //#if defined(EXTEND_IOCTL_COMMAND_SUD_ISP)
    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_SUD_IspShadowEnableBitTest(
            );
        LOG_INFO("Test Test_SUD_IspShadowEnableBitTest command is %s, "\
            "ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the SUD module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------
    //=========================================================================

    return ret;
}

//-----------------------------------------------------------------------------
