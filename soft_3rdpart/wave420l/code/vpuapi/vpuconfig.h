//--=========================================================================--
//  This file is a part of VPU Reference API project
//-----------------------------------------------------------------------------
//
//       This confidential and proprietary software may be used only
//     as authorized by a licensing agreement from Chips&Media Inc.
//     In the event of publication, the following notice is applicable:
//
//            (C) COPYRIGHT 2006 - 2011  CHIPS&MEDIA INC.
//                      ALL RIGHTS RESERVED
//
//       The entire notice above must be reproduced on all authorized
//       copies.
//		This file should be modified by some customers according to their SOC configuration.
//--=========================================================================--

#ifndef _VPU_CONFIG_H_
#define _VPU_CONFIG_H_
#include "../config.h"


#define BODA7503_CODE                   0x7503
#define CODA7542_CODE                   0x7542
#define BODA950_CODE                    0x9500
#define CODA960_CODE                    0x9600
#define CODA980_CODE                    0x9800
#define WAVE320_CODE                    0x3200
#define WAVE410_CODE                    0x4100          /* Wave410 version 1: Single vcore */
#define WAVE4102_CODE                   0x4102          /* Wave410 version 2: Multi vcore */
#define WAVE420_CODE                    0x4200          /* Wave420   */ 
#define WAVE420L_CODE                   0x4201          /* Wave420L  */ 
#define WAVE412_CODE                    0x4120
#define WAVE510_CODE                    0x5100
#define WAVE512_CODE                    0x5120
#define WAVE515_CODE                    0x5150
#define WAVE520_CODE                    0x5200
#define CODA7Q_CODE                     0x7101

#define PRODUCT_CODE_W_SERIES(x) (x == WAVE410_CODE || x == WAVE4102_CODE || x == WAVE420_CODE || x == WAVE412_CODE || x == CODA7Q_CODE || x == WAVE420L_CODE || x == WAVE510_CODE || x == WAVE512_CODE || x == WAVE515_CODE || x == WAVE520_CODE)
#define PRODUCT_CODE_NOT_W_SERIES(x) (x == BODA7503_CODE || x == CODA7542_CODE || x == BODA950_CODE || x == CODA960_CODE || x == CODA980_CODE || x == WAVE320_CODE)

#define MAX_INST_HANDLE_SIZE            48              /* DO NOT CHANGE THIS VALUE */
#define MAX_NUM_INSTANCE                4
#define MAX_NUM_VPU_CORE                1
#define MAX_NUM_VCORE                   1

    #define MAX_ENC_AVC_PIC_WIDTH           4096
    #define MAX_ENC_AVC_PIC_HEIGHT          2304  
#define MAX_ENC_PIC_WIDTH               4096
#define MAX_ENC_PIC_HEIGHT              2304
#define MIN_ENC_PIC_WIDTH               96
#define MIN_ENC_PIC_HEIGHT              16

// for WAVE420
#define W4_MIN_ENC_PIC_WIDTH            256
#define W4_MIN_ENC_PIC_HEIGHT           128
#define W4_MAX_ENC_PIC_WIDTH            8192
#define W4_MAX_ENC_PIC_HEIGHT           8192

#define MAX_DEC_PIC_WIDTH               4096
#define MAX_DEC_PIC_HEIGHT              2304

//  Application specific configuration
#ifdef HAPS_SIM
#define VPU_ENC_TIMEOUT                 (30000*10000)
#else
#define VPU_ENC_TIMEOUT                 30000
#endif

#define VPU_DEC_TIMEOUT                 10000

#ifdef HAPS_SIM
#define VPU_BUSY_CHECK_TIMEOUT          (5000*10000)
#else
#define VPU_BUSY_CHECK_TIMEOUT          5000
#endif

// codec specific configuration
#define VPU_REORDER_ENABLE              1   // it can be set to 1 to handle reordering DPB in host side.
#define CBCR_INTERLEAVE			        1 //[default 1 for BW checking with CnMViedo Conformance] 0 (chroma separate mode), 1 (chroma interleave mode) // if the type of tiledmap uses the kind of MB_RASTER_MAP. must set to enable CBCR_INTERLEAVE
#define VPU_ENABLE_BWB			        1 

#define HOST_ENDIAN                     VDI_128BIT_LITTLE_ENDIAN
#define VPU_FRAME_ENDIAN                HOST_ENDIAN
#define VPU_STREAM_ENDIAN               HOST_ENDIAN
#define VPU_USER_DATA_ENDIAN            HOST_ENDIAN
#define VPU_SOURCE_ENDIAN               HOST_ENDIAN
#define DRAM_BUS_WIDTH                  16

/************************************************************************/
/* VPU COMMON MEMORY                                                    */
/************************************************************************/
#define COMMAND_QUEUE_DEPTH             4
#define ONE_TASKBUF_SIZE_FOR_W5DEC_CQ         (8*1024*1024)   /* upto 8Kx4K, need 8Mbyte per task*/
#define ONE_TASKBUF_SIZE_FOR_W5ENC_CQ         (16*1024*1024)  /* upto 8Kx8K, need 16Mbyte per task. (worst case VLC data =: 13M for 8kx8k) */

#define ONE_TASKBUF_SIZE_FOR_CQ         0x0
#define SIZE_COMMON                     (2*1024*1024)

//=====4. VPU REPORT MEMORY  ======================//
#define SIZE_REPORT_BUF                 (0x10000)

#define STREAM_END_SIZE                 0
#define STREAM_END_SET_FLAG             0
#define STREAM_END_CLEAR_FLAG           -1
#define EXPLICIT_END_SET_FLAG           -2

#define USE_BIT_INTERNAL_BUF            0
#define USE_IP_INTERNAL_BUF             0
#define USE_DBKY_INTERNAL_BUF           0
#define USE_DBKC_INTERNAL_BUF           0
#define USE_OVL_INTERNAL_BUF            0
#define USE_BTP_INTERNAL_BUF            0
#define USE_ME_INTERNAL_BUF             0

/* WAVE410 only */
#define USE_BPU_INTERNAL_BUF            0
#define USE_VCE_IP_INTERNAL_BUF         0
#define USE_VCE_LF_ROW_INTERNAL_BUF     0

/* WAVE420 only */
#define USE_IMD_INTERNAL_BUF            0
#define USE_RDO_INTERNAL_BUF            0
#define USE_LF_INTERNAL_BUF             0


#endif  /* _VPU_CONFIG_H_ */

