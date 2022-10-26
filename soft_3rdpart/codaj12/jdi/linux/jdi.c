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
#if defined(linux) || defined(__linux) || defined(ANDROID)

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#ifdef    _KERNEL_
#include <linux/delay.h>
#endif
#include <signal.h>        /* SIGIO */
#include <fcntl.h>        /* fcntl */
#include <pthread.h>
#include <sys/mman.h>        /* mmap */
#include <sys/ioctl.h>        /* fopen/fread */
#include <sys/errno.h>        /* fopen/fread */
#include <sys/types.h>
#include <sys/time.h>
#include <termios.h>

#include "driver/jpu.h"
#include "../jdi.h"
#include "jpulog.h"
#include "jputypes.h"
#include "regdefine.h"


#define JPU_DEVICE_NAME "/dev/jpu"
#define JDI_INSTANCE_POOL_SIZE          sizeof(jpu_instance_pool_t)
#define JDI_INSTANCE_POOL_TOTAL_SIZE    (JDI_INSTANCE_POOL_SIZE + sizeof(MUTEX_HANDLE)*JDI_NUM_LOCK_HANDLES)
typedef pthread_mutex_t    MUTEX_HANDLE;

#endif /* CNM_FPGA_PLATFORM */



/***********************************************************************************
*
***********************************************************************************/

#define JDI_DRAM_PHYSICAL_BASE          0x00
#define JDI_DRAM_PHYSICAL_SIZE          (64*1024*1024)
#define JDI_SYSTEM_ENDIAN               JDI_LITTLE_ENDIAN
#define JPU_REG_SIZE                    0x300
#define JDI_NUM_LOCK_HANDLES            4

typedef struct jpudrv_buffer_pool_t
{
    jpudrv_buffer_t jdb;
    BOOL            inuse;
} jpudrv_buffer_pool_t;

typedef struct  {
    Int32                   jpu_fd;
    jpu_instance_pool_t*    pjip;
    Int32                   task_num;
    Int32                   clock_state;
    jpudrv_buffer_t         jdb_register;
    jpudrv_buffer_pool_t    jpu_buffer_pool[MAX_JPU_BUFFER_POOL];
    Int32                   jpu_buffer_pool_count;
    void*                   jpu_mutex;
} jdi_info_t;

static jdi_info_t s_jdi_info;

static Int32 swap_endian(BYTE* data, Uint32 len, Uint32 endian);

void jdi_flush_ddr(unsigned long start,unsigned long size,unsigned char flag)
{
	jdi_info_t *jdi;
	jpudrv_flush_cache_t cache_info;
	
	jdi = &s_jdi_info;
	cache_info.start = start;
	cache_info.size = size;
	cache_info.flag = flag;

	ioctl(jdi->jpu_fd, JDI_IOCTL_FLUSH_DCACHE, &cache_info);
}

int jdi_probe()
{
    int ret;

    ret = jdi_init();
    jdi_release();

    return ret;
}

/* @return number of tasks.
 */
int jdi_get_task_num()
{
    jdi_info_t *jdi;

    jdi = &s_jdi_info;

    if (jdi->jpu_fd == -1 || jdi->jpu_fd == 0x00) {
        return 0;
    }

    return jdi->task_num;
}


