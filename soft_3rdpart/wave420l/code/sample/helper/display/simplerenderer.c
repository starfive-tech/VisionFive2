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

#include "vpuapifunc.h"
#include <stdint.h>
#include <string.h>
#include "main_helper.h"

typedef struct {
    DecHandle       decHandle;      //!<< decoder handle
    Uint32          period;         //!<< period of displaying picture in ms 
    VpuThread       threadHandle;
    BOOL            isRunning;
    BOOL            stop;
    BOOL            hasPrevSequence;
    Queue*          Q;
    Queue*          freeQ;
    VpuMutex        lock;
    RenderDevice*   device;
    Uint32          numOfFrames;
} RendererContext;

typedef struct {
    DecOutputInfo   fbInfo;
    Uint8*          pYuv;
    Uint32          displayWidth;
    Uint32          displayHeight;
} DisplayInfo;

static BOOL NullDisplayDevice_Open(
    RenderDevice*   device
    )
{
    UNREFERENCED_PARAMETER(device);

    return TRUE;
}

static void NullDisplayDevice_Render(
    RenderDevice*   device,
    DecOutputInfo*  fbInfo,
    Uint8*          pYuv,
    Uint32          width,
    Uint32          height
    )
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(fbInfo);
    UNREFERENCED_PARAMETER(pYuv);
    UNREFERENCED_PARAMETER(width);
    UNREFERENCED_PARAMETER(height);
}

static BOOL NullDisplayDevice_Close(
    RenderDevice*   device
    )
{
    UNREFERENCED_PARAMETER(device);

    return TRUE;
}

BOOL FBDisplayDevice_Close(
    RenderDevice*   device
);

void FBDisplayDevice_Render(
    RenderDevice*   device,
    DecOutputInfo*  fbInfo,     //!<< fbInfo is not used
    Uint8*          pYuv,
    Uint32          width,
    Uint32          height
);

BOOL FBDisplayDevice_Open(
    RenderDevice*   device
);

BOOL HDMIDisplayDevice_Close(
    RenderDevice*   device
);

void HDMIDisplayDevice_Render(
    RenderDevice*   device,
    DecOutputInfo*  fbInfo,     //!<< fbInfo is not used
    Uint8*          pYuv,
    Uint32          width,
    Uint32          height
);

BOOL HDMIDisplayDevice_Open(
    RenderDevice*   device
);

static RenderDevice* CreateRenderDevice(
    DecHandle           decHandle,
    RenderDeviceType    type
    )
{
    RenderDevice* device = NULL;

    switch (type) {
    case RENDER_DEVICE_FBDEV:
        device = osal_malloc(sizeof(RenderDevice));
        device->context = NULL;
        device->Open    = &FBDisplayDevice_Open;
        device->Render  = &FBDisplayDevice_Render;
        device->Close   = &FBDisplayDevice_Close;
        break;
    case RENDER_DEVICE_HDMI:
        device = osal_malloc(sizeof(RenderDevice));
        device->context = NULL;
        device->Open    = &HDMIDisplayDevice_Open;
        device->Render  = &HDMIDisplayDevice_Render;
        device->Close   = &HDMIDisplayDevice_Close;
        break;
    default:
        device = osal_malloc(sizeof(RenderDevice));
        device->context = NULL;
        device->Open    = &NullDisplayDevice_Open;
        device->Render  = &NullDisplayDevice_Render;
        device->Close   = &NullDisplayDevice_Close;
        break;
    }

    device->decHandle = decHandle;

    return device;
}

static void RenderPicture(
    void* arg
    )
{
    RendererContext* ctx = (RendererContext*)arg;
    RenderDevice*    device;
    DisplayInfo*     temp;
    DisplayInfo      displayInfo;
    DecOutputInfo*   fbInfo = NULL; 
    Uint8*           pYuv  ; 
    Uint32           width ; 
    Uint32           height; 

    device = ctx->device;

    do {
        ctx->isRunning = TRUE;
        MSleep(ctx->period);

        VpuMutex_Lock(ctx->lock);
        temp = (DisplayInfo*)Queue_Dequeue(ctx->Q);

        if (temp != NULL) {
            osal_memcpy((void*)&displayInfo, temp, sizeof(DisplayInfo));
            fbInfo = &displayInfo.fbInfo;
            pYuv   = displayInfo.pYuv;
            width  = displayInfo.displayWidth;
            height = displayInfo.displayHeight;
            device->Render(device, fbInfo, pYuv, width, height);
            Queue_Enqueue(ctx->freeQ, (void*)&displayInfo.fbInfo);
            if (displayInfo.pYuv) osal_free(displayInfo.pYuv);
        }
        VpuMutex_Unlock(ctx->lock);
    } while (ctx->stop == FALSE);

    VpuMutex_Lock(ctx->lock);
    while ((temp=(DisplayInfo*)Queue_Dequeue(ctx->Q)) != NULL) {
        fbInfo = &temp->fbInfo;
        pYuv   = temp->pYuv;
        width  = temp->displayWidth;
        height = temp->displayHeight;
        device->Render(device, fbInfo, pYuv, width, height);
        if (temp->pYuv) osal_free(temp->pYuv);
    }
    VpuMutex_Unlock(ctx->lock);
}

