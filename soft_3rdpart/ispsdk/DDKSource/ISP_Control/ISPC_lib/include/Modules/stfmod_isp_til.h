/**
  ******************************************************************************
  * @file  stfmod_isp_til.h
  * @author  StarFive Isp Team
  * @version  V1.0
  * @date  05/11/2021
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


#ifndef __STFMOD_ISP_TIL_H__
#define __STFMOD_ISP_TIL_H__


#ifdef __cplusplus
extern "C" {
#endif


#include "stf_include.h"
#include "ci/ci_api_structs.h"
#include "stf_common/ci_alloc_info.h"
#include "registers/stf_isp_til_rdma.h"
#include "stflib_isp_pipeline.h"


/* ISP tiling module parameters structure */
//-----------------------------------------------------------------------------
// Define initial values
#define ISP_DEF_TIL_IMAGE_WIDTH                 (ISP_IMG_WIDTH)   /** Image width. */
#define ISP_DEF_TIL_IMAGE_HEIGHT                (ISP_IMG_HEIGHT)  /** Image Height. */
#define ISP_DEF_TIL_RAW_10_MODE                 (EN_TIL_RAW_10_MODE_6PIX_1QW) /** ISP RAW 10 bit format. */
#define ISP_DEF_TIL_LOOP_BACK                   (STF_FALSE)       /** Test pipeline control, but data from read tiling to write tiling. */
#define ISP_DEF_TIL_MOVE_DRAM_DATA              (STF_FALSE)       /** Enable the move data function from the reading address to the writing address. (move data directly from read to write, can be used for copying data). */
#define ISP_DEF_TIL_1_READ_BUFFER_ADDRESS       (0x6C000000)      /** Point to tiling 1 read buffer address. */
#define ISP_DEF_TIL_1_WRITE_BUFFER_ADDRESS      (0x6C400000)      /** Point to tiling 1 write 0 buffer address. */


#pragma pack(push, 1)

typedef enum _EN_TIL_UPDATE {
    EN_TIL_UPDATE_NONE = 0X00000000,
    EN_TIL_1_UPDATE_READ_FORMAT = 0x00000001,
    EN_TIL_1_UPDATE_READ_DATA_PATH_SEL = 0x00000002,
    EN_TIL_1_UPDATE_READ_AXI_ID = 0x00000004,
    EN_TIL_1_UPDATE_READ_IMG_SIZE_STRIDE = 0x00000008,
    EN_TIL_1_UPDATE_READ_BUFFER = 0x00000010,
    EN_TIL_1_UPDATE_WRITE_FORMAT = 0x00000020,
    EN_TIL_1_UPDATE_WRITE_DATA_PATH_SEL = 0x00000040,
    EN_TIL_1_UPDATE_WRITE_AXI_ID = 0x00000080,
    EN_TIL_1_UPDATE_WRITE_IMG_SIZE_STRIDE = 0x00000100,
    EN_TIL_1_UPDATE_WRITE_BUFFER = 0x00000200,
    EN_TIL_1_UPDATE_IMG_SIZE = 0x00000400,
    EN_TIL_1_UPDATE_RAW_10_MODE = 0x00000800,
    EN_TIL_1_UPDATE_LOOP_BACK = 0x00001000,
    EN_TIL_1_UPDATE_MOVE_DRAM_DATA = 0x00002000,
    EN_TIL_1_UPDATE_READ_WO_BUG_INFO = (EN_TIL_1_UPDATE_READ_FORMAT
                                      | EN_TIL_1_UPDATE_READ_DATA_PATH_SEL
                                      | EN_TIL_1_UPDATE_READ_AXI_ID
                                      ),
    EN_TIL_1_UPDATE_READ = (EN_TIL_1_UPDATE_READ_FORMAT
                          | EN_TIL_1_UPDATE_READ_DATA_PATH_SEL
                          | EN_TIL_1_UPDATE_READ_AXI_ID
                          | EN_TIL_1_UPDATE_READ_IMG_SIZE_STRIDE
                          | EN_TIL_1_UPDATE_READ_BUFFER
                          ),
    EN_TIL_1_UPDATE_WRITE_WO_BUG_INFO = (EN_TIL_1_UPDATE_WRITE_FORMAT
                                       | EN_TIL_1_UPDATE_WRITE_DATA_PATH_SEL
                                       | EN_TIL_1_UPDATE_WRITE_AXI_ID
                                       ),
    EN_TIL_1_UPDATE_WRITE = (EN_TIL_1_UPDATE_WRITE_FORMAT
                           | EN_TIL_1_UPDATE_WRITE_DATA_PATH_SEL
                           | EN_TIL_1_UPDATE_WRITE_AXI_ID
                           | EN_TIL_1_UPDATE_WRITE_IMG_SIZE_STRIDE
                           | EN_TIL_1_UPDATE_WRITE_BUFFER
                           ),
    EN_TIL_1_UPDATE_COMMON = (EN_TIL_1_UPDATE_IMG_SIZE
                            | EN_TIL_1_UPDATE_RAW_10_MODE
                            | EN_TIL_1_UPDATE_LOOP_BACK
                            | EN_TIL_1_UPDATE_MOVE_DRAM_DATA
                            ),
    EN_TIL_UPDATE_ALL_WO_BUG_INFO = (EN_TIL_1_UPDATE_READ_WO_BUG_INFO
                                   | EN_TIL_1_UPDATE_WRITE_WO_BUG_INFO
                                   | EN_TIL_1_UPDATE_COMMON
                                   ),
    EN_TIL_UPDATE_ALL = (EN_TIL_1_UPDATE_READ
                       | EN_TIL_1_UPDATE_WRITE
                       | EN_TIL_1_UPDATE_COMMON
                       ),
} EN_TIL_UPDATE, *PEN_TIL_UPDATE;

