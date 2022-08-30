// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2021 StarFive Technology Co., Ltd.
 */

#include <getopt.h>
#include <signal.h>
#include <inttypes.h>
#include <stdbool.h>
#include <libv4l2.h>
#include <poll.h>
#include "common.h"
#include "yuv.h"
#include "convert.h"
#include "stf_v4l2.h"
#include "stf_framebuffer.h"
#include "stf_drm.h"
#include "stf_log.h"

#include "ci/ci_api_structs.h"
#include "ci/ci_api.h"
#include "ISPC/stflib_isp_device.h"

#define USE_ISP_SDK_LIB
#define USE_ISP_SDK_LIB_FOR_VDO
#define USE_ISP_SDK_LIB_FOR_FB_AND_PP
#define USE_ISP_SDK_LIB_FOR_DRM

#define FB_DEVICE_NAME       "/dev/fb0"
#define STFBC_DEVICE_NAME    "/dev/stfbcdev"
#define DRM_DEVICE_NAME      "/dev/dri/card0"
#define V4L2_DFT_DEVICE_NAME "/dev/video0"
#define INNO_HDMI_CONNECTOR_ID  116
#define MIPI_RGB_CONNECTOR_ID   118

typedef struct enum_value_t {
  int    value;
  const char *name;
} enum_value_t;

static const enum_value_t g_disp_values[] = {
    { STF_DISP_NONE, "NONE"},
    { STF_DISP_FB,   "FB"},
    { STF_DISP_DRM,  "DRM"}
};

static const enum_value_t g_iomthd_values[] = {
    { IO_METHOD_MMAP,    "MMAP"},
    { IO_METHOD_USERPTR, "USERPTR"},
    { IO_METHOD_DMABUF,  "DMABUF"},
    { IO_METHOD_READ,    "READ"}
};

//Only support for using drm mmap dmabuf 0 & 1
typedef struct {
    volatile uint8_t readable[2];
    volatile uint8_t foreground_index;
} pingpong_buffer_index_t;


typedef struct {
#if defined(USE_ISP_SDK_LIB)
  #if defined(USE_ISP_SDK_LIB_FOR_VDO)
    ST_CI_DEVICE stVdoDevice;
  #endif //#if defined(USE_ISP_SDK_LIB_FOR_VDO)
  #if defined(USE_ISP_SDK_LIB_FOR_FB_AND_PP)
    ST_CI_DEVICE stFbDevice;
    ST_CI_DEVICE stPpDevice;
  #endif //#if defined(USE_ISP_SDK_LIB_FOR_FB_AND_PP)
  #if defined(USE_ISP_SDK_LIB_FOR_DRM)
    ST_CI_DEVICE stDrmDevice;
  #endif //#if defined(USE_ISP_SDK_LIB_FOR_DRM)
  #if defined(USE_ISP_SDK_LIB_FOR_VDO) || defined(USE_ISP_SDK_LIB_FOR_DRM)
    ST_DMA_BUF_INFO stDmaBufInfo[BUFCOUNT];
  #endif //#if defined(USE_ISP_SDK_LIB_FOR_VDO) || defined(USE_ISP_SDK_LIB_FOR_DRM)
#endif //#if defined(USE_ISP_SDK_LIB)
    V4l2Param_t v4l2_param;
    FBParam_t fb_param;
    DRMParam_t drm_param;

    enum STF_DISP_TYPE disp_type;
    enum IOMethod    io_mthd;
    int continuous;

    uint8_t jpegQuality;
    char* jpegFilename;
    FILE *rec_fp;

    int dmabufs[BUFCOUNT];  // for dmabuf use, mmap not use it
} ConfigParam_t;
ConfigParam_t *gp_cfg_param = NULL;

static int g_drm_buf_next_idx = -1;
static int g_drm_buf_curr_idx = 0;
#if defined(ENABLE_DRM_MMAP_PING_PONG_BUF)
static pingpong_buffer_index_t g_pp_index = {
    .readable = { 0, 0 },
    .foreground_index = 0,
};
#endif //#if defined(ENABLE_DRM_MMAP_PING_PONG_BUF)

static void alloc_default_config(ConfigParam_t **pp_data)
{
    ConfigParam_t *cfg_param = NULL;
    cfg_param = malloc(sizeof(*cfg_param));
    if (!cfg_param) {
        errno_exit("malloc");
    }
    memset(cfg_param, 0, sizeof(*cfg_param));

    cfg_param->disp_type = STF_DISP_NONE;
    cfg_param->continuous = 0;
    cfg_param->jpegQuality = 70;
    cfg_param->io_mthd = IO_METHOD_MMAP;

    cfg_param->v4l2_param.device_name = V4L2_DFT_DEVICE_NAME;
    cfg_param->v4l2_param.fd = -1;
    cfg_param->v4l2_param.io_mthd = cfg_param->io_mthd;
    cfg_param->v4l2_param.width = 1920;
    cfg_param->v4l2_param.height = 1080;
    cfg_param->v4l2_param.image_size = cfg_param->v4l2_param.width *
                cfg_param->v4l2_param.height * 3 / 2;
    cfg_param->v4l2_param.format = V4L2_PIX_FMT_NV12; // V4L2_PIX_FMT_RGB565
    cfg_param->v4l2_param.fps = 30;

    // the fb param will be updated after fb init
    cfg_param->fb_param.fd = -1;
    cfg_param->fb_param.pixformat = COLOR_YUV420_NV21; // COLOR_RGB565
    cfg_param->fb_param.width = 1920;
    cfg_param->fb_param.height = 1080;
    cfg_param->fb_param.bpp = 16;
    cfg_param->fb_param.screen_size = cfg_param->fb_param.width *
                cfg_param->fb_param.height * cfg_param->fb_param.bpp / 8;
    cfg_param->drm_param.connector_id = 0;

    *pp_data = cfg_param;
}

static void check_cfg_params(ConfigParam_t *cfg_param)
{
    LOG(STF_LEVEL_TRACE, "Enter\n");
    int disp_type = cfg_param->disp_type;
    int io_mthd = cfg_param->io_mthd;
    int ret = EXIT_FAILURE;

    assert(disp_type >= STF_DISP_NONE && disp_type <= STF_DISP_DRM);
    assert(io_mthd >= IO_METHOD_MMAP && io_mthd <= IO_METHOD_READ);

    // when mmap, support display NONE, DRM, FB
    if (IO_METHOD_MMAP == io_mthd) {
        ret = EXIT_SUCCESS;
    }

    // when dmabuf, only support DRM, and not save file
    if (IO_METHOD_DMABUF == io_mthd
            && STF_DISP_DRM == disp_type
            && !cfg_param->jpegFilename) {
        ret = EXIT_SUCCESS;
    }

    if (EXIT_FAILURE == ret) {
        LOG(STF_LEVEL_ERR, "Not support: io method is %s, display type is %s\n",
                g_iomthd_values[io_mthd].name, g_disp_values[disp_type].name);
        exit(EXIT_FAILURE);
    }

    LOG(STF_LEVEL_TRACE, "Exit\n");
}

/**
SIGINT interput handler
*/
void StopContCapture(int sig_id) {
    LOG(STF_LEVEL_INFO, "stoping continuous capture\n");
    gp_cfg_param->continuous = 0;
}

void InstallSIGINTHandler() {
    struct sigaction sa;
    CLEAR(sa);

    sa.sa_handler = StopContCapture;
    if (sigaction(SIGINT, &sa, 0) != 0) {
        LOG(STF_LEVEL_ERR, "could not install SIGINT handler, continuous capture disabled\n");
        gp_cfg_param->continuous = 0;
    }
}

/**
    process image read, recommand NV21 or NV12
    TODO: use ffmpeg or opencv to convert the image format
*/
static void imageProcess(const uint8_t* inbuf, uint8_t* outbuf,
        struct timeval timestamp)
{
#if defined(USE_ISP_SDK_LIB)
  #if defined(USE_ISP_SDK_LIB_FOR_FB_AND_PP)
    ST_FB_DEV_PARAM *pstFbDevParam = (ST_FB_DEV_PARAM *)gp_cfg_param->stFbDevice.pvDevParam;
  #endif //#if defined(USE_ISP_SDK_LIB_FOR_FB_AND_PP)
  #if defined(USE_ISP_SDK_LIB_FOR_DRM)
    ST_DRM_DEV_PARAM *pstDrmDevParam = (ST_DRM_DEV_PARAM *)gp_cfg_param->stDrmDevice.pvDevParam;
  #endif //#if defined(USE_ISP_SDK_LIB_FOR_DRM)
#endif //#if defined(USE_ISP_SDK_LIB)
    //timestamp.tv_sec
    //timestamp.tv_usec
    int in_width = gp_cfg_param->v4l2_param.width;
    int in_height = gp_cfg_param->v4l2_param.height;
    int in_imagesize = gp_cfg_param->v4l2_param.image_size;
    uint32_t in_format = gp_cfg_param->v4l2_param.format;
    char* jpegFilename = gp_cfg_param->jpegFilename;
    int disp_type = gp_cfg_param->disp_type;
    int new_in_format = 0;
    int out_format = 0;
    int out_size = 0;
    uint8_t* dst = malloc(in_width * in_height * 3);
    int is_yuv420sp = 0;  // NOTE: NV21 or NV12, it is special for starfive framebuffer
    static int s_frmcnt = 0;

    if (STF_DISP_FB == disp_type) {
        new_in_format = v4l2fmt_to_fbfmt(in_format);
#if defined(USE_ISP_SDK_LIB) && defined(USE_ISP_SDK_LIB_FOR_FB_AND_PP)
        out_format = pstFbDevParam->u32PixelFormat;
        out_size = pstFbDevParam->u32ScreenSize;
        is_yuv420sp = pstFbDevParam->stFbVarScreenInfo.grayscale;
#else
        out_format = gp_cfg_param->fb_param.pixformat;
        out_size = gp_cfg_param->fb_param.screen_size;
        is_yuv420sp = gp_cfg_param->fb_param.vinfo.grayscale;
#endif //#if defined(USE_ISP_SDK_LIB) && defined(USE_ISP_SDK_LIB_FOR_FB_AND_PP)
    } else if (STF_DISP_DRM == disp_type) {
        new_in_format = v4l2fmt_to_drmfmt(in_format);
#if defined(USE_ISP_SDK_LIB) && defined(USE_ISP_SDK_LIB_FOR_DRM)
        out_format = pstDrmDevParam->pstDrmDevHead->u32DrmFormat;
        out_size = pstDrmDevParam->pstDrmDevHead->stDrmBuf[0].u32Size;
#else
        out_format = gp_cfg_param->drm_param.dev_head->drm_format;
        out_size = gp_cfg_param->drm_param.dev_head->bufs[0].size;
#endif //#if defined(USE_ISP_SDK_LIB) && defined(USE_ISP_SDK_LIB_FOR_DRM)
    }

    LOG(STF_LEVEL_LOG, "in_width=%d, in_height=%d, in_imagesize=%d, out_size=%d, p=%p\n",
            in_width, in_height, in_imagesize, out_size, inbuf);