int jdi_init()
{
    jdi_info_t *jdi;
    int i;

    jdi = &s_jdi_info;

    if (jdi->jpu_fd != -1 && jdi->jpu_fd != 0x00)
    {
        jdi_lock();
        jdi->task_num++;
        jdi_unlock();
        return 0;
    }

#ifdef ANDROID
    system("/system/lib/modules/load_android.sh");
#endif /* ANDROID */

    jdi->jpu_fd = open(JPU_DEVICE_NAME, O_RDWR);
    if (jdi->jpu_fd < 0) {
        JLOG(ERR, "[JDI] Can't open jpu driver. [error=%s]. try to load jpu driver first \n", strerror(errno));
        return -1;
    }

    memset(jdi->jpu_buffer_pool, 0x00, sizeof(jpudrv_buffer_pool_t)*MAX_JPU_BUFFER_POOL);

    if (!jdi_get_instance_pool()) {
        JLOG(ERR, "[JDI] fail to create instance pool for saving context \n");
        goto ERR_JDI_INIT;
    }

    if (jdi->pjip->instance_pool_inited == FALSE) {
        Uint32* pCodecInst;
        pthread_mutexattr_t mutexattr;
        pthread_mutexattr_init(&mutexattr);
        pthread_mutexattr_setpshared(&mutexattr, PTHREAD_PROCESS_SHARED);
#if defined(ANDROID) || !defined(PTHREAD_MUTEX_ROBUST_NP)
#else
         /* If a process or a thread is terminated abnormally,
         * pthread_mutexattr_setrobust_np(attr, PTHREAD_MUTEX_ROBUST_NP) makes
         * next onwer call pthread_mutex_lock() without deadlock.
         */
        pthread_mutexattr_setrobust_np(&mutexattr, PTHREAD_MUTEX_ROBUST_NP);
#endif
        pthread_mutex_init((MUTEX_HANDLE *)jdi->jpu_mutex, &mutexattr);

        for( i = 0; i < MAX_NUM_INSTANCE; i++) {
            pCodecInst    = (Uint32*)jdi->pjip->jpgInstPool[i];
            pCodecInst[1] = i;    // indicate instIndex of CodecInst
            pCodecInst[0] = 0;    // indicate inUse of CodecInst
        }
        jdi->pjip->instance_pool_inited = TRUE;
    }
    if (ioctl(jdi->jpu_fd, JDI_IOCTL_GET_REGISTER_INFO, &jdi->jdb_register) < 0)
    {
        JLOG(ERR, "[JDI] fail to get host interface register\n");
        goto ERR_JDI_INIT;
    }
    jdi->jdb_register.virt_addr = (unsigned long)mmap(NULL, jdi->jdb_register.size, PROT_READ | PROT_WRITE, MAP_SHARED, jdi->jpu_fd, jdi->jdb_register.phys_addr);
    if (jdi->jdb_register.virt_addr == (unsigned long)MAP_FAILED) {
        JLOG(ERR, "[JDI] fail to map jpu registers \n");
        goto ERR_JDI_INIT;
    }
    JLOG(INFO, "[JDI] map jdb_register virtaddr=0x%lx, size=%d\n", jdi->jdb_register.virt_addr, jdi->jdb_register.size);
    jdi_set_clock_gate(1);



    if (jdi_lock() < 0)
    {
        JLOG(ERR, "[JDI] fail to handle lock function\n");
        goto ERR_JDI_INIT;
    }

    jdi->task_num++;
    jdi_unlock();

    JLOG(INFO, "[JDI] success to init driver \n");
    return 0;

ERR_JDI_INIT:
    jdi_unlock();
    jdi_release();
    return -1;
}

int jdi_release()
{
    jdi_info_t *jdi;

    jdi = &s_jdi_info;
    if (!jdi || jdi->jpu_fd == -1 || jdi->jpu_fd == 0x00) {
        return 0;
    }

    if (jdi_lock() < 0) {
        JLOG(ERR, "[JDI] fail to handle lock function\n");
        return -1;
    }

    if (jdi->task_num == 0) {
        JLOG(ERR, "[JDI] %s:%d task_num is 0\n", __FUNCTION__, __LINE__);
        jdi_unlock();
        return 0;
    }

    jdi->task_num--;
    if (jdi->task_num > 0) {// means that the opened instance remains
        jdi_unlock();
        return 0;
    }
    if (jdi->jdb_register.virt_addr) {
        if (munmap((void *)jdi->jdb_register.virt_addr, jdi->jdb_register.size) < 0) { //lint !e511
            JLOG(ERR, "%s:%d failed to munmap\n", __FUNCTION__, __LINE__);
        }
    }

    memset(&jdi->jdb_register, 0x00, sizeof(jpudrv_buffer_t));
    jdi_unlock();
    if (jdi->jpu_fd != -1 && jdi->jpu_fd != 0x00) {
        if (jdi->pjip != NULL) {
            if (munmap((void*)jdi->pjip, JDI_INSTANCE_POOL_TOTAL_SIZE) < 0) {
                JLOG(ERR, "%s:%d failed to munmap\n", __FUNCTION__, __LINE__);
            }
        }
        close(jdi->jpu_fd);
    }

    memset(jdi, 0x00, sizeof(jdi_info_t));

    return 0;
}

