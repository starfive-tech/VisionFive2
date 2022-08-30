/**
  ******************************************************************************
  * @file  stfmod_isp_cbar.h
  * @author  StarFive Isp Team
  * @version  V1.0
  * @date  09/01/2021
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


#ifndef __STFMOD_ISP_CBAR_H__
#define __STFMOD_ISP_CBAR_H__


#ifdef __cplusplus
extern "C" {
#endif


#include "stf_include.h"
#include "ci/ci_api_structs.h"
#include "registers/stf_isp_cbar_rdma.h"
#include "stflib_isp_pipeline.h"


/* auto white-balance module parameters structure */
//-----------------------------------------------------------------------------
// Define initial values
#define ISP_DEF_CBAR_USING_SENSOR_INFO_FOR_CALC (STF_TRUE)
#define ISP_DEF_CBAR_OUTPUT_WIDTH               (1920)
#define ISP_DEF_CBAR_OUTPUT_HEIGHT              (1080)
#define ISP_DEF_CBAR_FRAME_HORIZONTAL_SHIFT     (2)
#define ISP_DEF_CBAR_WIDTH                      (ISP_DEF_CBAR_OUTPUT_WIDTH + 4)
#define ISP_DEF_CBAR_HEIGHT                     (ISP_DEF_CBAR_OUTPUT_HEIGHT + 6)
#define ISP_DEF_CBAR_FRAME_LEFT                 (0)
#define ISP_DEF_CBAR_FRAME_TOP                  (0)
#define ISP_DEF_CBAR_FRAME_RIGHT                (ISP_DEF_CBAR_OUTPUT_WIDTH - 1)
#define ISP_DEF_CBAR_FRAME_BOTTOM               (ISP_DEF_CBAR_OUTPUT_HEIGHT - 1)
#define ISP_DEF_CBAR_DVP_MUX_EN                 (STF_TRUE)
#define ISP_DEF_CBAR_MIPI_MUX_EN                (STF_FALSE)
#define ISP_DEF_CBAR_SYNC_CSI                   (STF_TRUE)
#define ISP_DEF_CBAR_H_SYNC_POLARITY            (STF_FALSE)
#define ISP_DEF_CBAR_V_SYNC_POLARITY            (STF_TRUE)
#define ISP_DEF_CBAR_PATTERN_TYPE               (EN_CBAR_PATTERN_TYPE_H_GRADIENT)
#define ISP_DEF_CBAR_HOR_BAR_WIDTH              (4)
#define ISP_DEF_CBAR_Ver_BAR_WIDTH              (4)
#define ISP_DEF_CBAR_MODE_1                     (0x00000000)
#if 1
#define ISP_DEF_CBAR_LINE_0_PIXEL_0             (0x0000)
#define ISP_DEF_CBAR_LINE_0_PIXEL_1             (0x0000)
#define ISP_DEF_CBAR_LINE_0_PIXEL_2             (0x0222)
#define ISP_DEF_CBAR_LINE_0_PIXEL_3             (0x0222)
#define ISP_DEF_CBAR_LINE_0_PIXEL_4             (0x0444)
#define ISP_DEF_CBAR_LINE_0_PIXEL_5             (0x0444)
#define ISP_DEF_CBAR_LINE_0_PIXEL_6             (0x0666)
#define ISP_DEF_CBAR_LINE_0_PIXEL_7             (0x0666)
#define ISP_DEF_CBAR_LINE_0_PIXEL_8             (0x0888)
#define ISP_DEF_CBAR_LINE_0_PIXEL_9             (0x0888)
#define ISP_DEF_CBAR_LINE_0_PIXEL_A             (0x0AAA)
#define ISP_DEF_CBAR_LINE_0_PIXEL_B             (0x0AAA)
#define ISP_DEF_CBAR_LINE_0_PIXEL_C             (0x0CCC)
#define ISP_DEF_CBAR_LINE_0_PIXEL_D             (0x0CCC)
#define ISP_DEF_CBAR_LINE_0_PIXEL_E             (0x0EEE)
#define ISP_DEF_CBAR_LINE_0_PIXEL_F             (0x0EEE)
#define ISP_DEF_CBAR_LINE_1_PIXEL_0             (0x0000)
#define ISP_DEF_CBAR_LINE_1_PIXEL_1             (0x0000)
#define ISP_DEF_CBAR_LINE_1_PIXEL_2             (0x0222)
#define ISP_DEF_CBAR_LINE_1_PIXEL_3             (0x0222)
#define ISP_DEF_CBAR_LINE_1_PIXEL_4             (0x0444)
#define ISP_DEF_CBAR_LINE_1_PIXEL_5             (0x0444)
#define ISP_DEF_CBAR_LINE_1_PIXEL_6             (0x0666)
#define ISP_DEF_CBAR_LINE_1_PIXEL_7             (0x0666)
#define ISP_DEF_CBAR_LINE_1_PIXEL_8             (0x0888)
#define ISP_DEF_CBAR_LINE_1_PIXEL_9             (0x0888)
#define ISP_DEF_CBAR_LINE_1_PIXEL_A             (0x0AAA)
#define ISP_DEF_CBAR_LINE_1_PIXEL_B             (0x0AAA)
#define ISP_DEF_CBAR_LINE_1_PIXEL_C             (0x0CCC)
#define ISP_DEF_CBAR_LINE_1_PIXEL_D             (0x0CCC)
#define ISP_DEF_CBAR_LINE_1_PIXEL_E             (0x0EEE)
#define ISP_DEF_CBAR_LINE_1_PIXEL_F             (0x0EEE)
#else
#define ISP_DEF_CBAR_LINE_0_PIXEL_0             (0x0111)
#define ISP_DEF_CBAR_LINE_0_PIXEL_1             (0x0111)
#define ISP_DEF_CBAR_LINE_0_PIXEL_2             (0x0333)
#define ISP_DEF_CBAR_LINE_0_PIXEL_3             (0x0333)
#define ISP_DEF_CBAR_LINE_0_PIXEL_4             (0x0555)
#define ISP_DEF_CBAR_LINE_0_PIXEL_5             (0x0555)
#define ISP_DEF_CBAR_LINE_0_PIXEL_6             (0x0777)
#define ISP_DEF_CBAR_LINE_0_PIXEL_7             (0x0777)
#define ISP_DEF_CBAR_LINE_0_PIXEL_8             (0x0999)
#define ISP_DEF_CBAR_LINE_0_PIXEL_9             (0x0999)
#define ISP_DEF_CBAR_LINE_0_PIXEL_A             (0x0BBB)
#define ISP_DEF_CBAR_LINE_0_PIXEL_B             (0x0BBB)
#define ISP_DEF_CBAR_LINE_0_PIXEL_C             (0x0DDD)
#define ISP_DEF_CBAR_LINE_0_PIXEL_D             (0x0DDD)
#define ISP_DEF_CBAR_LINE_0_PIXEL_E             (0x0FFF)
#define ISP_DEF_CBAR_LINE_0_PIXEL_F             (0x0FFF)
#define ISP_DEF_CBAR_LINE_1_PIXEL_0             (0x0111)
#define ISP_DEF_CBAR_LINE_1_PIXEL_1             (0x0111)
#define ISP_DEF_CBAR_LINE_1_PIXEL_2             (0x0333)
#define ISP_DEF_CBAR_LINE_1_PIXEL_3             (0x0333)
#define ISP_DEF_CBAR_LINE_1_PIXEL_4             (0x0555)
#define ISP_DEF_CBAR_LINE_1_PIXEL_5             (0x0555)
#define ISP_DEF_CBAR_LINE_1_PIXEL_6             (0x0777)
#define ISP_DEF_CBAR_LINE_1_PIXEL_7             (0x0777)
#define ISP_DEF_CBAR_LINE_1_PIXEL_8             (0x0999)
#define ISP_DEF_CBAR_LINE_1_PIXEL_9             (0x0999)
#define ISP_DEF_CBAR_LINE_1_PIXEL_A             (0x0BBB)
#define ISP_DEF_CBAR_LINE_1_PIXEL_B             (0x0BBB)
#define ISP_DEF_CBAR_LINE_1_PIXEL_C             (0x0DDD)
#define ISP_DEF_CBAR_LINE_1_PIXEL_D             (0x0DDD)
#define ISP_DEF_CBAR_LINE_1_PIXEL_E             (0x0FFF)
#define ISP_DEF_CBAR_LINE_1_PIXEL_F             (0x0FFF)
#endif


