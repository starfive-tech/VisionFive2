/**
  ******************************************************************************
  * @file  stflib_isp_pipeline.h
  * @author  StarFive Isp Team
  * @version  V1.0
  * @date  09/07/2020
  * @brief Declaration of ISPC::Pipeline
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
  ******************************************************************************/


#ifndef __STFLIB_ISP_PIPELINE_H__
#define __STFLIB_ISP_PIPELINE_H__


#ifdef __cplusplus
extern "C" {
#endif


#include "stf_include.h"
#include "ci/ci_api_structs.h"

#ifdef UNUSED_CODE_AND_VARIABLE
#include "ISPC/Module.h"
#endif //UNUSED_CODE_AND_VARIABLE
#include "ISPC/stflib_isp_shot.h"
#include "ISPC/stflib_isp_defs.h"
#include "ISPC/stflib_isp_sensor.h"
#if defined(V4L2_DRIVER)
#include "ISPC/stflib_isp_device.h"
#endif //#if defined(V4L2_DRIVER)
#if defined(USE_MUTEX_PROTECT_PIPELINE_BUF) || defined(USE_MUTEX_PROTECT_MODULE_CONTROL_PARAM)

#include <pthread.h>
#endif //#if defined(USE_MUTEX_PROTECT_PIPELINE_BUF) || defined(USE_MUTEX_PROTECT_MODULE_CONTROL_PARAM)


#define TUNING_TOOL_CONTROL_ID_START_NO         (0x80)


/* pipeline parameters structure */
// Define initial values


//typedef struct _ST_MODULE_FUN *MODULE_HANDLE;
//
//
//typedef struct _ST_MODULE_INFO {
//    STF_U8 u8ModuleId;
//    STF_CHAR szModuleName[MODULE_NAME_MAX];
//    STF_BOOL8 bSetupFlag;
//    STF_U32 u32UpdateRequest;                   /** Flag to indicate that this module update has been requested. */
//} ST_MODULE_INFO, *PST_MODULE_INFO;
//
////typedef struct _ST_MODULE_FUN {
////    STF_RESULT (*UpdateParam)(MODULE_HANDLE hHandle, STF_U16 nIndex,
////        SENSOR_MODE *psModes);
////
////};


typedef enum _EN_MODULE_ID {
    EN_MODULE_ID_SUD_CSI_OFF = 0,   // 00
    EN_MODULE_ID_SUD_ISP_OFF,       // 01
    EN_MODULE_ID_CSI_IN,            // 02
    EN_MODULE_ID_CBAR,              // 03
    EN_MODULE_ID_CROP,              // 04
    EN_MODULE_ID_DC,                // 05
    EN_MODULE_ID_DEC,               // 06
    EN_MODULE_ID_OBA,               // 07
    EN_MODULE_ID_OBC,               // 08
    EN_MODULE_ID_CPD,               // 09
    EN_MODULE_ID_LCBQ,              // 10
    EN_MODULE_ID_SC,                // 11
    EN_MODULE_ID_DUMP,              // 12
    EN_MODULE_ID_ISP_IN,            // 13
    EN_MODULE_ID_DBC,               // 14
    EN_MODULE_ID_CTC,               // 15
    EN_MODULE_ID_STNR,              // 16
    EN_MODULE_ID_OECF,              // 17
    EN_MODULE_ID_OECFHM,            // 18
    EN_MODULE_ID_LCCF,              // 19
    EN_MODULE_ID_LS,                // 20
    EN_MODULE_ID_AWB,               // 21
    EN_MODULE_ID_PF,                // 22
    EN_MODULE_ID_CA,                // 23
    EN_MODULE_ID_CFA,               // 24
    EN_MODULE_ID_CAR,               // 25
    EN_MODULE_ID_CCM,               // 26
    EN_MODULE_ID_LUT,               // 27
    EN_MODULE_ID_GMARGB,            // 28
    EN_MODULE_ID_R2Y,               // 29
    EN_MODULE_ID_WDR,               // 30
    EN_MODULE_ID_YHIST,             // 31
    EN_MODULE_ID_YCRV,              // 32
    EN_MODULE_ID_SHRP,              // 33
    EN_MODULE_ID_DNYUV,             // 34
    EN_MODULE_ID_SAT,               // 35
    EN_MODULE_ID_OUT_UO,            // 36
    EN_MODULE_ID_OUT_SS0,           // 37
    EN_MODULE_ID_OUT_SS1,           // 38
    EN_MODULE_ID_OUT,               // 39
    EN_MODULE_ID_TIL_1_RD,          // 40
    EN_MODULE_ID_TIL_1_WR,          // 41
    EN_MODULE_ID_TIL_2_RD,          // 42
    EN_MODULE_ID_TIL_2_WR,          // 43
    EN_MODULE_ID_TIL,               // 44
    EN_MODULE_ID_BUF,               // 45
    EN_MODULE_ID_SUD_CSI,           // 46
    EN_MODULE_ID_SUD_ISP,           // 47
    EN_MODULE_ID_MAX                // 48
} EN_MODULE_ID, *PEN_MODULE_ID;

typedef enum _EN_CONTROL_ID {
    EN_CONTROL_ID_AE = 0,           // 00   // 128
    EN_CONTROL_ID_AWB,              // 01   // 129
    EN_CONTROL_ID_CSI_IN,           // 02   // 130
    EN_CONTROL_ID_CBAR,             // 03   // 131
    EN_CONTROL_ID_CROP,             // 04   // 132
    EN_CONTROL_ID_DC,               // 05   // 133
    EN_CONTROL_ID_DEC,              // 06   // 134
    EN_CONTROL_ID_OBA,              // 07   // 135
    EN_CONTROL_ID_OBC,              // 08   // 136
    EN_CONTROL_ID_CPD,              // 09   // 137
    EN_CONTROL_ID_LCBQ,             // 10   // 138
    EN_CONTROL_ID_SC,               // 11   // 139
    EN_CONTROL_ID_DUMP,             // 12   // 140
    EN_CONTROL_ID_ISP_IN,           // 13   // 141
    EN_CONTROL_ID_DBC,              // 14   // 142
    EN_CONTROL_ID_CTC,              // 15   // 143
    EN_CONTROL_ID_STNR,             // 16   // 144
    EN_CONTROL_ID_OECF,             // 17   // 145
    EN_CONTROL_ID_OECFHM,           // 18   // 146
    EN_CONTROL_ID_LCCF,             // 19   // 147
    EN_CONTROL_ID_LS,               // 20   // 148
    EN_CONTROL_ID_PF,               // 21   // 149
    EN_CONTROL_ID_CA,               // 22   // 150
    EN_CONTROL_ID_CFA,              // 23   // 151
    EN_CONTROL_ID_CAR,              // 24   // 152
    EN_CONTROL_ID_CCM,              // 25   // 153
    EN_CONTROL_ID_LUT,              // 26   // 154
    EN_CONTROL_ID_GMARGB,           // 27   // 155
    EN_CONTROL_ID_R2Y,              // 28   // 156
    EN_CONTROL_ID_WDR,              // 29   // 157
    EN_CONTROL_ID_YHIST,            // 30   // 158
    EN_CONTROL_ID_YCRV,             // 31   // 159
    EN_CONTROL_ID_SHRP,             // 32   // 160
    EN_CONTROL_ID_DNYUV,            // 33   // 161
    EN_CONTROL_ID_SAT,              // 34   // 162
    EN_CONTROL_ID_OUT_UO,           // 35   // 163
    EN_CONTROL_ID_OUT_SS0,          // 36   // 164
    EN_CONTROL_ID_OUT_SS1,          // 37   // 165
    EN_CONTROL_ID_OUT,              // 38   // 166
    EN_CONTROL_ID_TIL_1_RD,         // 39   // 167
    EN_CONTROL_ID_TIL_1_WR,         // 40   // 168
    EN_CONTROL_ID_TIL_2_RD,         // 41   // 169
    EN_CONTROL_ID_TIL_2_WR,         // 42   // 170
    EN_CONTROL_ID_TIL,              // 43   // 171
    EN_CONTROL_ID_BUF,              // 44   // 172
    EN_CONTROL_ID_SUD_CSI,          // 45   // 173
    EN_CONTROL_ID_SUD_ISP,          // 46   // 174
    EN_CONTROL_ID_MAX,              // 47   // 175
} EN_CONTROL_ID, *PEN_CONTROL_ID;

//=== Godspeed === Add new memory/buffer type support here.
typedef enum _EN_PIPELINE_BUF_TYPE {
    EN_PIPELINE_BUF_TYPE_UO = (1 << 0),
    EN_PIPELINE_BUF_TYPE_SS0 = (1 << 1),
    EN_PIPELINE_BUF_TYPE_SS1 = (1 << 2),
    EN_PIPELINE_BUF_TYPE_DUMP = (1 << 3),
    EN_PIPELINE_BUF_TYPE_TIL_1_RD = (1 << 4),
    EN_PIPELINE_BUF_TYPE_TIL_1_WR = (1 << 5),
    EN_PIPELINE_BUF_TYPE_ALL_OUT = (EN_PIPELINE_BUF_TYPE_UO
                                  | EN_PIPELINE_BUF_TYPE_SS0
                                  | EN_PIPELINE_BUF_TYPE_SS1
                                  | EN_PIPELINE_BUF_TYPE_DUMP
                                  | EN_PIPELINE_BUF_TYPE_TIL_1_WR
                                  ),
    EN_PIPELINE_BUF_TYPE_ALL = (EN_PIPELINE_BUF_TYPE_ALL_OUT
                              | EN_PIPELINE_BUF_TYPE_TIL_1_RD
                              ),
} EN_PIPELINE_BUF_TYPE, *PEN_PIPELINE_BUF_TYPE;

typedef enum _EN_PIPELINE_BUF_ID {
    EN_PIPELINE_BUF_ID_UO = 0,
    EN_PIPELINE_BUF_ID_SS0,
    EN_PIPELINE_BUF_ID_SS1,
    EN_PIPELINE_BUF_ID_DUMP,
    EN_PIPELINE_BUF_ID_TIL_1_RD,
    EN_PIPELINE_BUF_ID_TIL_1_WR,
    EN_PIPELINE_BUF_ID_SC_DUMP,
    EN_PIPELINE_BUF_ID_YHIST,
    EN_PIPELINE_BUF_ID_MAX,
} EN_PIPELINE_BUF_ID, *PEN_PIPELINE_BUF_ID;

typedef enum _EN_STAT_BUF_TYPE {
    EN_STAT_BUF_TYPE_SC_DUMP = 0,
    EN_STAT_BUF_TYPE_Y_HIST,
//    EN_STAT_BUF_TYPE_MAX
} EN_STAT_BUF_TYPE, *PEN_STAT_BUF_TYPE;
#if defined(USE_MUTEX_PROTECT_MODULE_CONTROL_PARAM)

typedef enum _EN_MUTEX_ID {
    EN_MUTEX_ID_CTL_AE_0 = 0,
    EN_MUTEX_ID_CTL_AE_1,
    EN_MUTEX_ID_CTL_AWB_0,
    EN_MUTEX_ID_CTL_AWB_1,
    EN_MUTEX_ID_CTL_CCM_0,
    EN_MUTEX_ID_CTL_CCM_1,
    EN_MUTEX_ID_CTL_DNYUV_LCCF_0,
    EN_MUTEX_ID_CTL_DNYUV_LCCF_1,
    EN_MUTEX_ID_CTL_SAT_SHRP_0,
    EN_MUTEX_ID_CTL_SAT_SHRP_1,
    EN_MUTEX_ID_CTL_YCRV_0,
    EN_MUTEX_ID_CTL_YCRV_1,
    EN_MUTEX_ID_MOD_CBAR_CSI_ISP_IN_0,
    EN_MUTEX_ID_MOD_CBAR_CSI_ISP_IN_1,
    EN_MUTEX_ID_MOD_AWB_0,
    EN_MUTEX_ID_MOD_AWB_1,
    EN_MUTEX_ID_MOD_BUF_0,
    EN_MUTEX_ID_MOD_BUF_1,
    EN_MUTEX_ID_MOD_CA_CAR_CFA_CPD_CROP_CTC_DBC_DC_DEC_0,
    EN_MUTEX_ID_MOD_CA_CAR_CFA_CPD_CROP_CTC_DBC_DC_DEC_1,
    EN_MUTEX_ID_MOD_CCM_0,
    EN_MUTEX_ID_MOD_CCM_1,
    EN_MUTEX_ID_MOD_DNYUV_GMARGB_LCBQ_LCCF_LS_LUT_0,
    EN_MUTEX_ID_MOD_DNYUV_GMARGB_LCBQ_LCCF_LS_LUT_1,
    EN_MUTEX_ID_MOD_DUMP_0,
    EN_MUTEX_ID_MOD_DUMP_1,
    EN_MUTEX_ID_MOD_OBA_OBC_OECF_OECFHM_0,
    EN_MUTEX_ID_MOD_OBA_OBC_OECF_OECFHM_1,
    EN_MUTEX_ID_MOD_OUT_0,
    EN_MUTEX_ID_MOD_OUT_1,
    EN_MUTEX_ID_MOD_OUT_UO_SSX_0,
    EN_MUTEX_ID_MOD_OUT_UO_SSX_1,
    EN_MUTEX_ID_MOD_R2Y_SAT_SHRP_0,
    EN_MUTEX_ID_MOD_R2Y_SAT_SHRP_1,
    EN_MUTEX_ID_MOD_SC_0,
    EN_MUTEX_ID_MOD_SC_1,
    EN_MUTEX_ID_MOD_SUD_CSI_ISP_0,
    EN_MUTEX_ID_MOD_SUD_CSI_ISP_1,
    EN_MUTEX_ID_MOD_TIL_0,
    EN_MUTEX_ID_MOD_TIL_1,
    EN_MUTEX_ID_MOD_TIL_1_RD_WR_0,
    EN_MUTEX_ID_MOD_TIL_1_RD_WR_1,
    EN_MUTEX_ID_MOD_YCRV_0,
    EN_MUTEX_ID_MOD_YCRV_1,
    EN_MUTEX_ID_MOD_YHIST_0,
    EN_MUTEX_ID_MOD_YHIST_1,
    EN_MUTEX_ID_MAX
} EN_MUTEX_ID, *PEN_MUTEX_ID;
#endif //#if defined(USE_MUTEX_PROTECT_MODULE_CONTROL_PARAM)
#if defined(V4L2_DRIVER)

typedef enum _EN_ISP_PORT_ID {
    EN_ISP_PORT_ID_UO,                  // 00
    EN_ISP_PORT_ID_SS0,                 // 01
    EN_ISP_PORT_ID_SS1,                 // 02
    EN_ISP_PORT_ID_TIL_1_WR,            // 03
    EN_ISP_PORT_ID_TIL_1_RD,            // 04
    EN_ISP_PORT_ID_DUMP,                // 05
    EN_ISP_PORT_ID_SC_DUMP_YHIST,       // 06
} EN_ISP_PORT_ID, *PEN_ISP_PORT_ID;
#define EN_ISP_PORT_ID_MAX              (EN_ISP_PORT_ID_SC_DUMP_YHIST + 1)
#endif //#if defined(V4L2_DRIVER)


#if defined(USE_MUTEX_PROTECT_MODULE_CONTROL_PARAM)
extern pthread_mutex_t g_stModCtlParamLock[EN_MUTEX_ID_MAX][2];
#endif //#if defined(USE_MUTEX_PROTECT_MODULE_CONTROL_PARAM)
#if defined(V4L2_DRIVER)
extern const STF_INT g_nIspVideoDeviceIndex[];
extern const STF_CHAR g_szIspVideoDeviceName[][24];
#endif //#if defined(V4L2_DRIVER)


#pragma pack(push, 1)

typedef union _ST_VERSION {
    STF_U32 u32Version;
    struct {
        STF_U8 u8BuildVersion;                  /** Build version. */
        STF_U8 u8MinorVersion;                  /** Minor version. */
        STF_U16 u16MajorVersion;                /** Major version. */
    } field;
} ST_VERSION, *PST_VERSION;

typedef struct _ST_HEADER {
    STF_U8 u8Id[4];                             /** It is using for identify the binary setting file. */
    STF_U32 u32DataSize;                        /** Indicate whole of binary setting file size. It is include the header, module/control information, module/control parameters and checksum. */
    ST_VERSION stVersion;                       /** Indicate the module/control parameters version. */
} ST_HEADER, *PST_HEADER;

//typedef struct _ST_TIL_BUF_INFO {
//    STF_U8 u8Enable;                            /** Enable tiling write/capture. */
//    STF_U8 u8ImgType;                           /** Image type, 0 - EN_TIL_IMG_TYPE_BAYER, 1 - EN_TIL_IMG_TYPE_RGB, 2 - EN_TIL_IMG_TYPE_YUV. */
//    STF_U8 u8UvSWap;                            /** UV swap. */
//    STF_U8 u8FlexibleBits;                      /** Bits per packet. */
//    STF_U8 u8YuvBits;                           /** Bits per pixel. */
//    STF_U8 u8Format;                            /** Image format. */
//    STF_U8 u8Compress;                          /** 0 - no compress, 1 - image data has been compress. */
//    STF_U16 *pu16ImgBuf;                        /** Write/Capture image buffer. */
//} ST_TIL_BUF_INFO, *PST_TIL_BUF_INFO;
//
//typedef struct _ST_BUF_INFO {
//    ST_SIZE stImgSize;                          /** Image size */
//    ST_TIL_BUF_INFO stWriteImg;                 /** Tiling write image buffer information. */
//    ST_TIL_BUF_INFO stCaptureImg;               /** Tiling write image buffer information. */
//    STF_U32 *pu32ScDumpBuf;                     /** SC dumping buffer. */
//    STF_U32 *pu32YHistDumpBuf;                  /** Y histogram dumping buffer. */
//} ST_BUF_INFO, *PST_BUF_INFO;
//
//typedef struct _ST_BUF_TBL {
//    STF_U8 u8TblCnt;                            /** Indicate buffer table counter. */
//    ST_BUF_INFO stBuf[PIPELINE_IMG_BUF_MAX];  /** Buffer table. */
//} ST_BUF_TBL, *PST_BUF_TBL;
//
//typedef struct _ST_BUF_QUEUE {
//    STF_U8 u8TblCnt;                            /** Indicate buffer table counter. */
//    STF_U8 u8TblIdx;                            /** Indicate next use buffer. */
//    STF_CHAR szName[16];                        /** Queue Name. */
//    ST_BUF_INFO *pstBuf[PIPELINE_IMG_BUF_MAX];  /** Buffer pointer table. */
//} ST_BUF_QUEUE, *PST_BUF_QUEUE;
//
typedef struct _ST_RAW_DUMP_TBL {
    STF_U8 u8Idx;                               /** Indicate current use buffer. */
    STF_U8 u8TblCnt;                            /** Indicate total allocate buffer counter. */
    STF_U8 u8CaptureCount;                      /** Indicate how many images to capture. */
    CI_BUFFTYPE enBufferType;                   /** Indicate those buffer will be use for which image buffer. */
#if defined(V4L2_DRIVER)
    CI_MEM_PARAM *pstDumpMem[CONTIGUOUS_CAPTURE_BUF_MAX];   /** RAW dump table. */
#else
    CI_MEM_PARAM stDumpMem[CONTIGUOUS_CAPTURE_BUF_MAX];     /** RAW dump table. */
#endif //#if defined(V4L2_DRIVER)
} ST_RAW_DUMP_TBL, *PST_RAW_DUMP_TBL;

typedef struct _ST_SHOT_TBL {
    STF_U8 u8TblCnt;                            /** Indicate buffer table counter. */
    ST_SHOT_INFO stShot[PIPELINE_IMG_BUF_MAX];  /** Shot table. */
} ST_SHOT_TBL, *PST_SHOT_TBL;

typedef struct _ST_SHOT_QUEUE {
    STF_U8 u8TblCnt;                            /** Indicate shot table counter. */
    STF_U8 u8TblIdx;                            /** Indicate next use shot. */
    STF_CHAR szName[16];                        /** Queue Name. */
    ST_SHOT_INFO *pstShot[PIPELINE_IMG_BUF_MAX]; /** Shot pointer table. */
} ST_SHOT_QUEUE, *PST_SHOT_QUEUE;

typedef struct _ST_MOD_CTL_INFO {
    STF_U8 u8Id;                                /** Module and/or control ID. Set the bit 7 as high to indicate it is control ID.*/
    STF_U32 u32Offset;                          /** Indicate this module/control parameters location offset (, count from Id field). */
    STF_U16 u16ParamSize;                       /** Indicate this module/control parameters size. */
} ST_MOD_CTL_INFO, *PST_MOD_CTL_INFO;

typedef struct _ST_ISP_BIN_PARAM {
    ST_HEADER stHeader;                         /** ISP binary setting file basically header. */
    STF_U8 u8Count;                             /** Indicate how much module/control parameters. */
    ST_MOD_CTL_INFO pstModCtlInfo[];            /** Module/Control information. */
} ST_ISP_BIN_PARAM, *PST_ISP_BIN_PARAM;

typedef struct _ST_ISP_MOD {
#if defined(VIRTUAL_IO_MAPPING)
    STF_VOID *pPipeline;                        /** Pointer to Pipeline. */
#endif //VIRTUAL_IO_MAPPING
    STF_VOID *pIspCtx;                          /** Pointer to ISP context. */
    STF_U32 u32Id;                              /** Module's/Control's ID. */
    STF_CHAR szName[16];                        /** Module's/Control's Name. */
    CI_MEM_PARAM *pstRdmaBuf;                   /** Module's/Control's DMA buffer pointer for RDMA. */
#if defined(SUPPORT_MULTI_ISP)
    STF_VOID *pstIspRdma;                       /** Module's/Control's RDMA structure pointer that include the ISP index. */
#endif //SUPPORT_MULTI_ISP
    STF_VOID *pstRdma;                          /** Module's/Control's RDMA structure pointer. */
    STF_VOID *pstParam;                         /** Module's/Control's parameters structure pointer. */
    STF_BOOL8 bRegistered;                      /** Module's/Control's register in pipeline. */
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
} ST_ISP_MOD, *PST_ISP_MOD;

typedef struct _ST_ISP_CTL {
#if defined(VIRTUAL_IO_MAPPING)
    STF_VOID *pPipeline;                        /** Pointer to Pipeline. */
#endif //VIRTUAL_IO_MAPPING
    STF_VOID *pIspCtx;                          /** Pointer to ISP context. */
    STF_U32 u32Id;                              /** Module/Control ID. */
    STF_CHAR szName[16];                        /** Module/Control Name. */
    //STF_VOID *psRdma;                           /** Module/Control RDMA structure pointer. */
    STF_VOID *pstParam;                         /** Module/Control parameters structure pointer. */
    STF_BOOL8 bRegistered;                      /** Module/Control register in pipeline. */
    STF_S32 (*Init)(STF_VOID *pIspCtx);
    STF_S32 (*LoadBinParam)(STF_VOID *pIspCtx, STF_U8 *pu8ParamBuf, STF_U16 u16ParamSize);
    STF_S32 (*Enable)(STF_VOID *pIspCtx, STF_BOOL8 bEnable);
    STF_BOOL8 (*IsEnable)(STF_VOID *pIspCtx);
    STF_S32 (*Registered)(STF_VOID *pIspCtx, STF_BOOL8 bRegistered);
    STF_BOOL8 (*IsRegistered)(STF_VOID *pIspCtx);
    STF_S32 (*Calculate)(STF_VOID *pIspCtx);
    STF_S32 (*FreeResource)(STF_VOID *pIspCtx);
    STF_S32 (*GetIqParam)(STF_VOID *pIspCtx, STF_VOID *pParamBuf, STF_U16 *pu16ParamSize);
    STF_S32 (*SetIqParam)(STF_VOID *pIspCtx, STF_VOID *pParamBuf, STF_U16 u16ParamSize);
} ST_ISP_CTL, *PST_ISP_CTL;

typedef struct _ST_ISP_CTX {
#if defined(VIRTUAL_IO_MAPPING)
    STF_VOID *pPipeline;                        /** Pointer to Pipeline. */
#endif //VIRTUAL_IO_MAPPING
    /** @brief HW ISP to use */
    STF_U8 u8IspIdx;
    ST_SIZE stImgSize;
    STF_U8 *pu8BinParamBuf;
    ST_ISP_BIN_PARAM *pstIspBinParam;
//    ST_BUF_INFO *pstAvailableBuf;
//    ST_BUF_INFO *pstPendingBuf;
//    ST_BUF_INFO *pstCompleteBuf;
//#if defined(PIPELINE_OUTPUT_QUEUE_ENABLE)
//    ST_BUF_INFO *pstOutputBuf;
//#endif //PIPELINE_OUTPUT_QUEUE_ENABLE
//    ST_BUF_TBL stImgBuf;
//    ST_BUF_QUEUE stAvailableImgBufQueue;
//    ST_BUF_QUEUE stPendingImgBufQueue;
//    ST_BUF_QUEUE stCompleteImgBufQueue;
#if defined(PIPELINE_OUTPUT_QUEUE_ENABLE)
    ST_BUF_QUEUE stOutputImgBufQueue;
#endif //PIPELINE_OUTPUT_QUEUE_ENABLE
    STF_BOOL8 m_bIqTuningDebugInfoEnable;
    ST_ISP_MOD *pstIspModCtx[EN_MODULE_ID_MAX];
    ST_ISP_CTL *pstIspCtlCtx[EN_CONTROL_ID_MAX];
#if defined(V4L2_DRIVER)
    STF_BOOL8 m_bUserConfigIsp;
    ST_CI_DEVICE stVideoDevice[EN_ISP_PORT_ID_MAX];
#endif //if defined(V4L2_DRIVER)

#if 0
    /**
     * @brief Get a module form the ISP context.
     */
    STF_VOID *(*GetModule)(STF_VOID *pIspCtx, EN_MODULE_ID enModuleId);

    /**
     * @brief Get the module's name form the ISP context.
     */
    STF_VOID *(*GetModuleName)(STF_VOID *pIspCtx, EN_MODULE_ID enModuleId);

    /**
     * @brief Get the module's DMA buffer pointer of Rdma form the ISP context.
     */
    STF_VOID *(*GetModuleRdmaBuf)(STF_VOID *pIspCtx, EN_MODULE_ID enModuleId);

    /**
     * @brief Get the module's IspRdma form the ISP context.
     */
    STF_VOID *(*GetModuleIspRdma)(STF_VOID *pIspCtx, EN_MODULE_ID enModuleId);

    /**
     * @brief Get the module's Rdma form the ISP context.
     */
    STF_VOID *(*GetModuleRdma)(STF_VOID *pIspCtx, EN_MODULE_ID enModuleId);

    /**
     * @brief Get the module's Param form the ISP context.
     */
    STF_VOID *(*GetModuleParam)(STF_VOID *pIspCtx, EN_MODULE_ID enModuleId);

    /**
     * @brief Get a control form the ISP context.
     */
    STF_VOID *(*GetControl)(STF_VOID *pIspCtx, EN_CONTROL_ID enControlId);

    /**
     * @brief Get the control's name form the ISP context.
     */
    STF_VOID *(*GetControlName)(STF_VOID *pIspCtx, EN_CONTROL_ID enControlId);

    /**
     * @brief Get the control's Param form the ISP context.
     */
    STF_VOID *(*GetControlParam)(STF_VOID *pIspCtx, EN_CONTROL_ID enControlId);
#endif
} ST_ISP_CTX, *PST_ISP_CTX;

typedef struct _ST_PIPELINE ST_PIPELINE;

typedef struct _ST_PIPELINE {
#ifdef UNUSED_CODE_AND_VARIABLE
    /**
     * @brief The module registry: stores the set of Module objects (1 per
     * actual HW module) used to setup the pipeline
     */
    std::map<SetupID, SetupModule *> moduleRegistry;

    typedef std::map<SetupID, SetupModule *>::const_iterator const_iterator;

#endif //UNUSED_CODE_AND_VARIABLE
    /** @brief indicate which ISP will be connect and use */
    STF_U8 u8IspIdx;

    /**
     * @brief The module registry: stores the set of Module objects (1 per
     * actual HW module) used to setup the pipeline
     */
    ST_ISP_CTX stIspCtx;

    /** @brief pointer to the camera sensor object */
    ST_SENSOR *pstSensor;

    /** @brief Connection to the CI layer */
    CI_CONNECTION *pstCIConnection;

#if !defined(V4L2_DRIVER)
    /** @brief pointer to a pipeline configuration in CI level */
    CI_PIPELINE *pstCIPipeline;

#endif //#if !defined(V4L2_DRIVER)
    //=== Godspeed === Add new memory/buffer type support here.
    /**
     * @brief Maximum output width size of the UO scaler in pixels.
     */
    STF_U16 u16MaxUoWidth;
    /**
     * @brief Maximum output height size of the UO scaler in pixels.
     */
    STF_U16 u16MaxUoHeight;

    /**
     * @brief Maximum output width size of the SS0 scaler in pixels.
     */
    STF_U16 u16MaxSs0Width;
    /**
     * @brief Maximum output height size of the SS0 scaler in pixels.
     */
    STF_U16 u16MaxSs0Height;

    /**
     * @brief Maximum output width size of the SS1 scaler in pixels.
     */
    STF_U16 u16MaxSs1Width;
    /**
     * @brief Maximum output height size of the SS1 scaler in pixels.
     */
    STF_U16 u16MaxSs1Height;

    /**
     * @brief Maximum output width size of the DUMP scaler in pixels.
     */
    STF_U16 u16MaxDumpWidth;
    /**
     * @brief Maximum output height size of the DUMP scaler in pixels.
     */
    STF_U16 u16MaxDumpHeight;

    /**
     * @brief Maximum output width size of the TILING_1_RD scaler in pixels.
     */
    STF_U16 u16MaxTiling_1_RdWidth;
    /**
     * @brief Maximum output height size of the TILING_1_RD scaler in pixels.
     */
    STF_U16 u16MaxTiling_1_RdHeight;

    /**
     * @brief Maximum output width size of the TILING_1_WR scaler in pixels.
     */
    STF_U16 u16MaxTiling_1_WrWidth;
    /**
     * @brief Maximum output height size of the TILING_1_WR scaler in pixels.
     */
    STF_U16 u16MaxTiling_1_WrHeight;

    /**
     * @brief status of the context (initialized, not initialized, ready,
     * etc.)
     *
     * @ define the states and have the object behave better (and a way
     * to clean erroneous state!)
     */
    EN_CTX_STATUS enCtxStatus;

    STF_BOOL8 bBufAllocated;
    ST_SHOT_INFO *pstAvailableShot;
    ST_SHOT_INFO *pstPendingShot;
    ST_SHOT_INFO *pstCompletedShot;
    ST_SHOT_INFO *pstOutputShot;
    ST_SHOT_INFO *pstRtspShot;
    ST_SHOT_INFO *pstTemporaryShot;
    ST_SHOT_TBL stImgShot;
    ST_SHOT_QUEUE stAvailableShotQueue;
    ST_SHOT_QUEUE stPendingShotQueue;
    ST_SHOT_QUEUE stCompletedShotQueue;
    ST_SHOT_QUEUE stOutputShotQueue;
    ST_SHOT_QUEUE stRtspShotQueue;
    ST_SHOT_QUEUE stTemporaryShotQueue;
    STF_BOOL8 bContiguousRawCaptureMode;
#if defined(ENABLE_CONTIGUOUS_CAPTURE)
    ST_RAW_DUMP_TBL stRawDump;
#endif //#if defined(ENABLE_CONTIGUOUS_CAPTURE)
#if defined(V4L2_DRIVER)
    ST_VDEV_PTR_TBL stPipelineVdoDevTbl;
    EN_ISP_PORT enIspPort;
#endif //#if defined(V4L2_DRIVER)

    STF_S8 s8PipelineModCnt;
    ST_ISP_MOD *pstPipelineMod[EN_MODULE_ID_MAX + 1];
    STF_S8 s8PipelineCtlCnt;
    ST_ISP_CTL *pstPipelineCtl[EN_CONTROL_ID_MAX + 1];

    STF_U64 u64FrameCount;

    //-------------------------------------------------------------------------
    /**
     * @brief Access to CI Connection to retrieve information about
     * HW and driver connection
     */

    CI_CONNECTION *(*GetConnection)(ST_PIPELINE *pstPipeline);

#if !defined(V4L2_DRIVER)
    /**
     * @brief Access to CI Pipeline (const) to retrieve converted information
     * from MC pipeline
     */
    CI_PIPELINE *(*GetCIPipeline)(ST_PIPELINE *pstPipeline);

#endif //#if !defined(V4L2_DRIVER)
    /**
     * @brief Get Global setup information from the configured modules
     *
     * @warning This function assumes the pMCPipeline is already populated
     * correctly for IIF!!!
     *
     * @param[out] ret (optional) return code
     */
    ST_GLOBAL_SETUP (*GetGlobalSetup)(ST_PIPELINE *pstPipeline, STF_RESULT *pRet);

    /**
     * @brief Get the camera sensor object.
     */
    ST_SENSOR *(*GetSensor)(ST_PIPELINE *pstPipeline);

    /**
     * @brief Set the camera sensor object.
     */
    STF_RESULT (*SetSensor)(ST_PIPELINE *pstPipeline, ST_SENSOR *pstSensor);

    /**
     * @brief Get the ISP context.
     */
    ST_ISP_CTX *(*GetIspContext)(ST_PIPELINE *pstPipeline);

    //-------------------------------------------------------------------------
    STF_RESULT (*BufAndShotInit)(ST_PIPELINE *pstPipeline);

#if defined(V4L2_DRIVER)
    STF_RESULT (*BufAllocate)(ST_PIPELINE *pstPipeline, STF_U8 u8BufCnt, STF_U8 u8BufType, STF_BOOL8 bClear, STF_BOOL8 bEnqueueBuf, ST_DMA_BUF_INFO *pstDmaBufInfo);
#else
    STF_RESULT (*BufAllocate)(ST_PIPELINE *pstPipeline, STF_U8 u8BufCnt, STF_U8 u8BufType, STF_BOOL8 bClear);
#endif //#if defined(V4L2_DRIVER)

    STF_RESULT (*BufFree)(ST_PIPELINE *pstPipeline);

#if defined(V4L2_DRIVER)
    STF_RESULT (*SetPipelineReady)(ST_PIPELINE *pstPipeline);

    STF_RESULT (*BufGetDirectly)(ST_PIPELINE *pstPipeline, STF_U8 u8Idx, EN_PIPELINE_BUF_ID enBufId, CI_MEM_PARAM **ppstMemParam);

    STF_RESULT (*ShotGetDirectly)(ST_PIPELINE *pstPipeline, STF_U8 u8Idx, ST_SHOT_INFO **ppstShotInfo);

    STF_U32 (*ShotGetAllocatedCount)(ST_PIPELINE *pstPipeline);

    STF_RESULT (*ShotAssignToCompleted)(ST_PIPELINE *pstPipeline, ST_SHOT_INFO *pstShotInfo);

    STF_RESULT (*ShotSwitchToCompletedFromVDev)(ST_PIPELINE *pstPipeline, ST_VDO_MEM **ppstUoVdoMem);

    STF_RESULT (*ShotDequeueBufToCompletedFromVDev)(ST_PIPELINE *pstPipeline, ST_VDO_MEM **ppstUoVdoMem);

    STF_RESULT (*ShotDequeueBufFromVDev)(ST_PIPELINE *pstPipeline, ST_SHOT_INFO *pstShotInfo, ST_VDO_MEM **ppstUoVdoMem);

    STF_RESULT (*ShotEnqueueBufToVDev)(ST_PIPELINE *pstPipeline, ST_SHOT_INFO *pstShotInfo);

#endif //#if defined(V4L2_DRIVER)
    STF_RESULT (*ShotPush)(ST_PIPELINE *pstPipeline, ST_SHOT_QUEUE *pstShotQueue, ST_SHOT_INFO *pstShotInfo);

    STF_RESULT (*ShotPop)(ST_PIPELINE *pstPipeline, ST_SHOT_QUEUE *pstShotQueue, ST_SHOT_INFO **ppstShotInfo);

    STF_RESULT (*ShotPopLast)(ST_PIPELINE *pstPipeline, ST_SHOT_QUEUE *pstShotQueue, ST_SHOT_INFO **ppstShotInfo);

    STF_RESULT (*ShotSwitchToAvailable)(ST_PIPELINE *pstPipeline);

    STF_RESULT (*ShotSwitchToAvailableSkipBackupShot)(ST_PIPELINE *pstPipeline);

    STF_RESULT (*ShotSwitchToPending)(ST_PIPELINE *pstPipeline);

    STF_RESULT (*ShotSwitchToCompleted)(ST_PIPELINE *pstPipeline);

    STF_RESULT (*ShotSwitchToOutput)(ST_PIPELINE *pstPipeline);

    STF_RESULT (*ShotSwitchToRtsp)(ST_PIPELINE *pstPipeline);

    STF_RESULT (*ShotSwitchRtspToAvailable)(ST_PIPELINE *pstPipeline);

    STF_RESULT (*ShotSwitchToTemporary)(ST_PIPELINE *pstPipeline);

    STF_RESULT (*ShotSwitchTemporaryToAvailable)(ST_PIPELINE *pstPipeline);

    STF_U32 (*ShotGetAvailableCount)(ST_PIPELINE *pstPipeline);

    STF_U32 (*ShotGetPendingCount)(ST_PIPELINE *pstPipeline);

    STF_U32 (*ShotGetCompletedCount)(ST_PIPELINE *pstPipeline);

    STF_U32 (*ShotGetOutputCount)(ST_PIPELINE *pstPipeline);

    STF_U32 (*ShotGetRtspCount)(ST_PIPELINE *pstPipeline);

    STF_U32 (*ShotGetTemporaryCount)(ST_PIPELINE *pstPipeline);

    STF_RESULT (*ShotGetShotInfo)(ST_PIPELINE *pstPipeline, ST_SHOT_INFO **ppstShotInfo);

    STF_RESULT (*ShotGetAvailableShot)(ST_PIPELINE *pstPipeline, ST_SHOT_INFO **ppstShotInfo);

    STF_RESULT (*ShotGetPendingShot)(ST_PIPELINE *pstPipeline, ST_SHOT_INFO **ppstShotInfo);

    STF_RESULT (*ShotGetCompletedShot)(ST_PIPELINE *pstPipeline, ST_SHOT_INFO **ppstShotInfo);

    STF_RESULT (*ShotGetOutputShot)(ST_PIPELINE *pstPipeline, ST_SHOT_INFO **ppstShotInfo);

    STF_RESULT (*ShotGetRtspShot)(ST_PIPELINE *pstPipeline, ST_SHOT_INFO **ppstShotInfo);

    STF_RESULT (*ShotGetTemporaryShot)(ST_PIPELINE *pstPipeline, ST_SHOT_INFO **ppstShotInfo);

    //=== Godspeed === Add new memory/buffer type support here.
    STF_RESULT (*ShotGetStatisticsBuf)(
        ST_PIPELINE *pstPipeline,
        EN_STAT_BUF_TYPE enStatBufType,
        STF_BOOL8 *pbIsScDumpForAe,
#if defined(V4L2_DRIVER)
        STF_U32 *pu32ScDumpCount,
#endif //#if defined(V4L2_DRIVER)
        STF_VOID **ppvBuffer
        );

    STF_RESULT (*ShotSetOutputAndStatisticsBufInfo)(ST_PIPELINE *pstPipeline, STF_BOOL8 bIsScDumpForAe);

    STF_RESULT (*ShotSetOutputAndStatisticsBuf)(ST_PIPELINE *pstPipeline, STF_BOOL8 bIsScDumpForAe);

    STF_BOOL8 (*ShotGetContiguousCaptureMode)(ST_PIPELINE *pstPipeline);

    STF_RESULT (*ShotSetContiguousCaptureMode)(ST_PIPELINE *pstPipeline, STF_BOOL8 bContiguousRawCaptureMode, STF_U16 u16BufferType, STF_U8 u8CaptureCount);

    STF_U32 (*ShotGetContiguousCaptureBufferCount)(ST_PIPELINE *pstPipeline);

    STF_RESULT (*ShotGetContiguousCaptureBuffer)(ST_PIPELINE *pstPipeline, STF_U8 u8Idx, CI_MEM_PARAM **ppstDumpMem);

    //-------------------------------------------------------------------------
#if defined(V4L2_DRIVER)
    /**
     * @brief initial the video devices.
     */
    STF_RESULT (*InitVideoDevice)(ST_PIPELINE *pstPipeline, EN_ISP_PORT enIspPort, EN_SNSR_IF enSensorInterface, STF_U32 u32PixelFormat);

    /**
     * @brief Get a video device from the ISP context.
     */
    ST_CI_DEVICE *(*GetVideoDevice)(ST_PIPELINE *pstPipeline, EN_ISP_PORT_ID enIspPortId);

    /**
     * @brief Get a video device from the ISP context.
     */
    ST_CI_DEVICE *(*GetVideoDevice2)(ST_PIPELINE *pstPipeline, CI_BUFFTYPE enBufferType);

    //-------------------------------------------------------------------------
#endif //#if defined(V4L2_DRIVER)
    /**
     * @brief Register a module in the pipeline.
     */
    STF_RESULT (*RegisterModule)(ST_PIPELINE *pstPipeline, ST_ISP_MOD *pstIspMod);

    /**
     * @brief Register a module in the pipeline.
     */
    STF_RESULT (*RegisterModuleById)(ST_PIPELINE *pstPipeline, EN_MODULE_ID enModuleId);

    /**
     * @brief Register all of enabled modules in the pipeline.
     */
    STF_RESULT (*RegisterEnabledModules)(ST_PIPELINE *pstPipeline);

    /**
     * @brief Register all of modules in the pipeline.
     */
    STF_RESULT (*RegisterAllOfModules)(ST_PIPELINE *pstPipeline);

    /**
     * @brief Unregister a module from the pipeline.
     */
    STF_RESULT (*UnregisterModule)(ST_PIPELINE *pstPipeline, EN_MODULE_ID enModuleId);

    /**
     * @brief Unregister all of disabled modules from the pipeline.
     */
    STF_RESULT (*UnregisterDisabledModules)(ST_PIPELINE *pstPipeline);

    /**
     * @brief Unregister all of disabled modules from the pipeline.
     */
    STF_RESULT (*UnregisterAllOfModules)(ST_PIPELINE *pstPipeline);

    /**
     * @brief Get a module form the ISP context.
     */
    ST_ISP_MOD *(*GetModule)(ST_PIPELINE *pstPipeline, EN_MODULE_ID enModuleId);

    /**
     * @brief Retrieve a registered module from the pipeline.
     */
    ST_ISP_MOD *(*GetRegisteredModule)(ST_PIPELINE *pstPipeline, EN_MODULE_ID enModuleId);

    /**
     * @brief Get the module's name form the ISP context.
     */
    STF_CHAR *(*GetModuleName)(ST_PIPELINE *pstPipeline, EN_MODULE_ID enModuleId);

    /**
     * @brief Retrieve a registered module's name from the pipeline.
     */
    STF_CHAR *(*GetRegisteredModuleName)(ST_PIPELINE *pstPipeline, EN_MODULE_ID enModuleId);

    /**
     * @brief Get the module's Rdma form the ISP context.
     */
    STF_VOID *(*GetModuleRdmaBuf)(ST_PIPELINE *pstPipeline, EN_MODULE_ID enModuleId);

    /**
     * @brief Retrieve a registered module's Rdma from the pipeline.
     */
    STF_VOID *(*GetRegisteredModuleRdmaBuf)(ST_PIPELINE *pstPipeline, EN_MODULE_ID enModuleId);

    /**
     * @brief Get the module's IspRdma form the ISP context.
     */
    STF_VOID *(*GetModuleIspRdma)(ST_PIPELINE *pstPipeline, EN_MODULE_ID enModuleId);

    /**
     * @brief Retrieve a registered module's IspRdma from the pipeline.
     */
    STF_VOID *(*GetRegisteredModuleIspRdma)(ST_PIPELINE *pstPipeline, EN_MODULE_ID enModuleId);

    /**
     * @brief Get the module's Rdma form the ISP context.
     */
    STF_VOID *(*GetModuleRdma)(ST_PIPELINE *pstPipeline, EN_MODULE_ID enModuleId);

    /**
     * @brief Retrieve a registered module's Rdma from the pipeline.
     */
    STF_VOID *(*GetRegisteredModuleRdma)(ST_PIPELINE *pstPipeline, EN_MODULE_ID enModuleId);

    /**
     * @brief Get the module's Param form the ISP context.
     */
    STF_VOID *(*GetModuleParam)(ST_PIPELINE *pstPipeline, EN_MODULE_ID enModuleId);

    /**
     * @brief Retrieve a registered module's Param from the pipeline.
     */
    STF_VOID *(*GetRegisteredModuleParam)(ST_PIPELINE *pstPipeline, EN_MODULE_ID enModuleId);

    //-------------------------------------------------------------------------
    /**
     * @brief Register a control in the pipeline.
     */
    STF_RESULT (*RegisterControl)(ST_PIPELINE *pstPipeline, ST_ISP_CTL *pstIspCtl);

    /**
     * @brief Register a control in the pipeline.
     */
    STF_RESULT (*RegisterControlById)(ST_PIPELINE *pstPipeline, EN_CONTROL_ID enControlId);

    /**
     * @brief Register all of enabled controls in the pipeline.
     */
    STF_RESULT (*RegisterEnabledControls)(ST_PIPELINE *pstPipeline);

    /**
     * @brief Register all of controls in the pipeline.
     */
    STF_RESULT (*RegisterAllOfControls)(ST_PIPELINE *pstPipeline);

    /**
     * @brief Unregister a control from the pipeline.
     */
    STF_RESULT (*UnregisterControl)(ST_PIPELINE *pstPipeline, EN_CONTROL_ID enControlId);

    /**
     * @brief Unregister all of disabled controls from the pipeline.
     */
    STF_RESULT (*UnregisterDisabledControls)(ST_PIPELINE *pstPipeline);

    /**
     * @brief Unregister all of disabled controls from the pipeline.
     */
    STF_RESULT (*UnregisterAllOfControls)(ST_PIPELINE *pstPipeline);

    /**
     * @brief Get a control form the ISP context.
     */
    ST_ISP_CTL *(*GetControl)(ST_PIPELINE *pstPipeline, EN_CONTROL_ID enControlId);

    /**
     * @brief Retrieve a registered control from the pipeline.
     */
    ST_ISP_CTL *(*GetRegisteredControl)(ST_PIPELINE *pstPipeline, EN_CONTROL_ID enControlId);

    /**
     * @brief Get the control's name form the ISP context.
     */
    STF_CHAR *(*GetControlName)(ST_PIPELINE *pstPipeline, EN_CONTROL_ID enControlId);

    /**
     * @brief Retrieve a registered control's name from the pipeline.
     */
    STF_CHAR *(*GetRegisteredControlName)(ST_PIPELINE *pstPipeline, EN_CONTROL_ID enControlId);

    /**
     * @brief Get the control's Param form the ISP context.
     */
    STF_VOID *(*GetControlParam)(ST_PIPELINE *pstPipeline, EN_CONTROL_ID enControlId);

    /**
     * @brief Retrieve a registered control's Param from the pipeline.
     */
    STF_VOID *(*GetRegisteredControlParam)(ST_PIPELINE *pstPipeline, EN_CONTROL_ID enControlId);

    //-------------------------------------------------------------------------
    /**
     * @brief Register all of enabled modules and controls in the pipeline.
     */
    STF_RESULT (*RegisterEnabledModulesAndControls)(ST_PIPELINE *pstPipeline);

    /**
     * @brief Register all of modules and controls in the pipeline.
     */
    STF_RESULT (*RegisterAllOfModulesAndControls)(ST_PIPELINE *pstPipeline);

    /**
     * @brief Unregister all of disabled modules and controls from the pipeline.
     */
    STF_RESULT (*UnregisterDisabledModulesAndControls)(ST_PIPELINE *pstPipeline);

    /**
     * @brief Unregister all of modules and controls from the pipeline.
     */
    STF_RESULT (*UnregisterAllOfModulesAndControls)(ST_PIPELINE *pstPipeline);

    /**
     * @brief Is the module enable from the pipeline.
     */
    STF_BOOL8 (*IsModuleEnable)(ST_PIPELINE *pstPipeline, EN_MODULE_ID enModuleId);

    /**
     * @brief Enable a module from the pipeline.
     */
    STF_RESULT (*ModuleEnable)(ST_PIPELINE *pstPipeline, EN_MODULE_ID enModuleId, STF_BOOL8 bEnable);

    /**
     * @brief Is the module update from the pipeline.
     */
    STF_BOOL8 (*IsModuleUpdate)(ST_PIPELINE *pstPipeline, EN_MODULE_ID enModuleId);

    /**
     * @brief Enable a module update feature from the pipeline.
     */
    STF_RESULT (*ModuleUpdate)(ST_PIPELINE *pstPipeline, EN_MODULE_ID enModuleId, STF_BOOL8 bUpdate);

    /**
     * @brief Is the control enable from the pipeline.
     */
    STF_BOOL8 (*IsControlEnable)(ST_PIPELINE *pstPipeline, EN_CONTROL_ID enControlId);

    /**
     * @brief Enable a control from the pipeline.
     */
    STF_RESULT (*ControlEnable)(ST_PIPELINE *pstPipeline, EN_CONTROL_ID enControlId, STF_BOOL8 bEnable);

    /**
     * @brief Removes and deletes all the modules and controls registered in the pipeline.
     *
     * @warning Each module is actually destroyed so it must not be destroyed
     * outside this function.
     */
    STF_RESULT (*ClearModulesAndControls)(ST_PIPELINE *pstPipeline);

    /**
     * @brief Print all the modules and controls name that registered in the pipeline.
     *
     * @warning Each module is actually destroyed so it must not be destroyed
     * outside this function.
     */
    STF_RESULT (*PrintRegisteredModulesAndControls)(ST_PIPELINE *pstPipeline);
    //-------------------------------------------------------------------------
#ifdef UNUSED_CODE_AND_VARIABLE

    /**
     * @brief Reload all the registered modules configuration from the
     * received list of parameters
     */
    STF_RESULT ReloadAll(const ParameterList &parameters);

    STF_RESULT ReloadAllModules(const ParameterList &parameters);

    STF_RESULT ReloadAllGlobals(const ParameterList &parameters);

    /**
     * @brief Reload the specified module configuration from the received
     * list of parameters
     */
    STF_RESULT ReloadModule(SetupID id, const ParameterList &parameters);

    /**
     * @brief Save the current modules configuration to a list of parameters
     */
    STF_RESULT SaveAll(ParameterList &paremeters,
        ModuleBase::SaveType t = ModuleBase::SAVE_VAL) const;

    /**
     * @brief Save a specific module to a list of parameters
     */
    STF_RESULT SaveModule(SetupID id, ParameterList &parameters,
        ModuleBase::SaveType t = ModuleBase::SAVE_VAL) const;
#endif //UNUSED_CODE_AND_VARIABLE

    /**
     * @name Modules setup
     * @brief Access to the registerd Modules
     * @{
     */

    //-------------------------------------------------------------------------
    /**
     * @brief Initialize all of modules and controls form the ISP context.
     */
    STF_RESULT (*InitAll)(ST_PIPELINE *pstPipeline);

    /**
     * @brief Initialize all of global modules form the ISP context.
     */
    STF_RESULT (*InitAllGlobals)(ST_PIPELINE *pstPipeline);

    /**
     * @brief Initialize all of modules form the ISP context.
     */
    STF_RESULT (*InitAllModules)(ST_PIPELINE *pstPipeline);

    /**
     * @brief Initialize all of controls form the ISP context.
     */
    STF_RESULT (*InitAllControls)(ST_PIPELINE *pstPipeline);

    /**
     * @brief Initialize all modules and controls form the pipeline.
     */
    STF_RESULT (*InitPipelineModulesAndControls)(ST_PIPELINE *pstPipeline);

    /**
     * @brief Initialize all modules's next Rdma form the pipeline.
     */
    STF_RESULT (*SetNextRdma)(ST_PIPELINE *pstPipeline);

    /**
     * @brief Update all the modules in the ISP context including globals.
     */
    STF_RESULT (*UpdateAll)(ST_PIPELINE *pstPipeline);

    /**
     * @brief Update only the global modules in the ISP context.
     */
    STF_RESULT (*UpdateAllGlobals)(ST_PIPELINE *pstPipeline);

    /**
     * @brief Update all modules but the global ones in the ISP context.
     */
    STF_RESULT (*UpdateAllModules)(ST_PIPELINE *pstPipeline);

    /**
     * @brief Calculate all controls in the ISP context.
     */
    STF_RESULT (*CalculateAllControls)(ST_PIPELINE *pstPipeline);

    /**
     * @brief Update a module's Param to Rdma in the ISP context.
     */
    STF_RESULT (*UpdateModule)(ST_PIPELINE *pstPipeline, EN_MODULE_ID enModuleId);

    /**
     * @brief Calculate a control's Param to module's Param in the ISP context.
     */
    STF_RESULT (*CalculateControl)(ST_PIPELINE *pstPipeline, EN_CONTROL_ID enControlId);

    /**
     * @brief Update all the modules's and controls's Param to Rdma
     * with the requested flag activated from the pipeline.
     */
    STF_RESULT (*UpdateAllRequested)(ST_PIPELINE *pstPipeline);

    /**
     * @brief Update all the modules's Param to Rdma
     * with the requested flag activated from the pipeline.
     */
    STF_RESULT (*UpdateAllRequestedModules)(ST_PIPELINE *pstPipeline);

    /**
     * @brief Calculate all the controls Param to module's Param
     * with the requested flag activated from the pipeline.
     */
    STF_RESULT (*CalculateAllRequestedControls)(ST_PIPELINE *pstPipeline);

    /**
     * @brief Setup all the modules in the ISP context.
     */
    STF_RESULT (*SetupAll)(ST_PIPELINE *pstPipeline);

    /**
     * @brief Setup only the global modules in the ISP context.
     */
    STF_RESULT (*SetupAllGlobals)(ST_PIPELINE *pstPipeline);

    /**
     * @brief Setup all modules but the global ones in the ISP context.
     */
    STF_RESULT (*SetupAllModules)(ST_PIPELINE *pstPipeline);

    /**
     * @brief Setup a module's in the ISP context.
     */
    STF_RESULT (*SetupModule)(ST_PIPELINE *pstPipeline, EN_MODULE_ID enModuleId);

    /**
     * @brief Setup all the modules's
     * with the requested flag activated from the pipeline.
     */
    STF_RESULT (*SetupRequested)(ST_PIPELINE *pstPipeline);

    /**
     * @brief Get a module parameters.
     * with the requested flag activated from the pipeline.
     */
    STF_RESULT (*ModuleGetIqParam)(ST_PIPELINE *pstPipeline, EN_MODULE_ID enModuleId, STF_VOID *pParamBuf, STF_U16 *pu16ParamSize);

    /**
     * @brief Set a module parameters.
     * with the requested flag activated from the pipeline.
     */
    STF_RESULT (*ModuleSetIqParam)(ST_PIPELINE *pstPipeline, EN_MODULE_ID enModuleId, STF_VOID *pParamBuf, STF_U16 u16ParamSize);

    /**
     * @brief Get a control parameters.
     * with the requested flag activated from the pipeline.
     */
    STF_RESULT (*ControlGetIqParam)(ST_PIPELINE *pstPipeline, EN_CONTROL_ID enControlId, STF_VOID *pParamBuf, STF_U16 *pu16ParamSize);

    /**
     * @brief Set a control parameters.
     * with the requested flag activated from the pipeline.
     */
    STF_RESULT (*ControlSetIqParam)(ST_PIPELINE *pstPipeline, EN_CONTROL_ID enControlId, STF_VOID *pParamBuf, STF_U16 u16ParamSize);

    /**
     * @brief Process binary parameters file.
     */
    STF_RESULT (*LoadBinParam)(ST_PIPELINE *pstPipeline, STF_CHAR *pszBinSettingFilename);

    /**
     * @brief Loop each modules to process binary calibration file if needed.
     */
    STF_RESULT (*LoadBinClbrt)(ST_PIPELINE *pstPipeline);


    //-------------------------------------------------------------------------
    /**
     * @brief Verifies if the configuration is correct for the current HW
     * version
     */
    STF_RESULT (*VerifyConfiguration)(ST_PIPELINE *pstPipeline);

    /**
     * @brief Update CI_PIPELINE information.
     */
    STF_RESULT (*UpdateCiPipeline)(ST_PIPELINE *pstPipeline);

    /**
     * @brief Program current configuration in the pipeline.
     */
    STF_RESULT (*ProgramPipeline)(ST_PIPELINE *pstPipeline, STF_BOOL8 bUpdateASAP);

    /**
     * @brief Adds a number of Shots to the Pipeline
     */
    STF_RESULT (*AddShots)(ST_PIPELINE *pstPipeline, STF_U8 u8Num);

    /**
     * @brief Delete all allocated Shots of the Pipeline - buffers are
     * unnacfected
     */
    STF_RESULT (*DeleteShots)(ST_PIPELINE *pstPipeline);

    /**
     * @brief Retrieve a captured shot from the pipeline. The Shot must be
     * released later
     */
    STF_RESULT (*AcquireShot)(ST_PIPELINE *pstPipeline, ST_SHOT *pstShot, STF_BOOL8 bBlocking);

    /**
     * @brief This function programs shot with provided buffer id.
     *
     * Once the buffers are allocated and the HW reserveed (with the call to
     * startCapture) we are able to program shots to be captured with this
     * function.
     *
     * Buffers should already be imported or allocated.
     */
    STF_RESULT (*ProgramSpecifiedShot)(ST_PIPELINE *pstPipeline, CI_BUFFID *pstBuffId);

    /**
     * @brief Program a shot to be captured in the pipeline (takes 1st
     * available Shot and 1st available Buffers)
     *
     * Once the buffers are allocated and the HW reserveed (with the call
     * to startCapture) we are able to program shots to be captured with this
     * function.
     */
    STF_RESULT (*ProgramShot)(ST_PIPELINE *pstPipeline);

    /**
     * @brief Configure a captured shot from the pipeline.
     */
    STF_VOID (*ProcessShot)(ST_PIPELINE *pstPipeline, ST_SHOT *pstShot, CI_SHOT *pstCIShotBuffer);

    /**
     * @brief Release a previously captured and retrieved shot.
     */
    STF_RESULT (*ReleaseShot)(ST_PIPELINE *pstPipeline, ST_SHOT *pstShot);

    /**
     * @brief Allocate 1 buffer for the selected output (pipeline must be
     * preconfigured)
     */
    STF_RESULT (*AllocateBuffer)(ST_PIPELINE *pstPipeline, CI_BUFFTYPE enBufferType, STF_U32 u32Size, STF_U32 *pu32BufferId);

    /**
     * @brief This function imports a buffer
     */
    STF_RESULT (*ImportBuffer)(ST_PIPELINE *pstPipeline, CI_BUFFTYPE enBufferType, STF_U32 u32IonFd, STF_U32 u32Size, STF_U32 *pu32BufferId);

    /**
     * @brief Deregisters an allocated or imported buffer (The pipeline must
     * be stopped)
     */
    STF_RESULT (*DeregisterBuffer)(ST_PIPELINE *pstPipeline, STF_U32 u32BufferID);

    //=== Godspeed === Add new memory/buffer type support here.
    //-------------------------------------------------------------------------
    /**
     * @brief Get the Uo output maximum dimensions
     */
    ST_RES (*GetUoDimensions)(ST_PIPELINE *pstPipeline);

    /**
     * @brief Change the Uo output maximum dimensions
     */
    STF_RESULT (*SetUoDimensions)(ST_PIPELINE *pstPipeline, STF_U16 u16Width, STF_U16 u16Height);

    /**
     * @brief Get the Ss0 output maximum dimensions
     */
    ST_RES (*GetSs0Dimensions)(ST_PIPELINE *pstPipeline);

    /**
     * @brief Change the Ss0 output maximum dimensions
     */
    STF_RESULT (*SetSs0Dimensions)(ST_PIPELINE *pstPipeline, STF_U16 u16Width, STF_U16 u16Height);

    /**
     * @brief Get the Ss1 output maximum dimensions
     */
    ST_RES (*GetSs1Dimensions)(ST_PIPELINE *pstPipeline);

    /**
     * @brief Change the Ss1 output maximum dimensions
     */
    STF_RESULT (*SetSs1Dimensions)(ST_PIPELINE *pstPipeline, STF_U16 u16Width, STF_U16 u16Height);

    /**
     * @brief Get the Dump output maximum dimensions
     */
    ST_RES (*GetDumpDimensions)(ST_PIPELINE *pstPipeline);

    /**
     * @brief Change the Dump output maximum dimensions
     */
    STF_RESULT (*SetDumpDimensions)(ST_PIPELINE *pstPipeline, STF_U16 u16Width, STF_U16 u16Height);

    /**
     * @brief Get the Tiling 1 Read output maximum dimensions
     */
    ST_RES (*GetTiling_1_ReadDimensions)(ST_PIPELINE *pstPipeline);

    /**
     * @brief Change the Tiling 1 Read output maximum dimensions
     */
    STF_RESULT (*SetTiling_1_ReadDimensions)(ST_PIPELINE *pstPipeline, STF_U16 u16Width, STF_U16 u16Height);

    /**
     * @brief Get the Tiling 1 write output maximum dimensions
     */
    ST_RES (*GetTiling_1_WriteDimensions)(ST_PIPELINE *pstPipeline);

    /**
     * @brief Change the Tiling 1 write output maximum dimensions
     */
    STF_RESULT (*SetTiling_1_WriteDimensions)(ST_PIPELINE *pstPipeline, STF_U16 u16Width, STF_U16 u16Height);

    //-------------------------------------------------------------------------
    /**
     * @brief Reserve the hardware for capture.
     */
    STF_RESULT (*StartCapture)(ST_PIPELINE *pstPipeline);

    /**
     * @brief Stop the capture process releasing the HW.
     */
    STF_RESULT (*StopCapture)(ST_PIPELINE *pstPipeline);

    /**
     * @brief Get the identifier of the 1st available buffers to run with the
     * current configuration.
     */
    STF_RESULT (*GetFirstAvailableBuffers)(ST_PIPELINE *pstPipeline, CI_BUFFID *pstBuffId);

    //-------------------------------------------------------------------------
    /**
     * @brief register the pipeline configuration into kernel driver.
     */
    STF_RESULT (*Register)(ST_PIPELINE *pstPipeline);

    /**
     * @brief set the active capture to kernel driver.
     */
    STF_RESULT (*SetActiveCapture)(ST_PIPELINE *pstPipeline);

    /**
     * @brief clear the active capture to kernel driver.
     */
    STF_RESULT (*ClearActiveCapture)(ST_PIPELINE *pstPipeline);

    /**
     * @brief Reset the ISP total frame count.
     */
    STF_RESULT (*ResetFrameCount)(ST_PIPELINE *pstPipeline);

    /**
     * @brief Get the ISP total frame count.
     */
    STF_U64 (*GetFrameCount)(ST_PIPELINE *pstPipeline);

    /**
     * @brief Set the ISP pending count.
     */
    STF_RESULT (*SetPendingCount)(ST_PIPELINE *pstPipeline, STF_U32 u32PendingCount);

    /**
     * @brief Increase the ISP pending count.
     */
    STF_U32 (*IncPendingCount)(ST_PIPELINE *pstPipeline);

    /**
     * @brief Get the ISP completed count.
     */
    STF_U32 (*GetCompletedCount)(ST_PIPELINE *pstPipeline, STF_BOOL8 bBlocking);

    /**
     * @brief Set the ISP completed count.
     */
    STF_RESULT (*SetCompletedCount)(ST_PIPELINE *pstPipeline, STF_U32 u32CompletedCount);

    /**
     * @brief Get the ISP pending and completed count.
     */
    STF_RESULT (*GetPendingCompletedCount)(ST_PIPELINE *pstPipeline, STF_BOOL8 bBlocking, STF_BOOL8 *pbHwCapture, STF_U32 *pu32PendingCount, STF_U32 *pu32CompletedCount);

    //-------------------------------------------------------------------------

    /**
     * @}
     */

} ST_PIPELINE, *PST_PIPELINE;

#pragma pack(pop)


////extern ST_ISP_CTX g_stIspCtx;
//extern STF_BOOL8 g_bBufferInit;


// Flow control APIs

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function
//extern STF_S32 STFLIB_ISP_PipelineBufInit(STF_VOID *pIspCtx);
//extern STF_S32 STFLIB_ISP_PipelineBufAllocate(STF_VOID *pIspCtx, STF_U8 u8BufCnt, STF_U8 u8BufType, STF_BOOL8 bClear, STF_BOOL8 bEnqueueBuf);
//extern STF_S32 STFLIB_ISP_PipelineBufSetImageInfo(STF_VOID *pIspCtx, STF_U8 u8BufType, STF_U8 u8Idx);
//extern STF_S32 STFLIB_ISP_PipelineBufSetImageInfoToAll(STF_VOID *pIspCtx, STF_U8 u8BufType);
//extern STF_S32 STFLIB_ISP_PipelineBufPush(STF_VOID *pIspCtx, ST_BUF_QUEUE *pstBufQueue, ST_BUF_INFO *pstBufInfo);
//extern STF_S32 STFLIB_ISP_PipelineBufPop(STF_VOID *pIspCtx, ST_BUF_QUEUE *pstBufQueue, ST_BUF_INFO **pstBufInfo);
//extern STF_S32 STFLIB_ISP_PipelineBufFree(STF_VOID *pIspCtx);
//extern STF_S32 STFLIB_ISP_PipelineBufSwitchToAvailable(STF_VOID *pIspCtx);
//extern STF_S32 STFLIB_ISP_PipelineBufSwitchToAvailableSkipBackupBuffer(STF_VOID *pIspCtx);
//extern STF_S32 STFLIB_ISP_PipelineBufSwitchToPending(STF_VOID *pIspCtx);
//extern STF_S32 STFLIB_ISP_PipelineBufSwitchToComplete(STF_VOID *pIspCtx);
//#if defined(PIPELINE_OUTPUT_QUEUE_ENABLE)
//extern STF_S32 STFLIB_ISP_PipelineBufSwitchToOutput(STF_VOID *pIspCtx);
//#endif //PIPELINE_OUTPUT_QUEUE_ENABLE
//extern STF_S32 STFLIB_ISP_PipelineBufSetOutputAndStatisticsBuf(STF_VOID *pIspCtx);

//extern STF_S32 STFLIB_ISP_PipelineGetImageSize(STF_VOID *pIspCtx, ST_SIZE *pstImgSize);
//extern STF_S32 STFLIB_ISP_PipelineSetImageSize(STF_VOID *pIspCtx, ST_SIZE stImgSize);

//extern STF_S32 STFLIB_ISP_PipelineLoadBinParam(STF_VOID *pIspCtx, STF_CHAR *pszBinSettingFilename);
//extern STF_S32 STFLIB_ISP_PipelineLoadBinClbrt(STF_VOID *pIspCtx);
//extern STF_S32 STFLIB_ISP_PipelineShadowEnable(STF_VOID *pIspCtx);
//extern STF_S32 STFLIB_ISP_PipelineLoopAllOfModulesControls(STF_VOID *pIspCtx);
//extern STF_S32 STFLIB_ISP_PipelineLoop(STF_VOID *pIspCtx);
//extern STF_S32 STFLIB_ISP_PipelineLoopModules(STF_VOID *pIspCtx);
//extern STF_S32 STFLIB_ISP_PipelineLoopControls(STF_VOID *pIspCtx);
//extern STF_S32 STFLIB_ISP_PipelineSetRegAllOfModules(STF_VOID *pIspCtx);
//extern STF_S32 STFLIB_ISP_PipelineSetReg(STF_VOID *pIspCtx);
//extern STF_S32 STFLIB_ISP_PipelineFree(STF_VOID *pIspCtx);

//=============================================================================
#if defined(V4L2_DRIVER)
extern
STF_VOID *STFLIB_ISP_GetVideoDevice(
    STF_VOID *pIspCtx,
    EN_ISP_PORT_ID enIspPortId
    );

extern
STF_VOID *STFLIB_ISP_GetVideoDevice2(
    STF_VOID *pIspCtx,
    CI_BUFFTYPE enBufferType
    );

//=============================================================================
#endif //#if defined(V4L2_DRIVER)
extern
STF_VOID *STFLIB_ISP_GetModule(
    STF_VOID *pIspCtx,
    EN_MODULE_ID enModuleId
    );
extern
STF_VOID *STFLIB_ISP_GetModuleName(
    STF_VOID *pIspCtx,
    EN_MODULE_ID enModuleId
    );
extern
STF_VOID *STFLIB_ISP_GetModuleRdmaBuf(
    STF_VOID *pIspCtx,
    EN_MODULE_ID enModuleId
    );
extern
STF_VOID *STFLIB_ISP_GetModuleIspRdma(
    STF_VOID *pIspCtx,
    EN_MODULE_ID enModuleId
    );
extern
STF_VOID *STFLIB_ISP_GetModuleRdma(
    STF_VOID *pIspCtx,
    EN_MODULE_ID enModuleId
    );
extern
STF_VOID *STFLIB_ISP_GetModuleParam(
    STF_VOID *pIspCtx,
    EN_MODULE_ID enModuleId
    );
extern
STF_VOID *STFLIB_ISP_GetControl(
    STF_VOID *pIspCtx,
    EN_CONTROL_ID enControlId
    );
extern
STF_VOID *STFLIB_ISP_GetControlName(
    STF_VOID *pIspCtx,
    EN_CONTROL_ID enControlId
    );
extern
STF_VOID *STFLIB_ISP_GetControlParam(
    STF_VOID *pIspCtx,
    EN_CONTROL_ID enControlId
    );

extern
STF_S32 STFLIB_ISP_IqTuningDebugInfoEnable(
    STF_VOID *pIspCtx,
    STF_BOOL8 bEnable
    );
extern
STF_BOOL8 STFLIB_ISP_IsIqTuningDebugInfoEnable(
    STF_VOID *pIspCtx
    );

//=============================================================================
extern
CI_CONNECTION *STFLIB_ISP_PIPELINE_GetConnection(
    ST_PIPELINE *pstPipeline
    );

#if !defined(V4L2_DRIVER)
extern
CI_PIPELINE *STFLIB_ISP_PIPELINE_GetCIPipeline(
    ST_PIPELINE *pstPipeline
    );

#endif //#if !defined(V4L2_DRIVER)
extern
ST_GLOBAL_SETUP STFLIB_ISP_PIPELINE_GetGlobalSetup(
    ST_PIPELINE *pstPipeline,
    STF_RESULT *pRet
    );

extern
ST_SENSOR *STFLIB_ISP_PIPELINE_GetSensor(
    ST_PIPELINE *pstPipeline
    );

extern
STF_RESULT STFLIB_ISP_PIPELINE_SetSensor(
    ST_PIPELINE *pstPipeline,
    ST_SENSOR *pstSensor
    );

extern
ST_ISP_CTX *STFLIB_ISP_PIPELINE_GetIspContext(
    ST_PIPELINE *pPipeline
    );

//-----------------------------------------------------------------------------
extern
STF_RESULT STFLIB_ISP_PIPELINE_BufAndShotInit(
    ST_PIPELINE *pstPipeline
    );

extern
STF_RESULT STFLIB_ISP_PIPELINE_BufAllocate(
    ST_PIPELINE *pstPipeline,
    STF_U8 u8BufCnt,
    STF_U8 u8BufType,
#if defined(V4L2_DRIVER)
    STF_BOOL8 bClear,
    STF_BOOL8 bEnqueueBuf,
    ST_DMA_BUF_INFO *pstDmaBufInfo
#else
    STF_BOOL8 bClear
#endif //#if defined(V4L2_DRIVER)
    );

extern
STF_RESULT STFLIB_ISP_PIPELINE_BufFree(
    ST_PIPELINE *pstPipeline
    );

#if defined(V4L2_DRIVER)
extern
STF_RESULT STFLIB_ISP_PIPELINE_SetPipelineReady(
    ST_PIPELINE *pstPipeline
    );

extern
STF_RESULT STFLIB_ISP_PIPELINE_BufGetDirectly(
    ST_PIPELINE *pstPipeline,
    STF_U8 u8Idx,
    EN_PIPELINE_BUF_ID enBufId,
    CI_MEM_PARAM **ppstMemParam
    );

extern
STF_RESULT STFLIB_ISP_PIPELINE_ShotGetDirectly(
    ST_PIPELINE *pstPipeline,
    STF_U8 u8Idx,
    ST_SHOT_INFO **ppstShotInfo
    );

extern
STF_U32 STFLIB_ISP_PIPELINE_ShotGetAllocatedCount(
    ST_PIPELINE *pstPipeline
    );

extern
STF_RESULT STFLIB_ISP_PIPELINE_ShotAssignToCompleted(
    ST_PIPELINE *pstPipeline,
    ST_SHOT_INFO *pstShotInfo
    );

extern
STF_RESULT STFLIB_ISP_PIPELINE_ShotSwitchToCompletedFromVDev(
    ST_PIPELINE *pstPipeline,
    ST_VDO_MEM **ppstUoVdoMem
    );

extern
STF_RESULT STFLIB_ISP_PIPELINE_ShotDequeueBufToCompletedFromVDev(
    ST_PIPELINE *pstPipeline,
    ST_VDO_MEM **ppstUoVdoMem
    );

extern
STF_RESULT STFLIB_ISP_PIPELINE_ShotDequeueBufFromVDev(
    ST_PIPELINE *pstPipeline,
    ST_SHOT_INFO *pstShotInfo,
    ST_VDO_MEM **ppstUoVdoMem
    );

extern
STF_RESULT STFLIB_ISP_PIPELINE_ShotEnqueueBufToVDev(
    ST_PIPELINE *pstPipeline,
    ST_SHOT_INFO *pstShotInfo
    );

#endif //#if defined(V4L2_DRIVER)
extern
STF_RESULT STFLIB_ISP_PIPELINE_ShotPush(
    ST_PIPELINE *pstPipeline,
    ST_SHOT_QUEUE *pstShotQueue,
    ST_SHOT_INFO *pstShotInfo
    );

extern
STF_RESULT STFLIB_ISP_PIPELINE_ShotPop(
    ST_PIPELINE *pstPipeline,
    ST_SHOT_QUEUE *pstShotQueue,
    ST_SHOT_INFO **ppstShotInfo
    );

extern
STF_RESULT STFLIB_ISP_PIPELINE_ShotPopLast(
    ST_PIPELINE *pstPipeline,
    ST_SHOT_QUEUE *pstShotQueue,
    ST_SHOT_INFO **ppstShotInfo
    );

extern
STF_RESULT STFLIB_ISP_PIPELINE_ShotSwitchToAvailable(
    ST_PIPELINE *pstPipeline
    );

extern
STF_RESULT STFLIB_ISP_PIPELINE_ShotSwitchToAvailableSkipBackupShot(
    ST_PIPELINE *pstPipeline
    );

extern
STF_RESULT STFLIB_ISP_PIPELINE_ShotSwitchToPending(
    ST_PIPELINE *pstPipeline
    );

extern
STF_RESULT STFLIB_ISP_PIPELINE_ShotSwitchToCompleted(
    ST_PIPELINE *pstPipeline
    );

extern
STF_RESULT STFLIB_ISP_PIPELINE_ShotSwitchToOutput(
    ST_PIPELINE *pstPipeline
    );

extern
STF_RESULT STFLIB_ISP_PIPELINE_ShotSwitchToRtsp(
    ST_PIPELINE *pstPipeline
    );

extern
STF_RESULT STFLIB_ISP_PIPELINE_ShotSwitchRtspToAvailable(
    ST_PIPELINE *pstPipeline
    );

extern
STF_RESULT STFLIB_ISP_PIPELINE_ShotSwitchToTemporary(
    ST_PIPELINE *pstPipeline
    );

extern
STF_RESULT STFLIB_ISP_PIPELINE_ShotSwitchTemporaryToAvailable(
    ST_PIPELINE *pstPipeline
    );

extern
STF_U32 STFLIB_ISP_PIPELINE_ShotGetAvailableCount(
    ST_PIPELINE *pstPipeline
    );

extern
STF_U32 STFLIB_ISP_PIPELINE_ShotGetPendingCount(
    ST_PIPELINE *pstPipeline
    );

extern
STF_U32 STFLIB_ISP_PIPELINE_ShotGetCompletedCount(
    ST_PIPELINE *pstPipeline
    );

extern
STF_U32 STFLIB_ISP_PIPELINE_ShotGetOutputCount(
    ST_PIPELINE *pstPipeline
    );

extern
STF_U32 STFLIB_ISP_PIPELINE_ShotGetRtspCount(
    ST_PIPELINE *pstPipeline
    );

extern
STF_U32 STFLIB_ISP_PIPELINE_ShotGetTemporaryCount(
    ST_PIPELINE *pstPipeline
    );

extern
STF_RESULT STFLIB_ISP_PIPELINE_ShotGetShotInfo(
    ST_PIPELINE *pstPipeline,
    ST_SHOT_INFO **ppstShotInfo
    );

extern
STF_RESULT STFLIB_ISP_PIPELINE_ShotGetAvailableShot(
    ST_PIPELINE *pstPipeline,
    ST_SHOT_INFO **ppstShotInfo
    );

extern
STF_RESULT STFLIB_ISP_PIPELINE_ShotGetPendingShot(
    ST_PIPELINE *pstPipeline,
    ST_SHOT_INFO **ppstShotInfo
    );

extern
STF_RESULT STFLIB_ISP_PIPELINE_ShotGetCompletedShot(
    ST_PIPELINE *pstPipeline,
    ST_SHOT_INFO **ppstShotInfo
    );

extern
STF_RESULT STFLIB_ISP_PIPELINE_ShotGetOutputShot(
    ST_PIPELINE *pstPipeline,
    ST_SHOT_INFO **ppstShotInfo
    );

extern
STF_RESULT STFLIB_ISP_PIPELINE_ShotGetRtspShot(
    ST_PIPELINE *pstPipeline,
    ST_SHOT_INFO **ppstShotInfo
    );

extern
STF_RESULT STFLIB_ISP_PIPELINE_ShotGetTemporaryShot(
    ST_PIPELINE *pstPipeline,
    ST_SHOT_INFO **ppstShotInfo
    );

//=== Godspeed === Add new memory/buffer type support here.
extern
STF_RESULT STFLIB_ISP_PIPELINE_ShotGetStatisticsBuf(
    ST_PIPELINE *pstPipeline,
    EN_STAT_BUF_TYPE enStatBufType,
    STF_BOOL8 *pbIsScDumpForAe,
#if defined(V4L2_DRIVER)
    STF_U32 *pu32ScDumpCount,
#endif //#if defined(V4L2_DRIVER)
    STF_VOID **ppvBuffer
    );

extern
STF_RESULT STFLIB_ISP_PIPELINE_ShotSetOutputAndStatisticsBufInfo(
    ST_PIPELINE *pstPipeline,
    STF_BOOL8 bIsScDumpForAe
    );

extern
STF_RESULT STFLIB_ISP_PIPELINE_ShotSetOutputAndStatisticsBuf(
    ST_PIPELINE *pstPipeline,
    STF_BOOL8 bIsScDumpForAe
    );

extern
STF_BOOL8 STFLIB_ISP_PIPELINE_ShotGetContiguousCaptureMode(
    ST_PIPELINE *pstPipeline
    );

extern
STF_RESULT STFLIB_ISP_PIPELINE_ShotSetContiguousCaptureMode(
    ST_PIPELINE *pstPipeline,
    STF_BOOL8 bContiguousRawCaptureMode,
    STF_U16 u16BufferType,
    STF_U8 u8CaptureCount
    );

extern
STF_U32 STFLIB_ISP_PIPELINE_ShotGetContiguousCaptureBufferCount(
    ST_PIPELINE *pstPipeline
    );

extern
STF_RESULT STFLIB_ISP_PIPELINE_ShotGetContiguousCaptureBuffer(
    ST_PIPELINE *pstPipeline,
    STF_U8 u8Idx,
    CI_MEM_PARAM **ppstDumpMem
    );

//-----------------------------------------------------------------------------
#if defined(V4L2_DRIVER)
extern
STF_RESULT STFLIB_ISP_FillMemoryInfo(
    CI_MEM_PARAM *pstMemParam,
    STF_U32 u32Width,
    STF_U32 u32Height,
    CI_BUFFTYPE enAllocBufferType,
    eFORMAT_TYPES enFormatType,
    ePxlFormat enPixelFormat,
    eMOSAIC enMosaic,
    STF_CHAR *szMemName,
    STF_U32 u32Index,
    STF_VOID *pvBuffer,
    STF_U32 u32Length
    );

extern
STF_RESULT STFLIB_ISP_PIPELINE_InitVideoDevice(
    ST_PIPELINE *pstPipeline,
    EN_ISP_PORT enIspPort,
    EN_SNSR_IF enSensorInterface,
    STF_U32 u32PixelFormat
    );

extern
ST_CI_DEVICE *STFLIB_ISP_PIPELINE_GetVideoDevice(
    ST_PIPELINE *pstPipeline,
    EN_ISP_PORT_ID enIspPortId
    );

extern
ST_CI_DEVICE *STFLIB_ISP_PIPELINE_GetVideoDevice2(
    ST_PIPELINE *pstPipeline,
    CI_BUFFTYPE enBufferType
    );

//-----------------------------------------------------------------------------
#endif //#if defined(V4L2_DRIVER)
extern
STF_RESULT STFLIB_ISP_PIPELINE_RegisterModule(
    ST_PIPELINE *pstPipeline,
    ST_ISP_MOD *pstIspMod
    );

extern
STF_RESULT STFLIB_ISP_PIPELINE_RegisterModuleById(
    ST_PIPELINE *pstPipeline,
    EN_MODULE_ID enModuleId
    );

extern
STF_RESULT STFLIB_ISP_PIPELINE_RegisterEnabledModules(
    ST_PIPELINE *pstPipeline
    );

extern
STF_RESULT STFLIB_ISP_PIPELINE_RegisterAllOfModules(
    ST_PIPELINE *pstPipeline
    );

extern
STF_RESULT STFLIB_ISP_PIPELINE_UnregisterModule(
    ST_PIPELINE *pstPipeline,
    EN_MODULE_ID enModuleId
    );

extern
STF_RESULT STFLIB_ISP_PIPELINE_UnregisterDisabledModules(
    ST_PIPELINE *pstPipeline
    );

extern
STF_RESULT STFLIB_ISP_PIPELINE_UnregisterAllOfModules(
    ST_PIPELINE *pstPipeline
    );

extern
ST_ISP_MOD *STFLIB_ISP_PIPELINE_GetModule(
    ST_PIPELINE *pstPipeline,
    EN_MODULE_ID enModuleId
    );

extern
ST_ISP_MOD *STFLIB_ISP_PIPELINE_GetRegisteredModule(
    ST_PIPELINE *pstPipeline,
    EN_MODULE_ID enModuleId
    );

extern
STF_CHAR *STFLIB_ISP_PIPELINE_GetModuleName(
    ST_PIPELINE *pstPipeline,
    EN_MODULE_ID enModuleId
    );

extern
STF_CHAR *STFLIB_ISP_PIPELINE_GetRegisteredModuleName(
    ST_PIPELINE *pstPipeline,
    EN_MODULE_ID enModuleId
    );

extern
STF_VOID *STFLIB_ISP_PIPELINE_GetModuleRdmaBuf(
    ST_PIPELINE *pstPipeline,
    EN_MODULE_ID enModuleId
    );

extern
STF_VOID *STFLIB_ISP_PIPELINE_GetRegisteredModuleRdmaBuf(
    ST_PIPELINE *pstPipeline,
    EN_MODULE_ID enModuleId
    );

extern
STF_VOID *STFLIB_ISP_PIPELINE_GetModuleIspRdma(
    ST_PIPELINE *pstPipeline,
    EN_MODULE_ID enModuleId
    );

extern
STF_VOID *STFLIB_ISP_PIPELINE_GetRegisteredModuleIspRdma(
    ST_PIPELINE *pstPipeline,
    EN_MODULE_ID enModuleId
    );

extern
STF_VOID *STFLIB_ISP_PIPELINE_GetModuleRdma(
    ST_PIPELINE *pstPipeline,
    EN_MODULE_ID enModuleId
    );

extern
STF_VOID *STFLIB_ISP_PIPELINE_GetRegisteredModuleRdma(
    ST_PIPELINE *pstPipeline,
    EN_MODULE_ID enModuleId
    );

extern
STF_VOID *STFLIB_ISP_PIPELINE_GetModuleParam(
    ST_PIPELINE *pstPipeline,
    EN_MODULE_ID enModuleId
    );

extern
STF_VOID *STFLIB_ISP_PIPELINE_GetRegisteredModuleParam(
    ST_PIPELINE *pstPipeline,
    EN_MODULE_ID enModuleId
    );

//-----------------------------------------------------------------------------
extern
STF_RESULT STFLIB_ISP_PIPELINE_RegisterControl(
    ST_PIPELINE *pstPipeline,
    ST_ISP_CTL *pstIspCtl
    );

extern
STF_RESULT STFLIB_ISP_PIPELINE_RegisterControlById(
    ST_PIPELINE *pstPipeline,
    EN_CONTROL_ID enControlId
    );

extern
STF_RESULT STFLIB_ISP_PIPELINE_RegisterEnabledControls(
    ST_PIPELINE *pstPipeline
    );

extern
STF_RESULT STFLIB_ISP_PIPELINE_RegisterAllOfControls(
    ST_PIPELINE *pstPipeline
    );

extern
STF_RESULT STFLIB_ISP_PIPELINE_UnregisterControl(
    ST_PIPELINE *pstPipeline,
    EN_CONTROL_ID enControlId
    );

extern
STF_RESULT STFLIB_ISP_PIPELINE_UnregisterDisabledControls(
    ST_PIPELINE *pstPipeline
    );

extern
STF_RESULT STFLIB_ISP_PIPELINE_UnregisterAllOfControls(
    ST_PIPELINE *pstPipeline
    );

extern
ST_ISP_CTL *STFLIB_ISP_PIPELINE_GetControl(
    ST_PIPELINE *pstPipeline,
    EN_CONTROL_ID enControlId
    );

extern
ST_ISP_CTL *STFLIB_ISP_PIPELINE_GetRegisteredControl(
    ST_PIPELINE *pstPipeline,
    EN_CONTROL_ID enControlId
    );

extern
STF_CHAR *STFLIB_ISP_PIPELINE_GetControlName(
    ST_PIPELINE *pstPipeline,
    EN_CONTROL_ID enControlId
    );

extern
STF_CHAR *STFLIB_ISP_PIPELINE_GetRegisteredControlName(
    ST_PIPELINE *pstPipeline,
    EN_CONTROL_ID enControlId
    );

extern
STF_VOID *STFLIB_ISP_PIPELINE_GetControlParam(
    ST_PIPELINE *pstPipeline,
    EN_CONTROL_ID enControlId
    );

extern
STF_VOID *STFLIB_ISP_PIPELINE_GetRegisteredControlParam(
    ST_PIPELINE *pstPipeline,
    EN_CONTROL_ID enControlId
    );

//-----------------------------------------------------------------------------
extern
STF_RESULT STFLIB_ISP_PIPELINE_RegisterEnabledModulesAndControls(
    ST_PIPELINE *pstPipeline
    );

extern
STF_RESULT STFLIB_ISP_PIPELINE_RegisterAllOfModulesAndControls(
    ST_PIPELINE *pstPipeline
    );

extern
STF_RESULT STFLIB_ISP_PIPELINE_UnregisterDisabledModulesAndControls(
    ST_PIPELINE *pstPipeline
    );

extern
STF_RESULT STFLIB_ISP_PIPELINE_UnregisterAllOfModulesAndControls(
    ST_PIPELINE *pstPipeline
    );

extern
STF_BOOL8 STFLIB_ISP_PIPELINE_IsModuleEnable(
    ST_PIPELINE *pstPipeline,
    EN_MODULE_ID enModuleId
    );

extern
STF_RESULT STFLIB_ISP_PIPELINE_ModuleEnable(
    ST_PIPELINE *pstPipeline,
    EN_MODULE_ID enModuleId,
    STF_BOOL8 bEnable
    );

extern
STF_BOOL8 STFLIB_ISP_PIPELINE_IsModuleUpdate(
    ST_PIPELINE *pstPipeline,
    EN_MODULE_ID enModuleId
    );

extern
STF_RESULT STFLIB_ISP_PIPELINE_ModuleUpdate(
    ST_PIPELINE *pstPipeline,
    EN_MODULE_ID enModuleId,
    STF_BOOL8 bUpdate
    );

extern
STF_BOOL8 STFLIB_ISP_PIPELINE_IsControlEnable(
    ST_PIPELINE *pstPipeline,
    EN_CONTROL_ID enControlId
    );

extern
STF_RESULT STFLIB_ISP_PIPELINE_ControlEnable(
    ST_PIPELINE *pstPipeline,
    EN_CONTROL_ID enControlId,
    STF_BOOL8 bEnable
    );

extern
STF_RESULT STFLIB_ISP_PIPELINE_ClearModulesAndControls(
    ST_PIPELINE *pstPipeline
    );

extern
STF_RESULT STFLIB_ISP_PIPELINE_PrintRegisteredModulesAndControls(
    ST_PIPELINE *pstPipeline
    );

//-----------------------------------------------------------------------------
extern
STF_RESULT STFLIB_ISP_PIPELINE_InitAll(
    ST_PIPELINE *pstPipeline
    );

extern
STF_RESULT STFLIB_ISP_PIPELINE_InitAllGlobals(
    ST_PIPELINE *pstPipeline
    );

extern
STF_RESULT STFLIB_ISP_PIPELINE_InitAllModules(
    ST_PIPELINE *pstPipeline
    );

extern
STF_RESULT STFLIB_ISP_PIPELINE_InitAllControls(
    ST_PIPELINE *pstPipeline
    );

extern
STF_RESULT STFLIB_ISP_PIPELINE_InitPipelineModulesAndControls(
    ST_PIPELINE *pstPipeline
    );

extern
STF_RESULT STFLIB_ISP_PIPELINE_SetNextRdma(
    ST_PIPELINE *pstPipeline
    );

extern
STF_RESULT STFLIB_ISP_PIPELINE_UpdateAll(
    ST_PIPELINE *pstPipeline
    );

extern
STF_RESULT STFLIB_ISP_PIPELINE_UpdateAllGlobals(
    ST_PIPELINE *pstPipeline
    );

extern
STF_RESULT STFLIB_ISP_PIPELINE_UpdateAllModules(
    ST_PIPELINE *pstPipeline
    );

extern
STF_RESULT STFLIB_ISP_PIPELINE_CalculateAllControls(
    ST_PIPELINE *pstPipeline
    );

extern
STF_RESULT STFLIB_ISP_PIPELINE_UpdateModule(
    ST_PIPELINE *pstPipeline,
    EN_MODULE_ID enModuleId
    );

extern
STF_RESULT STFLIB_ISP_PIPELINE_CalculateControl(
    ST_PIPELINE *pstPipeline,
    EN_CONTROL_ID enControlId
    );

extern
STF_RESULT STFLIB_ISP_PIPELINE_UpdateAllRequested(
    ST_PIPELINE *pstPipeline
    );

extern
STF_RESULT STFLIB_ISP_PIPELINE_UpdateAllRequestedModules(
    ST_PIPELINE *pstPipeline
    );

extern
STF_RESULT STFLIB_ISP_PIPELINE_CalculateAllRequestedControls(
    ST_PIPELINE *pstPipeline
    );

extern
STF_RESULT STFLIB_ISP_PIPELINE_SetupAll(
    ST_PIPELINE *pstPipeline
    );

extern
STF_RESULT STFLIB_ISP_PIPELINE_SetupAllGlobals(
    ST_PIPELINE *pstPipeline
    );

extern
STF_RESULT STFLIB_ISP_PIPELINE_SetupAllModules(
    ST_PIPELINE *pstPipeline
    );

extern
STF_RESULT STFLIB_ISP_PIPELINE_SetupModule(
    ST_PIPELINE *pstPipeline,
    EN_MODULE_ID enModuleId
    );

extern
STF_RESULT STFLIB_ISP_PIPELINE_SetupRequested(
    ST_PIPELINE *pstPipeline
    );

extern
STF_RESULT STFLIB_ISP_PIPELINE_ModuleGetIqParam(
    ST_PIPELINE *pstPipeline,
    EN_MODULE_ID enModuleId,
    STF_VOID *pParamBuf,
    STF_U16 *pu16ParamSize
    );

extern
STF_RESULT STFLIB_ISP_PIPELINE_ModuleSetIqParam(
    ST_PIPELINE *pstPipeline,
    EN_MODULE_ID enModuleId,
    STF_VOID *pParamBuf,
    STF_U16 u16ParamSize
    );

extern
STF_RESULT STFLIB_ISP_PIPELINE_ControlGetIqParam(
    ST_PIPELINE *pstPipeline,
    EN_CONTROL_ID enControlId,
    STF_VOID *pParamBuf,
    STF_U16 *pu16ParamSize
    );

extern
STF_RESULT STFLIB_ISP_PIPELINE_ControlSetIqParam(
    ST_PIPELINE *pstPipeline,
    EN_CONTROL_ID enControlId,
    STF_VOID *pParamBuf,
    STF_U16 u16ParamSize
    );

extern
STF_RESULT STFLIB_ISP_PIPELINE_LoadBinParam(
    ST_PIPELINE *pstPipeline,
    STF_CHAR *pszBinSettingFilename
    );

extern
STF_RESULT STFLIB_ISP_PIPELINE_LoadBinClbrt(
    ST_PIPELINE *pstPipeline
    );

//-----------------------------------------------------------------------------
extern
STF_RESULT STFLIB_ISP_PIPELINE_VerifyConfiguration(
    ST_PIPELINE *pstPipeline
    );

extern
STF_RESULT STFLIB_ISP_PIPELINE_UpdateCiPipeline(
    ST_PIPELINE *pstPipeline
    );

extern
STF_RESULT STFLIB_ISP_PIPELINE_ProgramPipeline(
    ST_PIPELINE *pstPipeline,
    STF_BOOL8 bUpdateASAP
    );

extern
STF_RESULT STFLIB_ISP_PIPELINE_AddShots(
    ST_PIPELINE *pstPipeline,
    STF_U8 u8Num
    );

extern
STF_RESULT STFLIB_ISP_PIPELINE_DeleteShots(
    ST_PIPELINE *pstPipeline
    );

extern
STF_RESULT STFLIB_ISP_PIPELINE_AcquireShot(
    ST_PIPELINE *pstPipeline,
    ST_SHOT *pstShot,
    STF_BOOL8 bBlocking
    );

extern
STF_RESULT STFLIB_ISP_PIPELINE_ProgramSpecifiedShot(
    ST_PIPELINE *pstPipeline,
    CI_BUFFID *pstBuffId
    );

extern
STF_RESULT STFLIB_ISP_PIPELINE_ProgramShot(
    ST_PIPELINE *pstPipeline
    );

extern
STF_VOID STFLIB_ISP_PIPELINE_ProcessShot(
    ST_PIPELINE *pstPipeline,
    ST_SHOT *pstShot,
    CI_SHOT *pstCIShotBuffer
    );

extern
STF_RESULT STFLIB_ISP_PIPELINE_ReleaseShot(
    ST_PIPELINE *pstPipeline,
    ST_SHOT *pstShot
    );

extern
STF_RESULT STFLIB_ISP_PIPELINE_AllocateBuffer(
    ST_PIPELINE *pstPipeline,
    CI_BUFFTYPE enBufferType,
    STF_U32 u32Size,
    STF_U32 *pu32BufferId
    );

extern
STF_RESULT STFLIB_ISP_PIPELINE_ImportBuffer(
    ST_PIPELINE *pstPipeline,
    CI_BUFFTYPE enBufferType,
    STF_U32 u32IonFd,
    STF_U32 u32Size,
    STF_U32 *pu32BufferId
    );

extern
STF_RESULT STFLIB_ISP_PIPELINE_DeregisterBuffer(
    ST_PIPELINE *pstPipeline,
    STF_U32 u32BufferID
    );

//-----------------------------------------------------------------------------
//=== Godspeed === Add new memory/buffer type support here.
extern
ST_RES STFLIB_ISP_PIPELINE_GetUoDimensions(
    ST_PIPELINE *pstPipeline
    );

extern
STF_RESULT STFLIB_ISP_PIPELINE_SetUoDimensions(
    ST_PIPELINE *pstPipeline,
    STF_U16 u16Width,
    STF_U16 u16Height
    );

extern
ST_RES STFLIB_ISP_PIPELINE_GetSs0Dimensions(
    ST_PIPELINE *pstPipeline
    );

extern
STF_RESULT STFLIB_ISP_PIPELINE_SetSs0Dimensions(
    ST_PIPELINE *pstPipeline,
    STF_U16 u16Width,
    STF_U16 u16Height
    );

extern
ST_RES STFLIB_ISP_PIPELINE_GetSs1Dimensions(
    ST_PIPELINE *pstPipeline
    );

extern
STF_RESULT STFLIB_ISP_PIPELINE_SetSs1Dimensions(
    ST_PIPELINE *pstPipeline,
    STF_U16 u16Width,
    STF_U16 u16Height
    );

extern
ST_RES STFLIB_ISP_PIPELINE_GetDumpDimensions(
    ST_PIPELINE *pstPipeline
    );

extern
STF_RESULT STFLIB_ISP_PIPELINE_SetDumpDimensions(
    ST_PIPELINE *pstPipeline,
    STF_U16 u16Width,
    STF_U16 u16Height
    );

extern
ST_RES STFLIB_ISP_PIPELINE_GetTiling_1_ReadDimensions(
    ST_PIPELINE *pstPipeline
    );

extern
STF_RESULT STFLIB_ISP_PIPELINE_SetTiling_1_ReadDimensions(
    ST_PIPELINE *pstPipeline,
    STF_U16 u16Width,
    STF_U16 u16Height
    );

extern
ST_RES STFLIB_ISP_PIPELINE_GetTiling_1_WriteDimensions(
    ST_PIPELINE *pstPipeline
    );

extern
STF_RESULT STFLIB_ISP_PIPELINE_SetTiling_1_WriteDimensions(
    ST_PIPELINE *pstPipeline,
    STF_U16 u16Width,
    STF_U16 u16Height
    );

//-----------------------------------------------------------------------------
extern
STF_RESULT STFLIB_ISP_PIPELINE_StartCapture(
    ST_PIPELINE *pstPipeline
    );

extern
STF_RESULT STFLIB_ISP_PIPELINE_StopCapture(
    ST_PIPELINE *pstPipeline
    );

extern
STF_RESULT STFLIB_ISP_PIPELINE_GetFirstAvailableBuffers(
    ST_PIPELINE *pstPipeline,
    CI_BUFFID *pstBuffId
    );

//-----------------------------------------------------------------------------
extern
STF_RESULT STFLIB_ISP_PIPELINE_Register(
    ST_PIPELINE *pstPipeline
    );

extern
STF_RESULT STFLIB_ISP_PIPELINE_SetActiveCapture(
    ST_PIPELINE *pstPipeline
    );

extern
STF_RESULT STFLIB_ISP_PIPELINE_ClearActiveCapture(
    ST_PIPELINE *pstPipeline
    );

extern
STF_RESULT STFLIB_ISP_PIPELINE_ResetFrameCount(
    ST_PIPELINE *pstPipeline
    );

extern
STF_U64 STFLIB_ISP_PIPELINE_GetFrameCount(
    ST_PIPELINE *pstPipeline
    );

extern
STF_RESULT STFLIB_ISP_PIPELINE_SetPendingCount(
    ST_PIPELINE *pstPipeline,
    STF_U32 u32PendingCount
    );

extern
STF_U32 STFLIB_ISP_PIPELINE_IncPendingCount(
    ST_PIPELINE *pstPipeline
    );

extern
STF_U32 STFLIB_ISP_PIPELINE_GetCompletedCount(
    ST_PIPELINE *pstPipeline,
    STF_BOOL8 bBlocking
    );

extern
STF_RESULT STFLIB_ISP_PIPELINE_SetCompletedCount(
    ST_PIPELINE *pstPipeline,
    STF_U32 u32CompletedCount
    );

extern
STF_RESULT STFLIB_ISP_PIPELINE_GetPendingCompletedCount(
    ST_PIPELINE *pstPipeline,
    STF_BOOL8 bBlocking,
    STF_BOOL8 *pbHwCapture,
    STF_U32 *pu32PendingCount,
    STF_U32 *pu32CompletedCount
    );

//-----------------------------------------------------------------------------
extern
STF_RESULT STFLIB_ISP_PIPELINE_StructInitialize(
    ST_PIPELINE *pstPipeline,
    STF_U8 u8IspIdx,
#if defined(VIRTUAL_IO_MAPPING)
    CI_CONNECTION *pCIConnection,
#endif //VIRTUAL_IO_MAPPING
    ST_SENSOR *pstSensor,
    EN_MEM_TYPE enMemType,
    ST_SIZE stCaptureSize,
    STF_BOOL8 bCheckSensorResolution
    );

extern
STF_RESULT STFLIB_ISP_PIPELINE_StructUninitialize(
    ST_PIPELINE *pstPipeline
    );


#ifdef __cplusplus
}
#endif


#endif //__STFLIB_ISP_PIPELINE_H__
