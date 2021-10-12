//--=========================================================================--
//  This file is a part of VPU Reference API project
//-----------------------------------------------------------------------------
//
//       This confidential and proprietary software may be used only
//     as authorized by a licensing agreement from Chips&Media Inc.
//     In the event of publication, the following notice is applicable:
//
//            (C) COPYRIGHT 2006 - 2013  CHIPS&MEDIA INC.
//                      ALL RIGHTS RESERVED
//
//       The entire notice above must be reproduced on all authorized
//       copies.
//
//--=========================================================================--

#include "main_helper.h"

typedef enum { 
    YUV444, YUV422, YUV420, NV12, NV21, 
    YUV400, YUYV, YVYU, UYVY, VYUY, YYY, 
    RGB_PLANAR, RGB32, RGB24, RGB16, 
    YUV2RGB_COLOR_FORMAT_MAX 
} YUV2RGBColorFmt;

#if defined(PLATFORM_LINUX) || defined(PLATFORM_QNX)
#include <unistd.h>
#endif

#if defined(PLATFORM_WIN32) || defined(PLATFORM_LINUX) || defined(PLATFORM_QNX)
static void ConvertYuvToRgb(
    Uint32      width, 
    Uint32      height, 
    YUV2RGBColorFmt format, 
    Uint8*      src, 
    Uint8*      rgba, 
    BOOL        chroma_reverse
    );

static YUV2RGBColorFmt ConvertToDeviceColorFormat(
    int yuv_format, 
    int interleave
    );
#endif // defined(PLATFORM_WIN32) || defined(PLATFORM_LINUX)

#if defined(PLATFORM_WIN32) 
#include <windows.h>
#include <stdio.h>

#pragma comment(lib,"User32.lib")
#pragma comment(lib,"gdi32.lib")

#define DRAW_IN_WINDOW
typedef struct
{
    BITMAPINFOHEADER bmih;
    RGBQUAD rgb[256];
} BITMAPINFO2;

static LRESULT CALLBACK WndProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

typedef struct 
{
    HBITMAP		s_dib_section;
    void		*s_dib_buffer;
    HDC			s_hdc_memory;
    HGDIOBJ		s_old_hobject;
    HWND	s_hWnd;
    int wndShow;
    Uint32      winWidth;
    Uint32      winHeight;
} FBDev;

static FBDev s_mixer[MAX_NUM_VPU_CORE*MAX_NUM_INSTANCE];
#endif	//defined(PLATFORM_WIN32) 

#ifdef PLATFORM_LINUX

#include <sys/ioctl.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/time.h>
#include <linux/fb.h>

typedef struct 
{
    int s_fd;
    unsigned char *s_scr_ptr;
    unsigned char *s_rgb_ptr;
    unsigned long s_product;
    int s_fb_stride;
    int s_fb_height;
    int s_fb_width;
    int s_fb_bpp;	
} FBDev;

static struct fb_var_screeninfo vscr_info;
static struct fb_fix_screeninfo fscr_info;

static FBDev s_mixer[MAX_NUM_VPU_CORE*MAX_NUM_INSTANCE];
#endif //PLATFORM_LINUX