#pragma pack(push, 1)

typedef enum _EN_CBAR_UPDATE {
    EN_CBAR_UPDATE_NONE = 0x00,
    EN_CBAR_UPDATE_ENABLE = 0x01,
    EN_CBAR_UPDATE_SIZE = 0x01,
    EN_CBAR_UPDATE_FRAME = 0x02,
    EN_CBAR_UPDATE_MODE = 0x04,
    EN_CBAR_UPDATE_PIXEL_VALUE = 0x08,
    EN_CBAR_UPDATE_ALL = (EN_CBAR_UPDATE_ENABLE
                        | EN_CBAR_UPDATE_SIZE
                        | EN_CBAR_UPDATE_FRAME
                        | EN_CBAR_UPDATE_MODE
                        | EN_CBAR_UPDATE_PIXEL_VALUE
                        ),
} EN_CBAR_UPDATE, *PEN_CBAR_UPDATE;

typedef enum _EN_CBAR_PATTERN_TYPE {
    EN_CBAR_PATTERN_TYPE_V_BAR = 0,
    EN_CBAR_PATTERN_TYPE_H_BAR,
    EN_CBAR_PATTERN_TYPE_V_GRADIENT,
    EN_CBAR_PATTERN_TYPE_H_GRADIENT,
    EN_CBAR_PATTERN_TYPE_MAX
} EN_CBAR_PATTERN_TYPE, *PEN_CBAR_PATTERN_TYPE;