    // write jpeg
    char filename[512];
    switch (in_format) {
        case V4L2_PIX_FMT_YUV420:
            // if (jpegFilename) {
            //     // sprintf(filename, "%d-yuv420-%s", s_frmcnt, jpegFilename);
            //     // YUV420toYUV444(in_width, in_height, inbuf, dst);
            //     // jpegWrite(dst, filename);
            //     sprintf(filename, "raw-%d-yuv420-%s", s_frmcnt, jpegFilename);
            //     write_file(filename, inbuf, in_imagesize);
            //     s_frmcnt++;
            // }
            if (gp_cfg_param->jpegFilename && gp_cfg_param->rec_fp) {
                fwrite(inbuf, in_imagesize, 1, gp_cfg_param->rec_fp);
            }
            break;
        case V4L2_PIX_FMT_YUYV:
        case V4L2_PIX_FMT_YVYU:
            // if (jpegFilename) {
            //     // sprintf(filename, "%d-yuv422-%s", s_frmcnt, jpegFilename);
            //     // YUV422toYUV444(in_width, in_height, inbuf, dst);
            //     // jpegWrite(dst, filename);
            //     sprintf(filename, "raw-%d-yuv422-%s", s_frmcnt, jpegFilename);
            //     write_file(filename, inbuf, in_imagesize);
            //     s_frmcnt++;
            // }
            if (gp_cfg_param->jpegFilename && gp_cfg_param->rec_fp) {
                fwrite(inbuf, in_imagesize, 1, gp_cfg_param->rec_fp);
            }
            if (outbuf) {
                if (out_format == new_in_format) {
                    yuyv_resize(inbuf, outbuf, in_width, in_height);
                } else if ((STF_DISP_FB == disp_type) && is_yuv420sp) {
                    convert_yuyv_to_nv12(inbuf, outbuf, in_width, in_height, 1);
                } else if ((STF_DISP_DRM == disp_type)
                    && (out_format == V4L2_PIX_FMT_NV12)) {
                    convert_yuyv_to_nv12(inbuf, outbuf, in_width, in_height, 1);
                } else {
                    convert_yuyv_to_rgb(inbuf, outbuf, in_width, in_height, 0);
                }
            }
            break;
        case V4L2_PIX_FMT_NV21:
            // if (gp_cfg_param->jpegFilename) {
            //     // sprintf(filename, "%d-nv21-%s", s_frmcnt, gp_cfg_param->jpegFilename);
            //     // YUV420NV21toYUV444(in_width, in_height, inbuf, dst, 0);
            //     // jpegWrite(dst, filename);
            //     sprintf(filename, "raw-nv21-%s",  gp_cfg_param->jpegFilename);
            //     write_file(filename, inbuf, in_imagesize);
            //     s_frmcnt++;
            // }
            if (gp_cfg_param->jpegFilename && gp_cfg_param->rec_fp) {
                fwrite(inbuf, in_imagesize, 1, gp_cfg_param->rec_fp);
            }
            LOG(STF_LEVEL_LOG, "out_format=%d, new_in_format=%d, is_yuv420sp=%d\n", out_format,
                    new_in_format, is_yuv420sp);
            if (outbuf) {
                if (out_format == new_in_format) {
                    convert_nv21_to_nv12(inbuf, outbuf, in_width, in_height, 0);
                } else if ((STF_DISP_FB == disp_type) && is_yuv420sp) {
                    convert_nv21_to_nv12(inbuf, outbuf, in_width, in_height, 1);
                } else {
                    convert_nv21_to_nv12(inbuf, outbuf, in_width, in_height, 1);
                }
            }
            break;
        case V4L2_PIX_FMT_NV12:
            // if (jpegFilename) {
            //     // sprintf(filename, "%d-nv12-%s", s_frmcnt, jpegFilename);
            //     // YUV420NV12toYUV444(in_width, in_height, inbuf, dst);
            //     // jpegWrite(dst, filename);
            //     sprintf(filename, "raw-%d-nv12-%s", s_frmcnt, jpegFilename);
            //     write_file(filename, inbuf, in_imagesize);
            //     s_frmcnt++;
            // }
            if (gp_cfg_param->jpegFilename && gp_cfg_param->rec_fp) {
                fwrite(inbuf, in_imagesize, 1, gp_cfg_param->rec_fp);
            }
            LOG(STF_LEVEL_DEBUG, "out_format=%d, new_in_format=%d, is_yuv420sp=%d\n", out_format,
                    new_in_format, is_yuv420sp);
            if (outbuf) {
                if (out_format == new_in_format) {
                    convert_nv21_to_nv12(inbuf, outbuf, in_width, in_height, 0);
                } else if ((STF_DISP_FB == disp_type) && is_yuv420sp) {
                    convert_nv21_to_nv12(inbuf, outbuf, in_width, in_height, 1);
                } else {
                    convert_nv21_to_rgb(inbuf, outbuf, in_width, in_height, 0);
                }
            }
            break;
        case V4L2_PIX_FMT_RGB24:
            // if (jpegFilename) {
            //     // sprintf(filename, "%d-rgb-%s", s_frmcnt, jpegFilename);
            //     // RGB565toRGB888(in_width, in_height, inbuf, dst);
            //     // write_JPEG_file(filename, inbuf, in_width, in_height, jpegQuality);
            //     sprintf(filename, "raw-%d-rgb-%s", s_frmcnt, jpegFilename);
            //     write_file(filename, inbuf, in_imagesize);
            //     s_frmcnt++;
            // }
            if (gp_cfg_param->jpegFilename && gp_cfg_param->rec_fp) {
                fwrite(inbuf, in_imagesize, 1, gp_cfg_param->rec_fp);
            }
            if (outbuf) {
                convert_rgb888_to_rgb(inbuf, outbuf, in_width, in_height, 0);
            }
            break;
        case V4L2_PIX_FMT_RGB565:
            // if (jpegFilename) {
            //     // sprintf(filename, "%d-rgb565-%s", s_frmcnt, jpegFilename);
            //     // RGB565toRGB888(in_width, in_height, inbuf, dst);
            //     // write_JPEG_file(filename, dst, in_width, in_height, jpegQuality);
            //     sprintf(filename, "raw-%d-rgb565-%s", s_frmcnt, jpegFilename);
            //     write_file(filename, inbuf, in_imagesize);
            //     s_frmcnt++;
            // }
            if (gp_cfg_param->jpegFilename && gp_cfg_param->rec_fp) {
                fwrite(inbuf, in_imagesize, 1, gp_cfg_param->rec_fp);
            }
            if (outbuf) {
                if (out_format == new_in_format)
                    convert_rgb565_to_rgb(inbuf, outbuf, in_width, in_height, 0);
                else if ((STF_DISP_FB == disp_type) && is_yuv420sp)
                    convert_rgb565_to_nv12(inbuf, outbuf, in_width, in_height, 0);
                else
                    convert_rgb565_to_rgb(inbuf, outbuf, in_width, in_height, 0);
            }
            break;
        case V4L2_PIX_FMT_SRGGB12:
            if (jpegFilename)
                sprintf(filename, "raw-%d-RGGB12-%s", s_frmcnt, jpegFilename);
            else
                sprintf(filename, "raw-%d-RGGB12.raw", s_frmcnt);
            write_file(filename, inbuf, in_imagesize);
            RAW12toRAW16(in_width, in_height, inbuf, dst);
            if (jpegFilename)
                sprintf(filename, "raw-%d-RGGB16-%s", s_frmcnt, jpegFilename);
            else
                sprintf(filename, "raw-%d-RGGB16.raw", s_frmcnt);
            write_file(filename, (const uint8_t *)dst, in_width * in_height * 2);
            s_frmcnt++;
            break;
        case V4L2_PIX_FMT_SGRBG12:
            if (jpegFilename)
                sprintf(filename, "raw-%d-GRBG12-%s", s_frmcnt, jpegFilename);
            else
                sprintf(filename, "raw-%d-GRBG12.raw", s_frmcnt);
            write_file(filename, inbuf, in_imagesize);
            RAW12toRAW16(in_width, in_height, inbuf, dst);
            if (jpegFilename)
                sprintf(filename, "raw-%d-GRBG16-%s", s_frmcnt, jpegFilename);
            else
                sprintf(filename, "raw-%d-GRBG16.raw", s_frmcnt);
            write_file(filename, (const uint8_t *)dst, in_width * in_height * 2);
            s_frmcnt++;
            break;
        case V4L2_PIX_FMT_SGBRG12:
            if (jpegFilename)
                sprintf(filename, "raw-%d-GBRG12-%s", s_frmcnt, jpegFilename);
            else
                sprintf(filename, "raw-%d-GBRG12.raw", s_frmcnt);
            write_file(filename, inbuf, in_imagesize);
            RAW12toRAW16(in_width, in_height, inbuf, dst);
            if (jpegFilename)
                sprintf(filename, "raw-%d-GBRG16-%s", s_frmcnt, jpegFilename);
            else
                sprintf(filename, "raw-%d-GBRG16.raw", s_frmcnt);
            write_file(filename, (const uint8_t *)dst, in_width * in_height * 2);
            s_frmcnt++;
            break;
        case V4L2_PIX_FMT_SBGGR12:
            if (jpegFilename)
                sprintf(filename, "raw-%d-BGGR12-%s", s_frmcnt, jpegFilename);
            else
                sprintf(filename, "raw-%d-BGGR12.raw", s_frmcnt);
            write_file(filename, inbuf, in_imagesize);
            RAW12toRAW16(in_width, in_height, inbuf, dst);
            if (jpegFilename)
                sprintf(filename, "raw-%d-BGGR16-%s", s_frmcnt, jpegFilename);
            else
                sprintf(filename, "raw-%d-BGGR16.raw", s_frmcnt);
            write_file(filename, (const uint8_t *)dst, in_width * in_height * 2);
            s_frmcnt++;
            break;
        default:
            LOG(STF_LEVEL_ERR, "unknow in_format\n");
            break;
    }

    // free temporary image
    free(dst);
}

void calc_frame_fps()
{
    static uint32_t frm_cnt = 0;
    static struct timespec ts_old;
    struct timespec ts;
    uint32_t fps = 0;
    uint32_t diff_ms = 0;

    if (frm_cnt == 0) {
        clock_gettime(CLOCK_MONOTONIC, &ts_old);
    }
    if (frm_cnt++ >= 50) {
        clock_gettime(CLOCK_MONOTONIC, &ts);
        diff_ms = (ts.tv_sec - ts_old.tv_sec) * 1000 + (ts.tv_nsec - ts_old.tv_nsec) / 1000000;
        fps = 1000  * (frm_cnt - 1) / diff_ms;
        frm_cnt = 0;
        LOG(STF_LEVEL_INFO, "pipeline display fps=%d\n", fps);
    }
}

