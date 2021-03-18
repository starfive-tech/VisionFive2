/*
 * Copyright (c) 2019, Chips&Media
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef __CODA9_VPU_CONFIG_H__
#define __CODA9_VPU_CONFIG_H__

#include "vpuconfig.h"
#include "vputypes.h"


#if MAX_DEC_PIC_WIDTH > 1920		// 4K		

#define USE_WTL          0          // not support 4K WTL
#define MAX_EXTRA_FRAME_BUFFER_NUM      1
#define MAX_PP_SRC_NUM                  2   //PPU buffer(decoding case) or Source buffer(encoding case)

#define MAX_DPB_MBS      110400     // 32768 for level 5

#elif MAX_DEC_PIC_WIDTH > 720

#define USE_WTL         0           // for WTL (needs MAX_DPB_NUM*2 frame)
#define MAX_EXTRA_FRAME_BUFFER_NUM      4
#define MAX_PP_SRC_NUM                  2   //PPU buffer(decoding case) or Source buffer(encoding case)

#define MAX_DPB_MBS      32768      // 32768 for level 4.1

#else

#define USE_WTL          0          // for WTL (needs MAX_DPB_NUM*2 frame)
#define MAX_EXTRA_FRAME_BUFFER_NUM      4
#define MAX_PP_SRC_NUM                  2   //PPU buffer(decoding case) or Source buffer(encoding case)

#define MAX_DPB_MBS      32768      // 32768 for level 4.1

#endif

// codec specific configuration
#define VPU_REORDER_ENABLE  1   // it can be set to 1 to handle reordering DPB in host side.
#define VPU_GMC_PROCESS_METHOD  0
#define VPU_AVC_X264_SUPPORT 1

// DRAM configuration for TileMap access
#define EM_RAS 13
#define EM_BANK  3
#define EM_CAS   10
#define EM_WIDTH 2

// DDR3 3BA, DDR4 1BG+2BA
//#define EM_RAS   16
//#define EM_BANK  3
//#define EM_CAS   10
//#define EM_WIDTH 3

// DDR3 4BA, DDR4 2BG+2BA
//#define EM_RAS   15
//#define EM_BANK  4
//#define EM_CAS   10
//#define EM_WIDTH 3

#define VPU_ME_LINEBUFFER_MODE          0

#define VPU_GBU_SIZE                    1024

//********************************************//
//      External Memory Map Table
//********************************************//
#   define CODE_BUF_SIZE                (272*1024)
#   define TEMP_BUF_SIZE                (470*1024)
#   define WORK_BUF_SIZE                (80*1024) 
#define PARA_BUF_SIZE                   (10*1024)

//=====2. VPU BITSTREAM MEMORY ===================//
#define MAX_STREAM_BUF_SIZE             0x300000  // max bitstream size

//===== 3. VPU PPS Save Size ===================//
//----- SPS/PPS save buffer --------------------//
#define PS_SAVE_SIZE                    ((320+8)*1024*4) // sps[64], pps[256], backupPsBuf[8] for rollback, error concealment (each 4Kbyte)
//----- VP8 MB save buffer -------------------//
#define VP8_MB_SAVE_SIZE                (17*4*(MAX_DEC_PIC_WIDTH*MAX_DEC_PIC_HEIGHT/256))    // MB information + split MVs)*4*MbNumbyte
//----- slice save buffer --------------------//
#define SLICE_SAVE_SIZE                 (MAX_DEC_PIC_WIDTH*MAX_DEC_PIC_HEIGHT*3/4)          // this buffer for ASO/FMO

//=====5. VPU Encoder Scratch buffer ssize ======================//
#define SIZE_AVCENC_QMAT_TABLE          ((16*6)+(64*2)) // 4x4 6, 8x8 2
#define SIZE_MP4ENC_DATA_PARTITION      (MAX_ENC_PIC_WIDTH*MAX_ENC_PIC_HEIGHT*3/4)

//=====6. Check VPU required memory size =======================//
#define MAX_FRM_SIZE                    ((MAX_DEC_PIC_WIDTH*MAX_DEC_PIC_HEIGHT*3)/2)
#define MAX_DEC_FRAME_BUFFERING         (MAX_DPB_MBS/((MAX_DEC_PIC_WIDTH*MAX_DEC_PIC_HEIGHT)/256))
#define MAX_DPB_NUM                     (16 < MAX_DEC_FRAME_BUFFERING ? (16+2+MAX_EXTRA_FRAME_BUFFER_NUM) : (MAX_DEC_FRAME_BUFFERING+2+MAX_EXTRA_FRAME_BUFFER_NUM)) //  (+2 for current and display_delay)
#define MAX_DPB_SIZE                    (((MAX_FRM_SIZE+0x3fff)&(~0x3fff))*MAX_DPB_NUM)    // frame buffer for codec (MAX_DPB_NUM)
#define MAX_MV_COL_SIZE                 (((MAX_FRM_SIZE+4)/5)*MAX_DPB_NUM)
#define MAX_PP_SRC_SIZE					(((MAX_FRM_SIZE+0x3fff)&(~0x3fff))*MAX_PP_SRC_NUM)

#define CODEC_FRAME_BASE                ((((MAX_STREAM_BUF_SIZE*MAX_NUM_INSTANCE)) + 0xff) & (~0xff))
#define REQUIRED_VPU_MEMORY_SIZE		(CODEC_FRAME_BASE+((MAX_DPB_SIZE*(1+USE_WTL))+MAX_MV_COL_SIZE+MAX_PP_SRC_SIZE)*MAX_NUM_INSTANCE)

#endif  /* __CODA9_VPU_CONFIG_H__ */
 