typedef enum _EN_TIL_IMG_FMT {
    EN_TIL_IMG_FMT_RAW10_6PIX_1QW = 0,          //0 : RAW10 bit (6 pixels per 1QW) QW= 64bit word, msb 4bit no use.
    EN_TIL_IMG_FMT_RAW12_5PIX_1QW,              //1 : RAW12 bit (5 pixels per 1QW), msb 4bit no use.
    EN_TIL_IMG_FMT_YUV420_SEMI,                 //2 : YUV semi-planar 8bit: Y plane, UV plane 4:2:0 JPEG/JFIF progressive mode using the averaging value of the 4 UV points between horizontal and vertical.
    EN_TIL_IMG_FMT_RAW10_32PIX_5QW,             //3 : Optional fully packed RAW10 (320 bits = 32 pixels = 5QW), no redundant bit.
    EN_TIL_IMG_FMT_RAW12_16PIX_3QW,             //4 : Optional fully packed RAW12 (192 bits = 16 pixels = 3QW), no redundant bit.
    EN_TIL_IMG_FMT_YUV420_SEMI_HOR_AVG,         //5 : YUV semi-planar 8bit: Y plane, UV plane 4:2:0 mode using the averaging value of the 2 UV points between the horizontal direction.
    EN_TIL_IMG_FMT_YUV420_SEMI_1ST_EVEN,        //6 : YUV semi-planar 8bit: Y plane, UV plane 4:2:0 mode using the first position of the even horizontal with no averaging value.
    EN_TIL_IMG_FMT_RAW_FLEX,                    //7 : RAW flexible bit whose bit number is less than 25 bits with no gap package. The bit number is specified in register 0x0B38 bit[24:20](Read) and bit[8:4](Write).
    EN_TIL_IMG_FMT_MAX
} EN_TIL_IMG_FMT, *PEN_TIL_IMG_FMT;


//=== Godspeed === Add new memory/buffer type support here.
typedef struct _ST_TILING_BUF_INFO {
    CI_BUFFTYPE enAllocBufferType;              /** Output image buffer type.
                                                  *  0 : CI_TYPE_NONE,
                                                  *  1 : CI_TYPE_UO,
                                                  *  2 : CI_TYPE_SS0,
                                                  *  3 : CI_TYPE_SS1,
                                                  *  4 : CI_TYPE_DUMP,
                                                  *  5 : CI_TYPE_TILING_1_RD,
                                                  *  6 : CI_TYPE_TILING_1_WR,
                                                  *  7 : CI_TYPE_SC_DUMP,
                                                  *  8 : CI_TYPE_Y_HIST
                                                  */
    eFORMAT_TYPES enFormatType;                 /** Input/output image buffer format type. 0:TYPE_NONE, 1:TYPE_RGB, 2:TYPE_YUV and 3:TYPE_BAYER.  */
    ePxlFormat enPixelFormat;                   /** Input/Output image pixel format. 0:PXL_NONE, 1:YVU_420_PL12_8, 2:YUV_420_PL12_8, ..., PXL_N and PXL_INVALID. */
    eMOSAIC enMosaic;                           /** Input/Output image mosaic type. 0:MOSAIC_NONE, 1:MOSAIC_RGGB, 2:MOSAIC_GRBG, 3:MOSAIC_GBRG and 4:MOSAIC_BGGR. */
    ST_SIZE stSize;                             /** Input/Output image size. */
    STF_U32 u32Stride;                          /** Input/Output image line stride, 8-byte(64bit) granularity. */
    STF_VOID *pvYBuffer;                        /** Tiling input/output image Y plane data store buffer. */
    STF_VOID *pvUvBuffer;                       /** Tiling input/output image UV plane data store buffer. */
} ST_TILING_BUF_INFO, *PST_TILING_BUF_INFO;