/**
    read single frame
*/
static int frameRead(void)
{
#if defined(USE_ISP_SDK_LIB)
  #if defined(USE_ISP_SDK_LIB_FOR_VDO)
    ST_CI_DEVICE *pstVdoDevice = &gp_cfg_param->stVdoDevice;
    ST_VDEV_PARAM *pstVDevParam = (ST_VDEV_PARAM *)gp_cfg_param->stVdoDevice.pvDevParam;
    ST_VDO_BUF stBuffer;
    ST_VDO_BUF_INFO stVideoBufferInfo[1] = { 0 };
    STF_INT s32BufCount;
  #endif //#if defined(USE_ISP_SDK_LIB_FOR_VDO)
  #if defined(USE_ISP_SDK_LIB_FOR_FB_AND_PP)
    ST_FB_DEV_PARAM *pstFbDevParam = (ST_FB_DEV_PARAM *)gp_cfg_param->stFbDevice.pvDevParam;
  #endif //#if defined(USE_ISP_SDK_LIB_FOR_FB_AND_PP)
  #if defined(USE_ISP_SDK_LIB_FOR_DRM)
    ST_DRM_DEV_PARAM *pstDrmDevParam = (ST_DRM_DEV_PARAM *)gp_cfg_param->stDrmDevice.pvDevParam;
  #endif //#if defined(USE_ISP_SDK_LIB_FOR_DRM)
    STF_RESULT Ret = STF_SUCCESS;
#endif //#if defined(USE_ISP_SDK_LIB)
    struct v4l2_buffer buf;
    V4l2Param_t *pv4l2_param = &gp_cfg_param->v4l2_param;
    uint8_t *dst = NULL;
    int background_index = 0;

    if (STF_DISP_FB == gp_cfg_param->disp_type) {
#if defined(USE_ISP_SDK_LIB) && defined(USE_ISP_SDK_LIB_FOR_FB_AND_PP)
        dst = pstFbDevParam->pvBuffer;
#else
        dst = gp_cfg_param->fb_param.screen_buf;
#endif //#if defined(USE_ISP_SDK_LIB) && defined(USE_ISP_SDK_LIB_FOR_FB_AND_PP)
    } else if (STF_DISP_DRM == gp_cfg_param->disp_type &&
            IO_METHOD_DMABUF != gp_cfg_param->io_mthd) {
  #if defined(ENABLE_DRM_MMAP_PING_PONG_BUF)
        background_index = !g_pp_index.foreground_index;
  #endif //#if defined(ENABLE_DRM_MMAP_PING_PONG_BUF)
        //Get ready to compose the backgound buffer
#if defined(USE_ISP_SDK_LIB) && defined(USE_ISP_SDK_LIB_FOR_DRM)
        dst = pstDrmDevParam->pstDrmDevHead->stDrmBuf[background_index].pu8Buf;
#else
        dst = gp_cfg_param->drm_param.dev_head->bufs[background_index].buf;
#endif //#if defined(USE_ISP_SDK_LIB) && defined(USE_ISP_SDK_LIB_FOR_DRM)
    } else {
        LOG(STF_LEVEL_LOG, "Not display\n");
    }

    switch (pv4l2_param->io_mthd) {
    case IO_METHOD_READ:
        {
#if defined(USE_ISP_SDK_LIB) && defined(USE_ISP_SDK_LIB_FOR_VDO)
            if (-1 == v4l2_read(
                pstVdoDevice->GetFileHandle(pstVdoDevice),
                pstVDevParam->stVdoMemTbl.pstVdoMem[0].pvBuffer,
                pstVDevParam->stVdoMemTbl.pstVdoMem[0].u32Length
                )) {
                switch (errno) {
                    case EAGAIN:
                        return 0;
                    case EIO:
                        // Could ignore EIO, see spec.
                        // fall through
                    default:
                        errno_exit("read");
                }
            }
#else
            if (-1 == v4l2_read(pv4l2_param->fd, pv4l2_param->pBuffers[0].start,
                    pv4l2_param->pBuffers[0].length)) {
                switch (errno) {
                case EAGAIN:
                    return 0;
                case EIO:
                    // Could ignore EIO, see spec.
                    // fall through
                default:
                    errno_exit("read");
                }
            }
#endif //#if defined(USE_ISP_SDK_LIB) && defined(USE_ISP_SDK_LIB_FOR_VDO)
            struct timespec ts;
            struct timeval timestamp;
            clock_gettime(CLOCK_MONOTONIC,&ts);
            timestamp.tv_sec = ts.tv_sec;
            timestamp.tv_usec = ts.tv_nsec/1000;
#if defined(USE_ISP_SDK_LIB) && defined(USE_ISP_SDK_LIB_FOR_VDO)
            imageProcess((uint8_t *)pstVDevParam->stVdoMemTbl.pstVdoMem[0].pvBuffer, dst, timestamp);
#else
            imageProcess((uint8_t *)(pv4l2_param->pBuffers[0].start), dst, timestamp);
#endif //#if defined(USE_ISP_SDK_LIB) && defined(USE_ISP_SDK_LIB_FOR_VDO)
        }
        break;

    case IO_METHOD_MMAP:
        {
#if defined(USE_ISP_SDK_LIB) && defined(USE_ISP_SDK_LIB_FOR_VDO)
            s32BufCount = -1;
            pstVdoDevice->DequeueBuf_2(
                pstVdoDevice,
                &s32BufCount,
                &stVideoBufferInfo[0]
                );
            if (0 < s32BufCount) {
                stBuffer = pstVdoDevice->GetBufInfo(pstVdoDevice,
                    stVideoBufferInfo[0].u8BufIdx);
                buf.index = stVideoBufferInfo[0].u8BufIdx;
                buf.timestamp = stVideoBufferInfo[0].stTimeStamp;
                buf.bytesused = stVideoBufferInfo[0].u32BytesUsed;
                LOG(STF_LEVEL_LOG, "buf.index=%d, n_buffers=%d\n",
                    stVideoBufferInfo[0].u8BufIdx,
                    gp_cfg_param->v4l2_param.n_buffers);
                imageProcess((uint8_t *)stBuffer.pvBuffer, dst, buf.timestamp);
  #if defined(ENABLE_DRM_MMAP_PING_PONG_BUF)
  #else
                Ret = pstVdoDevice->EnqueueBuf_2(pstVdoDevice, 1,
                    &stVideoBufferInfo[0]);
                if (STF_SUCCESS != Ret) {
                    LOG(STF_LEVEL_LOG, "Failed to enqueue the buffer "\
                        "into the \'SC_DUMP_YHIST\' video device.\n",
                        pstVdoDevice->szDeviceName);
                }
  #endif //#if defined(ENABLE_DRM_MMAP_PING_PONG_BUF)
                if (STF_DISP_DRM == gp_cfg_param->disp_type) {
  #if defined(ENABLE_DRM_MMAP_PING_PONG_BUF)
                    static int first_frame = 1;

                    g_pp_index.readable[background_index] = 1;
                    if (first_frame) {
                        g_pp_index.foreground_index = background_index;
    #if defined(USE_ISP_SDK_LIB) && defined(USE_ISP_SDK_LIB_FOR_DRM)
                        if (drmModeSetCrtc(
                            gp_cfg_param->stDrmDevice.GetFileHandle(&gp_cfg_param->stDrmDevice),
                            pstDrmDevParam->pstDrmDevHead->u32CrtId,
                            pstDrmDevParam->pstDrmDevHead->stDrmBuf[g_pp_index.foreground_index].u32FbId,
                            0, 0, &pstDrmDevParam->pstDrmDevHead->u32ConnId, 1,
                            &pstDrmDevParam->pstDrmDevHead->stDrmModeInfo
                            )) {
                            fatal("drmModeSetCrtc() failed");
                        }
                        /* First flip */
                        drmModePageFlip(
                            gp_cfg_param->stDrmDevice.GetFileHandle(&gp_cfg_param->stDrmDevice),
                            pstDrmDevParam->pstDrmDevHead->u32CrtId,
                            pstDrmDevParam->pstDrmDevHead->stDrmBuf[g_pp_index.foreground_index].u32FbId,
                            DRM_MODE_PAGE_FLIP_EVENT, pstDrmDevParam->pstDrmDevHead
                            );
    #else
                        if (drmModeSetCrtc(gp_cfg_param->drm_param.fd,
                            gp_cfg_param->drm_param.dev_head->crtc_id,
                            gp_cfg_param->drm_param.dev_head->bufs[g_pp_index.foreground_index].fb_id,
                            0, 0, &gp_cfg_param->drm_param.dev_head->conn_id, 1,
                            &gp_cfg_param->drm_param.dev_head->mode)) {
                            fatal("drmModeSetCrtc() failed");
                        }
                        /* First flip */
                        drmModePageFlip(gp_cfg_param->drm_param.fd,
                            gp_cfg_param->drm_param.dev_head->crtc_id,
                            gp_cfg_param->drm_param.dev_head->bufs[g_pp_index.foreground_index].fb_id,
                            DRM_MODE_PAGE_FLIP_EVENT, gp_cfg_param->drm_param.dev_head);
    #endif //#if defined(USE_ISP_SDK_LIB) && defined(USE_ISP_SDK_LIB_FOR_DRM)
                        first_frame = 0;
                    }
  #else
    #if defined(USE_ISP_SDK_LIB) && defined(USE_ISP_SDK_LIB_FOR_DRM)
                    drmModePageFlip(
                        gp_cfg_param->stDrmDevice.GetFileHandle(&gp_cfg_param->stDrmDevice),
                        pstDrmDevParam->pstDrmDevHead->u32CrtId,
                        pstDrmDevParam->pstDrmDevHead->stDrmBuf[background_index].u32FbId,
                        DRM_MODE_PAGE_FLIP_EVENT, pstDrmDevParam->pstDrmDevHead
                        );
    #else
                    drmModePageFlip(gp_cfg_param->drm_param.fd,
                        gp_cfg_param->drm_param.dev_head->crtc_id,
                        gp_cfg_param->drm_param.dev_head->bufs[background_index].fb_id,
                        DRM_MODE_PAGE_FLIP_EVENT, gp_cfg_param->drm_param.dev_head);
    #endif //#if defined(USE_ISP_SDK_LIB) && defined(USE_ISP_SDK_LIB_FOR_DRM)
  #endif //#if defined(ENABLE_DRM_MMAP_PING_PONG_BUF)
                }
  #if defined(ENABLE_DRM_MMAP_PING_PONG_BUF)
                Ret = pstVdoDevice->EnqueueBuf_2(pstVdoDevice, 1,
                    &stVideoBufferInfo[0]);
                if (STF_SUCCESS != Ret) {
                    LOG(STF_LEVEL_LOG, "Failed to enqueue the buffer "\
                        "into the \'SC_DUMP_YHIST\' video device.\n",
                        pstVdoDevice->szDeviceName);
                }
  #else
  #endif //#if defined(ENABLE_DRM_MMAP_PING_PONG_BUF)
                LOG(STF_LEVEL_LOG, "buf.index: %d, buf.bytesused=%d\n",
                    stVideoBufferInfo[0].u8BufIdx, buf.bytesused);
            } else {
                LOG(STF_LEVEL_LOG, "buf.index=%d, n_buffers=%d\n",
                    -1, gp_cfg_param->v4l2_param.n_buffers);
            }
#else
            stf_v4l2_dequeue_buffer(pv4l2_param, &buf);
            LOG(STF_LEVEL_LOG, "buf.index=%d, n_buffers=%d\n",
                buf.index, gp_cfg_param->v4l2_param.n_buffers);
            imageProcess((uint8_t *)(pv4l2_param->pBuffers[buf.index].start), dst, buf.timestamp);
  #if defined(ENABLE_DRM_MMAP_PING_PONG_BUF)
  #else
            stf_v4l2_queue_buffer(pv4l2_param, buf.index);
  #endif //#if defined(ENABLE_DRM_MMAP_PING_PONG_BUF)
            if (STF_DISP_DRM == gp_cfg_param->disp_type) {
  #if defined(ENABLE_DRM_MMAP_PING_PONG_BUF)
                static int first_frame = 1;

                g_pp_index.readable[background_index] = 1;
                if (first_frame) {
                    g_pp_index.foreground_index = background_index;
    #if defined(USE_ISP_SDK_LIB) && defined(USE_ISP_SDK_LIB_FOR_DRM)
                    if (drmModeSetCrtc(
                        gp_cfg_param->stDrmDevice.GetFileHandle(&gp_cfg_param->stDrmDevice),
                        pstDrmDevParam->pstDrmDevHead->u32CrtId,
                        pstDrmDevParam->pstDrmDevHead->stDrmBuf[g_pp_index.foreground_index].u32FbId,
                        0, 0, &pstDrmDevParam->pstDrmDevHead->u32ConnId, 1,
                        &pstDrmDevParam->pstDrmDevHead->stDrmModeInfo
                        )) {
                        fatal("drmModeSetCrtc() failed");
                    }
                    /* First flip */
                    drmModePageFlip(
                        gp_cfg_param->stDrmDevice.GetFileHandle(&gp_cfg_param->stDrmDevice),
                        pstDrmDevParam->pstDrmDevHead->u32CrtId,
                        pstDrmDevParam->pstDrmDevHead->stDrmBuf[g_pp_index.foreground_index].u32FbId,
                        DRM_MODE_PAGE_FLIP_EVENT, pstDrmDevParam->pstDrmDevHead
                        );
    #else
                    if (drmModeSetCrtc(gp_cfg_param->drm_param.fd,
                        gp_cfg_param->drm_param.dev_head->crtc_id,
                        gp_cfg_param->drm_param.dev_head->bufs[g_pp_index.foreground_index].fb_id,
                        0, 0, &gp_cfg_param->drm_param.dev_head->conn_id, 1,
                        &gp_cfg_param->drm_param.dev_head->mode)) {
                        fatal("drmModeSetCrtc() failed");
                    }
                    /* First flip */
                    drmModePageFlip(gp_cfg_param->drm_param.fd,
                        gp_cfg_param->drm_param.dev_head->crtc_id,
                        gp_cfg_param->drm_param.dev_head->bufs[g_pp_index.foreground_index].fb_id,
                        DRM_MODE_PAGE_FLIP_EVENT, gp_cfg_param->drm_param.dev_head);
    #endif //#if defined(USE_ISP_SDK_LIB) && defined(USE_ISP_SDK_LIB_FOR_DRM)
                    first_frame = 0;
                }
  #else
    #if defined(USE_ISP_SDK_LIB) && defined(USE_ISP_SDK_LIB_FOR_DRM)
                drmModePageFlip(
                    gp_cfg_param->stDrmDevice.GetFileHandle(&gp_cfg_param->stDrmDevice),
                    pstDrmDevParam->pstDrmDevHead->u32CrtId,
                    pstDrmDevParam->pstDrmDevHead->stDrmBuf[background_index].u32FbId,
                    DRM_MODE_PAGE_FLIP_EVENT, pstDrmDevParam->pstDrmDevHead
                    );
    #else
                drmModePageFlip(gp_cfg_param->drm_param.fd,
                    gp_cfg_param->drm_param.dev_head->crtc_id,
                    gp_cfg_param->drm_param.dev_head->bufs[background_index].fb_id,
                    DRM_MODE_PAGE_FLIP_EVENT, gp_cfg_param->drm_param.dev_head);
    #endif //#if defined(USE_ISP_SDK_LIB) && defined(USE_ISP_SDK_LIB_FOR_DRM)
  #endif //#if defined(ENABLE_DRM_MMAP_PING_PONG_BUF)
            }
  #if defined(ENABLE_DRM_MMAP_PING_PONG_BUF)
            stf_v4l2_queue_buffer(pv4l2_param, buf.index);
  #else
  #endif //#if defined(ENABLE_DRM_MMAP_PING_PONG_BUF)
            LOG(STF_LEVEL_LOG, "buf.index: %d, buf.bytesused=%d\n", buf.index, buf.bytesused);
#endif //#if defined(USE_ISP_SDK_LIB) && defined(USE_ISP_SDK_LIB_FOR_VDO)
        }
        break;

    case IO_METHOD_USERPTR:
        {
#if defined(USE_ISP_SDK_LIB) && defined(USE_ISP_SDK_LIB_FOR_VDO)
            s32BufCount = -1;
            pstVdoDevice->DequeueBuf_2(
                pstVdoDevice,
                &s32BufCount,
                &stVideoBufferInfo[0]
                );
            if (0 < s32BufCount) {
                stBuffer = pstVdoDevice->GetBufInfo(pstVdoDevice,
                    stVideoBufferInfo[0].u8BufIdx);
                buf.index = stVideoBufferInfo[0].u8BufIdx;
                buf.timestamp = stVideoBufferInfo[0].stTimeStamp;
                buf.bytesused = stVideoBufferInfo[0].u32BytesUsed;
                imageProcess(stBuffer.pvBuffer, dst, buf.timestamp);
                Ret = pstVdoDevice->EnqueueBuf_2(pstVdoDevice, 1,
                    &stVideoBufferInfo[0]);
                if (STF_SUCCESS != Ret) {
                    LOG(STF_LEVEL_LOG, "Failed to enqueue the buffer "\
                        "into the \'SC_DUMP_YHIST\' video device.\n",
                        pstVdoDevice->szDeviceName);
                }
            }
#else
            stf_v4l2_dequeue_buffer(pv4l2_param, &buf);
            imageProcess((uint8_t *)(buf.m.userptr), dst, buf.timestamp);
            stf_v4l2_queue_buffer(pv4l2_param, buf.index);
#endif //#if defined(USE_ISP_SDK_LIB) && defined(USE_ISP_SDK_LIB_FOR_VDO)
        }
        break;
    case IO_METHOD_DMABUF:
    default:
        break;
    }
    return 1;
}

