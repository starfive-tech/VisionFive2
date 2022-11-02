/**
  ******************************************************************************
  * @file  stflib_isp_device.h
  * @author  StarFive Isp Team
  * @version  V1.0
  * @date  05/26/2020
  * @brief Declaration of ISPC::Device which represents a connection to the
  * CI kernel driver
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


#ifndef __STFLIB_ISP_DEVICE_H__
#define __STFLIB_ISP_DEVICE_H__


#ifdef __cplusplus
extern "C" {
#endif


#include "stf_include.h"
#include "ci/ci_api_structs.h"
#if defined(V4L2_DRIVER)

#include <errno.h>
#include <linux/media.h>
#include <linux/types.h>
#include <linux/v4l2-mediabus.h>
#include <linux/v4l2-subdev.h>
#include <linux/videodev2.h>
#include <linux/fb.h>
#include <stdbool.h>
  //#if defined(ENABLE_DRM_METHOD)
#include <fcntl.h>
#include <libdrm/drm.h>
#include <libdrm/drm_fourcc.h>
#include <xf86drm.h>
#include <xf86drmMode.h>
  //#endif //#if defined(ENABLE_DRM_METHOD)
#include "linkedlist.h"
#endif //#if defined(V4L2_DRIVER)


#define DEVIVE_SUPPORT_GET_FILE_HANDLE
#define DEV_NAME_LEN_MAX                        (32)
#define VDO_BUF_MAX                             (256)
#define ITEM_INFO_MAX                           (20)
#define BAYER_FORMAT_MAX                        (4)


#if defined(V4L2_DRIVER)
typedef enum _EN_DEV_ID {
#if defined(OLD_ISP_VIDEO_DEVICE_NO_ORDER)
    EN_DEV_ID_DVP = 0,                          // 00
    EN_DEV_ID_CSIPHY_0,                         // 01
    EN_DEV_ID_CSIPHY_1,                         // 02
    EN_DEV_ID_CSI_0,                            // 03
    EN_DEV_ID_CSI_1,                            // 04
    EN_DEV_ID_ISP_0,                            // 05
  #if defined(CONFIG_STF_DUAL_ISP)
    EN_DEV_ID_ISP_1,                            // 06
  #endif //#if defined(CONFIG_STF_DUAL_ISP)
    EN_DEV_ID_VIN_WR,                           // 07
    EN_DEV_ID_VIN_ISP_0_UO,                     // 08
  #if defined(CONFIG_STF_DUAL_ISP)
    EN_DEV_ID_VIN_ISP_1_UO,                     // 09
  #endif //#if defined(CONFIG_STF_DUAL_ISP)
    EN_DEV_ID_VIN_ISP_0_SS0,                    // 10
  #if defined(CONFIG_STF_DUAL_ISP)
    EN_DEV_ID_VIN_ISP_1_SS0,                    // 11
  #endif //#if defined(CONFIG_STF_DUAL_ISP)
    EN_DEV_ID_VIN_ISP_0_SS1,                    // 12
  #if defined(CONFIG_STF_DUAL_ISP)
    EN_DEV_ID_VIN_ISP_1_SS1,                    // 13
  #endif //#if defined(CONFIG_STF_DUAL_ISP)
    EN_DEV_ID_VIN_ISP_0_ITI_WR,                 // 14
  #if defined(CONFIG_STF_DUAL_ISP)
    EN_DEV_ID_VIN_ISP_1_ITI_WR,                 // 15
  #endif //#if defined(CONFIG_STF_DUAL_ISP)
    EN_DEV_ID_VIN_ISP_0_ITI_RD,                 // 16
  #if defined(CONFIG_STF_DUAL_ISP)
    EN_DEV_ID_VIN_ISP_1_ITI_RD,                 // 17
  #endif //#if defined(CONFIG_STF_DUAL_ISP)
    EN_DEV_ID_VIN_ISP_0_DUMP,                   // 18
  #if defined(CONFIG_STF_DUAL_ISP)
    EN_DEV_ID_VIN_ISP_1_DUMP,                   // 19
  #endif //#if defined(CONFIG_STF_DUAL_ISP)
    EN_DEV_ID_VIN_ISP_0_SC_DUMP_Y_HIST,         // 20
  #if defined(CONFIG_STF_DUAL_ISP)
    EN_DEV_ID_VIN_ISP_1_SC_DUMP_Y_HIST,         // 21
  #endif //#if defined(CONFIG_STF_DUAL_ISP)
    EN_DEV_ID_SNR_MIPI_0,                       // 22
    EN_DEV_ID_SNR_MIPI_1,                       // 23
    EN_DEV_ID_SNR_DVP_0,                        // 24

    EN_DEV_ID_MEDIA_0,                          // 25
    EN_DEV_ID_FB,                               // 26
    EN_DEV_ID_PP,                               // 27
    EN_DEV_ID_DRM,                              // 28

    EN_DEV_ID_WR,                               // 29
    EN_DEV_ID_ISP_0_UO,                         // 30
  #if defined(CONFIG_STF_DUAL_ISP)
    EN_DEV_ID_ISP_1_UO,                         // 31
  #endif //#if defined(CONFIG_STF_DUAL_ISP)
    EN_DEV_ID_ISP_0_SS0,                        // 32
  #if defined(CONFIG_STF_DUAL_ISP)
    EN_DEV_ID_ISP_1_SS0,                        // 33
  #endif //#if defined(CONFIG_STF_DUAL_ISP)
    EN_DEV_ID_ISP_0_SS1,                        // 34
  #if defined(CONFIG_STF_DUAL_ISP)
    EN_DEV_ID_ISP_1_SS1,                        // 35
  #endif //#if defined(CONFIG_STF_DUAL_ISP)
    EN_DEV_ID_ISP_0_TIL_1_WR,                   // 36
  #if defined(CONFIG_STF_DUAL_ISP)
    EN_DEV_ID_ISP_1_TIL_1_WR,                   // 37
  #endif //#if defined(CONFIG_STF_DUAL_ISP)
    EN_DEV_ID_ISP_0_TIL_1_RD,                   // 38
  #if defined(CONFIG_STF_DUAL_ISP)
    EN_DEV_ID_ISP_1_TIL_1_RD,                   // 39
  #endif //#if defined(CONFIG_STF_DUAL_ISP)
    EN_DEV_ID_ISP_0_DUMP,                       // 40
  #if defined(CONFIG_STF_DUAL_ISP)
    EN_DEV_ID_ISP_1_DUMP,                       // 41
  #endif //#if defined(CONFIG_STF_DUAL_ISP)
    EN_DEV_ID_ISP_0_SC_DUMP_Y_HIST,             // 42
  #if defined(CONFIG_STF_DUAL_ISP)
    EN_DEV_ID_ISP_1_SC_DUMP_Y_HIST,             // 43
  #endif //#if defined(CONFIG_STF_DUAL_ISP)
#else
    EN_DEV_ID_DVP = 0,                          // 00
    EN_DEV_ID_CSIPHY_0,                         // 01
    EN_DEV_ID_CSIPHY_1,                         // 02
    EN_DEV_ID_CSI_0,                            // 03
    EN_DEV_ID_CSI_1,                            // 04
    EN_DEV_ID_ISP_0,                            // 05
  #if defined(CONFIG_STF_DUAL_ISP)
    EN_DEV_ID_ISP_1,                            // 06
  #endif //#if defined(CONFIG_STF_DUAL_ISP)
    EN_DEV_ID_VIN_WR,                           // 07
    EN_DEV_ID_VIN_ISP_0_UO,                     // 08
    EN_DEV_ID_VIN_ISP_0_SS0,                    // 09
    EN_DEV_ID_VIN_ISP_0_SS1,                    // 10
    EN_DEV_ID_VIN_ISP_0_ITI_WR,                 // 11
    EN_DEV_ID_VIN_ISP_0_ITI_RD,                 // 12
    EN_DEV_ID_VIN_ISP_0_DUMP,                   // 13
    EN_DEV_ID_VIN_ISP_0_SC_DUMP_Y_HIST,         // 14
  #if defined(CONFIG_STF_DUAL_ISP)
    EN_DEV_ID_VIN_ISP_1_UO,                     // 15
    EN_DEV_ID_VIN_ISP_1_SS0,                    // 16
    EN_DEV_ID_VIN_ISP_1_SS1,                    // 17
    EN_DEV_ID_VIN_ISP_1_ITI_WR,                 // 18
    EN_DEV_ID_VIN_ISP_1_ITI_RD,                 // 19
    EN_DEV_ID_VIN_ISP_1_DUMP,                   // 20
    EN_DEV_ID_VIN_ISP_1_SC_DUMP_Y_HIST,         // 21
  #endif //#if defined(CONFIG_STF_DUAL_ISP)
    EN_DEV_ID_SNR_MIPI_0,                       // 22
    EN_DEV_ID_SNR_MIPI_1,                       // 23
    EN_DEV_ID_SNR_DVP_0,                        // 24

    EN_DEV_ID_MEDIA_0,                          // 25
    EN_DEV_ID_FB,                               // 26
    EN_DEV_ID_PP,                               // 27
    EN_DEV_ID_DRM,                              // 28

    EN_DEV_ID_WR,                               // 29
    EN_DEV_ID_ISP_0_UO,                         // 30
    EN_DEV_ID_ISP_0_SS0,                        // 31
    EN_DEV_ID_ISP_0_SS1,                        // 32
    EN_DEV_ID_ISP_0_TIL_1_WR,                   // 33
    EN_DEV_ID_ISP_0_TIL_1_RD,                   // 34
    EN_DEV_ID_ISP_0_DUMP,                       // 35
    EN_DEV_ID_ISP_0_SC_DUMP_Y_HIST,             // 36
  #if defined(CONFIG_STF_DUAL_ISP)
    EN_DEV_ID_ISP_1_UO,                         // 37
    EN_DEV_ID_ISP_1_SS0,                        // 38
    EN_DEV_ID_ISP_1_SS1,                        // 39
    EN_DEV_ID_ISP_1_TIL_1_WR,                   // 40
    EN_DEV_ID_ISP_1_TIL_1_RD,                   // 41
    EN_DEV_ID_ISP_1_DUMP,                       // 42
    EN_DEV_ID_ISP_1_SC_DUMP_Y_HIST,             // 43
  #endif //#if defined(CONFIG_STF_DUAL_ISP)
#endif //#if defined(OLD_ISP_VIDEO_DEVICE_NO_ORDER)
} EN_DEV_ID, *PEN_DEV_ID;
#if defined(CONFIG_STF_DUAL_ISP)
#define EN_DEV_ID_MAX                           (EN_DEV_ID_ISP_1_SC_DUMP_Y_HIST + 1)
#else
#define EN_DEV_ID_MAX                           (EN_DEV_ID_ISP_0_SC_DUMP_Y_HIST + 1)
#endif //#if defined(CONFIG_STF_DUAL_ISP)

typedef enum _EN_VDEV_ID {
#if defined(OLD_ISP_VIDEO_DEVICE_NO_ORDER)
    EN_VDEV_ID_WR,                              // 00
    EN_VDEV_ID_ISP_0_UO,                        // 01
  #if defined(CONFIG_STF_DUAL_ISP)
    EN_VDEV_ID_ISP_1_UO,                        // 02
  #endif //#if defined(CONFIG_STF_DUAL_ISP)
    EN_VDEV_ID_ISP_0_SS0,                       // 03
  #if defined(CONFIG_STF_DUAL_ISP)
    EN_VDEV_ID_ISP_1_SS0,                       // 04
  #endif //#if defined(CONFIG_STF_DUAL_ISP)
    EN_VDEV_ID_ISP_0_SS1,                       // 05
  #if defined(CONFIG_STF_DUAL_ISP)
    EN_VDEV_ID_ISP_1_SS1,                       // 06
  #endif //#if defined(CONFIG_STF_DUAL_ISP)
    EN_VDEV_ID_ISP_0_TIL_1_WR,                  // 07
  #if defined(CONFIG_STF_DUAL_ISP)
    EN_VDEV_ID_ISP_1_TIL_1_WR,                  // 08
  #endif //#if defined(CONFIG_STF_DUAL_ISP)
    EN_VDEV_ID_ISP_0_TIL_1_RD,                  // 09
  #if defined(CONFIG_STF_DUAL_ISP)
    EN_VDEV_ID_ISP_1_TIL_1_RD,                  // 10
  #endif //#if defined(CONFIG_STF_DUAL_ISP)
    EN_VDEV_ID_ISP_0_DUMP,                      // 11
  #if defined(CONFIG_STF_DUAL_ISP)
    EN_VDEV_ID_ISP_1_DUMP,                      // 12
  #endif //#if defined(CONFIG_STF_DUAL_ISP)
    EN_VDEV_ID_ISP_0_SC_DUMP_Y_HIST,            // 13
  #if defined(CONFIG_STF_DUAL_ISP)
    EN_VDEV_ID_ISP_1_SC_DUMP_Y_HIST,            // 14
  #endif //#if defined(CONFIG_STF_DUAL_ISP)
#else
    EN_VDEV_ID_WR,                              // 00
    EN_VDEV_ID_ISP_0_UO,                        // 01
    EN_VDEV_ID_ISP_0_SS0,                       // 02
    EN_VDEV_ID_ISP_0_SS1,                       // 03
    EN_VDEV_ID_ISP_0_TIL_1_WR,                  // 04
    EN_VDEV_ID_ISP_0_TIL_1_RD,                  // 05
    EN_VDEV_ID_ISP_0_DUMP,                      // 06
    EN_VDEV_ID_ISP_0_SC_DUMP_Y_HIST,            // 07
  #if defined(CONFIG_STF_DUAL_ISP)
    EN_VDEV_ID_ISP_1_UO,                        // 08
    EN_VDEV_ID_ISP_1_SS0,                       // 09
    EN_VDEV_ID_ISP_1_SS1,                       // 10
    EN_VDEV_ID_ISP_1_TIL_1_WR,                  // 11
    EN_VDEV_ID_ISP_1_TIL_1_RD,                  // 12
    EN_VDEV_ID_ISP_1_DUMP,                      // 13
    EN_VDEV_ID_ISP_1_SC_DUMP_Y_HIST,            // 14
  #endif //#if defined(CONFIG_STF_DUAL_ISP)
#endif //#if defined(OLD_ISP_VIDEO_DEVICE_NO_ORDER)
} EN_VDEV_ID, *PEN_VDEV_ID;
#if defined(CONFIG_STF_DUAL_ISP)
#define EN_VDEV_ID_MAX                          (EN_VDEV_ID_ISP_1_SC_DUMP_Y_HIST + 1)
#else
#define EN_VDEV_ID_MAX                          (EN_VDEV_ID_ISP_0_SC_DUMP_Y_HIST + 1)
#endif //#if defined(CONFIG_STF_DUAL_ISP)

typedef enum _EN_SNSR_IF {
    EN_SNSR_IF_DVP = 0,
    EN_SNSR_IF_CSI_0,
    EN_SNSR_IF_CSI_1
} EN_SNSR_IF, *PEN_SNSR_IF;
#define EN_SNSR_IF_MAX                          (EN_SNSR_IF_CSI_1 + 1)

typedef enum _EN_ISP_DEV_IDX {
    EN_ISP_DEV_IDX_0 = 0,                       // 00
#if defined(CONFIG_STF_DUAL_ISP)
    EN_ISP_DEV_IDX_1,                           // 01
#endif //#if defined(CONFIG_STF_DUAL_ISP)
} EN_ISP_DEV_IDX, *PEN_ISP_DEV_IDX;
#if defined(CONFIG_STF_DUAL_ISP)
#define EN_ISP_DEV_IDX_MAX                      (EN_ISP_DEV_IDX_1 + 1)
#else
#define EN_ISP_DEV_IDX_MAX                      (EN_ISP_DEV_IDX_0 + 1)
#endif //#if defined(CONFIG_STF_DUAL_ISP)

typedef enum _EN_ISP_PORT_IDX {
    EN_ISP_PORT_IDX_UO = 0,
    EN_ISP_PORT_IDX_SS0,
    EN_ISP_PORT_IDX_SS1,
    EN_ISP_PORT_IDX_TIL_1_WR,
    EN_ISP_PORT_IDX_TIL_1_RD,
    EN_ISP_PORT_IDX_DUMP,
    EN_ISP_PORT_IDX_SC_DUMP_Y_HIST,
} EN_ISP_PORT_IDX, *PEN_ISP_PORT_IDX;
#define EN_ISP_PORT_IDX_MAX                     (EN_ISP_PORT_IDX_SC_DUMP_Y_HIST + 1)

typedef enum _EN_ISP_PORT {
    EN_ISP_PORT_NONE           = 0x0000,
    EN_ISP_PORT_UO             = (1 << EN_ISP_PORT_IDX_UO),
    EN_ISP_PORT_SS0            = (1 << EN_ISP_PORT_IDX_SS0),
    EN_ISP_PORT_SS1            = (1 << EN_ISP_PORT_IDX_SS1),
    EN_ISP_PORT_TIL_1_WR       = (1 << EN_ISP_PORT_IDX_TIL_1_WR),
    EN_ISP_PORT_TIL_1_RD       = (1 << EN_ISP_PORT_IDX_TIL_1_RD),
    EN_ISP_PORT_DUMP           = (1 << EN_ISP_PORT_IDX_DUMP),
    EN_ISP_PORT_SC_DUMP_Y_HIST = (1 << EN_ISP_PORT_IDX_SC_DUMP_Y_HIST),
} EN_ISP_PORT, *PEN_ISP_PORT;

typedef enum _EN_DEV_TYPE {
    EN_DEV_TYPE_NONE        = 0x0000,
    EN_DEV_TYPE_V4L_SUB_DEV = 0x0001,
    EN_DEV_TYPE_ISP         = EN_DEV_TYPE_V4L_SUB_DEV << 1,
    EN_DEV_TYPE_VIDEO       = EN_DEV_TYPE_ISP << 1,
    EN_DEV_TYPE_MEDIA       = EN_DEV_TYPE_VIDEO << 1,
    EN_DEV_TYPE_FB          = EN_DEV_TYPE_MEDIA << 1,
    EN_DEV_TYPE_PP          = EN_DEV_TYPE_FB << 1,
    EN_DEV_TYPE_DRM         = EN_DEV_TYPE_PP << 1,
} EN_DEV_TYPE, *PEN_DEV_TYPE;

typedef enum _EN_MEM_TYPE {
    EN_MEM_TYPE_NONE = 0,
    EN_MEM_TYPE_MMAP,
    EN_MEM_TYPE_DMA,
} EN_MEM_TYPE, *PEN_MEM_TYPE;
#define EN_MEM_TYPE_MAX                         (EN_MEM_TYPE_DMA + 1)

typedef enum _EN_PXL_FMT {
    EN_PXL_FMT_COLOR_YUV422_UYVY = 0,   // 00={Y1,V0,Y0,U0}
    EN_PXL_FMT_COLOR_YUV422_VYUY = 1,   // 01={Y1,U0,Y0,V0}
    EN_PXL_FMT_COLOR_YUV422_YUYV = 2,   // 10={V0,Y1,U0,Y0}
    EN_PXL_FMT_COLOR_YUV422_YVYU = 3,   // 11={U0,Y1,V0,Y0}

    EN_PXL_FMT_COLOR_YUV420P,           // 4
    EN_PXL_FMT_COLOR_YUV420_NV21,       // 5
    EN_PXL_FMT_COLOR_YUV420_NV12,       // 6

    EN_PXL_FMT_COLOR_RGB888_ARGB,       // 7
    EN_PXL_FMT_COLOR_RGB888_ABGR,       // 8
    EN_PXL_FMT_COLOR_RGB888_RGBA,       // 9
    EN_PXL_FMT_COLOR_RGB888_BGRA,       // 10
    EN_PXL_FMT_COLOR_RGB565,            // 11
} EN_PXL_FMT, *PEN_PXL_FMT;
#define EN_PXL_FMT_MAX                          (EN_PXL_FMT_COLOR_RGB565 + 1)
#define EN_PXL_FMT_NONE                         (v4l2_fourcc('N', 'O', 'N', 'E'))   // 0x4E4F4E45
#else
typedef enum _EN_DEV_ID {
    EN_DEV_ID_ISP_0 = 0,                        // 00
    EN_DEV_ID_ISP_1,                            // 01
} EN_DEV_ID, *PEN_DEV_ID;
#define EN_DEV_ID_MAX                           (EN_DEV_ID_ISP_1 + 1)

typedef enum _EN_DEV_TYPE {
    EN_DEV_TYPE_NONE        = 0x0000,
    EN_DEV_TYPE_ISP         = 0x0001,
} EN_DEV_TYPE, *PEN_DEV_TYPE;
#endif //#if defined(V4L2_DRIVER)


typedef struct _ST_PP_VDO_MODE {
    EN_PXL_FMT enPxlFmt;
    STF_UINT uiHeight;
    STF_UINT uiWidth;
    STF_UINT uiAddr;
} ST_PP_VDO_MODE, *PST_PP_VDO_MODE;

typedef struct _ST_PP_MODE {
    STF_U8 u8Id;
    bool bBusOut;           /*out to ddr*/
    bool bFifoOut;          /*out to lcdc*/
    bool bInitialized;
    ST_PP_VDO_MODE stPpVdoModeSrc;
    ST_PP_VDO_MODE stPpVdoModeDst;
} ST_PP_MODE, *PST_PP_MODE;

