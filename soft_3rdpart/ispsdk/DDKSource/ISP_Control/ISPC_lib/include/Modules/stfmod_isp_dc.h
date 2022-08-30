/**
  ******************************************************************************
  * @file  stfmod_isp_dc.h
  * @author  StarFive Isp Team
  * @version  V1.0
  * @date  05/26/2020
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


#ifndef __STFMOD_ISP_DC_H__
#define __STFMOD_ISP_DC_H__


#ifdef __cplusplus
extern "C" {
#endif


#include "stf_include.h"
#include "ci/ci_api_structs.h"
#include "registers/stf_isp_dc_rdma.h"
#include "stflib_isp_pipeline.h"


/* defective pixel correction module parameters structure */
//-----------------------------------------------------------------------------
// Define initial values
#define ISP_DEF_DC_BUFFER_ADDRESS               (0x68400000)    /** Point to defective pixel correction parameters buffer. */
#if 0
#define ISP_DEF_DC_FILENAME                     "dc_data.bin"   /** DC calibration data filename. */
#else
#define ISP_DEF_DC_FILENAME                     ""              /** DC calibration data filename. */
#endif
#define ISP_DEF_DC_AXI_ID                       (0)             /** ID of AXI bus for DC buffer read. */


#pragma pack(push, 1)

typedef enum _EN_DC_UPDATE {
    EN_DC_UPDATE_NONE = 0x00,
    EN_DC_UPDATE_AXI_ID = 0x01,
    EN_DC_UPDATE_DC_BUF = 0x02,
    EN_DC_UPDATE_ALL_WO_BUF_INFO = (EN_DC_UPDATE_AXI_ID),
    EN_DC_UPDATE_ALL = (EN_DC_UPDATE_AXI_ID
                      | EN_DC_UPDATE_DC_BUF
                      ),
} EN_DC_UPDATE, *PEN_DC_UPDATE;


typedef union _ST_DC_CLBRT_INFO {
    STF_U32 u32ClbrtInfo;
    struct {
        STF_U32 u14HPos:14;                     /** Horizontal position */
        STF_U32 u2Mode:2;                       /** Calibration mode,
                                                 *      0 : none.
                                                 *      1 : left mode.
                                                 *      2 : right mode.
                                                 *      3 : average mode.
                                                 */
        STF_U32 u14VPos:14;                     /** Vertical position */
        STF_U32 u2Rsvd30:2;                     /** Reserved. */
    };
} ST_DC_CLBRT_INFO, *PST_DC_CLBRT_INFO;

typedef struct _ST_MOD_DC_SETTING {
    //-------------------------------------------------------------------------
    // This section parameters value is assign from setting file.
    STF_BOOL8 bEnable;                          /** Enable/Disable DC module. */
    STF_CHAR szDcFilename[256];                 /** Filename of parameters of defective pixel correction. */
} ST_MOD_DC_SETTING, *PST_MOD_DC_SETTING;

typedef struct _ST_MOD_DC_INFO {
    STF_U8 u8AxiId;                             /** ID of AXI bus for DC buffer read. */
} ST_MOD_DC_INFO, *PST_MOD_DC_INFO;

typedef struct _ST_DC_PARAM {
    ST_MOD_DC_SETTING stSetting;                /** Module DC setting file parameters. */
    //-------------------------------------------------------------------------
    // This section parameters is internal using.
    ST_MOD_DC_INFO stInfo;                      /** Module DC output information parameters. */
    STF_BOOL8 bSetup;                           /** Flag set if the module setup has been completed. */
    STF_BOOL8 bUpdate;                          /** Enable/Disable program DC module register. */
    STF_U32 u32UpdateRequest;                   /** Flag to indicate that this module update has been requested. */
#if 0
    STF_U32 u32RetDcBufAddr;                    /** Hardware return defective pixel correction parameters buffer address. (Read Only) */
#endif
    STF_U32 *pu32DcBuf;                         /** Defective pixel correction parameters buffer. */
#if defined(VIRTUAL_IO_MAPPING)
    CI_MEM_PARAM stDcMem;                       /** Defective pixel correction parameters buffer. */
#endif //VIRTUAL_IO_MAPPING
} ST_DC_PARAM, *PST_DC_PARAM;

#define ST_MOD_DC_SETTING_LEN           (sizeof(ST_MOD_DC_SETTING))
#define ST_MOD_DC_INFO_LEN              (sizeof(ST_MOD_DC_INFO))
#define ST_MOD_DC_GET_PARAM_LEN         (ST_MOD_DC_SETTING_LEN + ST_MOD_DC_INFO_LEN)
#define ST_MOD_DC_SET_PARAM_LEN         (ST_MOD_DC_SETTING_LEN)
//#if defined(VIRTUAL_IO_MAPPING)
//#define ST_DC_PARAM_SIZE                (sizeof(ST_DC_PARAM) - sizeof(STF_U32 *) - sizeof(CI_MEM_PARAM))
//#else
//#define ST_DC_PARAM_SIZE                (sizeof(ST_DC_PARAM) - sizeof(STF_U32 *))
//#endif //VIRTUAL_IO_MAPPING
#define ST_DC_PARAM_SIZE                (sizeof(ST_DC_PARAM))
#define ST_DC_PARAM_LEN                 (ST_DC_PARAM_SIZE)