/**
 *  mainloop_select: read frames with select() and process them
 */
static void mainloop_select(void)
{
#if defined(USE_ISP_SDK_LIB) && defined(USE_ISP_SDK_LIB_FOR_VDO)
    STF_INT nFd = -1;
#endif //#if defined(USE_ISP_SDK_LIB) && defined(USE_ISP_SDK_LIB_FOR_VDO)
    int count, i;
    uint32_t numberOfTimeouts;

    numberOfTimeouts = 0;
    count = 3;

    while (count-- > 0) {
        for (i = 0; i < 1; i++) {
            fd_set fds;
            struct timeval tv;
            int r;

            FD_ZERO(&fds);
#if defined(USE_ISP_SDK_LIB) && defined(USE_ISP_SDK_LIB_FOR_VDO)
            nFd = gp_cfg_param->stVdoDevice.GetFileHandle(
                &gp_cfg_param->stVdoDevice);
            FD_SET(nFd, &fds);
#else
            FD_SET(gp_cfg_param->v4l2_param.fd, &fds);
#endif //#if defined(USE_ISP_SDK_LIB) && defined(USE_ISP_SDK_LIB_FOR_VDO)

            /* Timeout. */
            tv.tv_sec = 1;
            tv.tv_usec = 0;
#if defined(USE_ISP_SDK_LIB) && defined(USE_ISP_SDK_LIB_FOR_VDO)
            r = select(nFd + 1, &fds, NULL, NULL, &tv);
#else
            r = select(gp_cfg_param->v4l2_param.fd + 1, &fds, NULL, NULL, &tv);
#endif //#if defined(USE_ISP_SDK_LIB) && defined(USE_ISP_SDK_LIB_FOR_VDO)
            if (-1 == r) {
                if (EINTR == errno) {
                    continue;
                }
                errno_exit("select");
            } else if (0 == r) {
                if (numberOfTimeouts <= 0) {
                    // count++;
                } else {
                    LOG(STF_LEVEL_ERR, "select timeout\n");
                    exit(EXIT_FAILURE);
                }
            }
            if (gp_cfg_param->continuous == 1) {
                count = 3;
            }

            if (frameRead())
                break;

            /* EAGAIN - continue select loop. */
        }
    }
}

#if defined(ENABLE_DRM_MMAP_PING_PONG_BUF)
static uint8_t check_background_buf_readable(void)
{
    return g_pp_index.readable[!g_pp_index.foreground_index];
}

#endif //#if defined(ENABLE_DRM_MMAP_PING_PONG_BUF)
static void mmap_page_flip_handler(int fd, unsigned int frame,
            unsigned int sec, unsigned int usec,
            void *data)
{
#if defined(USE_ISP_SDK_LIB)
  #if defined(USE_ISP_SDK_LIB_FOR_DRM)
    ST_CI_DEVICE *pstDrmDevice = &gp_cfg_param->stDrmDevice;
    ST_DRM_DEV_PARAM *pstDrmDevParam = (ST_DRM_DEV_PARAM *)gp_cfg_param->stDrmDevice.pvDevParam;
    ST_DRM_DEV *pstDrmDev = data;
  #else
    struct drm_dev_t *dev = data;
  #endif //#if defined(USE_ISP_SDK_LIB_FOR_DRM)
#else
    struct drm_dev_t *dev = data;
#endif //#if defined(USE_ISP_SDK_LIB)
    STF_INT nBackgroundIndex = 0;

#if defined(ENABLE_DRM_MMAP_PING_PONG_BUF)
    if (check_background_buf_readable()) {
        //Move background buffer to foreground
        g_pp_index.readable[g_pp_index.foreground_index] = 0;
        g_pp_index.foreground_index = !g_pp_index.foreground_index;
    }
    nBackgroundIndex = g_pp_index.foreground_index;
#endif //#if defined(ENABLE_DRM_MMAP_PING_PONG_BUF)
    LOG(STF_LEVEL_LOG, "mmap_page_flip_handler, nBackgroundIndex = %d\n",
        nBackgroundIndex);
#if defined(USE_ISP_SDK_LIB) && defined(USE_ISP_SDK_LIB_FOR_DRM)
  #if 0
    drmModePageFlip(fd, pstDrmDev->u32CrtId,
        pstDrmDev->stDrmBuf[nBackgroundIndex].u32FbId,
        DRM_MODE_PAGE_FLIP_EVENT, pstDrmDev);
  #else
    drmModePageFlip(pstDrmDevice->GetFileHandle(pstDrmDevice),
        pstDrmDevParam->pstDrmDevHead->u32CrtId,
        pstDrmDevParam->pstDrmDevHead->stDrmBuf[nBackgroundIndex].u32FbId,
        DRM_MODE_PAGE_FLIP_EVENT, pstDrmDevParam->pstDrmDevHead);
  #endif //#if 0
#else
  #if 0
    drmModePageFlip(fd, dev->crtc_id,
        dev->bufs[nBackgroundIndex].fb_id,
        DRM_MODE_PAGE_FLIP_EVENT, dev);
  #else
    drmModePageFlip(gp_cfg_param->drm_param.fd,
        gp_cfg_param->drm_param.dev_head->crtc_id,
        gp_cfg_param->drm_param.dev_head->bufs[nBackgroundIndex].fb_id,
        DRM_MODE_PAGE_FLIP_EVENT, gp_cfg_param->drm_param.dev_head);
  #endif //#if 0
#endif //#if defined(USE_ISP_SDK_LIB) && defined(USE_ISP_SDK_LIB_FOR_DRM)
}

static void page_flip_handler(int fd, unsigned int frame,
            unsigned int sec, unsigned int usec,
            void *data)
{
#if defined(USE_ISP_SDK_LIB)
  #if defined(USE_ISP_SDK_LIB_FOR_VDO)
    ST_CI_DEVICE *pstVdoDevice = &gp_cfg_param->stVdoDevice;
    ST_VDO_BUF_INFO stVideoBufferInfo[1] = { 0 };
  #endif //#if defined(USE_ISP_SDK_LIB_FOR_VDO)
  #if defined(USE_ISP_SDK_LIB_FOR_DRM)
    ST_CI_DEVICE *pstDrmDevice = &gp_cfg_param->stDrmDevice;
    ST_DRM_DEV_PARAM *pstDrmDevParam = (ST_DRM_DEV_PARAM *)gp_cfg_param->stDrmDevice.pvDevParam;
    ST_DRM_DEV *pstDrmDev = data;
  #else
    struct drm_dev_t *dev = data;
  #endif //#if defined(USE_ISP_SDK_LIB_FOR_DRM)
    STF_RESULT Ret = STF_SUCCESS;
#else
    struct drm_dev_t *dev = data;
#endif //#if defined(USE_ISP_SDK_LIB)

    /* If we have a next buffer, then let's return the current one,
        * and grab the next one.
        */
    if (g_drm_buf_next_idx >= 0) {
#if defined(USE_ISP_SDK_LIB) && defined(USE_ISP_SDK_LIB_FOR_VDO)
        stVideoBufferInfo[0].u8BufIdx = g_drm_buf_curr_idx;
        Ret = pstVdoDevice->EnqueueBuf_2(pstVdoDevice, 1,
            &stVideoBufferInfo[0]);
        if (STF_SUCCESS != Ret) {
            LOG(STF_LEVEL_LOG, "Failed to enqueue the buffer index = %d "\
                "into the \'%s\' video device.\n",
                g_drm_buf_curr_idx, pstVdoDevice->szDeviceName);
        }
#else
        stf_v4l2_queue_buffer(&gp_cfg_param->v4l2_param, g_drm_buf_curr_idx);
#endif //#if defined(USE_ISP_SDK_LIB) && defined(USE_ISP_SDK_LIB_FOR_VDO)
        g_drm_buf_curr_idx = g_drm_buf_next_idx;
        g_drm_buf_next_idx = -1;
    }
#if defined(USE_ISP_SDK_LIB) && defined(USE_ISP_SDK_LIB_FOR_DRM)
    drmModePageFlip(fd, pstDrmDev->u32CrtId,
        pstDrmDev->stDrmBuf[g_drm_buf_curr_idx].u32FbId,
        DRM_MODE_PAGE_FLIP_EVENT, pstDrmDev);
#else
    drmModePageFlip(fd, dev->crtc_id, dev->bufs[g_drm_buf_curr_idx].fb_id,
        DRM_MODE_PAGE_FLIP_EVENT, dev);
#endif //#if defined(USE_ISP_SDK_LIB) && defined(USE_ISP_SDK_LIB_FOR_DRM)
}