typedef struct _ST_DRM_BUF {
    STF_U32 u32Pitch;
    STF_U32 u32Size;
    STF_U32 u32FbId;
    STF_INT nDmaBufFd;  // Used for DMA_BUF
    STF_INT nBufObjHandle;
    STF_U8 *pu8Buf;
} ST_DRM_BUF, *PST_DRM_BUF;

typedef struct _ST_DRM_DEV ST_DRM_DEV;

typedef struct _ST_DRM_DEV {
    STF_U32 u32ConnId;
    STF_U32 u32EncId;
    STF_U32 u32CrtId;
    STF_U32 u32Width;
    STF_U32 u32Height;
    STF_U32 u32Pitch;
    drmModeModeInfo stDrmModeInfo;
    drmModeCrtc *pstDrmCrtc;
    STF_U32 u32DrmFormat;
    ST_DRM_BUF stDrmBuf[PIPELINE_IMG_BUF_MAX];
    ST_DRM_DEV *pstDrmDevNext;
} ST_DRM_DEV, *PST_DRM_DEV;

typedef struct _ST_DMA_BUF_INFO {
    //STF_U8 u8BufIdx;
    STF_INT nDmaBufFd;
    //STF_U32 u32Length;
    STF_VOID *pvBuffer;
} ST_DMA_BUF_INFO, *PST_DMA_BUF_INFO;

