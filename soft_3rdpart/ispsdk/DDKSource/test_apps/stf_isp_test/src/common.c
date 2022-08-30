// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2021 StarFive Technology Co., Ltd.
 */
#include "common.h"
#include <stddef.h>
#include <jpeglib.h>
#include <libdrm/drm_fourcc.h>
#include "convert.h"
#include "stf_log.h"

/**
    Print error message and terminate programm with EXIT_FAILURE return code.
    \param s error message to print
*/
void errno_exit(const char* s)
{
    LOG(STF_LEVEL_ERR, "%s error %d, %s\n", s, errno, strerror(errno));
    exit(EXIT_FAILURE);
}

void errno_print(const char *s)
{
    LOG(STF_LEVEL_ERR, "%s error %d, %s\n", s, errno, strerror(errno));
}

void dump_fourcc(uint32_t fourcc)
{
    LOG(STF_LEVEL_LOG, " %c%c%c%c \n",
        fourcc,
        fourcc >> 8,
        fourcc >> 16,
        fourcc >> 24);
}

int is_raw_v4l2fmt(uint32_t format)
{
    int ret;

    switch (format) {
    case V4L2_PIX_FMT_SRGGB12:
    case V4L2_PIX_FMT_SGRBG12:
    case V4L2_PIX_FMT_SGBRG12:
    case V4L2_PIX_FMT_SBGGR12:
        ret = 1;
        break;
    default:
        ret = 0;
        break;
    }

    return ret;
}

// convert v4l2 format to fb format
int v4l2fmt_to_fbfmt(uint32_t format)
{
    int pixformat = COLOR_RGB565;

    switch (format) {
    case V4L2_PIX_FMT_RGB565:
        pixformat = COLOR_RGB565;
        break;
    case V4L2_PIX_FMT_RGB24:
        pixformat = COLOR_RGB888_ARGB;
        break;
    case V4L2_PIX_FMT_YUV420:
        pixformat = COLOR_YUV420P;
        break;
    case V4L2_PIX_FMT_YUYV:
        pixformat = COLOR_YUV422_YUYV;
        break;
    case V4L2_PIX_FMT_NV21:
        pixformat = COLOR_YUV420_NV21;
        break;
    case V4L2_PIX_FMT_NV12:
        pixformat = COLOR_YUV420_NV12;
        break;
    case V4L2_PIX_FMT_YVYU:
        pixformat = COLOR_YUV422_YVYU;
        break;
    default:
        pixformat = COLOR_RGB565;
        break;
    }

    return pixformat;
}

// convert v4l2 format to drm format
uint32_t v4l2fmt_to_drmfmt(uint32_t v4l2_fmt)
{
    uint32_t drm_fmt;
    // dump_fourcc(v4l2_fmt);

    switch (v4l2_fmt) {
    case V4L2_PIX_FMT_RGB565:
        drm_fmt = DRM_FORMAT_RGB565;
        break;
    case V4L2_PIX_FMT_RGB24:
        drm_fmt = DRM_FORMAT_ARGB8888;
        break;
    case V4L2_PIX_FMT_YUV420:
        drm_fmt = DRM_FORMAT_YUV420;
        break;
    case V4L2_PIX_FMT_YUYV:
        drm_fmt = DRM_FORMAT_YUYV;
        break;
    case V4L2_PIX_FMT_YVYU:
        drm_fmt = DRM_FORMAT_YVYU;
        break;
    case V4L2_PIX_FMT_NV21:
        drm_fmt = DRM_FORMAT_NV21;
        break;
    case V4L2_PIX_FMT_NV12:
        drm_fmt = DRM_FORMAT_NV12;
        break;
    default:
        drm_fmt = DRM_FORMAT_NV21;
        LOG(STF_LEVEL_WARN, "drm not support the V4L2_format\n");
        break;
    }
    return drm_fmt;
}

void test_performance()
{
    int count = 1920 * 1080 * 2;
    float r, g, b;
    float y = 10, u = 20, v = 30;
    int int_r, int_g, int_b;
    int int_y = 10, int_u = 20, int_v = 30;
    struct timeval tv1, tv2, tv3;
    long long elapse = 0;
    int i;

    gettimeofday(&tv1, NULL);
    while (count--) {
        b = 1.164 * (y - 16) + 2.018 * (u - 128);
        g = 1.164 * (y - 16) - 0.813 * (v - 128) - 0.391 * (u - 128);
        r = 1.164 * (y - 16) + 1.596 * (v - 128);
        y ++;
        u ++;
        v ++;
    }
    gettimeofday(&tv2, NULL);
    elapse = (tv2.tv_sec - tv1.tv_sec) * 1000 + (tv2.tv_usec - tv1.tv_usec) / 1000;
    LOG(STF_LEVEL_INFO, "elapse: %lldms, out: r=%f, g=%f, b=%f\n",
            elapse ,r, g, b);

    count = 1920 * 1080 * 2;
    gettimeofday(&tv1, NULL);
    while (count--) {
        int_b = 1164 * (y - 16) + 2018 * (u - 128);
        int_g = 1164 * (y - 16) - 813 * (v - 128) - 391 * (u - 128);
        int_r = 1164 * (y - 16) + 1596 * (v - 128);
        int_y ++;
        int_u ++;
        int_v ++;
    }
    gettimeofday(&tv2, NULL);
    elapse = (tv2.tv_sec - tv1.tv_sec) + (tv2.tv_usec - tv1.tv_usec);
    LOG(STF_LEVEL_INFO, "elapse: %lldus, out: r=%d, g=%d, b=%d\n",
            elapse , int_r, int_g, int_b);

    count = 1920 * 1080 * 2;
    unsigned char* arraybuf = NULL;
    arraybuf = (unsigned char*)malloc(count);
    if (!arraybuf) {
        LOG(STF_LEVEL_ERR, "arraybuf malloc error\n");
        return;
    }

    unsigned char* arraybuf2 = NULL;
    arraybuf2 = (unsigned char*)malloc(count);
    if (!arraybuf2) {
        LOG(STF_LEVEL_ERR, "arraybuf2 malloc error\n");
        return;
    }

    gettimeofday(&tv1, NULL);

    for (i = 0; i < count; i++) {
        arraybuf[i] = i + 1;
    }

    gettimeofday(&tv2, NULL);

    memcpy(arraybuf2, arraybuf, count);

    gettimeofday(&tv3, NULL);

    elapse = (tv2.tv_sec - tv1.tv_sec) * 1000 + (tv2.tv_usec - tv1.tv_usec) / 1000;
    LOG(STF_LEVEL_INFO, "for() elapse: %lldms\n", elapse);

    elapse = (tv3.tv_sec - tv2.tv_sec) * 1000000 + (tv3.tv_usec - tv2.tv_usec);
    LOG(STF_LEVEL_INFO, "memcpy() run elapse: %lldus\n", elapse);

    convert_nv21_to_rgb((const uint8_t*)arraybuf, arraybuf2, 1920, 1080, 1);

    free(arraybuf);
    free(arraybuf2);
}

