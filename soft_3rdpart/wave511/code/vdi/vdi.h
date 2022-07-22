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

#ifndef _VDI_H_
#define _VDI_H_

#ifdef USE_FEEDING_METHOD_BUFFER
    #include "wave511/vdi/mm.h"
    #include "wave511/vpuapi/vpuconfig.h"
    #include "wave511/vpuapi/vputypes.h"
#else
    #include "mm.h"
    #include "vpuconfig.h"
    #include "vputypes.h"
#endif



/************************************************************************/
/* COMMON REGISTERS                                                     */
/************************************************************************/
#define VPU_PRODUCT_NAME_REGISTER                 0x1040
#define VPU_PRODUCT_CODE_REGISTER                 0x1044

#define SUPPORT_MULTI_CORE_IN_ONE_DRIVER
#define MAX_VPU_CORE_NUM MAX_NUM_VPU_CORE

#define MAX_VPU_BUFFER_POOL 1000

#define VpuWriteReg( CORE, ADDR, DATA )                 vdi_write_register( CORE, ADDR, DATA )					// system register write
#define VpuReadReg( CORE, ADDR )                        vdi_read_register( CORE, ADDR )							// system register read
#define VpuWriteMem( CORE, ADDR, DATA, LEN, ENDIAN )    vdi_write_memory( CORE, ADDR, DATA, LEN, ENDIAN )		// system memory write
#define VpuReadMem( CORE, ADDR, DATA, LEN, ENDIAN )     vdi_read_memory( CORE, ADDR, DATA, LEN, ENDIAN )		// system memory read

typedef struct vpu_buffer_t {
    Uint32 size;
    PhysicalAddress phys_addr;
    unsigned long   base;
    unsigned long   virt_addr;
} vpu_buffer_t;

typedef enum {
    VDI_LITTLE_ENDIAN = 0,      /* 64bit LE */
    VDI_BIG_ENDIAN,             /* 64bit BE */
    VDI_32BIT_LITTLE_ENDIAN,
    VDI_32BIT_BIG_ENDIAN,
    /* WAVE PRODUCTS */
    VDI_128BIT_LITTLE_ENDIAN    = 16,
    VDI_128BIT_LE_BYTE_SWAP,
    VDI_128BIT_LE_WORD_SWAP,
    VDI_128BIT_LE_WORD_BYTE_SWAP,
    VDI_128BIT_LE_DWORD_SWAP,
    VDI_128BIT_LE_DWORD_BYTE_SWAP,
    VDI_128BIT_LE_DWORD_WORD_SWAP,
    VDI_128BIT_LE_DWORD_WORD_BYTE_SWAP,
    VDI_128BIT_BE_DWORD_WORD_BYTE_SWAP,
    VDI_128BIT_BE_DWORD_WORD_SWAP,
    VDI_128BIT_BE_DWORD_BYTE_SWAP,
    VDI_128BIT_BE_DWORD_SWAP,
    VDI_128BIT_BE_WORD_BYTE_SWAP,
    VDI_128BIT_BE_WORD_SWAP,
    VDI_128BIT_BE_BYTE_SWAP,
    VDI_128BIT_BIG_ENDIAN        = 31,
    VDI_ENDIAN_MAX
} EndianMode;

#define VDI_128BIT_ENDIAN_MASK      0xf

typedef struct vpu_instance_pool_t {
    unsigned char   codecInstPool[MAX_NUM_INSTANCE][MAX_INST_HANDLE_SIZE];  // Since VDI don't know the size of CodecInst structure, VDI should have the enough space not to overflow.
    vpu_buffer_t    vpu_common_buffer;
    int             vpu_instance_num;
    int             instance_pool_inited;
    void*           pendingInst;
    int             pendingInstIdxPlus1;
    Uint32          lastPerformanceCycles;
    video_mm_t      vmem;
} vpu_instance_pool_t;


