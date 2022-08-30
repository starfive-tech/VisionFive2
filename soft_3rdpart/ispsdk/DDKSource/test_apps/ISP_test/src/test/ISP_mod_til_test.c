/**
  ******************************************************************************
  * @file  ISP_mod_til_test.c
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

#define LOG_TAG "Mod_Til_Test"
#include "stf_common/userlog.h"

#include "ISP_mod_til_test.h"
#include "ISP_test.h"


/* ISPC TIL driver test structure */


/* ISPC TIL driver test variables */
static CI_CONNECTION *g_pstCiConnection = NULL;
static STF_CHAR g_szFormat[][64] = {
    "EN_TIL_FMT_RAW10_6PIX_1QW",        // 0 : RAW10 bit (6 pixels per 1QW) QW= 64bit word, msb 4bit no use.
    "EN_TIL_FMT_RAW12_5PIX_1QW",        // 1 : RAW12 bit (5 pixels per 1QW), msb 4bit no use.
    "EN_TIL_FMT_YUV420_SEMI",           // 2 : YUV semi-planar 8bit: Y plane, UV plane 4:2:0 JPEG/JFIF progressive mode using the averaging value of the 4 UV points between horizontal and vertical.
    "EN_TIL_FMT_RAW10_32PIX_5QW",       // 3 : Optional fully packed RAW10 (320 bits=32 pixels= 5QW), no redundant bit.
    "EN_TIL_FMT_RAW12_16PIX_3QW",       // 4 : Optional fully packed RAW12 (192 bits = 16 pixels = 3QW), no redundant bit.
    "EN_TIL_FMT_YUV420_SEMI_HOR_AVG",   // 5 : YUV semi-planar 8bit: Y plane, UV plane 4:2:0 mode using the averaging value of the 2 UV points between the horizontal direction.
    "EN_TIL_FMT_YUV420_SEMI_1ST_EVEN",  // 6 : YUV semi-planar 8bit: Y plane, UV plane 4:2:0 mode using the first position of the even horizontal with no averaging value.
    "EN_TIL_FMT_RAW_FLEX",              // 7 : RAW flexible bit whose bit number is less than 25 bits with no gap package. The bit number is specified in register 0x0B38 bit[24:20](Read) and bit[8:4](Write).
};
static STF_CHAR g_szRaw10Mode[][64] = {
    "EN_TIL_RAW_10_MODE_6PIX_1QW",      // 0 : RAW10 bit (6 pixels per 1QW) QW= 64bit word, msb 4bit no use.
    "EN_TIL_RAW_10_MODE_3PIX_1DW",      // 1 : RAW10bit format is changed to IMG ISP RAW 10 bit format (IMG 3 pack in a DW).
};
static STF_CHAR g_szTil_1_DataPath[][64] = {
    "EN_TIL_PATH_BF_DBC",               // 0 : RAW 10 bits, only for tiling read.
    "EN_TIL_PATH_BF_CFA",               // 1 : RAW 10 bits.
    "EN_TIL_PATH_BF_SHRP",              // 2 : YUV 10 bits.
    "EN_TIL_PATH_BF_SCALE_OUT",         // 3 : YUV 10 bits, only for tiling write.
};


/* ISPC TIL driver test interface */
//-----------------------------------------------------------------------------
STF_RESULT Test_TIL_SetReg(
    STF_VOID
    )
{
    int i;
    //STF_U8 u8Method;
    STF_U32 u32Offset;
    STF_U32 u32Length;
#if defined(V4L2_DRIVER)
    STF_U32 u32Buffer[ST_TIL_REG_LEN + 5];
#else
    STF_U32 u32Buffer[ST_TIL_REG_LEN];
#endif //#if defined(V4L2_DRIVER)
    CI_CONNECTION *pstCiConnection = NULL;
    STF_RESULT ret = STF_SUCCESS;

    pstCiConnection = g_pstCiConnection;

    //=========================================================================
    u32Offset = ADDR_REG_ITIIWSR;
#if defined(V4L2_DRIVER)
    u32Length = ST_TIL_REG_LEN + 5;
#else
    u32Length = ST_TIL_REG_LEN;
#endif //#if defined(V4L2_DRIVER)
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
        LOG_INFO("Reg[0x%08X] = 0x%08X\n", u32Offset + i * 4, u32Buffer[i]);
    }
    LOG_INFO("---------------------------------\n");
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_ITIPDFR;
    u32Buffer[0] = 0x00000000;
    ret = STFLIB_ISP_RegWriteOne(
        TEST_ISP_DEVICE,
        u32Offset,
        (STF_U32 *)&u32Buffer[0]
        );
    if (ret) {
        return ret;
    }
    LOG_INFO("Reg[0x%08X] <- 0x%08X\n", u32Offset, u32Buffer[0]);
    LOG_INFO("---------------------------------\n");
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_ITIPDFR;
    u32Buffer[0] = 0x00000000;
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
        ST_ISP_TIL_RDMA stIspTilRdma;

        stIspTilRdma.stTilRdma.stRdmaStartCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaStartCmd.RegAddr = ADDR_REG_ITIIWSR;
        stIspTilRdma.stTilRdma.stRdmaStartCmd.u32Param = ST_TIL_1_WS_REG_LEN;
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrSdCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrSdCmd.RegAddr = ADDR_REG_ITIDWLSR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrSdCmd.u32Param =
            ST_TIL_1_WR_SD_REG_LEN;
#if defined(V4L2_DRIVER)
#else
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrAddrCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrAddrCmd.RegAddr =
            ADDR_REG_ITIDWYSAR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrAddrCmd.u32Param =
            ST_TIL_1_WR_ADDR_REG_LEN;
#endif //#if defined(V4L2_DRIVER)
#if defined(V4L2_DRIVER)
#else
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdAddrCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdAddrCmd.RegAddr =
            ADDR_REG_ITIDRYSAR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdAddrCmd.u32Param =
            ST_TIL_1_RD_ADDR_REG_LEN;
#endif //#if defined(V4L2_DRIVER)
        stIspTilRdma.stTilRdma.stRdmaTil_1_PDFCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_PDFCmd.RegAddr = ADDR_REG_ITIPDFR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_PDFCmd.u32Param =
            ST_TIL_1_PDF_REG_LEN;
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdSdCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdSdCmd.RegAddr = ADDR_REG_ITIDRLSR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdSdCmd.u32Param =
            ST_TIL_1_RD_SD_REG_LEN;
#if defined(V4L2_DRIVER)
#else
        stIspTilRdma.stTilRdma.stRdmaTil_1_BSCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_BSCmd.RegAddr = ADDR_REG_ITIBSR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_BSCmd.u32Param = ST_TIL_1_BS_REG_LEN;
#endif //#if defined(V4L2_DRIVER)
        stIspTilRdma.stTilRdma.stRdmaTil_1_AxiCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_AxiCmd.RegAddr = ADDR_REG_ITIAIR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_AxiCmd.u32Param =
            ST_TIL_1_AXI_REG_LEN;
        stIspTilRdma.stTilRdma.stRdmaTil_1_DPSCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_DPSCmd.RegAddr = ADDR_REG_ITIDPSR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_DPSCmd.u32Param =
            ST_TIL_1_DPS_REG_LEN;
        stIspTilRdma.stTilRdma.stRdmaEndCmd.Tag = DESP_END;

        stIspTilRdma.stTilRdma.stTil_1_WSReg.Til_1_WindowSize.value =
            (rand() & 0xFFFF) | ((rand() & 0xFFFF) << 16);
#if defined(V4L2_DRIVER)
#else
        stIspTilRdma.stTilRdma.stTil_1_RdAddrReg.Til_1_ReadYAddr.value =
            (rand() & 0xFFFFFFF8);
        stIspTilRdma.stTilRdma.stTil_1_RdAddrReg.Til_1_ReadUvAddr.value =
            (rand() & 0xFFFFFFF8);
#endif //#if defined(V4L2_DRIVER)
        stIspTilRdma.stTilRdma.stTil_1_RdSdReg.Til_1_ReadStride.value =
            (rand() & 0x007FFFF8);
#if defined(V4L2_DRIVER)
#else
        stIspTilRdma.stTilRdma.stTil_1_WrAddrReg.Til_1_WriteYAddr.value =
            (rand() & 0xFFFFFFF8);
        stIspTilRdma.stTilRdma.stTil_1_WrAddrReg.Til_1_WriteUvAddr.value =
            (rand() & 0xFFFFFFF8);
#endif //#if defined(V4L2_DRIVER)
        stIspTilRdma.stTilRdma.stTil_1_WrSdReg.Til_1_WriteStride.value =
            (rand() & 0x007FFFF8);
        stIspTilRdma.stTilRdma.stTil_1_DPSReg.Til_1_PathSelect.value = 0;
        //stIspTilRdma.stTilRdma.stTil_1_DPSReg.Til_1_PathSelect.field.readindex =
        //    (rand() & 0xFF);
        stIspTilRdma.stTilRdma.stTil_1_DPSReg.Til_1_PathSelect.field.readindex =
            (rand() & 0x3);
        stIspTilRdma.stTilRdma.stTil_1_DPSReg.Til_1_PathSelect.field.rdnv21format =
            (rand() & 0x1);
        stIspTilRdma.stTilRdma.stTil_1_DPSReg.Til_1_PathSelect.field.independentrdrvi =
            (rand() & 0x1);
        stIspTilRdma.stTilRdma.stTil_1_DPSReg.Til_1_PathSelect.field.readdram =
            (rand() & 0x1);
        //stIspTilRdma.stTilRdma.stTil_1_DPSReg.Til_1_PathSelect.field.writeindex =
        //    (rand() & 0xFF);
        stIspTilRdma.stTilRdma.stTil_1_DPSReg.Til_1_PathSelect.field.writeindex =
            (rand() & 0x3);
        stIspTilRdma.stTilRdma.stTil_1_DPSReg.Til_1_PathSelect.field.wrnv21format =
            (rand() & 0x1);
        stIspTilRdma.stTilRdma.stTil_1_DPSReg.Til_1_PathSelect.field.writedram =
            (rand() & 0x1);
        stIspTilRdma.stTilRdma.stTil_1_DPSReg.Til_1_PathSelect.field.loopback =
            (rand() & 0x1);
        stIspTilRdma.stTilRdma.stTil_1_DPSReg.Til_1_PathSelect.field.movedramdata =
            (rand() & 0x1);
        stIspTilRdma.stTilRdma.stTil_1_PDFReg.Til_1_DataFormat.value = 0;
        stIspTilRdma.stTilRdma.stTil_1_PDFReg.Til_1_DataFormat.field.rdformat =
            (rand() & 0x7);
        stIspTilRdma.stTilRdma.stTil_1_PDFReg.Til_1_DataFormat.field.rdflexiblebit =
            (rand() & 0x1F);
        stIspTilRdma.stTilRdma.stTil_1_PDFReg.Til_1_DataFormat.field.rdlinebufoff =
            (rand() & 0x1);
        stIspTilRdma.stTilRdma.stTil_1_PDFReg.Til_1_DataFormat.field.wrformat =
            (rand() & 0x7);
        stIspTilRdma.stTilRdma.stTil_1_PDFReg.Til_1_DataFormat.field.wrflexiblebit =
            (rand() & 0x1F);
        stIspTilRdma.stTilRdma.stTil_1_PDFReg.Til_1_DataFormat.field.imgispraw10 =
            (rand() & 0x1);
        stIspTilRdma.stTilRdma.stTil_1_AxiReg.Til_1_AxiId.value = 0;
        stIspTilRdma.stTilRdma.stTil_1_AxiReg.Til_1_AxiId.field.yrid =
            (rand() & 0xFF);
        stIspTilRdma.stTilRdma.stTil_1_AxiReg.Til_1_AxiId.field.uvrid =
            (rand() & 0xFF);
        stIspTilRdma.stTilRdma.stTil_1_AxiReg.Til_1_AxiId.field.ywid =
            (rand() & 0xFF);
        stIspTilRdma.stTilRdma.stTil_1_AxiReg.Til_1_AxiId.field.uvwid =
            (rand() & 0xFF);
#if defined(V4L2_DRIVER)
#else
        stIspTilRdma.stTilRdma.stTil_1_BSReg.Til_1_BusyStatus.value = 0;
#endif //#if defined(V4L2_DRIVER)
        stIspTilRdma.u8IspIdx = 0;
        ret = STFDRV_ISP_TIL_SetReg(
            pstCiConnection,
            &stIspTilRdma
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_TIL_SetReg()\n", stIspTilRdma.u8IspIdx);
        LOG_INFO("------------ Tiling 1 -----------\n");
        LOG_INFO("Window size = 0x%08X - Width=0x%04X(%d), Height=0x%04X(%d)\n",
            stIspTilRdma.stTilRdma.stTil_1_WSReg.Til_1_WindowSize.value,
            stIspTilRdma.stTilRdma.stTil_1_WSReg.Til_1_WindowSize.field.wsize,
            stIspTilRdma.stTilRdma.stTil_1_WSReg.Til_1_WindowSize.field.wsize,
            stIspTilRdma.stTilRdma.stTil_1_WSReg.Til_1_WindowSize.field.hsize,
            stIspTilRdma.stTilRdma.stTil_1_WSReg.Til_1_WindowSize.field.hsize
            );
        LOG_INFO("Write - Stride = 0x%08X\n",
            stIspTilRdma.stTilRdma.stTil_1_WrSdReg.Til_1_WriteStride.value
            );
#if defined(V4L2_DRIVER)
#else
        LOG_INFO("Write - Y_Addr = 0x%08X, UV_Addr = 0x%08X\n",
            stIspTilRdma.stTilRdma.stTil_1_WrAddrReg.Til_1_WriteYAddr.value,
            stIspTilRdma.stTilRdma.stTil_1_WrAddrReg.Til_1_WriteUvAddr.value
            );
#endif //#if defined(V4L2_DRIVER)
#if defined(V4L2_DRIVER)
#else
        LOG_INFO("Read - Y_Addr = 0x%08X, UV_Addr = 0x%08X\n",
            stIspTilRdma.stTilRdma.stTil_1_RdAddrReg.Til_1_ReadYAddr.value,
            stIspTilRdma.stTilRdma.stTil_1_RdAddrReg.Til_1_ReadUvAddr.value
            );
#endif //#if defined(V4L2_DRIVER)
        LOG_INFO("DataFormat = 0x%08X\n",
            stIspTilRdma.stTilRdma.stTil_1_PDFReg.Til_1_DataFormat.value
            );
        LOG_INFO("DataFormat(Read) - Format = 0x%01X(%s), "\
            "FlexibleBit = 0x%02X, UvLineBuf = 0x%1X(%s)\n",
            stIspTilRdma.stTilRdma.stTil_1_PDFReg.Til_1_DataFormat.field.rdformat,
            g_szFormat[stIspTilRdma.stTilRdma.stTil_1_PDFReg.Til_1_DataFormat.field.rdformat],
            stIspTilRdma.stTilRdma.stTil_1_PDFReg.Til_1_DataFormat.field.rdflexiblebit,
            stIspTilRdma.stTilRdma.stTil_1_PDFReg.Til_1_DataFormat.field.rdlinebufoff,
            ((stIspTilRdma.stTilRdma.stTil_1_PDFReg.Til_1_DataFormat.field.rdlinebufoff)
                ? ("Disable") : ("Enable"))
            );
        LOG_INFO("DataFormat(Write) - Format = 0x%01X(%s), FlexibleBit = 0x%02X\n",
            stIspTilRdma.stTilRdma.stTil_1_PDFReg.Til_1_DataFormat.field.wrformat,
            g_szFormat[stIspTilRdma.stTilRdma.stTil_1_PDFReg.Til_1_DataFormat.field.wrformat],
            stIspTilRdma.stTilRdma.stTil_1_PDFReg.Til_1_DataFormat.field.wrflexiblebit
            );
        LOG_INFO("DataFormat - Raw_10_Mode = 0x%01X(%s)\n",
            stIspTilRdma.stTilRdma.stTil_1_PDFReg.Til_1_DataFormat.field.imgispraw10,
            g_szRaw10Mode[stIspTilRdma.stTilRdma.stTil_1_PDFReg.Til_1_DataFormat.field.imgispraw10]
            );
        LOG_INFO("Read - Stride = 0x%08X\n",
            stIspTilRdma.stTilRdma.stTil_1_RdSdReg.Til_1_ReadStride.value
            );
#if defined(V4L2_DRIVER)
#else
        LOG_INFO("BusyStatus = 0x%08X, ReadBusyStatus = 0x%01X(%s), "\
            "WriteBusyStatus = 0x%01X(%s)\n",
            stIspTilRdma.stTilRdma.stTil_1_BSReg.Til_1_BusyStatus.value,
            stIspTilRdma.stTilRdma.stTil_1_BSReg.Til_1_BusyStatus.field.rbusy,
            ((stIspTilRdma.stTilRdma.stTil_1_BSReg.Til_1_BusyStatus.field.rbusy)
                ? ("Busy") : ("Done")),
            stIspTilRdma.stTilRdma.stTil_1_BSReg.Til_1_BusyStatus.field.wbusy,
            ((stIspTilRdma.stTilRdma.stTil_1_BSReg.Til_1_BusyStatus.field.wbusy)
                ? ("Busy") : ("Done"))
            );
#endif //#if defined(V4L2_DRIVER)
        LOG_INFO("AxiId = 0x%08X\n",
            stIspTilRdma.stTilRdma.stTil_1_AxiReg.Til_1_AxiId.value
            );
        LOG_INFO("AxiId(Read) - Y_AxiId = 0x%02X, Uv_AxiId = 0x%02X\n",
            stIspTilRdma.stTilRdma.stTil_1_AxiReg.Til_1_AxiId.field.yrid,
            stIspTilRdma.stTilRdma.stTil_1_AxiReg.Til_1_AxiId.field.uvrid
            );
        LOG_INFO("AxiId(Write) - Y_AxiId = 0x%02X, Uv_AxiId = 0x%02X\n",
            stIspTilRdma.stTilRdma.stTil_1_AxiReg.Til_1_AxiId.field.ywid,
            stIspTilRdma.stTilRdma.stTil_1_AxiReg.Til_1_AxiId.field.uvwid
            );
        LOG_INFO("DataPath = 0x%08X\n",
            stIspTilRdma.stTilRdma.stTil_1_DPSReg.Til_1_PathSelect.value
            );
        LOG_INFO("DataPath(Read) - Path = 0x%02X(%s), "\
            "UvFormat = 0x%01X(%s), IndependentRvi = 0x%01X(%s), "\
            "Enable = 0x%01X(%s)\n",
            stIspTilRdma.stTilRdma.stTil_1_DPSReg.Til_1_PathSelect.field.readindex,
            g_szTil_1_DataPath[stIspTilRdma.stTilRdma.stTil_1_DPSReg.Til_1_PathSelect.field.readindex],
            stIspTilRdma.stTilRdma.stTil_1_DPSReg.Til_1_PathSelect.field.rdnv21format,
            ((stIspTilRdma.stTilRdma.stTil_1_DPSReg.Til_1_PathSelect.field.rdnv21format)
                ? ("NV21") : ("NV12")),
            stIspTilRdma.stTilRdma.stTil_1_DPSReg.Til_1_PathSelect.field.independentrdrvi,
            ((stIspTilRdma.stTilRdma.stTil_1_DPSReg.Til_1_PathSelect.field.independentrdrvi)
                ? ("Enable") : ("Disable")),
            stIspTilRdma.stTilRdma.stTil_1_DPSReg.Til_1_PathSelect.field.readdram,
            ((stIspTilRdma.stTilRdma.stTil_1_DPSReg.Til_1_PathSelect.field.readdram)
                ? ("Enable") : ("Disable"))
            );
        LOG_INFO("DataPath(Write) - Path = 0x%02X(%s), "\
            "UvFormat = 0x%01X(%s), Enable = 0x%01X(%s)\n",
            stIspTilRdma.stTilRdma.stTil_1_DPSReg.Til_1_PathSelect.field.writeindex,
            g_szTil_1_DataPath[stIspTilRdma.stTilRdma.stTil_1_DPSReg.Til_1_PathSelect.field.writeindex],
            stIspTilRdma.stTilRdma.stTil_1_DPSReg.Til_1_PathSelect.field.wrnv21format,
            ((stIspTilRdma.stTilRdma.stTil_1_DPSReg.Til_1_PathSelect.field.wrnv21format)
                ? ("NV21") : ("NV12")),
            stIspTilRdma.stTilRdma.stTil_1_DPSReg.Til_1_PathSelect.field.writedram,
            ((stIspTilRdma.stTilRdma.stTil_1_DPSReg.Til_1_PathSelect.field.writedram)
                ? ("Enable") : ("Disable"))
            );
        LOG_INFO("DataPath - Loopback = 0x%01X(%s), DMA = 0x%01X(%s)\n",
            stIspTilRdma.stTilRdma.stTil_1_DPSReg.Til_1_PathSelect.field.loopback,
            ((stIspTilRdma.stTilRdma.stTil_1_DPSReg.Til_1_PathSelect.field.loopback)
                ? ("Enable") : ("Disable")),
            stIspTilRdma.stTilRdma.stTil_1_DPSReg.Til_1_PathSelect.field.movedramdata,
            ((stIspTilRdma.stTilRdma.stTil_1_DPSReg.Til_1_PathSelect.field.movedramdata)
                ? ("Enable") : ("Disable"))
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_ITIIWSR;
#if defined(V4L2_DRIVER)
    u32Length = ST_TIL_REG_LEN + 5;
#else
    u32Length = ST_TIL_REG_LEN;
#endif //#if defined(V4L2_DRIVER)
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
        LOG_INFO("Reg[0x%08X] = 0x%08X\n", u32Offset + i * 4, u32Buffer[i]);
    }
    LOG_INFO("---------------------------------\n");
    //=========================================================================

    return ret;
}

