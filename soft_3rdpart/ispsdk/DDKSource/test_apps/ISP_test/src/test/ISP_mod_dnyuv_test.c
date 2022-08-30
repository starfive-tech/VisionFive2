/**
  ******************************************************************************
  * @file  ISP_mod_dnyuv_test.c
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

#define LOG_TAG "Mod_Dnyuv_Test"
#include "stf_common/userlog.h"

#include "ISP_mod_dnyuv_test.h"
#include "ISP_test.h"


/* ISPC DNYUV driver test structure */


/* ISPC DNYUV driver test variables */
static CI_CONNECTION *g_pstCiConnection = NULL;


/* ISPC DNYUV driver test interface */
//-----------------------------------------------------------------------------
STF_RESULT Test_DNYUV_SetReg(
    STF_VOID
    )
{
    int i;
    //STF_U8 u8Method;
    STF_U32 u32Offset;
    STF_U32 u32Length;
    STF_U32 u32Buffer[ST_DNYUV_REG_LEN];
    CI_CONNECTION *pstCiConnection = NULL;
    STF_RESULT ret = STF_SUCCESS;

    pstCiConnection = g_pstCiConnection;

    //=========================================================================
    u32Offset = ADDR_REG_DNYUV_YSWR0;
    u32Length = ST_DNYUV_REG_LEN;
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
        LOG_INFO("Reg[0x%08X] = 0x%08X\n",
            ADDR_REG_DNYUV_YSWR0+i*4, u32Buffer[i]);
    }
    LOG_INFO("---------------------------------\n");
    //-------------------------------------------------------------------------
    {
        STF_U32 *pu32Temp = NULL;
        STF_U32 u32Idx;
        STF_U32 u32Cnt;
        ST_ISP_DNYUV_RDMA stIspDnYuvRdma;

        stIspDnYuvRdma.stDnYuvRdma.stRdmaStartCmd.Tag = MREG_WR;
        stIspDnYuvRdma.stDnYuvRdma.stRdmaStartCmd.RegAddr =
            ADDR_REG_DNYUV_YSWR0;
        stIspDnYuvRdma.stDnYuvRdma.stRdmaStartCmd.u32Param = ST_DNYUV_REG_LEN;
        stIspDnYuvRdma.stDnYuvRdma.stRdmaEndCmd.Tag = DESP_END;

        stIspDnYuvRdma.stDnYuvRdma.stDnYuvReg.YSWeight0.value = 0;
        stIspDnYuvRdma.stDnYuvRdma.stDnYuvReg.YSWeight0.field.yswa =
            (rand() & 0x7);
        stIspDnYuvRdma.stDnYuvRdma.stDnYuvReg.YSWeight0.field.yswb =
            (rand() & 0x7);
        stIspDnYuvRdma.stDnYuvRdma.stDnYuvReg.YSWeight0.field.yswc =
            (rand() & 0x7);
        stIspDnYuvRdma.stDnYuvRdma.stDnYuvReg.YSWeight0.field.yswd =
            (rand() & 0x7);
        stIspDnYuvRdma.stDnYuvRdma.stDnYuvReg.YSWeight0.field.yswe =
            (rand() & 0x7);
        stIspDnYuvRdma.stDnYuvRdma.stDnYuvReg.YSWeight0.field.yswf =
            (rand() & 0x7);
        stIspDnYuvRdma.stDnYuvRdma.stDnYuvReg.YSWeight1.value = 0;
        stIspDnYuvRdma.stDnYuvRdma.stDnYuvReg.YSWeight1.field.yswg =
            (rand() & 0x7);
        stIspDnYuvRdma.stDnYuvRdma.stDnYuvReg.YSWeight1.field.yswh =
            (rand() & 0x7);
        stIspDnYuvRdma.stDnYuvRdma.stDnYuvReg.YSWeight1.field.yswi =
            (rand() & 0x7);
        stIspDnYuvRdma.stDnYuvRdma.stDnYuvReg.YSWeight1.field.yswj =
            (rand() & 0x7);
        stIspDnYuvRdma.stDnYuvRdma.stDnYuvReg.UvSWeight0.value = 0;
        stIspDnYuvRdma.stDnYuvRdma.stDnYuvReg.UvSWeight0.field.cswa =
            (rand() & 0x7);
        stIspDnYuvRdma.stDnYuvRdma.stDnYuvReg.UvSWeight0.field.cswb =
            (rand() & 0x7);
        stIspDnYuvRdma.stDnYuvRdma.stDnYuvReg.UvSWeight0.field.cswc =
            (rand() & 0x7);
        stIspDnYuvRdma.stDnYuvRdma.stDnYuvReg.UvSWeight0.field.cswd =
            (rand() & 0x7);
        stIspDnYuvRdma.stDnYuvRdma.stDnYuvReg.UvSWeight0.field.cswe =
            (rand() & 0x7);
        stIspDnYuvRdma.stDnYuvRdma.stDnYuvReg.UvSWeight0.field.cswf =
            (rand() & 0x7);
        stIspDnYuvRdma.stDnYuvRdma.stDnYuvReg.UvSWeight1.value = 0;
        stIspDnYuvRdma.stDnYuvRdma.stDnYuvReg.UvSWeight1.field.cswg =
            (rand() & 0x7);
        stIspDnYuvRdma.stDnYuvRdma.stDnYuvReg.UvSWeight1.field.cswh =
            (rand() & 0x7);
        stIspDnYuvRdma.stDnYuvRdma.stDnYuvReg.UvSWeight1.field.cswi =
            (rand() & 0x7);
        stIspDnYuvRdma.stDnYuvRdma.stDnYuvReg.UvSWeight1.field.cswj =
            (rand() & 0x7);
        u32Cnt = 3;
        pu32Temp = (STF_U32 *)&stIspDnYuvRdma.stDnYuvRdma.stDnYuvReg.YILevel;
        for (u32Idx = 0; u32Idx < u32Cnt; u32Idx++) {
            pu32Temp[u32Idx] = (rand() & 0x03FF) | ((rand() & 0x03FF) << 16);;
        }
        u32Cnt = 3;
        pu32Temp = (STF_U32 *)&stIspDnYuvRdma.stDnYuvRdma.stDnYuvReg.UvILevel;
        for (u32Idx = 0; u32Idx < u32Cnt; u32Idx++) {
            pu32Temp[u32Idx] = (rand() & 0x03FF) | ((rand() & 0x03FF) << 16);;
        }
        stIspDnYuvRdma.u8IspIdx = 0;
        ret = STFDRV_ISP_DNYUV_SetReg(
            pstCiConnection,
            &stIspDnYuvRdma
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_DNYUV_SetReg()\n",
            stIspDnYuvRdma.u8IspIdx);
        LOG_INFO("Y Channel weighting - 0x%01X(%d), 0x%01X(%d), "\
            "0x%01X(%d), 0x%01X(%d), 0x%01X(%d), 0x%01X(%d), \n"\
            "0x%01X(%d), 0x%01X(%d), 0x%01X(%d), 0x%01X(%d)\n",
            stIspDnYuvRdma.stDnYuvRdma.stDnYuvReg.YSWeight0.field.yswa,
            stIspDnYuvRdma.stDnYuvRdma.stDnYuvReg.YSWeight0.field.yswa,
            stIspDnYuvRdma.stDnYuvRdma.stDnYuvReg.YSWeight0.field.yswb,
            stIspDnYuvRdma.stDnYuvRdma.stDnYuvReg.YSWeight0.field.yswb,
            stIspDnYuvRdma.stDnYuvRdma.stDnYuvReg.YSWeight0.field.yswc,
            stIspDnYuvRdma.stDnYuvRdma.stDnYuvReg.YSWeight0.field.yswc,
            stIspDnYuvRdma.stDnYuvRdma.stDnYuvReg.YSWeight0.field.yswd,
            stIspDnYuvRdma.stDnYuvRdma.stDnYuvReg.YSWeight0.field.yswd,
            stIspDnYuvRdma.stDnYuvRdma.stDnYuvReg.YSWeight0.field.yswe,
            stIspDnYuvRdma.stDnYuvRdma.stDnYuvReg.YSWeight0.field.yswe,
            stIspDnYuvRdma.stDnYuvRdma.stDnYuvReg.YSWeight0.field.yswf,
            stIspDnYuvRdma.stDnYuvRdma.stDnYuvReg.YSWeight0.field.yswf,
            stIspDnYuvRdma.stDnYuvRdma.stDnYuvReg.YSWeight1.field.yswg,
            stIspDnYuvRdma.stDnYuvRdma.stDnYuvReg.YSWeight1.field.yswg,
            stIspDnYuvRdma.stDnYuvRdma.stDnYuvReg.YSWeight1.field.yswh,
            stIspDnYuvRdma.stDnYuvRdma.stDnYuvReg.YSWeight1.field.yswh,
            stIspDnYuvRdma.stDnYuvRdma.stDnYuvReg.YSWeight1.field.yswi,
            stIspDnYuvRdma.stDnYuvRdma.stDnYuvReg.YSWeight1.field.yswi,
            stIspDnYuvRdma.stDnYuvRdma.stDnYuvReg.YSWeight1.field.yswj,
            stIspDnYuvRdma.stDnYuvRdma.stDnYuvReg.YSWeight1.field.yswj
            );
        LOG_INFO("UV Channel weighting - 0x%01X(%d), 0x%01X(%d), "\
            "0x%01X(%d), 0x%01X(%d), 0x%01X(%d), 0x%01X(%d), \n"\
            "0x%01X(%d), 0x%01X(%d), 0x%01X(%d), 0x%01X(%d)\n",
            stIspDnYuvRdma.stDnYuvRdma.stDnYuvReg.UvSWeight0.field.cswa,
            stIspDnYuvRdma.stDnYuvRdma.stDnYuvReg.UvSWeight0.field.cswa,
            stIspDnYuvRdma.stDnYuvRdma.stDnYuvReg.UvSWeight0.field.cswb,
            stIspDnYuvRdma.stDnYuvRdma.stDnYuvReg.UvSWeight0.field.cswb,
            stIspDnYuvRdma.stDnYuvRdma.stDnYuvReg.UvSWeight0.field.cswc,
            stIspDnYuvRdma.stDnYuvRdma.stDnYuvReg.UvSWeight0.field.cswc,
            stIspDnYuvRdma.stDnYuvRdma.stDnYuvReg.UvSWeight0.field.cswd,
            stIspDnYuvRdma.stDnYuvRdma.stDnYuvReg.UvSWeight0.field.cswd,
            stIspDnYuvRdma.stDnYuvRdma.stDnYuvReg.UvSWeight0.field.cswe,
            stIspDnYuvRdma.stDnYuvRdma.stDnYuvReg.UvSWeight0.field.cswe,
            stIspDnYuvRdma.stDnYuvRdma.stDnYuvReg.UvSWeight0.field.cswf,
            stIspDnYuvRdma.stDnYuvRdma.stDnYuvReg.UvSWeight0.field.cswf,
            stIspDnYuvRdma.stDnYuvRdma.stDnYuvReg.UvSWeight1.field.cswg,
            stIspDnYuvRdma.stDnYuvRdma.stDnYuvReg.UvSWeight1.field.cswg,
            stIspDnYuvRdma.stDnYuvRdma.stDnYuvReg.UvSWeight1.field.cswh,
            stIspDnYuvRdma.stDnYuvRdma.stDnYuvReg.UvSWeight1.field.cswh,
            stIspDnYuvRdma.stDnYuvRdma.stDnYuvReg.UvSWeight1.field.cswi,
            stIspDnYuvRdma.stDnYuvRdma.stDnYuvReg.UvSWeight1.field.cswi,
            stIspDnYuvRdma.stDnYuvRdma.stDnYuvReg.UvSWeight1.field.cswj,
            stIspDnYuvRdma.stDnYuvRdma.stDnYuvReg.UvSWeight1.field.cswj
            );
        LOG_INFO("Y Channel Level - 0x%04X(%d), 0x%04X(%d), "\
            "0x%04X(%d), 0x%04X(%d), 0x%04X(%d), 0x%04X(%d)\n",
            stIspDnYuvRdma.stDnYuvRdma.stDnYuvReg.YILevel[0].field.yd0,
            stIspDnYuvRdma.stDnYuvRdma.stDnYuvReg.YILevel[0].field.yd0,
            stIspDnYuvRdma.stDnYuvRdma.stDnYuvReg.YILevel[0].field.yd1,
            stIspDnYuvRdma.stDnYuvRdma.stDnYuvReg.YILevel[0].field.yd1,
            stIspDnYuvRdma.stDnYuvRdma.stDnYuvReg.YILevel[1].field.yd0,
            stIspDnYuvRdma.stDnYuvRdma.stDnYuvReg.YILevel[1].field.yd0,
            stIspDnYuvRdma.stDnYuvRdma.stDnYuvReg.YILevel[1].field.yd1,
            stIspDnYuvRdma.stDnYuvRdma.stDnYuvReg.YILevel[1].field.yd1,
            stIspDnYuvRdma.stDnYuvRdma.stDnYuvReg.YILevel[2].field.yd0,
            stIspDnYuvRdma.stDnYuvRdma.stDnYuvReg.YILevel[2].field.yd0,
            stIspDnYuvRdma.stDnYuvRdma.stDnYuvReg.YILevel[2].field.yd1,
            stIspDnYuvRdma.stDnYuvRdma.stDnYuvReg.YILevel[2].field.yd1
            );
        LOG_INFO("UV Channel Level - 0x%04X(%d), 0x%04X(%d), "\
            "0x%04X(%d), 0x%04X(%d), 0x%04X(%d), 0x%04X(%d)\n",
            stIspDnYuvRdma.stDnYuvRdma.stDnYuvReg.UvILevel[0].field.cd0,
            stIspDnYuvRdma.stDnYuvRdma.stDnYuvReg.UvILevel[0].field.cd0,
            stIspDnYuvRdma.stDnYuvRdma.stDnYuvReg.UvILevel[0].field.cd1,
            stIspDnYuvRdma.stDnYuvRdma.stDnYuvReg.UvILevel[0].field.cd1,
            stIspDnYuvRdma.stDnYuvRdma.stDnYuvReg.UvILevel[1].field.cd0,
            stIspDnYuvRdma.stDnYuvRdma.stDnYuvReg.UvILevel[1].field.cd0,
            stIspDnYuvRdma.stDnYuvRdma.stDnYuvReg.UvILevel[1].field.cd1,
            stIspDnYuvRdma.stDnYuvRdma.stDnYuvReg.UvILevel[1].field.cd1,
            stIspDnYuvRdma.stDnYuvRdma.stDnYuvReg.UvILevel[2].field.cd0,
            stIspDnYuvRdma.stDnYuvRdma.stDnYuvReg.UvILevel[2].field.cd0,
            stIspDnYuvRdma.stDnYuvRdma.stDnYuvReg.UvILevel[2].field.cd1,
            stIspDnYuvRdma.stDnYuvRdma.stDnYuvReg.UvILevel[2].field.cd1
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_DNYUV_YSWR0;
    u32Length = ST_DNYUV_REG_LEN;
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
        LOG_INFO("Reg[0x%08X] = 0x%08X\n",
            ADDR_REG_DNYUV_YSWR0+i*4, u32Buffer[i]);
    }
    LOG_INFO("---------------------------------\n");
    //=========================================================================

    return ret;
}

#if defined(EXTEND_IOCTL_COMMAND_DNYUV)
//-----------------------------------------------------------------------------

#endif //#if defined(EXTEND_IOCTL_COMMAND_DNYUV)
//-----------------------------------------------------------------------------
STF_RESULT ISP_Mod_Dnyuv_Test(
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
        ret = Test_DNYUV_SetReg(
            );
        LOG_INFO("Test DNYUV_SetReg command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the DNYUV module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------
    //=========================================================================
#if defined(EXTEND_IOCTL_COMMAND_DNYUV)

    //=========================================================================
    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------
    //=========================================================================
#endif //#if defined(EXTEND_IOCTL_COMMAND_DNYUV)

    return ret;
}

//-----------------------------------------------------------------------------
