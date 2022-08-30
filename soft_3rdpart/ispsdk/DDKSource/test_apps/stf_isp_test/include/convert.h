
// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2021 StarFive Technology Co., Ltd.
 */
#ifndef _CONVERT_H_
#define _CONVERT_H_

extern int yuyv_resize(const uint8_t *inBuf, uint8_t *outBuf, int imgWidth, int imgHeight);

extern int convert_yuyv_to_nv12(const uint8_t *inBuf, uint8_t *outBuf, int imgWidth, int imgHeight, int is_yuyv);
extern int convert_nv21_to_nv12(const uint8_t *inBuf, uint8_t *outBuf, int imgWidth, int imgHeight, int is_nv21);
extern int convert_nv21_to_rgb(const uint8_t *inBuf, uint8_t *outBuf, int imgWidth, int imgHeight, int is_nv21);
extern int convert_rgb565_to_nv12(const uint8_t *inBuf, uint8_t *outBuf, int imgWidth, int imgHeight, int is_nv21);
extern int convert_yuyv_to_rgb(const uint8_t *inBuf, uint8_t *outBuf, int imgWidth, int imgHeight, int cvtMethod);
extern int convert_yuv444_to_rgb(uint8_t *inBuf, uint8_t *outBuf, int imgWidth, int imgHeight, int cvtMethod);
extern int convert_rgb565_to_rgb(const uint8_t *inBuf, uint8_t *outBuf, int imgWidth, int imgHeight, int cvtMethod);
extern int convert_rgb888_to_rgb(const uint8_t *inBuf, uint8_t *outBuf, int imgWidth, int imgHeight, int cvtMethod);

extern void update_videocvt_param(int distype, int scr_width, int scr_height,
        int scr_bpp, int scr_size);

#endif // _CONVERT_H_