static void mainloop()
{
#if defined(USE_ISP_SDK_LIB)
  #if defined(USE_ISP_SDK_LIB_FOR_VDO)
    ST_CI_DEVICE *pstVdoDevice = &gp_cfg_param->stVdoDevice;
    ST_VDO_BUF_INFO stVideoBufferInfo[1] = { 0 };
    STF_INT s32BufCount;
  #endif //#if defined(USE_ISP_SDK_LIB_FOR_VDO)
  #if defined(USE_ISP_SDK_LIB_FOR_FB_AND_PP)
    ST_FB_DEV_PARAM *pstFbDevParam = (ST_FB_DEV_PARAM *)gp_cfg_param->stFbDevice.pvDevParam;
  #endif //#if defined(USE_ISP_SDK_LIB_FOR_FB_AND_PP)
  #if defined(USE_ISP_SDK_LIB_FOR_DRM)
    ST_CI_DEVICE *pstDrmDevice = &gp_cfg_param->stDrmDevice;
    ST_DRM_DEV_PARAM *pstDrmDevParam = (ST_DRM_DEV_PARAM *)gp_cfg_param->stDrmDevice.pvDevParam;
  #endif //#if defined(USE_ISP_SDK_LIB_FOR_DRM)
#endif //#if defined(USE_ISP_SDK_LIB)
    struct v4l2_buffer buf;
    int r;
    int count = 3;
    drmEventContext ev;
    struct pollfd* fds = NULL;
    uint32_t nfds = 0;

    LOG(STF_LEVEL_TRACE, "Enter\n");
    if (STF_DISP_FB == gp_cfg_param->disp_type ||
        STF_DISP_NONE == gp_cfg_param->disp_type) {
        // fb
        nfds = 1;
        fds = (struct pollfd*)malloc(sizeof(struct pollfd) * nfds);
        memset(fds, 0, sizeof(struct pollfd) * nfds);
#if defined(USE_ISP_SDK_LIB) && defined(USE_ISP_SDK_LIB_FOR_VDO)
        fds[0].fd = pstVdoDevice->GetFileHandle(pstVdoDevice);
        fds[0].events = POLLIN;
#else
        fds[0].fd = gp_cfg_param->v4l2_param.fd;
        fds[0].events = POLLIN;
#endif //#if defined(USE_ISP_SDK_LIB) && defined(USE_ISP_SDK_LIB_FOR_VDO)
    } else if (STF_DISP_DRM == gp_cfg_param->disp_type &&
        IO_METHOD_MMAP == gp_cfg_param->io_mthd) {
        // drm + mmap
        nfds = 2;
        fds = (struct pollfd*)malloc(sizeof(struct pollfd) * nfds);
        memset(fds, 0, sizeof(struct pollfd) * nfds);

#if defined(USE_ISP_SDK_LIB)
  #if defined(USE_ISP_SDK_LIB_FOR_VDO)
        fds[0].fd = pstVdoDevice->GetFileHandle(pstVdoDevice);
        fds[0].events = POLLIN;
  #else
        fds[0].fd = gp_cfg_param->v4l2_param.fd;
        fds[0].events = POLLIN;
  #endif //#if defined(USE_ISP_SDK_LIB_FOR_VDO)
  #if defined(USE_ISP_SDK_LIB_FOR_DRM)
        fds[1].fd = pstDrmDevice->GetFileHandle(pstDrmDevice);
        fds[1].events = POLLIN;
  #else
        fds[1].fd = gp_cfg_param->drm_param.fd;
        fds[1].events = POLLIN;
  #endif //#if defined(USE_ISP_SDK_LIB_FOR_DRM)
#else
        fds[0].fd = gp_cfg_param->v4l2_param.fd;
        fds[0].events = POLLIN;
        fds[1].fd = gp_cfg_param->drm_param.fd;
        fds[1].events = POLLIN;
#endif //#if defined(USE_ISP_SDK_LIB)

        memset(&ev, 0, sizeof ev);
        ev.version = DRM_EVENT_CONTEXT_VERSION;
        ev.vblank_handler = NULL;
        ev.page_flip_handler = mmap_page_flip_handler;
    } else if (STF_DISP_DRM == gp_cfg_param->disp_type &&
        IO_METHOD_DMABUF == gp_cfg_param->io_mthd) {
        // (drm + dmabuf)
        nfds = 2;
        fds = (struct pollfd*)malloc(sizeof(struct pollfd) * nfds);
        memset(fds, 0, sizeof(struct pollfd) * nfds);
#if defined(USE_ISP_SDK_LIB)
  #if defined(USE_ISP_SDK_LIB_FOR_VDO)
        fds[0].fd = pstVdoDevice->GetFileHandle(pstVdoDevice);
        fds[0].events = POLLIN;
  #else
        fds[0].fd = gp_cfg_param->v4l2_param.fd;
        fds[0].events = POLLIN;
  #endif //#if defined(USE_ISP_SDK_LIB_FOR_VDO)
  #if defined(USE_ISP_SDK_LIB_FOR_DRM)
        fds[1].fd = pstDrmDevice->GetFileHandle(pstDrmDevice);
        fds[1].events = POLLIN;
  #else
        fds[1].fd = gp_cfg_param->drm_param.fd;
        fds[1].events = POLLIN;
  #endif //#if defined(USE_ISP_SDK_LIB_FOR_DRM)
#else
        fds[0].fd = gp_cfg_param->v4l2_param.fd;
        fds[0].events = POLLIN;
        fds[1].fd = gp_cfg_param->drm_param.fd;
        fds[1].events = POLLIN;
#endif //#if defined(USE_ISP_SDK_LIB)

        memset(&ev, 0, sizeof ev);
        ev.version = DRM_EVENT_CONTEXT_VERSION;
        ev.vblank_handler = NULL;
        ev.page_flip_handler = page_flip_handler;
    } else {
        LOG(STF_LEVEL_ERR, "Display type %d and io method type %d not support\n",
                gp_cfg_param->disp_type, gp_cfg_param->io_mthd);
        exit(EXIT_FAILURE);
    }

    if (!gp_cfg_param->rec_fp &&
        gp_cfg_param->jpegFilename &&
        !is_raw_v4l2fmt(gp_cfg_param->v4l2_param.format)) {
        gp_cfg_param->rec_fp = fopen(gp_cfg_param->jpegFilename, "w+");
        if (!gp_cfg_param->rec_fp) {
            LOG(STF_LEVEL_ERR, "can't open %s\n", gp_cfg_param->jpegFilename);
            exit(EXIT_FAILURE);
        }
    }

    while (count-- > 0) {
        r = poll(fds, nfds, 3000);
        if (-1 == r) {
            if (EINTR == errno) {
                continue;
            }
            LOG(STF_LEVEL_ERR, "error in poll %d", errno);
            break;
        }
        if (0 == r) {
            LOG(STF_LEVEL_ERR, "poll timeout, %d\n", errno);
            break;
        }

        if (STF_DISP_FB == gp_cfg_param->disp_type ||
            STF_DISP_NONE == gp_cfg_param->disp_type) {
            // fb
            if (fds[0].revents & POLLIN) {
                frameRead();
                calc_frame_fps();
            }
        } else if (STF_DISP_DRM == gp_cfg_param->disp_type &&
            IO_METHOD_MMAP == gp_cfg_param->io_mthd) {
            // drm + mmap
            if (fds[0].revents & POLLIN) {
                frameRead();
                calc_frame_fps();
            }

            if (fds[1].revents & POLLIN) {
#if defined(USE_ISP_SDK_LIB) && defined(USE_ISP_SDK_LIB_FOR_DRM)
                drmHandleEvent(
                    gp_cfg_param->stDrmDevice.GetFileHandle(&gp_cfg_param->stDrmDevice),
                    &ev
                    );
#else
                drmHandleEvent(gp_cfg_param->drm_param.fd, &ev);
#endif //#if defined(USE_ISP_SDK_LIB) && defined(USE_ISP_SDK_LIB_FOR_DRM)
            }
        } else if (STF_DISP_DRM == gp_cfg_param->disp_type &&
            IO_METHOD_DMABUF == gp_cfg_param->io_mthd) {
            // drm + dmabuf
            if (fds[0].revents & POLLIN) {
#if defined(USE_ISP_SDK_LIB) && defined(USE_ISP_SDK_LIB_FOR_VDO)
                s32BufCount = -1;
                pstVdoDevice->DequeueBuf_2(
                    pstVdoDevice,
                    &s32BufCount,
                    &stVideoBufferInfo[0]
                    );
                if (0 < s32BufCount) {
  #if defined(ENABLE_DRM_MMAP_PING_PONG_BUF)
                    static int first_frame = 1;

                    if (first_frame) {
    #if defined(USE_ISP_SDK_LIB) && defined(USE_ISP_SDK_LIB_FOR_DRM)
                        /* First buffer to DRM */
                        if (drmModeSetCrtc(
                            gp_cfg_param->stDrmDevice.GetFileHandle(&gp_cfg_param->stDrmDevice),
                            pstDrmDevParam->pstDrmDevHead->u32CrtId,
                            pstDrmDevParam->pstDrmDevHead->stDrmBuf[stVideoBufferInfo[0].u8BufIdx].u32FbId,
                            0, 0, &pstDrmDevParam->pstDrmDevHead->u32ConnId, 1,
                            &pstDrmDevParam->pstDrmDevHead->stDrmModeInfo
                            )) {
                            fatal("drmModeSetCrtc() failed");
                        }
                        /* First flip */
                        drmModePageFlip(
                            gp_cfg_param->stDrmDevice.GetFileHandle(&gp_cfg_param->stDrmDevice),
                            pstDrmDevParam->pstDrmDevHead->u32CrtId,
                            pstDrmDevParam->pstDrmDevHead->stDrmBuf[stVideoBufferInfo[0].u8BufIdx].u32FbId,
                            DRM_MODE_PAGE_FLIP_EVENT, pstDrmDevParam->pstDrmDevHead
                            );
    #else
                        /* First buffer to DRM */
                        if (drmModeSetCrtc(gp_cfg_param->drm_param.fd,
                            gp_cfg_param->drm_param.dev_head->crtc_id,
                            gp_cfg_param->drm_param.dev_head->bufs[stVideoBufferInfo[0].u8BufIdx].fb_id,
                            0, 0, &gp_cfg_param->drm_param.dev_head->conn_id, 1,
                            &gp_cfg_param->drm_param.dev_head->mode)) {
                            fatal("drmModeSetCrtc() failed");
                        }
                        /* First flip */
                        drmModePageFlip(gp_cfg_param->drm_param.fd,
                            gp_cfg_param->drm_param.dev_head->crtc_id,
                            gp_cfg_param->drm_param.dev_head->bufs[stVideoBufferInfo[0].u8BufIdx].fb_id,
                            DRM_MODE_PAGE_FLIP_EVENT, gp_cfg_param->drm_param.dev_head);
    #endif //#if defined(USE_ISP_SDK_LIB) && defined(USE_ISP_SDK_LIB_FOR_DRM)
                        g_drm_buf_curr_idx = stVideoBufferInfo[0].u8BufIdx;
                        first_frame = 0;
                    } else {
                        g_drm_buf_next_idx = stVideoBufferInfo[0].u8BufIdx;
                    }
  #else
                    g_drm_buf_next_idx = stVideoBufferInfo[0].u8BufIdx;
  #endif //#if defined(ENABLE_DRM_MMAP_PING_PONG_BUF)
                    frameRead(); // TODO: add support for save file later
                    calc_frame_fps();
                }
#else
                int dequeued = stf_v4l2_dequeue_buffer(&gp_cfg_param->v4l2_param, &buf);
                if (dequeued) {
  #if defined(ENABLE_DRM_MMAP_PING_PONG_BUF)
                    static int first_frame = 1;

                    if (first_frame) {
                        /* First buffer to DRM */
                        if (drmModeSetCrtc(gp_cfg_param->drm_param.fd,
                            gp_cfg_param->drm_param.dev_head->crtc_id,
                            gp_cfg_param->drm_param.dev_head->bufs[buf.index].fb_id,
                            0, 0, &gp_cfg_param->drm_param.dev_head->conn_id, 1,
                            &gp_cfg_param->drm_param.dev_head->mode)) {
                            fatal("drmModeSetCrtc() failed");
                        }
                        /* First flip */
                        drmModePageFlip(gp_cfg_param->drm_param.fd,
                            gp_cfg_param->drm_param.dev_head->crtc_id,
                            gp_cfg_param->drm_param.dev_head->bufs[buf.index].fb_id,
                            DRM_MODE_PAGE_FLIP_EVENT, gp_cfg_param->drm_param.dev_head);
                        g_drm_buf_curr_idx = buf.index;
                        first_frame = 0;
                    } else {
                        g_drm_buf_next_idx = buf.index;
                    }
  #else
                    g_drm_buf_next_idx = buf.index;
  #endif //#if defined(ENABLE_DRM_MMAP_PING_PONG_BUF)
                    frameRead(); // TODO: add support for save file later
                    calc_frame_fps();
                }
#endif //#if defined(USE_ISP_SDK_LIB) && defined(USE_ISP_SDK_LIB_FOR_VDO)
            }
            if (fds[1].revents & POLLIN) {
                if (g_drm_buf_next_idx >= 0) {
#if defined(USE_ISP_SDK_LIB) && defined(USE_ISP_SDK_LIB_FOR_DRM)
                    drmHandleEvent(pstDrmDevice->GetFileHandle(pstDrmDevice),
                        &ev);
#else
                    drmHandleEvent(gp_cfg_param->drm_param.fd, &ev);
#endif //#if defined(USE_ISP_SDK_LIB) && defined(USE_ISP_SDK_LIB_FOR_DRM)
                }
            }
        }

        if (gp_cfg_param->continuous == 1) {
            count = 3;
        }

        usleep(1 * 1000);
    }

    if (fds) {
        free(fds);
        fds = NULL;
    }


    if (gp_cfg_param->rec_fp) {
        fclose(gp_cfg_param->rec_fp);
        gp_cfg_param->rec_fp = NULL;
    }


    LOG(STF_LEVEL_TRACE, "Exit\n");
}

