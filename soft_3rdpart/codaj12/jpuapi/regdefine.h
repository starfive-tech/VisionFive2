/*
 * Copyright (c) 2018, Chips&Media
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
#include "../config.h"


#ifndef NPT_REGDEFINE_H_INCLUDED
#define NPT_REGDEFINE_H_INCLUDED

//------------------------------------------------------------------------------
// REGISTER BASE
//------------------------------------------------------------------------------
#define NPT_REG_SIZE        0x300
#define NPT_BASE            0x00
#define NPT_PROC_BASE        (NPT_BASE + (4*NPT_REG_SIZE))



/*--------------------------------------------------------------------
NIEUPORT REGISTERS
--------------------------------------------------------------------*/
// MAIN CONTROL REGISTER
#define MJPEG_PIC_START_REG         (NPT_BASE + 0x000)    // [3] - start partial encoding  [2] - pic stop en/decoding[1] - pic init en/decoding [0] - pic start en/decoding
#define MJPEG_PIC_STATUS_REG        (NPT_BASE + 0x004)    // [8] - stop [7:4] - partial buffer interrupt [3] - overflow, [2] - bbc interrupt, [1] - error, [0] - done
#define MJPEG_PIC_ERRMB_REG         (NPT_BASE + 0x008)    // [27:24] - error restart idx, [23:12] - error MCU pos X, [11:0] - error MCU pos Y
#define MJPEG_PIC_SETMB_REG         (NPT_BASE + 0x00C)    // [27:16] - MCU pos X, [11:0] - MCU pos Y

#define MJPEG_PIC_CTRL_REG          (NPT_BASE + 0x010)    // [12:7] huffman table index [6] - user huffman en, [4] - TC direction, [3] - encoder enable, [1:0] - operation mode
#define MJPEG_PIC_SIZE_REG          (NPT_BASE + 0x014)
#define MJPEG_MCU_INFO_REG          (NPT_BASE + 0x018)
#define MJPEG_ROT_INFO_REG          (NPT_BASE + 0x01C)    // [4] - rot-mir enable, [3:0] - rot-mir mode

#define MJPEG_SCL_INFO_REG          (NPT_BASE + 0x020)
#define MJPEG_IF_INFO_REG           (NPT_BASE + 0x024)    // [1] - sensor interface clear, [0] - display interface clear
#define MJPEG_CLP_INFO_REG          (NPT_BASE + 0x028)
#define MJPEG_OP_INFO_REG           (NPT_BASE + 0x02C)    // [31:16] - # of line in 1 partial buffer, [5:3] - # of partial buffers [2:0] - # of request

#define MJPEG_DPB_CONFIG_REG        (NPT_BASE + 0x030)
#define MJPEG_DPB_BASE00_REG        (NPT_BASE + 0x034)
#define MJPEG_DPB_BASE01_REG        (NPT_BASE + 0x038)
#define MJPEG_DPB_BASE02_REG        (NPT_BASE + 0x03C)

#define MJPEG_DPB_BASE10_REG        (NPT_BASE + 0x040)
#define MJPEG_DPB_BASE11_REG        (NPT_BASE + 0x044)
#define MJPEG_DPB_BASE12_REG        (NPT_BASE + 0x048)
#define MJPEG_DPB_BASE20_REG        (NPT_BASE + 0x04C)

#define MJPEG_DPB_BASE21_REG        (NPT_BASE + 0x050)
#define MJPEG_DPB_BASE22_REG        (NPT_BASE + 0x054)
#define MJPEG_DPB_BASE30_REG        (NPT_BASE + 0x058)
#define MJPEG_DPB_BASE31_REG        (NPT_BASE + 0x05C)

#define MJPEG_DPB_BASE32_REG        (NPT_BASE + 0x060)
#define MJPEG_DPB_YSTRIDE_REG       (NPT_BASE + 0x064)
#define MJPEG_DPB_CSTRIDE_REG       (NPT_BASE + 0x068)
#define MJPEG_WRESP_CHECK_REG       (NPT_BASE + 0x06C)

#define MJPEG_CLP_BASE_REG          (NPT_BASE + 0x070)
#define MJPEG_CLP_SIZE_REG          (NPT_BASE + 0x074)

#define MJPEG_HUFF_CTRL_REG         (NPT_BASE + 0x080)
#define MJPEG_HUFF_ADDR_REG         (NPT_BASE + 0x084)
#define MJPEG_HUFF_DATA_REG         (NPT_BASE + 0x088)

#define MJPEG_QMAT_CTRL_REG         (NPT_BASE + 0x090)
#define MJPEG_QMAT_ADDR_REG         (NPT_BASE + 0x094)
#define MJPEG_QMAT_DATA_REG         (NPT_BASE + 0x098)

