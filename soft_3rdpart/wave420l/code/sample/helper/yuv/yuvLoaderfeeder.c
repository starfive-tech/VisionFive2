//------------------------------------------------------------------------------
// File: vpuio.c
//
// Copyright (c) 2006, Chips & Media.  All rights reserved.
//------------------------------------------------------------------------------

#include <string.h>
#include "main_helper.h"

//#define SAVE_WHOLE_LOADED_YUV
//#define DUMP_REFC_TYPE

#ifdef SAVE_WHOLE_LOADED_YUV
FILE *save_fp;
int save_frame_cnt=0;
int save_frame_max=10;
#endif

typedef struct {
    osal_file_t* fp;
    Uint8*       pYuv;

    Uint32       cbcrInterleave;
    Uint32       nv21;
    Uint32       packedFormat;
} Context;

enum {
    OUTPUT_PLANAR           = 0,
    OUTPUT_NV12             = 2,
    OUTPUT_NV21             = 3,
    OUTPUT_PACKED           = 4,
};

typedef unsigned short pel; // the size of pel should be 16.
typedef struct{
    pel* src_buf[3];
    Int32   rot_mode;
    Int32   src_stride;
    Int32   src_width;
    Int32   src_height;

    Int32   padded_y_stride;
    Int32   padded_c_stride;

    Int32   padded_y_height;
    Int32   padded_c_height;

    Int32   enc_stride;
    Int32   enc_width;
    Int32   enc_height;

    Int32   enc_c_stride;

    pel* p_out_buf[3];
    pel* out_buf;
    pel* pad_buf;
}prp_t;