static void usage(FILE* fp, int argc, char** argv)
{
    fprintf(fp,
        "Usage: %s [options]\n\n"
        "Options:\n"
        "-d | --device name   Video device name [default /dev/video0]\n"
        "-h | --help          Print this message\n"
        "-o | --output        Save raw data to filename\n"
        //"-q | --quality       Set JPEG quality (0-100)\n"
        "-m | --method        Set V4L2 videobuf2 memory type, default 0\n"
        "                0: IO_METHOD_MMAP\n"
        "                1: IO_METHOD_USERPTR\n"
        "                2: IO_METHOD_DMABUF\n"
        "                3: IO_METHOD_READ\n"
        "-W | --width         Set v4l2 image width, default 1920\n"
        "-H | --height        Set v4l2 image height, default 1080\n"
        "-X | --left          Set v4l2 image crop x start\n"
        "-Y | --up            Set v4l2 image crop y start\n"
        "-R | --right         Set v4l2 image crop x width\n"
        "-D | --down          Set v4l2 image crop y height\n"
        "-I | --interval      Set frame interval (fps) (-1 to skip)\n"
        "-c | --continuous    Do continous capture, stop with SIGINT.\n"
        "-C | --connector     Display Connector.\n"
        "                0: INNO HDMI\n"
        "                1: MIPI/RGB HDMI\n"
        "-v | --version       Print version\n"
        "-f | --format        image format, default 5\n"
        "                0: V4L2_PIX_FMT_RGB565\n"
        "                1: V4L2_PIX_FMT_RGB24\n"
        "                2: V4L2_PIX_FMT_YUV420\n"
        "                3: V4L2_PIX_FMT_YUYV\n"
        "                4: V4L2_PIX_FMT_NV21\n"
        "                5: V4L2_PIX_FMT_NV12\n"
        "                6: V4L2_PIX_FMT_YVYU\n"
        "                7: V4L2_PIX_FMT_SRGGB12\n"
        "                8: V4L2_PIX_FMT_SGRBG12\n"
        "                9: V4L2_PIX_FMT_SGBRG12\n"
        "                10: V4L2_PIX_FMT_SBGGR12\n"
        "                default: V4L2_PIX_FMT_NV12\n"
        "-t | --distype       set display type, default 0\n"
        "                0: Not display\n"
        "                1: Use Framebuffer Display\n"
        "                2: Use DRM Display\n"
        "-l | --loadfw        load stfisp fw image\n"
        "-s | --g_imagesize     print image size\n"
        "-p | --isp_poll_event  test isp poll event\n"
        "-a | --isp_test_ioctl  test isp ioctl\n"
        "\n"
        "Eg:\n"
        "\t drm: v4l2test -d /dev/video2 -f 5 -c -W 1920 -H 1080 -m 2 -t 2\n"
        "\t fb:  v4l2test -d /dev/video2 -f 5 -c -W 1920 -H 1080 -m 0 -t 1\n"
        "\n"
        "Open debug log level: \n"
        "\t export V4L2_DEBUG=3\n"
        "\t default level 1, level range 0 ~ 7\n"
        "",
        argv[0]);
}

static const char short_options [] = "d:ho:q:m:W:H:I:vcf:t:X:Y:R:D:l:C:spa";

static const struct option long_options [] = {
    { "device",     required_argument,      NULL,           'd' },
    { "help",       no_argument,            NULL,           'h' },
    { "output",     required_argument,      NULL,           'o' },
    { "quality",    required_argument,      NULL,           'q' },
    { "method",     required_argument,      NULL,           'm' },
    { "width",      required_argument,      NULL,           'W' },
    { "height",     required_argument,      NULL,           'H' },
    { "left",       required_argument,      NULL,           'X' },
    { "up",         required_argument,      NULL,           'Y' },
    { "right",      required_argument,      NULL,           'R' },
    { "down",       required_argument,      NULL,           'D' },
    { "interval",   required_argument,      NULL,           'I' },
    { "version",    no_argument,            NULL,           'v' },
    { "continuous", no_argument,            NULL,           'c' },
    { "format",     required_argument,      NULL,           'f' },
    { "distype",    required_argument,      NULL,           't' },
    { "loadfw",     required_argument,      NULL,           'l' },
    { "connector",  required_argument,      NULL,           'C' },
    { "g_imagesize",no_argument,            NULL,           's' },
    { "isp_test_pollevent", no_argument,    NULL,           'p' },
    { "isp_test_ioctl", no_argument,  NULL,           'a' },
    { 0, 0, 0, 0 }
};

void parse_options(int argc, char **argv, ConfigParam_t *cfg_param)
{
    int index, c = 0;
    int value = 0;
    int test_poll_event = 0;

    while ((c = getopt_long(argc, argv, short_options, long_options, &index)) != -1) {
        switch (c) {
        case 0: /* getopt_long() flag */
            break;

        case 'd':
            cfg_param->v4l2_param.device_name = strdup(optarg);
            break;

        case 'h':
            usage(stdout, argc, argv);
            exit(EXIT_SUCCESS);

        case 'o':
            // set jpeg filename
            cfg_param->jpegFilename = strdup(optarg);
            break;

        case 'q':
            // set jpeg quality
            cfg_param->jpegQuality = atoi(optarg);
            break;

        case 'm':
            value = atoi(optarg);
            if (value < IO_METHOD_MMAP || value > IO_METHOD_READ) {
                LOG(STF_LEVEL_ERR, "io method %d is out of range [%d, %d]\n", value,
                        IO_METHOD_MMAP, IO_METHOD_READ);
                exit(EXIT_FAILURE);
            }
            LOG(STF_LEVEL_INFO, "io method: %s\n", g_iomthd_values[value].name);
            cfg_param->io_mthd = value;
            cfg_param->v4l2_param.io_mthd = cfg_param->io_mthd;
            break;

        case 'W':
            // set v4l2 width
            cfg_param->v4l2_param.width = atoi(optarg);
            break;

        case 'H':
            // set v4l2 height
            cfg_param->v4l2_param.height = atoi(optarg);
            break;

        case 'X':
            // set x start
            cfg_param->v4l2_param.crop_info.left = atoi(optarg);
            cfg_param->v4l2_param.crop_flag = 1;
            break;

        case 'Y':
            // set y start
            cfg_param->v4l2_param.crop_info.top = atoi(optarg);
            cfg_param->v4l2_param.crop_flag = 1;
            break;

        case 'R':
            // set x width
            cfg_param->v4l2_param.crop_info.width = atoi(optarg);
            cfg_param->v4l2_param.crop_flag = 1;
            break;

        case 'D':
            // set y height
            cfg_param->v4l2_param.crop_info.height = atoi(optarg);
            cfg_param->v4l2_param.crop_flag = 1;
            break;

        case 'I':
            // set fps
            cfg_param->v4l2_param.fps = atoi(optarg);
            break;

        case 'c':
            // set flag for continuous capture, interuptible by sigint
            cfg_param->continuous = 1;
            InstallSIGINTHandler();
            break;

        case 'C':
            value = atoi(optarg);
            if (value == 0)
                cfg_param->drm_param.connector_id = INNO_HDMI_CONNECTOR_ID;
            else
                cfg_param->drm_param.connector_id = MIPI_RGB_CONNECTOR_ID;
            break;

        case 'v':
            printf("Version: %s\n", TEST_VERSION);
            exit(EXIT_SUCCESS);
            break;

        case 'f':
            LOG(STF_LEVEL_INFO, "v4l2 format: %s\n", optarg);
            value = atoi(optarg);
            LOG(STF_LEVEL_INFO, "v4l2 format: %d\n", value);
            switch (value) {
            case  0:
                value = V4L2_PIX_FMT_RGB565;
                break;
            case  1:
                value = V4L2_PIX_FMT_RGB24;
                break;
            case  2:
                value = V4L2_PIX_FMT_YUV420;
                break;
            case  3:
                value = V4L2_PIX_FMT_YUYV;
                break;
            case  4:
                value = V4L2_PIX_FMT_NV21;
                break;
            case  5:
                value = V4L2_PIX_FMT_NV12;
                break;
            case  6:
                value = V4L2_PIX_FMT_YVYU;
                break;
            case  7:
                value = V4L2_PIX_FMT_SRGGB12;
                break;
            case  8:
                value = V4L2_PIX_FMT_SGRBG12;
                break;
            case  9:
                value = V4L2_PIX_FMT_SGBRG12;
                break;
            case  10:
                value = V4L2_PIX_FMT_SBGGR12;
                break;
            default:
                value = V4L2_PIX_FMT_RGB565;
                break;
            }
            cfg_param->v4l2_param.format = value;
            break;

        case 't':
            value = atoi(optarg);
            if (value < STF_DISP_NONE || value > STF_DISP_DRM) {
                LOG(STF_LEVEL_ERR, "Display Type %d is out of range [%d, %d]\n", value,
                        STF_DISP_NONE, STF_DISP_DRM);
                exit(EXIT_FAILURE);
            }
            LOG(STF_LEVEL_INFO, "Display Type: %s\n", g_disp_values[value].name);
            cfg_param->disp_type = value;
            break;

        case 'l':
            loadfw_start(optarg, &(cfg_param->v4l2_param));
            exit(EXIT_SUCCESS);
            break;
        case 's':
            sensor_image_size_info(&(cfg_param->v4l2_param));
            exit(EXIT_SUCCESS);
            break;
        case 'p':
            test_poll_event = 1;
            break;
        case 'a':
            if (cfg_param->v4l2_param.device_name)
                isp_test_ioctl(cfg_param->v4l2_param.device_name);
            else
                LOG(STF_LEVEL_INFO, "unkonw device name\n");
            exit(EXIT_SUCCESS);
            break;
       default:
            usage(stderr, argc, argv);
            exit(EXIT_FAILURE);
        }
    }

    if (test_poll_event) {
       if (cfg_param->v4l2_param.device_name)
           isp_poll_event(cfg_param->v4l2_param.device_name, &cfg_param->continuous);
       else
           LOG(STF_LEVEL_INFO, "unkonw device name\n");
       exit(EXIT_SUCCESS);
    }
}

