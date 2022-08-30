// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2021 StarFive Technology Co., Ltd.
 */

#include <sys/mman.h>
#include <sys/ioctl.h>
#include "common.h"
#include "stf_framebuffer.h"
#include "stf_log.h"

#define FBIOPAN_GET_PP_MODE        0x4609
#define FBIOPAN_SET_PP_MODE        0x460a

void stf_fb_open(FBParam_t *param, char *device_name, char *stfbc_name)
{
    LOG(STF_LEVEL_TRACE, "Enter\n");
    //open framebuffer
    param->fd = open(device_name, O_RDWR);
    if (param->fd == -1) {
        LOG(STF_LEVEL_ERR, "Error: cannot open framebuffer device.\n");
        exit(EXIT_FAILURE);
    }
    //open stfbc device for pp setting
    param->stfbc_fd = open(stfbc_name, O_RDWR);
    if (param->stfbc_fd == -1) {
        LOG(STF_LEVEL_ERR, "Error: cannot open stfbcdev device.\n");
        exit(EXIT_FAILURE);
    }
    LOG(STF_LEVEL_TRACE, "Exit\n");
}

void stf_fb_close(FBParam_t *param)
{
    LOG(STF_LEVEL_TRACE, "Enter\n");
    if (param->fd > 0) {
        close(param->fd);
        param->fd = 0;
    }

    if (param->stfbc_fd > 0) {
        close(param->stfbc_fd);
        param->stfbc_fd = 0;
    }
    LOG(STF_LEVEL_TRACE, "Exit\n");
}

void stf_fb_init(FBParam_t *param, uint32_t v4l2_fmt)
{
    int pixformat;
    struct pp_mode pp_info[3];
    struct fb_var_screeninfo vinfo;
    struct fb_fix_screeninfo finfo;

    LOG(STF_LEVEL_TRACE, "Enter\n");
    // step 1: try to set the fb format got from cmd options
    pixformat = v4l2fmt_to_fbfmt(v4l2_fmt);
    if (-1 == ioctl(param->stfbc_fd, FBIOPAN_GET_PP_MODE, &pp_info[0])) {
        LOG(STF_LEVEL_ERR, "Error reading variable information.\n");
        exit (EXIT_FAILURE);
    }
    LOG(STF_LEVEL_INFO, "Before set_pp_mode. get fb format:%d, will set fb format:%d\n",
            pp_info[1].src.format, pixformat);

    pp_info[1].src.format = pixformat;
    if (-1 == ioctl(param->stfbc_fd, FBIOPAN_SET_PP_MODE, &pp_info[0])) {
        LOG(STF_LEVEL_ERR, "Error reading variable information.\n");
        exit (EXIT_FAILURE);
    }

    if (-1 == ioctl(param->stfbc_fd, FBIOPAN_GET_PP_MODE, &pp_info[0])) {
        LOG(STF_LEVEL_ERR, "Error reading variable information.\n");
        exit (EXIT_FAILURE);
    }
    LOG(STF_LEVEL_INFO, "After set_pp_mode. get fb format:%d\n", pp_info[1].src.format);
    pixformat = pp_info[1].src.format;
    param->pixformat = pixformat;

    // step 2: Get fixed screen information
    if (-1 == ioctl(param->fd, FBIOGET_FSCREENINFO, &finfo)) {
        LOG(STF_LEVEL_ERR, "Error reading fixed information.\n");
        exit (EXIT_FAILURE);
    }
    memcpy(&(param->finfo), &finfo, sizeof(finfo));

    // step 3: Get variable screen information
    if (-1 == ioctl(param->fd, FBIOGET_VSCREENINFO, &vinfo)) {
        LOG(STF_LEVEL_ERR, "Error reading variable information.\n");
        exit (EXIT_FAILURE);
    }
    memcpy(&(param->vinfo), &vinfo, sizeof(vinfo));
    LOG(STF_LEVEL_INFO, "print screen information:\n");
    LOG(STF_LEVEL_INFO, "  vinfo.xres = %d, vinfo.yres = %d, grayscale = %d\n", vinfo.xres, vinfo.yres, vinfo.grayscale);
    LOG(STF_LEVEL_INFO, "  vinfo.xoffset = %d, vinfo.yoffset = %d\n", vinfo.xoffset, vinfo.yoffset);
    LOG(STF_LEVEL_INFO, "  vinfo.bits_per_pixel = %d, finfo.line_length = %d\n", vinfo.bits_per_pixel, finfo.line_length);
    // if (ioctl(param->fd, FBIOPUT_VSCREENINFO, &g_vinfo) < 0) {
    //     LOG(STF_LEVEL_ERR, "FBIOPUT_VSCREENINFO.\n");
    //     exit (EXIT_FAILURE);
    // }

    // step 4: mmap fb buf to user space and update the actually screen param
    param->width = vinfo.xres;
    param->height = vinfo.yres;
    param->bpp = vinfo.bits_per_pixel;
    param->screen_size = vinfo.xres * vinfo.yres * vinfo.bits_per_pixel / 8;
    // g_screensize = g_vinfo.xres * g_vinfo.yres * 32 / 8;
    param->screen_buf = (uint8_t *)mmap(NULL, param->screen_size,
            PROT_READ | PROT_WRITE, MAP_SHARED, param->fd, 0);
    if (param->screen_buf == (void *)(-1)) {
        LOG(STF_LEVEL_ERR, "Error: failed to map framebuffer device to memory.\n");
        exit (EXIT_FAILURE) ;
    }
    memset(param->screen_buf, 0x00, param->screen_size);

    LOG(STF_LEVEL_TRACE, "Exit\n");
}

void stf_fb_uninit(FBParam_t *param)
{
    LOG(STF_LEVEL_TRACE, "Enter\n");
    if (-1 == munmap((void *)param->screen_buf, param->screen_size)) {
        LOG(STF_LEVEL_ERR, " Error: framebuffer device munmap() failed.\n");
        exit (EXIT_FAILURE) ;
    }
    LOG(STF_LEVEL_TRACE, "Exit\n");
}