static Int32 PlatformDev_Open(
    Uint32      coreIndex, 
    Int32       width, 
    Int32       height
    )
{	
#ifdef PLATFORM_LINUX	
    FBDev *mixer = &s_mixer[coreIndex];
    char fb_name[256];

    if (mixer->s_fd)
        return 1;
#ifdef ANDROID
    sprintf(fb_name, "/dev/graphics/fb%d", coreIndex);
#else
    sprintf(fb_name, "/dev/fb%d", coreIndex);
#endif

    mixer->s_fd = open(fb_name, O_RDWR);
    if (mixer->s_fd< 0) {
        VLOG(ERR, "Unable to open framebuffer %s\n", fb_name);
        return 0;
    }
    /** frame buffer display configuration get */
    if (ioctl(mixer->s_fd, FBIOGET_VSCREENINFO, &vscr_info) != 0 || ioctl(mixer->s_fd, FBIOGET_FSCREENINFO, &fscr_info) != 0) {
        VLOG(ERR, "Error during ioctl to get framebuffer parameters!\n");
        return 0;
    }
    mixer->s_fb_bpp = vscr_info.bits_per_pixel/8;
    mixer->s_fb_width = vscr_info.xres;
    mixer->s_fb_stride = fscr_info.line_length;
    mixer->s_fb_height = vscr_info.yres;
    mixer->s_product= mixer->s_fb_stride * mixer->s_fb_height;
    /** memory map frame buf memory */
    mixer->s_scr_ptr = (unsigned char*) mmap(0, mixer->s_product, PROT_READ | PROT_WRITE, MAP_SHARED, mixer->s_fd, 0);
    if (mixer->s_scr_ptr == NULL)
    {
        VLOG(ERR, "in %s Failed to mmap framebuffer memory!\n", __func__);
        close (mixer->s_fd);
        return 0;
    }

    mixer->s_rgb_ptr = osal_malloc(width*height*mixer->s_fb_bpp);
    if (mixer->s_rgb_ptr == NULL) {
        VLOG(ERR, "in %s Failed to allocate rgb memory!\n", __func__);
        close (mixer->s_fd);
        return 0;
    }
    VLOG(TRACE, "Successfully opened %s for display.\n", fb_name);
    VLOG(TRACE, "mmap framebuffer memory =%p product=%d\n", mixer->s_scr_ptr, (unsigned int)mixer->s_product, (int)width);
    VLOG(TRACE, "Display Size: width=%d, height=%d, stride=%d, Bit Count: %d \n", (int)mixer->s_fb_width, (int)mixer->s_fb_height, (int)mixer->s_fb_stride, (int)mixer->s_fb_bpp);

#endif	//#ifdef PLATFORM_LINUX

#if defined(PLATFORM_WIN32) 
    FBDev *mixer = &s_mixer[coreIndex];

    HDC hdc;
    BITMAPINFO2	bmi2;
    if (mixer->s_dib_section)
        return 0;

    osal_memset(&bmi2, 0x00, sizeof(bmi2));
    bmi2.bmih.biSize = sizeof(BITMAPINFOHEADER);
    bmi2.bmih.biWidth = width;
    bmi2.bmih.biHeight = -(height);
    bmi2.bmih.biPlanes = 1;
    bmi2.bmih.biBitCount = 32;
    bmi2.bmih.biCompression = BI_RGB;
    if( bmi2.bmih.biBitCount == 16 )
    {
        bmi2.bmih.biCompression = BI_BITFIELDS;
        *(DWORD *)(&bmi2.rgb[0]) = 0xF800;
        *(DWORD *)(&bmi2.rgb[1]) = 0x07e0;
        *(DWORD *)(&bmi2.rgb[2]) = 0x001F;
    }
    else
        bmi2.bmih.biCompression = BI_RGB;

    mixer->s_dib_section = CreateDIBSection(
        NULL,
        (PBITMAPINFO)&bmi2,
        DIB_RGB_COLORS,
        &mixer->s_dib_buffer,
        NULL,
        0
        );

    hdc = GetDC( mixer->s_hWnd );
    mixer->s_hdc_memory = CreateCompatibleDC( hdc );
    if (!mixer->s_hdc_memory)
        return 0;
    ReleaseDC(mixer->s_hWnd, hdc);

    mixer->s_old_hobject = SelectObject(mixer->s_hdc_memory, mixer->s_dib_section);
    if (!mixer->s_old_hobject)
        return 0;

#ifdef DRAW_IN_WINDOW
    {
        WNDCLASSEX wcex;

        wcex.cbSize = sizeof(WNDCLASSEX); 
        wcex.style   = CS_HREDRAW | CS_VREDRAW;
        wcex.lpfnWndProc = WndProc;
        wcex.cbClsExtra  = 0;
        wcex.cbWndExtra  = 0;
        wcex.hInstance  = GetModuleHandle(NULL);
        wcex.hIcon   = LoadIcon(NULL, IDI_APPLICATION);
        wcex.hCursor  = LoadCursor(NULL, IDC_ARROW);
        wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
        wcex.lpszMenuName = NULL;
        wcex.lpszClassName = (LPCTSTR)"CNMMIXER";
        wcex.hIconSm  = LoadIcon(NULL, IDI_APPLICATION);

        RegisterClassEx(&wcex);

        mixer->s_hWnd = CreateWindow((LPCTSTR)"CNMMIXER", (LPCTSTR)"C&M YuvViewer", WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT, CW_USEDEFAULT, width, height, NULL, NULL, GetModuleHandle(NULL), NULL);
        MoveWindow(mixer->s_hWnd, coreIndex*width, 0, width+16, height+38, TRUE);
        mixer->wndShow = 0;
    }
#endif
#endif //#if defined(PLATFORM_WIN32) 

#if defined(PLATFORM_QNX)
#endif
    return 1;

}

