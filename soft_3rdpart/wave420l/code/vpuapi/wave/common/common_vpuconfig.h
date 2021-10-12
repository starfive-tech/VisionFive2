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
//		This file should be modified by some customers according to their SOC configuration.
//--=========================================================================--

#ifndef __COMMON_VPU_CONFIGURATION_H__
#define __COMMON_VPU_CONFIGURATION_H__

#include "vpuconfig.h"

#define WAVE4_MAX_CODE_BUF_SIZE         (1024*1024)
#define WAVE5_MAX_CODE_BUF_SIZE         (1024*1024)

#define WAVE4DEC_WORKBUF_SIZE           (3*1024*1024)    /* 2K(system) + 463K(codec) + 1850K(prescan, level 6.x) */
#define WAVE412DEC_WORKBUF_SIZE         (5*1024*1024)    /* 2K(system) + 2M(codec) + 1850K(prescan, level 6.x) */
#define WAVE512DEC_WORKBUF_SIZE         (2*1024*1024)   /* This code will be removed till merging with WAVE510 F/W */
#define WAVE510DEC_WORKBUF_SIZE         (512*1024)

#define WAVE4ENC_WORKBUF_SIZE           (128*1024)

#define CODA7Q_VPU_GMC_PROCESS_METHOD   0
#define CODA7Q_VPU_AVC_X264_SUPPORT     1
//----- slice save buffer --------------------//
#define CODA7Q_SLICE_SAVE_SIZE          (MAX_DEC_PIC_WIDTH*MAX_DEC_PIC_HEIGHT*3/4)          // this buffer for ASO/FMO

#define DEFAULT_TEMPBUF_SIZE                1024*1024
#define WAVE4_TEMPBUF_OFFSET                1024*1024 //(codeSize)
#endif /* __COMMON_VPU_CONFIGURATION_H__ */