Renderer SimpleRenderer_Create(
    DecHandle           decHandle,
    RenderDeviceType    deviceType,
    const char*         yuvPath
    )
{
    RendererContext* ctx = (RendererContext*)osal_malloc(sizeof(RendererContext));
#ifdef PLATFORM_NON_OS
#else
    VpuThreadRunner     runner = (VpuThreadRunner)RenderPicture;
#endif

    osal_memset((void*)ctx, 0x00, sizeof(RendererContext));

    ctx->decHandle   = decHandle;
#ifdef PLATFORM_NON_OS
    ctx->period      = 0;        // 33 ms, NTSC
    ctx->stop        = TRUE;
#else
    ctx->period      = 33;        // 33 ms, NTSC
    ctx->stop        = FALSE;
#endif
    ctx->isRunning   = FALSE;
    ctx->numOfFrames = 0;
    if ((ctx->Q=Queue_Create(32, sizeof(DisplayInfo))) == NULL) 
        goto __FAILURE_RETURN;

    if ((ctx->freeQ=Queue_Create(32, sizeof(DecOutputInfo))) == NULL) 
        goto __FAILURE_RETURN;

    if ((ctx->lock=VpuMutex_Create()) == NULL) 
        goto __FAILURE_RETURN;

    if ((ctx->device=CreateRenderDevice(decHandle, deviceType)) == NULL) 
        goto __FAILURE_RETURN;

    ctx->device->Open(ctx->device);
    ctx->hasPrevSequence = FALSE;

#ifdef PLATFORM_NON_OS
    ctx->threadHandle = NULL;
#else
    ctx->threadHandle = VpuThread_Create(runner, (void*)ctx);
    if (ctx->threadHandle == NULL) {
        ctx->device->Close(ctx->device);
        osal_free(ctx->device);
        goto __FAILURE_RETURN;
    }
#endif

    return (Renderer)ctx;

__FAILURE_RETURN:
    if (ctx->Q) Queue_Destroy(ctx->Q);
    if (ctx->freeQ) Queue_Destroy(ctx->freeQ);
    if (ctx->lock)  VpuMutex_Destroy(ctx->lock);
    osal_free(ctx);

    return NULL;
}

Uint32 SimpleRenderer_Act(
    Renderer        renderer, 
    DecOutputInfo*  fbInfo,
    Uint8*          pYuv,
    Uint32          width,
    Uint32          height
    )
{
    RendererContext* ctx = (RendererContext*)renderer;
    DisplayInfo      displayInfo;

    if (renderer == NULL) {
        VLOG(ERR, "%s:%d Invalid handle\n", __FUNCTION__, __LINE__);
        return 0;
    }

    ctx->numOfFrames++;

    VpuMutex_Lock(ctx->lock);
    osal_memset((void*)&displayInfo, 0x00, sizeof(DisplayInfo));
    osal_memcpy((void*)&displayInfo.fbInfo, fbInfo, sizeof(DecOutputInfo));
    displayInfo.pYuv          = pYuv;
    displayInfo.displayWidth  = width;
    displayInfo.displayHeight = height;
    Queue_Enqueue(ctx->Q, (void*)&displayInfo);
    VpuMutex_Unlock(ctx->lock);
#ifdef PLATFORM_NON_OS
    RenderPicture((void*)ctx);
#endif

    return ctx->numOfFrames; 
}

void SimpleRenderer_Flush(
    Renderer        renderer
    )
{
    RendererContext* ctx = (RendererContext*)renderer;
    DisplayInfo*     temp = NULL;

    VpuMutex_Lock(ctx->lock);

    while ((temp=(DisplayInfo*)Queue_Dequeue(ctx->Q)) != NULL) {
        Queue_Enqueue(ctx->freeQ, (void*)&temp->fbInfo);
        if (temp->pYuv) osal_free(temp->pYuv);
    } 

    for (temp=NULL; temp!=NULL; temp=(DisplayInfo*)Queue_Dequeue(ctx->freeQ)) {
        VPU_DecClrDispFlag(ctx->decHandle, temp->fbInfo.indexFrameDisplay);
    }
    VpuMutex_Unlock(ctx->lock);
}

void* SimpleRenderer_GetFreeFrameInfo(
    Renderer        renderer
    )
{
    RendererContext* ctx = (RendererContext*)renderer;
    void* data = NULL;

    if (renderer == NULL) {
        VLOG(ERR, "%s:%d Invalid handle\n", __FUNCTION__, __LINE__);
        return NULL;
    }

    VpuMutex_Lock(ctx->lock);
    data = Queue_Dequeue(ctx->freeQ);
    VpuMutex_Unlock(ctx->lock);

    return data;
}

BOOL SimpleRenderer_Destroy(
    Renderer        renderer
    )
{
    RendererContext* ctx = (RendererContext*)renderer;

    if (renderer == NULL) {
        VLOG(ERR, "%s:%d Invalid handle\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    ctx->stop = TRUE;

    if (ctx->threadHandle) {
        VpuThread_Join(ctx->threadHandle);
    }
	
    VpuMutex_Destroy(ctx->lock);
    Queue_Destroy(ctx->Q);
    Queue_Destroy(ctx->freeQ);
    ctx->device->Close(ctx->device);

    osal_free(ctx->device);
    osal_free(ctx);

    return TRUE;
}

BOOL SimpleRenderer_SetFrameRate(
    Renderer        renderer,
    Uint32          fps
    )
{
    RendererContext* ctx = (RendererContext*)renderer;

    if (renderer == NULL) {
        VLOG(ERR, "%s:%d Invalid handle\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    if (fps == 0) {
        ctx->period = 0;
    }
    else {
        ctx->period = (Uint32)(1000/fps);
    }

    return TRUE;
}
 