typedef struct _ST_CBAR_MODE {
    STF_BOOL8 bDvpMuxEnable;                    /** Enable DVP MUX (Switch to test pattern. */
    STF_BOOL8 bMipiMuxEnable;                   /** Enable MIPI MUX (Switch to test pattern. */
    STF_BOOL8 bSyncCsi;                         /** Wait for CSI enable (Color Bar Generator starts in sync with CSI enable). */
    STF_BOOL8 bHorBlankingPolarity;             /** Horizontal blanking polarity. 0:High active, 1:Low active. */
    STF_BOOL8 bVerBlankingPolarity;             /** Vertical blanking polarity. 0:High active, 1:Low active. */
    STF_U8 u8PatternType;                       /** Pattern type. 0:Vertical bars, 1:Horizontal bars, 2:Vertical gradient, 3:Horizontal gradient. */
    STF_U8 u8HorBarWidth;                       /** Horizontal bar width: 2^(WIDTH+1). Range from 0 to 15. */
    STF_U8 u8VerBarWidth;                       /** Vertical bar width: 2^(WIDTH+1). Range from 0 to 15. */
    STF_U32 u32Mode1;                           /** Color bar mode 1. TBD. */
} ST_CBAR_MODE, *PST_CBAR_MODE;

typedef struct _ST_CBAR_PXL_VAL {
    STF_U16 u16Pixel_0;                         /** Pixel 0 value. */
    STF_U16 u16Pixel_1;                         /** Pixel 1 value. */
    STF_U16 u16Pixel_2;                         /** Pixel 2 value. */
    STF_U16 u16Pixel_3;                         /** Pixel 3 value. */
    STF_U16 u16Pixel_4;                         /** Pixel 4 value. */
    STF_U16 u16Pixel_5;                         /** Pixel 5 value. */
    STF_U16 u16Pixel_6;                         /** Pixel 6 value. */
    STF_U16 u16Pixel_7;                         /** Pixel 7 value. */
    STF_U16 u16Pixel_8;                         /** Pixel 8 value. */
    STF_U16 u16Pixel_9;                         /** Pixel 9 value. */
    STF_U16 u16Pixel_A;                         /** Pixel A value. */
    STF_U16 u16Pixel_B;                         /** Pixel B value. */
    STF_U16 u16Pixel_C;                         /** Pixel C value. */
    STF_U16 u16Pixel_D;                         /** Pixel D value. */
    STF_U16 u16Pixel_E;                         /** Pixel E value. */
    STF_U16 u16Pixel_F;                         /** Pixel F value. */
} ST_CBAR_PXL_VAL, *PST_CBAR_PXL_VAL;

