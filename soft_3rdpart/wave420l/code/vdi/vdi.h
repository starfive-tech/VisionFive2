//------------------------------------------------------------------------------
// File: vdi.h
//
// Copyright (c) 2006, Chips & Media.  All rights reserved.
//------------------------------------------------------------------------------

#ifndef _VDI_H_
#define _VDI_H_

#include "mm.h"
#include "vpuconfig.h"
#include "vputypes.h"

/************************************************************************/
/* COMMON REGISTERS                                                     */
/************************************************************************/
#define VPU_PRODUCT_NAME_REGISTER                 0x1040
#define VPU_PRODUCT_CODE_REGISTER                 0x1044

//#define SUPPORT_MULTI_CORE_IN_ONE_DRIVER
#define MAX_VPU_CORE_NUM MAX_NUM_VPU_CORE
#ifdef SUPPORT_SRC_BUF_CONTROL 
#define MAX_VPU_BUFFER_POOL 2000
#else
#define MAX_VPU_BUFFER_POOL (64*MAX_NUM_INSTANCE+12*3)//+12*3 => mvCol + YOfsTable + COfsTable
#endif

#define VpuWriteReg( CORE, ADDR, DATA )                 vdi_write_register( CORE, ADDR, DATA )					// system register write
#define VpuReadReg( CORE, ADDR )                        vdi_read_register( CORE, ADDR )							// system register read
#define VpuWriteMem( CORE, ADDR, DATA, LEN, ENDIAN )    vdi_write_memory( CORE, ADDR, DATA, LEN, ENDIAN )		// system memory write
#define VpuReadMem( CORE, ADDR, DATA, LEN, ENDIAN )     vdi_read_memory( CORE, ADDR, DATA, LEN, ENDIAN )		// system memory read

typedef struct vpu_buffer_t {
    int size;
    unsigned long phys_addr;
    unsigned long base;
    unsigned long virt_addr;	
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

typedef struct vpu_pending_intr_t {
    int instance_id[COMMAND_QUEUE_DEPTH];
    int int_reason[COMMAND_QUEUE_DEPTH];
    int order_num[COMMAND_QUEUE_DEPTH];
    int in_use[COMMAND_QUEUE_DEPTH];
    int num_pending_intr;
    int count;
} vpu_pending_intr_t;

typedef enum {
    VDI_LINEAR_FRAME_MAP  = 0,
    VDI_TILED_FRAME_V_MAP = 1,
    VDI_TILED_FRAME_H_MAP = 2,
    VDI_TILED_FIELD_V_MAP = 3,
    VDI_TILED_MIXED_V_MAP = 4,
    VDI_TILED_FRAME_MB_RASTER_MAP = 5,
    VDI_TILED_FIELD_MB_RASTER_MAP = 6,
    VDI_TILED_FRAME_NO_BANK_MAP = 7,
    VDI_TILED_FIELD_NO_BANK_MAP = 8,
    VDI_LINEAR_FIELD_MAP  = 9,
    VDI_TILED_MAP_TYPE_MAX
} vdi_gdi_tiled_map;

typedef struct vpu_instance_pool_t {
    unsigned char   codecInstPool[MAX_NUM_INSTANCE][MAX_INST_HANDLE_SIZE];  // Since VDI don't know the size of CodecInst structure, VDI should have the enough space not to overflow.
    vpu_buffer_t    vpu_common_buffer;
    int             vpu_instance_num;
    int             instance_pool_inited;
    void*           pendingInst;    
    int             pendingInstIdxPlus1;
    video_mm_t      vmem;    
    vpu_pending_intr_t pending_intr_list;
} vpu_instance_pool_t;

#if defined (__cplusplus)
extern "C" {
#endif 
	int vdi_probe(unsigned long core_idx);
	int vdi_init(unsigned long core_idx);
	int vdi_release(unsigned long core_idx);	//this function may be called only at system off.

	vpu_instance_pool_t * vdi_get_instance_pool(unsigned long core_idx);
	int vdi_allocate_common_memory(unsigned long core_idx, Uint32 size);
	int vdi_get_common_memory(unsigned long core_idx, vpu_buffer_t *vb);
	int vdi_allocate_dma_memory(unsigned long core_idx, vpu_buffer_t *vb);
	int vdi_attach_dma_memory(unsigned long core_idx, vpu_buffer_t *vb);
	void vdi_free_dma_memory(unsigned long core_idx, vpu_buffer_t *vb);
	int vdi_get_sram_memory(unsigned long core_idx, vpu_buffer_t *vb);
	int vdi_dettach_dma_memory(unsigned long core_idx, vpu_buffer_t *vb);

	int vdi_wait_interrupt(unsigned long core_idx, int timeout, unsigned int addr_bit_int_reason);
	int vdi_wait_vpu_busy(unsigned long core_idx, int timeout, unsigned int addr_bit_busy_flag);
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
	int vdi_clear_memory(unsigned long core_idx, unsigned int addr, int len, int endian);
	int vdi_write_memory(unsigned long core_idx, unsigned int addr, unsigned char *data, int len, int endian);
	int vdi_read_memory(unsigned long core_idx, unsigned int addr, unsigned char *data, int len, int endian);

	int vdi_lock(unsigned long core_idx);
	int vdi_lock_check(unsigned long core_idx);
	void vdi_unlock(unsigned long core_idx);	
	int vdi_disp_lock(unsigned long core_idx);
	void vdi_disp_unlock(unsigned long core_idx);
    void vdi_set_sdram(unsigned long core_idx, unsigned int addr, int len, unsigned char data, int endian);
	void vdi_log(unsigned long core_idx, int cmd, int step);
	int vdi_open_instance(unsigned long core_idx, unsigned long inst_idx);
	int vdi_close_instance(unsigned long core_idx, unsigned long inst_idx);
	int vdi_set_bit_firmware_to_pm(unsigned long core_idx, const unsigned short *code);
    int vdi_get_system_endian(unsigned long core_idx);
    int vdi_convert_endian(unsigned long core_idx, unsigned int endian);
	void vdi_print_vpu_status(unsigned long coreIdx);
	void vdi_flush_ddr(unsigned long core_idx,unsigned long start,unsigned long size,unsigned char flag);


#if defined (__cplusplus)
}
#endif 

#endif //#ifndef _VDI_H_ 
 
