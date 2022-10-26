//------------------------------------------------------------------------------
// File: vdi.c
//
// Copyright (c) 2006, Chips & Media.  All rights reserved.
//------------------------------------------------------------------------------
#if defined(linux) || defined(__linux) || defined(ANDROID)

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#ifdef	_KERNEL_
#include <linux/delay.h> 
#endif
#include <signal.h>		/* SIGIO */
#include <fcntl.h>		/* fcntl */
#include <pthread.h>
#include <sys/mman.h>		/* mmap */
#include <sys/ioctl.h>		/* fopen/fread */
#include <sys/errno.h>		/* fopen/fread */
#include <sys/types.h>
#include <sys/time.h>
#include "driver/vpu.h"
#include "../vdi.h"
#include "../vdi_osal.h"
#include "coda9/coda9_regdefine.h"
#include "wave/common/common_regdefine.h"
#include "wave/wave4/wave4_regdefine.h"
#include "wave/coda7q/coda7q_regdefine.h"

#define VPU_DEVICE_NAME "/dev/venc"

typedef pthread_mutex_t	MUTEX_HANDLE;


#	define SUPPORT_INTERRUPT
#	define VPU_BIT_REG_SIZE	(0x4000*MAX_NUM_VPU_CORE)
#		define VDI_SRAM_BASE_ADDR	        0x00000000	// if we can know the sram address in SOC directly for vdi layer. it is possible to set in vdi layer without allocation from driver
#	    define VDI_WAVE410_SRAM_SIZE		0x25000     // 8Kx8K MAIN10 MAX size
#	    define VDI_WAVE412_SRAM_SIZE		0x80000
#	    define VDI_WAVE512_SRAM_SIZE		0x80000
#	    define VDI_WAVE515_SRAM_SIZE		0x80000
#	    define VDI_WAVE520_SRAM_SIZE	    0x25000     // 8Kx8X MAIN10 MAX size
#	    define VDI_WAVE420_SRAM_SIZE		0x2E000     // 8Kx8X MAIN10 MAX size
#	    define VDI_WAVE420L_SRAM_SIZE		0x2E000     // 8Kx8X MAIN10 MAX size
#	    define VDI_CODA9_SRAM_SIZE			0x34600     // FHD MAX size, 0x17D00  4K MAX size 0x34600
#   define VDI_SYSTEM_ENDIAN				VDI_LITTLE_ENDIAN
#   define VDI_128BIT_BUS_SYSTEM_ENDIAN 	VDI_128BIT_LITTLE_ENDIAN
#define VDI_NUM_LOCK_HANDLES				4

#ifdef SUPPORT_MULTI_CORE_IN_ONE_DRIVER
#define VPU_CORE_BASE_OFFSET 0x4000
#endif

typedef struct vpudrv_buffer_pool_t
{
    vpudrv_buffer_t vdb;
    int inuse;
} vpudrv_buffer_pool_t;

typedef struct  {
    unsigned long core_idx;
    unsigned int product_code;
    int vpu_fd;	
    vpu_instance_pool_t *pvip;
    int task_num;
    int clock_state;	
    vpudrv_buffer_t vdb_register;
    vpu_buffer_t vpu_common_memory;
    vpudrv_buffer_pool_t vpu_buffer_pool[MAX_VPU_BUFFER_POOL];
    int vpu_buffer_pool_count;
	
    void* vpu_mutex;
    void* vpu_omx_mutex;
    void* vpu_disp_mutex;

} vdi_info_t;

static vdi_info_t s_vdi_info[MAX_NUM_VPU_CORE];

static int swap_endian(unsigned long core_idx, unsigned char *data, int len, int endian);
static int allocate_common_memory(unsigned long core_idx);

void vdi_flush_ddr(unsigned long core_idx,unsigned long start,unsigned long size,unsigned char flag)
{
    vdi_info_t *vdi;
    vpudrv_flush_cache_t cache_info;
    
    vdi = &s_vdi_info[core_idx];
    cache_info.start = start;
    cache_info.size = size;
    cache_info.flag = flag;

    ioctl(vdi->vpu_fd, VDI_IOCTL_FLUSH_DCACHE, &cache_info);
}

int vdi_probe(unsigned long core_idx)
{
    int ret;

    ret = vdi_init(core_idx);
    vdi_release(core_idx);
    return ret;
}

int vdi_init(unsigned long core_idx)
{
    vdi_info_t *vdi;
    int i;

    if (core_idx >= MAX_NUM_VPU_CORE)
        return 0;

    vdi = &s_vdi_info[core_idx];

    if (vdi->vpu_fd != -1 && vdi->vpu_fd != 0x00)
    {
        vdi->task_num++;
        return 0;
    }



    vdi->vpu_fd = open(VPU_DEVICE_NAME, O_RDWR);	// if this API supports VPU parallel processing using multi VPU. the driver should be made to open multiple times.
    if (vdi->vpu_fd < 0) {
        VLOG(ERR, "[VDI] Can't open vpu driver. [error=%s]. try to load vpu driver first \n", strerror(errno));
        return -1;
    }

    memset(&vdi->vpu_buffer_pool, 0x00, sizeof(vpudrv_buffer_pool_t)*MAX_VPU_BUFFER_POOL);

    if (!vdi_get_instance_pool(core_idx))
    {
        VLOG(INFO, "[VDI] fail to create shared info for saving context \n");		
        goto ERR_VDI_INIT;
    }

    if (vdi->pvip->instance_pool_inited == FALSE)
    {
        int* pCodecInst;
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
        pthread_mutex_init((MUTEX_HANDLE *)vdi->vpu_mutex, &mutexattr);
        pthread_mutex_init((MUTEX_HANDLE *)vdi->vpu_disp_mutex, &mutexattr);

        for( i = 0; i < MAX_NUM_INSTANCE; i++) {
            pCodecInst = (int *)vdi->pvip->codecInstPool[i];
            pCodecInst[1] = i;	// indicate instIndex of CodecInst
            pCodecInst[0] = 0;	// indicate inUse of CodecInst
        }

        vdi->pvip->instance_pool_inited = TRUE;
    }

#ifdef USE_VMALLOC_FOR_INSTANCE_POOL_MEMORY
    if (ioctl(vdi->vpu_fd, VDI_IOCTL_GET_REGISTER_INFO, &vdi->vdb_register) < 0)
    {
        VLOG(ERR, "[VDI] fail to get host interface register\n");
        goto ERR_VDI_INIT;
    }
#endif
#ifdef USE_VMALLOC_FOR_INSTANCE_POOL_MEMORY
    vdi->vdb_register.virt_addr = (unsigned long)mmap(NULL, vdi->vdb_register.size, PROT_READ | PROT_WRITE, MAP_SHARED, vdi->vpu_fd, vdi->vdb_register.phys_addr);
#else
    vdi->vdb_register.size = VPU_BIT_REG_SIZE;
    vdi->vdb_register.virt_addr = (unsigned long)mmap(NULL, vdi->vdb_register.size, PROT_READ | PROT_WRITE, MAP_SHARED, vdi->vpu_fd, 0);
#endif
    if ((void *)vdi->vdb_register.virt_addr == MAP_FAILED) 
    {
        VLOG(ERR, "[VDI] fail to map vpu registers \n");
        goto ERR_VDI_INIT;
    }
    VLOG(INFO, "[VDI] map vdb_register core_idx=%d, virtaddr=0x%lx, size=%d\n", core_idx, vdi->vdb_register.virt_addr, vdi->vdb_register.size);

    vdi_set_clock_gate(core_idx, 1);


    vdi->product_code = vdi_read_register(core_idx, VPU_PRODUCT_CODE_REGISTER);

    if (PRODUCT_CODE_W_SERIES(vdi->product_code))
    {
        if (vdi_read_register(core_idx, W4_VCPU_CUR_PC) == 0) // if BIT processor is not running.
        {
            for (i=0; i<64; i++)
                vdi_write_register(core_idx, (i*4) + 0x100, 0x0); 
        }
    }
    else if (PRODUCT_CODE_NOT_W_SERIES(vdi->product_code)) // CODA9XX
    {
        if (vdi_read_register(core_idx, BIT_CUR_PC) == 0) // if BIT processor is not running.
        {
            for (i=0; i<64; i++)
                vdi_write_register(core_idx, (i*4) + 0x100, 0x0); 
        }
    }
    else {
        VLOG(ERR, "Unknown product id : %08x\n", vdi->product_code);
        goto ERR_VDI_INIT;
    }

    if (vdi_lock(core_idx) < 0)
    {
        VLOG(ERR, "[VDI] fail to handle lock function\n");
        goto ERR_VDI_INIT;
    }

    if (allocate_common_memory(core_idx) < 0) 
    {
        VLOG(ERR, "[VDI] fail to get vpu common buffer from driver\n");		
        goto ERR_VDI_INIT;
    }

    vdi->core_idx = core_idx; 
    vdi->task_num++;	
    vdi_unlock(core_idx);

    VLOG(INFO, "[VDI] success to init driver \n");	
    return 0;

ERR_VDI_INIT:
    vdi_unlock(core_idx);
    vdi_release(core_idx);
    return -1;
}

int vdi_set_bit_firmware_to_pm(unsigned long core_idx, const unsigned short *code)
{
    int i;
    vpu_bit_firmware_info_t bit_firmware_info;
    vdi_info_t *vdi;

    if (core_idx >= MAX_NUM_VPU_CORE)
        return 0;

    vdi = &s_vdi_info[core_idx];

    if (!vdi || vdi->vpu_fd == -1 || vdi->vpu_fd == 0x00)
        return 0;	

    bit_firmware_info.size = sizeof(vpu_bit_firmware_info_t);	
    bit_firmware_info.core_idx = core_idx;
#ifdef SUPPORT_MULTI_CORE_IN_ONE_DRIVER	
    bit_firmware_info.reg_base_offset = (core_idx*VPU_CORE_BASE_OFFSET);
#else
    bit_firmware_info.reg_base_offset = 0;
#endif	
    for (i=0; i<512; i++) 
        bit_firmware_info.bit_code[i] = code[i];

    if (write(vdi->vpu_fd, &bit_firmware_info, bit_firmware_info.size) < 0)
    {
        VLOG(ERR, "[VDI] fail to vdi_set_bit_firmware core=%d\n", bit_firmware_info.core_idx);
        return -1;
    }

    return 0;
}