typedef struct _ST_MOD_CBAR_SETTING {
    //-------------------------------------------------------------------------
    // This section parameters value is assign from setting file.
    STF_BOOL8 bEnable;                          /** Enable/Disable CBAR gain module. */
    STF_BOOL8 bUsingSensorInfoForCalc;          /** Enable this flag to using the assign sensor image size to calculate the color bar size, frame window and polarity. */
    ST_SIZE stColorBarSize;                     /** Color bar total output size. */
    ST_RECT stFrameWindow;                      /** Active frame window size. */
    ST_CBAR_MODE stMode;                        /** Color bar mode. */
    ST_CBAR_PXL_VAL stLine0PixelValue;          /** 16 pixels value for line 0. */
    ST_CBAR_PXL_VAL stLine1PixelValue;          /** 16 pixels value for line 1. */
} ST_MOD_CBAR_SETTING, *PST_MOD_CBAR_SETTING;

typedef struct _ST_MOD_CBAR_INFO {
    ST_SIZE stImgSize;                          /** Image size of pipeline. */
    STF_BOOL8 bSensorHorSyncPolarity;           /** Sensor horizontal polarity. */
    STF_BOOL8 bSensorVerSyncPolarity;           /** Sensor vertical polarity. */
} ST_MOD_CBAR_INFO, *PST_MOD_CBAR_INFO;

typedef struct _ST_CBAR_PARAM {
    ST_MOD_CBAR_SETTING stSetting;              /** Module CBAR setting file parameters. */
    //-------------------------------------------------------------------------
    // This section parameters is internal using.
    ST_MOD_CBAR_INFO stInfo;                    /** Module CBAR output information parameters. */
    STF_BOOL8 bSetup;                           /** Flag set if the module setup has been completed. */
    STF_BOOL8 bUpdate;                          /** Enable/Disable program CBAR module register. */
    STF_U32 u32UpdateRequest;                   /** Flag to indicate that this module update has been requested. */
} ST_CBAR_PARAM, *PST_CBAR_PARAM;

#define ST_MOD_CBAR_SETTING_LEN          (sizeof(ST_MOD_CBAR_SETTING))
#define ST_MOD_CBAR_INFO_LEN             (sizeof(ST_MOD_CBAR_INFO))
#define ST_MOD_CBAR_GET_PARAM_LEN        (ST_MOD_CBAR_SETTING_LEN + ST_MOD_CBAR_INFO_LEN)
#define ST_MOD_CBAR_SET_PARAM_LEN        (ST_MOD_CBAR_SETTING_LEN)
#define ST_CBAR_PARAM_SIZE              (sizeof(ST_CBAR_PARAM))
#define ST_CBAR_PARAM_LEN               (ST_CBAR_PARAM_SIZE)

typedef struct _ST_ISP_MOD_CBAR {
#if defined(VIRTUAL_IO_MAPPING)
    STF_VOID *pPipeline;                        /** Pointer to Pipeline. */
#endif //VIRTUAL_IO_MAPPING
    STF_VOID *pIspCtx;                          /** Pointer to ISP context. */
    STF_U32 u32Id;                              /** Module ID. */
    STF_CHAR szName[16];                        /** Module Name. */
    CI_MEM_PARAM *pstCBarRdmaBuf;               /** CBAR's DMA buffer pointer for RDMA. */
#if defined(SUPPORT_MULTI_ISP)
    ST_ISP_CBAR_RDMA *pstIspCBarRdma;           /** CBAR's RDMA structure pointer that include the ISP index.*/
#endif //SUPPORT_MULTI_ISP
    ST_CBAR_RDMA *pstCBarRdma;                  /** CBAR's RDMA structure pointer.*/
    ST_CBAR_PARAM *pstCBarParam;                /** CBAR's parameters structure pointer. */
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
} ST_ISP_MOD_CBAR, *PST_ISP_MOD_CBAR;