int main(int argc, char **argv)
{
#if defined(USE_ISP_SDK_LIB)
  #if defined(USE_ISP_SDK_LIB_FOR_VDO)
    STF_INT nIdx;
  #endif //#if defined(USE_ISP_SDK_LIB_FOR_VDO)
  #if defined(USE_ISP_SDK_LIB_FOR_FB_AND_PP)
    ST_FB_DEV_PARAM *pstFbDevParam = NULL;
  #endif //#if defined(USE_ISP_SDK_LIB_FOR_FB_AND_PP)
  #if defined(USE_ISP_SDK_LIB_FOR_DRM)
    ST_DRM_DEV_PARAM *pstDrmDevParam = NULL;
  #endif //#if defined(USE_ISP_SDK_LIB_FOR_DRM)
    V4l2Param_t *pstV4l2Param = NULL;
    STF_INT nIspIdx = -1;
    EN_MEM_TYPE enMemType = EN_MEM_TYPE_NONE;
    ST_RECT_2 stRect;
    STF_U32 u32PixelFormat = EN_PXL_FMT_COLOR_YUV420_NV12;
    STF_U32 u32BufCount;
#endif //#if defined(USE_ISP_SDK_LIB)

    init_log();
    alloc_default_config(&gp_cfg_param);
    parse_options(argc, argv, gp_cfg_param);
    check_cfg_params(gp_cfg_param);

    // open and initialize v4l2 device
#if defined(USE_ISP_SDK_LIB)
    pstV4l2Param = &gp_cfg_param->v4l2_param;
    LOG(STF_LEVEL_TRACE, "Trying to check ISP index\n");
    if ((0 == strcmp(pstV4l2Param->device_name, "/dev/video1"))
        || (0 == strcmp(pstV4l2Param->device_name, "/dev/video3"))
        || (0 == strcmp(pstV4l2Param->device_name, "/dev/video5"))
        || (0 == strcmp(pstV4l2Param->device_name, "/dev/video7"))
        || (0 == strcmp(pstV4l2Param->device_name, "/dev/video9"))
        || (0 == strcmp(pstV4l2Param->device_name, "/dev/video11"))
        || (0 == strcmp(pstV4l2Param->device_name, "/dev/video13"))) {
        nIspIdx = 0;
    } else if ((0 == strcmp(pstV4l2Param->device_name, "/dev/video2"))
        || (0 == strcmp(pstV4l2Param->device_name, "/dev/video4"))
        || (0 == strcmp(pstV4l2Param->device_name, "/dev/video6"))
        || (0 == strcmp(pstV4l2Param->device_name, "/dev/video8"))
        || (0 == strcmp(pstV4l2Param->device_name, "/dev/video10"))
        || (0 == strcmp(pstV4l2Param->device_name, "/dev/video12"))
        || (0 == strcmp(pstV4l2Param->device_name, "/dev/video14"))) {
        nIspIdx = 1;
    //} else if (0 == strcmp(pstV4l2Param->device_name, "/dev/video0")) {
    //    nIspIdx = 2;
    } else {
        LOG(STF_LEVEL_ERR, "The ISP SDK does not support "\
            "this \'%s\' deviece!\n", pstV4l2Param->device_name);
        exit(EXIT_FAILURE);
    }
    LOG(STF_LEVEL_INFO, "nIspIdx = %d\n", nIspIdx);
    switch (pstV4l2Param->io_mthd) {
        case IO_METHOD_READ:
            LOG(STF_LEVEL_ERR, "The ISP SDK does not support "\
                "the read i/o method!\n");
            exit(EXIT_FAILURE);
            break;

        case IO_METHOD_MMAP:
            enMemType = EN_MEM_TYPE_MMAP;
            break;

        case IO_METHOD_USERPTR:
            LOG(STF_LEVEL_ERR, "The ISP SDK does not support "\
                "the user pointer i/o method!\n");
            exit(EXIT_FAILURE);
            break;

        case IO_METHOD_DMABUF:
            enMemType = EN_MEM_TYPE_DMA;
            break;

        default:
            LOG(STF_LEVEL_ERR, "Does not specify the "\
                "streaming i/o method!\n");
            exit(EXIT_FAILURE);
            break;
    }
    LOG(STF_LEVEL_INFO, "enMemType = %d\n", enMemType);
    stRect.u16Left = pstV4l2Param->crop_info.left;
    stRect.u16Top = pstV4l2Param->crop_info.top;
    stRect.u16Width = pstV4l2Param->crop_info.width;
    stRect.u16Height = pstV4l2Param->crop_info.height;

    // Generate the device table.
    //=========================================================================
    LOG(STF_LEVEL_INFO, "Generate the device table.\n");
    if (STF_SUCCESS != STFLIB_ISP_DEVICE_GenerateDeviceTable(
        NULL,
        NULL
        )) {
        LOG(STF_LEVEL_ERR, "Failed to generate the video device table!!!\n");
        exit(EXIT_FAILURE);
    }
    //=========================================================================

  #if defined(USE_ISP_SDK_LIB_FOR_VDO)
    LOG(STF_LEVEL_TRACE, "Trying to open the video device\n");
    CLEAR(gp_cfg_param->stVdoDevice);
    if (STF_SUCCESS != STFLIB_ISP_DEVICE_StructInitializeWithDeviceName_2(
        &gp_cfg_param->stVdoDevice,
        (STF_CHAR *)pstV4l2Param->device_name,
        ((EN_MEM_TYPE_DMA == enMemType) ? (STF_TRUE) : (STF_FALSE)),
        enMemType,
        pstV4l2Param->width,
        pstV4l2Param->height,
        stRect,
        pstV4l2Param->fps
        )) {
        LOG(STF_LEVEL_TRACE, "Cannot open the ISP %d \'%s\' video device, "\
            "Error: %d(0x%08X), %s\n",
            nIspIdx,
            pstV4l2Param->device_name,
            errno,
            errno,
            strerror(errno)
            );
        exit(EXIT_FAILURE);
    }
    LOG(STF_LEVEL_TRACE, "Trying to initialize the video device\n");
    if (STF_SUCCESS != gp_cfg_param->stVdoDevice.InitDevice(
        &gp_cfg_param->stVdoDevice,
#if 0
        V4L2_PIX_FMT_NV12
#else
        gp_cfg_param->v4l2_param.format
#endif //#if 0
        )) {
        LOG(STF_LEVEL_TRACE, "Failed to initialize the ISP %d \'%s\' "\
            "video device, Error: %d(0x%08X), %s\n",
            nIspIdx,
            pstV4l2Param->device_name,
            errno,
            errno,
            strerror(errno)
            );
        exit(EXIT_FAILURE);
    }
  #else
    stf_v4l2_open(&gp_cfg_param->v4l2_param, gp_cfg_param->v4l2_param.device_name);
    stf_v4l2_init(&gp_cfg_param->v4l2_param);
  #endif //#if defined(USE_ISP_SDK_LIB_FOR_VDO)
#else
    stf_v4l2_open(&gp_cfg_param->v4l2_param, gp_cfg_param->v4l2_param.device_name);
    stf_v4l2_init(&gp_cfg_param->v4l2_param);
#endif //#if defined(USE_ISP_SDK_LIB)

    if (STF_DISP_FB == gp_cfg_param->disp_type) {
#if defined(USE_ISP_SDK_LIB) && defined(USE_ISP_SDK_LIB_FOR_FB_AND_PP)
        LOG(STF_LEVEL_TRACE, "Trying to open the FB and PP device\n");
        if (STF_SUCCESS != STFLIB_ISP_DEVICE_StructInitializeWithDeviceName(
            &gp_cfg_param->stFbDevice,
            FB_DEVICE_NAME,
            STF_FALSE,
            EN_MEM_TYPE_NONE,
            -1,
            -1
            )) {
            LOG(STF_LEVEL_TRACE, "Cannot open the \'%s\' frame buffer device, "\
                "Error: %d(0x%08X), %s\n",
                FB_DEVICE_NAME,
                errno,
                errno,
                strerror(errno)
                );
            exit(EXIT_FAILURE);
        }
        if (STF_SUCCESS != STFLIB_ISP_DEVICE_StructInitializeWithDeviceName(
            &gp_cfg_param->stPpDevice,
            STFBC_DEVICE_NAME,
            STF_FALSE,
            EN_MEM_TYPE_NONE,
            -1,
            -1
            )) {
            LOG(STF_LEVEL_TRACE, "Cannot open the \'%s\' pre-process device, "\
                "Error: %d(0x%08X), %s\n",
                STFBC_DEVICE_NAME,
                errno,
                errno,
                strerror(errno)
                );
            exit(EXIT_FAILURE);
        }
        LOG(STF_LEVEL_TRACE, "Trying to initialize the PP and FB device\n");
        if (STF_SUCCESS != gp_cfg_param->stPpDevice.InitDevice(
            &gp_cfg_param->stPpDevice,
            gp_cfg_param->v4l2_param.format
            )) {
            LOG(STF_LEVEL_TRACE, "Failed to initialize the \'%s\' "\
                "pre-process device, Error: %d(0x%08X), %s\n",
                STFBC_DEVICE_NAME,
                errno,
                errno,
                strerror(errno)
                );
            exit(EXIT_FAILURE);
        }
        u32PixelFormat = gp_cfg_param->stPpDevice.GetPixelFormat(
            &gp_cfg_param->stPpDevice);
        if (STF_SUCCESS != gp_cfg_param->stFbDevice.InitDevice(
            &gp_cfg_param->stFbDevice,
            u32PixelFormat
            )) {
            LOG(STF_LEVEL_TRACE, "Failed to initialize the \'%s\' "\
                "frame buffer device, Error: %d(0x%08X), %s\n",
                FB_DEVICE_NAME,
                errno,
                errno,
                strerror(errno)
                );
            exit(EXIT_FAILURE);
        }
        pstFbDevParam = (ST_FB_DEV_PARAM *)gp_cfg_param->stFbDevice.pvDevParam;
        gp_cfg_param->fb_param.width = pstFbDevParam->u16Width;
        gp_cfg_param->fb_param.height = pstFbDevParam->u16Height;
        gp_cfg_param->fb_param.bpp = pstFbDevParam->u32Bpp;
        gp_cfg_param->fb_param.screen_size = pstFbDevParam->u32ScreenSize;
        gp_cfg_param->fb_param.screen_buf = pstFbDevParam->pvBuffer;
        update_videocvt_param(gp_cfg_param->disp_type, gp_cfg_param->fb_param.width,
                gp_cfg_param->fb_param.height, gp_cfg_param->fb_param.bpp,
                gp_cfg_param->fb_param.screen_size);
#else
        stf_fb_open(&gp_cfg_param->fb_param, FB_DEVICE_NAME, STFBC_DEVICE_NAME);
        stf_fb_init(&gp_cfg_param->fb_param, gp_cfg_param->v4l2_param.format);
        update_videocvt_param(gp_cfg_param->disp_type, gp_cfg_param->fb_param.width,
                gp_cfg_param->fb_param.height, gp_cfg_param->fb_param.bpp,
                gp_cfg_param->fb_param.screen_size);
#endif //#if defined(USE_ISP_SDK_LIB) && defined(USE_ISP_SDK_LIB_FOR_FB_AND_PP)
    } else if (STF_DISP_DRM == gp_cfg_param->disp_type) {
#if defined(USE_ISP_SDK_LIB) && defined(USE_ISP_SDK_LIB_FOR_DRM)
        STF_U32 u32Idx;

        LOG(STF_LEVEL_TRACE, "Trying to open the DRM device\n");
        if (STF_SUCCESS != STFLIB_ISP_DEVICE_StructInitializeWithDeviceName(
            &gp_cfg_param->stDrmDevice,
            DRM_DEVICE_NAME,
            STF_FALSE,
            enMemType,
            pstV4l2Param->width,
            pstV4l2Param->height
            )) {
            LOG(STF_LEVEL_TRACE, "Cannot open the \'%s\' DRM device, "\
                "Error: %d(0x%08X), %s\n",
                DRM_DEVICE_NAME,
                errno,
                errno,
                strerror(errno)
                );
            exit(EXIT_FAILURE);
        }
        LOG(STF_LEVEL_TRACE, "Trying to initialize the DRM device\n");
        if (STF_SUCCESS != gp_cfg_param->stDrmDevice.InitDevice(
            &gp_cfg_param->stDrmDevice,
            gp_cfg_param->v4l2_param.format
            )) {
            LOG(STF_LEVEL_TRACE, "Failed to initialize the \'%s\' "\
                "DRM device, Error: %d(0x%08X), %s\n",
                DRM_DEVICE_NAME,
                errno,
                errno,
                strerror(errno)
                );
            exit(EXIT_FAILURE);
        }
        LOG(STF_LEVEL_TRACE, "Trying to allocate buffer for the DRM device\n");
        u32BufCount = sizeof(gp_cfg_param->dmabufs)
            / sizeof(gp_cfg_param->dmabufs[0]);
        for (u32Idx = 0; u32Idx < u32BufCount; u32Idx++) {
            gp_cfg_param->stDmaBufInfo[u32Idx].nDmaBufFd = -1;
            gp_cfg_param->stDmaBufInfo[u32Idx].pvBuffer = NULL;
        }
        if (STF_SUCCESS != gp_cfg_param->stDrmDevice.AllocateBuf(
            &gp_cfg_param->stDrmDevice,
            u32BufCount,
            gp_cfg_param->stDmaBufInfo
            )) {
            LOG(STF_LEVEL_TRACE, "Failed to allocate buffer for the \'%s\' "\
                "DRM device, Error: %d(0x%08X), %s\n",
                DRM_DEVICE_NAME,
                errno,
                errno,
                strerror(errno)
                );
            exit(EXIT_FAILURE);
        }
        for (u32Idx = 0; u32Idx < u32BufCount; u32Idx++) {
            gp_cfg_param->dmabufs[u32Idx] =
                gp_cfg_param->stDmaBufInfo[u32Idx].nDmaBufFd;
        }
        pstDrmDevParam =
            (ST_DRM_DEV_PARAM *)gp_cfg_param->stDrmDevice.pvDevParam;
        gp_cfg_param->drm_param.dev_head =
            (drm_dev_t *)pstDrmDevParam->pstDrmDevHead;
        update_videocvt_param(gp_cfg_param->disp_type,
            gp_cfg_param->drm_param.dev_head->width,
            gp_cfg_param->drm_param.dev_head->height, 32,
            gp_cfg_param->drm_param.dev_head->bufs[0].size);
#else
        stf_drm_open(&gp_cfg_param->drm_param, DRM_DEVICE_NAME, gp_cfg_param->io_mthd);
        stf_drm_init(&gp_cfg_param->drm_param, gp_cfg_param->v4l2_param.width,
                gp_cfg_param->v4l2_param.height, gp_cfg_param->v4l2_param.format,
                gp_cfg_param->io_mthd, gp_cfg_param->dmabufs,
                sizeof(gp_cfg_param->dmabufs) / sizeof(gp_cfg_param->dmabufs[0]));
        update_videocvt_param(gp_cfg_param->disp_type, gp_cfg_param->drm_param.dev_head->width,
                gp_cfg_param->drm_param.dev_head->height, 32,
                gp_cfg_param->drm_param.dev_head->bufs[0].size);
#endif //#if defined(USE_ISP_SDK_LIB) && defined(USE_ISP_SDK_LIB_FOR_DRM)
    }

    // prepare and start v4l2 capturing
#if defined(USE_ISP_SDK_LIB) && defined(USE_ISP_SDK_LIB_FOR_VDO)
    LOG(STF_LEVEL_TRACE, "Trying to allocate buffers for the ISP %d \'%s\' "\
        "video device.\n", nIspIdx, pstV4l2Param->device_name);
    u32BufCount = sizeof(gp_cfg_param->dmabufs)
        / sizeof(gp_cfg_param->dmabufs[0]);
    LOG(STF_LEVEL_TRACE, "u32BufCount = %d\n", u32BufCount);
    if (IO_METHOD_MMAP == pstV4l2Param->io_mthd) {
        // IO method is memory mapping and using the frame buffer and PP device.
  #if 0
        if (STF_SUCCESS != gp_cfg_param->stVdoDevice.AllocateBuf(
            &gp_cfg_param->stVdoDevice,
            u32BufCount,
            NULL
            )) {
            LOG(STF_LEVEL_TRACE, "Failed to allocate buffers for "\
                "the ISP %d \'%s\' video device, Error: %d(0x%08X), %s\n",
                nIspIdx,
                pstV4l2Param->device_name,
                errno,
                errno,
                strerror(errno)
                );
            exit(EXIT_FAILURE);
        }
  #else
        if (STF_SUCCESS != gp_cfg_param->stVdoDevice.AllocateAndEnqueueBuf(
            &gp_cfg_param->stVdoDevice,
            u32BufCount,
            u32BufCount,
            NULL
            )) {
            LOG(STF_LEVEL_TRACE, "Failed to allocate and enqueue buffers for "\
                "the ISP %d \'%s\' video device, Error: %d(0x%08X), %s\n",
                nIspIdx,
                pstV4l2Param->device_name,
                errno,
                errno,
                strerror(errno)
                );
            exit(EXIT_FAILURE);
        }
  #endif //#if 0
    } else if (IO_METHOD_DMABUF == pstV4l2Param->io_mthd) {
        // IO method is DMA buffering and using the DRM device.
  #if defined(USE_ISP_SDK_LIB) && defined(USE_ISP_SDK_LIB_FOR_DRM)
  #else
        STF_U32 u32Idx;

        for (u32Idx = 0; u32Idx < u32BufCount; u32Idx++) {
            gp_cfg_param->stDmaBufInfo[u32Idx].pvBuffer = NULL;
            gp_cfg_param->stDmaBufInfo[u32Idx].nDmaBufFd =
                gp_cfg_param->dmabufs[u32Idx];
        }
  #endif //#if defined(USE_ISP_SDK_LIB) && defined(USE_ISP_SDK_LIB_FOR_DRM)
  #if 0
        if (STF_SUCCESS != gp_cfg_param->stVdoDevice.AllocateBuf(
            &gp_cfg_param->stVdoDevice,
            u32BufCount,
            gp_cfg_param->stDmaBufInfo
            )) {
            LOG(STF_LEVEL_TRACE, "Failed to allocate buffers for "\
                "the ISP %d \'%s\' video device, Error: %d(0x%08X), %s\n",
                nIspIdx,
                pstV4l2Param->device_name,
                errno,
                errno,
                strerror(errno)
                );
            exit(EXIT_FAILURE);
        }
  #else
        if (STF_SUCCESS != gp_cfg_param->stVdoDevice.AllocateAndEnqueueBuf(
            &gp_cfg_param->stVdoDevice,
            u32BufCount,
            u32BufCount,
            gp_cfg_param->stDmaBufInfo
            )) {
            LOG(STF_LEVEL_TRACE, "Failed to allocate and enqueue buffers for "\
                "the ISP %d \'%s\' video device, Error: %d(0x%08X), %s\n",
                nIspIdx,
                pstV4l2Param->device_name,
                errno,
                errno,
                strerror(errno)
                );
            exit(EXIT_FAILURE);
        }
  #endif //#if 0
    } else {
        LOG(STF_LEVEL_ERR, "Does not support this streaming IO method!\n");
        exit(EXIT_FAILURE);
    }

  #if 0
    LOG(STF_LEVEL_TRACE, "Enqueue the buffers into the ISP %d \'%s\' "\
        "video device.\n", nIspIdx, pstV4l2Param->device_name);
    {
        ST_VDEV_PARAM *pstVDevParam = NULL;
        ST_VDO_BUF_INFO stVideoBufferInfo[BUFCOUNT + 3] = { 0 };
        STF_U32 u32Idx;
        STF_S32 s32BufCount;
        STF_RESULT Ret = STF_SUCCESS;

        pstVDevParam = (ST_VDEV_PARAM *)gp_cfg_param->stVdoDevice.pvDevParam;
        s32BufCount = pstVDevParam->stVdoMemTbl.u32Count;
        if (s32BufCount) {
            for (u32Idx = 0; u32Idx < s32BufCount; u32Idx++) {
                stVideoBufferInfo[u32Idx].u8BufIdx = u32Idx;
            }
            Ret = gp_cfg_param->stVdoDevice.EnqueueBuf_2(
                &gp_cfg_param->stVdoDevice,
                s32BufCount,
                &stVideoBufferInfo[0]
                );
            if (STF_SUCCESS != Ret) {
                LOG(STF_LEVEL_TRACE, "Failed to enqueue the buffers into "\
                    "the ISP %d \'%s\' video device.\n",
                    nIspIdx, pstV4l2Param->device_name);
                //exit(EXIT_FAILURE);
            }
        }
    }

  #endif //#if 0
    LOG(STF_LEVEL_TRACE, "Trying to stream on the \'%s\' video device.\n",
        pstV4l2Param->device_name);
    if (STF_SUCCESS != gp_cfg_param->stVdoDevice.SetStreamOn(
        &gp_cfg_param->stVdoDevice,
        STF_TRUE
        )) {
        LOG(STF_LEVEL_TRACE, "Failed to stream on the ISP %d \'%s\' "\
            "video device, Error: %d(0x%08X), %s\n",
            nIspIdx,
            pstV4l2Param->device_name,
            errno,
            errno,
            strerror(errno)
            );
        exit(EXIT_FAILURE);
    }
#else
    sft_v4l2_prepare_capturing(&gp_cfg_param->v4l2_param, gp_cfg_param->dmabufs, BUFCOUNT);
    sft_v4l2_start_capturing(&(gp_cfg_param->v4l2_param));
#endif //#if defined(USE_ISP_SDK_LIB) && defined(USE_ISP_SDK_LIB_FOR_VDO)

    // process frames
    mainloop();

#if defined(USE_ISP_SDK_LIB) && defined(USE_ISP_SDK_LIB_FOR_VDO)
    LOG(STF_LEVEL_TRACE, "Trying to stream off the ISP %d \'%s\' "\
        "video device.\n", nIspIdx, pstV4l2Param->device_name);
    if (STF_SUCCESS != gp_cfg_param->stVdoDevice.SetStreamOn(
        &gp_cfg_param->stVdoDevice,
        STF_FALSE
        )) {
        LOG(STF_LEVEL_TRACE, "Failed to stream off the ISP %d \'%s\' "\
            "video device, Error: %d(0x%08X), %s\n",
            nIspIdx,
            pstV4l2Param->device_name,
            errno,
            errno,
            strerror(errno)
            );
        exit(EXIT_FAILURE);
    }

    LOG(STF_LEVEL_TRACE, "Trying to release buffers from the ISP %d \'%s\' "\
        "video device.\n", nIspIdx, pstV4l2Param->device_name);
    if (STF_SUCCESS != gp_cfg_param->stVdoDevice.ReleaseBuf(
        &gp_cfg_param->stVdoDevice
        )) {
        LOG(STF_LEVEL_TRACE, "Failed to stream off the ISP %d \'%s\' "\
            "video device, Error: %d(0x%08X), %s\n",
            nIspIdx,
            pstV4l2Param->device_name,
            errno,
            errno,
            strerror(errno)
            );
        exit(EXIT_FAILURE);
    }

  #if 0
    LOG(STF_LEVEL_TRACE, "Trying to disconnect the ISP %d \'%s\' "\
        "video device.\n", nIspIdx, pstV4l2Param->device_name);
    if (STF_SUCCESS != gp_cfg_param->stVdoDevice.Disconnect(
        &gp_cfg_param->stVdoDevice
        )) {
        LOG(STF_LEVEL_TRACE, "Failed to disconnect the ISP %d \'%s\' "\
            "video device, Error: %d(0x%08X), %s\n",
            nIspIdx,
            pstV4l2Param->device_name,
            errno,
            errno,
            strerror(errno)
            );
        exit(EXIT_FAILURE);
    }

  #endif //#if 0
    LOG(STF_LEVEL_TRACE, "Trying to close the ISP %d \'%s\' "\
        "video device.\n", nIspIdx, pstV4l2Param->device_name);
    if (STF_SUCCESS != STFLIB_ISP_DEVICE_StructUninitialize(
        &gp_cfg_param->stVdoDevice
        )) {
        LOG(STF_LEVEL_TRACE, "Failed to close the ISP %d \'%s\' "\
            "video device, Error: %d(0x%08X), %s\n",
            nIspIdx,
            pstV4l2Param->device_name,
            errno,
            errno,
            strerror(errno)
            );
        exit(EXIT_FAILURE);
    }
#else
    stf_v4l2_stop_capturing(&gp_cfg_param->v4l2_param);
    stf_v4l2_uninit(&gp_cfg_param->v4l2_param);
    stf_v4l2_close(&gp_cfg_param->v4l2_param);
#endif //#if defined(USE_ISP_SDK_LIB) && defined(USE_ISP_SDK_LIB_FOR_VDO)

    if (STF_DISP_FB == gp_cfg_param->disp_type) {
#if defined(USE_ISP_SDK_LIB) && defined(USE_ISP_SDK_LIB_FOR_FB_AND_PP)
        LOG(STF_LEVEL_TRACE, "Trying to close the FB and PP device.\n");
        if (STF_SUCCESS != STFLIB_ISP_DEVICE_StructUninitialize(
            &gp_cfg_param->stPpDevice
            )) {
            LOG(STF_LEVEL_TRACE, "Failed to close the \'%s\' "\
                "pre-process device, Error: %d(0x%08X), %s\n",
                STFBC_DEVICE_NAME,
                errno,
                errno,
                strerror(errno)
                );
            //exit(EXIT_FAILURE);
        }
        if (STF_SUCCESS != STFLIB_ISP_DEVICE_StructUninitialize(
            &gp_cfg_param->stFbDevice
            )) {
            LOG(STF_LEVEL_TRACE, "Failed to close the \'%s\' "\
                "frame buffer device, Error: %d(0x%08X), %s\n",
                FB_DEVICE_NAME,
                errno,
                errno,
                strerror(errno)
                );
            //exit(EXIT_FAILURE);
        }
#else
        stf_fb_uninit(&gp_cfg_param->fb_param);
        stf_fb_close(&gp_cfg_param->fb_param);
#endif //#if defined(USE_ISP_SDK_LIB) && defined(USE_ISP_SDK_LIB_FOR_FB_AND_PP)
    } else if (STF_DISP_DRM == gp_cfg_param->disp_type) {
#if defined(USE_ISP_SDK_LIB) && defined(USE_ISP_SDK_LIB_FOR_DRM)
        LOG(STF_LEVEL_TRACE, "Trying to close the DRM device.\n");
        if (STF_SUCCESS != STFLIB_ISP_DEVICE_StructUninitialize(
            &gp_cfg_param->stDrmDevice
            )) {
            LOG(STF_LEVEL_TRACE, "Failed to close the \'%s\' "\
                "DRM device, Error: %d(0x%08X), %s\n",
                DRM_DEVICE_NAME,
                errno,
                errno,
                strerror(errno)
                );
            //exit(EXIT_FAILURE);
        }
#else
        stf_drm_close(&gp_cfg_param->drm_param);
#endif //#if defined(USE_ISP_SDK_LIB) && defined(USE_ISP_SDK_LIB_FOR_DRM)
    }

    deinit_log();
    free(gp_cfg_param);

    return 0;
}