void conv_image (unsigned short *outbuf, unsigned short *inbuf, 
    int pic_width, int pic_height, int out_width, int out_height, int frm_format, int packedFormat)
{
    int	x, y;
    int iaddr, oaddr;
    int temp, tbase;

    if ( packedFormat == NOT_PACKED )
    {
        if (frm_format == OUTPUT_PLANAR) {      // planar
            // Y
            iaddr = 0;
            oaddr = 0;
            for (y=0; y<out_height; y++) {
                for (x=0; x<out_width; x++) {
                    if (y >= pic_height) {
                        if (x >= pic_width)
                            outbuf[oaddr] = temp;
                        else {
                            temp          = inbuf[x + pic_width*(pic_height-1)];
                            outbuf[oaddr] = temp;
                        }
                    } else {
                        if (x >= pic_width)
                            outbuf[oaddr] = temp;
                        else {
                            temp          = inbuf[iaddr];
                            outbuf[oaddr] = temp;
                            iaddr++;
                        }
                    }
                    oaddr++;
                }
            }
            // U
            tbase = pic_width*pic_height;
            iaddr = tbase;
            oaddr = out_width*out_height;
            for (y=0; y<out_height/2; y++) {
                for (x=0; x<out_width/2; x++) {
                    if (y >= pic_height/2) {
                        if (x >= pic_width/2)
                            outbuf[oaddr] = temp;
                        else {
                            temp          = inbuf[tbase + x + (pic_width/2)*(pic_height/2-1)];
                            outbuf[oaddr] = temp;
                        }
                    } else {
                        if (x >= pic_width/2)
                            outbuf[oaddr] = temp;
                        else {
                            temp          = inbuf[iaddr];
                            outbuf[oaddr] = temp;
                            iaddr++;
                        }
                    }
                    oaddr++;
                }
            }
            // V
            tbase = pic_width*pic_height*5/4;
            iaddr = tbase;
            oaddr = out_width*out_height*5/4;
            for (y=0; y<out_height/2; y++) {
                for (x=0; x<out_width/2; x++) {
                    if (y >= pic_height/2) {
                        if (x >= pic_width/2)
                            outbuf[oaddr] = temp;
                        else {
                            temp          = inbuf[tbase + x + (pic_width/2)*(pic_height/2-1)];
                            outbuf[oaddr] = temp;
                        }
                    } else {
                        if (x >= pic_width/2)
                            outbuf[oaddr] = temp;
                        else {
                            temp          = inbuf[iaddr];
                            outbuf[oaddr] = temp;
                            iaddr++;
                        }
                    }
                    oaddr++;
                }
            }
        } else if (frm_format == OUTPUT_NV12 || frm_format == OUTPUT_NV21) {   // NV12, NV21
            // Y
            iaddr = 0;
            oaddr = 0;
            for (y=0; y<out_height; y++) {
                for (x=0; x<out_width; x++) {
                    if (y >= pic_height) {
                        if (x >= pic_width)
                            outbuf[oaddr] = temp;
                        else {
                            temp          = inbuf[x + pic_width*(pic_height-1)];
                            outbuf[oaddr] = temp;
                        }
                    } else {
                        if (x >= pic_width)
                            outbuf[oaddr] = temp;
                        else {
                            temp          = inbuf[iaddr];
                            outbuf[oaddr] = temp;
                            iaddr++;
                        }
                    }
                    oaddr++;
                }
            }
            // U
            tbase = (frm_format == OUTPUT_NV12) ? pic_width*pic_height : pic_width*pic_height*5/4;
            iaddr = tbase;
            oaddr = out_width*out_height;
            for (y=0; y<out_height/2; y++) {
                for (x=0; x<out_width/2; x++) {
                    if (y >= pic_height/2) {
                        if (x >= pic_width/2)
                            outbuf[oaddr] = temp;
                        else {
                            temp          = inbuf[tbase + x + (pic_width/2)*(pic_height/2-1)];
                            outbuf[oaddr] = temp;
                        }
                    } else {
                        if (x >= pic_width/2)
                            outbuf[oaddr] = temp;
                        else {
                            temp          = inbuf[iaddr];
                            outbuf[oaddr] = temp;
                            iaddr++;
                        }
                    }
                    oaddr+=2;
                }
            }
            // V
            tbase = (frm_format == OUTPUT_NV12) ? pic_width*pic_height*5/4 : pic_width*pic_height;
            iaddr = tbase;
            oaddr = out_width*out_height + 1;
            for (y=0; y<out_height/2; y++) {
                for (x=0; x<out_width/2; x++) {
                    if (y >= pic_height/2) {
                        if (x >= pic_width/2)
                            outbuf[oaddr] = temp;
                        else {
                            temp          = inbuf[tbase + x + (pic_width/2)*(pic_height/2-1)];
                            outbuf[oaddr] = temp;
                        }
                    } else {
                        if (x >= pic_width/2)
                            outbuf[oaddr] = temp;
                        else {
                            temp          = inbuf[iaddr];
                            outbuf[oaddr] = temp;
                            iaddr++;
                        }
                    }
                    oaddr+=2;
                }
            }
        } 
        else {
            printf("Error) Unknown frame format: %d\n", frm_format);
        }
    } else {/* packdformat */
        if ( packedFormat == PACKED_YUYV || packedFormat == PACKED_YVYU){
            oaddr = 0;
        } else {
            oaddr = 1;
        }

        // Y
        iaddr = 0;
        for (y=0; y<out_height; y++) {
            for (x=0; x<out_width; x++) {
                if (y >= pic_height) {
                    if (x >= pic_width)
                        outbuf[oaddr] = temp;
                    else {
                        temp          = inbuf[x + pic_width*(pic_height-1)];
                        outbuf[oaddr] = temp;
                    }
                } else {
                    if (x >= pic_width)
                        outbuf[oaddr] = temp;
                    else {
                        temp          = inbuf[iaddr];
                        outbuf[oaddr] = temp;
                        iaddr++;
                    }
                }
                oaddr+=2;
            }
        }

        // U
        switch (packedFormat)
        {
        case PACKED_YUYV: oaddr = 1;break;
        case PACKED_YVYU: oaddr = 3;break;
        case PACKED_UYVY: oaddr = 0;break;
        case PACKED_VYUY: oaddr = 2;break;
        default:
            VLOG(ERR, "%s:%d Not supported format(%d)\n", __FILE__, __LINE__, packedFormat);
            break;
        }
        tbase = pic_width*pic_height;
        iaddr = tbase;
        for (y=0; y<out_height; y++) {
            for (x=0; x<out_width/2; x++) {
                if (y >= pic_height) {
                    if (x >= pic_width/2)
                        outbuf[oaddr] = temp;
                    else {
                        temp          = inbuf[tbase + x + (pic_width/2)*(pic_height/2-1)];
                        outbuf[oaddr] = temp;
                    }
                } else {
                    if (x >= pic_width/2)
                        outbuf[oaddr] = temp;
                    else {
                        temp          = inbuf[iaddr];
                        outbuf[oaddr] = temp;
                        iaddr++;
                    }
                }
                oaddr+=4;
            }
            if (y%2 == 0)
                iaddr = iaddr - out_width/2;
        }
        // V
        switch (packedFormat)
        {
        case PACKED_YUYV: oaddr = 3;break;
        case PACKED_YVYU: oaddr = 1;break;
        case PACKED_UYVY: oaddr = 2;break;
        case PACKED_VYUY: oaddr = 0;break;
        default:
            VLOG(ERR, "%s:%d Not supported format(%d)\n", __FILE__, __LINE__, packedFormat);
            break;
        }
        tbase = pic_width*pic_height*5/4;
        iaddr = tbase;
        for (y=0; y<out_height; y++) {
            for (x=0; x<out_width/2; x++) {
                if (y >= pic_height) {
                    if (x >= pic_width/2)
                        outbuf[oaddr] = temp;
                    else {
                        temp          = inbuf[tbase + x + (pic_width/2)*(pic_height/2-1)];
                        outbuf[oaddr] = temp;
                    }
                } else {
                    if (x >= pic_width/2)
                        outbuf[oaddr] = temp;
                    else {
                        temp          = inbuf[iaddr];
                        outbuf[oaddr] = temp;
                        iaddr++;
                    }
                }
                oaddr+=4;
            }
            if (y%2 == 0)
                iaddr = iaddr - out_width/2;
        }
    }
}