#pragma pack(pop)


/* auto white-balance module parameters interface */
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_CBAR_InitRdma(
    ST_CBAR_RDMA *pstCBarRdma,
    ST_CBAR_PARAM *pstCBarParam,
    STF_U32 *pNextRdma
    );
extern
STF_S32 STFMOD_ISP_CBAR_UpdateRdma(
    ST_CBAR_RDMA *pstCBarRdma,
    ST_CBAR_PARAM *pstCBarParam
    );
extern
STF_S32 STFMOD_ISP_CBAR_UpdateEnable(
    ST_CBAR_RDMA *pstCBarRdma,
    ST_CBAR_PARAM *pstCBarParam
    );
extern
STF_S32 STFMOD_ISP_CBAR_UpdateSize(
    ST_CBAR_RDMA *pstCBarRdma,
    ST_CBAR_PARAM *pstCBarParam
    );
extern
STF_S32 STFMOD_ISP_CBAR_UpdateFrame(
    ST_CBAR_RDMA *pstCBarRdma,
    ST_CBAR_PARAM *pstCBarParam
    );
extern
STF_S32 STFMOD_ISP_CBAR_UpdateMode(
    ST_CBAR_RDMA *pstCBarRdma,
    ST_CBAR_PARAM *pstCBarParam
    );
extern
STF_S32 STFMOD_ISP_CBAR_UpdatePixelValue(
    ST_CBAR_RDMA *pstCBarRdma,
    ST_CBAR_PARAM *pstCBarParam
    );