#if defined (__cplusplus)
extern "C" {
#endif
    int vdi_probe(unsigned long core_idx);
    int vdi_init(unsigned long core_idx);
    int vdi_release(unsigned long core_idx);	//this function may be called only at system off.

    vpu_instance_pool_t * vdi_get_instance_pool(unsigned long core_idx);
    int vdi_allocate_common_memory(unsigned long core_idx);
    int vdi_get_common_memory(unsigned long core_idx, vpu_buffer_t *vb);
    int vdi_allocate_dma_memory(unsigned long core_idx, vpu_buffer_t *vb, int memTypes, int instIndex);
    int vdi_attach_dma_memory(unsigned long core_idx, vpu_buffer_t *vb);
    void vdi_free_dma_memory(unsigned long core_idx, vpu_buffer_t *vb, int memTypes, int instIndex);
    int vdi_get_sram_memory(unsigned long core_idx, vpu_buffer_t *vb);
    int vdi_dettach_dma_memory(unsigned long core_idx, vpu_buffer_t *vb);
    void* vdi_map_virt2(unsigned long core_idx, int size, PhysicalAddress bufY);
    int vdi_virt_to_phys(unsigned long core_idx, vpu_buffer_t *vb);

#ifdef SUPPORT_MULTI_INST_INTR
    int vdi_wait_interrupt(unsigned long coreIdx, unsigned int instIdx, int timeout);
#else
    int vdi_wait_interrupt(unsigned long core_idx, int timeout);
#endif

    int vdi_wait_vpu_busy(unsigned long core_idx, int timeout, unsigned int addr_bit_busy_flag);
    int vdi_wait_vcpu_bus_busy(unsigned long core_idx, int timeout, unsigned int addr_bit_busy_flag);
    int vdi_wait_bus_busy(unsigned long core_idx, int timeout, unsigned int gdi_busy_flag);
    int vdi_hw_reset(unsigned long core_idx);

    int vdi_set_clock_gate(unsigned long core_idx, int enable);
    int vdi_get_clock_gate(unsigned long core_idx);
    /**
     * @brief       make clock stable before changing clock frequency
     * @detail      Before inoking vdi_set_clock_freg() caller MUST invoke vdi_ready_change_clock() function.
     *              after changing clock frequency caller also invoke vdi_done_change_clock() function.
     * @return  0   failure
     *          1   success
     */
    int vdi_ready_change_clock(unsigned long core_idx);
    int vdi_set_change_clock(unsigned long core_idx, unsigned long clock_mask);
    int vdi_done_change_clock(unsigned long core_idx);

    int  vdi_get_instance_num(unsigned long core_idx);

    void vdi_write_register(unsigned long core_idx, unsigned int addr, unsigned int data);
    unsigned int vdi_read_register(unsigned long core_idx, unsigned int addr);
    void vdi_fio_write_register(unsigned long core_idx, unsigned int addr, unsigned int data);
    unsigned int vdi_fio_read_register(unsigned long core_idx, unsigned int addr);
    int vdi_clear_memory(unsigned long core_idx, PhysicalAddress addr, int len, int endian);
    int vdi_write_memory(unsigned long core_idx, PhysicalAddress addr, unsigned char *data, int len, int endian);
    int vdi_read_memory(unsigned long core_idx, PhysicalAddress addr, unsigned char *data, int len, int endian);
    int vdi_read_memory2(unsigned long core_idx, PhysicalAddress addr, unsigned char **ppdata, int len, int endian);


    int vdi_lock(unsigned long core_idx);
    void vdi_unlock(unsigned long core_idx);
    int vdi_disp_lock(unsigned long core_idx);
    void vdi_disp_unlock(unsigned long core_idx);
    int vdi_open_instance(unsigned long core_idx, unsigned long inst_idx);
    int vdi_close_instance(unsigned long core_idx, unsigned long inst_idx);
    int vdi_set_bit_firmware_to_pm(unsigned long core_idx, const unsigned short *code);
    int vdi_get_system_endian(unsigned long core_idx);
    int vdi_convert_endian(unsigned long core_idx, unsigned int endian);
    void vdi_flush_ddr(unsigned long core_idx,unsigned long start,unsigned long size,unsigned char flag);

#if defined(SUPPORT_SW_UART) || defined(SUPPORT_SW_UART_V2)
    int vdi_get_task_num(unsigned long core_idx);
#endif
#if defined (__cplusplus)
}
#endif

#endif //#ifndef _VDI_H_

