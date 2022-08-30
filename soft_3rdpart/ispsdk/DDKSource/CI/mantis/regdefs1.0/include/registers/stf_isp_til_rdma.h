/**
  ******************************************************************************
  * @file  stf_isp_til_rdma.h
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
#ifndef __STF_ISP_TIL_RDMA_H__
#define __STF_ISP_TIL_RDMA_H__


#include "registers/stf_isp_rdma.h"


/* add tiling input/output image registers structure */
typedef enum _EN_TIL_MODE {
    EN_TIL_MODE_NULL = 0,
    EN_TIL_MODE_1_READ,
    EN_TIL_MODE_1_WRITE,
    EN_TIL_MODE_MAX,
} EN_TIL_MODE, *PEN_TIL_MODE;

typedef enum _EN_TIL_IMG_TYPE {
    EN_TIL_IMG_TYPE_BAYER = 0,
    EN_TIL_IMG_TYPE_RGB,
    EN_TIL_IMG_TYPE_YUV,
    EN_TIL_IMG_TYPE_MAX,
} EN_TIL_IMG_TYPE, *PEN_TIL_IMG_TYPE;

typedef enum _EN_TIL_FORMAT {
    EN_TIL_FMT_RAW10_6PIX_1QW = 0,  //0 : RAW10 bit (6 pixels per 1QW) QW= 64bit word, msb 4bit no use.
    EN_TIL_FMT_RAW12_5PIX_1QW,      //1 : RAW12 bit (5 pixels per 1QW), msb 4bit no use.
    EN_TIL_FMT_YUV420_SEMI,         //2 : YUV semi-planar 8bit: Y plane, UV plane 4:2:0 JPEG/JFIF progressive mode using the averaging value of the 4 UV points between horizontal and vertical.
    EN_TIL_FMT_RAW10_32PIX_5QW,     //3 : Optional fully packed RAW10 (320 bits = 32 pixels = 5QW), no redundant bit.
    EN_TIL_FMT_RAW12_16PIX_3QW,     //4 : Optional fully packed RAW12 (192 bits = 16 pixels = 3QW), no redundant bit.
    EN_TIL_FMT_YUV420_SEMI_HOR_AVG, //5 : YUV semi-planar 8bit: Y plane, UV plane 4:2:0 mode using the averaging value of the 2 UV points between the horizontal direction.
    EN_TIL_FMT_YUV420_SEMI_1ST_EVEN,//6 : YUV semi-planar 8bit: Y plane, UV plane 4:2:0 mode using the first position of the even horizontal with no averaging value.
    EN_TIL_FMT_RAW_FLEX,            //7 : RAW flexible bit whose bit number is less than 25 bits with no gap package. The bit number is specified in register 0x0B38 bit[24:20](Read) and bit[8:4](Write).
    EN_TIL_FMT_MAX
} EN_TIL_FORMAT, *PEN_TIL_FORMAT;

typedef enum _EN_TIL_RAW_10_MODE {
    EN_TIL_RAW_10_MODE_6PIX_1QW = 0,//0 : RAW10 bit (6 pixels per 1QW) QW= 64bit word, msb 4bit no use.
    EN_TIL_RAW_10_MODE_3PIX_1DW,    //1 : RAW10bit format is changed to IMG ISP RAW 10 bit format (IMG 3 pack in a DW).
    EN_TIL_RAW_10_MODE_MAX
} EN_TIL_RAW_10_MODE, *PEN_TIL_RAW_10_MODE;

typedef enum _EN_TIL_YUV_BITMODE {
    EN_TIL_YUVBIT_8BIT = 0,
    EN_TIL_YUVBIT_10BIT,
} EN_TIL_YUV_BITMODE, *PEN_TIL_YUV_BITMODE;

typedef enum _EN_RD_UV_LINE_BUF {
    EN_RD_UV_LINE_BUF_ENABLE = 0,   //0 : enable the UV line buffer function. (no UV reload for odd lines).
    EN_RD_UV_LINE_BUF_DISABLE,      //1 : Disable the UV line buffer function when UV AXI READ controller reads the UV data form DRAM.
    EN_RD_UV_LINE_BUF_MAX
} EN_RD_UV_LINE_BUF, *PEN_RD_UV_LINE_BUF;