extern
STF_S32 STFMOD_ISP_CBAR_SetNextRdma(
    ST_CBAR_RDMA *pstCBarRdma,
    STF_U32 *pNextRdma
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_CBAR_SetEnable(
    ST_ISP_MOD_CBAR *pstModCBar,
    STF_BOOL8 bEnable
    );
extern
STF_S32 STFMOD_ISP_CBAR_SetUpdate(
    ST_ISP_MOD_CBAR *pstModCBar,
    STF_BOOL8 bUpdate
    );
extern
STF_S32 STFMOD_ISP_CBAR_SetEnableUsingSensorInfoForCalc(
    ST_ISP_MOD_CBAR *pstModCBar,
    STF_BOOL8 bUsingSensorInfoForCalc
    );
extern
STF_S32 STFMOD_ISP_CBAR_SetImageSize(
    ST_ISP_MOD_CBAR *pstModCBar,
    ST_SIZE stImgSize
    );
extern
STF_S32 STFMOD_ISP_CBAR_SetSensorPolarity(
    ST_ISP_MOD_CBAR *pstModCBar,
    STF_BOOL8 bSensorHorSyncPolarity,
    STF_BOOL8 bSensorVerSyncPolarity
    );
extern
STF_S32 STFMOD_ISP_CBAR_SetColorBarSize(
    ST_ISP_MOD_CBAR *pstModCBar,
    ST_SIZE stColorBarSize
    );
extern
STF_S32 STFMOD_ISP_CBAR_SetColorBarFrameWindow(
    ST_ISP_MOD_CBAR *pstModCBar,
    ST_RECT stFrameWindow
    );
extern
STF_S32 STFMOD_ISP_CBAR_SetColorBarMode(
    ST_ISP_MOD_CBAR *pstModCBar,
    ST_CBAR_MODE *pstMode
    );
extern
STF_S32 STFMOD_ISP_CBAR_SetDvpMuxEnable(
    ST_ISP_MOD_CBAR *pstModCBar,
    STF_BOOL8 bEnable
    );
extern
STF_S32 STFMOD_ISP_CBAR_SetMipiMuxEnable(
    ST_ISP_MOD_CBAR *pstModCBar,
    STF_BOOL8 bEnable
    );
extern
STF_S32 STFMOD_ISP_CBAR_SetSyncCsi(
    ST_ISP_MOD_CBAR *pstModCBar,
    STF_BOOL8 bSyncCsi
    );
extern
STF_S32 STFMOD_ISP_CBAR_SetHorSyncPolarity(
    ST_ISP_MOD_CBAR *pstModCBar,
    STF_BOOL8 bHorBlankingPolarity
    );
extern
STF_S32 STFMOD_ISP_CBAR_SetVerSyncPolarity(
    ST_ISP_MOD_CBAR *pstModCBar,
    STF_BOOL8 bVerBlankingPolarity
    );
extern
STF_S32 STFMOD_ISP_CBAR_SetPatternType(
    ST_ISP_MOD_CBAR *pstModCBar,
    STF_U8 u8PatternType
    );
extern
STF_S32 STFMOD_ISP_CBAR_SetHorBarWidth(
    ST_ISP_MOD_CBAR *pstModCBar,
    STF_U8 u8HorBarWidth
    );
extern
STF_S32 STFMOD_ISP_CBAR_SetVerBarWidth(
    ST_ISP_MOD_CBAR *pstModCBar,
    STF_U8 u8VerBarWidth
    );
extern
STF_S32 STFMOD_ISP_CBAR_SetMode1(
    ST_ISP_MOD_CBAR *pstModCBar,
    STF_U32 u32Mode1
    );
extern
STF_S32 STFMOD_ISP_CBAR_SetPixelValue(
    ST_ISP_MOD_CBAR *pstModCBar,
    ST_CBAR_PXL_VAL stLinePixelValue[2]
    );
extern
STF_S32 STFMOD_ISP_CBAR_SetLine0PixelValue(
    ST_ISP_MOD_CBAR *pstModCBar,
    ST_CBAR_PXL_VAL *pstLinePixelValue
    );
extern
STF_S32 STFMOD_ISP_CBAR_SetLine1PixelValue(
    ST_ISP_MOD_CBAR *pstModCBar,
    ST_CBAR_PXL_VAL *pstLinePixelValue
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_CBAR_Init(
    STF_VOID *pIspCtx,
    STF_U32 *pNextRdma
    );
extern
STF_S32 STFMOD_ISP_CBAR_LoadBinParam(
    STF_VOID *pIspCtx,
    STF_U8 *pu8ParamBuf,
    STF_U16 u16ParamSize
    );
extern
STF_S32 STFMOD_ISP_CBAR_LoadBinClbrt(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_CBAR_Enable(
    STF_VOID *pIspCtx,
    STF_BOOL8 bEnable
    );
extern
STF_BOOL8 STFMOD_ISP_CBAR_IsEnable(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_CBAR_Update(
    STF_VOID *pIspCtx,
    STF_BOOL8 bUpdate
    );
extern
STF_BOOL8 STFMOD_ISP_CBAR_IsUpdate(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_CBAR_Registered(
    STF_VOID *pIspCtx,
    STF_BOOL8 bRegistered
    );
extern
STF_BOOL8 STFMOD_ISP_CBAR_IsRegistered(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_CBAR_UpdateReg(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_CBAR_SetReg(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_CBAR_SetNext(
    STF_VOID *pIspCtx,
    STF_U32 *pNextRdma
    );
extern
STF_S32 STFMOD_ISP_CBAR_FreeResource(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_CBAR_GetIqParam(
    STF_VOID *pIspCtx,
    STF_VOID *pParamBuf,
    STF_U16 *pu16ParamSize
    );
extern
STF_S32 STFMOD_ISP_CBAR_SetIqParam(
    STF_VOID *pIspCtx,
    STF_VOID *pParamBuf,
    STF_U16 u16ParamSize
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_CBAR_Create(
    STF_VOID *pPipeline
    );
extern
STF_S32 STFMOD_ISP_CBAR_Destroy(
    STF_VOID *pPipeline
    );
//-----------------------------------------------------------------------------


#ifdef __cplusplus
}
#endif


#endif //__STFMOD_ISP_CBAR_H__