typedef struct _ST_VDO_BUF {
    STF_U32 u32Index;
    STF_INT nDmaBufFd;
    STF_U32 u32Length;
    STF_VOID *pvBuffer;
} ST_VDO_BUF, *PST_VDO_BUF;

typedef struct _ST_VDO_MEM {
    STF_U8 u8BufIdx;
    STF_INT nDmaBufFd;
    STF_U32 u32Length;
    STF_VOID *pvBuffer;
    STF_U32 u32BytesUsed;
    struct timeval stTimeStamp;
    STF_BOOL8 bIsScDumpForAe;                   /** It used to indentify the SC dump buffer is for AE or AWB. */
    CI_MEM_PARAM *pstMem1;                      /** Memory 1 information. */
    CI_MEM_PARAM *pstMem2;                      /** Memory 2 information. */
    sCell_T stCell;
} ST_VDO_MEM, *PST_VDO_MEM;

typedef struct _ST_VDO_MEM_TBL {
    STF_U32 u32Count;
    ST_VDO_MEM *pstVdoMem;
} ST_VDO_MEM_TBL, *PST_VDO_MEM_TBL;

typedef struct _ST_ITEM_INFO {
    STF_U8 u8Cnt;
    STF_U8 u8Idx;
    STF_CHAR szInfo[ITEM_INFO_MAX][128];
} ST_ITEM_INFO , *PST_ITEM_INFO;

typedef struct _ST_VDO_BUF_INFO {
    STF_U8 u8BufIdx;
    STF_BOOL8 bIsScDumpForAe;
    STF_U32 u32BytesUsed;
    struct timeval stTimeStamp;
} ST_VDO_BUF_INFO, *PST_VDO_BUF_INFO;

typedef struct _ST_FB_DEV_PARAM {
    STF_U16 u16Width;
    STF_U16 u16Height;
    STF_U32 u32Bpp;
    STF_U32 u32ScreenSize;
    STF_U32 u32PixelFormat;
    STF_VOID *pvBuffer;
    struct fb_var_screeninfo stFbVarScreenInfo;
    struct fb_fix_screeninfo stFbFixScreenInfo;
} ST_FB_DEV_PARAM, *PST_FB_DEV_PARAM;

typedef struct _ST_PP_DEV_PARAM {
    STF_U32 u32PixelFormat;
} ST_PP_DEV_PARAM, *PST_PP_DEV_PARAM;

typedef struct _ST_DRM_DEV_PARAM {
    STF_U16 u16Width;
    STF_U16 u16Height;
    STF_U32 u32PixelFormat;
    STF_U32 u32BufCount;
    STF_INT nConnectorId;
    ST_DRM_DEV *pstDrmDevHead;
} ST_DRM_DEV_PARAM, *PST_DRM_DEV_PARAM;

typedef struct _ST_VDEV_PARAM {
    STF_BOOL8 bForceGenDev;
    STF_BOOL8 bIsVideoOut;
    STF_BOOL8 bStreamOn;
    STF_BOOL8 bIsDmaBufAllocFromDrmDev;
    STF_U16 u16Width;
    STF_U16 u16Height;
    STF_U32 u32PixelFormat;
    STF_BOOL8 bCrop;
    ST_RECT_2 stCropRect;
    STF_U8 u8Fps;
    ST_VDO_MEM_TBL stVdoMemTbl;
    pthread_mutex_t stBufLock;
    pthread_mutex_t stListLock;
    sLinkedList_T stAvailableQueue;
    sLinkedList_T stCompletedQueue;
} ST_VDEV_PARAM, *PST_VDEV_PARAM;