typedef enum _EN_TIL_FLEX_BIT {
    EN_TIL_FLEX_BIT_RAW_8BIT = 0,
    EN_TIL_FLEX_BIT_RAW_10BIT,
    EN_TIL_FLEX_BIT_RAW_12BIT,
    EN_TIL_FLEX_BIT_RAW_14BIT,
    EN_TIL_FLEX_BIT_RGB_8BIT,
    EN_TIL_FLEX_BIT_RGB_10BIT,
    EN_TIL_FLEX_BIT_YUV_8BIT,
    EN_TIL_FLEX_BIT_YUV_10BIT,
    EN_TIL_FLEX_BIT_MAX,
} EN_TIL_FLEX_BIT, *PEN_TIL_FLEX_BIT;

typedef enum _EN_TIL_PATH {
    EN_TIL_PATH_BF_DBC = 0,         //0 : RAW 10 bits, only for tiling read.
    EN_TIL_PATH_BF_CFA,             //1 : RGB 10 bits.
    EN_TIL_PATH_BF_SHRP,            //2 : YUV 10 bits.
    EN_TIL_PATH_BF_SCALE_OUT,       //3 : YUV 10 bits, only for tiling write.
    EN_TIL_PATH_MAX,
} EN_TIL_PATH, *PEN_TIL_PATH;

typedef enum _EN_TIL_READ {
    EN_TIL_READ_DIS = 0,            // 0 : Disable the read function of the ITI interface from DRAM buffer.
    EN_TIL_READ_EN,                 // 1 : Enable the read function of the ITI interface from DRAM buffer.
    EN_TIL_READ_MAX
} EN_TIL_READ, *PEN_TIL_READ;

typedef enum _EN_TIL_WRITE {
    EN_TIL_WRITE_DIS = 0,           // 0 : Disable the write function of the ITI interface into DRAM buffer.
    EN_TIL_WRITE_EN,                // 1 : Enable the write function of the ITI interface into DRAM buffer.
    EN_TIL_WRITE_MAX
} EN_TIL_WRITE, *PEN_TIL_WRITE;

typedef enum _EN_TIL_LOOPBACK {
    EN_TIL_LOOPBACK_DIS = 0,        // 0 : Disable the loop back self test where the write AXI controller obtains the input data from the AXI read controller.
    EN_TIL_LOOPBACK_EN,             // 1 : Enable the loop back self test where the write AXI controller obtains the input data from the AXI read controller.
    EN_TIL_LOOPBACK_MAX
} EN_TIL_LOOPBACK, *PEN_TIL_LOOPBACK;

typedef enum _EN_TIL_MOVE_DRAM_DATA {
    EN_TIL_MOVE_DRAM_DATA_DIS = 0,  // 0 : Disable the move data function from the reading address to the writing address. (move data directly from read to write, can be used for copying data).
    EN_TIL_MOVE_DRAM_DATA_EN,       // 1 : Enable the move data function from the reading address to the writing address. (move data directly from read to write, can be used for copying data).
    EN_TIL_MOVE_DRAM_DATA_MAX
} EN_TIL_MOVE_DRAM_DATA, *PEN_TIL_MOVE_DRAM_DATA;

typedef enum _EN_TIL_RVI {
    EN_TIL_RVI_DEPENDENT = 0,       // 0 : Disable the independent READ RVI signal function under the normal DRAM read mode.
    EN_TIL_RVI_INDEPENDENT,         // 1 : Enable the independent READ RVI signal function under the normal DRAM read mode.
    EN_TIL_RVI_MAX
} EN_TIL_RD_RVI, *PEN_TIL_RD_RVI;

typedef enum _EN_TIL_YUV420_FMT {
    EN_TIL_YUV420_FMT_12 = 0,       // 0: DRAM data format for UV (YCbCr 420 NV12).
    EN_TIL_YUV420_FMT_21,           // 1: DRAM data format for VU (YCrCb 420 NV21).
    EN_TIL_YUV420_FMT_MAX
} EN_TIL_YUV420_FMT, *PEN_TIL_YUV420_FMT;

typedef enum _EN_TIL_SYMBOL {
    EN_TIL_SYMBOL_R = 0,
    EN_TIL_SYMBOL_GR,
    EN_TIL_SYMBOL_GB,
    EN_TIL_SYMBOL_B,
} EN_TIL_SYMBOL, *PEN_TIL_SYMBOL;


