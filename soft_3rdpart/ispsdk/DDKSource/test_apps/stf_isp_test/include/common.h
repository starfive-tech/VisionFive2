// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2021 StarFive Technology Co., Ltd.
 */
#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <time.h>
#include <asm/types.h>
#include <linux/videodev2.h>
#include <linux/fb.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <fcntl.h>
#include <malloc.h>

#ifndef TEST_VERSION
#define TEST_VERSION "v0.0.1"
#endif

#define PINGPONG_BUFCOUNT   2   /* For pingpong buffer mapping in drm + mmap mode*/
#define BUFCOUNT            4

#define CLEAR(x)  memset (&(x), 0, sizeof (x))
#define PCLEAR(x) memset ((x), 0, sizeof (*x))

struct buffer {
    void*   start;
    size_t  length;
    int     dmabuf_fd;
    int     index;
};

enum COLOR_FORMAT{
    COLOR_YUV422_UYVY = 0,  // 00={Y1,V0,Y0,U0}
    COLOR_YUV422_VYUY = 1,  // 01={Y1,U0,Y0,V0}
    COLOR_YUV422_YUYV = 2,  // 10={V0,Y1,U0,Y0}
    COLOR_YUV422_YVYU = 3,  // 11={U0,Y1,V0,Y0}

    COLOR_YUV420P,          // 4
    COLOR_YUV420_NV21,      // 5
    COLOR_YUV420_NV12,      // 6

    COLOR_RGB888_ARGB,      // 7
    COLOR_RGB888_ABGR,      // 8
    COLOR_RGB888_RGBA,      // 9
    COLOR_RGB888_BGRA,      // 10
    COLOR_RGB565,           // 11
};

typedef enum IOMethod {
    IO_METHOD_MMAP,
    IO_METHOD_USERPTR,
    IO_METHOD_DMABUF,
    IO_METHOD_READ
} IOMethod;

typedef enum STF_DISP_TYPE {
    STF_DISP_NONE = 0, // Not display
    STF_DISP_FB,       // Use framebuffer framework display
    STF_DISP_DRM       // Use drm framework display
} STF_DISP_TYPE;

extern void dump_fourcc(uint32_t fourcc);
extern int v4l2fmt_to_fbfmt(uint32_t format);
extern uint32_t v4l2fmt_to_drmfmt(uint32_t v4l2_fmt);
extern void test_performance();
extern void errno_exit(const char* s);
extern void errno_print(const char *s);
extern int write_file(char * filename, const unsigned char *image_buffer, int size);
// extern void jpegWrite(unsigned char* img, char* jpegFilename);
extern void jpegWrite(unsigned char* img, char* jpegFilename,
        uint32_t width, uint32_t height, int jpegQuality);
extern int write_JPEG_file(char * filename,unsigned char *image_buffer,
        int image_width, int image_height, int quality);
extern int is_raw_v4l2fmt(uint32_t format);

// inline int clip(int value, int min, int max) {
//     return (value > max ? max : value < min ? min : value);
// }


#endif // __COMMON_H__