int vdi_release(unsigned long core_idx)
{
    int i;
    vpudrv_buffer_t vdb;
    vdi_info_t *vdi;

    if (core_idx >= MAX_NUM_VPU_CORE)
        return 0;

    vdi = &s_vdi_info[core_idx];

    if (!vdi || vdi->vpu_fd == -1 || vdi->vpu_fd == 0x00)
        return 0;

    if (vdi_lock(core_idx) < 0)
    {
        VLOG(ERR, "[VDI] fail to handle lock function\n");
        return -1;
    }

    if (vdi->task_num > 1) // means that the opened instance remains 
    {
        vdi->task_num--;
        vdi_unlock(core_idx);
        return 0;
    }


    if (vdi->vdb_register.virt_addr)
        munmap((void *)vdi->vdb_register.virt_addr, vdi->vdb_register.size);

    osal_memset(&vdi->vdb_register, 0x00, sizeof(vpudrv_buffer_t));
    vdb.size = 0;	
    // get common memory information to free virtual address
    for (i=0; i<MAX_VPU_BUFFER_POOL; i++)
    {
        if (vdi->vpu_common_memory.phys_addr >= vdi->vpu_buffer_pool[i].vdb.phys_addr &&
            vdi->vpu_common_memory.phys_addr < (vdi->vpu_buffer_pool[i].vdb.phys_addr + vdi->vpu_buffer_pool[i].vdb.size))
        {
            vdi->vpu_buffer_pool[i].inuse = 0;
            vdi->vpu_buffer_pool_count--;
            vdb = vdi->vpu_buffer_pool[i].vdb;
            break;
        }
    }

    vdi_unlock(core_idx);

    if (vdb.size > 0)
    {
        munmap((void *)vdb.virt_addr, vdb.size);
        memset(&vdi->vpu_common_memory, 0x00, sizeof(vpu_buffer_t));
    }

    vdi->task_num--;

    if (vdi->vpu_fd != -1 && vdi->vpu_fd != 0x00)
    {
        close(vdi->vpu_fd);
        vdi->vpu_fd = -1;

    }

    memset(vdi, 0x00, sizeof(vdi_info_t));

    return 0;
}

int vdi_get_common_memory(unsigned long core_idx, vpu_buffer_t *vb)
{
    vdi_info_t *vdi;

    if (core_idx >= MAX_NUM_VPU_CORE)
        return -1;

    vdi = &s_vdi_info[core_idx];

    if(!vdi || vdi->vpu_fd==-1 || vdi->vpu_fd==0x00)
        return -1;

    osal_memcpy(vb, &vdi->vpu_common_memory, sizeof(vpu_buffer_t));

    return 0;
}

int allocate_common_memory(unsigned long core_idx)
{
    vdi_info_t *vdi = &s_vdi_info[core_idx];
    vpudrv_buffer_t vdb;
    int i;

    if (core_idx >= MAX_NUM_VPU_CORE)
        return -1;

    if(!vdi || vdi->vpu_fd==-1 || vdi->vpu_fd==0x00)
        return -1;

    osal_memset(&vdb, 0x00, sizeof(vpudrv_buffer_t));

    vdb.size = SIZE_COMMON*MAX_NUM_VPU_CORE;
    if (ioctl(vdi->vpu_fd, VDI_IOCTL_GET_COMMON_MEMORY, &vdb) < 0)
    {
        VLOG(ERR, "[VDI] fail to vdi_allocate_dma_memory size=%d\n", vdb.size);
        return -1;
    }

   // vdb.virt_addr = (unsigned long)mmap(NULL, vdb.size, PROT_READ | PROT_WRITE, MAP_SHARED, vdi->vpu_fd, vdb.phys_addr);
   vdb.virt_addr = (unsigned long)mmap(NULL, vdb.size, PROT_READ | PROT_WRITE, MAP_SHARED, vdi->vpu_fd, DRAM_MEM2SYS(vdb.phys_addr));
    if ((void *)vdb.virt_addr == MAP_FAILED) 
    {
        VLOG(ERR, "[VDI] fail to map common memory phyaddr=0x%lx, size = %d\n", (int)vdb.phys_addr, (int)vdb.size);
        return -1;
    }

    VLOG(INFO, "[VDI] allocate_common_memory, physaddr=0x%lx, virtaddr=0x%lx\n", (int)vdb.phys_addr, (int)vdb.virt_addr);

    // convert os driver buffer type to vpu buffer type
#ifdef SUPPORT_MULTI_CORE_IN_ONE_DRIVER
    vdi->pvip->vpu_common_buffer.size = SIZE_COMMON;
    vdi->pvip->vpu_common_buffer.phys_addr = (unsigned long)(vdb.phys_addr + (core_idx*SIZE_COMMON));
    vdi->pvip->vpu_common_buffer.base = (unsigned long)(vdb.base + (core_idx*SIZE_COMMON));
    vdi->pvip->vpu_common_buffer.virt_addr = (unsigned long)(vdb.virt_addr + (core_idx*SIZE_COMMON));
#else
    vdi->pvip->vpu_common_buffer.size = SIZE_COMMON;
    vdi->pvip->vpu_common_buffer.phys_addr = (unsigned long)(vdb.phys_addr);
    vdi->pvip->vpu_common_buffer.base = (unsigned long)(vdb.base);
    vdi->pvip->vpu_common_buffer.virt_addr = (unsigned long)(vdb.virt_addr);
#endif

    osal_memcpy(&vdi->vpu_common_memory, &vdi->pvip->vpu_common_buffer, sizeof(vpudrv_buffer_t));

    for (i=0; i<MAX_VPU_BUFFER_POOL; i++)
    {
        if (vdi->vpu_buffer_pool[i].inuse == 0)
        {
            vdi->vpu_buffer_pool[i].vdb = vdb;
            vdi->vpu_buffer_pool_count++;
            vdi->vpu_buffer_pool[i].inuse = 1;
            break;
        }
    }

    VLOG(INFO, "[VDI] vdi_get_common_memory physaddr=0x%lx, size=%d, virtaddr=0x%lx\n", (int)vdi->vpu_common_memory.phys_addr, (int)vdi->vpu_common_memory.size, (int)vdi->vpu_common_memory.virt_addr);

    return 0;
}

vpu_instance_pool_t *vdi_get_instance_pool(unsigned long core_idx)
{
    vdi_info_t *vdi;
    vpudrv_buffer_t vdb;

    if (core_idx >= MAX_NUM_VPU_CORE)
        return NULL;

    vdi = &s_vdi_info[core_idx];

    if(!vdi || vdi->vpu_fd == -1 || vdi->vpu_fd == 0x00 )
        return NULL;

    osal_memset(&vdb, 0x00, sizeof(vpudrv_buffer_t));
    if (!vdi->pvip)
    {
        vdb.size = sizeof(vpu_instance_pool_t) + sizeof(MUTEX_HANDLE)*VDI_NUM_LOCK_HANDLES;
#ifdef SUPPORT_MULTI_CORE_IN_ONE_DRIVER
        vdb.size  *= MAX_NUM_VPU_CORE;
#endif
        if (ioctl(vdi->vpu_fd, VDI_IOCTL_GET_INSTANCE_POOL, &vdb) < 0)
        {
            VLOG(ERR, "[VDI] fail to allocate get instance pool physical space=%d\n", (int)vdb.size);
            return NULL;
        }

#ifdef USE_VMALLOC_FOR_INSTANCE_POOL_MEMORY
		vdb.virt_addr = (unsigned long)mmap(NULL, vdb.size, PROT_READ | PROT_WRITE, MAP_SHARED, vdi->vpu_fd, 0);
#else
        vdb.virt_addr = (unsigned long)mmap(NULL, vdb.size, PROT_READ | PROT_WRITE, MAP_SHARED, vdi->vpu_fd, vdb.phys_addr);
#endif
        if ((void *)vdb.virt_addr == MAP_FAILED) 
        {
            VLOG(ERR, "[VDI] fail to map instance pool phyaddr=0x%lx, size = %d\n", (int)vdb.phys_addr, (int)vdb.size);
            return NULL;
        }

#ifdef SUPPORT_MULTI_CORE_IN_ONE_DRIVER
        vdi->pvip = (vpu_instance_pool_t *)(vdb.virt_addr + (core_idx*(sizeof(vpu_instance_pool_t) + sizeof(MUTEX_HANDLE)*VDI_NUM_LOCK_HANDLES)));
#else
        vdi->pvip = (vpu_instance_pool_t *)(vdb.virt_addr);
#endif
        vdi->vpu_mutex =      (void *)((unsigned long)vdi->pvip + sizeof(vpu_instance_pool_t));	//change the pointer of vpu_mutex to at end pointer of vpu_instance_pool_t to assign at allocated position.
        vdi->vpu_disp_mutex = (void *)((unsigned long)vdi->pvip + sizeof(vpu_instance_pool_t) + sizeof(MUTEX_HANDLE));		

        VLOG(INFO, "[VDI] instance pool physaddr=0x%lx, virtaddr=0x%lx, base=0x%lx, size=%ld\n", (int)vdb.phys_addr, (int)vdb.virt_addr, (int)vdb.base, (int)vdb.size);
    }

    return (vpu_instance_pool_t *)vdi->pvip;
}

int vdi_open_instance(unsigned long core_idx, unsigned long inst_idx)
{
    vdi_info_t *vdi;
    vpudrv_inst_info_t inst_info;
    
    if (core_idx >= MAX_NUM_VPU_CORE)
        return -1;

    vdi = &s_vdi_info[core_idx];

    if(!vdi || vdi->vpu_fd == -1 || vdi->vpu_fd == 0x00)
        return -1;

    inst_info.core_idx = core_idx;
    inst_info.inst_idx = inst_idx;
    if (ioctl(vdi->vpu_fd, VDI_IOCTL_OPEN_INSTANCE, &inst_info) < 0)
    {
        VLOG(ERR, "[VDI] fail to deliver open instance num inst_idx=%d\n", (int)inst_idx);
        return -1;
    }

    vdi->pvip->vpu_instance_num = inst_info.inst_open_count;

    return 0;
}

int vdi_close_instance(unsigned long core_idx, unsigned long inst_idx)
{
    vdi_info_t *vdi;
    vpudrv_inst_info_t inst_info;

    if (core_idx >= MAX_NUM_VPU_CORE)
        return -1;

    vdi = &s_vdi_info[core_idx];

    if(!vdi || vdi->vpu_fd == -1 || vdi->vpu_fd == 0x00)
        return -1;

    inst_info.core_idx = core_idx;
    inst_info.inst_idx = inst_idx;
    if (ioctl(vdi->vpu_fd, VDI_IOCTL_CLOSE_INSTANCE, &inst_info) < 0)
    {
        VLOG(ERR, "[VDI] fail to deliver open instance num inst_idx=%d\n", (int)inst_idx);
        return -1;
    }

    vdi->pvip->vpu_instance_num = inst_info.inst_open_count;

    return 0;
}