typedef struct _ST_TIL_ALL_BUSY_STATUS {
    STF_U8 u8IspIdx;
    STF_U32 u32BusyStatus;
} ST_TIL_ALL_BUSY_STATUS, *PST_TIL_ALL_BUSY_STATUS;

typedef struct _ST_TIL_BUSY_STATUS {
    STF_U8 u8IspIdx;
    STF_BOOL8 bBusyStatus;
} ST_TIL_BUSY_STATUS, *PST_TIL_BUSY_STATUS;

typedef struct _ST_TIL_1_CFG_REG {
    //-------------------------------------------------------------------------
    REG_ITIPDFR Til_1_DataFormat;   // 0x0B38, ITI package data format register.
    REG_ITIDPSR Til_1_PathSelect;   // 0x0B48, ISP tiling interface data path select register.
    //-------------------------------------------------------------------------
} ST_TIL_1_CFG_REG, *PST_TIL_1_CFG_REG;

#define ST_TIL_1_CFG_REG_SIZE       (sizeof(ST_TIL_1_CFG_REG))
#define ST_TIL_1_CFG_REG_LEN        (ST_TIL_1_CFG_REG_SIZE / sizeof(STF_U32))

typedef struct _ST_TIL_1_RD_REG {
    //-------------------------------------------------------------------------
    REG_ITIIWSR Til_1_WindowSize;   // 0x0B20, ITI image window size register.
#if defined(V4L2_DRIVER)
#else
    REG_ITIDRYSAR Til_1_ReadYAddr;  // 0x0B30, ITI DRAM read Y start address register, 8-byte alignment.
    REG_ITIDRUSAR Til_1_ReadUvAddr; // 0x0B34, ITI DRAM read UV start address register, 8-byte alignment.
#endif //#if defined(V4L2_DRIVER)
    REG_ITIDRLSR Til_1_ReadStride;  // 0x0B3C, ITI DRAM read line stride register.
    REG_ITIDPSR Til_1_PathSelect;   // 0x0B48, ISP tiling interface data path select register.
    REG_ITIPDFR Til_1_DataFormat;   // 0x0B38, ITI package data format register.
    REG_ITIAIR Til_1_AxiId;         // 0x0B44, ISP tiling interface AXI ID register.
    //-------------------------------------------------------------------------
} ST_TIL_1_RD_REG, *PST_TIL_1_RD_REG;

#define ST_TIL_1_RD_REG_SIZE        (sizeof(ST_TIL_1_RD_REG))
#define ST_TIL_1_RD_REG_LEN         (ST_TIL_1_RD_REG_SIZE / sizeof(STF_U32))

typedef struct _ST_TIL_1_WR_REG {
    //-------------------------------------------------------------------------
    REG_ITIIWSR Til_1_WindowSize;   // 0x0B20, ITI image window size register.
#if defined(V4L2_DRIVER)
#else
    REG_ITIDWYSAR Til_1_WriteYAddr; // 0x0B28, ITI DRAM write Y start address register, 8-byte alignment.
    REG_ITIDWUSAR Til_1_WriteUvAddr;// 0x0B2C, ITI DRAM write UV start address register, 8-byte alignment.
#endif //#if defined(V4L2_DRIVER)
    REG_ITIDWLSR Til_1_WriteStride; // 0x0B24, ITI DRAM write line stride register, 8-byte alignment.
    REG_ITIDPSR Til_1_PathSelect;   // 0x0B48, ISP tiling interface data path select register.
    REG_ITIPDFR Til_1_DataFormat;   // 0x0B38, ITI package data format register.
    REG_ITIAIR Til_1_AxiId;         // 0x0B44, ISP tiling interface AXI ID register.
    //-------------------------------------------------------------------------
} ST_TIL_1_WR_REG, *PST_TIL_1_WR_REG;

#define ST_TIL_1_WR_REG_SIZE        (sizeof(ST_TIL_1_WR_REG))
#define ST_TIL_1_WR_REG_LEN         (ST_TIL_1_WR_REG_SIZE / sizeof(STF_U32))

typedef struct _ST_TIL_1_WS_REG {
    //-------------------------------------------------------------------------
    REG_ITIIWSR Til_1_WindowSize;   // 0x0B20, ITI image window size register.
    //-------------------------------------------------------------------------
} ST_TIL_1_WS_REG, *PST_TIL_1_WS_REG;

