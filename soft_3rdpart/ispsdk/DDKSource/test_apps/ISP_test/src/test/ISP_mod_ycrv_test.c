/**
  ******************************************************************************
  * @file  ISP_mod_ycrv_test.c
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

#define LOG_TAG "Mod_YCrv_Test"
#include "stf_common/userlog.h"

#include "ISP_mod_ycrv_test.h"
#include "ISP_test.h"


/* ISPC YCRV driver test structure */


/* ISPC YCRV driver test variables */
static CI_CONNECTION *g_pstCiConnection = NULL;


/* ISPC YCRV driver test interface */
//-----------------------------------------------------------------------------
STF_RESULT Test_YCRV_SetReg(
    STF_VOID
    )
{
    int i;
    //STF_U8 u8Method;
    STF_U32 u32Offset;
    STF_U32 u32Length;
    STF_U32 u32Buffer[ST_YCRV_REG_LEN];
    CI_CONNECTION *pstCiConnection = NULL;
    STF_RESULT ret = STF_SUCCESS;

    pstCiConnection = g_pstCiConnection;

    //=========================================================================
    u32Offset = ADDR_REG_YCURVE_0;
    u32Length = ST_YCRV_REG_LEN;
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
        LOG_INFO("Reg[0x%08X] = 0x%08X\n", ADDR_REG_YCURVE_0+i*4, u32Buffer[i]);
    }
    LOG_INFO("---------------------------------\n");
    //-------------------------------------------------------------------------
    {
        //STF_U32 *pu32Temp = NULL;
        STF_U32 u32Idx;
        STF_U32 u32Cnt;
        ST_ISP_YCRV_RDMA stIspYCrvRdma;
        STF_CHAR szMessage[1024] = "";
        STF_CHAR szTemp[128] = "";

        stIspYCrvRdma.stYCrvRdma.stRdmaStartCmd.Tag = MREG_WR;
        stIspYCrvRdma.stYCrvRdma.stRdmaStartCmd.RegAddr = ADDR_REG_YCURVE_0;
        stIspYCrvRdma.stYCrvRdma.stRdmaStartCmd.u32Param = ST_YCRV_REG_LEN;
        stIspYCrvRdma.stYCrvRdma.stRdmaEndCmd.Tag = DESP_END;

        u32Cnt = YCRV_PNT;
        for (u32Idx = 0; u32Idx < u32Cnt; u32Idx++) {
            stIspYCrvRdma.stYCrvRdma.stYCrvReg.YCurve[u32Idx].value =
                (rand() & 0x3FF);
        }
        stIspYCrvRdma.u8IspIdx = 0;
        ret = STFDRV_ISP_YCRV_SetReg(
            pstCiConnection,
            &stIspYCrvRdma
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_YCRV_SetReg()\n", stIspYCrvRdma.u8IspIdx);
        szMessage[0] = '\0';
        u32Cnt = YCRV_PNT;
        for (u32Idx = 0; u32Idx < u32Cnt; u32Idx++) {
            if (0 == u32Idx) {
                sprintf(szTemp, "Y Curve - 0x%04X,",
                    stIspYCrvRdma.stYCrvRdma.stYCrvReg.YCurve[u32Idx].field.l_param
                    );
            } else if ((7 == (u32Idx % 8)) || ((u32Cnt - 1) == u32Idx)) {
                sprintf(szTemp, ", 0x%04X\n",
                    stIspYCrvRdma.stYCrvRdma.stYCrvReg.YCurve[u32Idx].field.l_param
                    );
            } else {
                sprintf(szTemp, ", 0x%04X",
                    stIspYCrvRdma.stYCrvRdma.stYCrvReg.YCurve[u32Idx].field.l_param
                    );
            }
            strcat(szMessage, szTemp);
        }
        LOG_INFO("%s", szMessage);
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_YCURVE_0;
    u32Length = ST_YCRV_REG_LEN;
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
        LOG_INFO("Reg[0x%08X] = 0x%08X\n", ADDR_REG_YCURVE_0+i*4, u32Buffer[i]);
    }
    LOG_INFO("---------------------------------\n");
    //=========================================================================

    return ret;
}

#if defined(EXTEND_IOCTL_COMMAND_YCRV)
//-----------------------------------------------------------------------------

#endif //#if defined(EXTEND_IOCTL_COMMAND_YCRV)
//-----------------------------------------------------------------------------
STF_RESULT ISP_Mod_YCrv_Test(
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
        ret = Test_YCRV_SetReg(
            );
        LOG_INFO("Test YCRV_SetReg command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the YCRV module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------
    //=========================================================================
#if defined(EXTEND_IOCTL_COMMAND_YCRV)

    //=========================================================================
    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------
    //=========================================================================
#endif //#if defined(EXTEND_IOCTL_COMMAND_YCRV)

    return ret;
}

//-----------------------------------------------------------------------------
