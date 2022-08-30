// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2021 StarFive Technology Co., Ltd.
 */
#ifndef __STF_FRAMEBUFFER_H__
#define __STF_FRAMEBUFFER_H__
#include <linux/fb.h>
#include <stdbool.h>
#include "common.h"

struct pp_video_mode {
    enum COLOR_FORMAT format;
    unsigned int height;
    unsigned int width;
    unsigned int addr;
};

struct pp_mode {
    char pp_id;
    bool bus_out;        /*out to ddr*/
    bool fifo_out;        /*out to lcdc*/
    bool inited;
    struct pp_video_mode src;
    struct pp_video_mode dst;
};

typedef struct FBParam_t {
    int         fd;
    int         stfbc_fd;
    struct fb_var_screeninfo vinfo;
    struct fb_fix_screeninfo finfo;
    uint8_t *screen_buf;
    uint32_t screen_size;

    int pixformat;      // default COLOR_YUV420_NV21
    uint32_t width;     // default 1920
    uint32_t height;    // default 1080
    uint32_t bpp;
} FBParam_t;

extern void stf_fb_open(FBParam_t *param, char *device_name, char *stfbc_name);
extern void stf_fb_close(FBParam_t *param);
extern void stf_fb_init(FBParam_t *param, uint32_t v4l2_fmt);
extern void stf_fb_uninit(FBParam_t *param);

#endif // __STF_FRAMEBUFFER_H__