typedef struct _ST_TIL_AXI_ID {
    STF_U8 u8YId;                               /** The AXI ID of the Y AXI read/write controller. */
    STF_U8 U8UvId;                              /** The AXI ID of the UV AXI read/write controller. */
} ST_TIL_AXI_ID, *PST_TIL_AXI_ID;

typedef struct _ST_TIL_RD_FMT {
    STF_U8 u8Format;                            /** Data package format type.
                                                 * RDFORMAT - Data Package Format type for the read path
                                                 * 3'h0 : RAW10 bit (6 pixels per 1QW) QW= 64bit word, msb 4bit no use.
                                                 * 3'h1 : RAW12 bit (5 pixels per 1QW), msb 4bit no use.
                                                 * 3'h2 : YUV semi-planar 8bit: Y plane, UV plane 4:2:0 JPEG/JFIF progressive mode using the averaging value of the 4 UV points between horizontal and vertical.
                                                 * 3'h3 : Optional fully packed RAW10 (320 bits = 32 pixels = 5QW), no redundant bit.
                                                 * 3'h4 : Optional fully packed RAW12 (192 bits = 16 pixels = 3QW), no redundant bit.
                                                 * 3'h5 : YUV semi-planar 8bit: Y plane, UV plane 4:2:0 mode using the averaging value of the 2 UV points between the horizontal direction.
                                                 * 3'h6 : YUV semi-planar 8bit: Y plane, UV plane 4:2:0 mode using the first position of the even horizontal with no averaging value.
                                                 * 3'h7 : RAW flexible bit whose bit number is less than 25 bits with no gap package. The bit number is specified in register 0x0B38 bit[24:20](Read) and bit[8:4](Write).
                                                 */
    STF_U8 u8FlexibleBit;                       /** The bit number of one RAW for data format is 3'h7.\n0 = BAYER_RGGB_NO_GAP_PACK_8,\n1 = BAYER_RGGB_NO_GAP_PACK_10,\n2 = BAYER_RGGB_NO_GAP_PACK_12,\n3 = BAYER_RGGB_NO_GAP_PACK_14,\n4 = RGB_888_24,\n5 = RGB_101010_30_NO_GAP,\n6 = PXL_ISP_444IL3YCbCr8,\n7 = PXL_ISP_444IL3YCbCr10_NO_GAP, */
    STF_BOOL8 bRdLineBufOff;                    /** Disable UV line buffer function for YUV de-package flow. */
} ST_TIL_RD_FMT, *PST_TIL_RD_FMT;

typedef struct _ST_TIL_WR_FMT {
    STF_U8 u8Format;                            /** Data package format type for the write path.
                                                 * 3'h0 : RAW10 bit (6 pixels per 1QW) QW= 64bit word, msb 4bit no use.
                                                 * 3'h1 : RAW12 bit (5 pixels per 1QW), msb 4bit no use.
                                                 * 3'h2 : YUV semi-planar 8bit: Y plane, UV plane 4:2:0 JPEG/JFIF progressive mode using the averaging value of the 4 UV points between horizontal and vertical.
                                                 * 3'h3 : Optional fully packed RAW10 (320 bits = 32 pixels = 5QW), no redundant bit.
                                                 * 3'h4 : Optional fully packed RAW12 (192 bits = 16 pixels = 3QW), no redundant bit.
                                                 * 3'h5 : YUV semi-planar 8bit: Y plane, UV plane 4:2:0 mode using the averaging value of the 2 UV points between the horizontal direction.
                                                 * 3'h6 : YUV semi-planar 8bit: Y plane, UV plane 4:2:0 mode using the first position of the even horizontal with no averaging value.
                                                 * 3'h7 : RAW flexible bit whose bit number is less than 25 bits with no gap package. The bit number is specified in register 0x0B38 bit[24:20](Read) and bit[8:4](Write).
                                                 * others : RSVD
                                                 */
    STF_U8 u8FlexibleBit;                       /** The bit number of one RAW for data format is 3'h7.\n0 = BAYER_RGGB_NO_GAP_PACK_8,\n1 = BAYER_RGGB_NO_GAP_PACK_10,\n2 = BAYER_RGGB_NO_GAP_PACK_12,\n3 = BAYER_RGGB_NO_GAP_PACK_14,\n4 = RGB_888_24,\n5 = RGB_101010_30_NO_GAP,\n6 = PXL_ISP_444IL3YCbCr8,\n7 = PXL_ISP_444IL3YCbCr10_NO_GAP, */
} ST_TIL_WR_FMT, *PST_TIL_WR_FMT;