int write_image_to_file (FILE *fp, unsigned short *outbuf, 
    int out_width, int out_height, int pxl_format, int frm_format, int packedFormat)
{
    int i, x, y;
    unsigned short tmp16;
    unsigned int   tmp32;
    int width, height;
    int size;
    unsigned char *temp = (unsigned char*)outbuf;

    size = (packedFormat) ? out_width*out_height*2 : out_width*out_height*3/2;
    //***** 8bit
    if (pxl_format == 0) {
        for (i=0; i<size; i++)
            fputc(temp[i]&0xff, fp);
    } 
    //***** 16bit
    else if (pxl_format == 1) {
        for (i=0; i<size; i++) {
            //tmp16 = (outbuf[i]&0x3ff)<<6;
            tmp16 = outbuf[i];
#ifdef DUMP_REFC_TYPE
            byte_swap((unsigned char *)&tmp16, 2);
#endif
            fputc(tmp16&0xff, fp);
            fputc((tmp16>>8)&0xff, fp);
            
        }
    } else if (pxl_format == 5) {
        for (i=0; i<size; i++) {
            //tmp16 = outbuf[i]&0x3ff;
            tmp16 = outbuf[i];
#ifdef DUMP_REFC_TYPE
            byte_swap((unsigned char *)&tmp16, 2);
#endif
            fputc(tmp16&0xff, fp);
            fputc((tmp16>>8)&0xff, fp);
            
        }
    } 
    //***** 32bit
    else {    
        // Y
        width  = (packedFormat) ? out_width*2  : out_width;
        height = out_height;
        for (y=0; y<height; y++) {
            for (x=0; x<width; x=x+3) {
                if (pxl_format == 2) {
                    if (x+1 >= width)
                        tmp32 = ((outbuf[x+0+width*y]&0x3ff)<<22);
                    else if (x+2 >= width)
                        tmp32 = ((outbuf[x+0+width*y]&0x3ff)<<22) | 
                        ((outbuf[x+1+width*y]&0x3ff)<<12);
                    else
                        tmp32 = ((outbuf[x+0+width*y]&0x3ff)<<22) | 
                        ((outbuf[x+1+width*y]&0x3ff)<<12) |
                        ((outbuf[x+2+width*y]&0x3ff)<<2);
                } else if (pxl_format == 6) {
                    if (x+1 >= width)
                        tmp32 = (outbuf[x+0+width*y]&0x3ff);
                    else if (x+2 >= width)
                        tmp32 =  (outbuf[x+0+width*y]&0x3ff) | 
                        ((outbuf[x+1+width*y]&0x3ff)<<10);
                    else
                        tmp32 =  (outbuf[x+0+width*y]&0x3ff) | 
                        ((outbuf[x+1+width*y]&0x3ff)<<10) |
                        ((outbuf[x+2+width*y]&0x3ff)<<20);
                }
                fputc((tmp32>>24)&0xff, fp);
                fputc((tmp32>>16)&0xff, fp);
                fputc((tmp32>>8)&0xff, fp);
                fputc(tmp32&0xff, fp);
            }
        }
        if (packedFormat)    // packed
            return 1;

        width  = (frm_format == 2 || frm_format == 3) ? out_width  : out_width/2;
        height = out_height/2;
        // Cb
        for (y=0; y<height; y++) {
            for (x=0; x<width; x=x+3) {
                if (pxl_format == 2) {
                    if (x+1 >= width)
                        tmp32 = ((outbuf[out_width*out_height+x+0+width*y]&0x3ff)<<22);
                    else if (x+2 >= width)
                        tmp32 = ((outbuf[out_width*out_height+x+0+width*y]&0x3ff)<<22) | 
                        ((outbuf[out_width*out_height+x+1+width*y]&0x3ff)<<12);
                    else
                        tmp32 = ((outbuf[out_width*out_height+x+0+width*y]&0x3ff)<<22) | 
                        ((outbuf[out_width*out_height+x+1+width*y]&0x3ff)<<12) |
                        ((outbuf[out_width*out_height+x+2+width*y]&0x3ff)<<2);
                } else if (pxl_format == 6) {
                    if (x+1 >= width)
                        tmp32 = (outbuf[out_width*out_height+x+0+width*y]&0x3ff);
                    else if (x+2 >= width)
                        tmp32 =  (outbuf[out_width*out_height+x+0+width*y]&0x3ff) | 
                        ((outbuf[out_width*out_height+x+1+width*y]&0x3ff)<<10);
                    else
                        tmp32 =  (outbuf[out_width*out_height+x+0+width*y]&0x3ff) | 
                        ((outbuf[out_width*out_height+x+1+width*y]&0x3ff)<<10) |
                        ((outbuf[out_width*out_height+x+2+width*y]&0x3ff)<<20);
                }
                fputc((tmp32>>24)&0xff, fp);
                fputc((tmp32>>16)&0xff, fp);
                fputc((tmp32>>8)&0xff, fp);
                fputc(tmp32&0xff, fp);
            }
        }
        if (frm_format == 2 || frm_format == 3)    // interleave
            return 1;

        width  = out_width/2;
        height = out_height/2;
        // Cr
        for (y=0; y<height; y++) {
            for (x=0; x<width; x=x+3) {
                if (pxl_format == 2) {
                    if (x+1 >= width)
                        tmp32 = ((outbuf[out_width*out_height*5/4+x+0+width*y]&0x3ff)<<22);
                    else if (x+2 >= width)
                        tmp32 = ((outbuf[out_width*out_height*5/4+x+0+width*y]&0x3ff)<<22) | 
                        ((outbuf[out_width*out_height*5/4+x+1+width*y]&0x3ff)<<12);
                    else
                        tmp32 = ((outbuf[out_width*out_height*5/4+x+0+width*y]&0x3ff)<<22) | 
                        ((outbuf[out_width*out_height*5/4+x+1+width*y]&0x3ff)<<12) |
                        ((outbuf[out_width*out_height*5/4+x+2+width*y]&0x3ff)<<2);
                } else if (pxl_format == 6) {
                    if (x+1 >= width)
                        tmp32 = (outbuf[out_width*out_height*5/4+x+0+width*y]&0x3ff);
                    else if (x+2 >= width)
                        tmp32 =  (outbuf[out_width*out_height*5/4+x+0+width*y]&0x3ff) | 
                        ((outbuf[out_width*out_height*5/4+x+1+width*y]&0x3ff)<<10);
                    else
                        tmp32 =  (outbuf[out_width*out_height*5/4+x+0+width*y]&0x3ff) | 
                        ((outbuf[out_width*out_height*5/4+x+1+width*y]&0x3ff)<<10) |
                        ((outbuf[out_width*out_height*5/4+x+2+width*y]&0x3ff)<<20);
                }
                fputc((tmp32>>24)&0xff, fp);
                fputc((tmp32>>16)&0xff, fp);
                fputc((tmp32>>8)&0xff, fp);
                fputc(tmp32&0xff, fp);
            }
        }
    }
    return 1;
}