STF_RESULT Test_TIL_GetAllBusyStatus(
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
    u32Length = 1;
    u32Buffer[0] = ADDR_REG_ITIBSR;
    u32Buffer[1] = 0;
    ret = STFLIB_ISP_RegReadByTable(
        TEST_ISP_DEVICE,
        u32Length,
        (ST_REG_TBL *)&u32Buffer[0]
        );
    if (ret) {
        return ret;
    }
    //-------------------------------------------------------------------------
    u32Length = 1;
    u32Buffer[0] = ADDR_REG_ITIBSR;
    u32Buffer[1] = (rand() & 0x3);
    ret = STFLIB_ISP_RegWriteByTable(
        TEST_ISP_DEVICE,
        u32Length,
        (ST_REG_TBL *)&u32Buffer[0]
        );
    if (ret) {
        return ret;
    }
    for (i = 0; i < u32Length; i++) {
        LOG_INFO("Reg[0x%08X] <- 0x%08X\n",
            u32Buffer[i * 2], u32Buffer[i * 2 + 1]);
    }
    LOG_INFO("---------------------------------\n");
    //-------------------------------------------------------------------------
    u32Length = 1;
    u32Buffer[0] = ADDR_REG_ITIBSR;
    u32Buffer[1] = 0;
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
        STF_U8 u8IspIdx = 0;
        STF_U32 u32Status = 0x03;

        u8IspIdx = 0;
        u32Status = STFDRV_ISP_TIL_GetAllBusyStatus(
            pstCiConnection,
            u8IspIdx
            );
        LOG_INFO("ISP_%d : STFDRV_ISP_TIL_GetAllBusyStatus(), "\
            "u32Status = 0x%08X\n",
            u8IspIdx,
            u32Status
            );
        LOG_INFO("TIL 1 BusyStatus = 0x%08X\n", (u32Status & 0x0000FFFF));
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Length = 1;
    u32Buffer[0] = ADDR_REG_ITIBSR;
    u32Buffer[1] = (rand() & 0x3);
    ret = STFLIB_ISP_RegWriteByTable(
        TEST_ISP_DEVICE,
        u32Length,
        (ST_REG_TBL *)&u32Buffer[0]
        );
    if (ret) {
        return ret;
    }
    for (i = 0; i < u32Length; i++) {
        LOG_INFO("Reg[0x%08X] <- 0x%08X\n",
            u32Buffer[i * 2], u32Buffer[i * 2 + 1]);
    }
    LOG_INFO("---------------------------------\n");
    //-------------------------------------------------------------------------
    u32Length = 1;
    u32Buffer[0] = ADDR_REG_ITIBSR;
    u32Buffer[1] = 0x00000000;
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
        STF_U8 u8IspIdx = 0;
        STF_U32 u32Status = 0x03;

        u8IspIdx = 0;
        u32Status = STFDRV_ISP_TIL_GetAllBusyStatus(
            pstCiConnection,
            u8IspIdx
            );
        LOG_INFO("ISP_%d : STFDRV_ISP_TIL_GetAllBusyStatus(), "\
            "u32Status = 0x%08X\n",
            u8IspIdx,
            u32Status
            );
        LOG_INFO("TIL 1 BusyStatus = 0x%08X\n", (u32Status & 0x0000FFFF));
        LOG_INFO("---------------------------------\n");
    }
    //=========================================================================

    return ret;
}