#ifdef PLATFORM_WIN32
static Uint8* convert16bitTo8bit(
    Uint8* src, 
    Uint32 w, 
    Uint32 h, 
    FrameBufferFormat format
    )
{
    Uint8* dst;
    Uint8* pdst;
    Uint16* psrc = (Uint16*)src;
    Uint32 x, y, shift;

    if (format == FORMAT_420_P10_16BIT_MSB) 
        shift = 8;
    else 
        shift = 2;

    pdst = dst = (Uint8*)osal_malloc(w*h*3/2);
    /* Y */
    for (y=0; y<h; y++) {
        for (x=0; x<w; x++) {
            *dst++ = ((*psrc++)>>shift)&0xff;
        }
    }
    /* Cb */
    for (y=0; y<h/2; y++) {
        for (x=0; x<w/2; x++) {
            *dst++ = (*psrc++>>shift)&0xff;
        }
    }
    /* Cr */
    for (y=0; y<h/2; y++) {
        for (x=0; x<w/2; x++) {
            *dst++ = (*psrc++>>shift)&0xff;
        }
    }

    return pdst;
}
#endif /* PLATFORM_WIN32 */

static void PlatformDev_Close(
    Uint32 coreIndex
    )
{
#ifdef PLATFORM_LINUX
    FBDev *mixer = &s_mixer[coreIndex];
    if (mixer->s_scr_ptr)
    {
        munmap(mixer->s_scr_ptr, mixer->s_product);
        mixer->s_scr_ptr = NULL;
    }
    if (mixer->s_rgb_ptr)
    {
        osal_free(mixer->s_rgb_ptr);
        mixer->s_rgb_ptr = NULL;
    }

    if (mixer->s_fd)
    {
        close(mixer->s_fd);
        mixer->s_fd = 0;
    }
#endif

#if defined(PLATFORM_WIN32) 
    FBDev *mixer = &s_mixer[coreIndex];
    if (mixer->s_old_hobject)
    {
        SelectObject(mixer->s_hdc_memory,mixer->s_old_hobject);
    }
    if (mixer->s_hdc_memory)
    {
        DeleteDC(mixer->s_hdc_memory);
    }
    if (mixer->s_dib_section)
    {
        DeleteObject(mixer->s_dib_section);
    }

    if (mixer->s_hWnd)
    {
        DestroyWindow(mixer->s_hWnd);
    }

    osal_memset(mixer, 0x00, sizeof(FBDev));
#endif	//#if defined(PLATFORM_WIN32) 

#if defined(PLATFORM_QNX)
#endif
}