int write_image_to_mem (unsigned short *out_mem, unsigned short *inbuf, 
    int out_width, int out_height, int pxl_format, int frm_format, int packedFormat)
{
    int i, x, y;
    unsigned int   tmp32;
    int width, height;
    int size;
    int oaddr=0;
    unsigned char *temp = (unsigned char *)out_mem;

    size = (packedFormat) ? out_width*out_height*2 : out_width*out_height*3/2;
    
    // 8BIT
    if (pxl_format == 0) {
        for (i=0; i<size; i++)
            temp[oaddr++] = inbuf[i]&0xff;
    } 
    // 16BIT
    else if (pxl_format == 1 || pxl_format == 5) {
        for (i=0; i<size; i++) {
            out_mem[oaddr++] = inbuf[i];
        }
    } 
    // 32BIT
    else {    
        // Y
        width  = (packedFormat) ? out_width*2  : out_width;
        height = out_height;
        for (y=0; y<height; y++) {
            for (x=0; x<width; x=x+3) {
                if (pxl_format == 2) {
                    if (x+1 >= width)
                        tmp32 = ((inbuf[x+0+width*y]&0x3ff)<<22);
                    else if (x+2 >= width)
                        tmp32 = ((inbuf[x+0+width*y]&0x3ff)<<22) | 
                        ((inbuf[x+1+width*y]&0x3ff)<<12);
                    else
                        tmp32 = ((inbuf[x+0+width*y]&0x3ff)<<22) | 
                        ((inbuf[x+1+width*y]&0x3ff)<<12) |
                        ((inbuf[x+2+width*y]&0x3ff)<<2);
                } else if (pxl_format == 6) {
                    if (x+1 >= width)
                        tmp32 = (inbuf[x+0+width*y]&0x3ff);
                    else if (x+2 >= width)
                        tmp32 =  (inbuf[x+0+width*y]&0x3ff) | 
                        ((inbuf[x+1+width*y]&0x3ff)<<10);
                    else
                        tmp32 =  (inbuf[x+0+width*y]&0x3ff) | 
                        ((inbuf[x+1+width*y]&0x3ff)<<10) |
                        ((inbuf[x+2+width*y]&0x3ff)<<20);
                }
                temp[oaddr++] = (tmp32>>24)&0xff;
                temp[oaddr++] = (tmp32>>16)&0xff;
                temp[oaddr++] = (tmp32>>8)&0xff;
                temp[oaddr++] = tmp32&0xff;
            }
        }
        if (packedFormat)    // packed
            return 1;

        width  = (frm_format == 2 || frm_format == 3) ? out_width  : out_width/2;
        height = out_height/2;
        // Cb
        for (y=0; y<height; y++) {
            for (x=0; x<width; x=x+3) {
                if (pxl_format == 2) {
                    if (x+1 >= width)
                        tmp32 = ((inbuf[out_width*out_height+x+0+width*y]&0x3ff)<<22);
                    else if (x+2 >= width)
                        tmp32 = ((inbuf[out_width*out_height+x+0+width*y]&0x3ff)<<22) | 
                        ((inbuf[out_width*out_height+x+1+width*y]&0x3ff)<<12);
                    else
                        tmp32 = ((inbuf[out_width*out_height+x+0+width*y]&0x3ff)<<22) | 
                        ((inbuf[out_width*out_height+x+1+width*y]&0x3ff)<<12) |
                        ((inbuf[out_width*out_height+x+2+width*y]&0x3ff)<<2);
                } else if (pxl_format == 6) {
                    if (x+1 >= width)
                        tmp32 = (inbuf[out_width*out_height+x+0+width*y]&0x3ff);
                    else if (x+2 >= width)
                        tmp32 =  (inbuf[out_width*out_height+x+0+width*y]&0x3ff) | 
                        ((inbuf[out_width*out_height+x+1+width*y]&0x3ff)<<10);
                    else
                        tmp32 =  (inbuf[out_width*out_height+x+0+width*y]&0x3ff) | 
                        ((inbuf[out_width*out_height+x+1+width*y]&0x3ff)<<10) |
                        ((inbuf[out_width*out_height+x+2+width*y]&0x3ff)<<20);
                }
                temp[oaddr++] = (tmp32>>24)&0xff;
                temp[oaddr++] = (tmp32>>16)&0xff;
                temp[oaddr++] = (tmp32>>8)&0xff;
                temp[oaddr++] = tmp32&0xff;
            }
        }
        if (frm_format == 2 || frm_format == 3)    // interleave
            return 1;

        width  = out_width/2;
        height = out_height/2;
        // Cr
        for (y=0; y<height; y++) {
            for (x=0; x<width; x=x+3) {
                if (pxl_format == 2) {
                    if (x+1 >= width)
                        tmp32 = ((inbuf[out_width*out_height*5/4+x+0+width*y]&0x3ff)<<22);
                    else if (x+2 >= width)
                        tmp32 = ((inbuf[out_width*out_height*5/4+x+0+width*y]&0x3ff)<<22) | 
                        ((inbuf[out_width*out_height*5/4+x+1+width*y]&0x3ff)<<12);
                    else
                        tmp32 = ((inbuf[out_width*out_height*5/4+x+0+width*y]&0x3ff)<<22) | 
                        ((inbuf[out_width*out_height*5/4+x+1+width*y]&0x3ff)<<12) |
                        ((inbuf[out_width*out_height*5/4+x+2+width*y]&0x3ff)<<2);
                } else if (pxl_format == 6) {
                    if (x+1 >= width)
                        tmp32 = (inbuf[out_width*out_height*5/4+x+0+width*y]&0x3ff);
                    else if (x+2 >= width)
                        tmp32 =  (inbuf[out_width*out_height*5/4+x+0+width*y]&0x3ff) | 
                        ((inbuf[out_width*out_height*5/4+x+1+width*y]&0x3ff)<<10);
                    else
                        tmp32 =  (inbuf[out_width*out_height*5/4+x+0+width*y]&0x3ff) | 
                        ((inbuf[out_width*out_height*5/4+x+1+width*y]&0x3ff)<<10) |
                        ((inbuf[out_width*out_height*5/4+x+2+width*y]&0x3ff)<<20);
                }
                temp[oaddr++] = (tmp32>>24)&0xff;
                temp[oaddr++] = (tmp32>>16)&0xff;
                temp[oaddr++] = (tmp32>>8)&0xff;
                temp[oaddr++] = tmp32&0xff;
            }
        }
    }
    return 1;
}

