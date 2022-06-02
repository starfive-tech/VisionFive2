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
#ifndef _JDI_HPI_H_
#define _JDI_HPI_H_
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#ifdef USE_FEEDING_METHOD_BUFFER
    #include "codaj12/jpuapi/jpuconfig.h"
    #include "codaj12/jdi/mm.h"
    #include "codaj12/jpuapi/jputypes.h"
#else
    #include "../jpuapi/jpuconfig.h"
    #include "mm.h"
    #include "jputypes.h"
#endif


#define MAX_JPU_BUFFER_POOL 32
#define JpuWriteInstReg( INST_IDX, ADDR, DATA )		jdi_write_register( ((unsigned long)INST_IDX*NPT_REG_SIZE)+ADDR, DATA ) // system register write 	with instance index
#define JpuReadInstReg( INST_IDX, ADDR )		jdi_read_register( ((unsigned long)INST_IDX*NPT_REG_SIZE)+ADDR ) // system register write 	with instance index
#define JpuWriteReg( ADDR, DATA )                   jdi_write_register( ADDR, DATA ) // system register write
#define JpuReadReg( ADDR )                          jdi_read_register( ADDR )           // system register write
#define JpuWriteMem( ADDR, DATA, LEN, ENDIAN )      jdi_write_memory( ADDR, DATA, LEN, ENDIAN ) // system memory write
#define JpuReadMem( ADDR, DATA, LEN, ENDIAN )       jdi_read_memory( ADDR, DATA, LEN, ENDIAN ) // system memory write


typedef struct jpu_buffer_t {
    unsigned int  size;
    unsigned long phys_addr;
    unsigned long base;
    unsigned long virt_addr;
} jpu_buffer_t;

typedef struct jpu_instance_pool_t {
    unsigned char   jpgInstPool[MAX_NUM_INSTANCE][MAX_INST_HANDLE_SIZE];
    Int32           jpu_instance_num;
    BOOL            instance_pool_inited;
	void*           instPendingInst[MAX_NUM_INSTANCE];
    jpeg_mm_t       vmem;
} jpu_instance_pool_t;


typedef enum {
    JDI_LITTLE_ENDIAN = 0,
    JDI_BIG_ENDIAN,
    JDI_32BIT_LITTLE_ENDIAN,
    JDI_32BIT_BIG_ENDIAN,
} EndianMode;


typedef enum {
    JDI_LOG_CMD_PICRUN  = 0,
    JDI_LOG_CMD_INIT  = 1,
    JDI_LOG_CMD_RESET  = 2,
    JDI_LOG_CMD_PAUSE_INST_CTRL = 3,
    JDI_LOG_CMD_MAX
} jdi_log_cmd;


#if defined (__cplusplus)
extern "C" {
#endif
    int jdi_probe();
    /* @brief It returns the number of task using JDI.
     */
    int jdi_get_task_num();
    int jdi_init();
    int jdi_release();    //this function may be called only at system off.
    jpu_instance_pool_t *jdi_get_instance_pool();
    int jdi_allocate_dma_memory(jpu_buffer_t *vb);
    void jdi_free_dma_memory(jpu_buffer_t *vb);

    int jdi_wait_interrupt(int timeout, unsigned int addr_int_reason, unsigned long instIdx);
    int jdi_hw_reset();
    int jdi_wait_inst_ctrl_busy(int timeout, unsigned int addr_flag_reg, unsigned int flag);
    int jdi_set_clock_gate(int enable);
    int jdi_get_clock_gate();

    int jdi_open_instance(unsigned long instIdx);
    int jdi_close_instance(unsigned long instIdx);
    int jdi_get_instance_num();


    void jdi_write_register(unsigned long addr, unsigned long data);
    unsigned long jdi_read_register(unsigned long addr);

    int jdi_write_memory(unsigned long addr, unsigned char *data, int len, int endian);
    int jdi_read_memory(unsigned long addr, unsigned char *data, int len, int endian);
    int jdi_attach_dma_memory(jpu_buffer_t *vb);

    int jdi_lock();
    void jdi_unlock();
    void jdi_log(int cmd, int step, int inst);
    void jdi_flush_ddr(unsigned long start,unsigned long size,unsigned char flag);
#define ACLK_MAX                    300
#define ACLK_MIN                    16
#define CCLK_MAX                    300
#define CCLK_MIN                    16




#if defined (__cplusplus)
}
#endif

#endif //#ifndef _JDI_HPI_H_