static int PlatformDev_Draw(
    Uint32      coreIndex, 
    Uint32      x, 
    Uint32      y, 
    Uint32      width, 
    Uint32      height, 
    FrameBufferFormat   format, 
    Uint8*      pbImage, 
    BOOL        interleave, 
    Uint32      bpp
    )
{

#ifdef PLATFORM_LINUX
    FBDev *mixer = &s_mixer[coreIndex];
    unsigned char* src_ptr = pbImage;
    YUV2RGBColorFmt colorFormat;

    if (mixer->s_fd < 0)
        return 0;

    colorFormat = ConvertToDeviceColorFormat(format, interleave);
    if (colorFormat == YUV2RGB_COLOR_FORMAT_MAX) {
        VLOG(ERR, "[ERROR]not supported image format\n");
        return 0;
    }

    if (mixer->s_fb_bpp == 8) {
        ConvertYuvToRgb(width, height, colorFormat, src_ptr,  mixer->s_rgb_ptr, 1);
    }
    else {
        return 0;
    }
    // Not implemented yet
#endif	//#ifdef PLATFORM_LINUX

#if defined(PLATFORM_WIN32) 
    FBDev *mixer = &s_mixer[coreIndex];
    HDC hdc_screen;
    YUV2RGBColorFmt color_format;
    Uint8* pYuv = NULL;

    if (mixer->winWidth != width || mixer->winHeight != height) {
        /* re-create window */
        PlatformDev_Close(coreIndex);
        PlatformDev_Open(coreIndex, width, height);
        mixer->winWidth  = width;
        mixer->winHeight = height;
    }

    if (!mixer->wndShow) {
        ShowWindow(mixer->s_hWnd, SW_SHOW);
        UpdateWindow(mixer->s_hWnd);
        //SetForegroundWindow(mixer->s_hWnd);
        mixer->wndShow = 1;
    }
    else
    {
        MSG msg={0};
        while(1)
        {
            if (PeekMessage(&msg, mixer->s_hWnd, 0, 0, PM_REMOVE))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
            else
                break;
        }		
    }

#ifdef DRAW_IN_WINDOW
    hdc_screen = GetDC( mixer->s_hWnd );
#else
    hdc_screen = GetDC( NULL );
#endif

    color_format  = ConvertToDeviceColorFormat(format, interleave);

    if (color_format == YUV2RGB_COLOR_FORMAT_MAX)
    {
        VLOG(ERR, "[ERROR]not supported image format\n");
        return 0;
    }

    if (bpp == 16)
        pYuv = convert16bitTo8bit(pbImage, width, height, format);
    else
        pYuv = pbImage;

    ConvertYuvToRgb(width, height, color_format, pYuv, (unsigned char *)mixer->s_dib_buffer, 1);

    if (bpp == 16) 
        osal_free(pYuv);

    if( hdc_screen ) {
#if 0
        RECT    rc;
        Int32   dispWidth;
        Int32   dispHeight;
        GetClientRect(mixer->s_hWnd, &rc);
        dispWidth = rc.right - rc.left;
        dispHeight = rc.bottom - rc.top;
        if (width != dispWidth || height != dispHeight)
            StretchBlt(hdc_screen, 0, 0, dispWidth, dispHeight, mixer->s_hdc_memory, x, y, width, height, SRCCOPY); 
        else
            BitBlt( hdc_screen, x, y, width, height, mixer->s_hdc_memory, 0,0, SRCCOPY );	
#else
        BitBlt( hdc_screen, x, y, width, height, mixer->s_hdc_memory, 0,0, SRCCOPY );    
#endif

        ReleaseDC(mixer->s_hWnd, hdc_screen);
    }

    //SetForegroundWindow(s_hWnd);
#endif	//#if defined(PLATFORM_WIN32) 

#if defined(PLATFORM_QNX)
#endif
    return 1;

}

#if defined(PLATFORM_WIN32) 
static LRESULT CALLBACK WndProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;
    switch (message)
    {
    case WM_CREATE:
        return (0);

    case WM_PAINT:
        BeginPaint (hwnd, &ps);

        EndPaint (hwnd, &ps);
        return (0);

    case WM_CLOSE:
        return (0);

    case WM_DESTROY:
        PostQuitMessage (0);
        return (0);
    }
    return DefWindowProc (hwnd, message, wParam, lParam);
}

#endif //defined(PLATFORM_WIN32) 

#if defined(PLATFORM_WIN32) || defined(PLATFORM_LINUX) || defined(PLATFORM_QNX)
// inteleave : 0 (chroma separate mode), 1 (cbcr interleave mode), 2 (crcb interleave mode)
static YUV2RGBColorFmt ConvertToDeviceColorFormat(
    int yuv_format, 
    int interleave
    ) 
{      
    YUV2RGBColorFmt format;

    switch(yuv_format)
    {
    case FORMAT_400: format = YUV400; break;
    case FORMAT_444: format = YUV444; break;
    case FORMAT_224:
    case FORMAT_422: format = YUV422; break;
    case FORMAT_420_P10_16BIT_LSB:
    case FORMAT_420_P10_16BIT_MSB:
    case FORMAT_420: 
        if (interleave == 0)
            format = YUV420; 
        else if (interleave == 1)
            format = NV12;				
        else
            format = NV21; 
        break;
    default:
        format = YUV2RGB_COLOR_FORMAT_MAX; 
    }

    return format;
}  
#endif // defined(PLATFORM_WIN32) || defined(PLATFORM_LINUX)