int write_file(char * filename, const uint8_t *image_buffer, int size)
{
    /*  More stuff */
    FILE * outfile;               /*  target file */
    if ((outfile = fopen(filename, "w+")) == NULL) {
        LOG(STF_LEVEL_ERR, "can't open %s\n", filename);
        return -1;
    }
    fwrite(image_buffer, size, 1, outfile);
    fclose(outfile);
    return 0 ;
}

int write_JPEG_file(char * filename, uint8_t *image_buffer,
        int image_width, int image_height, int quality)
{
    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;

    /*  More stuff */
    FILE * outfile;               /*  target file */
    JSAMPROW row_pointer[1];      /*  pointer to JSAMPLE row[s] */
    int row_stride;               /*  physical row width in image buffer */

    /*  Step 1: allocate and initialize JPEG compression object */
    cinfo.err = jpeg_std_error(&jerr);
    /*  Now we can initialize the JPEG compression object. */
    jpeg_create_compress(&cinfo);

    /*  Step 2: specify data destination (eg, a file) */
    /*  Note: steps 2 and 3 can be done in either order. */
    if ((outfile = fopen(filename, "w+")) == NULL) {
        LOG(STF_LEVEL_ERR, "can't open %s\n", filename);
        return -1;
    }
    jpeg_stdio_dest(&cinfo, outfile);

    /*  Step 3: set parameters for compression */
    cinfo.image_width = image_width;      /*  image width and height, in pixels */
    cinfo.image_height = image_height;
    cinfo.input_components = 3;           /*  # of color components per pixel */
    cinfo.in_color_space = JCS_RGB;       /*  colorspace of input image */
    jpeg_set_defaults(&cinfo);
    jpeg_set_quality(&cinfo, quality, TRUE /*  limit to baseline-JPEG values */);

    /*  Step 4: Start compressor */
    jpeg_start_compress(&cinfo, TRUE);

    /*  Step 5: while (scan lines remain to be written) */
    row_stride = image_width * 3; /*  JSAMPLEs per row in image_buffer */
    while (cinfo.next_scanline < cinfo.image_height) {
        row_pointer[0] = & image_buffer[cinfo.next_scanline * row_stride];
        (void) jpeg_write_scanlines(&cinfo, row_pointer, 1);
    }

    /*  Step 6: Finish compression */
    jpeg_finish_compress(&cinfo);
    /*  After finish_compress, we can close the output file. */
    fclose(outfile);

    /*  Step 7: release JPEG compression object */
    /*  This is an important step since it will release a good deal of memory. */
    jpeg_destroy_compress(&cinfo);

    /*  And we're done! */
    return 0;
}

void jpegWrite(uint8_t* img, char* jpegFilename,
        uint32_t width, uint32_t height, int jpegQuality)
{
    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;

    JSAMPROW row_pointer[1];
    FILE *outfile = fopen( jpegFilename, "wb" );

    // try to open file for saving
    if (!outfile) {
        errno_exit("jpeg");
    }

    // create jpeg data
    cinfo.err = jpeg_std_error( &jerr );
    jpeg_create_compress(&cinfo);
    jpeg_stdio_dest(&cinfo, outfile);

    // set image parameters
    cinfo.image_width = width;
    cinfo.image_height = height;
    cinfo.input_components = 3;
    cinfo.in_color_space = JCS_YCbCr;

    // set jpeg compression parameters to default
    jpeg_set_defaults(&cinfo);
    // and then adjust quality setting
    jpeg_set_quality(&cinfo, jpegQuality, TRUE);

    // start compress
    jpeg_start_compress(&cinfo, TRUE);

    // feed data
    while (cinfo.next_scanline < cinfo.image_height) {
        row_pointer[0] = &img[cinfo.next_scanline * cinfo.image_width *  cinfo.input_components];
        jpeg_write_scanlines(&cinfo, row_pointer, 1);
    }

    // finish compression
    jpeg_finish_compress(&cinfo);

    // destroy jpeg data
    jpeg_destroy_compress(&cinfo);

    // close output file
    fclose(outfile);
}