typedef struct _ST_TIL_BS {
    STF_BOOL8 bTilWrBusyStatus;                 /** Write busy status, 0 : free, 1 : busy. (Read-only) */
    STF_BOOL8 bTilRdBusyStatus;                 /** Read busy status, 0 : free, 1 : busy. (Read-only) */
} ST_TIL_BS, *PST_TIL_BS;

typedef struct _ST_TIL_RD {
    //ST_SIZE stSize;                             /** Final read image window size. */
    //STF_U32 u32LineStride;                      /** Line stride information. */
    ST_TIL_RD_FMT stFormat;                     /** Read data format information. */
    STF_U8 u8DataPathSel;                       /** Read data path select information. The index of the selected module whose input will be read from DRAM buffer. */
                                                /**
                                                 * 8'h0 : Before DBC, RAW 10 bits, only for tiling read.
                                                 * 8'h1 : After CFA, RAW 10 bits.
                                                 * 8'h2 : After Sharpness, YUV 10 bits.
                                                 * 8'h3 : After Scaling-Output, YUV 10 bits, only for tiling write.
                                                 * others : RSVD
                                                 */
    STF_BOOL8 bIndependentRdRvi;                /** Enable the independent READ RVI signal function under the normal DRAM read mode. when set, read index can't set to 1. */
    STF_BOOL8 bRdNv21Format;                    /** Read UV swap, 0 : NV12, 1 : NV21. */
    ST_TIL_AXI_ID stAxiId;                      /** Tiling read interface AXI ID. */
} ST_TIL_RD, *PST_TIL_RD;

typedef struct _ST_TIL_WRT {
    //ST_SIZE stSize;                             /** Final write image window size. */
    //STF_U32 u32LineStride;                      /** Line stride information. */
    ST_TIL_WR_FMT stFormat;                     /** Write data format information. */
    STF_U8 u8DataPathSel;                       /** Write data path select information. The index of the selected module whose output will be written into DRAM buffer. */
                                                /**
                                                 * 8'h0 : Before DBC, RAW 10 bits, only for tiling read.
                                                 * 8'h1 : After CFA, RAW 10 bits.
                                                 * 8'h2 : After Sharpness, YUV 10 bits.
                                                 * 8'h3 : After Scaling-Output, YUV 10 bits, only for tiling write.
                                                 * others : RSVD
                                                 */
    STF_BOOL8 bWrNv21Format;                    /** Write UV swap, 0 : NV12, 1 : NV21. */
    ST_TIL_AXI_ID stAxiId;                      /** Tiling write interface AXI ID. */
} ST_TIL_WRT, *PST_TIL_WRT;

typedef struct _ST_TIL {
    ST_SIZE stSize;                             /** Final read/write image window size. */
    ST_TIL_RD stTilRead;                        /** ISP tiling interface read information. */
    ST_TIL_WRT stTilWrite;                      /** ISP tiling interface write information. */
    STF_U8 u8Raw10Mode;                         /** ISP RAW 10 bit format, 0 : RAW10 bit (6 pixels per 1QW), 1 : RAW10 bit (3 pixels per 1DW) */
    STF_BOOL8 bLoopBack;                        /** Test pipeline control, but data from read tiling to write tiling. */
    STF_BOOL8 bMoveDramData;                    /** Enable the move data function from the reading address to the writing address. (move data directly from read to write, can be used for copying data). */
} ST_TIL, *PST_TIL;

typedef struct _ST_MOD_TIL_SETTING {
    //-------------------------------------------------------------------------
    // This section parameters value is assign from setting file.
    STF_BOOL8 bEnableTil_1_Read;                /** Enable/Disable the read function of the TIL 1 interface from DRAM buffer. */
    STF_BOOL8 bEnableTil_1_Write;               /** Enable/Disable the write function of the TIL 1 interface into DRAM buffer. */
    ST_TIL stTiling_1;                          /** Tiling 1 parameters structure. */
} ST_MOD_TIL_SETTING, *PST_MOD_TIL_SETTING;

typedef struct _ST_MOD_TIL_INFO {
    ST_TIL_BS stTil_1_BusyStatus;               /** ISP tiling 1 interface busy status. */
} ST_MOD_TIL_INFO, *PST_MOD_TIL_INFO;

typedef struct _ST_TIL_PARAM {
    ST_MOD_TIL_SETTING stSetting;               /** Module TIL setting file parameters. */
    //-------------------------------------------------------------------------
    // This section parameters is internal using.
    ST_MOD_TIL_INFO stInfo;                     /** Module TIL output information parameters. */
    STF_BOOL8 bSetup;                           /** Flag set if the module setup has been completed. */
    STF_BOOL8 bUpdateTil_1_Read;                /** Enable/Disable program TIL 1 read module register update. */
    STF_BOOL8 bUpdateTil_1_Write;               /** Enable/Disable program TIL 1 write module register update. */
    STF_U32 u32UpdateRequest;                   /** Flag to indicate that this module update has been requested. */
    ST_TILING_BUF_INFO stTiling_1_RdBufInfo;    /** Image buffer information for Tiling 1 read. */
    ST_TILING_BUF_INFO stTiling_1_WrBufInfo;    /** Image buffer information for Tiling 1 write. */
} ST_TIL_PARAM, *PST_TIL_PARAM;