#if defined(PLATFORM_WIN32) || defined(PLATFORM_LINUX) || defined(PLATFORM_QNX)
static void ConvertYuvToRgb(
    Uint32          width, 
    Uint32          height, 
    YUV2RGBColorFmt format, 
    Uint8*          src, 
    Uint8*          rgba, 
    BOOL            cbcr_reverse
    )
{
#define CLIP(var) ((var>=255)?255:(var<=0)?0:var)
    Uint32   j, i;
    Int32    c, d, e;

    Uint8*   line = rgba;
    Uint8*   cur;
    Uint8*   y = NULL;
    Uint8*   u = NULL;
    Uint8*   v = NULL;
    Uint8*   misc = NULL;

    Uint32   frame_size_y;
    Uint32   frame_size_uv;
    Uint32   t_width;

    frame_size_y = width*height;

    if( format == YUV444 || format == RGB_PLANAR)
        frame_size_uv = width*height;
    else if( format == YUV422 )
        frame_size_uv = (width*height)>>1;
    else if( format == YUV420 || format == NV12 || format == NV21 )
        frame_size_uv = (width*height)>>2;
    else 
        frame_size_uv = 0;

    t_width = width;

    if( format == YUYV || format == YVYU  || format == UYVY  || format == VYUY ) {
        misc = src;
    }
    else if( format == NV12 || format == NV21) {	
        y = src;
        misc = src + frame_size_y;
    }
    else if( format == RGB32 || format == RGB24 || format == RGB16 ) {
        misc = src;
    }
    else {
        y = src;
        u = src + frame_size_y;
        v = src + frame_size_y + frame_size_uv;		
    }

    if( format == YUV444 ){

        for( j = 0 ; j < height ; j++ ){
            cur = line;
            for( i = 0 ; i < width ; i++ ){
                c = y[j*width+i] - 16;
                d = u[j*width+i] - 128;
                e = v[j*width+i] - 128;

                if (!cbcr_reverse) {
                    d = u[j*width+i] - 128;
                    e = v[j*width+i] - 128;
                } else {
                    e = u[j*width+i] - 128;
                    e = v[j*width+i] - 128;
                }
                (*cur) = (BYTE)CLIP(( 298 * c           + 409 * e + 128) >> 8);cur++;
                (*cur) = (BYTE)CLIP(( 298 * c - 100 * d - 208 * e + 128) >> 8);cur++;
                (*cur) = (BYTE)CLIP(( 298 * c + 516 * d           + 128) >> 8);cur++;
                (*cur) = 0; cur++;
            }
            line += t_width<<2;
        }
    }
    else if( format == YUV422){
        for( j = 0 ; j < height ; j++ ){
            cur = line;
            for( i = 0 ; i < width ; i++ ){
                c = y[j*width+i] - 16;
                d = u[j*(width>>1)+(i>>1)] - 128;
                e = v[j*(width>>1)+(i>>1)] - 128;

                if (!cbcr_reverse) {
                    d = u[j*(width>>1)+(i>>1)] - 128;
                    e = v[j*(width>>1)+(i>>1)] - 128;
                } else {
                    e = u[j*(width>>1)+(i>>1)] - 128;
                    d = v[j*(width>>1)+(i>>1)] - 128;
                }

                (*cur) = (BYTE)CLIP(( 298 * c           + 409 * e + 128) >> 8);cur++;
                (*cur) = (BYTE)CLIP(( 298 * c - 100 * d - 208 * e + 128) >> 8);cur++;
                (*cur) = (BYTE)CLIP(( 298 * c + 516 * d           + 128) >> 8);cur++;
                (*cur) = 0; cur++;
            }
            line += t_width<<2;
        }
    }
    else if( format == YUYV || format == YVYU  || format == UYVY  || format == VYUY )
    {
        unsigned char* t = misc;
        for( j = 0 ; j < height ; j++ ){
            cur = line;
            for( i = 0 ; i < width ; i+=2 ){
                switch( format) {
                case YUYV:
                    c = *(t  ) - 16;
                    if (!cbcr_reverse) {
                        d = *(t+1) - 128;
                        e = *(t+3) - 128;
                    } else {
                        e = *(t+1) - 128;
                        d = *(t+3) - 128;
                    }
                    break;
                case YVYU:
                    c = *(t  ) - 16;
                    if (!cbcr_reverse) {
                        d = *(t+3) - 128;
                        e = *(t+1) - 128;
                    } else {
                        e = *(t+3) - 128;
                        d = *(t+1) - 128;
                    }
                    break;
                case UYVY:
                    c = *(t+1) - 16;
                    if (!cbcr_reverse) {
                        d = *(t  ) - 128;
                        e = *(t+2) - 128;
                    } else {
                        e = *(t  ) - 128;
                        d = *(t+2) - 128;
                    }
                    break;
                case VYUY:
                    c = *(t+1) - 16;
                    if (!cbcr_reverse) {
                        d = *(t+2) - 128;
                        e = *(t  ) - 128;
                    } else {
                        e = *(t+2) - 128;
                        d = *(t  ) - 128;
                    }
                    break;
                default: // like YUYV
                    c = *(t  ) - 16;
                    if (!cbcr_reverse) {
                        d = *(t+1) - 128;
                        e = *(t+3) - 128;
                    } else {
                        e = *(t+1) - 128;
                        d = *(t+3) - 128;
                    }
                    break;
                }       

                (*cur) = (BYTE)CLIP(( 298 * c           + 409 * e + 128) >> 8);cur++;
                (*cur) = (BYTE)CLIP(( 298 * c - 100 * d - 208 * e + 128) >> 8);cur++;
                (*cur) = (BYTE)CLIP(( 298 * c + 516 * d           + 128) >> 8);cur++;
                (*cur) = 0;cur++;

                switch( format) {
                case YUYV:
                case YVYU:
                    c = *(t+2) - 16;
                    break;

                case VYUY:
                case UYVY:
                    c = *(t+3) - 16;
                    break;
                default: // like YUYV
                    c = *(t+2) - 16;
                    break;
                }

                (*cur) = (BYTE)CLIP(( 298 * c           + 409 * e + 128) >> 8);cur++;
                (*cur) = (BYTE)CLIP(( 298 * c - 100 * d - 208 * e + 128) >> 8);cur++;
                (*cur) = (BYTE)CLIP(( 298 * c + 516 * d           + 128) >> 8);cur++;
                (*cur) = 0; cur++;

                t += 4;
            }
            line += t_width<<2;
        }
    }
    else if( format == YUV420 || format == NV12 || format == NV21){
        for( j = 0 ; j < height ; j++ ){
            cur = line;
            for( i = 0 ; i < width ; i++ ){
                c = y[j*width+i] - 16;
                if (format == YUV420) {
                    if (!cbcr_reverse) {
                        d = u[(j>>1)*((width)>>1)+(i>>1)] - 128;
                        e = v[(j>>1)*((width)>>1)+(i>>1)] - 128;					
                    } else {
                        e = u[(j>>1)*((width)>>1)+(i>>1)] - 128;
                        d = v[(j>>1)*((width)>>1)+(i>>1)] - 128;	
                    }
                }
                else if (format == NV12) {
                    if (!cbcr_reverse) {
                        d = misc[(j>>1)*width+((i>>1)<<1)  ] - 128;
                        e = misc[(j>>1)*width+((i>>1)<<1)+1] - 128;					
                    } else {
                        e = misc[(j>>1)*width+((i>>1)<<1)  ] - 128;
                        d = misc[(j>>1)*width+((i>>1)<<1)+1] - 128;	
                    }
                }
                else { // if (m_color == NV21)
                    if (!cbcr_reverse) {
                        d = misc[(j>>1)*width+((i>>1)<<1)+1] - 128;
                        e = misc[(j>>1)*width+((i>>1)<<1)  ] - 128;					
                    } else {
                        e = misc[(j>>1)*width+((i>>1)<<1)+1] - 128;
                        d = misc[(j>>1)*width+((i>>1)<<1)  ] - 128;		
                    }
                }
                (*cur) = (BYTE)CLIP(( 298 * c           + 409 * e + 128) >> 8);cur++;
                (*cur) = (BYTE)CLIP(( 298 * c - 100 * d - 208 * e + 128) >> 8);cur++;
                (*cur) = (BYTE)CLIP(( 298 * c + 516 * d           + 128) >> 8);cur++;
                (*cur) = 0; cur++;
            }
            line += t_width<<2;
        }
    }
    else if( format == RGB_PLANAR ){
        for( j = 0 ; j < height ; j++ ){
            cur = line;
            for( i = 0 ; i < width ; i++ ){
                (*cur) = y[j*width+i];cur++;
                (*cur) = u[j*width+i];cur++;
                (*cur) = v[j*width+i];cur++;
                (*cur) = 0; cur++;
            }
            line += t_width<<2;
        }
    }
    else if( format == RGB32 ){
        for( j = 0 ; j < height ; j++ ){
            cur = line;
            for( i = 0 ; i < width ; i++ ){
                (*cur) = misc[j*width*4+i];cur++;	// R
                (*cur) = misc[j*width*4+i+1];cur++;	// G
                (*cur) = misc[j*width*4+i+2];cur++;	// B
                (*cur) = misc[j*width*4+i+3];cur++;	// A
            }
            line += t_width<<2;
        }
    }
    else if( format == RGB24 ){
        for( j = 0 ; j < height ; j++ ){
            cur = line;
            for( i = 0 ; i < width ; i++ ){
                (*cur) = misc[j*width*3+i];cur++;	// R
                (*cur) = misc[j*width*3+i+1];cur++;	// G
                (*cur) = misc[j*width*3+i+2];cur++;	// B
                (*cur) = 0; cur++;
            }
            line += t_width<<2;
        }
    }
    else if( format == RGB16 ){
        for( j = 0 ; j < height ; j++ ){
            cur = line;
            for( i = 0 ; i < width ; i++ ){
                int tmp = misc[j*width*2+i]<<8 | misc[j*width*2+i+1];
                (*cur) = (BYTE)((tmp>>11)&0x1F<<3);cur++; // R(5bit)
                (*cur) = (BYTE)((tmp>>5 )&0x3F<<2);cur++; // G(6bit)
                (*cur) = (BYTE)((tmp    )&0x1F<<3);cur++; // B(5bit)
                (*cur) = 0; cur++;
            }
            line += t_width<<2;
        }
    }
    else { // YYY
        for( j = 0 ; j < height ; j++ ){
            cur = line;
            for( i = 0 ; i < width ; i++ ){
                (*cur) = (BYTE)y[j*width+i]; cur++;
                (*cur) = (BYTE)y[j*width+i]; cur++;
                (*cur) = (BYTE)y[j*width+i]; cur++;
                (*cur) = 0; cur++;
            }
            line += t_width<<2;
        }	
    }

}
#endif //defined(PLATFORM_WIN32) || defined(PLATFORM_LINUX)