int vdi_get_instance_num(unsigned long core_idx)
{
    vdi_info_t *vdi;

    if (core_idx >= MAX_NUM_VPU_CORE)
        return -1;

    vdi = &s_vdi_info[core_idx];

    if(!vdi || vdi->vpu_fd == -1 || vdi->vpu_fd == 0x00)
        return -1;

    return vdi->pvip->vpu_instance_num;
}

int vdi_hw_reset(unsigned long core_idx) // DEVICE_ADDR_SW_RESET
{
    vdi_info_t *vdi;

    if (core_idx >= MAX_NUM_VPU_CORE)
        return -1;

    vdi = &s_vdi_info[core_idx];

    if(!vdi || vdi->vpu_fd == -1 || vdi->vpu_fd == 0x00)
        return -1;

    return ioctl(vdi->vpu_fd, VDI_IOCTL_RESET, 0);

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

int vdi_lock(unsigned long core_idx)
{
    vdi_info_t *vdi;
#if defined(ANDROID) || !defined(PTHREAD_MUTEX_ROBUST_NP)
#else
    const int MUTEX_TIMEOUT = 0x7fffffff;
#endif

    if (core_idx >= MAX_NUM_VPU_CORE)
        return -1;

    vdi = &s_vdi_info[core_idx];

    if(!vdi || vdi->vpu_fd == -1 || vdi->vpu_fd == 0x00)
        return -1;
#if defined(ANDROID) || !defined(PTHREAD_MUTEX_ROBUST_NP)
	restore_mutex_in_dead((MUTEX_HANDLE *)vdi->vpu_mutex);
	pthread_mutex_lock((MUTEX_HANDLE*)vdi->vpu_mutex);
#else
    if (pthread_mutex_lock((MUTEX_HANDLE *)vdi->vpu_mutex) != 0) {
        VLOG(ERR, "%s:%d failed to pthread_mutex_locK\n", __FUNCTION__, __LINE__);
        return -1;
    }
#endif

    return 0;
}

int vdi_lock_check(unsigned long core_idx)
{
    vdi_info_t *vdi;
    int ret;

    if (core_idx >= MAX_NUM_VPU_CORE)
        return -1;

    vdi = &s_vdi_info[core_idx];

    if(!vdi || vdi->vpu_fd == -1 || vdi->vpu_fd == 0x00)
        return -1;

    ret = pthread_mutex_trylock((MUTEX_HANDLE *)vdi->vpu_mutex);
    if(ret == 0) 
    {
        vdi_unlock(core_idx);
        return -1;
    }
    else
    {
        return 0;
    }
}

void vdi_unlock(unsigned long core_idx)
{
    vdi_info_t *vdi;

    if (core_idx >= MAX_NUM_VPU_CORE)
        return;

    vdi = &s_vdi_info[core_idx];

    if(!vdi || vdi->vpu_fd == -1 || vdi->vpu_fd == 0x00)
        return;

    pthread_mutex_unlock((MUTEX_HANDLE *)vdi->vpu_mutex);
}

int vdi_disp_lock(unsigned long core_idx)
{
    vdi_info_t *vdi;
#if defined(ANDROID) || !defined(PTHREAD_MUTEX_ROBUST_NP)
#else
    const int MUTEX_TIMEOUT = 5000;  // ms
#endif

    if (core_idx >= MAX_NUM_VPU_CORE)
        return -1;

    vdi = &s_vdi_info[core_idx];

    if(!vdi || vdi->vpu_fd == -1 || vdi->vpu_fd == 0x00)
        return -1;
#if defined(ANDROID) || !defined(PTHREAD_MUTEX_ROBUST_NP)
	restore_mutex_in_dead((MUTEX_HANDLE *)vdi->vpu_disp_mutex);
	pthread_mutex_lock((MUTEX_HANDLE*)vdi->vpu_disp_mutex);
#else
    if (pthread_mutex_lock((MUTEX_HANDLE *)vdi->vpu_disp_mutex) != 0) {
        VLOG(ERR, "%s:%d failed to pthread_mutex_lock\n", __FUNCTION__, __LINE__);
        return -1;
    }

#endif /* ANDROID */

    return 0;
}

void vdi_disp_unlock(unsigned long core_idx)
{
    vdi_info_t *vdi;

    if (core_idx >= MAX_NUM_VPU_CORE)
        return;

    vdi = &s_vdi_info[core_idx];

    if(!vdi || vdi->vpu_fd == -1 || vdi->vpu_fd == 0x00)
        return;

    pthread_mutex_unlock((MUTEX_HANDLE *)vdi->vpu_disp_mutex);
}

void vdi_write_register(unsigned long core_idx, unsigned int addr, unsigned int data)
{
    vdi_info_t *vdi;
    unsigned long *reg_addr;

    if (core_idx >= MAX_NUM_VPU_CORE)
        return;

    vdi = &s_vdi_info[core_idx];

    if(!vdi || vdi->vpu_fd == -1 || vdi->vpu_fd == 0x00)
        return;


#ifdef SUPPORT_MULTI_CORE_IN_ONE_DRIVER
    reg_addr = (unsigned long *)(addr + (unsigned long)vdi->vdb_register.virt_addr + (core_idx*VPU_CORE_BASE_OFFSET));
#else
    reg_addr = (unsigned long *)(addr + (unsigned long)vdi->vdb_register.virt_addr);
#endif
    *(volatile unsigned int *)reg_addr = data;	
}

unsigned int vdi_read_register(unsigned long core_idx, unsigned int addr)
{
    vdi_info_t *vdi;
	unsigned long *reg_addr;

    if (core_idx >= MAX_NUM_VPU_CORE)
        return (unsigned int)-1;

    vdi = &s_vdi_info[core_idx];

    if(!vdi || vdi->vpu_fd == -1 || vdi->vpu_fd == 0x00)
        return (unsigned int)-1;


#ifdef SUPPORT_MULTI_CORE_IN_ONE_DRIVER
    reg_addr = (unsigned long *)(addr + (unsigned long)vdi->vdb_register.virt_addr + (core_idx*VPU_CORE_BASE_OFFSET));	
#else
    reg_addr = (unsigned long *)(addr + (unsigned long)vdi->vdb_register.virt_addr);
#endif
    return *(volatile unsigned int *)reg_addr;
}

#define FIO_TIMEOUT         100

unsigned int vdi_fio_read_register(unsigned long core_idx, unsigned int addr)
{
    unsigned int ctrl;
    unsigned int count = 0;
    unsigned int data  = 0xffffffff;

    ctrl  = (addr&0xffff);
    ctrl |= (0<<16);    /* read operation */
    vdi_write_register(core_idx, W4_VPU_FIO_CTRL_ADDR, ctrl);
    count = FIO_TIMEOUT;
    while (count--) {
        ctrl = vdi_read_register(core_idx, W4_VPU_FIO_CTRL_ADDR);
        if (ctrl & 0x80000000) {
            data = vdi_read_register(core_idx, W4_VPU_FIO_DATA);
            break;
        }
    }

    return data;
}

void vdi_fio_write_register(unsigned long core_idx, unsigned int addr, unsigned int data)
{
    unsigned int ctrl;
    
    vdi_write_register(core_idx, W4_VPU_FIO_DATA, data);
    ctrl  = (addr&0xffff);
    ctrl |= (1<<16);    /* write operation */
    vdi_write_register(core_idx, W4_VPU_FIO_CTRL_ADDR, ctrl);
}

#define VCORE_DBG_ADDR(__vCoreIdx)      0x8000+(0x1000*__vCoreIdx) + 0x300
#define VCORE_DBG_DATA(__vCoreIdx)      0x8000+(0x1000*__vCoreIdx) + 0x304
#define VCORE_DBG_READY(__vCoreIdx)     0x8000+(0x1000*__vCoreIdx) + 0x308

static void UNREFERENCED_FUNCTION write_vce_register(
    unsigned int   core_idx,
    unsigned int   vce_core_idx,
    unsigned int   vce_addr,
    unsigned int   udata
    )
{
    int vcpu_reg_addr;

    vdi_fio_write_register(core_idx, VCORE_DBG_READY(vce_core_idx),0);

    vcpu_reg_addr = vce_addr >> 2;

    vdi_fio_write_register(core_idx, VCORE_DBG_DATA(vce_core_idx),udata);
    vdi_fio_write_register(core_idx, VCORE_DBG_ADDR(vce_core_idx),(vcpu_reg_addr) & 0x00007FFF);

    while (vdi_fio_read_register(0, VCORE_DBG_READY(vce_core_idx)) < 0) {
        VLOG(ERR, "failed to write VCE register: 0x%04x\n", vce_addr);
    }
}

static unsigned int read_vce_register(
    unsigned int core_idx,
    unsigned int vce_core_idx,
    unsigned int vce_addr
    )
{
    int     vcpu_reg_addr;
    int     udata;
    int     vce_core_base = 0x8000 + 0x1000*vce_core_idx;

    vdi_fio_write_register(core_idx, VCORE_DBG_READY(vce_core_idx), 0);

    vcpu_reg_addr = vce_addr >> 2;

    vdi_fio_write_register(core_idx, VCORE_DBG_ADDR(vce_core_idx),vcpu_reg_addr + vce_core_base);

    while (TRUE) {
        if (vdi_fio_read_register(0, VCORE_DBG_READY(vce_core_idx)) == 1) {
            udata= vdi_fio_read_register(0, VCORE_DBG_DATA(vce_core_idx));
            break;
        }
    }

    return udata;
}


int vdi_clear_memory(unsigned long core_idx, unsigned int addr, int len, int endian)
{
    vdi_info_t *vdi;
    vpudrv_buffer_t vdb;
	unsigned long offset;
    
    int i;
    Uint8*  zero;

#ifdef SUPPORT_MULTI_CORE_IN_ONE_DRIVER
    core_idx = 0;
#endif

    if (core_idx >= MAX_NUM_VPU_CORE)
        return -1;

    vdi = &s_vdi_info[core_idx];

    if(!vdi || vdi->vpu_fd == -1 || vdi->vpu_fd == 0x00)
        return -1;

    osal_memset(&vdb, 0x00, sizeof(vpudrv_buffer_t));

    for (i=0; i<MAX_VPU_BUFFER_POOL; i++)
    {
        if (vdi->vpu_buffer_pool[i].inuse == 1)
        {
            vdb = vdi->vpu_buffer_pool[i].vdb;
            if (addr >= vdb.phys_addr && addr < (vdb.phys_addr + vdb.size))
                break;
        }
    }

    if (!vdb.size) {
        VLOG(ERR, "address 0x%08x is not mapped address!!!\n", (int)addr);
        return -1;
    }

    zero = (Uint8*)osal_malloc(len);
    osal_memset((void*)zero, 0x00, len);

	offset = addr - (unsigned long)vdb.phys_addr;
	vdi_flush_ddr(core_idx,(unsigned long )(vdb.phys_addr+offset),len,1);  //invalid cache before clear
    osal_memcpy((void *)((unsigned long)vdb.virt_addr+offset), zero, len);	

    osal_free(zero);

    return len;
}

void vdi_set_sdram(unsigned long coreIdx, unsigned int addr, int len, unsigned char data, int endian)
{

	vdi_info_t *vdi = &s_vdi_info[coreIdx];
	unsigned char *buf;

	if(!vdi || vdi->vpu_fd==-1 || vdi->vpu_fd == 0x00)
		return ;

	buf = (unsigned char *)osal_malloc(len);
	memset(buf, 0x00, len);
	vdi_write_memory(coreIdx, addr, buf, len, endian);
	free(buf);
}

int vdi_write_memory(unsigned long core_idx, unsigned int addr, unsigned char *data, int len, int endian)
{
    vdi_info_t *vdi;
    vpudrv_buffer_t vdb;
    unsigned long offset;
    int i;

#ifdef SUPPORT_MULTI_CORE_IN_ONE_DRIVER
    core_idx = 0;
#endif

    if (core_idx >= MAX_NUM_VPU_CORE)
        return -1;

    vdi = &s_vdi_info[core_idx];

    if(!vdi || vdi->vpu_fd==-1 || vdi->vpu_fd == 0x00)
        return -1;

    osal_memset(&vdb, 0x00, sizeof(vpudrv_buffer_t));

    for (i=0; i<MAX_VPU_BUFFER_POOL; i++)
    {
        if (vdi->vpu_buffer_pool[i].inuse == 1)
        {
            vdb = vdi->vpu_buffer_pool[i].vdb;
            if (addr >= vdb.phys_addr && addr < (vdb.phys_addr + vdb.size)) {
                break;
            }
        }
    }

    if (!vdb.size) {
        VLOG(ERR, "address 0x%08x is not mapped address!!!\n", (int)addr);
        return -1;
    }

    
	offset = addr - (unsigned long)vdb.phys_addr;
    swap_endian(core_idx, data, len, endian);
    osal_memcpy((void *)((unsigned long)vdb.virt_addr+offset), data, len);	

    return len;
}

int vdi_read_memory(unsigned long core_idx, unsigned int addr, unsigned char *data, int len, int endian)
{
    vdi_info_t *vdi;
    vpudrv_buffer_t vdb;
    unsigned long offset;
    int i;

#ifdef SUPPORT_MULTI_CORE_IN_ONE_DRIVER
    core_idx = 0;
#endif
    if (core_idx >= MAX_NUM_VPU_CORE)
        return -1;

    vdi = &s_vdi_info[core_idx];

    if(!vdi || vdi->vpu_fd==-1 || vdi->vpu_fd == 0x00)
        return -1;

    osal_memset(&vdb, 0x00, sizeof(vpudrv_buffer_t));

    for (i=0; i<MAX_VPU_BUFFER_POOL; i++)
    {
        if (vdi->vpu_buffer_pool[i].inuse == 1)
        {
            vdb = vdi->vpu_buffer_pool[i].vdb;
            if (addr >= vdb.phys_addr && addr < (vdb.phys_addr + vdb.size))
                break;		
        }
    }

    if (!vdb.size)
    {
        return -1;
    }

	offset = addr - (unsigned long)vdb.phys_addr;	
    osal_memcpy(data, (const void *)((unsigned long)vdb.virt_addr+offset), len);
    swap_endian(core_idx, data, len,  endian);

    return len;
}

int vdi_allocate_dma_memory(unsigned long core_idx, vpu_buffer_t *vb)
{
    vdi_info_t *vdi;
    int i;
    vpudrv_buffer_t vdb;

#ifdef SUPPORT_MULTI_CORE_IN_ONE_DRIVER
    core_idx = 0;
#endif
    if (core_idx >= MAX_NUM_VPU_CORE)
        return -1;

    vdi = &s_vdi_info[core_idx];

    if(!vdi || vdi->vpu_fd==-1 || vdi->vpu_fd == 0x00)
        return -1;	

    osal_memset(&vdb, 0x00, sizeof(vpudrv_buffer_t));

    vdb.size = vb->size;

    if (ioctl(vdi->vpu_fd, VDI_IOCTL_ALLOCATE_PHYSICAL_MEMORY, &vdb) < 0)
    {
        VLOG(ERR, "[VDI] fail to vdi_allocate_dma_memory size=%d\n", vb->size);		
        return -1;
    }

    vb->phys_addr = (unsigned long)vdb.phys_addr;
    vb->base = (unsigned long)vdb.base;

    //map to virtual address
    //vdb.virt_addr = (unsigned long)mmap(NULL, vdb.size, PROT_READ | PROT_WRITE,
    //    MAP_SHARED, vdi->vpu_fd, vdb.phys_addr);
	vdb.virt_addr = (unsigned long)mmap(NULL, vdb.size, PROT_READ | PROT_WRITE,
        MAP_SHARED, vdi->vpu_fd, DRAM_MEM2SYS(vdb.phys_addr));
    if ((void *)vdb.virt_addr == MAP_FAILED) 
    {
        memset(vb, 0x00, sizeof(vpu_buffer_t));
        return -1;
    }
    vb->virt_addr = vdb.virt_addr;

    for (i=0; i<MAX_VPU_BUFFER_POOL; i++)
    {
        if (vdi->vpu_buffer_pool[i].inuse == 0)
        {
            vdi->vpu_buffer_pool[i].vdb = vdb;
            vdi->vpu_buffer_pool_count++;
            vdi->vpu_buffer_pool[i].inuse = 1;
            break;
        }
    }
	VLOG(INFO, "[VDI] vdi_allocate_dma_memory, physaddr=%p, virtaddr=%p~%p, size=%d\n", 
        vb->phys_addr, vb->virt_addr, vb->virt_addr + vb->size, vb->size);
    return 0;
}

int vdi_attach_dma_memory(unsigned long core_idx, vpu_buffer_t *vb)
{
    vdi_info_t *vdi;
    int i;
    vpudrv_buffer_t vdb;

#ifdef SUPPORT_MULTI_CORE_IN_ONE_DRIVER
    core_idx = 0;
#endif

    if (core_idx >= MAX_NUM_VPU_CORE)
        return -1;

    vdi = &s_vdi_info[core_idx];

    if(!vdi || vdi->vpu_fd==-1 || vdi->vpu_fd == 0x00)
        return -1;	

    osal_memset(&vdb, 0x00, sizeof(vpudrv_buffer_t));

    vdb.size = vb->size;
    vdb.phys_addr = vb->phys_addr;
    vdb.base = vb->base;

    vdb.virt_addr = vb->virt_addr;

    for (i=0; i<MAX_VPU_BUFFER_POOL; i++)
    {
        if (vdi->vpu_buffer_pool[i].vdb.phys_addr == vb->phys_addr)
        {
            vdi->vpu_buffer_pool[i].vdb = vdb;
            vdi->vpu_buffer_pool[i].inuse = 1;
            break;
        }
        else
        {
            if (vdi->vpu_buffer_pool[i].inuse == 0)
            {
                vdi->vpu_buffer_pool[i].vdb = vdb;
                vdi->vpu_buffer_pool_count++;
                vdi->vpu_buffer_pool[i].inuse = 1;
                break;
            }
        }			
    }

    //VLOG(INFO, "[VDI] vdi_attach_dma_memory, physaddr=0x%lx, virtaddr=0x%lx, size=%d, index=%d\n", vb->phys_addr, vb->virt_addr, vb->size, i);

    return 0;
}

int vdi_dettach_dma_memory(unsigned long core_idx, vpu_buffer_t *vb)
{
    vdi_info_t *vdi;
    int i;

#ifdef SUPPORT_MULTI_CORE_IN_ONE_DRIVER
    core_idx = 0;
#endif

    if (core_idx >= MAX_NUM_VPU_CORE)
        return -1;

    vdi = &s_vdi_info[core_idx];

    if(!vb || !vdi || vdi->vpu_fd==-1 || vdi->vpu_fd == 0x00)
        return -1;

    if (vb->size == 0)
        return -1;

    for (i=0; i<MAX_VPU_BUFFER_POOL; i++)
    {
        if (vdi->vpu_buffer_pool[i].vdb.phys_addr == vb->phys_addr)
        {
            vdi->vpu_buffer_pool[i].inuse = 0;
            vdi->vpu_buffer_pool_count--;
            break;
        }
    }

    return 0;
}

void vdi_free_dma_memory(unsigned long core_idx, vpu_buffer_t *vb)
{
    vdi_info_t *vdi;
    int i;
    vpudrv_buffer_t vdb;

#ifdef SUPPORT_MULTI_CORE_IN_ONE_DRIVER
    core_idx = 0;
#endif

    if (core_idx >= MAX_NUM_VPU_CORE)
        return;

    vdi = &s_vdi_info[core_idx];

    if(!vb || !vdi || vdi->vpu_fd==-1 || vdi->vpu_fd == 0x00)
        return;

    if (vb->size == 0)
        return ;

    osal_memset(&vdb, 0x00, sizeof(vpudrv_buffer_t));

    for (i=0; i<MAX_VPU_BUFFER_POOL; i++)
    {
        if (vdi->vpu_buffer_pool[i].vdb.phys_addr == vb->phys_addr)
        {
            vdi->vpu_buffer_pool[i].inuse = 0;
            vdi->vpu_buffer_pool_count--;
            vdb = vdi->vpu_buffer_pool[i].vdb;
            break;
        }
    }

    if (!vdb.size)
    {
        VLOG(ERR, "[VDI] invalid buffer to free address = 0x%lx\n", (int)vdb.virt_addr);
        return ;
    }
    ioctl(vdi->vpu_fd, VDI_IOCTL_FREE_PHYSICALMEMORY, &vdb);

    if (munmap((void *)vdb.virt_addr, vdb.size) != 0)
    {
        VLOG(ERR, "[VDI] fail to vdi_free_dma_memory virtial address = 0x%lx\n", (int)vdb.virt_addr);					
    }
    osal_memset(vb, 0, sizeof(vpu_buffer_t));
}

int vdi_get_sram_memory(unsigned long core_idx, vpu_buffer_t *vb)
{
    vdi_info_t *vdi = NULL;
    vpudrv_buffer_t vdb;
    unsigned int sram_size = 0;

    if (core_idx >= MAX_NUM_VPU_CORE)
        return -1;

    vdi = &s_vdi_info[core_idx];

    if(!vb || !vdi || vdi->vpu_fd==-1 || vdi->vpu_fd == 0x00)
        return -1;

    osal_memset(&vdb, 0x00, sizeof(vpudrv_buffer_t));

    switch (vdi->product_code) {
    case WAVE410_CODE:
    case WAVE4102_CODE:
    case WAVE510_CODE:
        sram_size = VDI_WAVE410_SRAM_SIZE; break;
    case WAVE420_CODE:
        sram_size = VDI_WAVE420_SRAM_SIZE; break;
    case WAVE420L_CODE:
        sram_size = VDI_WAVE420L_SRAM_SIZE; break;
    case BODA7503_CODE:
    case CODA7542_CODE:
    case BODA950_CODE:
    case CODA960_CODE:
    case CODA980_CODE:
    case WAVE320_CODE:
    case CODA7Q_CODE:
        sram_size = VDI_CODA9_SRAM_SIZE; break;
    case WAVE412_CODE:
        sram_size = VDI_WAVE412_SRAM_SIZE; break;
    case WAVE512_CODE:
        sram_size = VDI_WAVE512_SRAM_SIZE; break;
    case WAVE515_CODE:
        sram_size = VDI_WAVE515_SRAM_SIZE; break;
    case WAVE520_CODE:
        sram_size = VDI_WAVE520_SRAM_SIZE; break;
    default:
        VLOG(ERR, "Check SRAM_SIZE(%d)\n", vdi->product_code);
        break;
    }

    if (sram_size > 0)	// if we can know the sram address directly in vdi layer, we use it first for sdram address
    {
        vb->phys_addr = VDI_SRAM_BASE_ADDR+(core_idx*sram_size);    // HOST can set DRAM base addr to VDI_SRAM_BASE_ADDR.
        vb->size = sram_size;

        return 0;
    }

    return 0;
}

int vdi_set_clock_gate(unsigned long core_idx, int enable)
{
    vdi_info_t *vdi = NULL;
    int ret;

    if (core_idx >= MAX_NUM_VPU_CORE)
        return -1;
    vdi = &s_vdi_info[core_idx];
    if(!vdi || vdi->vpu_fd==-1 || vdi->vpu_fd == 0x00)
        return -1;

    if (vdi->product_code == WAVE510_CODE || vdi->product_code == WAVE512_CODE || vdi->product_code == WAVE515_CODE) {
        return 0;
    }
    vdi->clock_state = enable;
    ret = ioctl(vdi->vpu_fd, VDI_IOCTL_SET_CLOCK_GATE, &enable);	

    return ret;
}

int vdi_get_clock_gate(unsigned long core_idx)
{
    vdi_info_t *vdi;
    int ret;

    if (core_idx >= MAX_NUM_VPU_CORE)
        return -1;

    vdi = &s_vdi_info[core_idx];

    if(!vdi || vdi->vpu_fd==-1 || vdi->vpu_fd == 0x00)
        return -1;

    ret = vdi->clock_state;
    return ret;
}

int vdi_wait_bus_busy(unsigned long core_idx, int timeout, unsigned int gdi_busy_flag)
{
    Int64 elapse, cur;
    struct timeval tv;    
    vdi_info_t *vdi;

    vdi = &s_vdi_info[core_idx];
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    gettimeofday(&tv, NULL);
    elapse = tv.tv_sec*1000 + tv.tv_usec/1000;

    while(1)
    {
        if (vdi->product_code == WAVE420L_CODE) {
            if (vdi_fio_read_register(core_idx, gdi_busy_flag) == 0)  {
                break;
            }
        }
        else if (vdi->product_code == WAVE520_CODE) {
            if (vdi_fio_read_register(core_idx, gdi_busy_flag) == 0x3f) break;
        }
        else if (PRODUCT_CODE_W_SERIES(vdi->product_code)) {
            if (vdi_fio_read_register(core_idx, gdi_busy_flag) == 0x738) break;
        }
        else if (PRODUCT_CODE_NOT_W_SERIES(vdi->product_code)) {
            if (vdi_read_register(core_idx, gdi_busy_flag) == 0x77) break;
        }
        else {
            VLOG(ERR, "Unknown product id : %08x\n", vdi->product_code);
            return -1;
        }

        if (timeout > 0) {
            gettimeofday(&tv, NULL);
            cur = tv.tv_sec * 1000 + tv.tv_usec / 1000;

            if ((cur - elapse) > timeout) {
                VLOG(ERR, "[VDI] vdi_wait_bus_busy timeout, PC=0x%lx\n", vdi_read_register(core_idx, 0x018));
                return -1;
            }
        }
    }
    return 0;

}

int vdi_wait_vpu_busy(unsigned long core_idx, int timeout, unsigned int addr_bit_busy_flag)
{
    Int64 elapse, cur;
    struct timeval tv;
    Uint32 pc;
    Uint32 code, normalReg = TRUE;

    tv.tv_sec = 0;
    tv.tv_usec = 0;
    gettimeofday(&tv, NULL);
    elapse = tv.tv_sec*1000 + tv.tv_usec/1000;

    code = vdi_read_register(core_idx, VPU_PRODUCT_CODE_REGISTER); /* read product code */
    if (PRODUCT_CODE_W_SERIES(code)) {
        pc = W4_VCPU_CUR_PC;
        if (addr_bit_busy_flag&0x8000) normalReg = FALSE;
    }
    else if (PRODUCT_CODE_NOT_W_SERIES(code)) {
        pc = BIT_CUR_PC;
    }
    else {
        VLOG(ERR, "Unknown product id : %08x\n", code);
        return -1;
    }

    while(1)
    {
        if (normalReg == TRUE) {
            if (vdi_read_register(core_idx, addr_bit_busy_flag) == 0) break;
        }
        else {
            if (vdi_fio_read_register(core_idx, addr_bit_busy_flag) == 0) break;
        }

        if (timeout > 0) {
            gettimeofday(&tv, NULL);
            cur = tv.tv_sec * 1000 + tv.tv_usec / 1000;

            if ((cur - elapse) > timeout) {
                Uint32 index;
                for (index=0; index<50; index++) {
                    VLOG(ERR, "[VDI] vdi_wait_vpu_busy timeout, PC=0x%lx\n", vdi_read_register(core_idx, pc));
                }
                return -1;
            }
        }
    }
    return 0;

}

int vdi_wait_interrupt(unsigned long coreIdx, int timeout, unsigned int addr_bit_int_reason)
{
    int intr_reason = 0;
    int ret;
    vdi_info_t *vdi;
    vpudrv_intr_info_t intr_info;

    if (coreIdx >= MAX_NUM_VPU_CORE)
        return -1;

    vdi = &s_vdi_info[coreIdx];

    if(!vdi || vdi->vpu_fd == -1 || vdi->vpu_fd == 0x00)
        return -1;
#ifdef SUPPORT_INTERRUPT
    intr_info.timeout     = timeout;
    intr_info.intr_reason = 0;
    ret = ioctl(vdi->vpu_fd, VDI_IOCTL_WAIT_INTERRUPT, (void*)&intr_info);
    if (ret != 0)
        return -1;
    intr_reason = intr_info.intr_reason;	
#else
    struct timeval  tv = {0};
    Uint32          intrStatusReg;
    Uint32          pc;
    Int32           startTime, endTime, elaspedTime;

    UNREFERENCED_PARAMETER(intr_info);

    if (PRODUCT_CODE_W_SERIES(vdi->product_code)) {
        pc            = W4_VCPU_CUR_PC;
        intrStatusReg = W4_VPU_VPU_INT_STS;
    }
    else if (PRODUCT_CODE_NOT_W_SERIES(vdi->product_code)) {
        pc            = BIT_CUR_PC;
        intrStatusReg = BIT_INT_STS;
    }
    else {
        VLOG(ERR, "Unknown product id : %08x\n", vdi->product_code);
        return -1;
    }

    gettimeofday(&tv, NULL);
    startTime = tv.tv_sec*1000 + tv.tv_usec/1000;
    while (TRUE) {
        if (vdi_read_register(coreIdx, intrStatusReg)) {
            if ((intr_reason=vdi_read_register(coreIdx, addr_bit_int_reason)))
                break;
        }
        gettimeofday(&tv, NULL);
        endTime = tv.tv_sec*1000 + tv.tv_usec/1000;
        if (timeout > 0 && (endTime-startTime) >= timeout) {
            return -1;
        }
    }
#endif

    return intr_reason;
}

static int read_pinfo_buffer(int core_idx, int addr)
{
    int ack;
    int rdata;
#define VDI_LOG_GDI_PINFO_ADDR  (0x1068)
#define VDI_LOG_GDI_PINFO_REQ   (0x1060)
#define VDI_LOG_GDI_PINFO_ACK   (0x1064)
#define VDI_LOG_GDI_PINFO_DATA  (0x106c)
    //------------------------------------------
    // read pinfo - indirect read
    // 1. set read addr     (GDI_PINFO_ADDR)
    // 2. send req          (GDI_PINFO_REQ)
    // 3. wait until ack==1 (GDI_PINFO_ACK)
    // 4. read data         (GDI_PINFO_DATA)
    //------------------------------------------
    vdi_write_register(core_idx, VDI_LOG_GDI_PINFO_ADDR, addr);
    vdi_write_register(core_idx, VDI_LOG_GDI_PINFO_REQ, 1);

    ack = 0;
    while (ack == 0)
    {
        ack = vdi_read_register(core_idx, VDI_LOG_GDI_PINFO_ACK);
    }

    rdata = vdi_read_register(core_idx, VDI_LOG_GDI_PINFO_DATA);

    //printf("[READ PINFO] ADDR[%x], DATA[%x]", addr, rdata);
    return rdata;
}

enum {
    VDI_PRODUCT_ID_980,
    VDI_PRODUCT_ID_960
};

static void printf_gdi_info(int core_idx, int num, int reset)
{
    int i;
    int bus_info_addr;
    int tmp;
    int val;
    int productId;

    val = vdi_read_register(core_idx, VPU_PRODUCT_CODE_REGISTER);
    if ((val&0xff00) == 0x3200) val = 0x3200;

    if (PRODUCT_CODE_W_SERIES(val)) {
        return;
    }
    else if (PRODUCT_CODE_NOT_W_SERIES(val)) {
        if (val == CODA960_CODE || val == BODA950_CODE)
            productId = VDI_PRODUCT_ID_960;
        else if (val == CODA980_CODE || val == WAVE320_CODE)
            productId = VDI_PRODUCT_ID_980;
    }
    else {
        VLOG(ERR, "Unknown product id : %08x\n", val);
        return;
    }

    if (productId == VDI_PRODUCT_ID_980)
        VLOG(INFO, "\n**GDI information for GDI_20\n");		
    else
        VLOG(INFO, "\n**GDI information for GDI_10\n");		

    for (i=0; i < num; i++)
    {

#define VDI_LOG_GDI_INFO_CONTROL 0x1400
        if (productId == VDI_PRODUCT_ID_980)
            bus_info_addr = VDI_LOG_GDI_INFO_CONTROL + i*(0x20);
        else
            bus_info_addr = VDI_LOG_GDI_INFO_CONTROL + i*0x14;
        if (reset)
        {
            vdi_write_register(core_idx, bus_info_addr, 0x00);
            bus_info_addr += 4;
            vdi_write_register(core_idx, bus_info_addr, 0x00);
            bus_info_addr += 4;
            vdi_write_register(core_idx, bus_info_addr, 0x00);
            bus_info_addr += 4;
            vdi_write_register(core_idx, bus_info_addr, 0x00);
            bus_info_addr += 4;
            vdi_write_register(core_idx, bus_info_addr, 0x00);

            if (productId == VDI_PRODUCT_ID_980)
            {
                bus_info_addr += 4;
                vdi_write_register(core_idx, bus_info_addr, 0x00);

                bus_info_addr += 4;
                vdi_write_register(core_idx, bus_info_addr, 0x00);

                bus_info_addr += 4;
                vdi_write_register(core_idx, bus_info_addr, 0x00);
            }

        }
        else
        {
            VLOG(INFO, "index = %02d", i);

            tmp = read_pinfo_buffer(core_idx, bus_info_addr);	//TiledEn<<20 ,GdiFormat<<17,IntlvCbCr,<<16 GdiYuvBufStride
            VLOG(INFO, " control = 0x%08x", tmp);

            bus_info_addr += 4;
            tmp = read_pinfo_buffer(core_idx, bus_info_addr);
            VLOG(INFO, " pic_size = 0x%08x", tmp);

            bus_info_addr += 4;
            tmp = read_pinfo_buffer(core_idx, bus_info_addr);
            VLOG(INFO, " y-top = 0x%08x", tmp);

            bus_info_addr += 4;
            tmp = read_pinfo_buffer(core_idx, bus_info_addr);
            VLOG(INFO, " cb-top = 0x%08x", tmp);

            bus_info_addr += 4;
            tmp = read_pinfo_buffer(core_idx, bus_info_addr);
            VLOG(INFO, " cr-top = 0x%08x", tmp);		
            if (productId == VDI_PRODUCT_ID_980)
            {
                bus_info_addr += 4;
                tmp = read_pinfo_buffer(core_idx, bus_info_addr);
                VLOG(INFO, " y-bot = 0x%08x", tmp);

                bus_info_addr += 4;
                tmp = read_pinfo_buffer(core_idx, bus_info_addr);
                VLOG(INFO, " cb-bot = 0x%08x", tmp);

                bus_info_addr += 4;
                tmp = read_pinfo_buffer(core_idx, bus_info_addr);
                VLOG(INFO, " cr-bot = 0x%08x", tmp);		
            }
            VLOG(INFO, "\n");				
        }
    }
}

void vdi_print_vpu_status(unsigned long coreIdx)
{
	unsigned int product_code;

	product_code = vdi_read_register(coreIdx, VPU_PRODUCT_CODE_REGISTER);

    if (PRODUCT_CODE_W_SERIES(product_code)) 
	{
		int      rd, wr;
		unsigned int    tq, ip, mc, lf;
		unsigned int    avail_cu, avail_tu, avail_tc, avail_lf, avail_ip;
		unsigned int	 ctu_fsm, nb_fsm, cabac_fsm, cu_info, mvp_fsm, tc_busy, lf_fsm, bs_data, bbusy, fv;
		unsigned int    reg_val;
		unsigned int    index;
		unsigned int    vcpu_reg[31]= {0,};

		printf("-------------------------------------------------------------------------------\n");
		printf("------                            VCPU STATUS                             -----\n");
		printf("-------------------------------------------------------------------------------\n");
		rd = VpuReadReg(coreIdx, W4_BS_RD_PTR);
		wr = VpuReadReg(coreIdx, W4_BS_WR_PTR);
		printf("RD_PTR: 0x%08x WR_PTR: 0x%08x BS_OPT: 0x%08x BS_PARAM: 0x%08x\n", 
			rd, wr, VpuReadReg(coreIdx, W4_BS_OPTION), VpuReadReg(coreIdx, W4_BS_PARAM));

		// --------- VCPU register Dump 
		printf("[+] VCPU REG Dump\n");
		for (index = 0; index < 25; index++) {
			VpuWriteReg (coreIdx, 0x14, (1<<9) | (index & 0xff));
			vcpu_reg[index] = VpuReadReg (coreIdx, 0x1c);

			if (index < 16) {
				printf("0x%08x\t",  vcpu_reg[index]);
				if ((index % 4) == 3) printf("\n");
			}
			else {
				switch (index) {
				case 16: printf("CR0: 0x%08x\t", vcpu_reg[index]); break;
				case 17: printf("CR1: 0x%08x\n", vcpu_reg[index]); break;
				case 18: printf("ML:  0x%08x\t", vcpu_reg[index]); break;
				case 19: printf("MH:  0x%08x\n", vcpu_reg[index]); break;
				case 21: printf("LR:  0x%08x\n", vcpu_reg[index]); break;
				case 22: printf("PC:  0x%08x\n", vcpu_reg[index]);break;
				case 23: printf("SR:  0x%08x\n", vcpu_reg[index]);break;
				case 24: printf("SSP: 0x%08x\n", vcpu_reg[index]);break;
				}
			}
		}
		printf("[-] VCPU REG Dump\n");
		// --------- BIT register Dump 
		printf("[+] BPU REG Dump\n");
		printf("BITPC = 0x%08x\n", vdi_fio_read_register(coreIdx, (W4_REG_BASE + 0x8000 + 0x18))); 
		printf("BIT START=0x%08x, BIT END=0x%08x\n", vdi_fio_read_register(coreIdx, (W4_REG_BASE + 0x8000 + 0x11c)), 
			vdi_fio_read_register(coreIdx, (W4_REG_BASE + 0x8000 + 0x120)) );
		if (product_code == WAVE410_CODE )
			printf("BIT COMMAND 0x%x\n", vdi_fio_read_register(coreIdx, (W4_REG_BASE + 0x8000 + 0x100))); 
		if (product_code == WAVE4102_CODE || product_code == WAVE510_CODE)
			printf("BIT COMMAND 0x%x\n", vdi_fio_read_register(coreIdx, (W4_REG_BASE + 0x8000 + 0x1FC))); 

		printf("CODE_BASE			%x \n", vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x7000 + 0x18)));	
		printf("VCORE_REINIT_FLAG	%x \n", vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x7000 + 0x0C)));	

		// --------- BIT HEVC Status Dump 
		ctu_fsm		= vdi_fio_read_register(coreIdx, (W4_REG_BASE + 0x8000 + 0x48));
		nb_fsm		= vdi_fio_read_register(coreIdx, (W4_REG_BASE + 0x8000 + 0x4c));
		cabac_fsm	= vdi_fio_read_register(coreIdx, (W4_REG_BASE + 0x8000 + 0x50));
		cu_info		= vdi_fio_read_register(coreIdx, (W4_REG_BASE + 0x8000 + 0x54));
		mvp_fsm		= vdi_fio_read_register(coreIdx, (W4_REG_BASE + 0x8000 + 0x58));
		tc_busy		= vdi_fio_read_register(coreIdx, (W4_REG_BASE + 0x8000 + 0x5c));
		lf_fsm		= vdi_fio_read_register(coreIdx, (W4_REG_BASE + 0x8000 + 0x60));
		bs_data		= vdi_fio_read_register(coreIdx, (W4_REG_BASE + 0x8000 + 0x64));
		bbusy		= vdi_fio_read_register(coreIdx, (W4_REG_BASE + 0x8000 + 0x68));
		fv		    = vdi_fio_read_register(coreIdx, (W4_REG_BASE + 0x8000 + 0x6C));

		printf("[DEBUG-BPUHEVC] CTU_X: %4d, CTU_Y: %4d\n",  vdi_fio_read_register(coreIdx, (W4_REG_BASE + 0x8000 + 0x40)), vdi_fio_read_register(coreIdx, (W4_REG_BASE + 0x8000 + 0x44))); 
		printf("[DEBUG-BPUHEVC] CTU_FSM>   Main: 0x%02x, FIFO: 0x%1x, NB: 0x%02x, DBK: 0x%1x\n", ((ctu_fsm >> 24) & 0xff), ((ctu_fsm >> 16) & 0xff), ((ctu_fsm >> 8) & 0xff), (ctu_fsm & 0xff));
		printf("[DEBUG-BPUHEVC] NB_FSM:	0x%02x\n", nb_fsm & 0xff);
		printf("[DEBUG-BPUHEVC] CABAC_FSM> SAO: 0x%02x, CU: 0x%02x, PU: 0x%02x, TU: 0x%02x, EOS: 0x%02x\n", ((cabac_fsm>>25) & 0x3f), ((cabac_fsm>>19) & 0x3f), ((cabac_fsm>>13) & 0x3f), ((cabac_fsm>>6) & 0x7f), (cabac_fsm & 0x3f));
		printf("[DEBUG-BPUHEVC] CU_INFO value = 0x%04x \n\t\t(l2cb: 0x%1x, cux: %1d, cuy; %1d, pred: %1d, pcm: %1d, wr_done: %1d, par_done: %1d, nbw_done: %1d, dec_run: %1d)\n", cu_info, 
			((cu_info>> 16) & 0x3), ((cu_info>> 13) & 0x7), ((cu_info>> 10) & 0x7), ((cu_info>> 9) & 0x3), ((cu_info>> 8) & 0x1), ((cu_info>> 6) & 0x3), ((cu_info>> 4) & 0x3), ((cu_info>> 2) & 0x3), (cu_info & 0x3));
		printf("[DEBUG-BPUHEVC] MVP_FSM> 0x%02x\n", mvp_fsm & 0xf);
		printf("[DEBUG-BPUHEVC] TC_BUSY> tc_dec_busy: %1d, tc_fifo_busy: 0x%02x\n", ((tc_busy >> 3) & 0x1), (tc_busy & 0x7));
		printf("[DEBUG-BPUHEVC] LF_FSM>  SAO: 0x%1x, LF: 0x%1x\n", ((lf_fsm >> 4) & 0xf), (lf_fsm  & 0xf));
		printf("[DEBUG-BPUHEVC] BS_DATA> ExpEnd=%1d, bs_valid: 0x%03x, bs_data: 0x%03x\n", ((bs_data >> 31) & 0x1), ((bs_data >> 16) & 0xfff), (bs_data & 0xfff));
		printf("[DEBUG-BPUHEVC] BUS_BUSY> mib_wreq_done: %1d, mib_busy: %1d, sdma_bus: %1d\n", ((bbusy >> 2) & 0x1), ((bbusy >> 1) & 0x1) , (bbusy & 0x1));
		printf("[DEBUG-BPUHEVC] FIFO_VALID> cu: %1d, tu: %1d, iptu: %1d, lf: %1d, coff: %1d\n\n", ((fv >> 4) & 0x1), ((fv >> 3) & 0x1), ((fv >> 2) & 0x1), ((fv >> 1) & 0x1), (fv & 0x1));
		printf("[-] BPU REG Dump\n");

		// --------- VCE register Dump 
		printf("[+] VCE REG Dump\n");
		tq = read_vce_register(0, 0, 0xd0);
		ip = read_vce_register(0, 0, 0xd4); 
		mc = read_vce_register(0, 0, 0xd8);
		lf = read_vce_register(0, 0, 0xdc);
		avail_cu = (read_vce_register(0, 0, 0x11C)>>16) - (read_vce_register(0, 0, 0x110)>>16);
		avail_tu = (read_vce_register(0, 0, 0x11C)&0xFFFF) - (read_vce_register(0, 0, 0x110)&0xFFFF);
		avail_tc = (read_vce_register(0, 0, 0x120)>>16) - (read_vce_register(0, 0, 0x114)>>16);
		avail_lf = (read_vce_register(0, 0, 0x120)&0xFFFF) - (read_vce_register(0, 0, 0x114)&0xFFFF);
		avail_ip = (read_vce_register(0, 0, 0x124)>>16) - (read_vce_register(0, 0, 0x118)>>16);
		printf("       TQ            IP              MC             LF      GDI_EMPTY          ROOM \n");
		printf("------------------------------------------------------------------------------------------------------------\n");
		printf("| %d %04d %04d | %d %04d %04d |  %d %04d %04d | %d %04d %04d | 0x%08x | CU(%d) TU(%d) TC(%d) LF(%d) IP(%d)\n",
			(tq>>22)&0x07, (tq>>11)&0x3ff, tq&0x3ff,
			(ip>>22)&0x07, (ip>>11)&0x3ff, ip&0x3ff,
			(mc>>22)&0x07, (mc>>11)&0x3ff, mc&0x3ff,
			(lf>>22)&0x07, (lf>>11)&0x3ff, lf&0x3ff,
			vdi_fio_read_register(0, 0x88f4),                      /* GDI empty */
			avail_cu, avail_tu, avail_tc, avail_lf, avail_ip);
		/* CU/TU Queue count */
		reg_val = read_vce_register(0, 0, 0x12C);
		printf("[DCIDEBUG] QUEUE COUNT: CU(%5d) TU(%5d) ", (reg_val>>16)&0xffff, reg_val&0xffff);
		reg_val = read_vce_register(0, 0, 0x1A0);
		printf("TC(%5d) IP(%5d) ", (reg_val>>16)&0xffff, reg_val&0xffff);
		reg_val = read_vce_register(0, 0, 0x1A4);
		printf("LF(%5d)\n", (reg_val>>16)&0xffff);
		printf("VALID SIGNAL : CU0(%d)  CU1(%d)  CU2(%d) TU(%d) TC(%d) IP(%5d) LF(%5d)\n" 
			"               DCI_FALSE_RUN(%d) VCE_RESET(%d) CORE_INIT(%d) SET_RUN_CTU(%d) \n",
			(reg_val>>6)&1, (reg_val>>5)&1, (reg_val>>4)&1, (reg_val>>3)&1, 
			(reg_val>>2)&1, (reg_val>>1)&1, (reg_val>>0)&1,
			(reg_val>>10)&1, (reg_val>>9)&1, (reg_val>>8)&1, (reg_val>>7)&1);

		printf("State TQ: 0x%08x IP: 0x%08x MC: 0x%08x LF: 0x%08x\n", 
			read_vce_register(0, 0, 0xd0), read_vce_register(0, 0, 0xd4), read_vce_register(0, 0, 0xd8), read_vce_register(0, 0, 0xdc));
		printf("BWB[1]: RESPONSE_CNT(0x%08x) INFO(0x%08x)\n", read_vce_register(0, 0, 0x194), read_vce_register(0, 0, 0x198));
		printf("BWB[2]: RESPONSE_CNT(0x%08x) INFO(0x%08x)\n", read_vce_register(0, 0, 0x194), read_vce_register(0, 0, 0x198));
		printf("DCI INFO\n");
		printf("READ_CNT_0 : 0x%08x\n", read_vce_register(0, 0, 0x110));
		printf("READ_CNT_1 : 0x%08x\n", read_vce_register(0, 0, 0x114));
		printf("READ_CNT_2 : 0x%08x\n", read_vce_register(0, 0, 0x118));
		printf("WRITE_CNT_0: 0x%08x\n", read_vce_register(0, 0, 0x11c));
		printf("WRITE_CNT_1: 0x%08x\n", read_vce_register(0, 0, 0x120));
		printf("WRITE_CNT_2: 0x%08x\n", read_vce_register(0, 0, 0x124));
		reg_val = read_vce_register(0, 0, 0x128);
		printf("LF_DEBUG_PT: 0x%08x\n", reg_val & 0xffffffff);
		printf("cur_main_state %2d, r_lf_pic_deblock_disable %1d, r_lf_pic_sao_disable %1d\n",
			(reg_val >> 16) & 0x1f, 
			(reg_val >> 15) & 0x1, 
			(reg_val >> 14) & 0x1);
		printf("para_load_done %1d, i_rdma_ack_wait %1d, i_sao_intl_col_done %1d, i_sao_outbuf_full %1d\n",
			(reg_val >> 13) & 0x1, 
			(reg_val >> 12) & 0x1, 
			(reg_val >> 11) & 0x1, 
			(reg_val >> 10) & 0x1);
		printf("lf_sub_done %1d, i_wdma_ack_wait %1d, lf_all_sub_done %1d, cur_ycbcr %1d, sub8x8_done %2d\n", 
			(reg_val >> 9) & 0x1, 
			(reg_val >> 8) & 0x1, 
			(reg_val >> 6) & 0x1, 
			(reg_val >> 4) & 0x1, 
			reg_val & 0xf);
		printf("[-] VCE REG Dump\n");
		printf("[-] VCE REG Dump\n");

		printf("-------------------------------------------------------------------------------\n");
	}
	else if (PRODUCT_CODE_NOT_W_SERIES(product_code)) {
	}
    else {
        VLOG(ERR, "Unknown product id : %08x\n", product_code);
    }
}