BOOL loaderYuvFeeder_Create(
    YuvFeederImpl* impl,
    const char*    path,
    Uint32         packed,
    Uint32         fbStride,
    Uint32         fbHeight)
{
    Context*        ctx;
    osal_file_t*    fp;
    Uint8*          pYuv;

    if ((fp=osal_fopen(path, "rb")) == NULL) {
        VLOG(ERR, "%s:%d failed to open yuv file: %s\n", __FUNCTION__, __LINE__, path);
        return FALSE;
    }

    if ( packed == 1 )
        pYuv = osal_malloc(fbStride*fbHeight*3*2);//packed, unsigned short
    else
        pYuv = osal_malloc(fbStride*fbHeight*3);//unsigned short

    if ((ctx=(Context*)osal_malloc(sizeof(Context))) == NULL) {
        osal_free(pYuv);
        osal_fclose(fp);
        return FALSE;
    }

    osal_memset(ctx, 0, sizeof(Context));

    ctx->fp       = fp;
    ctx->pYuv     = pYuv;
    impl->context = ctx;

    return TRUE;
}

BOOL loaderYuvFeeder_Destory(
    YuvFeederImpl *impl
    )
{
    Context*    ctx = (Context*)impl->context;

    osal_fclose(ctx->fp);
    osal_free(ctx->pYuv);
    osal_free(ctx);
    return TRUE;
}