#define MJPEG_COEF_CTRL_REG         (NPT_BASE + 0x0A0)
#define MJPEG_COEF_ADDR_REG         (NPT_BASE + 0x0A4)
#define MJPEG_COEF_DATA_REG         (NPT_BASE + 0x0A8)

#define MJPEG_RST_INTVAL_REG        (NPT_BASE + 0x0B0)
#define MJPEG_RST_INDEX_REG         (NPT_BASE + 0x0B4)
#define MJPEG_RST_COUNT_REG         (NPT_BASE + 0x0B8)

#define    MJPEG_INTR_MASK_REG      (NPT_BASE + 0x0C0)
#define MJPEG_CYCLE_INFO_REG        (NPT_BASE + 0x0C8)

#define MJPEG_DPCM_DIFF_Y_REG       (NPT_BASE + 0x0F0)
#define MJPEG_DPCM_DIFF_CB_REG      (NPT_BASE + 0x0F4)
#define MJPEG_DPCM_DIFF_CR_REG      (NPT_BASE + 0x0F8)
#define MJPEG_VERSION_INFO_REG      (NPT_BASE + 0x0FC)


// GBU CONTROL REGISTER
#define MJPEG_GBU_CTRL_REG          (NPT_BASE + 0x100) // [3] - GBU flush for encoding  [2] - init GBU for decoding [1] - init GBU ff emulation for decoding [0] - reserved
#define MJPEG_GBU_PBIT_BUSY_REG     (NPT_BASE + 0x104)

#define MJPEG_GBU_BPTR_REG          (NPT_BASE + 0x110)
#define MJPEG_GBU_WPTR_REG          (NPT_BASE + 0x114)
#define MJPEG_GBU_TCNT_REG          (NPT_BASE + 0x118)
//#define MJPEG_GBU_TCNT_REG+4      (NPT_BASE + 0x11C)

#define MJPEG_GBU_PBIT_08_REG       (NPT_BASE + 0x120)
#define MJPEG_GBU_PBIT_16_REG       (NPT_BASE + 0x124)
#define MJPEG_GBU_PBIT_24_REG       (NPT_BASE + 0x128)
#define MJPEG_GBU_PBIT_32_REG       (NPT_BASE + 0x12C)

#define MJPEG_GBU_BBSR_REG          (NPT_BASE + 0x140)
#define MJPEG_GBU_BBER_REG          (NPT_BASE + 0x144)
#define MJPEG_GBU_BBIR_REG          (NPT_BASE + 0x148)
#define MJPEG_GBU_BBHR_REG          (NPT_BASE + 0x14C)

#define MJPEG_GBU_BCNT_REG          (NPT_BASE + 0x158)

#define MJPEG_GBU_FF_RPTR_REG       (NPT_BASE + 0x160)
#define MJPEG_GBU_FF_WPTR_REG       (NPT_BASE + 0x164)

// BBC CONTROL REGISTER
#define MJPEG_BBC_END_ADDR_REG      (NPT_BASE + 0x208)
#define MJPEG_BBC_WR_PTR_REG        (NPT_BASE + 0x20C)
#define MJPEG_BBC_RD_PTR_REG        (NPT_BASE + 0x210)

#define MJPEG_BBC_EXT_ADDR_REG      (NPT_BASE + 0x214)
#define MJPEG_BBC_INT_ADDR_REG      (NPT_BASE + 0x218)
#define MJPEG_BBC_DATA_CNT_REG      (NPT_BASE + 0x21C)
#define MJPEG_BBC_COMMAND_REG       (NPT_BASE + 0x220) // [2:1] - endianness  [0] - load/save
#define MJPEG_BBC_BUSY_REG          (NPT_BASE + 0x224)

#define MJPEG_BBC_CTRL_REG          (NPT_BASE + 0x228) // [2:1] - endianness  [0] - BBC auto run
#define MJPEG_BBC_CUR_POS_REG       (NPT_BASE + 0x22C)

#define MJPEG_BBC_BAS_ADDR_REG      (NPT_BASE + 0x230)
#define MJPEG_BBC_STRM_CTRL_REG     (NPT_BASE + 0x234) // [31] - end of bitstream file  [23:0] - stream counter

#define MJPEG_BBC_FLUSH_CMD_REG     (NPT_BASE + 0x238)



#define MJPEG_SLICE_INFO_REG        (NPT_BASE + 0x240)
#define MJPEG_SLICE_POS_REG         (NPT_BASE + 0x244)
#define MJPEG_SLICE_DPB_POS_REG     (NPT_BASE + 0x248)

#define MJPEG_INST_CTRL_START_REG   (NPT_PROC_BASE + 0x000)
#define MJPEG_INST_CTRL_STATUS_REG  (NPT_PROC_BASE + 0x004)


#endif

