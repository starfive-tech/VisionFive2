#ifndef _YUV_H_
#define _YUV_H_

void YUV420toYUV444(int width, int height, unsigned char* src, unsigned char* dst);
void YUV422toYUV444(int width, int height, unsigned char* src, unsigned char* dst);
void RGB565toRGB888(int width, int height, unsigned char* src, unsigned char* dst);
void RAW12toRAW16(int width, int height, const unsigned char* src, unsigned char* dst);
void YUV420NV21toYUV444(int width, int height, unsigned char* src, unsigned char* dst,
        int is_nv21);
#endif