#define ST_TIL_1_WS_REG_SIZE        (sizeof(ST_TIL_1_WS_REG))
#define ST_TIL_1_WS_REG_LEN         (ST_TIL_1_WS_REG_SIZE / sizeof(STF_U32))

typedef struct _ST_TIL_1_WR_SD_REG {
    //-------------------------------------------------------------------------
    REG_ITIDWLSR Til_1_WriteStride; // 0x0B24, ITI DRAM write line stride register, 8-byte alignment.
    //-------------------------------------------------------------------------
} ST_TIL_1_WR_SD_REG, *PST_TIL_1_WR_SD_REG;

#define ST_TIL_1_WR_SD_REG_SIZE     (sizeof(ST_TIL_1_WR_SD_REG))
#define ST_TIL_1_WR_SD_REG_LEN      (ST_TIL_1_WR_SD_REG_SIZE / sizeof(STF_U32))

typedef struct _ST_TIL_1_WR_ADDR_REG {
    //-------------------------------------------------------------------------
    REG_ITIDWYSAR Til_1_WriteYAddr; // 0x0B28, ITI DRAM write Y start address register, 8-byte alignment.
    REG_ITIDWUSAR Til_1_WriteUvAddr;// 0x0B2C, ITI DRAM write UV start address register, 8-byte alignment.
    //-------------------------------------------------------------------------
} ST_TIL_1_WR_ADDR_REG, *PST_TIL_1_WR_BUF_REG;

#define ST_TIL_1_WR_ADDR_REG_SIZE   (sizeof(ST_TIL_1_WR_ADDR_REG))
#define ST_TIL_1_WR_ADDR_REG_LEN    (ST_TIL_1_WR_ADDR_REG_SIZE / sizeof(STF_U32))

typedef struct _ST_TIL_1_RD_ADDR_REG {
    //-------------------------------------------------------------------------
    REG_ITIDRYSAR Til_1_ReadYAddr;  // 0x0B30, ITI DRAM read Y start address register, 8-byte alignment.
    REG_ITIDRUSAR Til_1_ReadUvAddr; // 0x0B34, ITI DRAM read UV start address register, 8-byte alignment.
    //-------------------------------------------------------------------------
} ST_TIL_1_RD_ADDR_REG, *PST_TIL_1_RD_ADDR_REG;

#define ST_TIL_1_RD_ADDR_REG_SIZE   (sizeof(ST_TIL_1_RD_ADDR_REG))
#define ST_TIL_1_RD_ADDR_REG_LEN    (ST_TIL_1_RD_ADDR_REG_SIZE / sizeof(STF_U32))

typedef struct _ST_TIL_1_PDF_REG {
    //-------------------------------------------------------------------------
    REG_ITIPDFR Til_1_DataFormat;   // 0x0B38, ITI package data format register.
    //-------------------------------------------------------------------------
} ST_TIL_1_PDF_REG, *PST_TIL_1_PDF_REG;

#define ST_TIL_1_PDF_REG_SIZE       (sizeof(ST_TIL_1_PDF_REG))
#define ST_TIL_1_PDF_REG_LEN        (ST_TIL_1_PDF_REG_SIZE / sizeof(STF_U32))

typedef struct _ST_TIL_1_RD_SD_REG {
    //-------------------------------------------------------------------------
    REG_ITIDRLSR Til_1_ReadStride;  // 0x0B3C, ITI DRAM read line stride register.
    //-------------------------------------------------------------------------
} ST_TIL_1_RD_SD_REG, *PST_TIL_1_RD_SD_REG;

#define ST_TIL_1_RD_SD_REG_SIZE     (sizeof(ST_TIL_1_RD_SD_REG))
#define ST_TIL_1_RD_SD_REG_LEN      (ST_TIL_1_RD_SD_REG_SIZE / sizeof(STF_U32))

typedef struct _ST_TIL_1_BS_REG {
    //-------------------------------------------------------------------------
    REG_ITIBSR Til_1_BusyStatus;    // 0x0B40, ISP tiling interface BUSY status register.
    //-------------------------------------------------------------------------
} ST_TIL_1_BS_REG, *PST_TIL_1_BS_REG;

#define ST_TIL_1_BS_REG_SIZE        (sizeof(ST_TIL_1_BS_REG))
#define ST_TIL_1_BS_REG_LEN         (ST_TIL_1_BS_REG_SIZE / sizeof(STF_U32))