jpu_instance_pool_t *jdi_get_instance_pool()
{
    jdi_info_t *jdi;
    jpudrv_buffer_t jdb;

    jdi = &s_jdi_info;

    if(!jdi || jdi->jpu_fd == -1 || jdi->jpu_fd == 0x00 )
        return NULL;

    memset(&jdb, 0x00, sizeof(jpudrv_buffer_t));
    if (!jdi->pjip) {
        jdb.size = JDI_INSTANCE_POOL_TOTAL_SIZE;
        if (ioctl(jdi->jpu_fd, JDI_IOCTL_GET_INSTANCE_POOL, &jdb) < 0) {
            JLOG(ERR, "[JDI] fail to allocate get instance pool physical space=%d\n", (int)jdb.size);
            return NULL;
        }

        jdb.virt_addr = (unsigned long)mmap(NULL, jdb.size, PROT_READ | PROT_WRITE, MAP_SHARED, jdi->jpu_fd, 0);
        if (jdb.virt_addr == (unsigned long)MAP_FAILED) {
            JLOG(ERR, "[JDI] fail to map instance pool phyaddr=0x%lx, size = %d\n", (int)jdb.phys_addr, (int)jdb.size);
            return NULL;
        }
        jdi->pjip      = (jpu_instance_pool_t *)jdb.virt_addr;//lint !e511
        //change the pointer of jpu_mutex to at end pointer of jpu_instance_pool_t to assign at allocated position.
        jdi->jpu_mutex = (void *)((unsigned long)jdi->pjip + JDI_INSTANCE_POOL_SIZE); //lint !e511

        JLOG(INFO, "[JDI] instance pool physaddr=%p, virtaddr=%p, base=%p, size=%d\n", jdb.phys_addr, jdb.virt_addr, jdb.base, jdb.size);
    }

    return (jpu_instance_pool_t *)jdi->pjip;
}

int jdi_open_instance(unsigned long inst_idx)
{
    jdi_info_t *jdi;
    jpudrv_inst_info_t inst_info;

    jdi = &s_jdi_info;

    if(!jdi || jdi->jpu_fd == -1 || jdi->jpu_fd == 0x00)
        return -1;

    inst_info.inst_idx = inst_idx;
    if (ioctl(jdi->jpu_fd, JDI_IOCTL_OPEN_INSTANCE, &inst_info) < 0)
    {
        JLOG(ERR, "[JDI] fail to deliver open instance num inst_idx=%d\n", (int)inst_idx);
        return -1;
    }
    jdi->pjip->jpu_instance_num = inst_info.inst_open_count;

    return 0;
}

int jdi_close_instance(unsigned long inst_idx)
{
    jdi_info_t *jdi;
    jpudrv_inst_info_t inst_info;

    jdi = &s_jdi_info;

    if(!jdi || jdi->jpu_fd == -1 || jdi->jpu_fd == 0x00)
        return -1;

    inst_info.inst_idx = inst_idx;
    if (ioctl(jdi->jpu_fd, JDI_IOCTL_CLOSE_INSTANCE, &inst_info) < 0)
    {
        JLOG(ERR, "[JDI] fail to deliver open instance num inst_idx=%d\n", (int)inst_idx);
        return -1;
    }
    jdi->pjip->jpu_instance_num = inst_info.inst_open_count;

    return 0;
}
int jdi_get_instance_num()
{
    jdi_info_t *jdi;
    jdi = &s_jdi_info;

    if(!jdi || jdi->jpu_fd == -1 || jdi->jpu_fd == 0x00)
        return -1;

    return jdi->pjip->jpu_instance_num;
}

int jdi_hw_reset()
{
    jdi_info_t *jdi;
    jdi = &s_jdi_info;

    if(!jdi || jdi->jpu_fd == -1 || jdi->jpu_fd == 0x00)
        return -1;

    return ioctl(jdi->jpu_fd, JDI_IOCTL_RESET, 0);

}