typedef struct _ST_CI_DEVICE ST_CI_DEVICE;

typedef struct _ST_CI_DEVICE {
    /** @brief Connection to the CI */
    CI_CONNECTION *pstCiConnection;
    STF_U8 u8DeviceId;
    STF_U8 u8DeviceType;
    STF_CHAR szDeviceName[DEV_NAME_LEN_MAX];
    /** @brief Number of CI connections */
    STF_S32 s32CiConnections;
    STF_BOOL8 bConnected;
    STF_BOOL8 bInitDone;
    EN_MEM_TYPE enMemType;
    STF_VOID *pvDevParam;

    /**
     * @brief Destroy the ST_CI_DEVICE.
     */
    STF_RESULT (*Destroy)(ST_CI_DEVICE *pstDevice);
    /**
     * @brief Connect to the CI driver.
     *
     * @return STF_SUCCESS
     * @return STF_ERROR_UNEXPECTED_STATE if the device is in the wrong state
     * @return STF_ERROR_FATAL if CI failed to initialise
     */
    STF_RESULT (*Connection)(ST_CI_DEVICE *pstDevice);
    /**
     * @brief Disconnect from the CI driver.
     *
     * @return STF_SUCCESS
     * @return STF_ERROR_UNEXPECTED_STATE if the device is in the wrong state
     * @return STF_ERROR_FATAL for internal failure
     */
    STF_RESULT (*Disconnect)(ST_CI_DEVICE *pstDevice);

    /**
     * @brief Check the connection status.
     *
     * @return STF_TRUE / STF_FALSE
     */
    STF_BOOL8 (*IsConnected)(ST_CI_DEVICE *pstDevice);
    /**
     * @brief Get the number of CI connections.
     *
     * @return Number of CI connections
     */
    STF_S32 (*GetConnections)(ST_CI_DEVICE *pstDevice);
    /**
     * @brief Get the CI_CONNECTION pointer.
     *
     * @return CI_CONNECTION pointer
     */
    CI_CONNECTION *(*GetConnection)(ST_CI_DEVICE *pstDevice);
#if defined(DEVIVE_SUPPORT_GET_FILE_HANDLE)
    /**
     * @brief Get the file handle.
     *
     * @return file handle
     */
    int (*GetFileHandle)(ST_CI_DEVICE *pstDevice);
#endif //#if defined(DEVIVE_SUPPORT_GET_FILE_HANDLE)
    /**
     * @brief Subscribe an event to the video device.
     *
     * @param nEventType Type of the event, -1 is set V4L2_EVENT_SOURCE_CHANGE as default event.
     *
     * @return STF_SUCCESS
     * @return STF_ERROR_UNEXPECTED_STATE if the device is in the wrong state
     * @return STF_ERROR_FATAL if CI failed to initialise
     */
    STF_RESULT (*SubscribeEvent)(ST_CI_DEVICE *pstDevice, STF_S32 s32EventType);
    /**
     * @brief Unsubscribe an event to the video device.
     *
     * @param nEventType Type of the event, -1 is set V4L2_EVENT_SOURCE_CHANGE as default event.
     *
     * @return STF_SUCCESS
     * @return STF_ERROR_UNEXPECTED_STATE if the device is in the wrong state
     * @return STF_ERROR_FATAL if CI failed to initialise
     */
    STF_RESULT (*UnsubscribeEvent)(ST_CI_DEVICE *pstDevice, STF_S32 s32EventType);
    /**
     * @brief Dequeue an event from the video device.
     *
     * @return s32SourceCount Number of source to stream on the video device.
     * @return struct v4l2_event.
     */
    STF_RESULT (*DequeueEvent)(ST_CI_DEVICE *pstDevice, struct v4l2_event *pstEvent);
    /**
     * @brief initial the device.
     *
     * @return STF_SUCCESS
     * @return STF_ERROR_UNEXPECTED_STATE if the device is in the wrong state
     * @return STF_ERROR_FATAL if CI failed to initialise
     */
    STF_RESULT (*InitDevice)(ST_CI_DEVICE *pstDevice, STF_U32 u32PixelFormat);
    /**
     * @brief Get the pixel format from the device.
     *
     * @return u32PixelFormat pixel format.
     */
    STF_U32 (*GetPixelFormat)(ST_CI_DEVICE *pstDevice);
    /**
     * @brief Set the pixel format to the device.
     *
     * @param u32PixelFormat pixel format.
     *
     * @return STF_SUCCESS
     * @return STF_ERROR_UNEXPECTED_STATE if the device is in the wrong state
     * @return STF_ERROR_FATAL if CI failed to initialise
     */
    STF_RESULT (*SetPixelFormat)(ST_CI_DEVICE *pstDevice, STF_U32 u32PixelFormat);

    /**
     * @brief Query the cropping window from the video device.
     *
     * @return stRect cropping window.
     */
    struct v4l2_rect (*QueryCropWin)(ST_CI_DEVICE *pstDevice);
    /**
     * @brief Query the cropping window from the video device.
     *
     * @return stRect cropping window.
     */
    ST_RECT (*QueryCropWin_2)(ST_CI_DEVICE *pstDevice);
    /**
     * @brief Get the cropping window from the video device.
     *
     * @return stRect cropping window.
     */
    struct v4l2_rect (*GetCropWin)(ST_CI_DEVICE *pstDevice);
    /**
     * @brief Get the cropping window from the video device.
     *
     * @return stRect cropping window.
     */
    ST_RECT (*GetCropWin_2)(ST_CI_DEVICE *pstDevice);
    /**
     * @brief Set the cropping window into the video device.
     *
     * @param pstRect cropping window.
     *
     * @return STF_SUCCESS
     * @return STF_ERROR_UNEXPECTED_STATE if the device is in the wrong state
     * @return STF_ERROR_FATAL if CI failed to initialise
     */
    STF_RESULT (*SetCropWin)(ST_CI_DEVICE *pstDevice, struct v4l2_rect *pstRect);
    /**
     * @brief Set the cropping window into the video device.
     *
     * @param pstRect cropping window.
     *
     * @return STF_SUCCESS
     * @return STF_ERROR_UNEXPECTED_STATE if the device is in the wrong state
     * @return STF_ERROR_FATAL if CI failed to initialise
     */
    STF_RESULT (*SetCropWin_2)(ST_CI_DEVICE *pstDevice, ST_RECT *pstRect);
    /**
     * @brief Enumerate supported formats from the video device.
     *
     * @param .
     *
     * @return STF_SUCCESS
     * @return STF_ERROR_UNEXPECTED_STATE if the device is in the wrong state
     * @return STF_ERROR_FATAL if CI failed to initialise
     */
    STF_RESULT (*EnumFormat)(ST_CI_DEVICE *pstDevice);
    /**
     * @brief Get the data format from the video device.
     *
     * @param pstFormat data format pointer.
     *
     * @return STF_SUCCESS
     * @return STF_ERROR_UNEXPECTED_STATE if the device is in the wrong state
     * @return STF_ERROR_FATAL if CI failed to initialise
     */
    STF_RESULT (*GetFormat)(ST_CI_DEVICE *pstDevice, struct v4l2_format *pstFormat);
    /**
     * @brief Set the data format into the video device.
     *
     * @param u16Width the image width.
     * @param u16Height the image height.
     * @param u32PixelFormat the pixel format or type of compression.
     *
     * @return STF_SUCCESS
     * @return STF_ERROR_UNEXPECTED_STATE if the device is in the wrong state
     * @return STF_ERROR_FATAL if CI failed to initialise
     */
    STF_RESULT (*SetFormat)(ST_CI_DEVICE *pstDevice, STF_U16 u16Width, STF_U16 u16Height, STF_U32 u32PixelFormat);
    /**
     * @brief Test if the video device supports this data format.
     *
     * @param u32PixelFormat the pixel format or type of compression..
     *
     * @return STF_SUCCESS
     * @return STF_ERROR_UNEXPECTED_STATE if the device is in the wrong state
     * @return STF_ERROR_FATAL if CI failed to initialise
     */
    STF_RESULT (*TryFormat)(ST_CI_DEVICE *pstDevice, STF_U32 u32PixelFormat);
    /**
     * @brief Get streaming parameters from the video device.
     *
     * @param pu32Numerator data format pointer.
     * @param pu32Denominator data format pointer.
     *
     * @return STF_SUCCESS
     * @return STF_ERROR_UNEXPECTED_STATE if the device is in the wrong state
     * @return STF_ERROR_FATAL if CI failed to initialise
     */
    STF_RESULT (*GetParam)(ST_CI_DEVICE *pstDevice, STF_U32 *pu32Numerator, STF_U32 *pu32Denominator);
    /**
     * @brief Set streaming parameters into the video device.
     *
     * @param u32Numerator data format pointer.
     * @param u32Denominator data format pointer.
     *
     * @return STF_SUCCESS
     * @return STF_ERROR_UNEXPECTED_STATE if the device is in the wrong state
     * @return STF_ERROR_FATAL if CI failed to initialise
     */
    STF_RESULT (*SetParam)(ST_CI_DEVICE *pstDevice, STF_U32 u32Numerator, STF_U32 u32Denominator);
    /**
     * @brief Set streaming parameters into the video device.
     *
     * @param u32FPS the frame rate per second.
     *
     * @return STF_SUCCESS
     * @return STF_ERROR_UNEXPECTED_STATE if the device is in the wrong state
     * @return STF_ERROR_FATAL if CI failed to initialise
     */
    STF_RESULT (*SetParam_2)(ST_CI_DEVICE *pstDevice, STF_U32 u32FPS);
    /**
     * @brief Get selection rectangle from the video device.
     *
     * @return stRect the selection cropping window.
     */
    struct v4l2_rect (*GetSelection)(ST_CI_DEVICE *pstDevice);
    /**
     * @brief Get selection rectangle from the video device.
     *
     * @return stRect the selection cropping window.
     */
    ST_RECT (*GetSelection_2)(ST_CI_DEVICE *pstDevice);
    /**
     * @brief Set selection rectangle into the video device.
     *
     * @param pstRect the selection cropping window.
     *
     * @return STF_SUCCESS
     * @return STF_ERROR_UNEXPECTED_STATE if the device is in the wrong state
     * @return STF_ERROR_FATAL if CI failed to initialise
     */
    STF_RESULT (*SetSelection)(ST_CI_DEVICE *pstDevice, struct v4l2_rect *pstRect);
    /**
     * @brief Set selection rectangle into the video device.
     *
     * @param pstRect the selection cropping window.
     *
     * @return STF_SUCCESS
     * @return STF_ERROR_UNEXPECTED_STATE if the device is in the wrong state
     * @return STF_ERROR_FATAL if CI failed to initialise
     */
    STF_RESULT (*SetSelection_2)(ST_CI_DEVICE *pstDevice, ST_RECT *pstRect);
    /**
     * @brief Start/Stop video device streaming I/O.
     *
     * @param bOn On/Off the streaming.
     *
     * @return STF_SUCCESS
     * @return STF_ERROR_UNEXPECTED_STATE if the device is in the wrong state
     * @return STF_ERROR_FATAL if CI failed to initialise
     */
    STF_RESULT (*SetStreamOn)(ST_CI_DEVICE *pstDevice, STF_BOOL8 bOn);
    /**
     * @brief Check the streaming I/O status from the video device.
     *
     * @return STF_TRUE / STF_FALSE
     */
    STF_BOOL8 (*IsStreamOn)(ST_CI_DEVICE *pstDevice);
    /**
     * @brief Allocate buffer for the video device.
     *
     * @param u32Count The number of buffers requested or granted.
     *
     * @return STF_SUCCESS
     * @return STF_ERROR_UNEXPECTED_STATE if the device is in the wrong state
     * @return STF_ERROR_FATAL if CI failed to initialise
     */
    STF_RESULT (*AllocateBuf)(ST_CI_DEVICE *pstDevice, STF_U32 u32Count, ST_DMA_BUF_INFO *pstDmaBufInfo);
    /**
     * @brief Allocate and enqueue buffer into the video device.
     *
     * @param u32AllocateCount The number of buffers requested or granted.
     * @param u32EnqueueCount The number of buffers will be enqueue.
     *
     * @return STF_SUCCESS
     * @return STF_ERROR_UNEXPECTED_STATE if the device is in the wrong state
     * @return STF_ERROR_FATAL if CI failed to initialise
     */
    STF_RESULT (*AllocateAndEnqueueBuf)(ST_CI_DEVICE *pstDevice, STF_U32 u32AllocateCount, STF_U32 u32EnqueueCount, ST_DMA_BUF_INFO *pstDmaBufInfo);
    /**
     * @brief Get the video buffer from the video device.
     *
     * @param u32Index The specified index.
     *
     * @return ST_VDO_MEM Buffer pointer at the specified index.
     */
    ST_VDO_MEM *(*GetBuffer)(ST_CI_DEVICE *pstDevice, STF_U32 u32Index);
    /**
     * @brief Get the video buffer information from the video device.
     *
     * @param u32Index The specified index.
     *
     * @return ST_VDO_BUF Buffer information at the specified index.
     */
    ST_VDO_BUF (*GetBufInfo)(ST_CI_DEVICE *pstDevice, STF_U32 u32Index);
    /**
     * @brief Enqueue buffer into the video device.
     *
     * @param u8BufIdx The video buffer index.
     *
     * @return STF_SUCCESS
     * @return STF_ERROR_UNEXPECTED_STATE if the device is in the wrong state
     * @return STF_ERROR_FATAL if CI failed to initialise
     */
    STF_RESULT (*EnqueueBuf)(ST_CI_DEVICE *pstDevice, STF_U8 u8BufIdx);
    /**
     * @brief Enqueue buffers into the video device.
     *
     * @param u32Count The number of buffers will be enqueue into the video device.
     * @param pu32BufIndex The video buffer index array.
     *
     * @return STF_SUCCESS
     * @return STF_ERROR_UNEXPECTED_STATE if the device is in the wrong state
     * @return STF_ERROR_FATAL if CI failed to initialise
     */
    STF_RESULT (*EnqueueBuf_2)(ST_CI_DEVICE *pstDevice, STF_U32 u32Count, ST_VDO_BUF_INFO *pstVideoBufferInfo);
    /**
     * @brief Enqueue buffers from available queue to the video device.
     *
     * @param s32Count The number of buffers will be enqueue into the video device.
     *
     * @return STF_SUCCESS
     * @return STF_ERROR_UNEXPECTED_STATE if the device is in the wrong state
     * @return STF_ERROR_FATAL if CI failed to initialise
     */
    STF_RESULT (*EnqueueBufFromAvailableQueue)(ST_CI_DEVICE *pstDevice, STF_S32 s32Count);
    /**
     * @brief Enqueue buffers from completed queue to the video device.
     *
     * @param s32RemainCount The number of buffers will be remain on the completed queue.
     *
     * @return STF_SUCCESS
     * @return STF_ERROR_UNEXPECTED_STATE if the device is in the wrong state
     * @return STF_ERROR_FATAL if CI failed to initialise
     */
    STF_RESULT (*EnqueueBufFromCompletedQueue)(ST_CI_DEVICE *pstDevice, STF_S32 s32Count);
    /**
     * @brief Dequeue buffers from the video device.
     *
     * @param ppstVdoMem The video memory information structure pointer address.
     *
     * @return STF_SUCCESS
     * @return STF_ERROR_UNEXPECTED_STATE if the device is in the wrong state
     * @return STF_ERROR_FATAL if CI failed to initialise
     */
    STF_RESULT (*DequeueBuf)(ST_CI_DEVICE *pstDevice, ST_VDO_MEM **ppstVdoMem);
    /**
     * @brief Dequeue buffers from the video device.
     *
     * @param pu32Count The number of buffers has be dequeue from the video device.
     * @param pstVideoBufferInfo The video buffer information array.
     *
     * @return STF_SUCCESS
     * @return STF_ERROR_UNEXPECTED_STATE if the device is in the wrong state
     * @return STF_ERROR_FATAL if CI failed to initialise
     */
    STF_RESULT (*DequeueBuf_2)(ST_CI_DEVICE *pstDevice, STF_S32 *ps32Count, ST_VDO_BUF_INFO *pstVideoBufferInfo);
    /**
     * @brief Dequeue buffers from the video device to completed queue.
     *
     * @param s32Count The number of buffers has be dequeue from the video device.
     *
     * @return STF_SUCCESS
     * @return STF_ERROR_UNEXPECTED_STATE if the device is in the wrong state
     * @return STF_ERROR_FATAL if CI failed to initialise
     */
    STF_RESULT (*DequeueBufToCompletedQueue)(ST_CI_DEVICE *pstDevice, STF_S32 s32Count);
    /**
     * @brief Switch buffers from the completed queue to available queue.
     *
     * @param s32RemainCount The number of buffers will be remain on the completed queue.
     *
     * @return STF_SUCCESS
     * @return STF_ERROR_UNEXPECTED_STATE if the device is in the wrong state
     * @return STF_ERROR_FATAL if CI failed to initialise
     */
    STF_RESULT (*SwitchBufFromCompletedToAvailableQueue)(ST_CI_DEVICE *pstDevice, STF_S32 s32RemainCount);
    /**
     * @brief Push buffer into the queue.
     *
     * @param pstVdoMem The pointer of buffer.
     * @param pstListQueue The pointer of list queue.
     *
     * @return STF_SUCCESS
     * @return STF_ERROR_UNEXPECTED_STATE if the device is in the wrong state
     * @return STF_ERROR_FATAL if CI failed to initialise
     */
    STF_RESULT (*PushBufToQueue)(ST_CI_DEVICE *pstDevice, sLinkedList_T *pstListQueue, ST_VDO_MEM *pstVdoMem, STF_BOOL8 bIsPushToFront);
    /**
     * @brief Pop buffer from the queue.
     *
     * @param ppstVdoMem The pointer of pointer of buffer.
     * @param pstListQueue The pointer of list queue.
     *
     * @return STF_SUCCESS
     * @return STF_ERROR_UNEXPECTED_STATE if the device is in the wrong state
     * @return STF_ERROR_FATAL if CI failed to initialise
     */
    ST_VDO_MEM *(*PopBufFromQueue)(ST_CI_DEVICE *pstDevice, sLinkedList_T *pstListQueue, STF_BOOL8 bIsPopFromBack);
    /**
     * @brief Release buffers from the device.
     *
     * @return STF_SUCCESS
     * @return STF_ERROR_UNEXPECTED_STATE if the device is in the wrong state
     * @return STF_ERROR_FATAL if CI failed to initialise
     */
    STF_RESULT (*ReleaseBuf)(ST_CI_DEVICE *pstDevice);
    /**
     * @brief Generate the media link information.
     *
     * @param enSensorInterface indicate the sensor interface DVP0/CSI0/CSI1.
     * @param pstLinksInfo store the links information.
     *
     * @return STF_SUCCESS
     * @return STF_ERROR_UNEXPECTED_STATE if the device is in the wrong state
     * @return STF_ERROR_FATAL if CI failed to initialise
     */
    STF_RESULT (*GenerateLinksInfo)(ST_CI_DEVICE *pstDevice, EN_SNSR_IF enSensorInterface, ST_ITEM_INFO *pstLinksInfo);
    /**
     * @brief Process the media link settings.
     *
     * @param pstLinksInfo links information.
     *
     * @return STF_SUCCESS
     * @return STF_ERROR_UNEXPECTED_STATE if the device is in the wrong state
     * @return STF_ERROR_FATAL if CI failed to initialise
     */
    STF_RESULT (*ProcessLinks)(ST_CI_DEVICE *pstDevice, ST_ITEM_INFO *pstLinksInfo);
    /**
     * @brief Process the media link settings.
     *
     * @param enSensorInterface indicate the sensor interface DVP0/CSI0/CSI1.
     *
     * @return STF_SUCCESS
     * @return STF_ERROR_UNEXPECTED_STATE if the device is in the wrong state
     * @return STF_ERROR_FATAL if CI failed to initialise
     */
    STF_RESULT (*ProcessLinks_2)(ST_CI_DEVICE *pstDevice, EN_SNSR_IF enSensorInterface);
} ST_CI_DEVICE, *PST_CI_DEVICE;

