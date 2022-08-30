// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2021 StarFive Technology Co., Ltd.
 */
#ifndef __STF_DRM_H__
#define __STF_DRM_H__

#include <libdrm/drm.h>
#include <xf86drm.h>
#include <xf86drmMode.h>
#include "common.h"
#include "stf_log.h"

struct drm_buffer_t {
    uint32_t pitch, size;

    uint32_t fb_id;
    int dmabuf_fd;  // used for dmabuf
    int bo_handle;
    uint8_t *buf;
};

typedef struct drm_dev_t {
    uint32_t conn_id, enc_id, crtc_id;
    uint32_t width, height, pitch;
    drmModeModeInfo mode;
    drmModeCrtc *saved_crtc;

    // int v4l2_fd;
    // int drm_fd;
    uint32_t drm_format;

    struct drm_buffer_t bufs[BUFCOUNT];
    struct drm_dev_t *next;
} drm_dev_t;

typedef struct DRMParam_t {
    drm_dev_t* dev_head;
    int fd;
    int connector_id;
} DRMParam_t;


inline static void fatal(char *str)
{
    LOG(STF_LEVEL_ERR, "%s\n", str);
    exit(EXIT_FAILURE);
}

inline static void error(char *str)
{
    perror(str);
    exit(EXIT_FAILURE);
}

extern void stf_drm_open(DRMParam_t *param, char *device_name, int iomthd);
extern void stf_drm_init(DRMParam_t *param, uint32_t width, uint32_t height,
        uint32_t v4l2_fmt, int iomthd, int *dmabufs, int nsize);
extern void stf_drm_close(DRMParam_t *param);

#endif // __STF_DRM_H__