static void restore_mutex_in_dead(MUTEX_HANDLE *mutex)
{
    int mutex_value;

    if (!mutex)
        return;
#if defined(ANDROID)
    mutex_value = mutex->value;
#else
    memcpy(&mutex_value, mutex, sizeof(mutex_value));
#endif
    if (mutex_value == (int)0xdead10cc) // destroy by device driver
    {
        pthread_mutexattr_t mutexattr;
        pthread_mutexattr_init(&mutexattr);
        pthread_mutexattr_setpshared(&mutexattr, PTHREAD_PROCESS_SHARED);
        pthread_mutex_init(mutex, &mutexattr);
    }
}

int jdi_lock()
{
    jdi_info_t *jdi = &s_jdi_info;

    if(!jdi || jdi->jpu_fd == -1 || jdi->jpu_fd == 0x00) {
        JLOG(ERR, "%s:%d JDI handle isn't initialized\n", __FUNCTION__, __LINE__);
        return -1;
    }


#if defined(ANDROID) || !defined(PTHREAD_MUTEX_ROBUST_NP)
    restore_mutex_in_dead((MUTEX_HANDLE *)jdi->jpu_mutex);
    pthread_mutex_lock((MUTEX_HANDLE*)jdi->jpu_mutex);
#else
    if (pthread_mutex_lock(&jdi->jpu_mutex) != 0) {
        JLOG(ERR, "%s:%d failed to pthread_mutex_locK\n", __FUNCTION__, __LINE__);
        return -1;
    }
#endif

    return 0;//lint !e454
}

void jdi_unlock()
{
    jdi_info_t *jdi;

    jdi = &s_jdi_info;

    if(!jdi || jdi->jpu_fd <= 0)
        return;

    pthread_mutex_unlock((MUTEX_HANDLE *)jdi->jpu_mutex);//lint !e455
}
void jdi_write_register(unsigned long addr, unsigned long data)
{
    jdi_info_t *jdi;
    unsigned long *reg_addr;

    jdi = &s_jdi_info;

    if(!jdi || jdi->jpu_fd == -1 || jdi->jpu_fd == 0x00)
        return;


    reg_addr = (unsigned long *)(addr + (unsigned long)jdi->jdb_register.virt_addr);
    *(volatile unsigned int *)reg_addr = data;
}

unsigned long jdi_read_register(unsigned long addr)
{
    jdi_info_t *jdi;
    unsigned long *reg_addr;

    jdi = &s_jdi_info;

    if(!jdi || jdi->jpu_fd == -1 || jdi->jpu_fd == 0x00)
        return (unsigned int)-1;


    reg_addr = (unsigned long *)(addr + (unsigned long)jdi->jdb_register.virt_addr);
    return *(volatile unsigned int *)reg_addr;
}

int jdi_write_memory(unsigned long addr, unsigned char *data, int len, int endian)
{
    jdi_info_t *jdi;
    jpudrv_buffer_t jdb;
    Uint32          offset;
    Uint32          i;

    jdi = &s_jdi_info;

    if(!jdi || jdi->jpu_fd==-1 || jdi->jpu_fd == 0x00)
        return -1;

    memset(&jdb, 0x00, sizeof(jpudrv_buffer_t));

    for (i=0; i<MAX_JPU_BUFFER_POOL; i++)
    {
        if (jdi->jpu_buffer_pool[i].inuse == 1)
        {
            jdb = jdi->jpu_buffer_pool[i].jdb;
            if (addr >= jdb.phys_addr && addr < (jdb.phys_addr + jdb.size)) {
                break;
            }
        }
    }

    if (i == MAX_JPU_BUFFER_POOL) {
        JLOG(ERR, "%s NOT FOUND ADDRESS: %#lx\n", __FUNCTION__, addr);
        return -1;
    }

    if (!jdb.size) {
        JLOG(ERR, "address 0x%08x is not mapped address!!!\n", (int)addr);
        return -1;
    }

    offset = addr - (unsigned long)jdb.phys_addr;
    swap_endian(data, len, endian);
    memcpy((void *)((unsigned long)jdb.virt_addr+offset), data, len);

   jdi_flush_ddr((unsigned long )(jdb.phys_addr+offset),len,1);
	
    return len;
}