STF_RESULT Test_TIL_1_SetWindowReg(
    STF_VOID
    )
{
    int i;
    //STF_U8 u8Method;
    //STF_U32 u32Offset;
    STF_U32 u32Length;
    STF_U32 u32Buffer[1 * 2];
    CI_CONNECTION *pstCiConnection = NULL;
    STF_RESULT ret = STF_SUCCESS;

    pstCiConnection = g_pstCiConnection;

    //=========================================================================
    u32Length = 1;
    u32Buffer[0] = ADDR_REG_ITIIWSR;
    u32Buffer[1] = 0;
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
        ST_ISP_TIL_RDMA stIspTilRdma;

        stIspTilRdma.stTilRdma.stRdmaStartCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaStartCmd.RegAddr = ADDR_REG_ITIIWSR;
        stIspTilRdma.stTilRdma.stRdmaStartCmd.u32Param = ST_TIL_1_WS_REG_LEN;
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrSdCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrSdCmd.RegAddr = ADDR_REG_ITIDWLSR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrSdCmd.u32Param =
            ST_TIL_1_WR_SD_REG_LEN;
#if defined(V4L2_DRIVER)
#else
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrAddrCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrAddrCmd.RegAddr =
            ADDR_REG_ITIDWYSAR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrAddrCmd.u32Param =
            ST_TIL_1_WR_ADDR_REG_LEN;
#endif //#if defined(V4L2_DRIVER)
#if defined(V4L2_DRIVER)
#else
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdAddrCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdAddrCmd.RegAddr =
            ADDR_REG_ITIDRYSAR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdAddrCmd.u32Param =
            ST_TIL_1_RD_ADDR_REG_LEN;
#endif //#if defined(V4L2_DRIVER)
        stIspTilRdma.stTilRdma.stRdmaTil_1_PDFCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_PDFCmd.RegAddr = ADDR_REG_ITIPDFR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_PDFCmd.u32Param =
            ST_TIL_1_PDF_REG_LEN;
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdSdCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdSdCmd.RegAddr = ADDR_REG_ITIDRLSR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdSdCmd.u32Param =
            ST_TIL_1_RD_SD_REG_LEN;
#if defined(V4L2_DRIVER)
#else
        stIspTilRdma.stTilRdma.stRdmaTil_1_BSCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_BSCmd.RegAddr = ADDR_REG_ITIBSR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_BSCmd.u32Param = ST_TIL_1_BS_REG_LEN;
#endif //#if defined(V4L2_DRIVER)
        stIspTilRdma.stTilRdma.stRdmaTil_1_AxiCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_AxiCmd.RegAddr = ADDR_REG_ITIAIR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_AxiCmd.u32Param =
            ST_TIL_1_AXI_REG_LEN;
        stIspTilRdma.stTilRdma.stRdmaTil_1_DPSCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_DPSCmd.RegAddr = ADDR_REG_ITIDPSR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_DPSCmd.u32Param =
            ST_TIL_1_DPS_REG_LEN;
        stIspTilRdma.stTilRdma.stRdmaEndCmd.Tag = DESP_END;

        stIspTilRdma.stTilRdma.stTil_1_WSReg.Til_1_WindowSize.value =
            (rand() & 0xFFFF) | ((rand() & 0xFFFF) << 16);
        stIspTilRdma.u8IspIdx = 0;
        ret = STFDRV_ISP_TIL_1_SetWindowReg(
            pstCiConnection,
            &stIspTilRdma
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_TIL_1_SetWindowReg()\n",
            stIspTilRdma.u8IspIdx);
        LOG_INFO("Window size = 0x%08X - Width=0x%04X(%d), Height=0x%04X(%d)\n",
            stIspTilRdma.stTilRdma.stTil_1_WSReg.Til_1_WindowSize.value,
            stIspTilRdma.stTilRdma.stTil_1_WSReg.Til_1_WindowSize.field.wsize,
            stIspTilRdma.stTilRdma.stTil_1_WSReg.Til_1_WindowSize.field.wsize,
            stIspTilRdma.stTilRdma.stTil_1_WSReg.Til_1_WindowSize.field.hsize,
            stIspTilRdma.stTilRdma.stTil_1_WSReg.Til_1_WindowSize.field.hsize
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Length = 1;
    u32Buffer[0] = ADDR_REG_ITIIWSR;
    u32Buffer[1] = 0;
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

STF_RESULT Test_TIL_1_SetConfigReg(
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
    u32Buffer[0] = ADDR_REG_ITIPDFR;
    u32Buffer[1] = 0;
    u32Buffer[2] = ADDR_REG_ITIDPSR;
    u32Buffer[3] = 0;
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
        ST_ISP_TIL_RDMA stIspTilRdma;

        stIspTilRdma.stTilRdma.stRdmaStartCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaStartCmd.RegAddr = ADDR_REG_ITIIWSR;
        stIspTilRdma.stTilRdma.stRdmaStartCmd.u32Param = ST_TIL_1_WS_REG_LEN;
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrSdCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrSdCmd.RegAddr = ADDR_REG_ITIDWLSR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrSdCmd.u32Param =
            ST_TIL_1_WR_SD_REG_LEN;
#if defined(V4L2_DRIVER)
#else
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrAddrCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrAddrCmd.RegAddr =
            ADDR_REG_ITIDWYSAR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrAddrCmd.u32Param =
            ST_TIL_1_WR_ADDR_REG_LEN;
#endif //#if defined(V4L2_DRIVER)
#if defined(V4L2_DRIVER)
#else
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdAddrCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdAddrCmd.RegAddr =
            ADDR_REG_ITIDRYSAR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdAddrCmd.u32Param =
            ST_TIL_1_RD_ADDR_REG_LEN;
#endif //#if defined(V4L2_DRIVER)
        stIspTilRdma.stTilRdma.stRdmaTil_1_PDFCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_PDFCmd.RegAddr = ADDR_REG_ITIPDFR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_PDFCmd.u32Param =
            ST_TIL_1_PDF_REG_LEN;
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdSdCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdSdCmd.RegAddr = ADDR_REG_ITIDRLSR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdSdCmd.u32Param =
            ST_TIL_1_RD_SD_REG_LEN;
#if defined(V4L2_DRIVER)
#else
        stIspTilRdma.stTilRdma.stRdmaTil_1_BSCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_BSCmd.RegAddr = ADDR_REG_ITIBSR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_BSCmd.u32Param = ST_TIL_1_BS_REG_LEN;
#endif //#if defined(V4L2_DRIVER)
        stIspTilRdma.stTilRdma.stRdmaTil_1_AxiCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_AxiCmd.RegAddr = ADDR_REG_ITIAIR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_AxiCmd.u32Param =
            ST_TIL_1_AXI_REG_LEN;
        stIspTilRdma.stTilRdma.stRdmaTil_1_DPSCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_DPSCmd.RegAddr = ADDR_REG_ITIDPSR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_DPSCmd.u32Param =
            ST_TIL_1_DPS_REG_LEN;
        stIspTilRdma.stTilRdma.stRdmaEndCmd.Tag = DESP_END;

        stIspTilRdma.stTilRdma.stTil_1_PDFReg.Til_1_DataFormat.value = 0;
        stIspTilRdma.stTilRdma.stTil_1_PDFReg.Til_1_DataFormat.field.imgispraw10 =
            (rand() & 0x1);
        stIspTilRdma.stTilRdma.stTil_1_DPSReg.Til_1_PathSelect.value = 0;
        stIspTilRdma.stTilRdma.stTil_1_DPSReg.Til_1_PathSelect.field.loopback =
            (rand() & 0x1);
        stIspTilRdma.stTilRdma.stTil_1_DPSReg.Til_1_PathSelect.field.movedramdata =
            (rand() & 0x1);
        stIspTilRdma.u8IspIdx = 0;
        ret = STFDRV_ISP_TIL_1_SetConfigReg(
            pstCiConnection,
            &stIspTilRdma
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_TIL_1_SetConfigReg()\n",
            stIspTilRdma.u8IspIdx);
        LOG_INFO("DataFormat = 0x%08X\n",
            stIspTilRdma.stTilRdma.stTil_1_PDFReg.Til_1_DataFormat.value
            );
        LOG_INFO("DataFormat - Raw_10_Mode = 0x%01X(%s)\n",
            stIspTilRdma.stTilRdma.stTil_1_PDFReg.Til_1_DataFormat.field.imgispraw10,
            g_szRaw10Mode[stIspTilRdma.stTilRdma.stTil_1_PDFReg.Til_1_DataFormat.field.imgispraw10]
            );
        LOG_INFO("DataPath = 0x%08X\n",
            stIspTilRdma.stTilRdma.stTil_1_DPSReg.Til_1_PathSelect.value
            );
        LOG_INFO("DataPath - Loopback = 0x%01X(%s), DMA = 0x%01X(%s)\n",
            stIspTilRdma.stTilRdma.stTil_1_DPSReg.Til_1_PathSelect.field.loopback,
            ((stIspTilRdma.stTilRdma.stTil_1_DPSReg.Til_1_PathSelect.field.loopback)
                ? ("Enable") : ("Disable")),
            stIspTilRdma.stTilRdma.stTil_1_DPSReg.Til_1_PathSelect.field.movedramdata,
            ((stIspTilRdma.stTilRdma.stTil_1_DPSReg.Til_1_PathSelect.field.movedramdata)
                ? ("Enable") : ("Disable"))
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Length = 2;
    u32Buffer[0] = ADDR_REG_ITIPDFR;
    u32Buffer[1] = 0;
    u32Buffer[2] = ADDR_REG_ITIDPSR;
    u32Buffer[3] = 0;
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

STF_RESULT Test_TIL_1_SetReadReg(
    STF_VOID
    )
{
    int i;
    //STF_U8 u8Method;
    //STF_U32 u32Offset;
    STF_U32 u32Length;
    STF_U32 u32Buffer[7 * 2];
    CI_CONNECTION *pstCiConnection = NULL;
    STF_RESULT ret = STF_SUCCESS;

    pstCiConnection = g_pstCiConnection;

    //=========================================================================
    u32Length = 7;
    u32Buffer[0] = ADDR_REG_ITIIWSR;
    u32Buffer[1] = 0;
    u32Buffer[2] = ADDR_REG_ITIDRYSAR;
    u32Buffer[3] = 0;
    u32Buffer[4] = ADDR_REG_ITIDRUSAR;
    u32Buffer[5] = 0;
    u32Buffer[6] = ADDR_REG_ITIDRLSR;
    u32Buffer[7] = 0;
    u32Buffer[8] = ADDR_REG_ITIPDFR;
    u32Buffer[9] = 0;
    u32Buffer[10] = ADDR_REG_ITIAIR;
    u32Buffer[11] = 0;
    u32Buffer[12] = ADDR_REG_ITIDPSR;
    u32Buffer[13] = 0;
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
        //STF_U8 *pu8Temp = NULL;
        //STF_U32 u32Idx;
        //STF_U32 u32Cnt;
        ST_ISP_TIL_RDMA stIspTilRdma;

        stIspTilRdma.stTilRdma.stRdmaStartCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaStartCmd.RegAddr = ADDR_REG_ITIIWSR;
        stIspTilRdma.stTilRdma.stRdmaStartCmd.u32Param = ST_TIL_1_WS_REG_LEN;
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrSdCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrSdCmd.RegAddr = ADDR_REG_ITIDWLSR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrSdCmd.u32Param =
            ST_TIL_1_WR_SD_REG_LEN;
#if defined(V4L2_DRIVER)
#else
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrAddrCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrAddrCmd.RegAddr =
            ADDR_REG_ITIDWYSAR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrAddrCmd.u32Param =
            ST_TIL_1_WR_ADDR_REG_LEN;
#endif //#if defined(V4L2_DRIVER)
#if defined(V4L2_DRIVER)
#else
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdAddrCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdAddrCmd.RegAddr =
            ADDR_REG_ITIDRYSAR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdAddrCmd.u32Param =
            ST_TIL_1_RD_ADDR_REG_LEN;
#endif //#if defined(V4L2_DRIVER)
        stIspTilRdma.stTilRdma.stRdmaTil_1_PDFCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_PDFCmd.RegAddr = ADDR_REG_ITIPDFR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_PDFCmd.u32Param =
            ST_TIL_1_PDF_REG_LEN;
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdSdCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdSdCmd.RegAddr = ADDR_REG_ITIDRLSR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdSdCmd.u32Param =
            ST_TIL_1_RD_SD_REG_LEN;
#if defined(V4L2_DRIVER)
#else
        stIspTilRdma.stTilRdma.stRdmaTil_1_BSCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_BSCmd.RegAddr = ADDR_REG_ITIBSR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_BSCmd.u32Param = ST_TIL_1_BS_REG_LEN;
#endif //#if defined(V4L2_DRIVER)
        stIspTilRdma.stTilRdma.stRdmaTil_1_AxiCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_AxiCmd.RegAddr = ADDR_REG_ITIAIR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_AxiCmd.u32Param =
            ST_TIL_1_AXI_REG_LEN;
        stIspTilRdma.stTilRdma.stRdmaTil_1_DPSCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_DPSCmd.RegAddr = ADDR_REG_ITIDPSR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_DPSCmd.u32Param =
            ST_TIL_1_DPS_REG_LEN;
        stIspTilRdma.stTilRdma.stRdmaEndCmd.Tag = DESP_END;

        stIspTilRdma.stTilRdma.stTil_1_WSReg.Til_1_WindowSize.value =
            (rand() & 0xFFFF) | ((rand() & 0xFFFF) << 16);
#if defined(V4L2_DRIVER)
#else
        stIspTilRdma.stTilRdma.stTil_1_RdAddrReg.Til_1_ReadYAddr.value =
            (rand() & 0xFFFFFFF8);
        stIspTilRdma.stTilRdma.stTil_1_RdAddrReg.Til_1_ReadUvAddr.value =
            (rand() & 0xFFFFFFF8);
#endif //#if defined(V4L2_DRIVER)
        stIspTilRdma.stTilRdma.stTil_1_RdSdReg.Til_1_ReadStride.value =
            (rand() & 0x007FFFF8);
        stIspTilRdma.stTilRdma.stTil_1_DPSReg.Til_1_PathSelect.value = 0;
        //stIspTilRdma.stTilRdma.stTil_1_DPSReg.Til_1_PathSelect.field.readindex =
        //    (rand() & 0xFF);
        stIspTilRdma.stTilRdma.stTil_1_DPSReg.Til_1_PathSelect.field.readindex =
            (rand() & 0x3);
        stIspTilRdma.stTilRdma.stTil_1_DPSReg.Til_1_PathSelect.field.rdnv21format =
            (rand() & 0x1);
        stIspTilRdma.stTilRdma.stTil_1_DPSReg.Til_1_PathSelect.field.independentrdrvi =
            (rand() & 0x1);
        stIspTilRdma.stTilRdma.stTil_1_DPSReg.Til_1_PathSelect.field.loopback =
            (rand() & 0x1);
        stIspTilRdma.stTilRdma.stTil_1_DPSReg.Til_1_PathSelect.field.movedramdata =
            (rand() & 0x1);
        stIspTilRdma.stTilRdma.stTil_1_DPSReg.Til_1_PathSelect.field.readdram =
            (rand() & 0x1);
        stIspTilRdma.stTilRdma.stTil_1_PDFReg.Til_1_DataFormat.value = 0;
        stIspTilRdma.stTilRdma.stTil_1_PDFReg.Til_1_DataFormat.field.rdformat =
            (rand() & 0x7);
        stIspTilRdma.stTilRdma.stTil_1_PDFReg.Til_1_DataFormat.field.rdflexiblebit =
            (rand() & 0x1F);
        stIspTilRdma.stTilRdma.stTil_1_PDFReg.Til_1_DataFormat.field.rdlinebufoff =
            (rand() & 0x1);
        stIspTilRdma.stTilRdma.stTil_1_PDFReg.Til_1_DataFormat.field.imgispraw10 =
            (rand() & 0x1);
        stIspTilRdma.stTilRdma.stTil_1_AxiReg.Til_1_AxiId.value = 0;
        stIspTilRdma.stTilRdma.stTil_1_AxiReg.Til_1_AxiId.field.yrid =
            (rand() & 0xFF);
        stIspTilRdma.stTilRdma.stTil_1_AxiReg.Til_1_AxiId.field.uvrid =
            (rand() & 0xFF);
#if defined(V4L2_DRIVER)
#else
        stIspTilRdma.stTilRdma.stTil_1_BSReg.Til_1_BusyStatus.value = 0;
#endif //#if defined(V4L2_DRIVER)
        stIspTilRdma.u8IspIdx = 0;
        ret = STFDRV_ISP_TIL_1_SetReadReg(
            pstCiConnection,
            &stIspTilRdma
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_TIL_1_SetReadReg()\n",
            stIspTilRdma.u8IspIdx);
        LOG_INFO("Window size = 0x%08X - Width=0x%04X(%d), Height=0x%04X(%d)\n",
            stIspTilRdma.stTilRdma.stTil_1_WSReg.Til_1_WindowSize.value,
            stIspTilRdma.stTilRdma.stTil_1_WSReg.Til_1_WindowSize.field.wsize,
            stIspTilRdma.stTilRdma.stTil_1_WSReg.Til_1_WindowSize.field.wsize,
            stIspTilRdma.stTilRdma.stTil_1_WSReg.Til_1_WindowSize.field.hsize,
            stIspTilRdma.stTilRdma.stTil_1_WSReg.Til_1_WindowSize.field.hsize
            );
#if defined(V4L2_DRIVER)
#else
        LOG_INFO("Read - Y_Addr = 0x%08X, UV_Addr = 0x%08X\n",
            stIspTilRdma.stTilRdma.stTil_1_RdAddrReg.Til_1_ReadYAddr.value,
            stIspTilRdma.stTilRdma.stTil_1_RdAddrReg.Til_1_ReadUvAddr.value
            );
#endif //#if defined(V4L2_DRIVER)
        LOG_INFO("Read - Stride = 0x%08X\n",
            stIspTilRdma.stTilRdma.stTil_1_RdSdReg.Til_1_ReadStride.value
            );
        LOG_INFO("DataFormat = 0x%08X\n",
            stIspTilRdma.stTilRdma.stTil_1_PDFReg.Til_1_DataFormat.value
            );
        LOG_INFO("DataFormat(Read) - Format = 0x%01X(%s), "\
            "FlexibleBit = 0x%02X, UvLineBuf = 0x%1X(%s)\n",
            stIspTilRdma.stTilRdma.stTil_1_PDFReg.Til_1_DataFormat.field.rdformat,
            g_szFormat[stIspTilRdma.stTilRdma.stTil_1_PDFReg.Til_1_DataFormat.field.rdformat],
            stIspTilRdma.stTilRdma.stTil_1_PDFReg.Til_1_DataFormat.field.rdflexiblebit,
            stIspTilRdma.stTilRdma.stTil_1_PDFReg.Til_1_DataFormat.field.rdlinebufoff,
            ((stIspTilRdma.stTilRdma.stTil_1_PDFReg.Til_1_DataFormat.field.rdlinebufoff)
                ? ("Disable") : ("Enable"))
            );
        LOG_INFO("DataFormat - Raw_10_Mode = 0x%01X(%s)\n",
            stIspTilRdma.stTilRdma.stTil_1_PDFReg.Til_1_DataFormat.field.imgispraw10,
            g_szRaw10Mode[stIspTilRdma.stTilRdma.stTil_1_PDFReg.Til_1_DataFormat.field.imgispraw10]
            );
        LOG_INFO("AxiId = 0x%08X\n",
            stIspTilRdma.stTilRdma.stTil_1_AxiReg.Til_1_AxiId.value
            );
        LOG_INFO("AxiId(Read) - Y_AxiId = 0x%02X, Uv_AxiId = 0x%02X\n",
            stIspTilRdma.stTilRdma.stTil_1_AxiReg.Til_1_AxiId.field.yrid,
            stIspTilRdma.stTilRdma.stTil_1_AxiReg.Til_1_AxiId.field.uvrid
            );
        LOG_INFO("DataPath = 0x%08X\n",
            stIspTilRdma.stTilRdma.stTil_1_DPSReg.Til_1_PathSelect.value
            );
        LOG_INFO("DataPath(Read) - Path = 0x%02X(%s), "\
            "UvFormat = 0x%01X(%s), IndependentRvi = 0x%01X(%s), "\
            "Enable = 0x%01X(%s)\n",
            stIspTilRdma.stTilRdma.stTil_1_DPSReg.Til_1_PathSelect.field.readindex,
            g_szTil_1_DataPath[stIspTilRdma.stTilRdma.stTil_1_DPSReg.Til_1_PathSelect.field.readindex],
            stIspTilRdma.stTilRdma.stTil_1_DPSReg.Til_1_PathSelect.field.rdnv21format,
            ((stIspTilRdma.stTilRdma.stTil_1_DPSReg.Til_1_PathSelect.field.rdnv21format)
                ? ("NV21") : ("NV12")),
            stIspTilRdma.stTilRdma.stTil_1_DPSReg.Til_1_PathSelect.field.independentrdrvi,
            ((stIspTilRdma.stTilRdma.stTil_1_DPSReg.Til_1_PathSelect.field.independentrdrvi)
                ? ("Enable") : ("Disable")),
            stIspTilRdma.stTilRdma.stTil_1_DPSReg.Til_1_PathSelect.field.readdram,
            ((stIspTilRdma.stTilRdma.stTil_1_DPSReg.Til_1_PathSelect.field.readdram)
                ? ("Enable") : ("Disable"))
            );
        LOG_INFO("DataPath - Loopback = 0x%01X(%s), DMA = 0x%01X(%s)\n",
            stIspTilRdma.stTilRdma.stTil_1_DPSReg.Til_1_PathSelect.field.loopback,
            ((stIspTilRdma.stTilRdma.stTil_1_DPSReg.Til_1_PathSelect.field.loopback)
                ? ("Enable") : ("Disable")),
            stIspTilRdma.stTilRdma.stTil_1_DPSReg.Til_1_PathSelect.field.movedramdata,
            ((stIspTilRdma.stTilRdma.stTil_1_DPSReg.Til_1_PathSelect.field.movedramdata)
                ? ("Enable") : ("Disable"))
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Length = 7;
    u32Buffer[0] = ADDR_REG_ITIIWSR;
    u32Buffer[1] = 0;
    u32Buffer[2] = ADDR_REG_ITIDRYSAR;
    u32Buffer[3] = 0;
    u32Buffer[4] = ADDR_REG_ITIDRUSAR;
    u32Buffer[5] = 0;
    u32Buffer[6] = ADDR_REG_ITIDRLSR;
    u32Buffer[7] = 0;
    u32Buffer[8] = ADDR_REG_ITIPDFR;
    u32Buffer[9] = 0;
    u32Buffer[10] = ADDR_REG_ITIAIR;
    u32Buffer[11] = 0;
    u32Buffer[12] = ADDR_REG_ITIDPSR;
    u32Buffer[13] = 0;
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

STF_RESULT Test_TIL_1_SetWriteReg(
    STF_VOID
    )
{
    int i;
    //STF_U8 u8Method;
    //STF_U32 u32Offset;
    STF_U32 u32Length;
    STF_U32 u32Buffer[7 * 2];
    CI_CONNECTION *pstCiConnection = NULL;
    STF_RESULT ret = STF_SUCCESS;

    pstCiConnection = g_pstCiConnection;

    //=========================================================================
    u32Length = 7;
    u32Buffer[0] = ADDR_REG_ITIIWSR;
    u32Buffer[1] = 0;
    u32Buffer[2] = ADDR_REG_ITIDWYSAR;
    u32Buffer[3] = 0;
    u32Buffer[4] = ADDR_REG_ITIDWUSAR;
    u32Buffer[5] = 0;
    u32Buffer[6] = ADDR_REG_ITIDWLSR;
    u32Buffer[7] = 0;
    u32Buffer[8] = ADDR_REG_ITIPDFR;
    u32Buffer[9] = 0;
    u32Buffer[10] = ADDR_REG_ITIAIR;
    u32Buffer[11] = 0;
    u32Buffer[12] = ADDR_REG_ITIDPSR;
    u32Buffer[13] = 0;
    ret = STFLIB_ISP_RegReadByTable(
        TEST_ISP_DEVICE,
        u32Length,
        (ST_REG_TBL *)&u32Buffer[0]
        );
    if (ret) {
        return ret;
    }
    for (i = 0; i < u32Length; i++) {
        LOG_INFO("Reg[0x%08X] = 0x%08X\n", u32Buffer[i * 2],
            u32Buffer[i * 2 + 1]);
    }
    LOG_INFO("---------------------------------\n");
    //-------------------------------------------------------------------------
    {
        //STF_U8 *pu8Temp = NULL;
        //STF_U32 u32Idx;
        //STF_U32 u32Cnt;
        ST_ISP_TIL_RDMA stIspTilRdma;

        stIspTilRdma.stTilRdma.stRdmaStartCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaStartCmd.RegAddr = ADDR_REG_ITIIWSR;
        stIspTilRdma.stTilRdma.stRdmaStartCmd.u32Param = ST_TIL_1_WS_REG_LEN;
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrSdCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrSdCmd.RegAddr = ADDR_REG_ITIDWLSR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrSdCmd.u32Param =
            ST_TIL_1_WR_SD_REG_LEN;
#if defined(V4L2_DRIVER)
#else
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrAddrCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrAddrCmd.RegAddr =
            ADDR_REG_ITIDWYSAR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrAddrCmd.u32Param =
            ST_TIL_1_WR_ADDR_REG_LEN;
#endif //#if defined(V4L2_DRIVER)
#if defined(V4L2_DRIVER)
#else
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdAddrCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdAddrCmd.RegAddr =
            ADDR_REG_ITIDRYSAR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdAddrCmd.u32Param =
            ST_TIL_1_RD_ADDR_REG_LEN;
#endif //#if defined(V4L2_DRIVER)
        stIspTilRdma.stTilRdma.stRdmaTil_1_PDFCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_PDFCmd.RegAddr = ADDR_REG_ITIPDFR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_PDFCmd.u32Param =
            ST_TIL_1_PDF_REG_LEN;
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdSdCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdSdCmd.RegAddr = ADDR_REG_ITIDRLSR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdSdCmd.u32Param =
            ST_TIL_1_RD_SD_REG_LEN;
#if defined(V4L2_DRIVER)
#else
        stIspTilRdma.stTilRdma.stRdmaTil_1_BSCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_BSCmd.RegAddr = ADDR_REG_ITIBSR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_BSCmd.u32Param = ST_TIL_1_BS_REG_LEN;
#endif //#if defined(V4L2_DRIVER)
        stIspTilRdma.stTilRdma.stRdmaTil_1_AxiCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_AxiCmd.RegAddr = ADDR_REG_ITIAIR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_AxiCmd.u32Param =
            ST_TIL_1_AXI_REG_LEN;
        stIspTilRdma.stTilRdma.stRdmaTil_1_DPSCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_DPSCmd.RegAddr = ADDR_REG_ITIDPSR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_DPSCmd.u32Param =
            ST_TIL_1_DPS_REG_LEN;
        stIspTilRdma.stTilRdma.stRdmaEndCmd.Tag = DESP_END;

        stIspTilRdma.stTilRdma.stTil_1_WSReg.Til_1_WindowSize.value =
            (rand() & 0xFFFF) | ((rand() & 0xFFFF) << 16);
#if defined(V4L2_DRIVER)
#else
        stIspTilRdma.stTilRdma.stTil_1_WrAddrReg.Til_1_WriteYAddr.value =
            (rand() & 0xFFFFFFF8);
        stIspTilRdma.stTilRdma.stTil_1_WrAddrReg.Til_1_WriteUvAddr.value =
            (rand() & 0xFFFFFFF8);
#endif //#if defined(V4L2_DRIVER)
        stIspTilRdma.stTilRdma.stTil_1_WrSdReg.Til_1_WriteStride.value =
            (rand() & 0x007FFFF8);
        stIspTilRdma.stTilRdma.stTil_1_DPSReg.Til_1_PathSelect.value = 0;
        //stIspTilRdma.stTilRdma.stTil_1_DPSReg.Til_1_PathSelect.field.writeindex =
        //    (rand() & 0xFF);
        stIspTilRdma.stTilRdma.stTil_1_DPSReg.Til_1_PathSelect.field.writeindex =
            (rand() & 0x3);
        stIspTilRdma.stTilRdma.stTil_1_DPSReg.Til_1_PathSelect.field.wrnv21format =
            (rand() & 0x1);
        stIspTilRdma.stTilRdma.stTil_1_DPSReg.Til_1_PathSelect.field.loopback =
            (rand() & 0x1);
        stIspTilRdma.stTilRdma.stTil_1_DPSReg.Til_1_PathSelect.field.movedramdata =
            (rand() & 0x1);
        stIspTilRdma.stTilRdma.stTil_1_DPSReg.Til_1_PathSelect.field.writedram =
            (rand() & 0x1);
        stIspTilRdma.stTilRdma.stTil_1_PDFReg.Til_1_DataFormat.value = 0;
        stIspTilRdma.stTilRdma.stTil_1_PDFReg.Til_1_DataFormat.field.wrformat =
            (rand() & 0x7);
        stIspTilRdma.stTilRdma.stTil_1_PDFReg.Til_1_DataFormat.field.wrflexiblebit =
            (rand() & 0x1F);
        stIspTilRdma.stTilRdma.stTil_1_PDFReg.Til_1_DataFormat.field.imgispraw10 =
            (rand() & 0x1);
        stIspTilRdma.stTilRdma.stTil_1_AxiReg.Til_1_AxiId.value = 0;
        stIspTilRdma.stTilRdma.stTil_1_AxiReg.Til_1_AxiId.field.ywid =
            (rand() & 0xFF);
        stIspTilRdma.stTilRdma.stTil_1_AxiReg.Til_1_AxiId.field.uvwid =
            (rand() & 0xFF);
#if defined(V4L2_DRIVER)
#else
        stIspTilRdma.stTilRdma.stTil_1_BSReg.Til_1_BusyStatus.value = 0;
#endif //#if defined(V4L2_DRIVER)
        stIspTilRdma.u8IspIdx = 0;
        ret = STFDRV_ISP_TIL_1_SetWriteReg(
            pstCiConnection,
            &stIspTilRdma
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_TIL_1_SetWriteReg()\n",
            stIspTilRdma.u8IspIdx);
        LOG_INFO("Window size = 0x%08X - Width=0x%04X(%d), Height=0x%04X(%d)\n",
            stIspTilRdma.stTilRdma.stTil_1_WSReg.Til_1_WindowSize.value,
            stIspTilRdma.stTilRdma.stTil_1_WSReg.Til_1_WindowSize.field.wsize,
            stIspTilRdma.stTilRdma.stTil_1_WSReg.Til_1_WindowSize.field.wsize,
            stIspTilRdma.stTilRdma.stTil_1_WSReg.Til_1_WindowSize.field.hsize,
            stIspTilRdma.stTilRdma.stTil_1_WSReg.Til_1_WindowSize.field.hsize
            );
#if defined(V4L2_DRIVER)
#else
        LOG_INFO("Write - Y_Addr = 0x%08X, UV_Addr = 0x%08X\n",
            stIspTilRdma.stTilRdma.stTil_1_WrAddrReg.Til_1_WriteYAddr.value,
            stIspTilRdma.stTilRdma.stTil_1_WrAddrReg.Til_1_WriteUvAddr.value
            );
#endif //#if defined(V4L2_DRIVER)
        LOG_INFO("Write - Stride = 0x%08X\n",
            stIspTilRdma.stTilRdma.stTil_1_WrSdReg.Til_1_WriteStride.value
            );
        LOG_INFO("DataFormat = 0x%08X\n",
            stIspTilRdma.stTilRdma.stTil_1_PDFReg.Til_1_DataFormat.value
            );
        LOG_INFO("DataFormat(Write) - Format = 0x%01X(%s), "\
            "FlexibleBit = 0x%02X\n",
            stIspTilRdma.stTilRdma.stTil_1_PDFReg.Til_1_DataFormat.field.wrformat,
            g_szFormat[stIspTilRdma.stTilRdma.stTil_1_PDFReg.Til_1_DataFormat.field.wrformat],
            stIspTilRdma.stTilRdma.stTil_1_PDFReg.Til_1_DataFormat.field.wrflexiblebit
            );
        LOG_INFO("DataFormat - Raw_10_Mode = 0x%01X(%s)\n",
            stIspTilRdma.stTilRdma.stTil_1_PDFReg.Til_1_DataFormat.field.imgispraw10,
            g_szRaw10Mode[stIspTilRdma.stTilRdma.stTil_1_PDFReg.Til_1_DataFormat.field.imgispraw10]
            );
        LOG_INFO("AxiId = 0x%08X\n",
            stIspTilRdma.stTilRdma.stTil_1_AxiReg.Til_1_AxiId.value
            );
        LOG_INFO("AxiId(Write) - Y_AxiId = 0x%02X, Uv_AxiId = 0x%02X\n",
            stIspTilRdma.stTilRdma.stTil_1_AxiReg.Til_1_AxiId.field.ywid,
            stIspTilRdma.stTilRdma.stTil_1_AxiReg.Til_1_AxiId.field.uvwid
            );
        LOG_INFO("DataPath = 0x%08X\n",
            stIspTilRdma.stTilRdma.stTil_1_DPSReg.Til_1_PathSelect.value
            );
        LOG_INFO("DataPath(Write) - Path = 0x%02X(%s), "\
            "UvFormat = 0x%01X(%s), Enable = 0x%01X(%s)\n",
            stIspTilRdma.stTilRdma.stTil_1_DPSReg.Til_1_PathSelect.field.writeindex,
            g_szTil_1_DataPath[stIspTilRdma.stTilRdma.stTil_1_DPSReg.Til_1_PathSelect.field.writeindex],
            stIspTilRdma.stTilRdma.stTil_1_DPSReg.Til_1_PathSelect.field.wrnv21format,
            ((stIspTilRdma.stTilRdma.stTil_1_DPSReg.Til_1_PathSelect.field.wrnv21format)
                ? ("NV21") : ("NV12")),
            stIspTilRdma.stTilRdma.stTil_1_DPSReg.Til_1_PathSelect.field.writedram,
            ((stIspTilRdma.stTilRdma.stTil_1_DPSReg.Til_1_PathSelect.field.writedram)
                ? ("Enable") : ("Disable"))
            );
        LOG_INFO("DataPath - Loopback = 0x%01X(%s), DMA = 0x%01X(%s)\n",
            stIspTilRdma.stTilRdma.stTil_1_DPSReg.Til_1_PathSelect.field.loopback,
            ((stIspTilRdma.stTilRdma.stTil_1_DPSReg.Til_1_PathSelect.field.loopback)
                ? ("Enable") : ("Disable")),
            stIspTilRdma.stTilRdma.stTil_1_DPSReg.Til_1_PathSelect.field.movedramdata,
            ((stIspTilRdma.stTilRdma.stTil_1_DPSReg.Til_1_PathSelect.field.movedramdata)
                ? ("Enable") : ("Disable"))
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Length = 7;
    u32Buffer[0] = ADDR_REG_ITIIWSR;
    u32Buffer[1] = 0;
    u32Buffer[2] = ADDR_REG_ITIDWYSAR;
    u32Buffer[3] = 0;
    u32Buffer[4] = ADDR_REG_ITIDWUSAR;
    u32Buffer[5] = 0;
    u32Buffer[6] = ADDR_REG_ITIDWLSR;
    u32Buffer[7] = 0;
    u32Buffer[8] = ADDR_REG_ITIPDFR;
    u32Buffer[9] = 0;
    u32Buffer[10] = ADDR_REG_ITIAIR;
    u32Buffer[11] = 0;
    u32Buffer[12] = ADDR_REG_ITIDPSR;
    u32Buffer[13] = 0;
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

STF_RESULT Test_TIL_1_GetBusyStatus(
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
    u32Offset = ADDR_REG_ITIBSR;
    ret = STFLIB_ISP_RegReadOne(
        TEST_ISP_DEVICE,
        u32Offset,
        (STF_U32 *)&u32Buffer[0]
        );
    if (ret) {
        return ret;
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_ITIBSR;
    u32Buffer[0] = (rand() & 0x3);
    ret = STFLIB_ISP_RegWriteOne(
        TEST_ISP_DEVICE,
        u32Offset,
        (STF_U32 *)&u32Buffer[0]
        );
    if (ret) {
        return ret;
    }
    LOG_INFO("Reg[0x%08X] <- 0x%08X\n", u32Offset, u32Buffer[0]);
    LOG_INFO("---------------------------------\n");
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_ITIBSR;
    u32Buffer[0] = 0x00000000;
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
        STF_U8 u8IspIdx = 0;
        STF_U32 u32Status = 0x03;

        u8IspIdx = 0;
        u32Status = STFDRV_ISP_TIL_1_GetBusyStatus(
            pstCiConnection,
            u8IspIdx
            );
        LOG_INFO("ISP_%d : STFDRV_ISP_TIL_1_GetBusyStatus(), "\
            "u32Status = 0x%08X\n",
            u8IspIdx,
            u32Status
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_ITIBSR;
    u32Buffer[0] = (rand() & 0x3);
    ret = STFLIB_ISP_RegWriteOne(
        TEST_ISP_DEVICE,
        u32Offset,
        (STF_U32 *)&u32Buffer[0]
        );
    if (ret) {
        return ret;
    }
    LOG_INFO("Reg[0x%08X] <- 0x%08X\n", u32Offset, u32Buffer[0]);
    LOG_INFO("---------------------------------\n");
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_ITIBSR;
    u32Buffer[0] = 0x00000000;
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
        STF_U8 u8IspIdx = 0;
        STF_U32 u32Status = 0x03;

        u8IspIdx = 0;
        u32Status = STFDRV_ISP_TIL_1_GetBusyStatus(
            pstCiConnection,
            u8IspIdx
            );
        LOG_INFO("ISP_%d : STFDRV_ISP_TIL_1_GetBusyStatus(), "\
            "u32Status = 0x%08X\n",
            u8IspIdx,
            u32Status
            );
        LOG_INFO("---------------------------------\n");
    }
    //=========================================================================

    return ret;
}

#if defined(EXTEND_IOCTL_COMMAND_TIL)
STF_RESULT Test_TIL_1_IsReadBusy(
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
    u32Offset = ADDR_REG_ITIBSR;
    ret = STFLIB_ISP_RegReadOne(
        TEST_ISP_DEVICE,
        u32Offset,
        (STF_U32 *)&u32Buffer[0]
        );
    if (ret) {
        return ret;
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_ITIBSR;
    u32Buffer[0] |= 0x02;
    ret = STFLIB_ISP_RegWriteOne(
        TEST_ISP_DEVICE,
        u32Offset,
        (STF_U32 *)&u32Buffer[0]
        );
    if (ret) {
        return ret;
    }
    LOG_INFO("Reg[0x%08X] <- 0x%08X\n", u32Offset, u32Buffer[0]);
    LOG_INFO("---------------------------------\n");
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_ITIBSR;
    u32Buffer[0] = 0x00000000;
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
        STF_U8 u8IspIdx = 0;
        STF_BOOL8 bStatus = STF_FALSE;

        u8IspIdx = 0;
        bStatus = STFDRV_ISP_TIL_1_IsReadBusy(
            pstCiConnection,
            u8IspIdx
            );
        LOG_INFO("ISP_%d : STFDRV_ISP_TIL_1_IsReadBusy(), bStatus = %s\n",
            u8IspIdx,
            ((bStatus) ? ("Busy") : ("Done"))
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_ITIBSR;
    u32Buffer[0] &= 0xFFFFFFFD;
    ret = STFLIB_ISP_RegWriteOne(
        TEST_ISP_DEVICE,
        u32Offset,
        (STF_U32 *)&u32Buffer[0]
        );
    if (ret) {
        return ret;
    }
    LOG_INFO("Reg[0x%08X] <- 0x%08X\n", u32Offset, u32Buffer[0]);
    LOG_INFO("---------------------------------\n");
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_ITIBSR;
    u32Buffer[0] = 0x00000000;
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
        STF_U8 u8IspIdx = 0;
        STF_BOOL8 bStatus = STF_FALSE;

        u8IspIdx = 0;
        bStatus = STFDRV_ISP_TIL_1_IsReadBusy(
            pstCiConnection,
            u8IspIdx
            );
        LOG_INFO("ISP_%d : STFDRV_ISP_TIL_1_IsReadBusy(), bStatus = %s\n",
            u8IspIdx,
            ((bStatus) ? ("Busy") : ("Done"))
            );
        LOG_INFO("---------------------------------\n");
    }
    //=========================================================================

    return ret;
}

STF_RESULT Test_TIL_1_IsWriteBusy(
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
    u32Offset = ADDR_REG_ITIBSR;
    ret = STFLIB_ISP_RegReadOne(
        TEST_ISP_DEVICE,
        u32Offset,
        (STF_U32 *)&u32Buffer[0]
        );
    if (ret) {
        return ret;
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_ITIBSR;
    u32Buffer[0] |= 0x01;
    ret = STFLIB_ISP_RegWriteOne(
        TEST_ISP_DEVICE,
        u32Offset,
        (STF_U32 *)&u32Buffer[0]
        );
    if (ret) {
        return ret;
    }
    LOG_INFO("Reg[0x%08X] <- 0x%08X\n", u32Offset, u32Buffer[0]);
    LOG_INFO("---------------------------------\n");
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_ITIBSR;
    u32Buffer[0] = 0x00000000;
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
        STF_U8 u8IspIdx = 0;
        STF_BOOL8 bStatus = STF_FALSE;

        u8IspIdx = 0;
        bStatus = STFDRV_ISP_TIL_1_IsWriteBusy(
            pstCiConnection,
            u8IspIdx
            );
        LOG_INFO("ISP_%d : STFDRV_ISP_TIL_1_IsWriteBusy(), bStatus = %s\n",
            u8IspIdx,
            ((bStatus) ? ("Busy") : ("Done"))
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_ITIBSR;
    u32Buffer[0] &= 0xFFFFFFFE;
    ret = STFLIB_ISP_RegWriteOne(
        TEST_ISP_DEVICE,
        u32Offset,
        (STF_U32 *)&u32Buffer[0]
        );
    if (ret) {
        return ret;
    }
    LOG_INFO("Reg[0x%08X] <- 0x%08X\n", u32Offset, u32Buffer[0]);
    LOG_INFO("---------------------------------\n");
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_ITIBSR;
    u32Buffer[0] = 0x00000000;
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
        STF_U8 u8IspIdx = 0;
        STF_BOOL8 bStatus = STF_FALSE;

        u8IspIdx = 0;
        bStatus = STFDRV_ISP_TIL_1_IsWriteBusy(
            pstCiConnection,
            u8IspIdx
            );
        LOG_INFO("ISP_%d : STFDRV_ISP_TIL_1_IsWriteBusy(), bStatus = %s\n",
            u8IspIdx,
            ((bStatus) ? ("Busy") : ("Done"))
            );
        LOG_INFO("---------------------------------\n");
    }
    //=========================================================================

    return ret;
}

#endif //#if defined(EXTEND_IOCTL_COMMAND_TIL)
#if defined(EXTEND_IOCTL_COMMAND_TIL)
//-----------------------------------------------------------------------------
STF_RESULT Test_TIL_1_SetWindowSize(
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
    u32Offset = ADDR_REG_ITIIWSR;
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
        ST_ISP_TIL_RDMA stIspTilRdma;

        stIspTilRdma.stTilRdma.stRdmaStartCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaStartCmd.RegAddr = ADDR_REG_ITIIWSR;
        stIspTilRdma.stTilRdma.stRdmaStartCmd.u32Param = ST_TIL_1_WS_REG_LEN;
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrSdCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrSdCmd.RegAddr = ADDR_REG_ITIDWLSR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrSdCmd.u32Param =
            ST_TIL_1_WR_SD_REG_LEN;
#if defined(V4L2_DRIVER)
#else
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrAddrCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrAddrCmd.RegAddr =
            ADDR_REG_ITIDWYSAR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrAddrCmd.u32Param =
            ST_TIL_1_WR_ADDR_REG_LEN;
#endif //#if defined(V4L2_DRIVER)
#if defined(V4L2_DRIVER)
#else
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdAddrCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdAddrCmd.RegAddr =
            ADDR_REG_ITIDRYSAR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdAddrCmd.u32Param =
            ST_TIL_1_RD_ADDR_REG_LEN;
#endif //#if defined(V4L2_DRIVER)
        stIspTilRdma.stTilRdma.stRdmaTil_1_PDFCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_PDFCmd.RegAddr = ADDR_REG_ITIPDFR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_PDFCmd.u32Param =
            ST_TIL_1_PDF_REG_LEN;
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdSdCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdSdCmd.RegAddr = ADDR_REG_ITIDRLSR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdSdCmd.u32Param =
            ST_TIL_1_RD_SD_REG_LEN;
#if defined(V4L2_DRIVER)
#else
        stIspTilRdma.stTilRdma.stRdmaTil_1_BSCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_BSCmd.RegAddr = ADDR_REG_ITIBSR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_BSCmd.u32Param = ST_TIL_1_BS_REG_LEN;
#endif //#if defined(V4L2_DRIVER)
        stIspTilRdma.stTilRdma.stRdmaTil_1_AxiCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_AxiCmd.RegAddr = ADDR_REG_ITIAIR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_AxiCmd.u32Param =
            ST_TIL_1_AXI_REG_LEN;
        stIspTilRdma.stTilRdma.stRdmaTil_1_DPSCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_DPSCmd.RegAddr = ADDR_REG_ITIDPSR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_DPSCmd.u32Param =
            ST_TIL_1_DPS_REG_LEN;
        stIspTilRdma.stTilRdma.stRdmaEndCmd.Tag = DESP_END;

        stIspTilRdma.stTilRdma.stTil_1_WSReg.Til_1_WindowSize.value =
            (rand() & 0xFFFF) | ((rand() & 0xFFFF) << 16);
        stIspTilRdma.u8IspIdx = 0;
        ret = STFDRV_ISP_TIL_1_SetWindowSize(
            pstCiConnection,
            &stIspTilRdma
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_TIL_1_SetWindowSize()\n",
            stIspTilRdma.u8IspIdx);
        LOG_INFO("Window size = 0x%08X - Width=0x%04X(%d), Height=0x%04X(%d)\n",
            stIspTilRdma.stTilRdma.stTil_1_WSReg.Til_1_WindowSize.value,
            stIspTilRdma.stTilRdma.stTil_1_WSReg.Til_1_WindowSize.field.wsize,
            stIspTilRdma.stTilRdma.stTil_1_WSReg.Til_1_WindowSize.field.wsize,
            stIspTilRdma.stTilRdma.stTil_1_WSReg.Til_1_WindowSize.field.hsize,
            stIspTilRdma.stTilRdma.stTil_1_WSReg.Til_1_WindowSize.field.hsize
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_ITIIWSR;
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

STF_RESULT Test_TIL_1_SetReadBufAddr(
    STF_VOID
    )
{
    int i;
    //STF_U8 u8Method;
    STF_U32 u32Offset;
    STF_U32 u32Length;
    STF_U32 u32Buffer[2];
    CI_CONNECTION *pstCiConnection = NULL;
    STF_RESULT ret = STF_SUCCESS;

    pstCiConnection = g_pstCiConnection;

    //=========================================================================
    u32Offset = ADDR_REG_ITIDRYSAR;
    u32Length = 2;
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
        LOG_INFO("Reg[0x%08X] = 0x%08X\n", u32Offset + i * 4, u32Buffer[i]);
    }
    LOG_INFO("---------------------------------\n");
    //-------------------------------------------------------------------------
    {
        ST_ISP_TIL_RDMA stIspTilRdma;

        stIspTilRdma.stTilRdma.stRdmaStartCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaStartCmd.RegAddr = ADDR_REG_ITIIWSR;
        stIspTilRdma.stTilRdma.stRdmaStartCmd.u32Param = ST_TIL_1_WS_REG_LEN;
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrSdCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrSdCmd.RegAddr = ADDR_REG_ITIDWLSR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrSdCmd.u32Param =
            ST_TIL_1_WR_SD_REG_LEN;
#if defined(V4L2_DRIVER)
#else
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrAddrCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrAddrCmd.RegAddr =
            ADDR_REG_ITIDWYSAR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrAddrCmd.u32Param =
            ST_TIL_1_WR_ADDR_REG_LEN;
#endif //#if defined(V4L2_DRIVER)
#if defined(V4L2_DRIVER)
#else
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdAddrCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdAddrCmd.RegAddr =
            ADDR_REG_ITIDRYSAR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdAddrCmd.u32Param =
            ST_TIL_1_RD_ADDR_REG_LEN;
#endif //#if defined(V4L2_DRIVER)
        stIspTilRdma.stTilRdma.stRdmaTil_1_PDFCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_PDFCmd.RegAddr = ADDR_REG_ITIPDFR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_PDFCmd.u32Param =
            ST_TIL_1_PDF_REG_LEN;
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdSdCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdSdCmd.RegAddr = ADDR_REG_ITIDRLSR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdSdCmd.u32Param =
            ST_TIL_1_RD_SD_REG_LEN;
#if defined(V4L2_DRIVER)
#else
        stIspTilRdma.stTilRdma.stRdmaTil_1_BSCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_BSCmd.RegAddr = ADDR_REG_ITIBSR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_BSCmd.u32Param = ST_TIL_1_BS_REG_LEN;
#endif //#if defined(V4L2_DRIVER)
        stIspTilRdma.stTilRdma.stRdmaTil_1_AxiCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_AxiCmd.RegAddr = ADDR_REG_ITIAIR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_AxiCmd.u32Param =
            ST_TIL_1_AXI_REG_LEN;
        stIspTilRdma.stTilRdma.stRdmaTil_1_DPSCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_DPSCmd.RegAddr = ADDR_REG_ITIDPSR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_DPSCmd.u32Param =
            ST_TIL_1_DPS_REG_LEN;
        stIspTilRdma.stTilRdma.stRdmaEndCmd.Tag = DESP_END;

#if defined(V4L2_DRIVER)
        LOG_ERROR("ISP SDK not support this ioctl for V4L2 driver!!!\n");
#else
        stIspTilRdma.stTilRdma.stTil_1_RdAddrReg.Til_1_ReadYAddr.value =
            (rand() & 0xFFFFFFF8);
        stIspTilRdma.stTilRdma.stTil_1_RdAddrReg.Til_1_ReadUvAddr.value =
            (rand() & 0xFFFFFFF8);
        stIspTilRdma.u8IspIdx = 0;
        ret = STFDRV_ISP_TIL_1_SetReadBufAddr(
            pstCiConnection,
            &stIspTilRdma
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_TIL_1_SetReadBufAddr()\n",
            stIspTilRdma.u8IspIdx);
        LOG_INFO("Read - Y_Addr = 0x%08X, UV_Addr = 0x%08X\n",
            stIspTilRdma.stTilRdma.stTil_1_RdAddrReg.Til_1_ReadYAddr.value,
            stIspTilRdma.stTilRdma.stTil_1_RdAddrReg.Til_1_ReadUvAddr.value
            );
#endif //#if defined(V4L2_DRIVER)
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_ITIDRYSAR;
    u32Length = 2;
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
        LOG_INFO("Reg[0x%08X] = 0x%08X\n", u32Offset + i * 4, u32Buffer[i]);
    }
    LOG_INFO("---------------------------------\n");
    //=========================================================================

    return ret;
}

STF_RESULT Test_TIL_1_SetReadBufStride(
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
    u32Offset = ADDR_REG_ITIDRLSR;
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
        ST_ISP_TIL_RDMA stIspTilRdma;

        stIspTilRdma.stTilRdma.stRdmaStartCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaStartCmd.RegAddr = ADDR_REG_ITIIWSR;
        stIspTilRdma.stTilRdma.stRdmaStartCmd.u32Param = ST_TIL_1_WS_REG_LEN;
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrSdCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrSdCmd.RegAddr = ADDR_REG_ITIDWLSR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrSdCmd.u32Param =
            ST_TIL_1_WR_SD_REG_LEN;
#if defined(V4L2_DRIVER)
#else
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrAddrCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrAddrCmd.RegAddr =
            ADDR_REG_ITIDWYSAR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrAddrCmd.u32Param =
            ST_TIL_1_WR_ADDR_REG_LEN;
#endif //#if defined(V4L2_DRIVER)
#if defined(V4L2_DRIVER)
#else
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdAddrCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdAddrCmd.RegAddr =
            ADDR_REG_ITIDRYSAR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdAddrCmd.u32Param =
            ST_TIL_1_RD_ADDR_REG_LEN;
#endif //#if defined(V4L2_DRIVER)
        stIspTilRdma.stTilRdma.stRdmaTil_1_PDFCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_PDFCmd.RegAddr = ADDR_REG_ITIPDFR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_PDFCmd.u32Param =
            ST_TIL_1_PDF_REG_LEN;
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdSdCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdSdCmd.RegAddr = ADDR_REG_ITIDRLSR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdSdCmd.u32Param =
            ST_TIL_1_RD_SD_REG_LEN;
#if defined(V4L2_DRIVER)
#else
        stIspTilRdma.stTilRdma.stRdmaTil_1_BSCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_BSCmd.RegAddr = ADDR_REG_ITIBSR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_BSCmd.u32Param = ST_TIL_1_BS_REG_LEN;
#endif //#if defined(V4L2_DRIVER)
        stIspTilRdma.stTilRdma.stRdmaTil_1_AxiCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_AxiCmd.RegAddr = ADDR_REG_ITIAIR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_AxiCmd.u32Param =
            ST_TIL_1_AXI_REG_LEN;
        stIspTilRdma.stTilRdma.stRdmaTil_1_DPSCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_DPSCmd.RegAddr = ADDR_REG_ITIDPSR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_DPSCmd.u32Param =
            ST_TIL_1_DPS_REG_LEN;
        stIspTilRdma.stTilRdma.stRdmaEndCmd.Tag = DESP_END;

        stIspTilRdma.stTilRdma.stTil_1_RdSdReg.Til_1_ReadStride.value =
            (rand() & 0x007FFFF8);
        stIspTilRdma.u8IspIdx = 0;
        ret = STFDRV_ISP_TIL_1_SetReadBufStride(
            pstCiConnection,
            &stIspTilRdma
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_TIL_1_SetReadBufStride()\n",
            stIspTilRdma.u8IspIdx);
        LOG_INFO("Read - Stride = 0x%08X\n",
            stIspTilRdma.stTilRdma.stTil_1_RdSdReg.Til_1_ReadStride.value
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_ITIDRLSR;
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

STF_RESULT Test_TIL_1_SetReadDataPath(
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
    u32Offset = ADDR_REG_ITIDPSR;
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
        ST_ISP_TIL_RDMA stIspTilRdma;

        stIspTilRdma.stTilRdma.stRdmaStartCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaStartCmd.RegAddr = ADDR_REG_ITIIWSR;
        stIspTilRdma.stTilRdma.stRdmaStartCmd.u32Param = ST_TIL_1_WS_REG_LEN;
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrSdCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrSdCmd.RegAddr = ADDR_REG_ITIDWLSR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrSdCmd.u32Param =
            ST_TIL_1_WR_SD_REG_LEN;
#if defined(V4L2_DRIVER)
#else
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrAddrCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrAddrCmd.RegAddr =
            ADDR_REG_ITIDWYSAR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrAddrCmd.u32Param =
            ST_TIL_1_WR_ADDR_REG_LEN;
#endif //#if defined(V4L2_DRIVER)
#if defined(V4L2_DRIVER)
#else
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdAddrCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdAddrCmd.RegAddr =
            ADDR_REG_ITIDRYSAR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdAddrCmd.u32Param =
            ST_TIL_1_RD_ADDR_REG_LEN;
#endif //#if defined(V4L2_DRIVER)
        stIspTilRdma.stTilRdma.stRdmaTil_1_PDFCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_PDFCmd.RegAddr = ADDR_REG_ITIPDFR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_PDFCmd.u32Param =
            ST_TIL_1_PDF_REG_LEN;
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdSdCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdSdCmd.RegAddr = ADDR_REG_ITIDRLSR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdSdCmd.u32Param =
            ST_TIL_1_RD_SD_REG_LEN;
#if defined(V4L2_DRIVER)
#else
        stIspTilRdma.stTilRdma.stRdmaTil_1_BSCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_BSCmd.RegAddr = ADDR_REG_ITIBSR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_BSCmd.u32Param = ST_TIL_1_BS_REG_LEN;
#endif //#if defined(V4L2_DRIVER)
        stIspTilRdma.stTilRdma.stRdmaTil_1_AxiCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_AxiCmd.RegAddr = ADDR_REG_ITIAIR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_AxiCmd.u32Param =
            ST_TIL_1_AXI_REG_LEN;
        stIspTilRdma.stTilRdma.stRdmaTil_1_DPSCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_DPSCmd.RegAddr = ADDR_REG_ITIDPSR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_DPSCmd.u32Param =
            ST_TIL_1_DPS_REG_LEN;
        stIspTilRdma.stTilRdma.stRdmaEndCmd.Tag = DESP_END;

        stIspTilRdma.stTilRdma.stTil_1_DPSReg.Til_1_PathSelect.value = 0;
        //stIspTilRdma.stTilRdma.stTil_1_DPSReg.Til_1_PathSelect.field.readindex =
        //    (rand() & 0xFF);
        stIspTilRdma.stTilRdma.stTil_1_DPSReg.Til_1_PathSelect.field.readindex =
            (rand() & 0x3);
        stIspTilRdma.u8IspIdx = 0;
        ret = STFDRV_ISP_TIL_1_SetReadDataPath(
            pstCiConnection,
            &stIspTilRdma
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_TIL_1_SetReadDataPath()\n",
            stIspTilRdma.u8IspIdx);
        LOG_INFO("DataPath = 0x%08X\n",
            stIspTilRdma.stTilRdma.stTil_1_DPSReg.Til_1_PathSelect.value
            );
        LOG_INFO("DataPath(Read) - Path = 0x%02X(%s)\n",
            stIspTilRdma.stTilRdma.stTil_1_DPSReg.Til_1_PathSelect.field.readindex,
            g_szTil_1_DataPath[stIspTilRdma.stTilRdma.stTil_1_DPSReg.Til_1_PathSelect.field.readindex]
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_ITIDPSR;
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

STF_RESULT Test_TIL_1_SetReadNv21(
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
    u32Offset = ADDR_REG_ITIDPSR;
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
        ST_ISP_TIL_RDMA stIspTilRdma;

        stIspTilRdma.stTilRdma.stRdmaStartCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaStartCmd.RegAddr = ADDR_REG_ITIIWSR;
        stIspTilRdma.stTilRdma.stRdmaStartCmd.u32Param = ST_TIL_1_WS_REG_LEN;
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrSdCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrSdCmd.RegAddr = ADDR_REG_ITIDWLSR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrSdCmd.u32Param =
            ST_TIL_1_WR_SD_REG_LEN;
#if defined(V4L2_DRIVER)
#else
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrAddrCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrAddrCmd.RegAddr =
            ADDR_REG_ITIDWYSAR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrAddrCmd.u32Param =
            ST_TIL_1_WR_ADDR_REG_LEN;
#endif //#if defined(V4L2_DRIVER)
#if defined(V4L2_DRIVER)
#else
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdAddrCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdAddrCmd.RegAddr =
            ADDR_REG_ITIDRYSAR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdAddrCmd.u32Param =
            ST_TIL_1_RD_ADDR_REG_LEN;
#endif //#if defined(V4L2_DRIVER)
        stIspTilRdma.stTilRdma.stRdmaTil_1_PDFCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_PDFCmd.RegAddr = ADDR_REG_ITIPDFR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_PDFCmd.u32Param =
            ST_TIL_1_PDF_REG_LEN;
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdSdCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdSdCmd.RegAddr = ADDR_REG_ITIDRLSR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdSdCmd.u32Param =
            ST_TIL_1_RD_SD_REG_LEN;
#if defined(V4L2_DRIVER)
#else
        stIspTilRdma.stTilRdma.stRdmaTil_1_BSCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_BSCmd.RegAddr = ADDR_REG_ITIBSR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_BSCmd.u32Param = ST_TIL_1_BS_REG_LEN;
#endif //#if defined(V4L2_DRIVER)
        stIspTilRdma.stTilRdma.stRdmaTil_1_AxiCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_AxiCmd.RegAddr = ADDR_REG_ITIAIR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_AxiCmd.u32Param =
            ST_TIL_1_AXI_REG_LEN;
        stIspTilRdma.stTilRdma.stRdmaTil_1_DPSCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_DPSCmd.RegAddr = ADDR_REG_ITIDPSR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_DPSCmd.u32Param =
            ST_TIL_1_DPS_REG_LEN;
        stIspTilRdma.stTilRdma.stRdmaEndCmd.Tag = DESP_END;

        stIspTilRdma.stTilRdma.stTil_1_DPSReg.Til_1_PathSelect.value = 0;
        stIspTilRdma.stTilRdma.stTil_1_DPSReg.Til_1_PathSelect.field.rdnv21format =
            (rand() & 0x1);
        stIspTilRdma.u8IspIdx = 0;
        ret = STFDRV_ISP_TIL_1_SetReadNv21(
            pstCiConnection,
            &stIspTilRdma
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_TIL_1_SetReadNv21()\n",
            stIspTilRdma.u8IspIdx);
        LOG_INFO("DataPath = 0x%08X\n",
            stIspTilRdma.stTilRdma.stTil_1_DPSReg.Til_1_PathSelect.value
            );
        LOG_INFO("DataPath(Read) - UvFormat = 0x%01X(%s)\n",
            stIspTilRdma.stTilRdma.stTil_1_DPSReg.Til_1_PathSelect.field.rdnv21format,
            ((stIspTilRdma.stTilRdma.stTil_1_DPSReg.Til_1_PathSelect.field.rdnv21format)
                ? ("NV21") : ("NV12"))
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_ITIDPSR;
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

STF_RESULT Test_TIL_1_SetReadDataFormat(
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
    u32Offset = ADDR_REG_ITIPDFR;
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
        ST_ISP_TIL_RDMA stIspTilRdma;

        stIspTilRdma.stTilRdma.stRdmaStartCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaStartCmd.RegAddr = ADDR_REG_ITIIWSR;
        stIspTilRdma.stTilRdma.stRdmaStartCmd.u32Param = ST_TIL_1_WS_REG_LEN;
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrSdCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrSdCmd.RegAddr = ADDR_REG_ITIDWLSR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrSdCmd.u32Param =
            ST_TIL_1_WR_SD_REG_LEN;
#if defined(V4L2_DRIVER)
#else
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrAddrCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrAddrCmd.RegAddr =
            ADDR_REG_ITIDWYSAR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrAddrCmd.u32Param =
            ST_TIL_1_WR_ADDR_REG_LEN;
#endif //#if defined(V4L2_DRIVER)
#if defined(V4L2_DRIVER)
#else
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdAddrCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdAddrCmd.RegAddr =
            ADDR_REG_ITIDRYSAR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdAddrCmd.u32Param =
            ST_TIL_1_RD_ADDR_REG_LEN;
#endif //#if defined(V4L2_DRIVER)
        stIspTilRdma.stTilRdma.stRdmaTil_1_PDFCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_PDFCmd.RegAddr = ADDR_REG_ITIPDFR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_PDFCmd.u32Param =
            ST_TIL_1_PDF_REG_LEN;
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdSdCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdSdCmd.RegAddr = ADDR_REG_ITIDRLSR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdSdCmd.u32Param =
            ST_TIL_1_RD_SD_REG_LEN;
#if defined(V4L2_DRIVER)
#else
        stIspTilRdma.stTilRdma.stRdmaTil_1_BSCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_BSCmd.RegAddr = ADDR_REG_ITIBSR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_BSCmd.u32Param = ST_TIL_1_BS_REG_LEN;
#endif //#if defined(V4L2_DRIVER)
        stIspTilRdma.stTilRdma.stRdmaTil_1_AxiCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_AxiCmd.RegAddr = ADDR_REG_ITIAIR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_AxiCmd.u32Param =
            ST_TIL_1_AXI_REG_LEN;
        stIspTilRdma.stTilRdma.stRdmaTil_1_DPSCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_DPSCmd.RegAddr = ADDR_REG_ITIDPSR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_DPSCmd.u32Param =
            ST_TIL_1_DPS_REG_LEN;
        stIspTilRdma.stTilRdma.stRdmaEndCmd.Tag = DESP_END;

        stIspTilRdma.stTilRdma.stTil_1_PDFReg.Til_1_DataFormat.value = 0;
        stIspTilRdma.stTilRdma.stTil_1_PDFReg.Til_1_DataFormat.field.rdformat =
            (rand() & 0x7);
        stIspTilRdma.u8IspIdx = 0;
        ret = STFDRV_ISP_TIL_1_SetReadDataFormat(
            pstCiConnection,
            &stIspTilRdma
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_TIL_1_SetReadDataFormat()\n",
            stIspTilRdma.u8IspIdx);
        LOG_INFO("DataFormat = 0x%08X\n",
            stIspTilRdma.stTilRdma.stTil_1_PDFReg.Til_1_DataFormat.value
            );
        LOG_INFO("DataFormat(Read) - Format = 0x%01X(%s)\n",
            stIspTilRdma.stTilRdma.stTil_1_PDFReg.Til_1_DataFormat.field.rdformat,
            g_szFormat[stIspTilRdma.stTilRdma.stTil_1_PDFReg.Til_1_DataFormat.field.rdformat]
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_ITIPDFR;
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

STF_RESULT Test_TIL_1_SetReadFlexibleBits(
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
    u32Offset = ADDR_REG_ITIPDFR;
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
        ST_ISP_TIL_RDMA stIspTilRdma;

        stIspTilRdma.stTilRdma.stRdmaStartCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaStartCmd.RegAddr = ADDR_REG_ITIIWSR;
        stIspTilRdma.stTilRdma.stRdmaStartCmd.u32Param = ST_TIL_1_WS_REG_LEN;
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrSdCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrSdCmd.RegAddr = ADDR_REG_ITIDWLSR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrSdCmd.u32Param =
            ST_TIL_1_WR_SD_REG_LEN;
#if defined(V4L2_DRIVER)
#else
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrAddrCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrAddrCmd.RegAddr =
            ADDR_REG_ITIDWYSAR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrAddrCmd.u32Param =
            ST_TIL_1_WR_ADDR_REG_LEN;
#endif //#if defined(V4L2_DRIVER)
#if defined(V4L2_DRIVER)
#else
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdAddrCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdAddrCmd.RegAddr =
            ADDR_REG_ITIDRYSAR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdAddrCmd.u32Param =
            ST_TIL_1_RD_ADDR_REG_LEN;
#endif //#if defined(V4L2_DRIVER)
        stIspTilRdma.stTilRdma.stRdmaTil_1_PDFCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_PDFCmd.RegAddr = ADDR_REG_ITIPDFR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_PDFCmd.u32Param =
            ST_TIL_1_PDF_REG_LEN;
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdSdCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdSdCmd.RegAddr = ADDR_REG_ITIDRLSR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdSdCmd.u32Param =
            ST_TIL_1_RD_SD_REG_LEN;
#if defined(V4L2_DRIVER)
#else
        stIspTilRdma.stTilRdma.stRdmaTil_1_BSCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_BSCmd.RegAddr = ADDR_REG_ITIBSR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_BSCmd.u32Param = ST_TIL_1_BS_REG_LEN;
#endif //#if defined(V4L2_DRIVER)
        stIspTilRdma.stTilRdma.stRdmaTil_1_AxiCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_AxiCmd.RegAddr = ADDR_REG_ITIAIR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_AxiCmd.u32Param =
            ST_TIL_1_AXI_REG_LEN;
        stIspTilRdma.stTilRdma.stRdmaTil_1_DPSCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_DPSCmd.RegAddr = ADDR_REG_ITIDPSR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_DPSCmd.u32Param =
            ST_TIL_1_DPS_REG_LEN;
        stIspTilRdma.stTilRdma.stRdmaEndCmd.Tag = DESP_END;

        stIspTilRdma.stTilRdma.stTil_1_PDFReg.Til_1_DataFormat.value = 0;
        stIspTilRdma.stTilRdma.stTil_1_PDFReg.Til_1_DataFormat.field.rdflexiblebit =
            (rand() & 0x1F);
        stIspTilRdma.u8IspIdx = 0;
        ret = STFDRV_ISP_TIL_1_SetReadFlexibleBits(
            pstCiConnection,
            &stIspTilRdma
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_TIL_1_SetReadFlexibleBits()\n",
            stIspTilRdma.u8IspIdx);
        LOG_INFO("DataFormat = 0x%08X\n",
            stIspTilRdma.stTilRdma.stTil_1_PDFReg.Til_1_DataFormat.value
            );
        LOG_INFO("DataFormat(Read) - FlexibleBit = 0x%02X\n",
            stIspTilRdma.stTilRdma.stTil_1_PDFReg.Til_1_DataFormat.field.rdflexiblebit
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_ITIPDFR;
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

STF_RESULT Test_TIL_1_SetReadIndependentRvi(
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
    u32Offset = ADDR_REG_ITIDPSR;
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
        ST_ISP_TIL_RDMA stIspTilRdma;

        stIspTilRdma.stTilRdma.stRdmaStartCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaStartCmd.RegAddr = ADDR_REG_ITIIWSR;
        stIspTilRdma.stTilRdma.stRdmaStartCmd.u32Param = ST_TIL_1_WS_REG_LEN;
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrSdCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrSdCmd.RegAddr = ADDR_REG_ITIDWLSR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrSdCmd.u32Param =
            ST_TIL_1_WR_SD_REG_LEN;
#if defined(V4L2_DRIVER)
#else
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrAddrCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrAddrCmd.RegAddr =
            ADDR_REG_ITIDWYSAR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrAddrCmd.u32Param =
            ST_TIL_1_WR_ADDR_REG_LEN;
#endif //#if defined(V4L2_DRIVER)
#if defined(V4L2_DRIVER)
#else
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdAddrCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdAddrCmd.RegAddr =
            ADDR_REG_ITIDRYSAR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdAddrCmd.u32Param =
            ST_TIL_1_RD_ADDR_REG_LEN;
#endif //#if defined(V4L2_DRIVER)
        stIspTilRdma.stTilRdma.stRdmaTil_1_PDFCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_PDFCmd.RegAddr = ADDR_REG_ITIPDFR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_PDFCmd.u32Param =
            ST_TIL_1_PDF_REG_LEN;
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdSdCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdSdCmd.RegAddr = ADDR_REG_ITIDRLSR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdSdCmd.u32Param =
            ST_TIL_1_RD_SD_REG_LEN;
#if defined(V4L2_DRIVER)
#else
        stIspTilRdma.stTilRdma.stRdmaTil_1_BSCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_BSCmd.RegAddr = ADDR_REG_ITIBSR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_BSCmd.u32Param = ST_TIL_1_BS_REG_LEN;
#endif //#if defined(V4L2_DRIVER)
        stIspTilRdma.stTilRdma.stRdmaTil_1_AxiCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_AxiCmd.RegAddr = ADDR_REG_ITIAIR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_AxiCmd.u32Param =
            ST_TIL_1_AXI_REG_LEN;
        stIspTilRdma.stTilRdma.stRdmaTil_1_DPSCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_DPSCmd.RegAddr = ADDR_REG_ITIDPSR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_DPSCmd.u32Param =
            ST_TIL_1_DPS_REG_LEN;
        stIspTilRdma.stTilRdma.stRdmaEndCmd.Tag = DESP_END;

        stIspTilRdma.stTilRdma.stTil_1_DPSReg.Til_1_PathSelect.value = 0;
        stIspTilRdma.stTilRdma.stTil_1_DPSReg.Til_1_PathSelect.field.independentrdrvi =
            (rand() & 0x1);
        stIspTilRdma.u8IspIdx = 0;
        ret = STFDRV_ISP_TIL_1_SetReadIndependentRvi(
            pstCiConnection,
            &stIspTilRdma
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_TIL_1_SetReadIndependentRvi()\n",
            stIspTilRdma.u8IspIdx);
        LOG_INFO("DataPath = 0x%08X\n",
            stIspTilRdma.stTilRdma.stTil_1_DPSReg.Til_1_PathSelect.value
            );
        LOG_INFO("DataPath(Read) - IndependentRvi = 0x%01X(%s)\n",
            stIspTilRdma.stTilRdma.stTil_1_DPSReg.Til_1_PathSelect.field.independentrdrvi,
            ((stIspTilRdma.stTilRdma.stTil_1_DPSReg.Til_1_PathSelect.field.independentrdrvi)
                ? ("Enable") : ("Disable"))
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_ITIDPSR;
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

STF_RESULT Test_TIL_1_SetReadLineBufOff(
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
    u32Offset = ADDR_REG_ITIPDFR;
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
        ST_ISP_TIL_RDMA stIspTilRdma;

        stIspTilRdma.stTilRdma.stRdmaStartCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaStartCmd.RegAddr = ADDR_REG_ITIIWSR;
        stIspTilRdma.stTilRdma.stRdmaStartCmd.u32Param = ST_TIL_1_WS_REG_LEN;
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrSdCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrSdCmd.RegAddr = ADDR_REG_ITIDWLSR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrSdCmd.u32Param =
            ST_TIL_1_WR_SD_REG_LEN;
#if defined(V4L2_DRIVER)
#else
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrAddrCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrAddrCmd.RegAddr =
            ADDR_REG_ITIDWYSAR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrAddrCmd.u32Param =
            ST_TIL_1_WR_ADDR_REG_LEN;
#endif //#if defined(V4L2_DRIVER)
#if defined(V4L2_DRIVER)
#else
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdAddrCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdAddrCmd.RegAddr =
            ADDR_REG_ITIDRYSAR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdAddrCmd.u32Param =
            ST_TIL_1_RD_ADDR_REG_LEN;
#endif //#if defined(V4L2_DRIVER)
        stIspTilRdma.stTilRdma.stRdmaTil_1_PDFCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_PDFCmd.RegAddr = ADDR_REG_ITIPDFR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_PDFCmd.u32Param =
            ST_TIL_1_PDF_REG_LEN;
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdSdCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdSdCmd.RegAddr = ADDR_REG_ITIDRLSR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdSdCmd.u32Param =
            ST_TIL_1_RD_SD_REG_LEN;
#if defined(V4L2_DRIVER)
#else
        stIspTilRdma.stTilRdma.stRdmaTil_1_BSCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_BSCmd.RegAddr = ADDR_REG_ITIBSR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_BSCmd.u32Param = ST_TIL_1_BS_REG_LEN;
#endif //#if defined(V4L2_DRIVER)
        stIspTilRdma.stTilRdma.stRdmaTil_1_AxiCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_AxiCmd.RegAddr = ADDR_REG_ITIAIR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_AxiCmd.u32Param =
            ST_TIL_1_AXI_REG_LEN;
        stIspTilRdma.stTilRdma.stRdmaTil_1_DPSCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_DPSCmd.RegAddr = ADDR_REG_ITIDPSR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_DPSCmd.u32Param =
            ST_TIL_1_DPS_REG_LEN;
        stIspTilRdma.stTilRdma.stRdmaEndCmd.Tag = DESP_END;

        stIspTilRdma.stTilRdma.stTil_1_PDFReg.Til_1_DataFormat.value = 0;
        stIspTilRdma.stTilRdma.stTil_1_PDFReg.Til_1_DataFormat.field.rdlinebufoff =
            (rand() & 0x1);
        stIspTilRdma.u8IspIdx = 0;
        ret = STFDRV_ISP_TIL_1_SetReadLineBufOff(
            pstCiConnection,
            &stIspTilRdma
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_TIL_1_SetReadLineBufOff()\n",
            stIspTilRdma.u8IspIdx);
        LOG_INFO("DataFormat = 0x%08X\n",
            stIspTilRdma.stTilRdma.stTil_1_PDFReg.Til_1_DataFormat.value
            );
        LOG_INFO("DataFormat(Read) - UvLineBuf = 0x%1X(%s)\n",
            stIspTilRdma.stTilRdma.stTil_1_PDFReg.Til_1_DataFormat.field.rdlinebufoff,
            ((stIspTilRdma.stTilRdma.stTil_1_PDFReg.Til_1_DataFormat.field.rdlinebufoff)
                ? ("Disable") : ("Enable"))
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_ITIPDFR;
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

STF_RESULT Test_TIL_1_SetReadAxiId(
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
    u32Offset = ADDR_REG_ITIAIR;
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
        ST_ISP_TIL_RDMA stIspTilRdma;

        stIspTilRdma.stTilRdma.stRdmaStartCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaStartCmd.RegAddr = ADDR_REG_ITIIWSR;
        stIspTilRdma.stTilRdma.stRdmaStartCmd.u32Param = ST_TIL_1_WS_REG_LEN;
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrSdCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrSdCmd.RegAddr = ADDR_REG_ITIDWLSR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrSdCmd.u32Param =
            ST_TIL_1_WR_SD_REG_LEN;
#if defined(V4L2_DRIVER)
#else
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrAddrCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrAddrCmd.RegAddr =
            ADDR_REG_ITIDWYSAR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrAddrCmd.u32Param =
            ST_TIL_1_WR_ADDR_REG_LEN;
#endif //#if defined(V4L2_DRIVER)
#if defined(V4L2_DRIVER)
#else
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdAddrCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdAddrCmd.RegAddr =
            ADDR_REG_ITIDRYSAR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdAddrCmd.u32Param =
            ST_TIL_1_RD_ADDR_REG_LEN;
#endif //#if defined(V4L2_DRIVER)
        stIspTilRdma.stTilRdma.stRdmaTil_1_PDFCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_PDFCmd.RegAddr = ADDR_REG_ITIPDFR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_PDFCmd.u32Param =
            ST_TIL_1_PDF_REG_LEN;
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdSdCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdSdCmd.RegAddr = ADDR_REG_ITIDRLSR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdSdCmd.u32Param =
            ST_TIL_1_RD_SD_REG_LEN;
#if defined(V4L2_DRIVER)
#else
        stIspTilRdma.stTilRdma.stRdmaTil_1_BSCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_BSCmd.RegAddr = ADDR_REG_ITIBSR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_BSCmd.u32Param = ST_TIL_1_BS_REG_LEN;
#endif //#if defined(V4L2_DRIVER)
        stIspTilRdma.stTilRdma.stRdmaTil_1_AxiCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_AxiCmd.RegAddr = ADDR_REG_ITIAIR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_AxiCmd.u32Param =
            ST_TIL_1_AXI_REG_LEN;
        stIspTilRdma.stTilRdma.stRdmaTil_1_DPSCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_DPSCmd.RegAddr = ADDR_REG_ITIDPSR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_DPSCmd.u32Param =
            ST_TIL_1_DPS_REG_LEN;
        stIspTilRdma.stTilRdma.stRdmaEndCmd.Tag = DESP_END;

        stIspTilRdma.stTilRdma.stTil_1_AxiReg.Til_1_AxiId.value = 0;
        stIspTilRdma.stTilRdma.stTil_1_AxiReg.Til_1_AxiId.field.yrid =
            (rand() & 0xFF);
        stIspTilRdma.stTilRdma.stTil_1_AxiReg.Til_1_AxiId.field.uvrid =
            (rand() & 0xFF);
        stIspTilRdma.u8IspIdx = 0;
        ret = STFDRV_ISP_TIL_1_SetReadAxiId(
            pstCiConnection,
            &stIspTilRdma
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_TIL_1_SetReadAxiId()\n",
            stIspTilRdma.u8IspIdx);
        LOG_INFO("AxiId = 0x%08X\n",
            stIspTilRdma.stTilRdma.stTil_1_AxiReg.Til_1_AxiId.value
            );
        LOG_INFO("AxiId(Read) - Y_AxiId = 0x%02X, Uv_AxiId = 0x%02X\n",
            stIspTilRdma.stTilRdma.stTil_1_AxiReg.Til_1_AxiId.field.yrid,
            stIspTilRdma.stTilRdma.stTil_1_AxiReg.Til_1_AxiId.field.uvrid
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_ITIAIR;
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

STF_RESULT Test_TIL_1_SetReadEnable(
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
    u32Offset = ADDR_REG_ITIDPSR;
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
        ST_ISP_TIL_RDMA stIspTilRdma;

        stIspTilRdma.stTilRdma.stRdmaStartCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaStartCmd.RegAddr = ADDR_REG_ITIIWSR;
        stIspTilRdma.stTilRdma.stRdmaStartCmd.u32Param = ST_TIL_1_WS_REG_LEN;
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrSdCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrSdCmd.RegAddr = ADDR_REG_ITIDWLSR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrSdCmd.u32Param =
            ST_TIL_1_WR_SD_REG_LEN;
#if defined(V4L2_DRIVER)
#else
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrAddrCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrAddrCmd.RegAddr =
            ADDR_REG_ITIDWYSAR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrAddrCmd.u32Param =
            ST_TIL_1_WR_ADDR_REG_LEN;
#endif //#if defined(V4L2_DRIVER)
#if defined(V4L2_DRIVER)
#else
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdAddrCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdAddrCmd.RegAddr =
            ADDR_REG_ITIDRYSAR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdAddrCmd.u32Param =
            ST_TIL_1_RD_ADDR_REG_LEN;
#endif //#if defined(V4L2_DRIVER)
        stIspTilRdma.stTilRdma.stRdmaTil_1_PDFCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_PDFCmd.RegAddr = ADDR_REG_ITIPDFR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_PDFCmd.u32Param =
            ST_TIL_1_PDF_REG_LEN;
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdSdCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdSdCmd.RegAddr = ADDR_REG_ITIDRLSR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdSdCmd.u32Param =
            ST_TIL_1_RD_SD_REG_LEN;
#if defined(V4L2_DRIVER)
#else
        stIspTilRdma.stTilRdma.stRdmaTil_1_BSCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_BSCmd.RegAddr = ADDR_REG_ITIBSR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_BSCmd.u32Param = ST_TIL_1_BS_REG_LEN;
#endif //#if defined(V4L2_DRIVER)
        stIspTilRdma.stTilRdma.stRdmaTil_1_AxiCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_AxiCmd.RegAddr = ADDR_REG_ITIAIR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_AxiCmd.u32Param =
            ST_TIL_1_AXI_REG_LEN;
        stIspTilRdma.stTilRdma.stRdmaTil_1_DPSCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_DPSCmd.RegAddr = ADDR_REG_ITIDPSR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_DPSCmd.u32Param =
            ST_TIL_1_DPS_REG_LEN;
        stIspTilRdma.stTilRdma.stRdmaEndCmd.Tag = DESP_END;

        stIspTilRdma.stTilRdma.stTil_1_DPSReg.Til_1_PathSelect.value = 0;
        stIspTilRdma.stTilRdma.stTil_1_DPSReg.Til_1_PathSelect.field.readdram =
            (rand() & 0x1);
        stIspTilRdma.u8IspIdx = 0;
        ret = STFDRV_ISP_TIL_1_SetReadEnable(
            pstCiConnection,
            &stIspTilRdma
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_TIL_1_SetReadEnable()\n",
            stIspTilRdma.u8IspIdx);
        LOG_INFO("DataPath = 0x%08X\n",
            stIspTilRdma.stTilRdma.stTil_1_DPSReg.Til_1_PathSelect.value
            );
        LOG_INFO("DataPath(Read) - Enable = 0x%01X(%s)\n",
            stIspTilRdma.stTilRdma.stTil_1_DPSReg.Til_1_PathSelect.field.readdram,
            ((stIspTilRdma.stTilRdma.stTil_1_DPSReg.Til_1_PathSelect.field.readdram)
                ? ("Enable") : ("Disable"))
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_ITIDPSR;
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

STF_RESULT Test_TIL_1_SetWriteBufAddr(
    STF_VOID
    )
{
    int i;
    //STF_U8 u8Method;
    STF_U32 u32Offset;
    STF_U32 u32Length;
    STF_U32 u32Buffer[2];
    CI_CONNECTION *pstCiConnection = NULL;
    STF_RESULT ret = STF_SUCCESS;

    pstCiConnection = g_pstCiConnection;

    //=========================================================================
    u32Offset = ADDR_REG_ITIDWYSAR;
    u32Length = 2;
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
            ADDR_REG_ITIDWYSAR+i*4, u32Buffer[i]);
    }
    LOG_INFO("---------------------------------\n");
    //-------------------------------------------------------------------------
    {
        ST_ISP_TIL_RDMA stIspTilRdma;

        stIspTilRdma.stTilRdma.stRdmaStartCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaStartCmd.RegAddr = ADDR_REG_ITIIWSR;
        stIspTilRdma.stTilRdma.stRdmaStartCmd.u32Param = ST_TIL_1_WS_REG_LEN;
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrSdCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrSdCmd.RegAddr = ADDR_REG_ITIDWLSR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrSdCmd.u32Param =
            ST_TIL_1_WR_SD_REG_LEN;
#if defined(V4L2_DRIVER)
#else
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrAddrCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrAddrCmd.RegAddr =
            ADDR_REG_ITIDWYSAR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrAddrCmd.u32Param =
            ST_TIL_1_WR_ADDR_REG_LEN;
#endif //#if defined(V4L2_DRIVER)
#if defined(V4L2_DRIVER)
#else
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdAddrCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdAddrCmd.RegAddr =
            ADDR_REG_ITIDRYSAR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdAddrCmd.u32Param =
            ST_TIL_1_RD_ADDR_REG_LEN;
#endif //#if defined(V4L2_DRIVER)
        stIspTilRdma.stTilRdma.stRdmaTil_1_PDFCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_PDFCmd.RegAddr = ADDR_REG_ITIPDFR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_PDFCmd.u32Param =
            ST_TIL_1_PDF_REG_LEN;
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdSdCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdSdCmd.RegAddr = ADDR_REG_ITIDRLSR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdSdCmd.u32Param =
            ST_TIL_1_RD_SD_REG_LEN;
#if defined(V4L2_DRIVER)
#else
        stIspTilRdma.stTilRdma.stRdmaTil_1_BSCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_BSCmd.RegAddr = ADDR_REG_ITIBSR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_BSCmd.u32Param = ST_TIL_1_BS_REG_LEN;
#endif //#if defined(V4L2_DRIVER)
        stIspTilRdma.stTilRdma.stRdmaTil_1_AxiCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_AxiCmd.RegAddr = ADDR_REG_ITIAIR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_AxiCmd.u32Param =
            ST_TIL_1_AXI_REG_LEN;
        stIspTilRdma.stTilRdma.stRdmaTil_1_DPSCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_DPSCmd.RegAddr = ADDR_REG_ITIDPSR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_DPSCmd.u32Param =
            ST_TIL_1_DPS_REG_LEN;
        stIspTilRdma.stTilRdma.stRdmaEndCmd.Tag = DESP_END;

#if defined(V4L2_DRIVER)
        LOG_ERROR("ISP SDK not support this ioctl for V4L2 driver!!!\n");
#else
        stIspTilRdma.stTilRdma.stTil_1_WrAddrReg.Til_1_WriteYAddr.value =
            (rand() & 0xFFFFFFF8);
        stIspTilRdma.stTilRdma.stTil_1_WrAddrReg.Til_1_WriteUvAddr.value =
            (rand() & 0xFFFFFFF8);
        stIspTilRdma.u8IspIdx = 0;
        ret = STFDRV_ISP_TIL_1_SetWriteBufAddr(
            pstCiConnection,
            &stIspTilRdma
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_TIL_1_SetWriteBufAddr()\n",
            stIspTilRdma.u8IspIdx);
        LOG_INFO("Write - Y_Addr = 0x%08X, UV_Addr = 0x%08X\n",
            stIspTilRdma.stTilRdma.stTil_1_WrAddrReg.Til_1_WriteYAddr.value,
            stIspTilRdma.stTilRdma.stTil_1_WrAddrReg.Til_1_WriteUvAddr.value
            );
#endif //#if defined(V4L2_DRIVER)
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_ITIDWYSAR;
    u32Length = 2;
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
            ADDR_REG_ITIDWYSAR+i*4, u32Buffer[i]);
    }
    LOG_INFO("---------------------------------\n");
    //=========================================================================

    return ret;
}

STF_RESULT Test_TIL_1_SetWriteBufStride(
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
    u32Offset = ADDR_REG_ITIDWLSR;
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
        ST_ISP_TIL_RDMA stIspTilRdma;

        stIspTilRdma.stTilRdma.stRdmaStartCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaStartCmd.RegAddr = ADDR_REG_ITIIWSR;
        stIspTilRdma.stTilRdma.stRdmaStartCmd.u32Param = ST_TIL_1_WS_REG_LEN;
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrSdCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrSdCmd.RegAddr = ADDR_REG_ITIDWLSR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrSdCmd.u32Param =
            ST_TIL_1_WR_SD_REG_LEN;
#if defined(V4L2_DRIVER)
#else
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrAddrCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrAddrCmd.RegAddr =
            ADDR_REG_ITIDWYSAR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrAddrCmd.u32Param =
            ST_TIL_1_WR_ADDR_REG_LEN;
#endif //#if defined(V4L2_DRIVER)
#if defined(V4L2_DRIVER)
#else
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdAddrCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdAddrCmd.RegAddr =
            ADDR_REG_ITIDRYSAR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdAddrCmd.u32Param =
            ST_TIL_1_RD_ADDR_REG_LEN;
#endif //#if defined(V4L2_DRIVER)
        stIspTilRdma.stTilRdma.stRdmaTil_1_PDFCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_PDFCmd.RegAddr = ADDR_REG_ITIPDFR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_PDFCmd.u32Param =
            ST_TIL_1_PDF_REG_LEN;
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdSdCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdSdCmd.RegAddr = ADDR_REG_ITIDRLSR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdSdCmd.u32Param =
            ST_TIL_1_RD_SD_REG_LEN;
#if defined(V4L2_DRIVER)
#else
        stIspTilRdma.stTilRdma.stRdmaTil_1_BSCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_BSCmd.RegAddr = ADDR_REG_ITIBSR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_BSCmd.u32Param = ST_TIL_1_BS_REG_LEN;
#endif //#if defined(V4L2_DRIVER)
        stIspTilRdma.stTilRdma.stRdmaTil_1_AxiCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_AxiCmd.RegAddr = ADDR_REG_ITIAIR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_AxiCmd.u32Param =
            ST_TIL_1_AXI_REG_LEN;
        stIspTilRdma.stTilRdma.stRdmaTil_1_DPSCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_DPSCmd.RegAddr = ADDR_REG_ITIDPSR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_DPSCmd.u32Param =
            ST_TIL_1_DPS_REG_LEN;
        stIspTilRdma.stTilRdma.stRdmaEndCmd.Tag = DESP_END;

        stIspTilRdma.stTilRdma.stTil_1_WrSdReg.Til_1_WriteStride.value =
            (rand() & 0x007FFFF8);
        stIspTilRdma.u8IspIdx = 0;
        ret = STFDRV_ISP_TIL_1_SetWriteBufStride(
            pstCiConnection,
            &stIspTilRdma
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_TIL_1_SetWriteBufStride()\n",
            stIspTilRdma.u8IspIdx);
        LOG_INFO("Write - Stride = 0x%08X\n",
            stIspTilRdma.stTilRdma.stTil_1_WrSdReg.Til_1_WriteStride.value
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_ITIDWLSR;
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

STF_RESULT Test_TIL_1_SetWriteDataPath(
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
    u32Offset = ADDR_REG_ITIDPSR;
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
        ST_ISP_TIL_RDMA stIspTilRdma;

        stIspTilRdma.stTilRdma.stRdmaStartCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaStartCmd.RegAddr = ADDR_REG_ITIIWSR;
        stIspTilRdma.stTilRdma.stRdmaStartCmd.u32Param = ST_TIL_1_WS_REG_LEN;
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrSdCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrSdCmd.RegAddr = ADDR_REG_ITIDWLSR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrSdCmd.u32Param =
            ST_TIL_1_WR_SD_REG_LEN;
#if defined(V4L2_DRIVER)
#else
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrAddrCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrAddrCmd.RegAddr =
            ADDR_REG_ITIDWYSAR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrAddrCmd.u32Param =
            ST_TIL_1_WR_ADDR_REG_LEN;
#endif //#if defined(V4L2_DRIVER)
#if defined(V4L2_DRIVER)
#else
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdAddrCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdAddrCmd.RegAddr =
            ADDR_REG_ITIDRYSAR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdAddrCmd.u32Param =
            ST_TIL_1_RD_ADDR_REG_LEN;
#endif //#if defined(V4L2_DRIVER)
        stIspTilRdma.stTilRdma.stRdmaTil_1_PDFCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_PDFCmd.RegAddr = ADDR_REG_ITIPDFR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_PDFCmd.u32Param =
            ST_TIL_1_PDF_REG_LEN;
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdSdCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdSdCmd.RegAddr = ADDR_REG_ITIDRLSR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdSdCmd.u32Param =
            ST_TIL_1_RD_SD_REG_LEN;
#if defined(V4L2_DRIVER)
#else
        stIspTilRdma.stTilRdma.stRdmaTil_1_BSCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_BSCmd.RegAddr = ADDR_REG_ITIBSR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_BSCmd.u32Param = ST_TIL_1_BS_REG_LEN;
#endif //#if defined(V4L2_DRIVER)
        stIspTilRdma.stTilRdma.stRdmaTil_1_AxiCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_AxiCmd.RegAddr = ADDR_REG_ITIAIR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_AxiCmd.u32Param =
            ST_TIL_1_AXI_REG_LEN;
        stIspTilRdma.stTilRdma.stRdmaTil_1_DPSCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_DPSCmd.RegAddr = ADDR_REG_ITIDPSR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_DPSCmd.u32Param =
            ST_TIL_1_DPS_REG_LEN;
        stIspTilRdma.stTilRdma.stRdmaEndCmd.Tag = DESP_END;

        stIspTilRdma.stTilRdma.stTil_1_DPSReg.Til_1_PathSelect.value = 0;
        //stIspTilRdma.stTilRdma.stTil_1_DPSReg.Til_1_PathSelect.field.writeindex =
        //    (rand() & 0xFF);
        stIspTilRdma.stTilRdma.stTil_1_DPSReg.Til_1_PathSelect.field.writeindex =
            (rand() & 0x3);
        stIspTilRdma.u8IspIdx = 0;
        ret = STFDRV_ISP_TIL_1_SetWriteDataPath(
            pstCiConnection,
            &stIspTilRdma
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_TIL_1_SetWriteDataPath()\n",
            stIspTilRdma.u8IspIdx);
        LOG_INFO("DataPath = 0x%08X\n",
            stIspTilRdma.stTilRdma.stTil_1_DPSReg.Til_1_PathSelect.value
            );
        LOG_INFO("DataPath(Write) - Path = 0x%02X(%s)\n",
            stIspTilRdma.stTilRdma.stTil_1_DPSReg.Til_1_PathSelect.field.writeindex,
            g_szTil_1_DataPath[stIspTilRdma.stTilRdma.stTil_1_DPSReg.Til_1_PathSelect.field.writeindex]
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_ITIDPSR;
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

STF_RESULT Test_TIL_1_SetWriteNv21(
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
    u32Offset = ADDR_REG_ITIDPSR;
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
        ST_ISP_TIL_RDMA stIspTilRdma;

        stIspTilRdma.stTilRdma.stRdmaStartCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaStartCmd.RegAddr = ADDR_REG_ITIIWSR;
        stIspTilRdma.stTilRdma.stRdmaStartCmd.u32Param = ST_TIL_1_WS_REG_LEN;
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrSdCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrSdCmd.RegAddr = ADDR_REG_ITIDWLSR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrSdCmd.u32Param =
            ST_TIL_1_WR_SD_REG_LEN;
#if defined(V4L2_DRIVER)
#else
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrAddrCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrAddrCmd.RegAddr =
            ADDR_REG_ITIDWYSAR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrAddrCmd.u32Param =
            ST_TIL_1_WR_ADDR_REG_LEN;
#endif //#if defined(V4L2_DRIVER)
#if defined(V4L2_DRIVER)
#else
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdAddrCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdAddrCmd.RegAddr =
            ADDR_REG_ITIDRYSAR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdAddrCmd.u32Param =
            ST_TIL_1_RD_ADDR_REG_LEN;
#endif //#if defined(V4L2_DRIVER)
        stIspTilRdma.stTilRdma.stRdmaTil_1_PDFCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_PDFCmd.RegAddr = ADDR_REG_ITIPDFR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_PDFCmd.u32Param =
            ST_TIL_1_PDF_REG_LEN;
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdSdCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdSdCmd.RegAddr = ADDR_REG_ITIDRLSR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdSdCmd.u32Param =
            ST_TIL_1_RD_SD_REG_LEN;
#if defined(V4L2_DRIVER)
#else
        stIspTilRdma.stTilRdma.stRdmaTil_1_BSCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_BSCmd.RegAddr = ADDR_REG_ITIBSR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_BSCmd.u32Param = ST_TIL_1_BS_REG_LEN;
#endif //#if defined(V4L2_DRIVER)
        stIspTilRdma.stTilRdma.stRdmaTil_1_AxiCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_AxiCmd.RegAddr = ADDR_REG_ITIAIR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_AxiCmd.u32Param =
            ST_TIL_1_AXI_REG_LEN;
        stIspTilRdma.stTilRdma.stRdmaTil_1_DPSCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_DPSCmd.RegAddr = ADDR_REG_ITIDPSR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_DPSCmd.u32Param =
            ST_TIL_1_DPS_REG_LEN;
        stIspTilRdma.stTilRdma.stRdmaEndCmd.Tag = DESP_END;

        stIspTilRdma.stTilRdma.stTil_1_DPSReg.Til_1_PathSelect.value = 0;
        stIspTilRdma.stTilRdma.stTil_1_DPSReg.Til_1_PathSelect.field.wrnv21format =
            (rand() & 0x1);
        stIspTilRdma.u8IspIdx = 0;
        ret = STFDRV_ISP_TIL_1_SetWriteNv21(
            pstCiConnection,
            &stIspTilRdma
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_TIL_1_SetWriteNv21()\n",
            stIspTilRdma.u8IspIdx);
        LOG_INFO("DataPath = 0x%08X\n",
            stIspTilRdma.stTilRdma.stTil_1_DPSReg.Til_1_PathSelect.value
            );
        LOG_INFO("DataPath(Write) - UvFormat = 0x%01X(%s)\n",
            stIspTilRdma.stTilRdma.stTil_1_DPSReg.Til_1_PathSelect.field.wrnv21format,
            ((stIspTilRdma.stTilRdma.stTil_1_DPSReg.Til_1_PathSelect.field.wrnv21format)
                ? ("NV21") : ("NV12"))
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_ITIDPSR;
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

STF_RESULT Test_TIL_1_SetWriteDataFormat(
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
    u32Offset = ADDR_REG_ITIPDFR;
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
        ST_ISP_TIL_RDMA stIspTilRdma;

        stIspTilRdma.stTilRdma.stRdmaStartCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaStartCmd.RegAddr = ADDR_REG_ITIIWSR;
        stIspTilRdma.stTilRdma.stRdmaStartCmd.u32Param = ST_TIL_1_WS_REG_LEN;
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrSdCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrSdCmd.RegAddr = ADDR_REG_ITIDWLSR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrSdCmd.u32Param =
            ST_TIL_1_WR_SD_REG_LEN;
#if defined(V4L2_DRIVER)
#else
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrAddrCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrAddrCmd.RegAddr =
            ADDR_REG_ITIDWYSAR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrAddrCmd.u32Param =
            ST_TIL_1_WR_ADDR_REG_LEN;
#endif //#if defined(V4L2_DRIVER)
#if defined(V4L2_DRIVER)
#else
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdAddrCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdAddrCmd.RegAddr =
            ADDR_REG_ITIDRYSAR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdAddrCmd.u32Param =
            ST_TIL_1_RD_ADDR_REG_LEN;
#endif //#if defined(V4L2_DRIVER)
        stIspTilRdma.stTilRdma.stRdmaTil_1_PDFCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_PDFCmd.RegAddr = ADDR_REG_ITIPDFR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_PDFCmd.u32Param =
            ST_TIL_1_PDF_REG_LEN;
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdSdCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdSdCmd.RegAddr = ADDR_REG_ITIDRLSR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdSdCmd.u32Param =
            ST_TIL_1_RD_SD_REG_LEN;
#if defined(V4L2_DRIVER)
#else
        stIspTilRdma.stTilRdma.stRdmaTil_1_BSCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_BSCmd.RegAddr = ADDR_REG_ITIBSR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_BSCmd.u32Param = ST_TIL_1_BS_REG_LEN;
#endif //#if defined(V4L2_DRIVER)
        stIspTilRdma.stTilRdma.stRdmaTil_1_AxiCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_AxiCmd.RegAddr = ADDR_REG_ITIAIR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_AxiCmd.u32Param =
            ST_TIL_1_AXI_REG_LEN;
        stIspTilRdma.stTilRdma.stRdmaTil_1_DPSCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_DPSCmd.RegAddr = ADDR_REG_ITIDPSR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_DPSCmd.u32Param =
            ST_TIL_1_DPS_REG_LEN;
        stIspTilRdma.stTilRdma.stRdmaEndCmd.Tag = DESP_END;

        stIspTilRdma.stTilRdma.stTil_1_PDFReg.Til_1_DataFormat.value = 0;
        stIspTilRdma.stTilRdma.stTil_1_PDFReg.Til_1_DataFormat.field.wrformat =
            (rand() & 0x7);
        stIspTilRdma.u8IspIdx = 0;
        ret = STFDRV_ISP_TIL_1_SetWriteDataFormat(
            pstCiConnection,
            &stIspTilRdma
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_TIL_1_SetWriteDataFormat()\n",
            stIspTilRdma.u8IspIdx);
        LOG_INFO("DataFormat = 0x%08X\n",
            stIspTilRdma.stTilRdma.stTil_1_PDFReg.Til_1_DataFormat.value
            );
        LOG_INFO("DataFormat(Write) - Format = 0x%01X(%s)\n",
            stIspTilRdma.stTilRdma.stTil_1_PDFReg.Til_1_DataFormat.field.wrformat,
            g_szFormat[stIspTilRdma.stTilRdma.stTil_1_PDFReg.Til_1_DataFormat.field.wrformat]
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_ITIPDFR;
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

STF_RESULT Test_TIL_1_SetWriteFlexibleBits(
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
    u32Offset = ADDR_REG_ITIPDFR;
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
        ST_ISP_TIL_RDMA stIspTilRdma;

        stIspTilRdma.stTilRdma.stRdmaStartCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaStartCmd.RegAddr = ADDR_REG_ITIIWSR;
        stIspTilRdma.stTilRdma.stRdmaStartCmd.u32Param = ST_TIL_1_WS_REG_LEN;
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrSdCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrSdCmd.RegAddr = ADDR_REG_ITIDWLSR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrSdCmd.u32Param =
            ST_TIL_1_WR_SD_REG_LEN;
#if defined(V4L2_DRIVER)
#else
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrAddrCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrAddrCmd.RegAddr =
            ADDR_REG_ITIDWYSAR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrAddrCmd.u32Param =
            ST_TIL_1_WR_ADDR_REG_LEN;
#endif //#if defined(V4L2_DRIVER)
#if defined(V4L2_DRIVER)
#else
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdAddrCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdAddrCmd.RegAddr =
            ADDR_REG_ITIDRYSAR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdAddrCmd.u32Param =
            ST_TIL_1_RD_ADDR_REG_LEN;
#endif //#if defined(V4L2_DRIVER)
        stIspTilRdma.stTilRdma.stRdmaTil_1_PDFCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_PDFCmd.RegAddr = ADDR_REG_ITIPDFR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_PDFCmd.u32Param =
            ST_TIL_1_PDF_REG_LEN;
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdSdCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdSdCmd.RegAddr = ADDR_REG_ITIDRLSR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdSdCmd.u32Param =
            ST_TIL_1_RD_SD_REG_LEN;
#if defined(V4L2_DRIVER)
#else
        stIspTilRdma.stTilRdma.stRdmaTil_1_BSCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_BSCmd.RegAddr = ADDR_REG_ITIBSR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_BSCmd.u32Param = ST_TIL_1_BS_REG_LEN;
#endif //#if defined(V4L2_DRIVER)
        stIspTilRdma.stTilRdma.stRdmaTil_1_AxiCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_AxiCmd.RegAddr = ADDR_REG_ITIAIR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_AxiCmd.u32Param =
            ST_TIL_1_AXI_REG_LEN;
        stIspTilRdma.stTilRdma.stRdmaTil_1_DPSCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_DPSCmd.RegAddr = ADDR_REG_ITIDPSR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_DPSCmd.u32Param =
            ST_TIL_1_DPS_REG_LEN;
        stIspTilRdma.stTilRdma.stRdmaEndCmd.Tag = DESP_END;

        stIspTilRdma.stTilRdma.stTil_1_PDFReg.Til_1_DataFormat.value = 0;
        stIspTilRdma.stTilRdma.stTil_1_PDFReg.Til_1_DataFormat.field.wrflexiblebit =
            (rand() & 0x1F);
        stIspTilRdma.u8IspIdx = 0;
        ret = STFDRV_ISP_TIL_1_SetWriteFlexibleBits(
            pstCiConnection,
            &stIspTilRdma
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_TIL_1_SetWriteFlexibleBits()\n",
            stIspTilRdma.u8IspIdx);
        LOG_INFO("DataFormat = 0x%08X\n",
            stIspTilRdma.stTilRdma.stTil_1_PDFReg.Til_1_DataFormat.value
            );
        LOG_INFO("DataFormat(Write) - FlexibleBit = 0x%02X\n",
            stIspTilRdma.stTilRdma.stTil_1_PDFReg.Til_1_DataFormat.field.wrflexiblebit
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_ITIPDFR;
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

STF_RESULT Test_TIL_1_SetWriteAxiId(
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
    u32Offset = ADDR_REG_ITIAIR;
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
        ST_ISP_TIL_RDMA stIspTilRdma;

        stIspTilRdma.stTilRdma.stRdmaStartCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaStartCmd.RegAddr = ADDR_REG_ITIIWSR;
        stIspTilRdma.stTilRdma.stRdmaStartCmd.u32Param = ST_TIL_1_WS_REG_LEN;
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrSdCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrSdCmd.RegAddr = ADDR_REG_ITIDWLSR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrSdCmd.u32Param =
            ST_TIL_1_WR_SD_REG_LEN;
#if defined(V4L2_DRIVER)
#else
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrAddrCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrAddrCmd.RegAddr =
            ADDR_REG_ITIDWYSAR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrAddrCmd.u32Param =
            ST_TIL_1_WR_ADDR_REG_LEN;
#endif //#if defined(V4L2_DRIVER)
#if defined(V4L2_DRIVER)
#else
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdAddrCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdAddrCmd.RegAddr =
            ADDR_REG_ITIDRYSAR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdAddrCmd.u32Param =
            ST_TIL_1_RD_ADDR_REG_LEN;
#endif //#if defined(V4L2_DRIVER)
        stIspTilRdma.stTilRdma.stRdmaTil_1_PDFCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_PDFCmd.RegAddr = ADDR_REG_ITIPDFR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_PDFCmd.u32Param =
            ST_TIL_1_PDF_REG_LEN;
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdSdCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdSdCmd.RegAddr = ADDR_REG_ITIDRLSR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdSdCmd.u32Param =
            ST_TIL_1_RD_SD_REG_LEN;
#if defined(V4L2_DRIVER)
#else
        stIspTilRdma.stTilRdma.stRdmaTil_1_BSCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_BSCmd.RegAddr = ADDR_REG_ITIBSR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_BSCmd.u32Param = ST_TIL_1_BS_REG_LEN;
#endif //#if defined(V4L2_DRIVER)
        stIspTilRdma.stTilRdma.stRdmaTil_1_AxiCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_AxiCmd.RegAddr = ADDR_REG_ITIAIR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_AxiCmd.u32Param =
            ST_TIL_1_AXI_REG_LEN;
        stIspTilRdma.stTilRdma.stRdmaTil_1_DPSCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_DPSCmd.RegAddr = ADDR_REG_ITIDPSR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_DPSCmd.u32Param =
            ST_TIL_1_DPS_REG_LEN;
        stIspTilRdma.stTilRdma.stRdmaEndCmd.Tag = DESP_END;

        stIspTilRdma.stTilRdma.stTil_1_AxiReg.Til_1_AxiId.value = 0;
        stIspTilRdma.stTilRdma.stTil_1_AxiReg.Til_1_AxiId.field.ywid =
            (rand() & 0xFF);
        stIspTilRdma.stTilRdma.stTil_1_AxiReg.Til_1_AxiId.field.uvwid =
            (rand() & 0xFF);
        stIspTilRdma.u8IspIdx = 0;
        ret = STFDRV_ISP_TIL_1_SetWriteAxiId(
            pstCiConnection,
            &stIspTilRdma
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_TIL_1_SetWriteAxiId()\n",
            stIspTilRdma.u8IspIdx);
        LOG_INFO("AxiId = 0x%08X\n",
            stIspTilRdma.stTilRdma.stTil_1_AxiReg.Til_1_AxiId.value
            );
        LOG_INFO("AxiId(Write) - Y_AxiId = 0x%02X, Uv_AxiId = 0x%02X\n",
            stIspTilRdma.stTilRdma.stTil_1_AxiReg.Til_1_AxiId.field.ywid,
            stIspTilRdma.stTilRdma.stTil_1_AxiReg.Til_1_AxiId.field.uvwid
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_ITIAIR;
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

STF_RESULT Test_TIL_1_SetWriteEnable(
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
    u32Offset = ADDR_REG_ITIDPSR;
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
        //STF_U8 *pu8Temp = NULL;
        //STF_U32 u32Idx;
        //STF_U32 u32Cnt;
        ST_ISP_TIL_RDMA stIspTilRdma;

        stIspTilRdma.stTilRdma.stRdmaStartCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaStartCmd.RegAddr = ADDR_REG_ITIIWSR;
        stIspTilRdma.stTilRdma.stRdmaStartCmd.u32Param = ST_TIL_1_WS_REG_LEN;
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrSdCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrSdCmd.RegAddr = ADDR_REG_ITIDWLSR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrSdCmd.u32Param =
            ST_TIL_1_WR_SD_REG_LEN;
#if defined(V4L2_DRIVER)
#else
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrAddrCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrAddrCmd.RegAddr =
            ADDR_REG_ITIDWYSAR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrAddrCmd.u32Param =
            ST_TIL_1_WR_ADDR_REG_LEN;
#endif //#if defined(V4L2_DRIVER)
#if defined(V4L2_DRIVER)
#else
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdAddrCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdAddrCmd.RegAddr =
            ADDR_REG_ITIDRYSAR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdAddrCmd.u32Param =
            ST_TIL_1_RD_ADDR_REG_LEN;
#endif //#if defined(V4L2_DRIVER)
        stIspTilRdma.stTilRdma.stRdmaTil_1_PDFCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_PDFCmd.RegAddr = ADDR_REG_ITIPDFR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_PDFCmd.u32Param =
            ST_TIL_1_PDF_REG_LEN;
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdSdCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdSdCmd.RegAddr = ADDR_REG_ITIDRLSR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdSdCmd.u32Param =
            ST_TIL_1_RD_SD_REG_LEN;
#if defined(V4L2_DRIVER)
#else
        stIspTilRdma.stTilRdma.stRdmaTil_1_BSCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_BSCmd.RegAddr = ADDR_REG_ITIBSR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_BSCmd.u32Param = ST_TIL_1_BS_REG_LEN;
#endif //#if defined(V4L2_DRIVER)
        stIspTilRdma.stTilRdma.stRdmaTil_1_AxiCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_AxiCmd.RegAddr = ADDR_REG_ITIAIR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_AxiCmd.u32Param =
            ST_TIL_1_AXI_REG_LEN;
        stIspTilRdma.stTilRdma.stRdmaTil_1_DPSCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_DPSCmd.RegAddr = ADDR_REG_ITIDPSR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_DPSCmd.u32Param =
            ST_TIL_1_DPS_REG_LEN;
        stIspTilRdma.stTilRdma.stRdmaEndCmd.Tag = DESP_END;

        stIspTilRdma.stTilRdma.stTil_1_DPSReg.Til_1_PathSelect.value = 0;
        stIspTilRdma.stTilRdma.stTil_1_DPSReg.Til_1_PathSelect.field.writedram =
            (rand() & 0x1);
        stIspTilRdma.u8IspIdx = 0;
        ret = STFDRV_ISP_TIL_1_SetWriteEnable(
            pstCiConnection,
            &stIspTilRdma
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_TIL_1_SetWriteEnable()\n",
            stIspTilRdma.u8IspIdx);
        LOG_INFO("DataPath = 0x%08X\n",
            stIspTilRdma.stTilRdma.stTil_1_DPSReg.Til_1_PathSelect.value
            );
        LOG_INFO("DataPath(Write) - Enable = 0x%01X(%s)\n",
            stIspTilRdma.stTilRdma.stTil_1_DPSReg.Til_1_PathSelect.field.writedram,
            ((stIspTilRdma.stTilRdma.stTil_1_DPSReg.Til_1_PathSelect.field.writedram)
                ? ("Enable") : ("Disable"))
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_ITIDPSR;
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

STF_RESULT Test_TIL_1_SetIspRaw10InDW(
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
    u32Offset = ADDR_REG_ITIPDFR;
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
        //STF_U8 *pu8Temp = NULL;
        //STF_U32 u32Idx;
        //STF_U32 u32Cnt;
        ST_ISP_TIL_RDMA stIspTilRdma;

        stIspTilRdma.stTilRdma.stRdmaStartCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaStartCmd.RegAddr = ADDR_REG_ITIIWSR;
        stIspTilRdma.stTilRdma.stRdmaStartCmd.u32Param = ST_TIL_1_WS_REG_LEN;
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrSdCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrSdCmd.RegAddr = ADDR_REG_ITIDWLSR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrSdCmd.u32Param =
            ST_TIL_1_WR_SD_REG_LEN;
#if defined(V4L2_DRIVER)
#else
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrAddrCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrAddrCmd.RegAddr =
            ADDR_REG_ITIDWYSAR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrAddrCmd.u32Param =
            ST_TIL_1_WR_ADDR_REG_LEN;
#endif //#if defined(V4L2_DRIVER)
#if defined(V4L2_DRIVER)
#else
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdAddrCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdAddrCmd.RegAddr =
            ADDR_REG_ITIDRYSAR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdAddrCmd.u32Param =
            ST_TIL_1_RD_ADDR_REG_LEN;
#endif //#if defined(V4L2_DRIVER)
        stIspTilRdma.stTilRdma.stRdmaTil_1_PDFCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_PDFCmd.RegAddr = ADDR_REG_ITIPDFR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_PDFCmd.u32Param =
            ST_TIL_1_PDF_REG_LEN;
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdSdCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdSdCmd.RegAddr = ADDR_REG_ITIDRLSR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdSdCmd.u32Param =
            ST_TIL_1_RD_SD_REG_LEN;
#if defined(V4L2_DRIVER)
#else
        stIspTilRdma.stTilRdma.stRdmaTil_1_BSCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_BSCmd.RegAddr = ADDR_REG_ITIBSR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_BSCmd.u32Param = ST_TIL_1_BS_REG_LEN;
#endif //#if defined(V4L2_DRIVER)
        stIspTilRdma.stTilRdma.stRdmaTil_1_AxiCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_AxiCmd.RegAddr = ADDR_REG_ITIAIR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_AxiCmd.u32Param =
            ST_TIL_1_AXI_REG_LEN;
        stIspTilRdma.stTilRdma.stRdmaTil_1_DPSCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_DPSCmd.RegAddr = ADDR_REG_ITIDPSR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_DPSCmd.u32Param =
            ST_TIL_1_DPS_REG_LEN;
        stIspTilRdma.stTilRdma.stRdmaEndCmd.Tag = DESP_END;

        stIspTilRdma.stTilRdma.stTil_1_PDFReg.Til_1_DataFormat.value = 0;
        stIspTilRdma.stTilRdma.stTil_1_PDFReg.Til_1_DataFormat.field.imgispraw10 =
            (rand() & 0x1);
        stIspTilRdma.u8IspIdx = 0;
        ret = STFDRV_ISP_TIL_1_SetIspRaw10InDW(
            pstCiConnection,
            &stIspTilRdma
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_TIL_1_SetIspRaw10InDW()\n",
            stIspTilRdma.u8IspIdx);
        LOG_INFO("DataFormat = 0x%08X\n",
            stIspTilRdma.stTilRdma.stTil_1_PDFReg.Til_1_DataFormat.value
            );
        LOG_INFO("DataFormat - Raw_10_Mode = 0x%01X(%s)\n",
            stIspTilRdma.stTilRdma.stTil_1_PDFReg.Til_1_DataFormat.field.imgispraw10,
            g_szRaw10Mode[stIspTilRdma.stTilRdma.stTil_1_PDFReg.Til_1_DataFormat.field.imgispraw10]
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_ITIPDFR;
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

STF_RESULT Test_TIL_1_SetLoopbackMode(
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
    u32Offset = ADDR_REG_ITIDPSR;
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
        //STF_U8 *pu8Temp = NULL;
        //STF_U32 u32Idx;
        //STF_U32 u32Cnt;
        ST_ISP_TIL_RDMA stIspTilRdma;

        stIspTilRdma.stTilRdma.stRdmaStartCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaStartCmd.RegAddr = ADDR_REG_ITIIWSR;
        stIspTilRdma.stTilRdma.stRdmaStartCmd.u32Param = ST_TIL_1_WS_REG_LEN;
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrSdCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrSdCmd.RegAddr = ADDR_REG_ITIDWLSR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrSdCmd.u32Param =
            ST_TIL_1_WR_SD_REG_LEN;
#if defined(V4L2_DRIVER)
#else
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrAddrCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrAddrCmd.RegAddr =
            ADDR_REG_ITIDWYSAR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrAddrCmd.u32Param =
            ST_TIL_1_WR_ADDR_REG_LEN;
#endif //#if defined(V4L2_DRIVER)
#if defined(V4L2_DRIVER)
#else
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdAddrCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdAddrCmd.RegAddr =
            ADDR_REG_ITIDRYSAR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdAddrCmd.u32Param =
            ST_TIL_1_RD_ADDR_REG_LEN;
#endif //#if defined(V4L2_DRIVER)
        stIspTilRdma.stTilRdma.stRdmaTil_1_PDFCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_PDFCmd.RegAddr = ADDR_REG_ITIPDFR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_PDFCmd.u32Param =
            ST_TIL_1_PDF_REG_LEN;
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdSdCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdSdCmd.RegAddr = ADDR_REG_ITIDRLSR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdSdCmd.u32Param =
            ST_TIL_1_RD_SD_REG_LEN;
#if defined(V4L2_DRIVER)
#else
        stIspTilRdma.stTilRdma.stRdmaTil_1_BSCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_BSCmd.RegAddr = ADDR_REG_ITIBSR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_BSCmd.u32Param = ST_TIL_1_BS_REG_LEN;
#endif //#if defined(V4L2_DRIVER)
        stIspTilRdma.stTilRdma.stRdmaTil_1_AxiCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_AxiCmd.RegAddr = ADDR_REG_ITIAIR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_AxiCmd.u32Param =
            ST_TIL_1_AXI_REG_LEN;
        stIspTilRdma.stTilRdma.stRdmaTil_1_DPSCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_DPSCmd.RegAddr = ADDR_REG_ITIDPSR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_DPSCmd.u32Param =
            ST_TIL_1_DPS_REG_LEN;
        stIspTilRdma.stTilRdma.stRdmaEndCmd.Tag = DESP_END;

        stIspTilRdma.stTilRdma.stTil_1_DPSReg.Til_1_PathSelect.value = 0;
        stIspTilRdma.stTilRdma.stTil_1_DPSReg.Til_1_PathSelect.field.loopback =
            (rand() & 0x1);
        stIspTilRdma.u8IspIdx = 0;
        ret = STFDRV_ISP_TIL_1_SetLoopbackMode(
            pstCiConnection,
            &stIspTilRdma
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_TIL_1_SetLoopbackMode()\n",
            stIspTilRdma.u8IspIdx);
        LOG_INFO("DataPath = 0x%08X\n",
            stIspTilRdma.stTilRdma.stTil_1_DPSReg.Til_1_PathSelect.value
            );
        LOG_INFO("DataPath - Loopback = 0x%01X(%s)\n",
            stIspTilRdma.stTilRdma.stTil_1_DPSReg.Til_1_PathSelect.field.loopback,
            ((stIspTilRdma.stTilRdma.stTil_1_DPSReg.Til_1_PathSelect.field.loopback)
                ? ("Enable") : ("Disable"))
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_ITIDPSR;
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

STF_RESULT Test_TIL_1_SetDmaMode(
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
    u32Offset = ADDR_REG_ITIDPSR;
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
        //STF_U8 *pu8Temp = NULL;
        //STF_U32 u32Idx;
        //STF_U32 u32Cnt;
        ST_ISP_TIL_RDMA stIspTilRdma;

        stIspTilRdma.stTilRdma.stRdmaStartCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaStartCmd.RegAddr = ADDR_REG_ITIIWSR;
        stIspTilRdma.stTilRdma.stRdmaStartCmd.u32Param = ST_TIL_1_WS_REG_LEN;
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrSdCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrSdCmd.RegAddr = ADDR_REG_ITIDWLSR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrSdCmd.u32Param =
            ST_TIL_1_WR_SD_REG_LEN;
#if defined(V4L2_DRIVER)
#else
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrAddrCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrAddrCmd.RegAddr =
            ADDR_REG_ITIDWYSAR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_WrAddrCmd.u32Param =
            ST_TIL_1_WR_ADDR_REG_LEN;
#endif //#if defined(V4L2_DRIVER)
#if defined(V4L2_DRIVER)
#else
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdAddrCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdAddrCmd.RegAddr =
            ADDR_REG_ITIDRYSAR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdAddrCmd.u32Param =
            ST_TIL_1_RD_ADDR_REG_LEN;
#endif //#if defined(V4L2_DRIVER)
        stIspTilRdma.stTilRdma.stRdmaTil_1_PDFCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_PDFCmd.RegAddr = ADDR_REG_ITIPDFR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_PDFCmd.u32Param =
            ST_TIL_1_PDF_REG_LEN;
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdSdCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdSdCmd.RegAddr = ADDR_REG_ITIDRLSR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_RdSdCmd.u32Param =
            ST_TIL_1_RD_SD_REG_LEN;
#if defined(V4L2_DRIVER)
#else
        stIspTilRdma.stTilRdma.stRdmaTil_1_BSCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_BSCmd.RegAddr = ADDR_REG_ITIBSR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_BSCmd.u32Param = ST_TIL_1_BS_REG_LEN;
#endif //#if defined(V4L2_DRIVER)
        stIspTilRdma.stTilRdma.stRdmaTil_1_AxiCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_AxiCmd.RegAddr = ADDR_REG_ITIAIR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_AxiCmd.u32Param =
            ST_TIL_1_AXI_REG_LEN;
        stIspTilRdma.stTilRdma.stRdmaTil_1_DPSCmd.Tag = MREG_WR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_DPSCmd.RegAddr = ADDR_REG_ITIDPSR;
        stIspTilRdma.stTilRdma.stRdmaTil_1_DPSCmd.u32Param =
            ST_TIL_1_DPS_REG_LEN;
        stIspTilRdma.stTilRdma.stRdmaEndCmd.Tag = DESP_END;

        stIspTilRdma.stTilRdma.stTil_1_DPSReg.Til_1_PathSelect.value = 0;
        stIspTilRdma.stTilRdma.stTil_1_DPSReg.Til_1_PathSelect.field.movedramdata =
            (rand() & 0x1);
        stIspTilRdma.u8IspIdx = 0;
        ret = STFDRV_ISP_TIL_1_SetDmaMode(
            pstCiConnection,
            &stIspTilRdma
            );
        if (ret) {
            return ret;
        }
        LOG_INFO("ISP_%d : STFDRV_ISP_TIL_1_SetDmaMode()\n",
            stIspTilRdma.u8IspIdx);
        LOG_INFO("DataPath = 0x%08X\n",
            stIspTilRdma.stTilRdma.stTil_1_DPSReg.Til_1_PathSelect.value
            );
        LOG_INFO("DataPath - DMA = 0x%01X(%s)\n",
            stIspTilRdma.stTilRdma.stTil_1_DPSReg.Til_1_PathSelect.field.movedramdata,
            ((stIspTilRdma.stTilRdma.stTil_1_DPSReg.Til_1_PathSelect.field.movedramdata)
                ? ("Enable") : ("Disable"))
            );
        LOG_INFO("---------------------------------\n");
    }
    //-------------------------------------------------------------------------
    u32Offset = ADDR_REG_ITIDPSR;
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

#endif //#if defined(EXTEND_IOCTL_COMMAND_TIL)
//-----------------------------------------------------------------------------
STF_RESULT ISP_Mod_Til_Test(
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
        ret = Test_TIL_SetReg(
            );
        LOG_INFO("Test TIL_SetReg command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the TIL module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_TIL_GetAllBusyStatus(
            );
        LOG_INFO("Test TIL_GetAllBusyStatus command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the TIL module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_TIL_1_SetWindowReg(
            );
        LOG_INFO("Test TIL_1_SetWindowReg command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the TIL module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_TIL_1_SetConfigReg(
            );
        LOG_INFO("Test TIL_1_SetConfigReg command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the TIL module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_TIL_1_SetReadReg(
            );
        LOG_INFO("Test TIL_1_SetReadReg command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the TIL module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_TIL_1_SetWriteReg(
            );
        LOG_INFO("Test TIL_1_SetWriteReg command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the TIL module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_TIL_1_GetBusyStatus(
            );
        LOG_INFO("Test TIL_1_GetBusyStatus command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the TIL module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------
#if defined(EXTEND_IOCTL_COMMAND_TIL)

    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_TIL_1_IsReadBusy(
            );
        LOG_INFO("Test TIL_1_IsReadBusy command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the TIL module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_TIL_1_IsWriteBusy(
            );
        LOG_INFO("Test TIL_1_IsWriteBusy command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the TIL module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------
#endif //#if defined(EXTEND_IOCTL_COMMAND_TIL)
    //=========================================================================
#if defined(EXTEND_IOCTL_COMMAND_TIL)

    //=========================================================================
    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_TIL_1_SetWindowSize(
            );
        LOG_INFO("Test TIL_1_SetWindowSize command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the TIL module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_TIL_1_SetReadBufAddr(
            );
        LOG_INFO("Test TIL_1_SetReadBufAddr command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the TIL module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_TIL_1_SetReadBufStride(
            );
        LOG_INFO("Test TIL_1_SetReadBufStride command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the TIL module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_TIL_1_SetReadDataPath(
            );
        LOG_INFO("Test TIL_1_SetReadDataPath command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the TIL module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_TIL_1_SetReadNv21(
            );
        LOG_INFO("Test TIL_1_SetReadNv21 command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the TIL module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_TIL_1_SetReadDataFormat(
            );
        LOG_INFO("Test TIL_1_SetReadDataFormat command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the TIL module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_TIL_1_SetReadFlexibleBits(
            );
        LOG_INFO("Test TIL_1_SetReadFlexibleBits command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the TIL module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_TIL_1_SetReadIndependentRvi(
            );
        LOG_INFO("Test TIL_1_SetReadIndependentRvi command is %s, "\
            "ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the TIL module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_TIL_1_SetReadLineBufOff(
            );
        LOG_INFO("Test TIL_1_SetReadLineBufOff command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the TIL module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_TIL_1_SetReadAxiId(
            );
        LOG_INFO("Test TIL_1_SetReadAxiId command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the TIL module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_TIL_1_SetReadEnable(
            );
        LOG_INFO("Test TIL_1_SetReadEnable command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the TIL module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_TIL_1_SetWriteBufAddr(
            );
        LOG_INFO("Test TIL_1_SetWriteBufAddr command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the TIL module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_TIL_1_SetWriteBufStride(
            );
        LOG_INFO("Test TIL_1_SetWriteBufStride command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the TIL module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_TIL_1_SetWriteDataPath(
            );
        LOG_INFO("Test TIL_1_SetWriteDataPath command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the TIL module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_TIL_1_SetWriteNv21(
            );
        LOG_INFO("Test TIL_1_SetWriteNv21 command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the TIL module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_TIL_1_SetWriteDataFormat(
            );
        LOG_INFO("Test TIL_1_SetWriteDataFormat command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the TIL module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_TIL_1_SetWriteFlexibleBits(
            );
        LOG_INFO("Test TIL_1_SetWriteFlexibleBits command is %s, "\
            "ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the TIL module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_TIL_1_SetWriteAxiId(
            );
        LOG_INFO("Test TIL_1_SetWriteAxiId command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the TIL module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_TIL_1_SetWriteEnable(
            );
        LOG_INFO("Test TIL_1_SetWriteEnable command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the TIL module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_TIL_1_SetIspRaw10InDW(
            );
        LOG_INFO("Test TIL_1_SetIspRaw10InDW command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the TIL module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_TIL_1_SetLoopbackMode(
            );
        LOG_INFO("Test TIL_1_SetLoopbackMode command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the TIL module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    LOG_INFO("=================================\n");
    {
        ret = Test_TIL_1_SetDmaMode(
            );
        LOG_INFO("Test TIL_1_SetDmaMode command is %s, ret = %d(%s)\n",
            ((STF_SUCCESS == ret) ? ("successful") : ("failed")),
            ret,
            STF_STR_ERROR(ret)
            );
        LOG_INFO("---------------------------------\n");
        if (ret) {
            LOG_ERROR("Stop the TIL module command test.\n");
            return ret;
        }
    }
    //LOG_INFO("=================================\n");
    //-------------------------------------------------------------------------
    //=========================================================================
#endif //#if defined(EXTEND_IOCTL_COMMAND_TIL)

    return ret;
}

//-----------------------------------------------------------------------------