typedef struct _ST_TIL_1_AXI_REG {
    //-------------------------------------------------------------------------
    REG_ITIAIR Til_1_AxiId;         // 0x0B44, ISP tiling interface AXI ID register.
    //-------------------------------------------------------------------------
} ST_TIL_1_AXI_REG, *PST_TIL_1_AXI_REG;

#define ST_TIL_1_AXI_REG_SIZE       (sizeof(ST_TIL_1_AXI_REG))
#define ST_TIL_1_AXI_REG_LEN        (ST_TIL_1_AXI_REG_SIZE / sizeof(STF_U32))

typedef struct _ST_TIL_1_DPS_REG {
    //-------------------------------------------------------------------------
    REG_ITIDPSR Til_1_PathSelect;   // 0x0B48, ISP tiling interface data path select register.
    //-------------------------------------------------------------------------
} ST_TIL_1_DPS_REG, *PST_TIL_1_DPS_REG;

#define ST_TIL_1_DPS_REG_SIZE       (sizeof(ST_TIL_1_DPS_REG))
#define ST_TIL_1_DPS_REG_LEN        (ST_TIL_1_DPS_REG_SIZE / sizeof(STF_U32))

typedef struct _ST_TIL_REG {
    //-------------------------------------------------------------------------
    REG_ITIIWSR Til_1_WindowSize;   // 0x0B20, ITI image window size register.

    REG_ITIDWLSR Til_1_WriteStride; // 0x0B24, ITI DRAM write line stride register, 8-byte alignment.
#if defined(V4L2_DRIVER)
#else
    REG_ITIDWYSAR Til_1_WriteYAddr; // 0x0B28, ITI DRAM write Y start address register, 8-byte alignment.
    REG_ITIDWUSAR Til_1_WriteUvAddr;// 0x0B2C, ITI DRAM write UV start address register, 8-byte alignment.
#endif //#if defined(V4L2_DRIVER)

#if defined(V4L2_DRIVER)
#else
    REG_ITIDRYSAR Til_1_ReadYAddr;  // 0x0B30, ITI DRAM read Y start address register, 8-byte alignment.
    REG_ITIDRUSAR Til_1_ReadUvAddr; // 0x0B34, ITI DRAM read UV start address register, 8-byte alignment.
#endif //#if defined(V4L2_DRIVER)

    REG_ITIPDFR Til_1_DataFormat;   // 0x0B38, ITI package data format register.

    REG_ITIDRLSR Til_1_ReadStride;  // 0x0B3C, ITI DRAM read line stride register.

#if defined(V4L2_DRIVER)
#else
    REG_ITIBSR Til_1_BusyStatus;    // 0x0B40, ISP tiling interface BUSY status register.
#endif //#if defined(V4L2_DRIVER)

    REG_ITIAIR Til_1_AxiId;         // 0x0B44, ISP tiling interface AXI ID register.

    REG_ITIDPSR Til_1_PathSelect;   // 0x0B48, ISP tiling interface data path select register.
    //-------------------------------------------------------------------------
} ST_TIL_REG, *PST_TIL_REG;

#define ST_TIL_REG_SIZE             (sizeof(ST_TIL_REG))
#define ST_TIL_REG_LEN              (ST_TIL_REG_SIZE / sizeof(STF_U32))