int jdi_read_memory(unsigned long addr, unsigned char *data, int len, int endian)
{
    jdi_info_t *jdi;
    jpudrv_buffer_t jdb;
    unsigned long offset;
    int i;

    jdi = &s_jdi_info;

    if(!jdi || jdi->jpu_fd==-1 || jdi->jpu_fd == 0x00)
        return -1;

    memset(&jdb, 0x00, sizeof(jpudrv_buffer_t));

    for (i=0; i<MAX_JPU_BUFFER_POOL; i++)
    {
        if (jdi->jpu_buffer_pool[i].inuse == 1)
        {
            jdb = jdi->jpu_buffer_pool[i].jdb;
            if (addr >= jdb.phys_addr && addr < (jdb.phys_addr + jdb.size))
                break;
        }
    }

    if (len == 0) {
        return 0;
    }

    if (!jdb.size)
        return -1;



    offset = addr - (unsigned long)jdb.phys_addr;

    jdi_flush_ddr((unsigned long )(jdb.phys_addr+offset),len,1);
    memcpy(data, (const void *)((unsigned long)jdb.virt_addr+offset), len);
    swap_endian(data, len,  endian);

    return len;
}

int jdi_attach_dma_memory(jpu_buffer_t *vb)
{
    jdi_info_t *jdi;
    int i;
    jpudrv_buffer_t jdb;

    jdi = &s_jdi_info;

    if(!jdi || jdi->jpu_fd==-1 || jdi->jpu_fd == 0x00)
        return -1;

    memset(&jdb, 0x00, sizeof(jpudrv_buffer_t));

    jdb.virt_addr = vb->virt_addr;
    if (ioctl(jdi->jpu_fd, JDI_IOCTL_GET_PHYSICAL_MEMORY, &jdb) < 0)
    {
        JLOG(ERR, "[JDI] fail to jdi_allocate_dma_memory size=%d\n", vb->size);
        return -1;
    }

    vb->phys_addr = (unsigned long)jdb.phys_addr;
    vb->base = (unsigned long)jdb.base;

    jdi_lock();
    for (i=0; i<MAX_JPU_BUFFER_POOL; i++)
    {
        if (jdi->jpu_buffer_pool[i].inuse == 0)
        {
            jdi->jpu_buffer_pool[i].jdb = jdb;
            jdi->jpu_buffer_pool_count++;
            jdi->jpu_buffer_pool[i].inuse = 1;
            break;
        }
    }
    jdi_unlock();
    JLOG(INFO, "[JDI] jdi_attach_dma_memory, physaddr=%p, virtaddr=%p~%p, size=%d\n",
         vb->phys_addr, vb->virt_addr, vb->virt_addr + vb->size, vb->size);
    return 0;
}

int jdi_allocate_dma_memory(jpu_buffer_t *vb)
{
    jdi_info_t *jdi;
    int i;
    jpudrv_buffer_t jdb;

    jdi = &s_jdi_info;

    if(!jdi || jdi->jpu_fd==-1 || jdi->jpu_fd == 0x00)
        return -1;

    memset(&jdb, 0x00, sizeof(jpudrv_buffer_t));

    jdb.size = vb->size;
    if (ioctl(jdi->jpu_fd, JDI_IOCTL_ALLOCATE_PHYSICAL_MEMORY, &jdb) < 0)
    {
        JLOG(ERR, "[JDI] fail to jdi_allocate_dma_memory size=%d\n", vb->size);
        return -1;
    }

    vb->phys_addr = (unsigned long)jdb.phys_addr;
    vb->base = (unsigned long)jdb.base;

    //map to virtual address
    jdb.virt_addr = (unsigned long)mmap(NULL, jdb.size, PROT_READ | PROT_WRITE, MAP_SHARED, jdi->jpu_fd, MEM2SYS(jdb.phys_addr));
    if (jdb.virt_addr == (unsigned long)MAP_FAILED) {
        memset(vb, 0x00, sizeof(jpu_buffer_t));
        return -1;
    }
    vb->virt_addr = jdb.virt_addr;

    jdi_lock();
    for (i=0; i<MAX_JPU_BUFFER_POOL; i++)
    {
        if (jdi->jpu_buffer_pool[i].inuse == 0)
        {
            jdi->jpu_buffer_pool[i].jdb = jdb;
            jdi->jpu_buffer_pool_count++;
            jdi->jpu_buffer_pool[i].inuse = 1;
            break;
        }
    }
    jdi_unlock();
    JLOG(INFO, "[JDI] jdi_allocate_dma_memory, physaddr=%p, virtaddr=%p~%p, size=%d\n",
         vb->phys_addr, vb->virt_addr, vb->virt_addr + vb->size, vb->size);
    return 0;
}

