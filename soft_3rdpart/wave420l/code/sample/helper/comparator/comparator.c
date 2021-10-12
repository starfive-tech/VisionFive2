//--=========================================================================--
//  This file is a part of VPU Reference API project
//-----------------------------------------------------------------------------
//
//       This confidential and proprietary software may be used only
//     as authorized by a licensing agreement from Chips&Media Inc.
//     In the event of publication, the following notice is applicable:
//
//            (C) COPYRIGHT CHIPS&MEDIA INC.
//                      ALL RIGHTS RESERVED
//
//       The entire notice above must be reproduced on all authorized
//       copies.
//
//--=========================================================================--

#include <stdarg.h>
#include "main_helper.h"
#include "../misc/skip.h"


extern ComparatorImpl binComparatorImpl;
extern ComparatorImpl yuvComparatorImpl;

static BOOL NullComparator_Create(
    ComparatorImpl* impl,
    char*           path
    )
{
    UNREFERENCED_PARAMETER(impl);
    UNREFERENCED_PARAMETER(path);
    return TRUE;
}

static BOOL NullComparator_Destroy(
    ComparatorImpl*  impl
    )
{
    UNREFERENCED_PARAMETER(impl);

    return TRUE;
}

static BOOL NullComparator_Compare(
    ComparatorImpl* impl,
    void*           data,
    Uint32          size
    )
{
    UNREFERENCED_PARAMETER(impl);
    UNREFERENCED_PARAMETER(data);
    UNREFERENCED_PARAMETER(size);
    return TRUE;
}

static BOOL NullComparator_Configure(
    ComparatorImpl*     impl,
    ComparatorConfType  cmd,
    void*               val
    )
{
    UNREFERENCED_PARAMETER(impl);
    UNREFERENCED_PARAMETER(cmd);
    UNREFERENCED_PARAMETER(val);

    return TRUE;
}

static BOOL NullComparator_Rewind(
    ComparatorImpl*     impl
    )
{
    UNREFERENCED_PARAMETER(impl);
    return TRUE;
}

ComparatorImpl nullComparatorImpl = {
    NULL,
    NULL,
    0, 
    0,
    NullComparator_Create,
    NullComparator_Destroy,
    NullComparator_Compare,
    NullComparator_Configure,
    NullComparator_Rewind
};

BOOL BinComparator_Create(
    ComparatorImpl* impl,
    char*           path
);

BOOL BinComparator_Destroy(
    ComparatorImpl*  impl
);

BOOL BinComparator_Compare(
    ComparatorImpl* impl,
    void*           data,
    Uint32          size
);

BOOL BinComparator_Configure(
    ComparatorImpl*     impl,
    ComparatorConfType  type,
    void*               val
);

BOOL YUVComparator_Create(
    ComparatorImpl* impl,
    char*           path
);

BOOL YUVComparator_Destroy(
    ComparatorImpl*  impl
);

BOOL YUVComparator_Compare(
    ComparatorImpl* impl,
    void*           data,
    Uint32          size
);

BOOL YUVComparator_Configure(
    ComparatorImpl*    impl,
    ComparatorConfType cmd,
    void*              val
);

BOOL YUVComparator_Rewind(
    ComparatorImpl*     impl
    );