BOOL FBDisplayDevice_Open(
    RenderDevice*   device
    )
{
    Uint32      coreIndex;
    DecHandle   decHandle = device->decHandle;

    VLOG(INFO, "%s\n", __FUNCTION__);
    coreIndex = VPU_HANDLE_CORE_INDEX(decHandle);
    PlatformDev_Open((coreIndex+1)*(device->decHandle->instIndex), 0, 0);

    return TRUE;
}

void FBDisplayDevice_Render(
    RenderDevice*   device,
    DecOutputInfo*  fbInfo,     //!<< fbInfo is not used
    Uint8*          pYuv,
    Uint32          width,
    Uint32          height
    )
{
    Uint32          coreIndex = VPU_HANDLE_CORE_INDEX(device->decHandle);
    Uint32          Bpp = 1;                                            //!<< Byte per pixel
    FrameBuffer*    fbSrc = &fbInfo->dispFrame;

    UNREFERENCED_PARAMETER(fbInfo);

    switch (fbSrc->format) {
    case FORMAT_420_P10_16BIT_LSB:
    case FORMAT_420_P10_16BIT_MSB:
    case FORMAT_420_P10_32BIT_LSB:
    case FORMAT_420_P10_32BIT_MSB:
        Bpp = 2;
        break;
    default:
        Bpp = 1;
    }
    PlatformDev_Draw((coreIndex+1)*(device->decHandle->instIndex), 0, 0, width, height, fbSrc->format, pYuv, FALSE, Bpp*8);
}

BOOL FBDisplayDevice_Close(
    RenderDevice*   device
    )
{
    DecHandle   decHandle = device->decHandle;
    Uint32      coreIndex = VPU_HANDLE_CORE_INDEX(decHandle);

    PlatformDev_Close((coreIndex+1)*(device->decHandle->instIndex));

    return TRUE;
}
 