void jdi_free_dma_memory(jpu_buffer_t *vb)
{
    jdi_info_t *jdi;
    int i;
    jpudrv_buffer_t jdb;


    jdi = &s_jdi_info;

    if(!vb || !jdi || jdi->jpu_fd==-1 || jdi->jpu_fd == 0x00)
        return;

    if (vb->size == 0)
        return ;

    memset(&jdb, 0x00, sizeof(jpudrv_buffer_t));

    jdi_lock();
    for (i=0; i<MAX_JPU_BUFFER_POOL; i++) {
        if (jdi->jpu_buffer_pool[i].jdb.phys_addr == vb->phys_addr) {
            jdi->jpu_buffer_pool[i].inuse = 0;
            jdi->jpu_buffer_pool_count--;
            jdb = jdi->jpu_buffer_pool[i].jdb;
            break;
        }
    }
	jdi_unlock();

    if (!jdb.size)
    {
        JLOG(ERR, "[JDI] invalid buffer to free address = 0x%lx\n", (int)jdb.virt_addr);
        return ;
    }
    ioctl(jdi->jpu_fd, JDI_IOCTL_FREE_PHYSICALMEMORY, &jdb);

    if (munmap((void *)jdb.virt_addr, jdb.size) != 0) {
        JLOG(ERR, "[JDI] fail to jdi_free_dma_memory virtial address = 0x%lx\n", (int)jdb.virt_addr);
    }
    memset(vb, 0, sizeof(jpu_buffer_t));
}



int jdi_set_clock_gate(int enable)
{
    jdi_info_t *jdi = NULL;
    int ret;

    jdi = &s_jdi_info;
    if(!jdi || jdi->jpu_fd==-1 || jdi->jpu_fd == 0x00)
        return -1;

    jdi->clock_state = enable;
    ret = ioctl(jdi->jpu_fd, JDI_IOCTL_SET_CLOCK_GATE, &enable);

    return ret;
}

int jdi_get_clock_gate()
{
    jdi_info_t *jdi;
    int ret;

    jdi = &s_jdi_info;

    if(!jdi || jdi->jpu_fd==-1 || jdi->jpu_fd == 0x00)
        return -1;

    ret = jdi->clock_state;

    return ret;
}




int jdi_wait_inst_ctrl_busy(int timeout, unsigned int addr_flag_reg, unsigned int flag)
{
	Int64 elapse, cur;
	unsigned int data_flag_reg;
	struct timeval tv;
	int retry_count;
	tv.tv_sec = 0;
	tv.tv_usec = 0;
	gettimeofday(&tv, NULL);
	elapse = tv.tv_sec*1000 + tv.tv_usec/1000;


	while(1)
	{
		data_flag_reg = jdi_read_register(addr_flag_reg);

		if (((data_flag_reg >> 4)&0xf) == flag)
			break;

		gettimeofday(&tv, NULL);
		cur = tv.tv_sec * 1000 + tv.tv_usec / 1000;

		if (timeout > 0 && (cur - elapse) > timeout)
		{

			for (retry_count=0; retry_count<10; retry_count++) {
				JLOG(ERR, "[VDI] jdi_wait_inst_ctrl_busy timeout, 0x%x=0x%lx\n", addr_flag_reg, jdi_read_register(addr_flag_reg));
			}
			return -1;
		}
	}
	return 0;

}


int jdi_wait_interrupt(int timeout, unsigned int addr_int_reason, unsigned long instIdx)
{
    int intr_reason = 0;
    jdi_info_t *jdi;
    int ret;
    jpudrv_intr_info_t intr_info;


    jdi = &s_jdi_info;

    if(!jdi || jdi->jpu_fd <= 0)
        return -1;


    intr_info.timeout     = timeout;
    intr_info.intr_reason = 0;
    intr_info.inst_idx    = instIdx;
    ret = ioctl(jdi->jpu_fd, JDI_IOCTL_WAIT_INTERRUPT, (void*)&intr_info);
    if (ret != 0)
        return -1;
    intr_reason = intr_info.intr_reason;

    return intr_reason;
}