typedef struct _ST_VDEV_TBL {
    STF_U32 u32Count;
    ST_CI_DEVICE stVDev[EN_ISP_PORT_IDX_MAX + 1];
} ST_VDEV_TBL, *PST_VDEV_TBL;

typedef struct _ST_VDEV_PTR_TBL {
    STF_U32 u32Count;
    ST_CI_DEVICE *pstVDev[EN_ISP_PORT_IDX_MAX + 1];
} ST_VDEV_PTR_TBL, *PST_VDEV_PTR_TBL;


extern STF_CHAR g_szDeviceName[EN_DEV_ID_MAX + 1][DEV_NAME_LEN_MAX];
extern const EN_DEV_ID g_enIspDevId[EN_ISP_DEV_IDX_MAX];
extern const STF_CHAR g_szSensorInterface[EN_SNSR_IF_MAX][16];
extern const STF_CHAR g_szIspDeviceInterface[EN_ISP_DEV_IDX_MAX][16];
extern const STF_CHAR g_szIspPortInterface[EN_ISP_PORT_IDX_MAX][16];
extern const STF_U32 g_u32IspPortPixelFormatDef[EN_ISP_PORT_IDX_MAX];
extern const STF_CHAR g_szIspPortPixelFormatDef[EN_ISP_PORT_IDX_MAX][24];
extern const STF_U32 g_u32BayerFormatDef[BAYER_FORMAT_MAX * 2];
extern const STF_CHAR g_szBayerFormatDef[BAYER_FORMAT_MAX * 2][24];
extern const STF_CHAR g_szMemType[EN_MEM_TYPE_MAX][24];


