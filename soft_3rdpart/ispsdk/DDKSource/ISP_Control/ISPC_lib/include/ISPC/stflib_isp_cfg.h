/**
  ******************************************************************************
  * @file  stflib_isp_cfg.h
  * @author  StarFive Isp Team
  * @version  V1.0
  * @date  07/19/2022
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


#ifndef __STFLIB_ISP_CFG_H__
#define __STFLIB_ISP_CFG_H__


#ifdef __cplusplus
extern "C" {
#endif


#include "stf_include.h"


#define STF_ISP_PARAM_LEN           (256)
#define STF_ISP_PARAM_MAX           (32)


/* ISP configuration library structure */
typedef enum _EN_SENSOR_ID {
    EN_SENSOR_ID_SC2235_DVP = 0,
    EN_SENSOR_ID_OV4689_MIPI,
    EN_SENSOR_ID_IMX219_MIPI,
} EN_SENSOR_ID, *PEN_SENSOR_ID;
#define EN_SENSOR_ID_MAX            (EN_SENSOR_ID_IMX219_MIPI + 1)

typedef enum _EN_ISP_CFG_PARAM_TYPE {
    EN_ISP_CFG_PARAM_TYPE_BOOL = 0,
    EN_ISP_CFG_PARAM_TYPE_INT,
    EN_ISP_CFG_PARAM_TYPE_UINT,
    EN_ISP_CFG_PARAM_TYPE_INT64,
    EN_ISP_CFG_PARAM_TYPE_UINT64,
    EN_ISP_CFG_PARAM_TYPE_FLOAT,
    EN_ISP_CFG_PARAM_TYPE_DOUBLE,
    EN_ISP_CFG_PARAM_TYPE_STR,
    EN_ISP_CFG_PARAM_TYPE_MAX,
} EN_ISP_CFG_PARAM_TYPE, *PEN_ISP_CFG_PARAM_TYPE;

typedef enum _EN_ISP_PARAM_ITEM {
    EN_ISP_PARAM_ITEM_SENSOR_0 = 0,
    EN_ISP_PARAM_ITEM_SENSOR_1,
    EN_ISP_PARAM_ITEM_DVP,
    EN_ISP_PARAM_ITEM_CSI_0,
    EN_ISP_PARAM_ITEM_CSI_1,
    EN_ISP_PARAM_ITEM_ISP_0,
    EN_ISP_PARAM_ITEM_ISP_1,
    EN_ISP_PARAM_ITEM_ISP_0_WIDTH,
    EN_ISP_PARAM_ITEM_ISP_1_WIDTH,
    EN_ISP_PARAM_ITEM_ISP_0_HEIGHT,
    EN_ISP_PARAM_ITEM_ISP_1_HEIGHT,
    EN_ISP_PARAM_ITEM_ISP_0_AUTO,
    EN_ISP_PARAM_ITEM_ISP_1_AUTO,
    EN_ISP_PARAM_ITEM_MAX,
} EN_ISP_PARAM_ITEM, *PEN_ISP_API_PARAM_ITEM;


typedef struct _ST_CFG_PARAM {
    STF_CHAR szName[STF_ISP_PARAM_LEN];
    STF_CHAR szValue[STF_ISP_PARAM_LEN];
    EN_ISP_CFG_PARAM_TYPE enType;
} ST_CFG_PARAM, *PST_CFG_PARAM;

typedef struct _ST_ISP_CFG_PARAMS {
    STF_INT nCount;
    ST_CFG_PARAM stParam[STF_ISP_PARAM_MAX];
} ST_ISP_CFG_PARAMS, *PST_ISP_CFG_PARAMS;

typedef struct _ST_ISP_PARAMS {
    STF_CHAR szSensor[2][STF_ISP_PARAM_LEN];
    STF_INT nSensorInterface[3];
    STF_INT nISP[2];
    STF_CHAR szConfigFile[STF_ISP_PARAM_LEN];
    STF_INT nIspWidth[2];
    STF_INT nIspHeight[2];
    STF_BOOL8 bIspAutoDetect[2];
} ST_ISP_PARAMS, *PST_ISP_PARAMS;


/* ISP configuration library variables */
extern const STF_CHAR g_szSENSOR_NAME[EN_SENSOR_ID_MAX][24];
#define SENSOR_NAME_ITEM_MAX        (sizeof(g_szSENSOR_NAME) / sizeof(g_szSENSOR_NAME[0]))


/* ISP configuration library interface */
//-----------------------------------------------------------------------------
extern
STF_CHAR *STFLIB_ISP_CFG_ToLower(
    STF_CHAR *s
    );

extern
STF_CHAR *STFLIB_ISP_CFG_ToUpper(
    STF_CHAR *s
    );

extern
STF_CHAR *STFLIB_ISP_CFG_StrTrim(
    STF_CHAR *pszString
    );

extern
STF_RESULT STFLIB_ISP_CFG_ConvertParamValue(
    ST_CFG_PARAM *pstCfgParam,
    STF_VOID *pvResult
    );

extern
STF_VOID STFLIB_ISP_CFG_ParseConfig(
    STF_CHAR *pszConfigFile,
    ST_ISP_CFG_PARAMS *pstCfgParams,
    ST_ISP_PARAMS *pstParams
    );

extern
STF_VOID DumpConfigParam(
    ST_ISP_PARAMS *pstIspParams
    );

//-----------------------------------------------------------------------------


#ifdef __cplusplus
}
#endif


#endif //__STFLIB_ISP_CFG_H__