void jdi_log(int cmd, int step, int inst)
{
    Int32   i;

    switch(cmd)
    {
    case JDI_LOG_CMD_PICRUN:
        if (step == 1)    //
            JLOG(INFO, "\n**PIC_RUN INST=%d start\n", inst);
        else
            JLOG(INFO, "\n**PIC_RUN INST=%d  end \n", inst);
        break;
    case JDI_LOG_CMD_INIT:
        if (step == 1)    //
            JLOG(INFO, "\n**INIT INST=%d  start\n", inst);
        else
            JLOG(INFO, "\n**INIT INST=%d  end \n", inst);
        break;
    case JDI_LOG_CMD_RESET:
        if (step == 1)    //
            JLOG(INFO, "\n**RESET INST=%d  start\n", inst);
        else
            JLOG(INFO, "\n**RESET INST=%d  end \n", inst);
        break;
    case JDI_LOG_CMD_PAUSE_INST_CTRL:
        if (step == 1)    //
            JLOG(INFO, "\n**PAUSE INST_CTRL  INST=%d start\n", inst);
        else
            JLOG(INFO, "\n**PAUSE INST_CTRL  INST=%d end\n", inst);
        break;
    }

    for (i=(inst*NPT_REG_SIZE); i<=((inst*NPT_REG_SIZE)+0x250); i=i+16)
    {
        JLOG(INFO, "0x%04xh: 0x%08x 0x%08x 0x%08x 0x%08x\n", i,
            jdi_read_register(i), jdi_read_register(i+4),
            jdi_read_register(i+8), jdi_read_register(i+0xc));
    }

    JLOG(INFO, "0x%04xh: 0x%08x 0x%08x 0x%08x 0x%08x\n", NPT_PROC_BASE,
        jdi_read_register(NPT_PROC_BASE+0x00), jdi_read_register(NPT_PROC_BASE+4),
        jdi_read_register(NPT_PROC_BASE+8), jdi_read_register(NPT_PROC_BASE+0xc));
}


static void SwapByte(Uint8* data, Uint32 len)
{
    Uint8   temp;
    Uint32  i;

    for (i=0; i<len; i+=2) {
        temp      = data[i];
        data[i]   = data[i+1];
        data[i+1] = temp;
    }
}

static void SwapWord(Uint8* data, Uint32 len)
{
    Uint16  temp;
    Uint16* ptr = (Uint16*)data;
    Int32   i, size = len/sizeof(Uint16);

    for (i=0; i<size; i+=2) {
        temp      = ptr[i];
        ptr[i]   = ptr[i+1];
        ptr[i+1] = temp;
    }
}

static void SwapDword(Uint8* data, Uint32 len)
{
    Uint32  temp;
    Uint32* ptr = (Uint32*)data;
    Int32   i, size = len/sizeof(Uint32);

    for (i=0; i<size; i+=2) {
        temp      = ptr[i];
        ptr[i]   = ptr[i+1];
        ptr[i+1] = temp;
    }
}


Int32 swap_endian(BYTE* data, Uint32 len, Uint32 endian)
{
    Uint8   endianMask[8] = {   // endianMask : [2] - 4byte unit swap
        0x00, 0x07, 0x04, 0x03, //              [1] - 2byte unit swap
        0x06, 0x05, 0x02, 0x01  //              [0] - 1byte unit swap
    };
    Uint8   targetEndian;
    Uint8   systemEndian;
    Uint8   changes;
    BOOL    byteSwap=FALSE, wordSwap=FALSE, dwordSwap=FALSE;

    if (endian > 7) {
        JLOG(ERR, "Invalid endian mode: %d, expected value: 0~7\n", endian);
        return -1;
    }

    targetEndian = endianMask[endian];
    systemEndian = endianMask[JDI_SYSTEM_ENDIAN];
    changes      = targetEndian ^ systemEndian;
    byteSwap     = changes & 0x01 ? TRUE : FALSE;
    wordSwap     = changes & 0x02 ? TRUE : FALSE;
    dwordSwap    = changes & 0x04 ? TRUE : FALSE;

    if (byteSwap == TRUE)  SwapByte(data, len);
    if (wordSwap == TRUE)  SwapWord(data, len);
    if (dwordSwap == TRUE) SwapDword(data, len);

    return changes == 0 ? 0 : 1;
}