Comparator Comparator_Create(
    Uint32      type,
    char*       goldenPath,
    ...
    )
{
    /*lint -esym(438, ap) */
    AbstractComparator* comp;
    ComparatorImpl* impl = NULL;
    va_list         ap;
    BOOL            success = FALSE;

    if (type != NO_COMPARE && goldenPath == NULL) {
        VLOG(ERR, "%s:%d golden path is NULL\n", __FUNCTION__, __LINE__);
        return NULL;
    }

    switch (type) {
    case NO_COMPARE:
        impl = (ComparatorImpl*)osal_malloc(sizeof(ComparatorImpl));
        osal_memset((void*)impl, 0x00, sizeof(ComparatorImpl));
        impl->Create    = NullComparator_Create;
        impl->Compare   = NullComparator_Compare;
        impl->Destroy   = NullComparator_Destroy;
        impl->Configure = NullComparator_Configure;
        impl->Rewind    = NullComparator_Rewind;
        success = impl->Create(impl, goldenPath);
        break;
    case YUV_COMPARE:
        impl = (ComparatorImpl*)osal_malloc(sizeof(ComparatorImpl));
        osal_memset((void*)impl, 0x00, sizeof(ComparatorImpl));
        impl->Create    = YUVComparator_Create;
        impl->Compare   = YUVComparator_Compare;
        impl->Destroy   = YUVComparator_Destroy;
        impl->Configure = YUVComparator_Configure;
        impl->Rewind    = YUVComparator_Rewind;
        if ((success=impl->Create(impl, goldenPath)) == TRUE) {
            PictureInfo picInfo;
            va_start(ap, goldenPath);
            picInfo.width  = va_arg(ap, Uint32);
            picInfo.height = va_arg(ap, Uint32);
            picInfo.format = (FrameBufferFormat)va_arg(ap, Uint32);
            picInfo.cbcrInterleave = va_arg(ap, BOOL);
            picInfo.isVp9  = va_arg(ap, BOOL);
            va_end(ap);
            impl->Configure(impl, COMPARATOR_CONF_SET_PICINFO, (void*)&picInfo);
        }
        break;
    case STREAM_COMPARE:
        impl = osal_malloc(sizeof(ComparatorImpl));
        osal_memset((void*)impl, 0x00, sizeof(ComparatorImpl));
        impl->Create = &BinComparator_Create;
        impl->Compare = &BinComparator_Compare;
        impl->Destroy = &BinComparator_Destroy;
        impl->Configure = &BinComparator_Configure;
        va_start(ap, goldenPath);
        impl->filename = va_arg(ap, char*);
        va_end(ap);
        success = impl->Create(impl, goldenPath);
        break;
    default:
        VLOG(ERR, "Invalid comparison type:%d\n", type);
        return NULL;
    }

    if (success == FALSE) 
        return NULL;

    comp = (AbstractComparator*)osal_malloc(sizeof(AbstractComparator));
    impl->curIndex      = 0;
    comp->impl          = impl;
    comp->totalFrames   = impl->numOfFrames;

    return comp;
    /*lint +esym(438, ap) */
}

BOOL Comparator_Destroy(
    Comparator  comp
    )
{
    ComparatorImpl*     impl = NULL;
    AbstractComparator* absComp = (AbstractComparator*)comp;

    if (comp == NULL) {
        VLOG(ERR, "%s:%d Invalid handle\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    impl = absComp->impl;

    impl->Destroy(impl);

    osal_free(impl);

    return TRUE;
}

BOOL Comparator_Act(
    Comparator  comp,
    void*       data,
    Uint32      size
    )
{
    ComparatorImpl*     impl = NULL;
    AbstractComparator* absComp = (AbstractComparator*)comp;

    if (comp == NULL) {
        VLOG(ERR, "%s:%d Invalid handle\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    impl = absComp->impl;
    impl->curIndex++;
    if (impl->usePrevDataOneTime == TRUE)
        impl->curIndex--;

    return impl->Compare(impl, data, size);
}

BOOL Comparator_CheckFrameCount(
    Comparator  comp
    )
{
    ComparatorImpl*     impl = NULL;
    AbstractComparator* absComp = (AbstractComparator*)comp;
    BOOL                match = TRUE;

    if (comp == NULL) {
        VLOG(ERR, "%s:%d Invalid handle\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    impl = absComp->impl;

    if (impl->curIndex != absComp->totalFrames) {
        VLOG(ERR, "MISMATCH FRAME COUNT: GOLDEN(%d) DECODED(%d)\n", 
            impl->numOfFrames, impl->curIndex);
        match = FALSE;
    }

    return match;
}

/* \brief       When scan mode is enable, Comparator_Act() tries to find data matched with decoded result 
 *              by scanning all data
 */
BOOL Comparator_SetScanMode(
    Comparator  comp,
    BOOL        enable
    )
{
    AbstractComparator* absComp = (AbstractComparator*)comp;
    ComparatorImpl*     impl = NULL;

    if (absComp == NULL) {
        return FALSE;
    }
    impl = absComp->impl;

    impl->enableScanMode = enable;

    return TRUE;
}

BOOL Comparator_Rewind(
    Comparator  comp
    )
{
    AbstractComparator* absComp = (AbstractComparator*)comp;
    ComparatorImpl*     impl = NULL;

    if (absComp == NULL) {
        return FALSE;
    }
    impl                  = absComp->impl;
    absComp->totalFrames += impl->numOfFrames;

    return impl->Rewind(impl);
}

BOOL Comparator_CheckEOF(
    Comparator  comp
    )
{
    ComparatorImpl*     impl     = NULL;
    AbstractComparator* absComp  = (AbstractComparator*)comp;
    BOOL                match    = TRUE;

    if (comp == NULL) {
        VLOG(ERR, "%s:%d Invalid handle\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    impl = absComp->impl;

    if (impl->eof == FALSE) {
        VLOG(ERR, "It is not the end of file.\n");
        match = FALSE;
    }

    return match;
}

 