extern
STF_RESULT STFLIB_ISP_DEVICE_Destroy(
    ST_CI_DEVICE *pstDevice
    );

extern
STF_RESULT STFLIB_ISP_DEVICE_Connection(
    ST_CI_DEVICE *pstDevice
    );

extern
STF_RESULT STFLIB_ISP_DEVICE_Disconnect(
    ST_CI_DEVICE *pstDevice
    );

extern
STF_BOOL8 STFLIB_ISP_DEVICE_IsConnected(
    ST_CI_DEVICE *pstDevice
    );

extern
STF_S32 STFLIB_ISP_DEVICE_GetConnections(
    ST_CI_DEVICE *pstDevice
    );

extern
CI_CONNECTION *STFLIB_ISP_DEVICE_GetConnection(
    ST_CI_DEVICE *pstDevice
    );

#if defined(DEVIVE_SUPPORT_GET_FILE_HANDLE)
extern
int STFLIB_ISP_DEVICE_GetFileHandle(
    ST_CI_DEVICE *pstDevice
    );

#endif //#if defined(DEVIVE_SUPPORT_GET_FILE_HANDLE)
extern
STF_RESULT STFLIB_ISP_DEVICE_SubscribeEvent(
    ST_CI_DEVICE *pstDevice,
    STF_S32 s32EventType
    );

extern
STF_RESULT STFLIB_ISP_DEVICE_UnsubscribeEvent(
    ST_CI_DEVICE *pstDevice,
    STF_S32 s32EventType
    );

extern
STF_RESULT STFLIB_ISP_DEVICE_DequeueEvent(
    ST_CI_DEVICE *pstDevice,
    struct v4l2_event *pstEvent
    );

extern
STF_RESULT STFLIB_ISP_DEVICE_InitDevice(
    ST_CI_DEVICE *pstDevice,
    STF_U32 u32PixelFormat
    );

extern
STF_U32 STFLIB_ISP_DEVICE_FB_ConvertV4l2FormatToFbFormat(
    STF_U32 u32V4l2Format
    );

extern
STF_RESULT STFLIB_ISP_DEVICE_FB_InitDevice(
    ST_CI_DEVICE *pstDevice,
    STF_U32 u32PixelFormat
    );

extern
STF_RESULT STFLIB_ISP_DEVICE_PP_InitDevice(
    ST_CI_DEVICE *pstDevice,
    STF_U32 u32PixelFormat
    );

extern
STF_U32 STFLIB_ISP_DEVICE_DRM_ConvertV4l2FormatToDrmFormat(
    STF_U32 u32V4l2Format
    );

extern
STF_RESULT STFLIB_ISP_DEVICE_DRM_InitDevice(
    ST_CI_DEVICE *pstDevice,
    STF_U32 u32PixelFormat
    );

extern
STF_RESULT STFLIB_ISP_DEVICE_VDO_InitDevice(
    ST_CI_DEVICE *pstDevice,
    STF_U32 u32PixelFormat
    );

extern
STF_U32 STFLIB_ISP_DEVICE_GetPixelFormat(
    ST_CI_DEVICE *pstDevice
    );

extern
STF_U32 STFLIB_ISP_DEVICE_FB_GetPixelFormat(
    ST_CI_DEVICE *pstDevice
    );

extern
STF_U32 STFLIB_ISP_DEVICE_PP_GetPixelFormat(
    ST_CI_DEVICE *pstDevice
    );

extern
STF_U32 STFLIB_ISP_DEVICE_DRM_GetPixelFormat(
    ST_CI_DEVICE *pstDevice
    );

extern
STF_U32 STFLIB_ISP_DEVICE_VDO_GetPixelFormat(
    ST_CI_DEVICE *pstDevice
    );

extern
STF_RESULT STFLIB_ISP_DEVICE_SetPixelFormat(
    ST_CI_DEVICE *pstDevice,
    STF_U32 u32PixelFormat
    );

extern
STF_RESULT STFLIB_ISP_DEVICE_FB_SetPixelFormat(
    ST_CI_DEVICE *pstDevice,
    STF_U32 u32PixelFormat
    );

extern
STF_RESULT STFLIB_ISP_DEVICE_PP_SetPixelFormat(
    ST_CI_DEVICE *pstDevice,
    STF_U32 u32PixelFormat
    );