#pragma pack(push, 8)
typedef struct _ST_TIL_RDMA {
#if defined(SUPPORT_RDMA_FEATURE)
    ST_RDMA_CMD stRdmaStartCmd __attribute__ ((aligned (8)));
#endif //SUPPORT_RDMA_FEATURE
    ST_TIL_1_WS_REG stTil_1_WSReg __attribute__ ((aligned (8)));
#if defined(SUPPORT_RDMA_FEATURE)
    ST_RDMA_CMD stRdmaTil_1_WrSdCmd __attribute__ ((aligned (8)));
#endif //SUPPORT_RDMA_FEATURE
    ST_TIL_1_WR_SD_REG stTil_1_WrSdReg __attribute__ ((aligned (8)));
#if defined(V4L2_DRIVER)
#else
  #if defined(SUPPORT_RDMA_FEATURE)
    ST_RDMA_CMD stRdmaTil_1_WrAddrCmd __attribute__ ((aligned (8)));
  #endif //SUPPORT_RDMA_FEATURE
    ST_TIL_1_WR_ADDR_REG stTil_1_WrAddrReg __attribute__ ((aligned (8)));
#endif //#if defined(V4L2_DRIVER)
#if defined(V4L2_DRIVER)
#else
  #if defined(SUPPORT_RDMA_FEATURE)
    ST_RDMA_CMD stRdmaTil_1_RdAddrCmd __attribute__ ((aligned (8)));
  #endif //SUPPORT_RDMA_FEATURE
    ST_TIL_1_RD_ADDR_REG stTil_1_RdAddrReg __attribute__ ((aligned (8)));
#endif //#if defined(V4L2_DRIVER)
#if defined(SUPPORT_RDMA_FEATURE)
    ST_RDMA_CMD stRdmaTil_1_PDFCmd __attribute__ ((aligned (8)));
#endif //SUPPORT_RDMA_FEATURE
    ST_TIL_1_PDF_REG stTil_1_PDFReg __attribute__ ((aligned (8)));
#if defined(SUPPORT_RDMA_FEATURE)
    ST_RDMA_CMD stRdmaTil_1_RdSdCmd __attribute__ ((aligned (8)));
#endif //SUPPORT_RDMA_FEATURE
    ST_TIL_1_RD_SD_REG stTil_1_RdSdReg __attribute__ ((aligned (8)));
#if defined(V4L2_DRIVER)
#else
  #if defined(SUPPORT_RDMA_FEATURE)
    ST_RDMA_CMD stRdmaTil_1_BSCmd __attribute__ ((aligned (8)));
  #endif //SUPPORT_RDMA_FEATURE
    ST_TIL_1_BS_REG stTil_1_BSReg __attribute__ ((aligned (8)));
#endif //#if defined(V4L2_DRIVER)
#if defined(SUPPORT_RDMA_FEATURE)
    ST_RDMA_CMD stRdmaTil_1_AxiCmd __attribute__ ((aligned (8)));
#endif //SUPPORT_RDMA_FEATURE
    ST_TIL_1_AXI_REG stTil_1_AxiReg __attribute__ ((aligned (8)));
#if defined(SUPPORT_RDMA_FEATURE)
    ST_RDMA_CMD stRdmaTil_1_DPSCmd __attribute__ ((aligned (8)));
#endif //SUPPORT_RDMA_FEATURE
    ST_TIL_1_DPS_REG stTil_1_DPSReg __attribute__ ((aligned (8)));
#if defined(SUPPORT_RDMA_FEATURE)
    ST_RDMA_CMD stRdmaEndCmd __attribute__ ((aligned (8)));
#endif //SUPPORT_RDMA_FEATURE
} ST_TIL_RDMA, *PST_TIL_RDMA;