typedef struct _ST_ISP_MOD_DC {
#if defined(VIRTUAL_IO_MAPPING)
    STF_VOID *pPipeline;                        /** Pointer to Pipeline. */
#endif //VIRTUAL_IO_MAPPING
    STF_VOID *pIspCtx;                          /** Pointer to ISP context. */
    STF_U32 u32Id;                              /** Module ID. */
    STF_CHAR szName[16];                        /** Module Name. */
    CI_MEM_PARAM *pstDcRdmaBuf;                 /** DC's DMA buffer pointer for RDMA. */
#if defined(SUPPORT_MULTI_ISP)
    ST_ISP_DC_RDMA *pstIspDcRdma;               /** DC's RDMA structure pointer that include ISP index.*/
#endif //SUPPORT_MULTI_ISP
    ST_DC_RDMA *pstDcRdma;                      /** DC's RDMA structure pointer.*/
    ST_DC_PARAM *pstDcParam;                    /** DC's parameters structure pointer. */
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
} ST_ISP_MOD_DC, *PST_ISP_MOD_DC;

#pragma pack(pop)


/* defective pixel correction module parameters interface */
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_DC_InitRdma(
    ST_DC_RDMA *pstDcRdma,
    ST_DC_PARAM *pstDcParam,
    STF_U32 *pNextRdma
    );
extern
STF_S32 STFMOD_ISP_DC_UpdateRdma(
    ST_DC_RDMA *pstDcRdma,
    ST_DC_PARAM *pstDcParam
    );
extern
STF_S32 STFMOD_ISP_DC_UpdateBufferAddress(
    ST_DC_RDMA *pstDcRdma,
    ST_DC_PARAM *pstDcParam
    );
extern
STF_S32 STFMOD_ISP_DC_UpdateAxiId(
    ST_DC_RDMA *pstDcRdma,
    ST_DC_PARAM *pstDcParam
    );
extern
STF_S32 STFMOD_ISP_DC_SetNextRdma(
    ST_DC_RDMA *pstDcRdma,
    STF_U32 *pNextRdma
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_DC_SetEnable(
    ST_ISP_MOD_DC *pstModDc,
    STF_BOOL8 bEnable
    );
extern
STF_S32 STFMOD_ISP_DC_SetUpdate(
    ST_ISP_MOD_DC *pstModDc,
    STF_BOOL8 bUpdate
    );
extern
STF_S32 STFMOD_ISP_DC_SetDcFilename(
    ST_ISP_MOD_DC *pstModDc,
    STF_CHAR *pszDcFilename
    );
extern
STF_S32 STFMOD_ISP_DC_SetDcBuffer(
    ST_ISP_MOD_DC *pstModDc,
    STF_U32 *pu32DcBuf
    );
extern
STF_S32 STFMOD_ISP_DC_SetAxiId(
    ST_ISP_MOD_DC *pstModDc,
    STF_U8 u8AxiId
    );
extern
STF_S32 STFMOD_ISP_DC_LoadDcCalibrationFile(
    ST_ISP_MOD_DC *pstModDc
    );
extern
STF_S32 STFMOD_ISP_DC_GetDcBufffer(
    ST_ISP_MOD_DC *pstModDc,
    STF_U32 **ppu32DcBuf
    );
#if 0
extern
STF_S32 STFMOD_ISP_DC_GetRetBufAddr(
    ST_ISP_MOD_DC *pstModDc,
    STF_U32 *pu32RetDcBufAddr
    );
#endif
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_DC_Init(
    STF_VOID *pIspCtx,
    STF_U32 *pNextRdma
    );
extern
STF_S32 STFMOD_ISP_DC_LoadBinParam(
    STF_VOID *pIspCtx,
    STF_U8 *pu8ParamBuf,
    STF_U16 u16ParamSize
    );
extern
STF_S32 STFMOD_ISP_DC_LoadBinClbrt(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_DC_Enable(
    STF_VOID *pIspCtx,
    STF_BOOL8 bEnable
    );
extern
STF_BOOL8 STFMOD_ISP_DC_IsEnable(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_DC_Update(
    STF_VOID *pIspCtx,
    STF_BOOL8 bUpdate
    );
extern
STF_BOOL8 STFMOD_ISP_DC_IsUpdate(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_DC_Registered(
    STF_VOID *pIspCtx,
    STF_BOOL8 bRegistered
    );
extern
STF_BOOL8 STFMOD_ISP_DC_IsRegistered(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_DC_UpdateReg(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_DC_SetReg(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_DC_SetNext(
    STF_VOID *pIspCtx,
    STF_U32 *pNextRdma
    );
extern
STF_S32 STFMOD_ISP_DC_FreeResource(
    STF_VOID *pIspCtx
    );
extern
STF_S32 STFMOD_ISP_DC_GetIqParam(
    STF_VOID *pIspCtx,
    STF_VOID *pParamBuf,
    STF_U16 *pu16ParamSize
    );
extern
STF_S32 STFMOD_ISP_DC_SetIqParam(
    STF_VOID *pIspCtx,
    STF_VOID *pParamBuf,
    STF_U16 u16ParamSize
    );
//-----------------------------------------------------------------------------
extern
STF_S32 STFMOD_ISP_DC_Create(
    STF_VOID *pPipeline
    );
extern
STF_S32 STFMOD_ISP_DC_Destroy(
    STF_VOID *pPipeline
    );
//-----------------------------------------------------------------------------


#ifdef __cplusplus
}
#endif


#endif //__STFMOD_ISP_DC_H__