#define ST_MOD_TIL_SETTING_LEN          (sizeof(ST_MOD_TIL_SETTING))
#define ST_MOD_TIL_INFO_LEN             (sizeof(ST_MOD_TIL_INFO))
#if 0
#define ST_MOD_TIL_GET_PARAM_LEN        (ST_MOD_TIL_SETTING_LEN + ST_MOD_TIL_INFO_LEN)
#else
#define ST_MOD_TIL_GET_PARAM_LEN        (ST_MOD_TIL_SETTING_LEN)
#endif //#if 0
#define ST_MOD_TIL_SET_PARAM_LEN        (ST_MOD_TIL_SETTING_LEN)
//#if 0
//#define ST_TIL_PARAM_SIZE               (sizeof(ST_TIL_PARAM) - (sizeof(ST_TILING_BUF_INFO) * 2))
//#else
//#define ST_TIL_PARAM_SIZE               (sizeof(ST_TIL_PARAM) - (sizeof(ST_TIL_BS)) - (sizeof(ST_TILING_BUF_INFO) * 2))
//#endif
#define ST_TIL_PARAM_SIZE               (sizeof(ST_TIL_PARAM))
#define ST_TIL_PARAM_LEN                (ST_TIL_PARAM_SIZE)

typedef struct _ST_ISP_MOD_TIL {
#if defined(VIRTUAL_IO_MAPPING)
    STF_VOID *pPipeline;                        /** Pointer to Pipeline. */
#endif //VIRTUAL_IO_MAPPING
    STF_VOID *pIspCtx;                          /** Pointer to ISP context. */
    STF_U32 u32Id;                              /** Module ID. */
    STF_CHAR szName[16];                        /** Module Name. */
    CI_MEM_PARAM *pstTilRdmaBuf;                /** TIL's DMA buffer pointer for RDMA. */
#if defined(SUPPORT_MULTI_ISP)
    ST_ISP_TIL_RDMA *pstIspTilRdma;             /** TIL's RDMA structure pointer that include ISP index.*/
#endif //SUPPORT_MULTI_ISP
    ST_TIL_RDMA *pstTilRdma;                    /** TIL's RDMA structure pointer.*/
    ST_TIL_PARAM *pstTilParam;                  /** TIL's parameters structure pointer. */
    STF_BOOL8 bRegistered;                      /** Module register in pipeline. */
    STF_S32 (*Init)(STF_VOID *pIspCtx, STF_U32 *pNextRdma);
    STF_S32 (*LoadBinParam)(STF_VOID *pIspCtx, STF_U8 *pu8ParamBuf, STF_U16 u16ParamSize);
    STF_S32 (*LoadBinClbrt)(STF_VOID *pIspCtx);
    STF_S32 (*Enable)(STF_VOID *pIspCtx, STF_BOOL8 bEnable);
    STF_BOOL8 (*IsEnable)(STF_VOID *pIspCtx);
    STF_S32 (*Update)(STF_VOID *pIspCtx, STF_BOOL8 bUpdate);
    STF_BOOL8 (*IsUpdate)(STF_VOID *pIspCtx);
    STF_S32 (*Registered)(STF_VOID *pIspCtx, STF_BOOL8 bRegistered);
    STF_BOOL8 (*IsRegistered)(STF_VOID *pIspCtx);
    STF_S32 (*UpdateReg)(STF_VOID *pIspCtx);
    STF_S32 (*SetReg)(STF_VOID *pIspCtx);
    STF_S32 (*SetNext)(STF_VOID *pIspCtx, STF_U32 *pNextRdma);
    STF_S32 (*FreeResource)(STF_VOID *pIspCtx);
    STF_S32 (*GetIqParam)(STF_VOID *pIspCtx, STF_VOID *pParamBuf, STF_U16 *pu16ParamSize);
    STF_S32 (*SetIqParam)(STF_VOID *pIspCtx, STF_VOID *pParamBuf, STF_U16 u16ParamSize);
} ST_ISP_MOD_TIL, *PST_ISP_MOD_TIL;

#pragma pack(pop)


/* ISP tiling module parameters interface */
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_TIL_CalTilingBufFormat(
    STF_U8 u8DataPathSel,
    EN_TIL_IMG_FMT enTilingImageFormat,
    STF_U8 u8TilingFlexBit,
    eMOSAIC enMosaic,
    ST_TILING_BUF_INFO *pstTilingBufInfo,
    eFORMAT_TYPES *penFormatTypes
    );
