/**
  ******************************************************************************
  * @file  stf_isp_api.h
  * @author  StarFive Isp Team
  * @version  V1.0
  * @date  06/06/2022
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
  ******************************************************************************/


#ifndef __STF_ISP_API_H__
#define __STF_ISP_API_H__


#ifdef __cplusplus
extern "C" {
#endif


#include <stf_include.h>


//#define ISP_MAIN_FUNCTION_ENABLE
#define CAPTURE_WIDTH               (1920)
#define CAPTURE_HEIGHT              (1080)


/* ISP APIs structure */
#ifndef __STF_ISP_BASE_REG_H__
typedef enum _EN_KRN_MOD_ID {
    //---------- NONE ----------
    EN_KRN_MOD_ID_SUD_OFF = 0,      // 00       // 12 bits
    //---------- 0x0010 - REG_FUNCTION_MODE ----------
    EN_KRN_MOD_ID_DC,               // 01       // 12 bits
    EN_KRN_MOD_ID_DEC,              // 02       // 12 bits
    EN_KRN_MOD_ID_OBC,              // 03       // 12 bits
    EN_KRN_MOD_ID_LCBQ,             // 04       // 12 bits
    EN_KRN_MOD_ID_OECF,             // 05       // 12 bits
    EN_KRN_MOD_ID_OECFHM,           // 06       // 12 bits
    EN_KRN_MOD_ID_LCCF,             // 07       // 12 bits
    EN_KRN_MOD_ID_AWB,              // 08       // 12 bits
    EN_KRN_MOD_ID_OBA,              // 09       // 12 bits
    EN_KRN_MOD_ID_SC,               // 10       // 12 bits
    EN_KRN_MOD_ID_DUMP,             // 11       // 12 bits
    //---------- 0x0A08 - REG_ISPCTRL1 ----------
    EN_KRN_MOD_ID_DBC,              // 12       // 12 bits
    EN_KRN_MOD_ID_CTC,              // 13       // 12 bits
    EN_KRN_MOD_ID_CFA,              // 14       // 12 bits
    EN_KRN_MOD_ID_CAR,              // 15       // 12 bits
    EN_KRN_MOD_ID_CCM,              // 16       // 12 bits
    EN_KRN_MOD_ID_GMARGB,           // 17       // 12 bits
    EN_KRN_MOD_ID_R2Y,              // 18       // 12 bits
    EN_KRN_MOD_ID_YCRV,             // 19       // 12 bits
    EN_KRN_MOD_ID_SHRP,             // 20       // 12 bits
    EN_KRN_MOD_ID_DNYUV,            // 21       // 12 bits
    EN_KRN_MOD_ID_SAT,              // 22       // 12 bits
    EN_KRN_MOD_ID_YHIST,            // 23
    //---------- 0x0A00 - REG_ISPCTRL0 ----------
    EN_KRN_MOD_ID_OUT,              // 24       // 8 bits
    EN_KRN_MOD_ID_UO,               // 25       // 8 bits
    EN_KRN_MOD_ID_SS0,              // 26       // 8 bits
    EN_KRN_MOD_ID_SS1,              // 27       // 8 bits
    //---------- 0x0B48 - REG_ITIDPSR  ----------
    EN_KRN_MOD_ID_TIL,              // 28       // 10 - 12 bits
    EN_KRN_MOD_ID_TIL_1_RD,         // 29       // 10 - 12 bits
    EN_KRN_MOD_ID_TIL_1_WR,         // 30       // 10 - 12 bits
    //---------- 0x0008 - REG_CSIINTS and 0x0A50 - REG_IESHD ----------
    EN_KRN_MOD_ID_SUD,              // 31
    //---------- 0x0008 - REG_CSIINTS ----------
    EN_KRN_MOD_ID_SUD_CSI,          // 32
    //---------- 0x0A50 - REG_IESHD ----------
    EN_KRN_MOD_ID_SUD_ISP,          // 33
    //---------- 0x0000 - REG_ENABLE_STATUS ----------
    EN_KRN_MOD_ID_CSI,              // 34
    //---------- 0x0A00 - REG_ISPCTRL0 ----------
    EN_KRN_MOD_ID_ISP,              // 35
    //---------- NONE ----------
    EN_KRN_MOD_ID_BUF,              // 36
    EN_KRN_MOD_ID_MAX               // 37
} EN_KRN_MOD_ID, *PEN_KRN_MOD_ID;

//=============================================================================
#define EN_KRN_MOD_SUD_OFF      (1LL << EN_KRN_MOD_ID_SUD_OFF)
//=============================================================================
#define EN_KRN_MOD_DC           (1LL << EN_KRN_MOD_ID_DC)       // 0x0010 - REG_FUNCTION_MODE: (1 << 0).
#define EN_KRN_MOD_DEC          (1LL << EN_KRN_MOD_ID_DEC)      // 0x0010 - REG_FUNCTION_MODE: (1 << 1).
#define EN_KRN_MOD_OBC          (1LL << EN_KRN_MOD_ID_OBC)      // 0x0010 - REG_FUNCTION_MODE: (1 << 2).
#define EN_KRN_MOD_LCBQ         (1LL << EN_KRN_MOD_ID_LCBQ)     // 0x0010 - REG_FUNCTION_MODE: (1 << 3).
#define EN_KRN_MOD_OECF         (1LL << EN_KRN_MOD_ID_OECF)     // 0x0010 - REG_FUNCTION_MODE: (1 << 4).
#define EN_KRN_MOD_OECFHM       (1LL << EN_KRN_MOD_ID_OECFHM)   // 0x0010 - REG_FUNCTION_MODE: (1 << 5).
#define EN_KRN_MOD_LCCF         (1LL << EN_KRN_MOD_ID_LCCF)     // 0x0010 - REG_FUNCTION_MODE: (1 << 6).
#define EN_KRN_MOD_AWB          (1LL << EN_KRN_MOD_ID_AWB)      // 0x0010 - REG_FUNCTION_MODE: (1 << 7).
//-----------------------------------------------------------------------------
#define EN_KRN_MOD_OBA          (1LL << EN_KRN_MOD_ID_OBA)      // 0x0010 - REG_FUNCTION_MODE: (1 << 16).
#define EN_KRN_MOD_SC           (1LL << EN_KRN_MOD_ID_SC)       // 0x0010 - REG_FUNCTION_MODE: (1 << 17).
//-----------------------------------------------------------------------------
#define EN_KRN_MOD_DUMP         (1LL << EN_KRN_MOD_ID_DUMP)     // 0x0010 - REG_FUNCTION_MODE: (1 << 19).
//=============================================================================
#define EN_KRN_MOD_DBC          (1LL << EN_KRN_MOD_ID_DBC)      // 0x0A08 - REG_ISPCTRL1: (1 << 22).
#define EN_KRN_MOD_CTC          (1LL << EN_KRN_MOD_ID_CTC)      // 0x0A08 - REG_ISPCTRL1: (1 << 21).
#define EN_KRN_MOD_CFA          (1LL << EN_KRN_MOD_ID_CFA)      // 0x0A08 - REG_ISPCTRL1: (1 << 1).
#define EN_KRN_MOD_CAR          (1LL << EN_KRN_MOD_ID_CAR)      // 0x0A08 - REG_ISPCTRL1: (1 << 2).
#define EN_KRN_MOD_CCM          (1LL << EN_KRN_MOD_ID_CCM)      // 0x0A08 - REG_ISPCTRL1: (1 << 3).
#define EN_KRN_MOD_GMARGB       (1LL << EN_KRN_MOD_ID_GMARGB)   // 0x0A08 - REG_ISPCTRL1: (1 << 4).
#define EN_KRN_MOD_R2Y          (1LL << EN_KRN_MOD_ID_R2Y)      // 0x0A08 - REG_ISPCTRL1: (1 << 5).
#define EN_KRN_MOD_YCRV         (1LL << EN_KRN_MOD_ID_YCRV)     // 0x0A08 - REG_ISPCTRL1: (1 << 19).
#define EN_KRN_MOD_SHRP         (1LL << EN_KRN_MOD_ID_SHRP)     // 0x0A08 - REG_ISPCTRL1: (1 << 7).
#define EN_KRN_MOD_DNYUV        (1LL << EN_KRN_MOD_ID_DNYUV)    // 0x0A08 - REG_ISPCTRL1: (1 << 17).
#define EN_KRN_MOD_SAT          (1LL << EN_KRN_MOD_ID_SAT)      // 0x0A08 - REG_ISPCTRL1: (1 << 8).
//-----------------------------------------------------------------------------
#define EN_KRN_MOD_YHIST        (1LL << EN_KRN_MOD_ID_YHIST)    // 0x0A08 - REG_ISPCTRL1: (1 << 20).
//=============================================================================
#define EN_KRN_MOD_OUT          (1LL << EN_KRN_MOD_ID_OUT)      // 0x0A00 - REG_ISPCTRL0: ((1 << 20) | (1 << 11) | (1 << 12)).
#define EN_KRN_MOD_UO           (1LL << EN_KRN_MOD_ID_UO)       // 0x0A00 - REG_ISPCTRL0: (1 << 20).
//-----------------------------------------------------------------------------
#define EN_KRN_MOD_SS0          (1LL << EN_KRN_MOD_ID_SS0)      // 0x0A00 - REG_ISPCTRL0: (1 << 11).
#define EN_KRN_MOD_SS1          (1LL << EN_KRN_MOD_ID_SS1)      // 0x0A00 - REG_ISPCTRL0: (1 << 12).
//=============================================================================
#define EN_KRN_MOD_TIL          (1LL << EN_KRN_MOD_ID_TIL)      // 0x0B48 - REG_ITIDPSR: ((1 << 16) | (1 << 17)).
#define EN_KRN_MOD_TIL_1_RD     (1LL << EN_KRN_MOD_ID_TIL_1_RD) // 0x0B48 - REG_ITIDPSR: (1 << 16).
#define EN_KRN_MOD_TIL_1_WR     (1LL << EN_KRN_MOD_ID_TIL_1_WR) // 0x0B48 - REG_ITIDPSR: (1 << 17).
//=============================================================================
#define EN_KRN_MOD_SUD          (1LL << EN_KRN_MOD_ID_SUD)      // 0x0008 - REG_CSIINTS: (1 << 16), 0x0A50 - REG_IESHD: (1 << 0).
//=============================================================================
#define EN_KRN_MOD_SUD_CSI      (1LL << EN_KRN_MOD_ID_SUD_CSI)  // 0x0008 - REG_CSIINTS: (1 << 16).
//=============================================================================
#define EN_KRN_MOD_SUD_ISP      (1LL << EN_KRN_MOD_ID_SUD_ISP)  // 0x0A50 - REG_IESHD: (1 << 0).
//=============================================================================
#define EN_KRN_MOD_CSI          (1LL << EN_KRN_MOD_ID_CSI)      // 0x0000 - REG_ENABLE_STATUS: (1 << 0).
//=============================================================================
#define EN_KRN_MOD_ISP          (1LL << EN_KRN_MOD_ID_ISP)      // 0x0A00 - REG_ISPCTRL0: (1 << 0).
//=============================================================================
#define EN_KRN_MOD_BUF          (1LL << EN_KRN_MOD_ID_BUF)      // 0xXXXX - XXXX.
//=============================================================================

#endif //#ifndef __STF_ISP_BASE_REG_H__
#ifndef __STFLIB_ISP_PIPELINE_H__
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

#endif //#ifndef __STFLIB_ISP_PIPELINE_H__
//=== Godspeed === Add new memory/buffer type support here.
typedef enum _EN_SHOT_KIND {
    EN_SHOT_KIND_AVAILABLE = 0,
    EN_SHOT_KIND_PENDING,
    EN_SHOT_KIND_COMPLETED,
    EN_SHOT_KIND_OUTPUT,
    EN_SHOT_KIND_RTSP,
    EN_SHOT_KIND_TEMPORARY,
    EN_SHOT_KIND_MAX
} EN_SHOT_KIND, *PEN_SHOT_KIND;

typedef enum _EN_MEM_KIND {
    EN_MEM_KIND_UO = 0x0001,
    EN_MEM_KIND_SS0 = 0x0002,
    EN_MEM_KIND_SS1 = 0x0004,
    EN_MEM_KIND_DUMP = 0x0008,
    EN_MEM_KIND_TIL_1_RD = 0x0010,
    EN_MEM_KIND_TIL_1_WR = 0x0020,
    EN_MEM_KIND_SC = 0x0100,
    EN_MEM_KIND_Y_HIST = 0x0200,
    EN_MEM_KIND_OUT = (EN_MEM_KIND_UO
                     | EN_MEM_KIND_SS0
                     | EN_MEM_KIND_SS1
                     ),
    EN_MEM_KIND_TIL_1 = (EN_MEM_KIND_TIL_1_RD
                       | EN_MEM_KIND_TIL_1_WR
                       ),
    EN_MEM_KIND_TIL = (EN_MEM_KIND_TIL_1
                     ),
    EN_MEM_KIND_STAT = (EN_MEM_KIND_SC
                      | EN_MEM_KIND_Y_HIST
                      ),
    EN_MEM_KIND_ALL = (EN_MEM_KIND_OUT
                     | EN_MEM_KIND_DUMP
                     | EN_MEM_KIND_TIL
                     | EN_MEM_KIND_STAT
                     ),
} EN_MEM_KIND, *PEN_MEM_KIND;

typedef enum _EN_ISP_MAIN_STEP {
    EN_ISP_MAIN_STEP_NONE = 0,
    EN_ISP_MAIN_STEP_BUFFER_INIT,
    EN_ISP_MAIN_STEP_PREPARE_STOP,
    EN_ISP_MAIN_STEP_STOP,
    EN_ISP_MAIN_STEP_PREPARE_CAPTURE,
    EN_ISP_MAIN_STEP_CAPTURE,
    EN_ISP_MAIN_STEP_MAX
} EN_ISP_MAIN_STEP, *PEN_ISP_MAIN_STEP;

typedef enum _EN_SETTING_KIND {
    EN_SETTING_KIND_ORIGINAL = 0,
    EN_SETTING_KIND_CUSTOMIZE
} EN_SETTING_KIND, *PEN_SETTING_KIND;
#define EN_SETTING_KIND_MAX         (EN_SETTING_KIND_CUSTOMIZE + 1)


#ifndef __STFLIB_ISP_BASE_H__
typedef struct _ST_REG_TBL {
    STF_U32 u32Offset;                          /** Register offset. */
    STF_U32 u32Value;                           /** Register value. */
} ST_REG_TBL, *PST_REG_TBL;

#endif //__STFLIB_ISP_BASE_H__
typedef struct _ST_MEM_INFO {
    STF_U32 u32ImgWidth;                        /** Width of image. */
    STF_U32 u32ImgHeight;                       /** Heigt of image. */
    STF_U32 u32ImgStride;                       /** Line stride of image. */
    STF_U32 u32BitDepth;                        /** Bits of per pixel. */
    STF_U32 u32MosaicType;                      /** Mosaic type. 0: MOSAIC_NONE, 1: MOSAIC_RGGB, 2: MOSAIC_GRBG, 3: MOSAIC_GBRG, 4: MOSAIC_BGGR. */
    STF_U32 u32MemSize;                         /** Total allocate memory size. */
    STF_VOID *pvMemBuf;                         /** Memory pointer. */
} ST_MEM_INFO, *PST_MEM_INFO;

typedef struct _ST_ISP_MEMS_INFO {
    ST_MEM_INFO stMemInfoUo;                    /** Uo memory information. */
    ST_MEM_INFO stMemInfoSs0;                   /** Ss0 memory information. */
    ST_MEM_INFO stMemInfoSs1;                   /** Ss1 memory information. */
    ST_MEM_INFO stMemInfoDump;                  /** Dump memory information. */
    ST_MEM_INFO stMemInfoTiling_1_Read;         /** Tiling 1 read memory information. */
    ST_MEM_INFO stMemInfoTiling_1_Write;        /** Tiling 1 write memory information. */
    ST_MEM_INFO stMemInfoScDump;                /** ScDump memory information. */
    ST_MEM_INFO stMemInfoYHist;                 /** YHist memory information. */
} ST_ISP_MEMS_INFO, *PST_ISP_MEMS_INFO;


/* ISP APIs variables */


/* ISP APIs interface */
//-----------------------------------------------------------------------------
extern
STF_RESULT STFAPI_ISP_RegReadSeries(
    STF_S8 s8IspIdx,
    STF_U32 u32RegAddr,
    STF_U32 u32BufLen,
    STF_U32 *pu32RegBuf
    );

extern
STF_RESULT STFAPI_ISP_RegWriteSeries(
    STF_S8 s8IspIdx,
    STF_U32 u32RegAddr,
    STF_U32 u32BufLen,
    STF_U32 *pu32RegBuf
    );

extern
STF_RESULT STFAPI_ISP_RegReadByTable(
    STF_S8 s8IspIdx,
    STF_U32 u32BufLen,
    ST_REG_TBL *pstRegTbl
    );

extern
STF_RESULT STFAPI_ISP_RegWriteByTable(
    STF_S8 s8IspIdx,
    STF_U32 u32BufLen,
    ST_REG_TBL *pstRegTbl
    );

extern
STF_RESULT STFAPI_ISP_MemBufInfoGet(
    EN_MEM_KIND enMemKind,
    ST_ISP_MEMS_INFO *pstIspMemsInfo
    );

extern
STF_RESULT STFAPI_ISP_MemBufGet(
    EN_MEM_KIND enMemKind,
    ST_ISP_MEMS_INFO *pstIspMemsInfo,
    STF_VOID *pstSrvShotInfo
    );

extern
STF_RESULT STFAPI_ISP_MemBufPush(
    STF_VOID *pstSrvShotInfo
    );

extern
STF_RESULT STFAPI_ISP_MemBufPop(
    EN_MEM_KIND enMemKind,
    ST_ISP_MEMS_INFO *pstIspMemsInfo,
    STF_VOID **ppstSrvShotInfo
    );

extern
STF_RESULT STFAPI_ISP_ShotMemBufGet(
    EN_SHOT_KIND enShotKind,
    EN_MEM_KIND enMemKind,
    ST_MEM_INFO *pstMemInfo,
    STF_VOID *pstSrvShotInfo,
    STF_BOOL8 bAeDebugMessage
    );

extern
STF_U64 STFAPI_ISP_GetIspVersion(
    STF_VOID
    );

extern
STF_U64 STFAPI_ISP_GetIspSdkVersion(
    STF_VOID
    );

extern
STF_RESULT STFAPI_ISP_Pipeline_ModuleEnable(
    STF_U8 u8IspIdx,
    STF_U16 u16ModuleId,
    STF_BOOL8 bEnable
    );

extern
STF_BOOL8 STFAPI_ISP_Pipeline_IsModuleEnable(
    STF_U8 u8IspIdx,
    STF_U16 u16ModuleId
    );

extern
STF_RESULT STFAPI_ISP_Pipeline_ModuleUpdate(
    STF_U8 u8IspIdx,
    STF_U16 u16ModuleId,
    STF_BOOL8 bUpdate
    );

extern
STF_BOOL8 STFAPI_ISP_Pipeline_IsModuleUpdate(
    STF_U8 u8IspIdx,
    STF_U16 u16ModuleId
    );

extern
STF_RESULT STFAPI_ISP_Pipeline_ControlEnable(
    STF_U8 u8IspIdx,
    STF_U16 u16ControlId,
    STF_BOOL8 bEnable
    );

extern
STF_BOOL8 STFAPI_ISP_Pipeline_IsControlEnable(
    STF_U8 u8IspIdx,
    STF_U16 u16ControlId
    );

extern
STF_RESULT STFAPI_ISP_SetModulesEnable(
    STF_U8 u8IspIdx,
    STF_U64 u64Modules,
    STF_BOOL8 bEnable
    );

extern
STF_RESULT STFAPI_ISP_GetIqParam(
    STF_U32 u32ModuleControlId,
    STF_VOID *pvParamBuf,
    STF_U16 *pu16ParamSize
    );

extern
STF_RESULT STFAPI_ISP_SetIqParam(
    STF_U32 u32ModuleControlId,
    STF_VOID *pvParamBuf,
    STF_U16 u16ParamSize
    );

extern
STF_RESULT STFAPI_ISP_RestoreSetting(
    STF_U8 u8SettingKind
    );

//-----------------------------------------------------------------------------
extern
EN_ISP_MAIN_STEP STFAPI_ISP_GetIspMainStep(
    STF_VOID
    );

extern
STF_INT STFAPI_ISP_StartIspMainThread(
    STF_VOID
    );

extern
STF_INT STFAPI_ISP_StopIspMainThread(
    STF_VOID
    );

extern
STF_RESULT STFAPI_ISP_GetResolution(
    STF_S32 *ps32Width,
    STF_S32 *ps32Height
    );

extern
STF_RESULT STFAPI_ISP_SetResolution(
    STF_S32 s32Width,
    STF_S32 s32Height
    );

extern
STF_VOID STFAPI_ISP_StartIspCapture(
    STF_VOID
    );

extern
STF_VOID STFAPI_ISP_StopIspCapture(
    STF_VOID
    );

//-----------------------------------------------------------------------------
extern
STF_U16 STFAPI_ISP_ReadSensorReg(
    STF_U16 u16RegAddr
    );

extern
STF_RESULT STFAPI_ISP_WriteSensorReg(
    STF_U16 u16RegAddr,
    STF_U16 u16RegValue
    );

extern
STF_RESULT STFAPI_ISP_GetExposureAndGain(
    STF_U32 *pu32SnrExposureTime,
    STF_DOUBLE *pdSnrGain
    );

extern
STF_RESULT STFAPI_ISP_SetExposureAndGain(
    STF_U32 u32SnrExposureTime,
    STF_DOUBLE dSnrGain
    );

extern
STF_RESULT STFAPI_ISP_GetAwbGain(
    STF_DOUBLE *pdRedGain,
    STF_DOUBLE *pdBlueGain,
    STF_DOUBLE *pdDigitalGain
    );

extern
STF_RESULT STFAPI_ISP_SetAwbGain(
    STF_DOUBLE dRedGain,
    STF_DOUBLE dBlueGain,
    STF_DOUBLE dDigitalGain
    );

//-----------------------------------------------------------------------------
extern
double currTime(
    STF_VOID
    );

extern
STF_VOID Isp_Main_Thread(
    STF_VOID *pstThreadData
    );

#ifdef ISP_MAIN_FUNCTION_ENABLE
extern
STF_RESULT Isp_Main(
    STF_VOID
    );

#endif //ISP_MAIN_FUNCTION_ENABLE
//-----------------------------------------------------------------------------


#ifdef __cplusplus
}
#endif


#endif /* __STF_ISP_API_H__ */