#define ST_TIL_RDMA_SIZE            (sizeof(ST_TIL_RDMA))
#define ST_TIL_RDMA_LEN             (ST_TIL_RDMA_SIZE / (sizeof(STF_U32) * 2))
#define ST_TIL_1_WS_RDMA_SIZE       (sizeof(ST_RDMA_CMD) + (((sizeof(ST_TIL_1_WS_REG) + 7) / 8) * 8))
#define ST_TIL_1_WS_RDMA_LEN        (ST_TIL_1_WS_RDMA_SIZE / (sizeof(STF_U32) * 2))
#define ST_TIL_1_WR_SD_RDMA_SIZE    (sizeof(ST_RDMA_CMD) + (((sizeof(ST_TIL_1_WR_SD_REG) + 7) / 8) * 8))
#define ST_TIL_1_WR_SD_RDMA_LEN     (ST_TIL_1_WR_SD_RDMA_SIZE / (sizeof(STF_U32) * 2))
#define ST_TIL_1_WR_ADDR_RDMA_SIZE  (sizeof(ST_RDMA_CMD) + (((sizeof(ST_TIL_1_WR_ADDR_REG) + 7) / 8) * 8))
#define ST_TIL_1_WR_ADDR_RDMA_LEN   (ST_TIL_1_WR_ADDR_RDMA_SIZE / (sizeof(STF_U32) * 2))
#define ST_TIL_1_RD_ADDR_RDMA_SIZE  (sizeof(ST_RDMA_CMD) + (((sizeof(ST_TIL_1_RD_ADDR_REG) + 7) / 8) * 8))
#define ST_TIL_1_RD_ADDR_RDMA_LEN   (ST_TIL_1_RD_ADDR_RDMA_SIZE / (sizeof(STF_U32) * 2))
#define ST_TIL_1_PDF_RDMA_SIZE      (sizeof(ST_RDMA_CMD) + (((sizeof(ST_TIL_1_PDF_REG) + 7) / 8) * 8))
#define ST_TIL_1_PDF_RDMA_LEN       (ST_TIL_1_PDF_RDMA_SIZE / (sizeof(STF_U32) * 2))
#define ST_TIL_1_RD_SD_RDMA_SIZE    (sizeof(ST_RDMA_CMD) + (((sizeof(ST_TIL_1_RD_SD_REG) + 7) / 8) * 8))
#define ST_TIL_1_RD_SD_RDMA_LEN     (ST_TIL_1_RD_SD_RDMA_SIZE / (sizeof(STF_U32) * 2))
#define ST_TIL_1_BS_RDMA_SIZE       (sizeof(ST_RDMA_CMD) + (((sizeof(ST_TIL_1_BS_REG) + 7) / 8) * 8))
#define ST_TIL_1_BS_RDMA_LEN        (ST_TIL_1_BS_RDMA_SIZE / (sizeof(STF_U32) * 2))
#define ST_TIL_1_AXI_RDMA_SIZE      (sizeof(ST_RDMA_CMD) + (((sizeof(ST_TIL_1_AXI_REG) + 7) / 8) * 8))
#define ST_TIL_1_AXI_RDMA_LEN       (ST_TIL_1_AXI_RDMA_SIZE / (sizeof(STF_U32) * 2))
#define ST_TIL_1_DPS_RDMA_SIZE      (sizeof(ST_RDMA_CMD) + (((sizeof(ST_TIL_1_DPS_REG) + 7) / 8) * 8))
#define ST_TIL_1_DPS_RDMA_LEN       (ST_TIL_1_DPS_RDMA_SIZE / (sizeof(STF_U32) * 2))

typedef struct _ST_ISP_TIL_RDMA {
    STF_U8 u8IspIdx;
    ST_TIL_RDMA stTilRdma __attribute__ ((aligned (8)));
} ST_ISP_TIL_RDMA, *PST_ISP_TIL_RDMA;
#pragma pack(pop)