void vdi_make_log(unsigned long core_idx, const char *str, int step)
{
    int val;

    val = VpuReadReg(core_idx, W4_INST_INDEX);
    val &= 0xffff;
    if (step == 1)
        VLOG(INFO, "\n**%s start(%d)\n", str, val);
    else if (step == 2)	// 
        VLOG(INFO, "\n**%s timeout(%d)\n", str, val);		
    else
        VLOG(INFO, "\n**%s end(%d)\n", str, val);		
}

void vdi_log(unsigned long core_idx, int cmd, int step)
{
    vdi_info_t *vdi;
    int i;

    // BIT_RUN command
    enum {
        SEQ_INIT = 1,
        SEQ_END = 2,
        PIC_RUN = 3,
        SET_FRAME_BUF = 4,
        ENCODE_HEADER = 5,
        ENC_PARA_SET = 6,
        DEC_PARA_SET = 7,
        DEC_BUF_FLUSH = 8,
        RC_CHANGE_PARAMETER	= 9,
        VPU_SLEEP = 10,
        VPU_WAKE = 11,
        ENC_ROI_INIT = 12,
        FIRMWARE_GET = 0xf,
        VPU_RESET = 0x10,
    };

    if (core_idx >= MAX_NUM_VPU_CORE)
        return ;

    vdi = &s_vdi_info[core_idx];

    if(!vdi || vdi->vpu_fd==-1 || vdi->vpu_fd == 0)
        return ;

    if (PRODUCT_CODE_W_SERIES(vdi->product_code))
	{
		switch(cmd)
		{
        case INIT_VPU:
            vdi_make_log(core_idx, "INIT_VPU", step);			 
            break;
        case DEC_PIC_HDR: //SET_PARAM for ENC
            vdi_make_log(core_idx, "SET_PARAM(ENC), DEC_PIC_HDR(DEC)", step);
            break;
        case FINI_SEQ:
            vdi_make_log(core_idx, "FINI_SEQ", step);
            break;
        case DEC_PIC://ENC_PIC for ENC
            vdi_make_log(core_idx, "DEC_PIC, ENC_PIC", step);
            break;
		case SET_FRAMEBUF:
            vdi_make_log(core_idx, "SET_FRAMEBUF", step);
			break;
		case FLUSH_DECODER:
            vdi_make_log(core_idx, "FLUSH_DECODER", step);
			break;
		case GET_FW_VERSION:
            vdi_make_log(core_idx, "GET_FW_VERSION", step);
			break;
		case QUERY_DECODER:
            vdi_make_log(core_idx, "QUERY_DECODER", step);
			break;
		case SLEEP_VPU:
            vdi_make_log(core_idx, "SLEEP_VPU", step);
			break;
		case CREATE_INSTANCE:
            vdi_make_log(core_idx, "CREATE_INSTANCE", step);
			break;
		case RESET_VPU:
			vdi_make_log(core_idx, "RESET_VPU", step);
			break;	
		default:
            vdi_make_log(core_idx, "ANY_CMD", step);
			break;
		}
	}
	else if (PRODUCT_CODE_NOT_W_SERIES(vdi->product_code))
	{
		switch(cmd)
		{
		case SEQ_INIT:
            vdi_make_log(core_idx, "SEQ_INIT", step);	
			break;
		case SEQ_END:
            vdi_make_log(core_idx, "SEQ_END", step);		
			break;
		case PIC_RUN:
            vdi_make_log(core_idx, "PIC_RUN", step);	
			break;
		case SET_FRAME_BUF:
            vdi_make_log(core_idx, "SET_FRAME_BUF", step);	
			break;
		case ENCODE_HEADER:
            vdi_make_log(core_idx, "ENCODE_HEADER", step);	
			break;
		case RC_CHANGE_PARAMETER:
            vdi_make_log(core_idx, "RC_CHANGE_PARAMETER", step);	
			break;
		case DEC_BUF_FLUSH:
            vdi_make_log(core_idx, "DEC_BUF_FLUSH", step);	
			break;
		case FIRMWARE_GET:
            vdi_make_log(core_idx, "FIRMWARE_GET", step);	
			break;
		case VPU_RESET:
            vdi_make_log(core_idx, "VPU_RESET", step);
			break;
		case ENC_PARA_SET:
            vdi_make_log(core_idx, "ENC_PARA_SET", step);
			break;
		case DEC_PARA_SET:
            vdi_make_log(core_idx, "DEC_PARA_SET", step);
			break;
		default:
            vdi_make_log(core_idx, "ANY_CMD", step);
			break;
		}
	}
    else {
        VLOG(ERR, "Unknown product id : %08x\n", vdi->product_code);
        return;
    }

	for (i=0; i<0x200; i=i+16)
	{
		VLOG(INFO, "0x%04xh: 0x%08x 0x%08x 0x%08x 0x%08x\n", i,
            vdi_read_register(core_idx, i), vdi_read_register(core_idx, i+4),
            vdi_read_register(core_idx, i+8), vdi_read_register(core_idx, i+0xc));
	}

    if (PRODUCT_CODE_W_SERIES(vdi->product_code)) // WAVE4xx
	{
		if (cmd == INIT_VPU || cmd == VPU_RESET || cmd == CREATE_INSTANCE)
		{
			vdi_print_vpu_status(core_idx);
		}

	}
	else if (PRODUCT_CODE_NOT_W_SERIES(vdi->product_code))
	{
		//if ((cmd == PIC_RUN && step== 0) || cmd == VPU_RESET)
		if (cmd == VPU_RESET)
		{
			printf_gdi_info(core_idx, 32, 0);

#define VDI_LOG_MBC_BUSY 0x0440
#define VDI_LOG_MC_BASE	 0x0C00
#define VDI_LOG_MC_BUSY	 0x0C04
#define VDI_LOG_GDI_BUS_STATUS (0x10F4)
#define VDI_LOG_ROT_SRC_IDX	 (0x400 + 0x10C)
#define VDI_LOG_ROT_DST_IDX	 (0x400 + 0x110)

			VLOG(INFO, "MBC_BUSY = %x\n", vdi_read_register(core_idx, VDI_LOG_MBC_BUSY));
			VLOG(INFO, "MC_BUSY = %x\n", vdi_read_register(core_idx, VDI_LOG_MC_BUSY));
			VLOG(INFO, "MC_MB_XY_DONE=(y:%d, x:%d)\n", (vdi_read_register(core_idx, VDI_LOG_MC_BASE) >> 20) & 0x3F, (vdi_read_register(core_idx, VDI_LOG_MC_BASE) >> 26) & 0x3F);
			VLOG(INFO, "GDI_BUS_STATUS = %x\n", vdi_read_register(core_idx, VDI_LOG_GDI_BUS_STATUS));

			VLOG(INFO, "ROT_SRC_IDX = %x\n", vdi_read_register(core_idx, VDI_LOG_ROT_SRC_IDX));
			VLOG(INFO, "ROT_DST_IDX = %x\n", vdi_read_register(core_idx, VDI_LOG_ROT_DST_IDX));

			VLOG(INFO, "P_MC_PIC_INDEX_0 = %x\n", vdi_read_register(core_idx, MC_BASE+0x200));
			VLOG(INFO, "P_MC_PIC_INDEX_1 = %x\n", vdi_read_register(core_idx, MC_BASE+0x20c));
			VLOG(INFO, "P_MC_PIC_INDEX_2 = %x\n", vdi_read_register(core_idx, MC_BASE+0x218));
			VLOG(INFO, "P_MC_PIC_INDEX_3 = %x\n", vdi_read_register(core_idx, MC_BASE+0x230));
			VLOG(INFO, "P_MC_PIC_INDEX_3 = %x\n", vdi_read_register(core_idx, MC_BASE+0x23C));
			VLOG(INFO, "P_MC_PIC_INDEX_4 = %x\n", vdi_read_register(core_idx, MC_BASE+0x248));
			VLOG(INFO, "P_MC_PIC_INDEX_5 = %x\n", vdi_read_register(core_idx, MC_BASE+0x254));
			VLOG(INFO, "P_MC_PIC_INDEX_6 = %x\n", vdi_read_register(core_idx, MC_BASE+0x260));
			VLOG(INFO, "P_MC_PIC_INDEX_7 = %x\n", vdi_read_register(core_idx, MC_BASE+0x26C));
			VLOG(INFO, "P_MC_PIC_INDEX_8 = %x\n", vdi_read_register(core_idx, MC_BASE+0x278));
			VLOG(INFO, "P_MC_PIC_INDEX_9 = %x\n", vdi_read_register(core_idx, MC_BASE+0x284));
			VLOG(INFO, "P_MC_PIC_INDEX_a = %x\n", vdi_read_register(core_idx, MC_BASE+0x290));
			VLOG(INFO, "P_MC_PIC_INDEX_b = %x\n", vdi_read_register(core_idx, MC_BASE+0x29C));
			VLOG(INFO, "P_MC_PIC_INDEX_c = %x\n", vdi_read_register(core_idx, MC_BASE+0x2A8));
			VLOG(INFO, "P_MC_PIC_INDEX_d = %x\n", vdi_read_register(core_idx, MC_BASE+0x2B4));

			VLOG(INFO, "P_MC_PICIDX_0 = %x\n", vdi_read_register(core_idx, MC_BASE+0x028));
			VLOG(INFO, "P_MC_PICIDX_1 = %x\n", vdi_read_register(core_idx, MC_BASE+0x02C));
		}
	}
    else {
        VLOG(ERR, "Unknown product id : %08x\n", vdi->product_code);
        return;
    }
}