BOOL loaderYuvFeeder_Feed(
    YuvFeederImpl*  impl,
    Int32           coreIdx,
    FrameBuffer*    fb,
    size_t          picWidth,
    size_t          picHeight,
    void*           arg
    )
{
    size_t      srcFrameSize, lumaSize, chromaSize;
    Int32       ystride;
    Int32       out_frm_format;
    Context*    ctx = (Context*)impl->context;

    Uint8*      pYuv_int8 = ctx->pYuv;
    osal_file_t yuvFp = ctx->fp;
    Uint16*     pYuv_int16 = NULL, *pYuv2_int16 = NULL;
    Int32       outsize, out_width, out_height, pxl_format, yuv3p4b = 0;
    Int32       bitdepth = 0;
    Uint32      i;

    UNREFERENCED_PARAMETER(arg);

    if ( fb->cbcrInterleave == TRUE ) {
        if ( ctx->nv21)
            out_frm_format = OUTPUT_NV21 ;
        else
            out_frm_format = OUTPUT_NV12 ;
    } 
    else {
        out_frm_format = OUTPUT_PLANAR;
    }

    switch (fb->format) {
    case FORMAT_420:
    case FORMAT_YUYV:
    case FORMAT_YVYU:
    case FORMAT_UYVY:
    case FORMAT_VYUY:
        pxl_format = 0;
        break;
    case FORMAT_420_P10_16BIT_LSB:
    case FORMAT_YUYV_P10_16BIT_LSB:
    case FORMAT_YVYU_P10_16BIT_LSB:
    case FORMAT_UYVY_P10_16BIT_LSB:
    case FORMAT_VYUY_P10_16BIT_LSB:
        bitdepth = 10;
        pxl_format = 5;
        break;
    case FORMAT_420_P10_16BIT_MSB:
    case FORMAT_YUYV_P10_16BIT_MSB:
    case FORMAT_YVYU_P10_16BIT_MSB:
    case FORMAT_UYVY_P10_16BIT_MSB:
    case FORMAT_VYUY_P10_16BIT_MSB:
        bitdepth = 10;
        pxl_format = 1;
        break;
    case FORMAT_420_P10_32BIT_LSB:
    case FORMAT_YUYV_P10_32BIT_LSB:
    case FORMAT_YVYU_P10_32BIT_LSB:
    case FORMAT_UYVY_P10_32BIT_LSB:
    case FORMAT_VYUY_P10_32BIT_LSB:
        bitdepth = 10;
        pxl_format = 6;
        yuv3p4b = 1;
        break;
    case FORMAT_420_P10_32BIT_MSB:
    case FORMAT_YUYV_P10_32BIT_MSB: 
    case FORMAT_YVYU_P10_32BIT_MSB:
    case FORMAT_UYVY_P10_32BIT_MSB:
    case FORMAT_VYUY_P10_32BIT_MSB:
        bitdepth = 10;
        pxl_format = 2;
        yuv3p4b = 1;
        break;
    default:
        VLOG(ERR, "%s:%d Not supported format(%d)\n", __FILE__, __LINE__, fb->format);
        break;
    }

    {
        int input_format = fb->format;
        input_format = FORMAT_420;
        if ( yuv3p4b == 1 )//calculate input YUV(16BIT) size 
            CalcYuvSize(FORMAT_420_P10_16BIT_LSB, picWidth, picHeight, fb->cbcrInterleave, &lumaSize, &chromaSize, &srcFrameSize, NULL, NULL, NULL);
        else if (bitdepth == 10)
            CalcYuvSize(FORMAT_420_P10_16BIT_LSB, picWidth, picHeight, fb->cbcrInterleave, &lumaSize, &chromaSize, &srcFrameSize, NULL, NULL, NULL);
        else
            CalcYuvSize(input_format, picWidth, picHeight, fb->cbcrInterleave, &lumaSize, &chromaSize, &srcFrameSize, NULL, NULL, NULL);
    }

    out_width  = (yuv3p4b&&ctx->packedFormat==0) ? ((picWidth+31)/32)*32  : picWidth;
    out_height = (yuv3p4b) ? ((picHeight+7)/8)*8 : picHeight;

    outsize = (ctx->packedFormat) ? out_width*out_height*2 : out_width*out_height*3/2;

    outsize*=2;//align margin
    if ((pYuv_int16 = (Uint16*)osal_malloc(outsize * sizeof(Uint16))) == NULL) {
        printf("malloc error: outbuf\n");
        return 0;
    }
    if ((pYuv2_int16 = (Uint16*)osal_malloc(outsize * sizeof(Uint16))) == NULL) {
        printf("malloc error: outbuf\n");
        return 0;
    }

    if ( bitdepth == 0 ) {
        if( !osal_fread(pYuv_int8, 1, srcFrameSize, yuvFp) ) 
        {
            if( !osal_feof( yuvFp ) )
                VLOG(ERR, "YUV Data osal_fread failed file handle is 0x%x \n", yuvFp );
            
            if ( pYuv_int16 )
                osal_free(pYuv_int16);
            if ( pYuv2_int16 )
                osal_free(pYuv2_int16);
            return 0;
        }
        for (i=0; i<srcFrameSize; i++)
            pYuv_int16[i] = (unsigned short) pYuv_int8[i];
    }
    else {
        if( !osal_fread(pYuv_int16, 1, srcFrameSize, yuvFp) ) 
        {
            if( !osal_feof( yuvFp ) )
                VLOG(ERR, "YUV Data osal_fread failed file handle is 0x%x \n", yuvFp );
            
            if ( pYuv_int16 )
                osal_free(pYuv_int16);
            if ( pYuv2_int16 )
                osal_free(pYuv2_int16);
            return 0;
        }
        if ( yuv3p4b == 0 )
        {
            /**************************************************************
            * when 10bit source YUV == LITTLE ENDIAN, 16LSB, alway do byte-swapping 
            **************************************************************/
            for (i=0; i<srcFrameSize/2; i++) { 
                pYuv_int8[i*2] = (Uint8)(pYuv_int16[i]>>8)&0xff;    
                pYuv_int8[i*2 + 1] = (Uint8)(pYuv_int16[i])&0xff;
            }
#if 0
            /***************************************************************
            * when 10bit source YUV == BIG ENDIAN, 16LSB
            **************************************************************/
            osal_memcpy(pYuv_int8, pYuv_int16, srcFrameSize);
#endif
        }
        else
        {
            /***************************************************************
            * when 10bit source YUV == LITTLE ENDIAN, 16LSB
            **************************************************************/
            osal_memcpy(pYuv_int8, pYuv_int16, srcFrameSize);
#if 0
            /***************************************************************
            * when 10bit source YUV == BIG ENDIAN, 16LSB
            **************************************************************/
            for (i=0; i<srcFrameSize/2; i++) { 
                pYuv_int8[i*2] = (Uint8)(pYuv_int16[i]>>8)&0xff;    
                pYuv_int8[i*2 + 1] = (Uint8)(pYuv_int16[i])&0xff;
            }
#endif
        }
    }
    if (yuv3p4b==1 && ctx->packedFormat) {
        ystride = picWidth;
    } else if (yuv3p4b == 1) {
        ystride = VPU_ALIGN32(picWidth);
    } else {
        ystride = VPU_ALIGN16(picWidth);
    }

    if ( out_frm_format != OUTPUT_PLANAR || ctx->packedFormat || yuv3p4b )
    {
        conv_image((unsigned short *)pYuv2_int16, ((bitdepth==0) ? ((unsigned short*)pYuv_int16) : ((unsigned short *)pYuv_int8)), picWidth, picHeight, out_width, out_height, out_frm_format, ctx->packedFormat);
        write_image_to_mem((unsigned short *)pYuv_int8, (unsigned short *)pYuv2_int16, ystride, out_height, pxl_format, out_frm_format, ctx->packedFormat);
    }

#if 0
    {
        char fileName[MAX_FILE_PATH];
#ifdef SAVE_WHOLE_LOADED_YUV
#else
        FILE *save_fp;
#endif
        char *frm_string;
        char *pixel_string;
        if ( ctx->packedFormat == 1)
            frm_string = "YUYV";
        else if ( ctx->packedFormat == 2)
            frm_string = "YVYU";
        else if ( ctx->packedFormat == 3)
            frm_string = "UYVY";
        else if ( ctx->packedFormat == 4)
            frm_string = "VYUY";
        else if ( out_frm_format == OUTPUT_NV12 ) {
            frm_string = "NV12";
        } else if ( out_frm_format == OUTPUT_NV21 ) {
            frm_string = "NV21";
        }
        else
        {
            frm_string = "I420";
        }

        if ( yuv3p4b )
            pixel_string = "3P4B";
        else if ( bitdepth == 10 )
            pixel_string = "1P2B";
        else
            pixel_string = "8b";

        sprintf(fileName, "loaded_%s_%dx%d_%s.yuv", pixel_string, picWidth, picHeight, frm_string);

        VLOG(INFO, "save %s\n", fileName);
#ifdef SAVE_WHOLE_LOADED_YUV
        if ( save_frame_cnt == 0)
            save_fp = osal_fopen(fileName, "wb");
#else
        save_fp = osal_fopen(fileName, "wb");
#endif
        if ( yuv3p4b == 0 )
            write_image_to_file(save_fp, (unsigned short *)pYuv_int8, out_width, out_height, pxl_format, out_frm_format, ctx->packedFormat);
        else
            write_image_to_file(save_fp, (unsigned short *)pYuv2_int16, out_width, out_height, pxl_format, out_frm_format, ctx->packedFormat);
        osal_fflush(save_fp);
#ifdef SAVE_WHOLE_LOADED_YUV
        if ( ++save_frame_cnt == save_frame_max )
            osal_fclose(save_fp);
#else
        osal_fclose(save_fp);
#endif
    }
#endif

    if ( yuv3p4b  && ctx->packedFormat) {
        out_width = ((picWidth*2)+2)/3*4;
    }
    else if(ctx->packedFormat) {
        out_width *= 2;           // 8bit packed mode(YUYV) only. (need to add 10bit cases later)
        if (bitdepth != 0)      // 10bit packed
            out_width *= 2;
    }
    LoadYuvImageBurstFormat(coreIdx, pYuv_int8, out_width, out_height, fb, TRUE);


    if ( pYuv_int16 )
        osal_free(pYuv_int16);
    if ( pYuv2_int16 )
        osal_free(pYuv2_int16);

    return TRUE;
}

BOOL loaderYuvFeeder_Configure(
    YuvFeederImpl* impl,
    Uint32         cmd,
    YuvInfo        yuv
    )
{
    Context* ctx = (Context*)impl->context;
    UNREFERENCED_PARAMETER(cmd);

    ctx->cbcrInterleave = yuv.cbcrInterleave;
    ctx->nv21           = yuv.nv21;
    ctx->packedFormat   = yuv.packedFormat;

    return TRUE;
}

YuvFeederImpl loaderYuvFeederImpl = {
    NULL,
    loaderYuvFeeder_Create,
    loaderYuvFeeder_Feed,
    loaderYuvFeeder_Destory,
    loaderYuvFeeder_Configure,
};
 