extern
STF_RESULT STFLIB_ISP_DEVICE_DRM_SetPixelFormat(
    ST_CI_DEVICE *pstDevice,
    STF_U32 u32PixelFormat
    );

extern
STF_RESULT STFLIB_ISP_DEVICE_VDO_SetPixelFormat(
    ST_CI_DEVICE *pstDevice,
    STF_U32 u32PixelFormat
    );

extern
struct v4l2_rect STFLIB_ISP_DEVICE_QueryCropWin(
    ST_CI_DEVICE *pstDevice
    );

extern
struct v4l2_rect STFLIB_ISP_DEVICE_VDO_QueryCropWin(
    ST_CI_DEVICE *pstDevice
    );

extern
ST_RECT STFLIB_ISP_DEVICE_QueryCropWin_2(
    ST_CI_DEVICE *pstDevice
    );

extern
ST_RECT STFLIB_ISP_DEVICE_VDO_QueryCropWin_2(
    ST_CI_DEVICE *pstDevice
    );

extern
struct v4l2_rect STFLIB_ISP_DEVICE_GetCropWin(
    ST_CI_DEVICE *pstDevice
    );

extern
struct v4l2_rect STFLIB_ISP_DEVICE_VDO_GetCropWin(
    ST_CI_DEVICE *pstDevice
    );

extern
ST_RECT STFLIB_ISP_DEVICE_GetCropWin_2(
    ST_CI_DEVICE *pstDevice
    );

extern
ST_RECT STFLIB_ISP_DEVICE_VDO_GetCropWin_2(
    ST_CI_DEVICE *pstDevice
    );

extern
STF_RESULT STFLIB_ISP_DEVICE_SetCropWin(
    ST_CI_DEVICE *pstDevice,
    struct v4l2_rect *pstRect
    );

extern
STF_RESULT STFLIB_ISP_DEVICE_VDO_SetCropWin(
    ST_CI_DEVICE *pstDevice,
    struct v4l2_rect *pstRect
    );

extern
STF_RESULT STFLIB_ISP_DEVICE_SetCropWin_2(
    ST_CI_DEVICE *pstDevice,
    ST_RECT *pstRect
    );

extern
STF_RESULT STFLIB_ISP_DEVICE_VDO_SetCropWin_2(
    ST_CI_DEVICE *pstDevice,
    ST_RECT *pstRect
    );

extern
STF_RESULT STFLIB_ISP_DEVICE_EnumFormat(
    ST_CI_DEVICE *pstDevice
    );

extern
STF_RESULT STFLIB_ISP_DEVICE_VDO_EnumFormat(
    ST_CI_DEVICE *pstDevice
    );

extern
STF_RESULT STFLIB_ISP_DEVICE_GetFormat(
    ST_CI_DEVICE *pstDevice,
    struct v4l2_format *pstFormat
    );

extern
STF_RESULT STFLIB_ISP_DEVICE_VDO_GetFormat(
    ST_CI_DEVICE *pstDevice,
    struct v4l2_format *pstFormat
    );

extern
STF_RESULT STFLIB_ISP_DEVICE_SetFormat(
    ST_CI_DEVICE *pstDevice,
    STF_U16 u16Width,
    STF_U16 u16Height,
    STF_U32 u32PixelFormat
    );

extern
STF_RESULT STFLIB_ISP_DEVICE_VDO_SetFormat(
    ST_CI_DEVICE *pstDevice,
    STF_U16 u16Width,
    STF_U16 u16Height,
    STF_U32 u32PixelFormat
    );

extern
STF_RESULT STFLIB_ISP_DEVICE_TryFormat(
    ST_CI_DEVICE *pstDevice,
    STF_U32 u32PixelFormat
    );

extern
STF_RESULT STFLIB_ISP_DEVICE_VDO_TryFormat(
    ST_CI_DEVICE *pstDevice,
    STF_U32 u32PixelFormat
    );

extern
STF_RESULT STFLIB_ISP_DEVICE_GetParam(
    ST_CI_DEVICE *pstDevice,
    STF_U32 *pu32Numerator,
    STF_U32 *pu32Denominator
    );

extern
STF_RESULT STFLIB_ISP_DEVICE_VDO_GetParam(
    ST_CI_DEVICE *pstDevice,
    STF_U32 *pu32Numerator,
    STF_U32 *pu32Denominator
    );

extern
STF_RESULT STFLIB_ISP_DEVICE_SetParam(
    ST_CI_DEVICE *pstDevice,
    STF_U32 u32Numerator,
    STF_U32 u32Denominator
    );

extern
STF_RESULT STFLIB_ISP_DEVICE_VDO_SetParam(
    ST_CI_DEVICE *pstDevice,
    STF_U32 u32Numerator,
    STF_U32 u32Denominator
    );

extern
STF_RESULT STFLIB_ISP_DEVICE_SetParam_2(
    ST_CI_DEVICE *pstDevice,
    STF_U32 u32FPS
    );

extern
STF_RESULT STFLIB_ISP_DEVICE_VDO_SetParam_2(
    ST_CI_DEVICE *pstDevice,
    STF_U32 u32FPS
    );

extern
struct v4l2_rect STFLIB_ISP_DEVICE_GetSelection(
    ST_CI_DEVICE *pstDevice
    );

extern
struct v4l2_rect STFLIB_ISP_DEVICE_VDO_GetSelection(
    ST_CI_DEVICE *pstDevice
    );

extern
ST_RECT STFLIB_ISP_DEVICE_GetSelection_2(
    ST_CI_DEVICE *pstDevice
    );

extern
ST_RECT STFLIB_ISP_DEVICE_VDO_GetSelection_2(
    ST_CI_DEVICE *pstDevice
    );

extern
STF_RESULT STFLIB_ISP_DEVICE_SetSelection(
    ST_CI_DEVICE *pstDevice,
    struct v4l2_rect *pstRect
    );

extern
STF_RESULT STFLIB_ISP_DEVICE_VDO_SetSelection(
    ST_CI_DEVICE *pstDevice,
    struct v4l2_rect *pstRect
    );

extern
STF_RESULT STFLIB_ISP_DEVICE_SetSelection_2(
    ST_CI_DEVICE *pstDevice,
    ST_RECT *pstRect
    );

extern
STF_RESULT STFLIB_ISP_DEVICE_VDO_SetSelection_2(
    ST_CI_DEVICE *pstDevice,
    ST_RECT *pstRect
    );

extern
STF_RESULT STFLIB_ISP_DEVICE_SetStreamOn(
    ST_CI_DEVICE *pstDevice,
    STF_BOOL8 bOn
    );

extern
STF_RESULT STFLIB_ISP_DEVICE_VDO_SetStreamOn(
    ST_CI_DEVICE *pstDevice,
    STF_BOOL8 bOn
    );

extern
STF_BOOL8 STFLIB_ISP_DEVICE_IsStreamOn(
    ST_CI_DEVICE *pstDevice
    );

extern
STF_BOOL8 STFLIB_ISP_DEVICE_VDO_IsStreamOn(
    ST_CI_DEVICE *pstDevice
    );

extern
STF_RESULT STFLIB_ISP_DEVICE_AllocateBuf(
    ST_CI_DEVICE *pstDevice,
    STF_U32 u32Count,
    ST_DMA_BUF_INFO *pstDmaBufInfo
    );

extern
STF_RESULT STFLIB_ISP_DEVICE_DRM_AllocateBuf(
    ST_CI_DEVICE *pstDevice,
    STF_U32 u32Count,
    ST_DMA_BUF_INFO *pstDmaBufInfo
    );

extern
STF_RESULT STFLIB_ISP_DEVICE_VDO_AllocateBuf(
    ST_CI_DEVICE *pstDevice,
    STF_U32 u32Count,
    ST_DMA_BUF_INFO *pstDmaBufInfo
    );

extern
STF_RESULT STFLIB_ISP_DEVICE_AllocateAndEnqueueBuf(
    ST_CI_DEVICE *pstDevice,
    STF_U32 u32AllocateCount,
    STF_U32 u32EnqueueCount,
    ST_DMA_BUF_INFO *pstDmaBufInfo
    );

extern
STF_RESULT STFLIB_ISP_DEVICE_VDO_AllocateAndEnqueueBuf(
    ST_CI_DEVICE *pstDevice,
    STF_U32 u32AllocateCount,
    STF_U32 u32EnqueueCount,
    ST_DMA_BUF_INFO *pstDmaBufInfo
    );

extern
ST_VDO_MEM *STFLIB_ISP_DEVICE_GetBuffer(
    ST_CI_DEVICE *pstDevice,
    STF_U32 u32Index
    );