///* tiling input/output image registers interface */
////-----------------------------------------------------------------------------
//extern
//STF_VOID STFHAL_ISP_TIL_SetReg(
//    STF_U8 u8IspIdx,
//    ST_TIL_RDMA *pstTilRdma
//    );
//extern
//STF_U32 STFHAL_ISP_TIL_GetAllBusyStatus(
//    STF_U8 u8IspIdx
//    );
//extern
//STF_VOID STFHAL_ISP_TIL_1_SetWindowReg(
//    STF_U8 u8IspIdx,
//    ST_TIL_RDMA *pstTilRdma
//    );
//extern
//STF_VOID STFHAL_ISP_TIL_1_SetConfigReg(
//    STF_U8 u8IspIdx,
//    ST_TIL_RDMA *pstTilRdma
//    );
//extern
//STF_VOID STFHAL_ISP_TIL_1_SetReadReg(
//    STF_U8 u8IspIdx,
//    ST_TIL_RDMA *pstTilRdma
//    );
//extern
//STF_VOID STFHAL_ISP_TIL_1_SetWriteReg(
//    STF_U8 u8IspIdx,
//    ST_TIL_RDMA *pstTilRdma
//    );
//extern
//STF_U32 STFHAL_ISP_TIL_1_GetBusyStatus(
//    STF_U8 u8IspIdx
//    );
//extern
//STF_BOOL8 STFHAL_ISP_TIL_1_IsReadBusy(
//    STF_U8 u8IspIdx
//    );
//extern
//STF_BOOL8 STFHAL_ISP_TIL_1_IsWriteBusy(
//    STF_U8 u8IspIdx
//    );
////-----------------------------------------------------------------------------
//extern
//STF_VOID STFHAL_ISP_TIL_1_SetWindowSize(
//    STF_U8 u8IspIdx,
//    ST_TIL_RDMA *pstTilRdma
//    );
//extern
//STF_VOID STFHAL_ISP_TIL_1_SetReadBufAddr(
//    STF_U8 u8IspIdx,
//    ST_TIL_RDMA *pstTilRdma
//    );
//extern
//STF_VOID STFHAL_ISP_TIL_1_SetReadBufStride(
//    STF_U8 u8IspIdx,
//    ST_TIL_RDMA *pstTilRdma
//    );
//extern
//STF_VOID STFHAL_ISP_TIL_1_SetReadDataPath(
//    STF_U8 u8IspIdx,
//    ST_TIL_RDMA *pstTilRdma
//    );
//extern
//STF_VOID STFHAL_ISP_TIL_1_SetReadNv21(
//    STF_U8 u8IspIdx,
//    ST_TIL_RDMA *pstTilRdma
//    );
//extern
//STF_VOID STFHAL_ISP_TIL_1_SetReadDataFormat(
//    STF_U8 u8IspIdx,
//    ST_TIL_RDMA *pstTilRdma
//    );
//extern
//STF_VOID STFHAL_ISP_TIL_1_SetReadFlexibleBits(
//    STF_U8 u8IspIdx,
//    ST_TIL_RDMA *pstTilRdma
//    );
//extern
//STF_VOID STFHAL_ISP_TIL_1_SetReadIndependentRvi(
//    STF_U8 u8IspIdx,
//    ST_TIL_RDMA *pstTilRdma
//    );
//extern
//STF_VOID STFHAL_ISP_TIL_1_SetReadLineBufOff(
//    STF_U8 u8IspIdx,
//    ST_TIL_RDMA *pstTilRdma
//    );
//extern
//STF_VOID STFHAL_ISP_TIL_1_SetReadAxiId(
//    STF_U8 u8IspIdx,
//    ST_TIL_RDMA *pstTilRdma
//    );
//extern
//STF_VOID STFHAL_ISP_TIL_1_SetReadEnable(
//    STF_U8 u8IspIdx,
//    ST_TIL_RDMA *pstTilRdma
//    );
//extern
//STF_VOID STFHAL_ISP_TIL_1_SetWriteBufAddr(
//    STF_U8 u8IspIdx,
//    ST_TIL_RDMA *pstTilRdma
//    );
//extern
//STF_VOID STFHAL_ISP_TIL_1_SetWriteBufStride(
//    STF_U8 u8IspIdx,
//    ST_TIL_RDMA *pstTilRdma
//    );
//extern
//STF_VOID STFHAL_ISP_TIL_1_SetWriteDataPath(
//    STF_U8 u8IspIdx,
//    ST_TIL_RDMA *pstTilRdma
//    );
//extern
//STF_VOID STFHAL_ISP_TIL_1_SetWriteNv21(
//    STF_U8 u8IspIdx,
//    ST_TIL_RDMA *pstTilRdma
//    );
//extern
//STF_VOID STFHAL_ISP_TIL_1_SetWriteDataFormat(
//    STF_U8 u8IspIdx,
//    ST_TIL_RDMA *pstTilRdma
//    );
//extern
//STF_VOID STFHAL_ISP_TIL_1_SetWriteFlexibleBits(
//    STF_U8 u8IspIdx,
//    ST_TIL_RDMA *pstTilRdma
//    );
//extern
//STF_VOID STFHAL_ISP_TIL_1_SetWriteAxiId(
//    STF_U8 u8IspIdx,
//    ST_TIL_RDMA *pstTilRdma
//    );
//extern
//STF_VOID STFHAL_ISP_TIL_1_SetWriteEnable(
//    STF_U8 u8IspIdx,
//    ST_TIL_RDMA *pstTilRdma
//    );
//extern
//STF_VOID STFHAL_ISP_TIL_1_SetIspRaw10InDW(
//    STF_U8 u8IspIdx,
//    ST_TIL_RDMA *pstTilRdma
//    );
//extern
//STF_VOID STFHAL_ISP_TIL_1_SetLoopbackMode(
//    STF_U8 u8IspIdx,
//    ST_TIL_RDMA *pstTilRdma
//    );
//extern
//STF_VOID STFHAL_ISP_TIL_1_SetDmaMode(
//    STF_U8 u8IspIdx,
//    ST_TIL_RDMA *pstTilRdma
//    );
////-----------------------------------------------------------------------------
//
//
#endif //__STF_ISP_TIL_RDMA_H__