static void byte_swap(unsigned char* data, int len)
{
    Uint8 temp;
    Int32 i;

    for (i=0; i<len; i+=2) {
        temp      = data[i];
        data[i]   = data[i+1];
        data[i+1] = temp;
    }
}

static void word_swap(unsigned char* data, int len)
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

static void dword_swap(unsigned char* data, int len)
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

static void lword_swap(unsigned char* data, int len)
{
    Uint64  temp;
    Uint64* ptr = (Uint64*)data;
    Int32   i, size = len/sizeof(Uint64);

    for (i=0; i<size; i+=2) {
        temp      = ptr[i];
        ptr[i]   = ptr[i+1];
        ptr[i+1] = temp;
    }
}

int vdi_get_system_endian(unsigned long core_idx)
{
    vdi_info_t *vdi;

    if (core_idx >= MAX_NUM_VPU_CORE)
        return -1;

    vdi = &s_vdi_info[core_idx];

    if(!vdi || vdi->vpu_fd == -1 || vdi->vpu_fd == 0x00)
        return -1;

    if (PRODUCT_CODE_W_SERIES(vdi->product_code)) {
        return VDI_128BIT_BUS_SYSTEM_ENDIAN;
    }
    else if(PRODUCT_CODE_NOT_W_SERIES(vdi->product_code)) {
        return VDI_SYSTEM_ENDIAN;
    }
    else {
        VLOG(ERR, "Unknown product id : %08x\n", vdi->product_code);
        return -1;
    }
}