extern
ST_VDO_MEM *STFLIB_ISP_DEVICE_VDO_GetBuffer(
    ST_CI_DEVICE *pstDevice,
    STF_U32 u32Index
    );

extern
ST_VDO_BUF STFLIB_ISP_DEVICE_GetBufInfo(
    ST_CI_DEVICE *pstDevice,
    STF_U32 u32Index
    );

extern
ST_VDO_BUF STFLIB_ISP_DEVICE_VDO_GetBufInfo(
    ST_CI_DEVICE *pstDevice,
    STF_U32 u32Index
    );

extern
STF_RESULT STFLIB_ISP_DEVICE_EnqueueBuf(
    ST_CI_DEVICE *pstDevice,
    STF_U8 u8BufIdx
    );

extern
STF_RESULT STFLIB_ISP_DEVICE_VDO_EnqueueBuf(
    ST_CI_DEVICE *pstDevice,
    STF_U8 u8BufIdx
    );

extern
STF_RESULT STFLIB_ISP_DEVICE_EnqueueBuf_2(
    ST_CI_DEVICE *pstDevice,
    STF_U32 u32Count,
    ST_VDO_BUF_INFO *pstVideoBufferInfo
    );

extern
STF_RESULT STFLIB_ISP_DEVICE_VDO_EnqueueBuf_2(
    ST_CI_DEVICE *pstDevice,
    STF_U32 u32Count,
    ST_VDO_BUF_INFO *pstVideoBufferInfo
    );

extern
STF_RESULT STFLIB_ISP_DEVICE_EnqueueBufFromAvailableQueue(
    ST_CI_DEVICE *pstDevice,
    STF_S32 s32Count
    );

extern
STF_RESULT STFLIB_ISP_DEVICE_VDO_EnqueueBufFromAvailableQueue(
    ST_CI_DEVICE *pstDevice,
    STF_S32 s32Count
    );

extern
STF_RESULT STFLIB_ISP_DEVICE_EnqueueBufFromCompletedQueue(
    ST_CI_DEVICE *pstDevice,
    STF_S32 s32RemainCount
    );

extern
STF_RESULT STFLIB_ISP_DEVICE_VDO_EnqueueBufFromCompletedQueue(
    ST_CI_DEVICE *pstDevice,
    STF_S32 s32RemainCount
    );

extern
STF_RESULT STFLIB_ISP_DEVICE_DequeueBuf(
    ST_CI_DEVICE *pstDevice,
    ST_VDO_MEM **ppstVdoMem
    );

extern
STF_RESULT STFLIB_ISP_DEVICE_VDO_DequeueBuf(
    ST_CI_DEVICE *pstDevice,
    ST_VDO_MEM **ppstVdoMem
    );

extern
STF_RESULT STFLIB_ISP_DEVICE_DequeueBuf_2(
    ST_CI_DEVICE *pstDevice,
    STF_S32 *ps32Count,
    ST_VDO_BUF_INFO *pstVideoBufferInfo
    );

extern
STF_RESULT STFLIB_ISP_DEVICE_VDO_DequeueBuf_2(
    ST_CI_DEVICE *pstDevice,
    STF_S32 *ps32Count,
    ST_VDO_BUF_INFO *pstVideoBufferInfo
    );

extern
STF_RESULT STFLIB_ISP_DEVICE_DequeueBufToCompletedQueue(
    ST_CI_DEVICE *pstDevice,
    STF_S32 s32Count
    );

extern
STF_RESULT STFLIB_ISP_DEVICE_VDO_DequeueBufToCompletedQueue(
    ST_CI_DEVICE *pstDevice,
    STF_S32 s32Count
    );

extern
STF_RESULT STFLIB_ISP_DEVICE_SwitchBufFromCompletedToAvailableQueue(
    ST_CI_DEVICE *pstDevice,
    STF_S32 s32RemainCount
    );

extern
STF_RESULT STFLIB_ISP_DEVICE_VDO_SwitchBufFromCompletedToAvailableQueue(
    ST_CI_DEVICE *pstDevice,
    STF_S32 s32RemainCount
    );

extern
STF_RESULT STFLIB_ISP_DEVICE_PushBufToQueue(
    ST_CI_DEVICE *pstDevice,
    sLinkedList_T *pstListQueue,
    ST_VDO_MEM *pstVdoMem,
    STF_BOOL8 bIsPushToFront
    );

extern
STF_RESULT STFLIB_ISP_DEVICE_VDO_PushBufToQueue(
    ST_CI_DEVICE *pstDevice,
    sLinkedList_T *pstListQueue,
    ST_VDO_MEM *pstVdoMem,
    STF_BOOL8 bIsPushToFront
    );

extern
ST_VDO_MEM *STFLIB_ISP_DEVICE_PopBufFromQueue(
    ST_CI_DEVICE *pstDevice,
    sLinkedList_T *pstListQueue,
    STF_BOOL8 bIsPopFromBack
    );

extern
ST_VDO_MEM *STFLIB_ISP_DEVICE_VDO_PopBufFromQueue(
    ST_CI_DEVICE *pstDevice,
    sLinkedList_T *pstListQueue,
    STF_BOOL8 bIsPopFromBack
    );

extern
STF_RESULT STFLIB_ISP_DEVICE_ReleaseBuf(
    ST_CI_DEVICE *pstDevice
    );

extern
STF_RESULT STFLIB_ISP_DEVICE_DRM_ReleaseBuf(
    ST_CI_DEVICE *pstDevice
    );

extern
STF_RESULT STFLIB_ISP_DEVICE_VDO_ReleaseBuf(
    ST_CI_DEVICE *pstDevice
    );

extern
STF_RESULT STFLIB_ISP_DEVICE_GenerateLinksInfo(
    ST_CI_DEVICE *pstDevice,
    EN_SNSR_IF enSensorInterface,
    ST_ITEM_INFO *pstLinksInfo
    );

extern
STF_RESULT STFLIB_ISP_DEVICE_VDO_GenerateLinksInfo(
    ST_CI_DEVICE *pstDevice,
    EN_SNSR_IF enSensorInterface,
    ST_ITEM_INFO *pstLinksInfo
    );

extern
STF_RESULT STFLIB_ISP_DEVICE_ProcessLinks(
    ST_CI_DEVICE *pstDevice,
    ST_ITEM_INFO *pstLinksInfo
    );

extern
STF_RESULT STFLIB_ISP_DEVICE_VDO_ProcessLinks(
    ST_CI_DEVICE *pstDevice,
    ST_ITEM_INFO *pstLinksInfo
    );

extern
STF_RESULT STFLIB_ISP_DEVICE_ProcessLinks_2(
    ST_CI_DEVICE *pstDevice,
    EN_SNSR_IF enSensorInterface
    );

extern
STF_RESULT STFLIB_ISP_DEVICE_VDO_ProcessLinks_2(
    ST_CI_DEVICE *pstDevice,
    EN_SNSR_IF enSensorInterface
    );

extern
STF_RESULT STFLIB_ISP_DEVICE_StructInitialize(
    ST_CI_DEVICE *pstDevice,
    EN_DEV_ID enDevId,
    STF_BOOL8 bForceGenDev,
    EN_MEM_TYPE enMemType,
    STF_U16 u16Width,
    STF_U16 u16Height
    );

extern
STF_RESULT STFLIB_ISP_DEVICE_StructInitialize_2(
    ST_CI_DEVICE *pstDevice,
    EN_DEV_ID enDevId,
    STF_BOOL8 bForceGenDev,
    EN_MEM_TYPE enMemType,
    STF_U16 u16Width,
    STF_U16 u16Height,
    ST_RECT_2 stCropRect,
    STF_U8 u8Fps
    );

extern
STF_RESULT STFLIB_ISP_DEVICE_StructInitializeWithDeviceName(
    ST_CI_DEVICE *pstDevice,
    STF_CHAR *pszDeviceName,
    STF_BOOL8 bForceGenDev,
    EN_MEM_TYPE enMemType,
    STF_U16 u16Width,
    STF_U16 u16Height
    );

extern
STF_RESULT STFLIB_ISP_DEVICE_StructInitializeWithDeviceName_2(
    ST_CI_DEVICE *pstDevice,
    STF_CHAR *pszDeviceName,
    STF_BOOL8 bForceGenDev,
    EN_MEM_TYPE enMemType,
    STF_U16 u16Width,
    STF_U16 u16Height,
    ST_RECT_2 stCropRect,
    STF_U8 u8Fps
    );

extern
STF_RESULT STFLIB_ISP_DEVICE_StructUninitialize(
    ST_CI_DEVICE *pstDevice
    );

extern
STF_RESULT STFLIB_ISP_DEVICE_GenerateDeviceTable(
    STF_CHAR *pszDriverName,
    STF_CHAR *pszModelName
    );


#ifdef __cplusplus
}
#endif


#endif //__STFLIB_ISP_DEVICE_H__