extern
STF_S32 STFMOD_ISP_TIL_CalTilingBufInfo(
    STF_U8 u8DataPathSel,
    EN_TIL_IMG_FMT enTilingImageFormat,
    STF_U8 u8TilingFlexBit,
    eMOSAIC enMosaic,
    ST_SIZE *pstSize,
    ST_TILING_BUF_INFO *pstTilingBufInfo,
    eFORMAT_TYPES *penFormatTypes,
    PIXELTYPE *pstPxlType,
    struct CI_SIZEINFO *pstSizeInfo
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_TIL_InitRdma(
    ST_TIL_RDMA *pstTilRdma,
    ST_TIL_PARAM *pstTilParam,
    STF_U32 *pNextRdma
    );
extern
STF_S32 STFMOD_ISP_TIL_UpdateRdma(
    ST_TIL_RDMA *pstTilRdma,
    ST_TIL_PARAM *pstTilParam
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_TIL_1_UpdateTilRead(
    ST_TIL_RDMA *pstTilRdma,
    ST_TIL_PARAM *pstTilParam
    );
extern
STF_S32 STFMOD_ISP_TIL_1_UpdateReadImgSizeStride(
    ST_TIL_RDMA *pstTilRdma,
    ST_TIL_PARAM *pstTilParam
    );
extern
STF_S32 STFMOD_ISP_TIL_1_UpdateReadFormat(
    ST_TIL_RDMA *pstTilRdma,
    ST_TIL_PARAM *pstTilParam
    );
extern
STF_S32 STFMOD_ISP_TIL_1_UpdateReadDataPathSel(
    ST_TIL_RDMA *pstTilRdma,
    ST_TIL_PARAM *pstTilParam
    );
extern
STF_S32 STFMOD_ISP_TIL_1_UpdateReadAxiId(
    ST_TIL_RDMA *pstTilRdma,
    ST_TIL_PARAM *pstTilParam
    );
extern
STF_S32 STFMOD_ISP_TIL_1_UpdateReadBuf(
    ST_TIL_RDMA *pstTilRdma,
    ST_TIL_PARAM *pstTilParam
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_TIL_1_UpdateTilWrite(
    ST_TIL_RDMA *pstTilRdma,
    ST_TIL_PARAM *pstTilParam
    );
extern
STF_S32 STFMOD_ISP_TIL_1_UpdateWriteImgSizeStride(
    ST_TIL_RDMA *pstTilRdma,
    ST_TIL_PARAM *pstTilParam
    );
extern
STF_S32 STFMOD_ISP_TIL_1_UpdateWriteFormat(
    ST_TIL_RDMA *pstTilRdma,
    ST_TIL_PARAM *pstTilParam
    );
extern
STF_S32 STFMOD_ISP_TIL_1_UpdateWriteDataPathSel(
    ST_TIL_RDMA *pstTilRdma,
    ST_TIL_PARAM *pstTilParam
    );
extern
STF_S32 STFMOD_ISP_TIL_1_UpdateWriteAxiId(
    ST_TIL_RDMA *pstTilRdma,
    ST_TIL_PARAM *pstTilParam
    );
extern
STF_S32 STFMOD_ISP_TIL_1_UpdateWriteBuf(
    ST_TIL_RDMA *pstTilRdma,
    ST_TIL_PARAM *pstTilParam
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_TIL_1_UpdateCommon(
    ST_TIL_RDMA *pstTilRdma,
    ST_TIL_PARAM *pstTilParam
    );
extern
STF_S32 STFMOD_ISP_TIL_1_UpdateImageSize(
    ST_TIL_RDMA *pstTilRdma,
    ST_TIL_PARAM *pstTilParam
    );
extern
STF_S32 STFMOD_ISP_TIL_1_UpdateRaw10Mode(
    ST_TIL_RDMA *pstTilRdma,
    ST_TIL_PARAM *pstTilParam
    );
extern
STF_S32 STFMOD_ISP_TIL_1_UpdateLoopback(
    ST_TIL_RDMA *pstTilRdma,
    ST_TIL_PARAM *pstTilParam
    );
extern
STF_S32 STFMOD_ISP_TIL_1_UpdateMoveDramData(
    ST_TIL_RDMA *pstTilRdma,
    ST_TIL_PARAM *pstTilParam
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_TIL_SetNextRdma(
    ST_TIL_RDMA *pstTilRdma,
    STF_U32 *pNextRdma
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_TIL_SetUpdate(
    ST_ISP_MOD_TIL *pstModTil,
    STF_BOOL8 bUpdate
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_TIL_1_SetEnableTilRead(
    ST_ISP_MOD_TIL *pstModTil,
    STF_BOOL8 bEnable
    );
extern
STF_S32 STFMOD_ISP_TIL_1_SetUpdateTilRead(
    ST_ISP_MOD_TIL *pstModTil,
    STF_BOOL8 bUpdate
    );
extern
STF_S32 STFMOD_ISP_TIL_1_SetTilRead(
    ST_ISP_MOD_TIL *pstModTil,
    ST_TIL_RD *pstTilRead
    );
#if 0
extern
STF_S32 STFMOD_ISP_TIL_1_SetTilReadSize(
    ST_ISP_MOD_TIL *pstModTil,
    ST_SIZE stSize
    );
#endif
extern
STF_S32 STFMOD_ISP_TIL_1_SetTilReadLineStride(
    ST_ISP_MOD_TIL *pstModTil,
    STF_U32 u32LineStride
    );
extern
STF_S32 STFMOD_ISP_TIL_1_SetTilReadFormat(
    ST_ISP_MOD_TIL *pstModTil,
    ST_TIL_RD_FMT *pstFormat,
    eMOSAIC enMosaic
    );
extern
STF_S32 STFMOD_ISP_TIL_1_SetTilReadDataPathSel(
    ST_ISP_MOD_TIL *pstModTil,
    STF_U8 u8DataPathSel
    );
extern
STF_S32 STFMOD_ISP_TIL_1_SetTilReadIndependentRdRvi(
    ST_ISP_MOD_TIL *pstModTil,
    STF_BOOL8 bIndependentRdRvi
    );
extern
STF_S32 STFMOD_ISP_TIL_1_SetTilReadNv21Format(
    ST_ISP_MOD_TIL *pstModTil,
    STF_BOOL8 bRdNv21Format
    );
extern
STF_S32 STFMOD_ISP_TIL_1_SetTilReadAxiId(
    ST_ISP_MOD_TIL *pstModTil,
    ST_TIL_AXI_ID *pstAxiId
    );
extern
STF_S32 STFMOD_ISP_TIL_1_SetTilReadBuffer(
    STF_VOID *pIspCtx,
    STF_VOID *pvYBuf,
    STF_VOID *pvUvBuf
    );
extern
STF_S32 STFMOD_ISP_TIL_1_SetTilReadBufferInfo(
    STF_VOID *pIspCtx,
    ST_TILING_BUF_INFO *pstTilingBufInfo
    );
extern
ePxlFormat STFMOD_ISP_TIL_1_GetTilReadPixelFormat(
    STF_VOID *pIspCtx
    );
extern
ST_TILING_BUF_INFO STFMOD_ISP_TIL_1_GetTilReadBufferInfo(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_TIL_1_GetTilReadBuffer(
    STF_VOID *pIspCtx,
    STF_VOID **ppvYBuf,
    STF_VOID **ppvUvBuf
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_TIL_1_SetEnableTilWrite(
    ST_ISP_MOD_TIL *pstModTil,
    STF_BOOL8 bEnable
    );
extern
STF_S32 STFMOD_ISP_TIL_1_SetUpdateTilWrite(
    ST_ISP_MOD_TIL *pstModTil,
    STF_BOOL8 bUpdate
    );
extern
STF_S32 STFMOD_ISP_TIL_1_SetTilWrite(
    ST_ISP_MOD_TIL *pstModTil,
    ST_TIL_WRT *pstTilWrite
    );
#if 0
extern
STF_S32 STFMOD_ISP_TIL_1_SetTilWriteSize(
    ST_ISP_MOD_TIL *pstModTil,
    ST_SIZE stSize
    );
#endif
extern
STF_S32 STFMOD_ISP_TIL_1_SetTilWriteLineStride(
    ST_ISP_MOD_TIL *pstModTil,
    STF_U32 u32LineStride
    );
extern
STF_S32 STFMOD_ISP_TIL_1_SetTilWriteFormat(
    ST_ISP_MOD_TIL *pstModTil,
    ST_TIL_WR_FMT *pstFormat,
    eMOSAIC enMosaic
    );
extern
STF_S32 STFMOD_ISP_TIL_1_SetTilWriteDataPathSel(
    ST_ISP_MOD_TIL *pstModTil,
    STF_U8 u8DataPathSel
    );
extern
STF_S32 STFMOD_ISP_TIL_1_SetTilWriteNv21Format(
    ST_ISP_MOD_TIL *pstModTil,
    STF_BOOL8 bWrNv21Format
    );
extern
STF_S32 STFMOD_ISP_TIL_1_SetTilWriteAxiId(
    ST_ISP_MOD_TIL *pstModTil,
    ST_TIL_AXI_ID *pstAxiId
    );
extern
STF_S32 STFMOD_ISP_TIL_1_SetTilWriteBuffer(
    STF_VOID *pIspCtx,
    STF_VOID *pvYBuf,
    STF_VOID *pvUvBuf
    );
extern
STF_S32 STFMOD_ISP_TIL_1_SetTilWriteBufferInfo(
    STF_VOID *pIspCtx,
    ST_TILING_BUF_INFO *pstTilingBufInfo
    );
extern
ePxlFormat STFMOD_ISP_TIL_1_GetTilWritePixelFormat(
    STF_VOID *pIspCtx
    );
extern
ST_TILING_BUF_INFO STFMOD_ISP_TIL_1_GetTilWriteBufferInfo(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_TIL_1_GetTilWriteBuffer(
    STF_VOID *pIspCtx,
    STF_VOID **ppvYBuf,
    STF_VOID **ppvUvBuf
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_TIL_1_SetImageSize(
    STF_VOID *pIspCtx,
    ST_SIZE stSize
    );
extern
STF_S32 STFMOD_ISP_TIL_1_SetRaw10Mode(
    ST_ISP_MOD_TIL *pstModTil,
#if 0
    EN_TIL_RAW_10_MODE enRaw10Mode
#else
    STF_U8 u8Raw10Mode
#endif
    );
extern
STF_S32 STFMOD_ISP_TIL_1_SetLoopback(
    ST_ISP_MOD_TIL *pstModTil,
    STF_BOOL8 bLoopback
    );
extern
STF_S32 STFMOD_ISP_TIL_1_SetMoveDramData(
    ST_ISP_MOD_TIL *pstModTil,
    STF_BOOL8 bMoveDramData
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_TIL_Init(
    STF_VOID *pIspCtx,
    STF_U32 *pNextRdma
    );
extern
STF_S32 STFMOD_ISP_TIL_LoadBinParam(
    STF_VOID *pIspCtx,
    STF_U8 *pu8ParamBuf,
    STF_U16 u16ParamSize
    );
extern
STF_S32 STFMOD_ISP_TIL_LoadBinClbrt(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_TIL_Enable(
    STF_VOID *pIspCtx,
    STF_BOOL8 bEnable
    );
extern
STF_S32 STFMOD_ISP_TIL_1_EnableRead(
    STF_VOID *pIspCtx,
    STF_BOOL8 bEnable
    );
extern
STF_S32 STFMOD_ISP_TIL_1_EnableWrite(
    STF_VOID *pIspCtx,
    STF_BOOL8 bEnable
    );
extern
STF_BOOL8 STFMOD_ISP_TIL_IsEnable(
    STF_VOID *pIspCtx
    );
extern
STF_BOOL8 STFMOD_ISP_TIL_1_IsTilReadEnable(
    STF_VOID *pIspCtx
    );
extern
STF_BOOL8 STFMOD_ISP_TIL_1_IsTilWriteEnable(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_TIL_Update(
    STF_VOID *pIspCtx,
    STF_BOOL8 bUpdate
    );
extern
STF_S32 STFMOD_ISP_TIL_1_UpdateRead(
    STF_VOID *pIspCtx,
    STF_BOOL8 bUpdate
    );
extern
STF_S32 STFMOD_ISP_TIL_1_UpdateWrite(
    STF_VOID *pIspCtx,
    STF_BOOL8 bUpdate
    );
extern
STF_BOOL8 STFMOD_ISP_TIL_IsUpdate(
    STF_VOID *pIspCtx
    );
extern
STF_BOOL8 STFMOD_ISP_TIL_1_IsUpdateRead(
    STF_VOID *pIspCtx
    );
extern
STF_BOOL8 STFMOD_ISP_TIL_1_IsUpdateWrite(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_TIL_Registered(
    STF_VOID *pIspCtx,
    STF_BOOL8 bRegistered
    );
extern
STF_BOOL8 STFMOD_ISP_TIL_IsRegistered(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_TIL_UpdateReg(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_TIL_SetReg(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_TIL_SetNext(
    STF_VOID *pIspCtx,
    STF_U32 *pNextRdma
    );
extern
STF_S32 STFMOD_ISP_TIL_FreeResource(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_TIL_GetIqParam(
    STF_VOID *pIspCtx,
    STF_VOID *pParamBuf,
    STF_U16 *pu16ParamSize
    );
extern
STF_S32 STFMOD_ISP_TIL_SetIqParam(
    STF_VOID *pIspCtx,
    STF_VOID *pParamBuf,
    STF_U16 u16ParamSize
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_TIL_Create(
    STF_VOID *pPipeline
    );
extern
STF_S32 STFMOD_ISP_TIL_Destroy(
    STF_VOID *pPipeline
    );
//-----------------------------------------------------------------------------


#ifdef __cplusplus
}
#endif


#endif //__STFMOD_ISP_TIL_H__