int vdi_convert_endian(unsigned long core_idx, unsigned int endian)
{
    vdi_info_t *vdi;

    if (core_idx >= MAX_NUM_VPU_CORE)
        return -1;

    vdi = &s_vdi_info[core_idx];

    if(!vdi || !vdi || vdi->vpu_fd == -1 || vdi->vpu_fd == 0x00)
        return -1;

    if (PRODUCT_CODE_W_SERIES(vdi->product_code)) {
        switch (endian) {
        case VDI_LITTLE_ENDIAN:       endian = 0x00; break;
        case VDI_BIG_ENDIAN:          endian = 0x0f; break;
        case VDI_32BIT_LITTLE_ENDIAN: endian = 0x04; break;
        case VDI_32BIT_BIG_ENDIAN:    endian = 0x03; break;
        }
    } 
    else if(PRODUCT_CODE_NOT_W_SERIES(vdi->product_code)) {
    }
    else {
        VLOG(ERR, "Unknown product id : %08x\n", vdi->product_code);
        return -1;
    }

    return (endian&0x0f); 
}

static Uint32 convert_endian_coda9_to_wave4(Uint32 endian)
{
    Uint32 converted_endian = endian;
    switch(endian) {
    case VDI_LITTLE_ENDIAN:       converted_endian = 0; break;
    case VDI_BIG_ENDIAN:          converted_endian = 7; break;
    case VDI_32BIT_LITTLE_ENDIAN: converted_endian = 4; break;
    case VDI_32BIT_BIG_ENDIAN:    converted_endian = 3; break;
    }
    return converted_endian;
}


int swap_endian(unsigned long core_idx, unsigned char *data, int len, int endian)
{
    vdi_info_t *vdi;
    int changes;
    int sys_endian;
    BOOL byteChange, wordChange, dwordChange, lwordChange;

    if (core_idx >= MAX_NUM_VPU_CORE)
        return -1;

    vdi = &s_vdi_info[core_idx];

    if(!vdi || vdi->vpu_fd == -1 || vdi->vpu_fd == 0x00)
        return -1;

    if (PRODUCT_CODE_W_SERIES(vdi->product_code)) {
        sys_endian = VDI_128BIT_BUS_SYSTEM_ENDIAN;
    }
    else if(PRODUCT_CODE_NOT_W_SERIES(vdi->product_code)) {
        sys_endian = VDI_SYSTEM_ENDIAN;
    }
    else {
        VLOG(ERR, "Unknown product id : %08x\n", vdi->product_code);
        return -1;
    }

    endian     = vdi_convert_endian(core_idx, endian);
    sys_endian = vdi_convert_endian(core_idx, sys_endian);
    if (endian == sys_endian)
        return 0;

    if (PRODUCT_CODE_W_SERIES(vdi->product_code)) {
    }
    else if (PRODUCT_CODE_NOT_W_SERIES(vdi->product_code)) {
        endian     = convert_endian_coda9_to_wave4(endian);
        sys_endian = convert_endian_coda9_to_wave4(sys_endian);
    }
    else {
        VLOG(ERR, "Unknown product id : %08x\n", vdi->product_code);
        return -1;
    }

    changes     = endian ^ sys_endian;
    byteChange  = changes&0x01;
    wordChange  = ((changes&0x02) == 0x02);
    dwordChange = ((changes&0x04) == 0x04);
    lwordChange = ((changes&0x08) == 0x08);

    if (byteChange)  byte_swap(data, len);
    if (wordChange)  word_swap(data, len);
    if (dwordChange) dword_swap(data, len);
    if (lwordChange) lword_swap(data, len);

    return 1;
}


#endif	//#if defined(linux) || defined(__linux) || defined(ANDROID)
 
