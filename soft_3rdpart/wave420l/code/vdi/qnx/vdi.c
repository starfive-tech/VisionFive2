//------------------------------------------------------------------------------
// File: vdi.c
//
// Copyright (c) 2006, Chips & Media.  All rights reserved.
//------------------------------------------------------------------------------
#if defined(unix) || defined(__unix)

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
#include <errno.h>		    /* fopen/fread */
#include <sys/types.h>
#include <sys/time.h>
#include "driver/vpu.h"
#include "../vdi.h"
#include "../vdi_osal.h"
#include "coda9/coda9_regdefine.h"
#include "wave/common/common_regdefine.h"
#include "wave/wave4/wave4_regdefine.h"
#include "wave/coda7q/coda7q_regdefine.h"

#define VPU_DEVICE_NAME "/dev/vpu"

typedef pthread_mutex_t	MUTEX_HANDLE;

#ifdef CNM_FPGA_PLATFORM		//this definition is only for chipsnmedia FPGA board env. so for SOC env of customers can be ignored.

#define ACLK_MAX					25
#define ACLK_MIN					16
#define CCLK_MAX					25
#define CCLK_MIN					16

#define CLOCK_ID_ACLK               0
#define CLOCK_ID_CCLK               1
#ifdef SUPPORT_2000T
#define CLOCK_ID_VCLK               4
#define CLOCK_ID_BCLK               2
#define CLOCK_ID_MCLK               3
#else
#define CLOCK_ID_VCLK               2
#define CLOCK_ID_BCLK               3
#define CLOCK_ID_MCLK               4
#endif
#define CLOCK_ID_EXTRA              5
#define CLOCK_MASK(_id)             (1<<_id)
#define CLOCK_MASK_ALL              0x1f

static BOOL s_EnableTrace;

#ifdef CNM_FPGA_USB_INTERFACE

// chipsnmedia clock generator in FPGA
#define DEVICE0_ADDR_COMMAND        0x1f000000
#define DEVICE0_ADDR_PARAM0         0x1f000004
#define	DEVICE0_ADDR_PARAM1			0x00	//?
#define DEVICE1_ADDR_COMMAND        0x1f000008
#define DEVICE1_ADDR_PARAM0         0x1f00000c
#define	DEVICE1_ADDR_PARAM1			0x00	//?
#define DEVICE_ADDR_SW_RESET		0x1f000010
#define	DEVICE2_ADDR_COMMAND		0x1f000014
#define DEVICE2_ADDR_PARAM0			0x1f000018
#define	DEVICE2_ADDR_PARAM1			0x00	//?
#define	DEVICE3_ADDR_COMMAND		0x1f00001c
#define DEVICE3_ADDR_PARAM0			0x1f000020
#define	DEVICE3_ADDR_PARAM1			0x00	//?
#define	DEVICE4_ADDR_COMMAND		0x1f000024
#define DEVICE4_ADDR_PARAM0			0x1f000028
#define	DEVICE4_ADDR_PARAM1			0x00	//?
#define	DEVICE5_ADDR_COMMAND		0x1f00002c
#define DEVICE5_ADDR_PARAM0			0x1f000030
#define	DEVICE5_ADDR_PARAM1			0x00	//?
#define	DEVICE6_ADDR_COMMAND		0x1f000034  /*!<< used for change clocks */
#define DEVICE6_ADDR_PARAM0			0x1f000038
#define	DEVICE6_ADDR_PARAM1			0x00	//?
#define	DEVICE7_ADDR_COMMAND		0x1f00003c
#define DEVICE7_ADDR_PARAM0			0x1f000040
#define	DEVICE7_ADDR_PARAM1			0x00	//?
#define CLOCK_MUX_REG			    0x1f000044      /*!<< BEFORE CHANGING CLOCKS SET VALUE TO 0x1F */ 
#define TOTAL_CLOCK_NUMBER          8

#ifdef SUPPORT_CONF_TEST
BOOL USB_TRANSACTION_DOING;
#endif
int usb_init(unsigned long core_idx, unsigned long dram_base);
static void usb_release(unsigned long core_idx);
static int usb_write_register(unsigned long core_idx, int base, unsigned int addr, unsigned int data);
static unsigned int usb_read_register(unsigned long core_idx, int base, unsigned int addr);
static int usb_write_memory(unsigned long core_idx, int base, unsigned int addr, unsigned char *data, int len, int endian);
static int usb_read_memory(unsigned long core_idx, int base, unsigned int addr, unsigned char *data, int len, int endian);
static int usb_hw_reset(int base);
static int usb_chip_reset(int base);
static int usb_set_timing_opt(unsigned long core_idx, int base);
static int usb_ics307m_set_clock_freg(int base, int Device, int OutFreqMHz);
static void usb_write_reg_timing(unsigned long addr, unsigned int data);
static int usb_axi_read_burst(int base, unsigned int addr, unsigned char *buf, int len);
static int usb_axi_write_burst(int base, unsigned int addr, unsigned char *buf, int len);

#endif	/* #ifdef CNM_FPGA_USB_INTERFACE */

#ifdef CNM_FPGA_PCI_INTERFACE

#define HPI_MAX_BUS_LENGTH      16

/*------------------------------------------------------------------------
ChipsnMedia HPI register definitions
------------------------------------------------------------------------*/
#define HPI_CHECK_STATUS			1
#define HPI_WAIT_TIME				0x100000
#define HPI_WAIT_TIMEOUT            10000
#define HPI_BASE					0x20030000
#define HPI_ADDR_CMD				(0x00<<2)
#define HPI_ADDR_STATUS				(0x01<<2)
#define HPI_ADDR_ADDR_H				(0x02<<2)
#define HPI_ADDR_ADDR_L				(0x03<<2)
#define HPI_ADDR_ADDR_M				(0x06<<2)
#define HPI_ADDR_DATA				(0x80<<2)

#define HPI_MAX_PKSIZE 256

// chipsnmedia clock generator in FPGA
#define	DEVICE0_ADDR_COMMAND		0x75
#define DEVICE0_ADDR_PARAM0			0x76
#define	DEVICE0_ADDR_PARAM1			0x77
#define	DEVICE1_ADDR_COMMAND		0x78
#define DEVICE1_ADDR_PARAM0			0x79
#define	DEVICE1_ADDR_PARAM1			0x7a
#define DEVICE_ADDR_SW_RESET		0x7b

#define	DEVICE2_ADDR_COMMAND		0x7c
#define DEVICE2_ADDR_PARAM0			0x7d
#define	DEVICE2_ADDR_PARAM1			0x7e
#define	DEVICE3_ADDR_COMMAND		0x7f
#define DEVICE3_ADDR_PARAM0			0x60
#define	DEVICE3_ADDR_PARAM1			0x61
#define	DEVICE4_ADDR_COMMAND		0x62
#define DEVICE4_ADDR_PARAM0			0x63
#define	DEVICE4_ADDR_PARAM1			0x64
#define	DEVICE5_ADDR_COMMAND		0x65
#define DEVICE5_ADDR_PARAM0			0x66
#define	DEVICE5_ADDR_PARAM1			0x67
#define	DEVICE6_ADDR_COMMAND		0x68    /*!<< used for change clocks */
#define DEVICE6_ADDR_PARAM0			0x69
#define	DEVICE6_ADDR_PARAM1			0x6a
#define	DEVICE7_ADDR_COMMAND		0x6b
#define DEVICE7_ADDR_PARAM0			0x6c
#define	DEVICE7_ADDR_PARAM1			0x6d
#define CLOCK_MUX_REG			    0x6e        /*!<< BEFORE CHANGING CLOCKS SET VALUE TO 0x1F */ 
#define TOTAL_CLOCK_NUMBER          8

static int hpi_init(unsigned long core_idx, unsigned long dram_base);
static void hpi_release(unsigned long core_idx);
static int hpi_write_register(unsigned long core_idx, void * base, unsigned int addr, unsigned int data);
static unsigned int hpi_read_register(unsigned long core_idx, void * base, unsigned int addr);
static int hpi_write_memory(unsigned long core_idx,void * base, uint32_t busLength, unsigned int addr, unsigned char *data, int len, int endian);
static int hpi_read_memory(unsigned long core_idx, void * base, uint32_t busLength, unsigned int addr, unsigned char *data, int len, int endian);
static int hpi_hw_reset(void * base);
static int hpi_set_timing_opt(unsigned long core_idx, void * base);
static int hpi_ics307m_set_clock_freg(void * base, int Device, int OutFreqMHz);
static int io_lock(unsigned long core_idx);
static int io_unlock(unsigned long core_idx);

#endif /* CNM_FPGA_PCI_INTERFACE */

static struct _clock_gen_reg_struct {
    Uint32 command;
    Uint32 param0;
    Uint32 param1;
} s_clock_gen_reg[TOTAL_CLOCK_NUMBER] = {
    { DEVICE0_ADDR_COMMAND, DEVICE0_ADDR_PARAM0, DEVICE0_ADDR_PARAM1 },
    { DEVICE1_ADDR_COMMAND, DEVICE1_ADDR_PARAM0, DEVICE1_ADDR_PARAM1 },
    { DEVICE2_ADDR_COMMAND, DEVICE2_ADDR_PARAM0, DEVICE2_ADDR_PARAM1 },
    { DEVICE3_ADDR_COMMAND, DEVICE3_ADDR_PARAM0, DEVICE3_ADDR_PARAM1 },
    { DEVICE4_ADDR_COMMAND, DEVICE4_ADDR_PARAM0, DEVICE4_ADDR_PARAM1 },
    { DEVICE5_ADDR_COMMAND, DEVICE5_ADDR_PARAM0, DEVICE5_ADDR_PARAM1 },
    { DEVICE6_ADDR_COMMAND, DEVICE6_ADDR_PARAM0, DEVICE6_ADDR_PARAM1 },
    { DEVICE7_ADDR_COMMAND, DEVICE7_ADDR_PARAM0, DEVICE7_ADDR_PARAM1 },
};

#define CLOCK_GEN_COMMAND(_device_no)   s_clock_gen_reg[_device_no].command
#define CLOCK_GEN_PARAM0(_device_no)    s_clock_gen_reg[_device_no].param0
#define CLOCK_GEN_PARAM1(_device_no)    s_clock_gen_reg[_device_no].param1

static int io_lock(unsigned long core_idx);
static int io_unlock(unsigned long core_idx);

#endif	/* #ifdef CNM_FPGA_PLATFORM */

#ifdef CNM_FPGA_PLATFORM
#define SUPPORT_INTERRUPT
#	define VPU_BIT_REG_SIZE		(0x4000*MAX_NUM_VPU_CORE)
#	if defined (WAVE410) || defined (WAVE420) || defined (WAVE412) || defined (CODA7Q)
#		define VPU_BIT_REG_BASE				0x40000000
#		define VDI_DRAM_PHYSICAL_SIZE       (1024*1024*1024)
#	else
#		define VPU_BIT_REG_BASE				0x10000000
#		define VDI_DRAM_PHYSICAL_SIZE       (128*1024*1024)
#	endif
#	define VDI_SRAM_BASE_ADDR				0x00
#ifdef SUPPORT_MINI_PIPPEN
#	define VDI_WAVE410_SRAM_SIZE			0x30000     // 8Kx8K MAIN10 MAX size
#else
#	define VDI_WAVE410_SRAM_SIZE			0x25000     // 8Kx8K MAIN10 MAX size
#endif
#	define VDI_WAVE420_SRAM_SIZE			0x2E000     // 8Kx8X MAIN10 MAX size
#	define VDI_CODA9_SRAM_SIZE				0x34600     // FHD MAX size, 0x17D00  4K MAX size 0x34600
#	ifdef CNM_FPGA_PCI_INTERFACE
#   define VDI_SYSTEM_ENDIAN                VDI_BIG_ENDIAN
#   define VDI_128BIT_BUS_SYSTEM_ENDIAN 	VDI_128BIT_BIG_ENDIAN
#	endif
#	ifdef CNM_FPGA_USB_INTERFACE
#   define VDI_SYSTEM_ENDIAN                VDI_LITTLE_ENDIAN
#   define VDI_128BIT_BUS_SYSTEM_ENDIAN 	VDI_128BIT_LITTLE_ENDIAN
#	endif
#	define VDI_DRAM_PHYSICAL_BASE           0x80000000
#else
#	define SUPPORT_INTERRUPT
#	define VPU_BIT_REG_SIZE	(0x4000*MAX_NUM_VPU_CORE)
#	ifdef SHIP_BUILD
#		define VDI_SRAM_BASE_ADDR	        0x00000000	// if we can know the sram address in SOC directly for vdi layer. it is possible to set in vdi layer without allocation from driver
#ifdef SUPPORT_MINI_PIPPEN
#	    define VDI_WAVE410_SRAM_SIZE		0x30000     // 8Kx8K MAIN10 MAX size
#else
#	    define VDI_WAVE410_SRAM_SIZE		0x25000     // 8Kx8K MAIN10 MAX size
#endif
#	    define VDI_WAVE420_SRAM_SIZE		0x2E000     // 8Kx8X MAIN10 MAX size
#	    define VDI_CODA9_SRAM_SIZE			0x34600     // FHD MAX size, 0x17D00  4K MAX size 0x34600
#	else
#		define VDI_SRAM_BASE_ADDR	0xf0000000	// for TCC980X
#ifdef SUPPORT_MINI_PIPPEN
#	    define VDI_WAVE410_SRAM_SIZE		0x30000     // 8Kx8K MAIN10 MAX size
#else
#	    define VDI_WAVE410_SRAM_SIZE		0x25000     // 8Kx8K MAIN10 MAX size
#endif
#	    define VDI_WAVE420_SRAM_SIZE		0x2E000     // 8Kx8X MAIN10 MAX size
#	    define VDI_CODA9_SRAM_SIZE			0x34600     // FHD MAX size, 0x17D00  4K MAX size 0x34600
#	endif
#   define VDI_SYSTEM_ENDIAN				VDI_LITTLE_ENDIAN
#   define VDI_128BIT_BUS_SYSTEM_ENDIAN 	VDI_128BIT_LITTLE_ENDIAN
#endif
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
#ifdef CNM_FPGA_PLATFORM
	int vpu_allocated_buffer_ref_count;
    uint32_t    hpiBusLength;
	void* io_mutex;
#endif /* CNM_FPGA_PLATFORM */

} vdi_info_t;

#ifdef CNM_FPGA_PLATFORM	
int vdi_current_aclk = 20;
int vdi_current_cclk = 20;
int vdi_current_vclk = 20;
vdi_info_t s_vdi_info[MAX_NUM_VPU_CORE];
#else
static vdi_info_t s_vdi_info[MAX_NUM_VPU_CORE];
#endif

static int swap_endian(unsigned long core_idx, unsigned char *data, int len, int endian);
static int allocate_common_memory(unsigned long core_idx);

#ifdef SUPPORT_CONF_TEST
#ifdef CNM_FPGA_USB_INTERFACE
/* To avoid running more than two qctool at same time.
 * In this case, Jenkins shell script will assert SIGTERM signal.
 */
static uint32_t runningCores[MAX_NUM_VPU_CORE];
static BOOL     isIOLocked;
static void TermSigHandler(
    int no
    )
{
    int32_t i;
    int32_t timeoutCount;
    extern int usb_chip_reset(int);

    for (i=0; i<MAX_NUM_VPU_CORE; i++) {
        if (runningCores[i] == TRUE) {
            timeoutCount = 0;
            if (vdi_lock_check(i) == 0) {
                vdi_unlock(i);
            }
            if (isIOLocked == TRUE) {
                io_unlock(i);
            }
            printf("USB_TRANSACTION_DOING: %d\n", USB_TRANSACTION_DOING);
            vdi_hw_reset(i);
            vdi_release(i);
        }
    }

#ifdef	_KERNEL_
    udelay(1000*1000);
#else
    usleep(1000*1000);
#endif // _KERNEL_
    VLOG(ERR, "HI....\n");

    exit(10);
}
#endif /* CNM_FPGA_USB_INTERFACE */
#endif /* SUPPORT_CONF_TEST */

int vdi_probe(unsigned long core_idx)
{
    int ret;

    ret = vdi_init(core_idx);
#ifdef CNM_FPGA_PLATFORM
#ifdef CNM_FPGA_USB_INTERFACE
    if (ret == 0)
    {
        // to probe PROM is programed. only read operation can check an error of USB
        if(vdi_read_register(core_idx, VPU_PRODUCT_CODE_REGISTER) == -1) 
            ret = -2;
    }
#endif

#else
    vdi_release(core_idx);
#endif	
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

#ifdef SUPPORT_CONF_TEST
#ifdef CNM_FPGA_USB_INTERFACE
    struct sigaction termsig;

    termsig.sa_handler = TermSigHandler;
    sigemptyset(&termsig.sa_mask);
    termsig.sa_flags = 0;

    if (sigaction(SIGTERM, &termsig, 0) == -1) {
        VLOG(ERR, "Failed to register signal handler\n");
    }
    runningCores[core_idx] = TRUE;
#endif /* CNM_FPGA_USB_INTERFACE */
#endif /* SUPPORT_CONF_TEST */

#ifdef SHIP_BUILD
#else
#ifdef ANDROID
    system("/system/lib/modules/load_android.sh");
#endif
#endif

    vdi->vpu_fd = open(VPU_DEVICE_NAME, O_RDWR);	// if this API supports VPU parallel processing using multi VPU. the driver should be made to open multiple times.
    if (vdi->vpu_fd < 0) {
        VLOG(ERR, "[VDI] Can't open vpu driver. [error=%s]. try to run vdi/qnx/driver/load.sh script \n", strerror(errno));		
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
#if defined(CNM_FPGA_PLATFORM) && defined(CNM_FPGA_USB_INTERFACE) 
        // no needed register virtual address for usb interface due to usb interface is not linear type of bus.
        vdi->vdb_register.virt_addr = 0x00;
#else
        VLOG(ERR, "[VDI] fail to map vpu registers \n");
        goto ERR_VDI_INIT;
#endif
    }

    VLOG(INFO, "[VDI] map vdb_register core_idx=%d, virtaddr=0x%lx, size=%d\n", core_idx, vdi->vdb_register.virt_addr, vdi->vdb_register.size);

    vdi_set_clock_gate(core_idx, 1);

#ifdef CNM_FPGA_PLATFORM		
    //pthread_mutex_init(&vdi->io_mutex, NULL);
#ifdef CNM_FPGA_USB_INTERFACE
    usb_init(core_idx, VDI_DRAM_PHYSICAL_BASE);
	usb_chip_reset((int)vdi->vpu_fd);
#endif
#ifdef CNM_FPGA_PCI_INTERFACE
    hpi_init(core_idx, VDI_DRAM_PHYSICAL_BASE);
#endif
#endif

    vdi->product_code = vdi_read_register(core_idx, VPU_PRODUCT_CODE_REGISTER);
#ifdef SUPPORT_CONF_TEST
    if ( vdi->product_code == -200 )
        VLOG(INFO, "[VDI] vdi->product_code=%x %s\n", vdi->product_code, vdi->product_code==-200?"HPI timeout":"");
#endif

#ifdef CNM_FPGA_PLATFORM	
    switch (vdi->product_code) {
    case WAVE410_CODE:
    case WAVE4102_CODE:
    case WAVE412_CODE:
    case WAVE420_CODE:
    case CODA7Q_CODE:
        vdi->hpiBusLength = 16;
        break;
    default:
        vdi->hpiBusLength = 8;
        break;
    }
#else
    if (PRODUCT_CODE_W_SERIES(vdi->product_code))
    {
        if (vdi_read_register(core_idx, W4_VCPU_CUR_PC) == 0) // if BIT processor is not running.
        {
            for (i=0; i<64; i++)
                vdi_write_register(core_idx, (i*4) + 0x100, 0x0); 
        }
    }
    else // CODA9XX
    {
        if (vdi_read_register(core_idx, BIT_CUR_PC) == 0) // if BIT processor is not running.
        {
            for (i=0; i<64; i++)
                vdi_write_register(core_idx, (i*4) + 0x100, 0x0); 
        }
    }
#endif

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

#ifdef TEST_SLEEP_WAKE_DRIVER
int vdi_sleep_wake(unsigned long core_idx, int iSleepWake)
{
    vdi_info_t *vdi;

    if (core_idx >= MAX_NUM_VPU_CORE)
        return -1;

    vdi = &s_vdi_info[core_idx];

    if (!vdi || vdi->vpu_fd == -1 || vdi->vpu_fd == 0x00)
        return 0;

    if ( iSleepWake == 1 )
        return ioctl((int)vdi->vpu_fd, VDI_IOCTL_SLEEP, NULL);
    else
        return ioctl((int)vdi->vpu_fd, VDI_IOCTL_WAKEUP, NULL);
}
#endif

int vdi_release(unsigned long core_idx)
{
    int i;
    vpudrv_buffer_t vdb;
    vdi_info_t *vdi;
#ifdef CNM_FPGA_PLATFORM
    BOOL    hasMemoryLeak = FALSE;
#endif

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

#ifdef CNM_FPGA_PLATFORM
    if (vdi->vpu_allocated_buffer_ref_count != 0) {
        hasMemoryLeak = TRUE;
        VLOG(ERR, "[VDI] fail to commit memory allocator ref_count=%d\n", vdi->vpu_allocated_buffer_ref_count);		
    }
#endif

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
#ifdef CNM_FPGA_PLATFORM

#else
        munmap((void *)vdb.virt_addr, vdb.size);
#endif
        memset(&vdi->vpu_common_memory, 0x00, sizeof(vpu_buffer_t));
    }

    vdi->task_num--;

    if (vdi->vpu_fd != -1 && vdi->vpu_fd != 0x00)
    {
        close(vdi->vpu_fd);
        vdi->vpu_fd = -1;

#ifdef CNM_FPGA_PLATFORM
#ifdef CNM_FPGA_USB_INTERFACE
        usb_release(core_idx);
#endif
#ifdef CNM_FPGA_PCI_INTERFACE
        hpi_release(core_idx);
#endif
#endif
    }

    memset(vdi, 0x00, sizeof(vdi_info_t));
#ifdef SUPPORT_CONF_TEST
#ifdef CNM_FPGA_USB_INTERFACE
    runningCores[core_idx] = FALSE;
#endif /* CNM_FPGA_USB_INTERFACE */
#endif /* SUPPORT_CONF_TEST */

#ifdef CNM_FPGA_PLATFORM
    return hasMemoryLeak == TRUE ? -1 : 0;
#else
    return 0;
#endif
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

#ifdef CNM_FPGA_PLATFORM
    vdb.virt_addr = (unsigned long)vdb.phys_addr;		
#else
    vdb.virt_addr = (unsigned long)mmap(NULL, vdb.size, PROT_READ | PROT_WRITE, MAP_SHARED, vdi->vpu_fd, vdb.phys_addr);
    if ((void *)vdb.virt_addr == MAP_FAILED) 
    {
        VLOG(ERR, "[VDI] fail to map common memory phyaddr=0x%lx, size = %d\n", (int)vdb.phys_addr, (int)vdb.size);
        return -1;
    }
#endif

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

        vdi->pvip = (vpu_instance_pool_t *)(vdb.virt_addr + (core_idx*(sizeof(vpu_instance_pool_t) + sizeof(MUTEX_HANDLE)*VDI_NUM_LOCK_HANDLES)));

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

#ifdef CNM_FPGA_PLATFORM
#ifdef CNM_FPGA_USB_INTERFACE
    return usb_hw_reset(vdi->vpu_fd);
#endif	
#ifdef CNM_FPGA_PCI_INTERFACE
    return hpi_hw_reset((void *)vdi->vdb_register.virt_addr);
#endif
#else
    return ioctl(vdi->vpu_fd, VDI_IOCTL_RESET, 0);
#endif

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
    while(1)
    {
        int _ret, i;
        for(i = 0; (_ret = pthread_mutex_trylock((MUTEX_HANDLE *)vdi->vpu_mutex)) != 0 && i < MUTEX_TIMEOUT; i++) 
        {

            if(i == 0) 
                VLOG(ERR, "vdi_lock: mutex is already locked - try again\n");				
#ifdef	_KERNEL_
            udelay(1*1000);
#else
            usleep(1*1000);
#endif // _KERNEL_

            if (PRODUCT_CODE_W_SERIES(product_code))
            {
                if (i == 1000) // check whether vpu is really busy 
                {
                    if (vdi_read_register(core_idx, W4_VPU_BUSY_STATUS) == 0)
                        break;
                }

                if (i > VPU_BUSY_CHECK_TIMEOUT)  
                {
                    if (vdi_read_register(core_idx, W4_VPU_BUSY_STATUS) == 1)
                    {
                        vdi_write_register(core_idx, W4_BS_OPTION, 0x03);
                        vdi_write_register(core_idx, W4_VINT_REASON_CLEAR, 0);
                        vdi_write_register(core_idx, W4_VPU_INT_CLEAR, 1);					
                    }
                    break;
                }
            }
            else
            {
                if (i == 1000) // check whether vpu is really busy 
                {
                    if (vdi_read_register(core_idx, BIT_BUSY_FLAG) == 0)
                        break;
                }

                if (i > VPU_BUSY_CHECK_TIMEOUT)  
                {
                    if (vdi_read_register(core_idx, BIT_BUSY_FLAG) == 1)
                    {
                        vdi_write_register(core_idx, BIT_BIT_STREAM_PARAM, (1 << 2));
                        vdi_write_register(core_idx, BIT_INT_REASON, 0);
                        vdi_write_register(core_idx, BIT_INT_CLEAR, 1);					
                    }
                    break;
                }
            }
        }

        if(_ret == 0) 
            break;

        VLOG(ERR, "vdi_lock: can't get lock - force to unlock and clear pendingInst[%d:%s]\n", _ret, strerror(_ret));
        if (_ret == EINVAL) {
            Uint32* pInt = (Uint32*)vdi->vpu_mutex;
            *pInt = 0xdead10cc;
            restore_mutex_in_dead((MUTEX_HANDLE*)vdi->vpu_mutex);
        }
        vdi_unlock(core_idx);
        vdi->pvip->pendingInst = NULL;	
        vdi->pvip->pendingInstIdxPlus1 = 0;
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
    while(1)
    {
        int _ret, i;
        for(i = 0; (_ret = pthread_mutex_trylock((MUTEX_HANDLE *)vdi->vpu_disp_mutex)) != 0 && i < MUTEX_TIMEOUT; i++)
        {
            if(i == 0)
                VLOG(ERR, "vdi_disp_lock: mutex is already locked - try again\n");                
#ifdef	_KERNEL_
            udelay(1*1000);
#else
            usleep(1*1000);
#endif // _KERNEL_
        }

        if(_ret == 0)
            break;

        if (_ret == EINVAL) {
            Uint32* pInt = (Uint32*)vdi->vpu_disp_mutex;
            *pInt = 0xdead10cc;
            restore_mutex_in_dead((MUTEX_HANDLE*)vdi->vpu_disp_mutex);
        }

        VLOG(ERR, "vdi_disp_lock: can't get lock - force to unlock. [%d:%s]\n", _ret, strerror(_ret));
        vdi_disp_unlock(core_idx);
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
#ifdef CNM_FPGA_PLATFORM
#else
    unsigned long *reg_addr;
#endif

    if (core_idx >= MAX_NUM_VPU_CORE)
        return;

    vdi = &s_vdi_info[core_idx];

    if(!vdi || vdi->vpu_fd == -1 || vdi->vpu_fd == 0x00)
        return;

#ifdef CNM_FPGA_PLATFORM		
    if(!vdi->clock_state)
    { 
        VLOG(ERR, "[VDI] vdi_write_register clock is in off. enter infinite loop\n");
        while (1); 
    }
#endif

#ifdef CNM_FPGA_PLATFORM
#ifdef SUPPORT_MULTI_CORE_IN_ONE_DRIVER
#ifdef CNM_FPGA_USB_INTERFACE
    usb_write_register(core_idx, vdi->vpu_fd, VPU_BIT_REG_BASE + (core_idx*VPU_CORE_BASE_OFFSET) + addr, data);
#endif
#ifdef CNM_FPGA_PCI_INTERFACE
    hpi_write_register(core_idx, (void *)vdi->vdb_register.virt_addr, VPU_BIT_REG_BASE + (core_idx*VPU_CORE_BASE_OFFSET) + addr, data);
#endif	
#else
#ifdef CNM_FPGA_USB_INTERFACE
    usb_write_register(core_idx, vdi->vpu_fd, VPU_BIT_REG_BASE+ addr, data);
#endif	
#ifdef CNM_FPGA_PCI_INTERFACE
    hpi_write_register(core_idx, (void *)vdi->vdb_register.virt_addr, VPU_BIT_REG_BASE+ addr, data);
#endif	
#endif	
#else
#ifdef SUPPORT_MULTI_CORE_IN_ONE_DRIVER
    reg_addr = (unsigned long *)(addr + (unsigned long)vdi->vdb_register.virt_addr + (core_idx*VPU_CORE_BASE_OFFSET));
#else
    reg_addr = (unsigned long *)(addr + (unsigned long)vdi->vdb_register.virt_addr);
#endif
    *(volatile unsigned int *)reg_addr = data;	
#endif
}

unsigned int vdi_read_register(unsigned long core_idx, unsigned int addr)
{
    vdi_info_t *vdi;
#ifdef CNM_FPGA_PLATFORM
#else
	unsigned long *reg_addr;
#endif

    if (core_idx >= MAX_NUM_VPU_CORE)
        return (unsigned int)-1;

    vdi = &s_vdi_info[core_idx];

    if(!vdi || vdi->vpu_fd == -1 || vdi->vpu_fd == 0x00)
        return (unsigned int)-1;

#ifdef CNM_FPGA_PLATFORM		
    if(!vdi->clock_state)
    { 
        VLOG(ERR, "[VDI] vdi_read_register clock is in off. enter infinite loop\n");
        while (1); 
    }
#endif

#ifdef CNM_FPGA_PLATFORM
#ifdef SUPPORT_MULTI_CORE_IN_ONE_DRIVER
#ifdef CNM_FPGA_USB_INTERFACE
    return usb_read_register(core_idx, vdi->vpu_fd, VPU_BIT_REG_BASE + (core_idx*VPU_CORE_BASE_OFFSET) + addr);
#endif
#ifdef CNM_FPGA_PCI_INTERFACE
    return hpi_read_register(core_idx, (void *)vdi->vdb_register.virt_addr, VPU_BIT_REG_BASE + (core_idx*VPU_CORE_BASE_OFFSET) + addr);
#endif
#else
#ifdef CNM_FPGA_USB_INTERFACE
    return usb_read_register(core_idx, vdi->vpu_fd, VPU_BIT_REG_BASE+addr);
#endif
#ifdef CNM_FPGA_PCI_INTERFACE
    return hpi_read_register(core_idx, (void *)vdi->vdb_register.virt_addr, VPU_BIT_REG_BASE+addr);
#endif
#endif

#else
#ifdef SUPPORT_MULTI_CORE_IN_ONE_DRIVER
    reg_addr = (unsigned long *)(addr + (unsigned long)vdi->vdb_register.virt_addr + (core_idx*VPU_CORE_BASE_OFFSET));	
#else
    reg_addr = (unsigned long *)(addr + (unsigned long)vdi->vdb_register.virt_addr);
#endif
    return *(volatile unsigned int *)reg_addr;
#endif
}

#ifdef SUPPORT_FIO_ACCESS
#define FIO_TIMEOUT         10000

unsigned int vdi_fio_read_register(unsigned long core_idx, unsigned int addr)
{
    unsigned int ctrl;
    unsigned int count = 0;
    unsigned int data  = 0xffffffff;

    ctrl  = (addr&0xffff);
    ctrl |= (0<<16);    /* read operation */
    vdi_write_register(core_idx, W4_VCPU_FIO_CTRL, ctrl);
    count = FIO_TIMEOUT;
    while (count--) {
        ctrl = vdi_read_register(core_idx, W4_VCPU_FIO_CTRL);
        if (ctrl & 0x80000000) {
            data = vdi_read_register(core_idx, W4_VCPU_FIO_DATA);
            break;
        }
    }

    return data;
}

void vdi_fio_write_register(unsigned long core_idx, unsigned int addr, unsigned int data)
{
    unsigned int ctrl;
    
    vdi_write_register(core_idx, W4_VCPU_FIO_DATA, data);
    ctrl  = (addr&0xffff);
    ctrl |= (1<<16);    /* write operation */
    vdi_write_register(core_idx, W4_VCPU_FIO_CTRL, ctrl);
}

#define VCORE_DBG_ADDR(__vCoreIdx)      0x8000+(0x1000*__vCoreIdx) + 0x300
#define VCORE_DBG_DATA(__vCoreIdx)      0x8000+(0x1000*__vCoreIdx) + 0x304
#define VCORE_DBG_READY(__vCoreIdx)     0x8000+(0x1000*__vCoreIdx) + 0x308

static void write_vce_register(
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

	if (vdi_fio_read_register(0, VCORE_DBG_READY(vce_core_idx)) < 0) 
		VLOG(ERR, "failed to write VCE register: 0x%04x\n", vce_addr);

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

	if (vdi_fio_read_register(0, VCORE_DBG_READY(vce_core_idx)) == 1)
		udata= vdi_fio_read_register(0, VCORE_DBG_DATA(vce_core_idx));
	else 
		udata = -1;

	return udata;
}

#endif /* SUPPORT_FIO_ACCESS */

int vdi_clear_memory(unsigned long core_idx, unsigned int addr, int len, int endian)
{
    vdi_info_t *vdi;
    vpudrv_buffer_t vdb;
#ifdef CNM_FPGA_PLATFORM
#else
	unsigned long offset;
#endif
    
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

#ifdef CNM_FPGA_PLATFORM
#ifdef CNM_FPGA_PCI_INTERFACE
    hpi_write_memory(core_idx, (void *)vdi->vdb_register.virt_addr, vdi->hpiBusLength, addr, zero, len, endian);
#endif
#ifdef CNM_FPGA_USB_INTERFACE
    usb_write_memory(core_idx, vdi->vpu_fd, addr, zero, len, endian);
#endif
#else
	offset = addr - (unsigned long)vdb.phys_addr;
    osal_memcpy((void *)((unsigned long)vdb.virt_addr+offset), zero, len);	
#endif

    osal_free(zero);

    return len;
}

int vdi_write_memory(unsigned long core_idx, unsigned int addr, unsigned char *data, int len, int endian)
{
    vdi_info_t *vdi;
    vpudrv_buffer_t vdb;
#ifdef CNM_FPGA_PLATFORM
#else
    unsigned long offset;
#endif
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

    if (!vdb.size) {
        VLOG(ERR, "address 0x%08x is not mapped address!!!\n", (int)addr);
        return -1;
    }

#ifdef CNM_FPGA_PLATFORM
	// to check some writing of common buffer 
	if (vdb.phys_addr >= vdi->vpu_common_memory.phys_addr &&
		vdb.phys_addr < (vdi->vpu_common_memory.phys_addr + SIZE_COMMON))
	{
        if (PRODUCT_CODE_W_SERIES(vdi->product_code))
		{
			if (vdi_read_register(core_idx, W4_VCPU_CUR_PC) != 0) 
			{
				VLOG(ERR, "not to permit writing common buffer addr=%lx, size=%d\n", vdb.phys_addr, vdb.size);
				VLOG(ERR, "this process will be exit\n");
				exit(-1);					
				return -1;
			}
		}
	}
#endif
    
#ifdef CNM_FPGA_PLATFORM
#ifdef CNM_FPGA_USB_INTERFACE
    usb_write_memory(core_idx, vdi->vpu_fd, addr, data, len, endian);
#endif	
#ifdef CNM_FPGA_PCI_INTERFACE
    hpi_write_memory(core_idx, (void *)vdi->vdb_register.virt_addr, vdi->hpiBusLength, addr, data, len, endian);
#endif
#else
	offset = addr - (unsigned long)vdb.phys_addr;
    swap_endian(core_idx, data, len, endian);
    osal_memcpy((void *)((unsigned long)vdb.virt_addr+offset), data, len);	
#endif

    return len;
}

int vdi_read_memory(unsigned long core_idx, unsigned int addr, unsigned char *data, int len, int endian)
{
    vdi_info_t *vdi;
    vpudrv_buffer_t vdb;
#ifdef CNM_FPGA_PLATFORM
#else
    unsigned long offset;
#endif
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
        return -1;

#ifdef CNM_FPGA_PLATFORM
#ifdef CNM_FPGA_USB_INTERFACE
    usb_read_memory(core_idx, vdi->vpu_fd, addr, data, len, endian);
#endif
#ifdef CNM_FPGA_PCI_INTERFACE
    hpi_read_memory(core_idx, (void *)vdi->vdb_register.virt_addr, vdi->hpiBusLength, addr, data, len, endian);
#endif
#else
	offset = addr - (unsigned long)vdb.phys_addr;	
    osal_memcpy(data, (const void *)((unsigned long)vdb.virt_addr+offset), len);
    swap_endian(core_idx, data, len,  endian);
#endif

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

#ifdef CNM_FPGA_PLATFORM	
#ifdef SUPPORT_OMXIL_VPU_COMPONENT
	vdb.virt_addr = (unsigned long)osal_malloc(vdb.size);
#else
    vdb.virt_addr  = (unsigned long)vb->phys_addr;
#endif
	vb->virt_addr = vdb.virt_addr;
#else
    //map to virtual address
    vdb.virt_addr = (unsigned long)mmap(NULL, vdb.size, PROT_READ | PROT_WRITE,
        MAP_SHARED, vdi->vpu_fd, vdb.phys_addr);
    if ((void *)vdb.virt_addr == MAP_FAILED) 
    {
        memset(vb, 0x00, sizeof(vpu_buffer_t));
        return -1;
    }
    vb->virt_addr = vdb.virt_addr;
#endif

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
#ifdef CNM_FPGA_PLATFORM
	vdi->vpu_allocated_buffer_ref_count++;
#endif
	VLOG(INFO, "[VDI] vdi_allocate_dma_memory, physaddr=%p, virtaddr=%p, size=%d\n", vb->phys_addr, vb->virt_addr, vb->size);
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

#ifdef CNM_FPGA_PLATFORM
    vdb.virt_addr = vb->phys_addr;
#else
    vdb.virt_addr = vb->virt_addr;
#endif

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

#ifdef CNM_FPGA_PLATFORM
#ifdef SUPPORT_OMXIL_VPU_COMPONENT
	if (vdb.virt_addr)
		free((void *)vdb.virt_addr);
#endif
#else
    if (munmap((void *)vdb.virt_addr, vdb.size) != 0)
    {
        VLOG(ERR, "[VDI] fail to vdi_free_dma_memory virtial address = 0x%lx\n", (int)vdb.virt_addr);					
    }
#endif
    osal_memset(vb, 0, sizeof(vpu_buffer_t));
#ifdef CNM_FPGA_PLATFORM
	vdi->vpu_allocated_buffer_ref_count--;
#endif
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
            sram_size = VDI_WAVE410_SRAM_SIZE; break;
        case WAVE420_CODE:
            sram_size = VDI_WAVE420_SRAM_SIZE; break;
        case CODA7Q_CODE:
            sram_size = VDI_CODA9_SRAM_SIZE; break; // fix me
        default:
            sram_size = VDI_CODA9_SRAM_SIZE; break;
    }

    if (sram_size > 0)	// if we can know the sram address directly in vdi layer, we use it first for sdram address
    {
#ifdef CNM_FPGA_PLATFORM
        vb->phys_addr = VDI_SRAM_BASE_ADDR;		// CNM FPGA platform has different SRAM per core
#else
        vb->phys_addr = VDI_SRAM_BASE_ADDR+(core_idx*sram_size);		
#endif
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

#ifdef CNM_FPGA_PLATFORM
	io_lock(core_idx);
#endif /* CNM_FPGA_PLATFORM */
    vdi->clock_state = enable;
    ret = ioctl(vdi->vpu_fd, VDI_IOCTL_SET_CLOCK_GATE, &enable);	
#ifdef CNM_FPGA_PLATFORM
	io_unlock(core_idx);
#endif /* CNM_FPGA_PLATFORM */

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
#ifdef SUPPORT_FIO_ACCESS
    vdi_info_t *vdi;

    vdi = &s_vdi_info[core_idx];
#endif
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    gettimeofday(&tv, NULL);
    elapse = tv.tv_sec*1000 + tv.tv_usec/1000;
	  
    while(1)
    {
#ifdef SUPPORT_FIO_ACCESS
        if (PRODUCT_CODE_W_SERIES(vdi->product_code)) {
            if (vdi_fio_read_register(core_idx, gdi_busy_flag) == 0x738) break;
        }
        else {
            if (vdi_read_register(core_idx, gdi_busy_flag) == 0x77) break;
        }
#else
        if (vdi_read_register(core_idx, gdi_busy_flag) == 0x77) 
            break;
#endif

        gettimeofday(&tv, NULL);
        cur = tv.tv_sec * 1000 + tv.tv_usec / 1000;

        if ((cur - elapse) > timeout)
        {
            VLOG(ERR, "[VDI] vdi_wait_bus_busy timeout, PC=0x%lx\n", vdi_read_register(core_idx, 0x018));
            return -1;
        }
#ifdef CNM_FPGA_PLATFORM        
#ifdef	_KERNEL_	//do not use in real system.
        udelay(1*1000);
#else
        usleep(1*1000);
#endif // _KERNEL_		
#endif
    }
    return 0;

}

int vdi_wait_vpu_busy(unsigned long core_idx, int timeout, unsigned int addr_bit_busy_flag)
{

    Int64 elapse, cur;
    struct timeval tv;
    Uint32 pc, code;
    Uint32 normalReg = TRUE;

    tv.tv_sec = 0;
    tv.tv_usec = 0;
    gettimeofday(&tv, NULL);
    elapse = tv.tv_sec*1000 + tv.tv_usec/1000;

    code = vdi_read_register(core_idx, VPU_PRODUCT_CODE_REGISTER); /* read product code */

    if (PRODUCT_CODE_W_SERIES(code)) {
        pc = W4_VCPU_CUR_PC;
        if (addr_bit_busy_flag&0x8000) normalReg = FALSE;
    }
    else {
        pc = BIT_CUR_PC;
    }

    while(1)
    {
#ifdef SUPPORT_FIO_ACCESS
        if (normalReg == TRUE) {
            if (vdi_read_register(core_idx, addr_bit_busy_flag) == 0) break;
        }
        else {
            if (vdi_fio_read_register(core_idx, addr_bit_busy_flag) == 0) break;
        }
#else
        if (vdi_read_register(core_idx, addr_bit_busy_flag) == 0)
            break;
#endif

        gettimeofday(&tv, NULL);
        cur = tv.tv_sec * 1000 + tv.tv_usec / 1000;

        if ((cur - elapse) > timeout) {
            Uint32 index;
            for (index=0; index<50; index++) {
                VLOG(ERR, "[VDI] vdi_wait_vpu_busy timeout, PC=0x%lx\n", vdi_read_register(core_idx, pc));
#ifdef CNM_FPGA_PLATFORM                
				#ifdef	_KERNEL_	//do not use in real system.
				        udelay(1*1000);
				#else
				        usleep(1*1000);
				#endif // _KERNEL_	
#endif
            }
            return -1;
        }
#ifdef CNM_FPGA_PLATFORM        
#ifdef	_KERNEL_	//do not use in real system.
        udelay(1*1000);
#else
        usleep(1*1000);
#endif // _KERNEL_		
#endif
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
    uint32_t        intrStatusReg;
    uint32_t        pc;
    int32_t         startTime, endTime, elaspedTime;

    UNREFERENCED_PARAMETER(intr_info);

    if (PRODUCT_CODE_W_SERIES(vdi->product_code)) {
        pc            = W4_VCPU_CUR_PC;
        intrStatusReg = W4_VPU_INT_STS;
    }
    else {
        pc            = BIT_CUR_PC;
        intrStatusReg = BIT_INT_STS;
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
        if ((endTime-startTime) >= timeout) {
            return -1;
        }
#ifdef CNM_FPGA_PLATFORM  
        MSleep(0);
#endif        
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

    if (val == CODA960_CODE || val == BODA950_CODE) 
        productId = VDI_PRODUCT_ID_960;
    else if (val == CODA980_CODE || val == WAVE320_CODE) 
        productId = VDI_PRODUCT_ID_980;
    else if (PRODUCT_CODE_W_SERIES(val)) 
        return;

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
#ifdef SUPPORT_FIO_ACCESS
		unsigned int    tq, ip, mc, lf;
		unsigned int    avail_cu, avail_tu, avail_tc, avail_lf, avail_ip;
		unsigned int	 ctu_fsm, nb_fsm, cabac_fsm, cu_info, mvp_fsm, tc_busy, lf_fsm, bs_data, bbusy, fv;
		unsigned int    reg_val;
#endif /* SUPPORT_FIO_ACCESS */
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
#ifdef SUPPORT_FIO_ACCESS
		printf("[-] VCPU REG Dump\n");
		// --------- BIT register Dump 
		printf("[+] BPU REG Dump\n");
		printf("BITPC = 0x%08x\n", vdi_fio_read_register(coreIdx, (W4_REG_BASE + 0x8000 + 0x18))); 
		printf("BIT START=0x%08x, BIT END=0x%08x\n", vdi_fio_read_register(coreIdx, (W4_REG_BASE + 0x8000 + 0x11c)), 
			vdi_fio_read_register(coreIdx, (W4_REG_BASE + 0x8000 + 0x120)) );
		if (product_code == WAVE410_CODE )
			printf("BIT COMMAND 0x%x\n", vdi_fio_read_register(coreIdx, (W4_REG_BASE + 0x8000 + 0x100))); 
		if (product_code == WAVE4102_CODE) 
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
#endif /* SUPPORT_FIO_ACCESS */

		printf("-------------------------------------------------------------------------------\n");
	}
	else
	{

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
		case CHANGE_INST:
            vdi_make_log(core_idx, "CHANGE_INST", step);
			break;
		case SET_WRITE_PROTECTION:
            vdi_make_log(core_idx, "SET_WRITE_PROTECTION", step);
			break;
		case CREATE_INSTANCE:
            vdi_make_log(core_idx, "CREATE_INSTANCE", step);
			break;
		case UPDATE_BS:
            vdi_make_log(core_idx, "UPDATE_BS", step);
			break;			 
		case RESET_VPU:
			vdi_make_log(core_idx, "RESET_VPU", step);
			break;	
		default:
            vdi_make_log(core_idx, "ANY_CMD", step);
			break;
		}
	}
	else
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
	else
	{
		if ((cmd == PIC_RUN && step== 0) || cmd == VPU_RESET)
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

    if (PRODUCT_CODE_W_SERIES(vdi->product_code))
        return VDI_128BIT_BUS_SYSTEM_ENDIAN;
    else
        return VDI_SYSTEM_ENDIAN;
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

#ifdef SUPPORT_ENC_ELAPSED_TIME
unsigned int vdi_get_current_time_ms()
{
    struct timespec ts;

    clock_gettime(CLOCK_REALTIME, &ts);

    return (ts.tv_sec*1000 + ts.tv_nsec/1000000.0);
}
#endif

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

    if (PRODUCT_CODE_W_SERIES(vdi->product_code))
    {
        sys_endian = VDI_128BIT_BUS_SYSTEM_ENDIAN;
    }
    else
    {
        sys_endian = VDI_SYSTEM_ENDIAN;
    }

    endian     = vdi_convert_endian(core_idx, endian);
    sys_endian = vdi_convert_endian(core_idx, sys_endian);
    if (endian == sys_endian)
        return 0;

    if (PRODUCT_CODE_NOT_W_SERIES(vdi->product_code)) {
        endian     = convert_endian_coda9_to_wave4(endian);
        sys_endian = convert_endian_coda9_to_wave4(sys_endian);
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

#ifdef CNM_FPGA_PLATFORM
void vdi_set_trace(unsigned long coreIdx, BOOL enable)
{
    s_EnableTrace = enable;
}

static int io_lock(unsigned long core_idx)
{
    vdi_info_t *vdi;

    if (core_idx >= MAX_NUM_VPU_CORE)
        return -1;

    vdi = &s_vdi_info[core_idx];

    if(!vdi || vdi->vpu_fd==-1 || vdi->vpu_fd == 0x00)
        return -1;
#ifdef SUPPORT_CONF_TEST
#ifdef CNM_FPGA_USB_INTERFACE
    isIOLocked = TRUE;
#endif /* CNM_FPGA_USB_INTERFACE */
#endif
    return ioctl(vdi->vpu_fd, VDI_IOCTL_IO_LOCK, NULL);
}

static int io_unlock(unsigned long core_idx)
{
    vdi_info_t *vdi;
    int         ret;

    if (core_idx >= MAX_NUM_VPU_CORE)
        return -1;

    vdi = &s_vdi_info[core_idx];

    if(!vdi || vdi->vpu_fd==-1 || vdi->vpu_fd == 0x00)
        return -1;

    ret = ioctl(vdi->vpu_fd, VDI_IOCTL_IO_UNLOCK, NULL);
#ifdef SUPPORT_CONF_TEST
#ifdef CNM_FPGA_USB_INTERFACE
    isIOLocked = FALSE;
#endif /* CNM_FPGA_USB_INTERFACE */
#endif
    return ret;
}

int vdi_set_timing_opt(unsigned long core_idx )
{
    vdi_info_t *vdi;

    if (core_idx >= MAX_NUM_VPU_CORE)
        return -1;

    vdi = &s_vdi_info[core_idx];

    if(!vdi || vdi->vpu_fd==-1 || vdi->vpu_fd == 0x00)
        return -1;

#ifdef CNM_FPGA_USB_INTERFACE
    return usb_set_timing_opt(core_idx, vdi->vpu_fd);
#endif
#ifdef CNM_FPGA_PCI_INTERFACE
    return hpi_set_timing_opt(core_idx, (void *)vdi->vdb_register.virt_addr);
#endif
}

int vdi_set_clock_freg(unsigned long core_idx, int Device, int OutFreqMHz)
{
    vdi_info_t *vdi;

    if (core_idx >= MAX_NUM_VPU_CORE)
        return -1;

    vdi = &s_vdi_info[core_idx];

    if(!vdi || vdi->vpu_fd==-1 || vdi->vpu_fd == 0x00)
        return -1;

    if (Device == CLOCK_ID_ACLK)        vdi_current_aclk = OutFreqMHz;
    else if (Device == CLOCK_ID_CCLK)   vdi_current_cclk = OutFreqMHz;
    else if (Device == CLOCK_ID_VCLK)   vdi_current_vclk = OutFreqMHz;
#ifdef CNM_FPGA_USB_INTERFACE
    return usb_ics307m_set_clock_freg(vdi->vpu_fd, Device, OutFreqMHz);
#endif
#ifdef CNM_FPGA_PCI_INTERFACE
    return hpi_ics307m_set_clock_freg((void *)vdi->vdb_register.virt_addr, Device, OutFreqMHz);
#endif
}

void vdi_init_fpga(int core_idx)
{
    uint32_t aclk_freq = 20;
    uint32_t bclk_freq = 20;
    uint32_t cclk_freq = 20;
    uint32_t mclk_freq = 20;
    uint32_t vclk_freq = 20;

	/* configure extra clock: All clock sources are replaced by extra clock by 
	 * calling vdi_ready_change_clock() function
	 */
	vdi_set_clock_freg(core_idx, 5, 16);

	vdi_ready_change_clock(core_idx);
	printf("Set default ACLK to %d\n", aclk_freq);
	vdi_set_clock_freg(core_idx, 0, aclk_freq);	// ACLK	
	printf("Set default CCLK to %d\n", cclk_freq);
	vdi_set_clock_freg(core_idx, 1, cclk_freq);	// CCLK
	//check the default value
#ifdef SUPPORT_2000T
	printf("Set default VCLK to %d\n", vclk_freq);
	vdi_set_clock_freg(core_idx, 4, vclk_freq);	// VCLK	
	printf("Set default BCLK to %d\n", bclk_freq);
	vdi_set_clock_freg(core_idx, 2, bclk_freq);	// BCLK
	printf("Set default MCLK to %d\n", mclk_freq);
	vdi_set_clock_freg(core_idx, 3, mclk_freq);	// MCLK
#else
	printf("Set default VCLK to %d\n", vclk_freq);
	vdi_set_clock_freg(core_idx, 2, vclk_freq);	// VCLK	
	printf("Set default BCLK to %d\n", bclk_freq);
	vdi_set_clock_freg(core_idx, 3, bclk_freq);	// BCLK
	printf("Set default MCLK to %d\n", mclk_freq);
	vdi_set_clock_freg(core_idx, 4, mclk_freq);	// MCLK
#endif
	vdi_done_change_clock(core_idx);

    vdi_set_timing_opt(core_idx);

    vdi_hw_reset(core_idx);
    usleep(500*1000);
}

#ifdef CNM_FPGA_USB_INTERFACE

#define EP_CTRL_IN 0x80
#define EP_CTRL_OUT	0x00
#define EP_IN 0x86
#define EP_OUT 0x02

#define CNM_USB_VENDORID 0x04b4
#define CNM_USB_DEVICEID 0x1004
#define CNM_USB_CONFIG 1
#define CNM_USB_INTF 0

#define CNM_USB_BULK_TIMEOUT 2000
#define CNM_USB_BULK_OUT_SIZE 512
#define CNM_USB_BULK_IN_SIZE 512
#define CNM_USB_MAX_BURST_SIZE 1024
#define CNM_USB_BUS_LEN 16
#define CNM_USB_BUS_LEN_ALIGN 15

#define CNM_USB_CMD_READ (0<<15)
#define CNM_USB_CMD_WRITE (1<<15)
#define CNM_USB_CMD_APB (0<<14)
#define CNM_USB_CMD_AXI (1<<14)
#define CNM_USB_CMD_SIZE(SIZE) (SIZE&0x3FFF)

static unsigned long s_dram_base;

static int usb_axi_read_burst(int base, unsigned int addr, unsigned char *buf,int len);
static int usb_axi_write_burst(int base, unsigned int addr, unsigned char *buf, int len);
static int usb_read_endpoint(int base, unsigned char *buf, int len);
static int usb_write_endpoint(int base, unsigned char *buf, int len);

typedef struct cnm_usb_apb_write_req_data_t {
    unsigned int  addr;
    unsigned int  data;
} cnm_usb_apb_write_req_data_t;

typedef struct cnm_usb_apb_write_req_packet_t {
    unsigned int  cmd;
    cnm_usb_apb_write_req_data_t data[128];
    unsigned char dummy[508];	// to make the size of apb usb packet to 1536(512*3)
} cnm_usb_apb_write_req_packet_t;

typedef struct cnm_usb_apb_write_res_packet_t {
    unsigned char  data[1024];	
} cnm_usb_apb_write_res_packet_t;

typedef struct cnm_usb_apb_read_req_data_t {
    unsigned int  addr;	
} cnm_usb_apb_read_req_data_t;

typedef struct cnm_usb_apb_read_res_packet_t {
    unsigned char  data[1024];	
} cnm_usb_apb_read_res_packet_t;

typedef struct cnm_usb_apb_read_req_packet_t {
    unsigned int  cmd;
    cnm_usb_apb_read_req_data_t data[128];
    unsigned char dummy[1020];	// to make the size of apb usb packet to 1536(512*3)
} cnm_usb_apb_read_req_packet_t;

typedef struct cnm_usb_axi_write_req_packet_t {
    unsigned int  cmd;
    unsigned int  addr;
    unsigned char data[CNM_USB_MAX_BURST_SIZE];
    unsigned char dummy[1536-CNM_USB_MAX_BURST_SIZE-8];	
} cnm_usb_axi_write_req_packet_t;

typedef struct cnm_usb_axi_write_res_packet_t {
    unsigned char  dummy[1024];	
} cnm_usb_axi_write_res_packet_t;

typedef struct cnm_usb_axi_read_req_packet_t {
    unsigned int  cmd;
    unsigned int  addr;
    unsigned char dummy[1528];	
} cnm_usb_axi_read_req_packet_t;

typedef struct cnm_usb_axi_read_res_packet_t {
    unsigned char  data[CNM_USB_MAX_BURST_SIZE];	
    unsigned char  dummy[(1024-CNM_USB_MAX_BURST_SIZE)];	
} cnm_usb_axi_read_res_packet_t;

int vdi_load_prom_via_usb(unsigned long core_idx, int need_init, const char* path)
{

#define	USB_BURST_SIZE	4096
#define	STARTCFG	0xF6
#define	ENDCFG		0xF7
#define	STATUSCFG	0xF8
#define REVERSE(b)		((b * 0x0802LU & 0x22110LU) | (b * 0x8020LU & 0x88440LU)) * 0x10101LU >> 16

    int ret, per, pre_per;
    FILE *fp;
    unsigned int len = 0;
    BYTE *strmBuf=NULL, buffer[USB_BURST_SIZE];
    unsigned int i = 0, j = 0, actual_length;
    int byte, ff_count;
    int cur_pos = -1;
    vpudrv_usb_control_packet_t packet;
    int usb;
    int res = 0;
    vdi_info_t *vdi;
    int retry_count = 0;

RE_TRY_WRITE_PROM:

    retry_count++;
    if (retry_count > 3)
    {
        VLOG(INFO ,"vdi_load_prom_via_usb %s writing FAIL completely\n", path);
        goto ERR_USB_INIT;
    }

    memset(&packet, 0x00, sizeof(vpudrv_usb_control_packet_t));

    if (access(path, F_OK) != 0)
    {
        VLOG(ERR, "%s: File not exist:%s\n", __func__, path);
        return -1;
    }

    if (core_idx >= MAX_NUM_VPU_CORE)
        return -1;

    if ( need_init )
    {
        vdi_init(core_idx);
    }

    vdi = &s_vdi_info[core_idx];

    if(!vdi || vdi->vpu_fd==-1 || vdi->vpu_fd == 0x00)
        return -1;

    usb = vdi->vpu_fd;

    VLOG(INFO ,"%d'st %s writing START \n", retry_count, path);

    fp = fopen(path, "rb");

    ff_count = 0;
    while (!feof(fp)) 
    {
        byte = fgetc(fp);
        if (byte == 0xff) 
        {
            if (cur_pos == -1) 
                cur_pos = ftell(fp)-1;
            ff_count++;
        } 
        else 
        {
            cur_pos = -1;
        }

        if (ff_count == 32) 
            break;
    }

    if (ff_count != 32) 
    {
        fprintf(stderr, "failed to find 32 0xff\n");
        goto ERR_USB_INIT;
    }

    fseek(fp, 0, SEEK_END);
    len = ftell(fp) - cur_pos;
    //printf("cur_pos = %02d\n", cur_pos);

    fseek(fp, cur_pos, SEEK_SET);

    strmBuf = (BYTE *)osal_malloc(len);
    memset(strmBuf, 0, len);
    fread(strmBuf, len, 1, fp);
    fclose(fp);

    packet.dir = 0; // out
    packet.request = ENDCFG;
    packet.val = 0;
    packet.index = 0;
    packet.length = sizeof(packet.data);
    packet.timeout = CNM_USB_BULK_TIMEOUT;
    ret = ioctl(usb, VDI_IOCTL_CONTROL_TRANSFER, &packet);
    if(ret < 0) 
    {
        VLOG(ERR, "[VDI] VDI_IOCTL_CONTROL_TRANSFER  ret=%d, val=%d, index=%d, length=%d, request=0x%lx\n", ret, packet.val, packet.index, packet.length, packet.request);		
        free(strmBuf);
        goto RE_TRY_WRITE_PROM;
    }

    packet.request = STARTCFG;
    packet.length = sizeof(packet.data);
    ret = ioctl(usb, VDI_IOCTL_CONTROL_TRANSFER, &packet);
    if(ret < 0) 
    {
        VLOG(ERR, "[VDI] VDI_IOCTL_CONTROL_TRANSFER  ret=%d, val=%d, index=%d, length=%d, request=0x%lx\n", ret, packet.val, packet.index, packet.length, packet.request);		
        free(strmBuf);
        goto RE_TRY_WRITE_PROM;
    }

    for(i=0; i<len; i++) 
        strmBuf[i] = (unsigned char)(REVERSE(strmBuf[i]));

    for(j=0; j<len; j+= USB_BURST_SIZE)
    {
        ret = (len-j) < USB_BURST_SIZE ? (len-j) : USB_BURST_SIZE;
        memcpy(buffer, strmBuf + j, ret);
        per = j*100/len;
        if ( !(per%10) && per != pre_per )
            printf("%3d%%\r", per);
        pre_per = per;

        if(ret < USB_BURST_SIZE){
            for(i=0; ret+i<USB_BURST_SIZE; i++) buffer[ret+i] = 0;
            ret = ret + i;
        }
        actual_length = usb_write_endpoint(usb, buffer, sizeof(buffer));
        if (actual_length != sizeof(buffer))
        {
            VLOG(ERR, "[VDI] usb_write_endpoint error usb write fail , actual_length=%d\n", actual_length);
            break;
        }
    }

    printf("100%%\n");
    free(strmBuf);

    packet.dir = 0; // out
    packet.request = ENDCFG;
    packet.val = 0;
    packet.index = 0;
    packet.length = sizeof(packet.data);
    packet.timeout = CNM_USB_BULK_TIMEOUT;
    ret = ioctl(usb, VDI_IOCTL_CONTROL_TRANSFER, &packet);
    if(ret < 0)
    {
        VLOG(ERR, "[VDI] VDI_IOCTL_CONTROL_TRANSFER  ret=%d, val=%d, index=%d, length=%d, request=0x%lx\n", ret, packet.val, packet.index, packet.length, packet.request);		
        goto RE_TRY_WRITE_PROM;
    }

    packet.dir = 1; // in
    packet.request = STATUSCFG;
    packet.val = 0;
    packet.index = 0;
    packet.length = sizeof(packet.data);
    packet.timeout = CNM_USB_BULK_TIMEOUT;
    ret = ioctl(usb, VDI_IOCTL_CONTROL_TRANSFER, &packet);
    if(ret < 0)
    {
        VLOG(ERR, "[VDI] VDI_IOCTL_CONTROL_TRANSFER  ret=%d, val=%d, index=%d, length=%d, request=%d\n", ret, packet.val, packet.index, packet.length, packet.request);		
        goto RE_TRY_WRITE_PROM;
    }

    if(packet.data[2]) 
        VLOG(INFO ,"\nvdi_load_prom_via_usb %s writing SUCCESS\n", path);
    else 
    { 
        VLOG(ERR, "\nvdi_load_prom_via_usb %s writing FAIL\n", path);
        goto RE_TRY_WRITE_PROM; 
    }
    res = 0;

    if (need_init && usb)
    {
        vdi_release(core_idx);
    }

    return res;

ERR_USB_INIT:

    res = -1;
    if (need_init && usb)
    {
        vdi_release(core_idx);
    }

    return res;
}

int vdi_ready_change_clock(unsigned long core_idx)
{
    vdi_info_t *vdi;

    if (core_idx >= MAX_NUM_VPU_CORE)
        return 0;

    vdi = &s_vdi_info[core_idx];

    if(!vdi || vdi->vpu_fd==(int)-1 || vdi->vpu_fd == (int)0x00)
        return 0;

    return usb_write_register(core_idx, vdi->vpu_fd, CLOCK_MUX_REG, CLOCK_MASK_ALL);
}

int vdi_set_change_clock(unsigned long core_idx, unsigned long clock_mask)
{
    vdi_info_t *vdi;

    if (core_idx >= MAX_NUM_VPU_CORE)
        return 0;

    vdi = &s_vdi_info[core_idx];

    if(!vdi || vdi->vpu_fd==(int)-1 || vdi->vpu_fd == (int)0x00)
        return 0;

    return usb_write_register(core_idx, vdi->vpu_fd, CLOCK_MUX_REG, clock_mask);
}

int vdi_done_change_clock(unsigned long core_idx)
{
    vdi_info_t *vdi;

    if (core_idx >= MAX_NUM_VPU_CORE)
        return 0;

    vdi = &s_vdi_info[core_idx];

    if(!vdi || vdi->vpu_fd==(int)-1 || vdi->vpu_fd == (int)0x00)
        return 0;

    return usb_write_register(core_idx, vdi->vpu_fd, CLOCK_MUX_REG, 0);
}

int usb_init(unsigned long core_idx, unsigned long dram_base)
{
    if (core_idx >= MAX_NUM_VPU_CORE)
        return -1;

    s_dram_base = dram_base;

    return 1;
}

void usb_release(unsigned long core_idx)
{	
}

int usb_write_register(unsigned long core_idx, int base, unsigned int addr, unsigned int data)
{
    cnm_usb_apb_write_req_packet_t req;
    cnm_usb_apb_write_res_packet_t res;
    int actual_length;

    if (io_lock(core_idx) < 0)
        return -1;

    req.cmd = CNM_USB_CMD_WRITE | CNM_USB_CMD_APB | CNM_USB_CMD_SIZE(4);

    req.data[0].addr = addr;
    req.data[0].data = data;

    actual_length = usb_write_endpoint((int)base, (unsigned char *)&req, sizeof(cnm_usb_apb_write_req_packet_t));
    if (actual_length != sizeof(cnm_usb_apb_write_req_packet_t))
    {
        VLOG(ERR, "[VDI] usb_write_register error usb write fail , actual_length=%d\n", actual_length);
        io_unlock(core_idx);
        return 0;

    }

    actual_length = usb_read_endpoint((int)base, (unsigned char *)&res, sizeof(cnm_usb_apb_write_res_packet_t));
    if (actual_length != sizeof(cnm_usb_apb_write_res_packet_t))
    {
        VLOG(ERR, "[VDI] usb_write_register error usb read fail , actual_length=%d\n", actual_length);
        io_unlock(core_idx);
        return 0;
    }

    io_unlock(core_idx);

    return 1;
}

unsigned int usb_read_register(unsigned long core_idx, int base, unsigned int addr)
{
    int actual_length;
    unsigned int data;
    cnm_usb_apb_read_req_packet_t req;
    cnm_usb_apb_read_res_packet_t res;

    if (io_lock(core_idx) < 0)
        return -1;

    memset((void*)&req, 0x00, sizeof(cnm_usb_apb_read_req_packet_t));
    req.cmd = CNM_USB_CMD_READ | CNM_USB_CMD_APB | CNM_USB_CMD_SIZE(4);
    req.data[0].addr = addr;

    actual_length = usb_write_endpoint((int)base, (unsigned char *)&req, sizeof(cnm_usb_apb_write_req_packet_t));
    if (actual_length != sizeof(cnm_usb_apb_write_req_packet_t))
    {
        VLOG(ERR, "[VDI] usb_read_register error usb write fail, actual_length=%d\n", actual_length);
        io_unlock(core_idx);
        return -1;
    }

    actual_length = usb_read_endpoint((int)base, (unsigned char *)&res, sizeof(cnm_usb_apb_write_res_packet_t));
    if (actual_length != sizeof(cnm_usb_apb_write_res_packet_t))
    {
        VLOG(ERR, "[VDI] usb_write_register error usb read fail, actual_length=%d\n", actual_length);		
        io_unlock(core_idx);
        return -1;
    }

    data = (res.data[3]<<24 | res.data[2]<<16 | res.data[1]<<8 | res.data[0]);

    io_unlock(core_idx);

    return data;
}

int usb_write_memory(unsigned long core_idx, int base, unsigned int addr, unsigned char *data, int len, int endian)
{	
    unsigned int    numberOf4KBlocks;
    unsigned int    next4Kaddr;
    int             sizeToWrite, remainSize;
    unsigned char*  pBuf;
    unsigned char   lsBuf[CNM_USB_BUS_LEN];
    unsigned int     alignSize = CNM_USB_BUS_LEN;
    unsigned int     alignMask = CNM_USB_BUS_LEN-1, alignedAddr;
    unsigned int     offset;

    if (addr < s_dram_base) {
        fprintf(stderr, "[USB] invalid address base address is 0x%lx\n", (int)addr);
        return 0;
    }

    if (io_lock(core_idx) < 0)
        return -1;

    if (len==0) {
        io_unlock(core_idx);
        return 0;
    }

    addr = addr - s_dram_base;

    numberOf4KBlocks = ((len+4095)&~4095)>>12;

    alignedAddr = addr&~alignMask;
    offset      = addr - alignedAddr;
    pBuf        = (BYTE*)osal_malloc((len+offset+alignMask)&~alignMask);
    if (offset) {
        usb_axi_read_burst(base, alignedAddr, lsBuf, (offset+alignMask)&~alignMask);
        swap_endian(core_idx, lsBuf, alignSize, endian);
        memcpy(pBuf, lsBuf, offset);
    }
    addr       = alignedAddr;
    remainSize = len;
    next4Kaddr = (addr+4095)&~4095;
    if (addr != next4Kaddr && (addr+len) > next4Kaddr) {
        sizeToWrite = next4Kaddr - addr - offset;
        memcpy(pBuf+offset, data, sizeToWrite);
        swap_endian(core_idx, pBuf, (sizeToWrite+offset+alignMask)&~alignMask, endian);
        usb_axi_write_burst(base, addr, (unsigned char *)pBuf, (sizeToWrite+offset+alignMask)&~alignMask);	

        data       += sizeToWrite;
        remainSize -= sizeToWrite;
        addr        = next4Kaddr;
        offset      = 0;
    } 

    sizeToWrite = remainSize+offset;
    memcpy(pBuf+offset, data, remainSize);
    swap_endian(core_idx, pBuf, (sizeToWrite+alignMask)&~alignMask, endian);

    usb_axi_write_burst(base, addr, pBuf, (sizeToWrite+alignMask)&~alignMask);

    free(pBuf);

    io_unlock(core_idx);

    return len;	
}

int usb_read_memory(unsigned long core_idx, int base, unsigned int addr, unsigned char *data, int len, int endian)
{
    unsigned int     numberOf4KBlocks;
    unsigned int     next4Kaddr;
    int              sizeToRead, remainSize;
    unsigned char*   pBuf;
    unsigned char    lsBuf[CNM_USB_BUS_LEN];
    unsigned int     alignSize = CNM_USB_BUS_LEN;
    unsigned int     alignMask = CNM_USB_BUS_LEN-1, alignedAddr;
    unsigned int     offset;

    if (addr < s_dram_base) {
        fprintf(stderr, "[USB] invalid address base address is 0x%lx\n", (int)addr);
        return 0;
    }

    if (io_lock(core_idx) < 0)
        return -1;

    if (len==0) {
        io_unlock(core_idx);
        return 0;
    }

    addr = addr - s_dram_base;

    numberOf4KBlocks = ((len+4095)&~4095)>>12;

    alignedAddr = addr&~alignMask;
    offset      = addr - alignedAddr;
    pBuf        = (BYTE*)osal_malloc((len+offset+alignMask)&~alignMask);
    addr        = alignedAddr;
    remainSize  = len + offset;
    next4Kaddr  = (addr+4095)&~4095;
    if (addr != next4Kaddr && (addr+len) > next4Kaddr) {
        sizeToRead  = next4Kaddr - addr;
        usb_axi_read_burst(base, addr, pBuf, (sizeToRead+alignMask)&~alignMask);
        swap_endian(core_idx, pBuf, (sizeToRead+offset+alignMask)&~alignMask, endian);
        memcpy(data, pBuf+offset, sizeToRead-offset);

        data       += sizeToRead-offset;
        remainSize -= sizeToRead;
        addr        = next4Kaddr;
        offset      = 0;
        //numberOf4KBlocks--;
    } 

    if (numberOf4KBlocks > 0 && len > 0) {
        usb_axi_read_burst(base, addr, pBuf, (remainSize+alignMask)&~alignMask);
        swap_endian(core_idx, pBuf, (remainSize+alignMask)&~alignMask, endian);
        memcpy(data, pBuf+offset, remainSize-offset);
    }

	osal_free(pBuf);
    io_unlock(core_idx);

    return len;
}

int usb_chip_reset(int base)
{
	vpudrv_usb_control_packet_t packet;
	int ret;

	if (io_lock(0) < 0 )
		return -1;

	memset(&packet, 0x00, sizeof(vpudrv_usb_control_packet_t));

// 	if (ioctl((int)base, VDI_IOCTL_ENDPOINT_RESET, NULL) == -1)
// 	{
// 		ioctl((int)base, VDI_IOCTL_USB_RESET, NULL);
// 		usleep(1000*1000);
// 	}	

#define RESET_FIFO_STATUS	0xf2
	packet.dir = 0; // out
	packet.request = RESET_FIFO_STATUS;
	packet.val = 0;
	packet.index = 0;
	packet.length = 0;
	packet.timeout = CNM_USB_BULK_TIMEOUT;
	ret = ioctl((int)base, VDI_IOCTL_CONTROL_TRANSFER, &packet);
	if(ret < 0) 
	{
		VLOG(WARN, "[VDI] RESET_FIFO_STATUS is not implemented in USB firmware  ret=%d, val=%d, index=%d, length=%d, request=0x%lx\n", ret, packet.val, packet.index, packet.length, packet.request);		
	}

#define SUPPORT_FPGA_PIN_RESET
#ifdef SUPPORT_FPGA_PIN_RESET
#define FPGA_PIN_RESET 0xf1
	packet.dir = 0; // out
	packet.request = FPGA_PIN_RESET;
	packet.val = 0;
	packet.index = 0;
	packet.length = 0;
	packet.timeout = CNM_USB_BULK_TIMEOUT;
	ret = ioctl((int)base, VDI_IOCTL_CONTROL_TRANSFER, &packet);
	if(ret < 0) 
	{
		VLOG(WARN, "[VDI] FPGA_PIN_RESET is not implemented in USB firmware  ret=%d, val=%d, index=%d, length=%d, request=0x%lx\n", ret, packet.val, packet.index, packet.length, packet.request);		
	}
	usleep(1000*1000);			
#endif

	io_unlock(0);

	return ret;
}
int usb_hw_reset(int base)
{
    cnm_usb_apb_write_req_packet_t req;
	vpudrv_usb_control_packet_t packet;
    int actual_length;
	int ret;

	usb_chip_reset(base);

    if (io_lock(0) < 0 )
        return -1;

    req.cmd = CNM_USB_CMD_WRITE | CNM_USB_CMD_APB | CNM_USB_CMD_SIZE(4);

    req.data[0].addr = DEVICE_ADDR_SW_RESET;
    req.data[0].data = 1;

    actual_length = usb_write_endpoint((int)base, (unsigned char *)&req, sizeof(cnm_usb_apb_write_req_packet_t));
    if (actual_length != sizeof(cnm_usb_apb_write_req_packet_t))
    {
        VLOG(ERR, "[VDI] usb_hw_reset write function req fail, actual_length=%d\n", actual_length);
        io_unlock(0);
        return actual_length;
    }

    VLOG(INFO, "[VDI] usb_hw_reset success actual_length=%d\n", actual_length);

    usleep(3000*1000);

    io_unlock(0);

    return 0;
}

/*------------------------------------------------------------------------
Usage : used to program output frequency of ICS307M
Argument :
Device		: first device selected if 0, second device if 1.
OutFreqMHz	: Target output frequency in MHz.
Return : TRUE if success, FALSE if invalid OutFreqMHz.
------------------------------------------------------------------------*/
int usb_ics307m_set_clock_freg(int base, int Device, int OutFreqMHz)
{

    int		VDW, RDW, OD, SDW, tmp;
    int		min_clk ; 
    int		max_clk ;

    if ( Device == 0 )
    {   
        min_clk = ACLK_MIN ;
        max_clk = ACLK_MAX ;
    }
    else
    {   
        min_clk = CCLK_MIN ;
        max_clk = CCLK_MAX ;
    }

    if (OutFreqMHz < min_clk || OutFreqMHz > max_clk) {
        // printf ("Target Frequency should be from %2d to %2d !!!\n", min_clk, max_clk);
        return 0;
    }

    if (OutFreqMHz >= min_clk && OutFreqMHz < 14) {
        switch (OutFreqMHz) {
        case 6: VDW=4; RDW=2; OD=10; break;
        case 7: VDW=6; RDW=2; OD=10; break;
        case 8: VDW=8; RDW=2; OD=10; break;
        case 9: VDW=10; RDW=2; OD=10; break;
        case 10: VDW=12; RDW=2; OD=10; break;
        case 11: VDW=14; RDW=2; OD=10; break;
        case 12: VDW=16; RDW=2; OD=10; break;
        case 13: VDW=18; RDW=2; OD=10; break;
        } 
    } else {
        VDW = OutFreqMHz - 8;	// VDW
        RDW = 3;				// RDW
        OD = 4;					// OD
    } 

    switch (OD) {			// change OD to SDW: s2:s1:s0 
    case 0: SDW = 0; break;
    case 1: SDW = 0; break;
    case 2: SDW = 1; break;
    case 3: SDW = 6; break;
    case 4: SDW = 3; break;
    case 5: SDW = 4; break;
    case 6: SDW = 7; break;
    case 7: SDW = 4; break;
    case 8: SDW = 2; break;
    case 9: SDW = 0; break;
    case 10: SDW = 0; break;
    default: SDW = 0; break;
    }

    //for USB
    tmp = (0x20|SDW) << 16 | (((VDW << 7)&0xff80 | RDW));	// for param0
    usb_write_register(0, base, (CLOCK_GEN_PARAM0(Device)), tmp);		// write data 0
    tmp = 1;
    usb_write_register(0, base, (CLOCK_GEN_COMMAND(Device)), tmp);		// write command set
    tmp = 0;
    usb_write_register(0, base, (CLOCK_GEN_COMMAND(Device)), tmp);		// write command reset

    usleep(100*1000);
    return 1;
}

int usb_set_timing_opt(unsigned long core_idx, int base)
{
    printf ("\nusb_set_timing_opt : No required for USB interface\n");
    return 0;
}

int usb_axi_read_burst(int base, unsigned int addr, unsigned char *buf,int len)
{
    cnm_usb_axi_read_req_packet_t req;
    cnm_usb_axi_read_res_packet_t res;
    int actual_length;
    int pos;
    int req_len;

    pos = 0;
    req_len = CNM_USB_MAX_BURST_SIZE;
    if (len < CNM_USB_MAX_BURST_SIZE)
        req_len = len;

    if (req_len < CNM_USB_BUS_LEN) 
    {
        VLOG(ERR, "[VDI] usb_axi_read_burst error transfer len=%d is invalid\n", len);
        return 0;
    }

    do 
    {
        req.cmd = CNM_USB_CMD_READ | CNM_USB_CMD_AXI | CNM_USB_CMD_SIZE(req_len);
        req.addr = addr+pos;

        actual_length = usb_write_endpoint((int)base, (unsigned char *)&req, sizeof(cnm_usb_axi_read_req_packet_t));
        if (actual_length != sizeof(cnm_usb_axi_read_req_packet_t))
        {
            VLOG(ERR, "[VDI] usb_axi_read_burst error usb write fail, actual_length=%d\n", actual_length);
            return 0;
        }
#ifdef SUPPORT_CONF_TEST
#ifdef CNM_FPGA_USB_INTERFACE
        USB_TRANSACTION_DOING=TRUE;
#endif /* CNM_FPGA_USB_INTERFACE */
#endif
        actual_length = usb_read_endpoint((int)base, (unsigned char *)&res, sizeof(cnm_usb_axi_read_res_packet_t));
        if (actual_length != sizeof(cnm_usb_axi_read_res_packet_t))
        {
            VLOG(ERR, "[VDI] usb_axi_read_burst error usb read , actual_length=%d\n", actual_length);		
            return 0;
        }
#ifdef SUPPORT_CONF_TEST
#ifdef CNM_FPGA_USB_INTERFACE
        USB_TRANSACTION_DOING=FALSE;
#endif /* CNM_FPGA_USB_INTERFACE */
#endif

        memcpy(buf+pos, &res.data[0], req_len);
        len -= CNM_USB_MAX_BURST_SIZE;
        if (len < CNM_USB_MAX_BURST_SIZE)
            req_len = len;
        pos += CNM_USB_MAX_BURST_SIZE;

    } while(len > 0);

    return 1;
}

int usb_axi_write_burst(int base, unsigned int addr, unsigned char *buf, int len)
{
    cnm_usb_axi_write_req_packet_t req;
    cnm_usb_axi_write_res_packet_t res;
    int actual_length;
    int pos, req_len;

    req_len = CNM_USB_MAX_BURST_SIZE;
    if (len < CNM_USB_MAX_BURST_SIZE)
        req_len = len;

    if (req_len < CNM_USB_BUS_LEN) 
    {
        VLOG(ERR, "[VDI] usb_axi_write_burst error transfer len=%d is invalid\n", len);
        return 0;
    }

    pos = 0;
    do 
    {
        req.cmd = CNM_USB_CMD_WRITE | CNM_USB_CMD_AXI | CNM_USB_CMD_SIZE(req_len);
        req.addr = addr+pos;
        memcpy(&req.data[0], &buf[pos], req_len);

        actual_length = usb_write_endpoint((int)base, (unsigned char *)&req, sizeof(cnm_usb_axi_write_req_packet_t));
        if (actual_length != sizeof(cnm_usb_axi_write_req_packet_t))
        {
            VLOG(ERR, "[VDI] usb_axi_write_burst error usb write fail, actual_length=%d\n", actual_length);
            return 0;
        }

#ifdef SUPPORT_CONF_TEST
#ifdef CNM_FPGA_USB_INTERFACE
        USB_TRANSACTION_DOING=TRUE;
#endif /* CNM_FPGA_USB_INTERFACE */
#endif
        actual_length = usb_read_endpoint((int)base, (unsigned char *)&res, sizeof(cnm_usb_axi_write_res_packet_t));
        if (actual_length != sizeof(cnm_usb_axi_write_res_packet_t))
        {
            VLOG(ERR, "[VDI] usb_axi_write_burst error usb read fail, actual_length=%d\n", actual_length);		
            return 0;
        }
#ifdef SUPPORT_CONF_TEST
#ifdef CNM_FPGA_USB_INTERFACE
        USB_TRANSACTION_DOING=FALSE;
#endif /* CNM_FPGA_USB_INTERFACE */
#endif

        len -= CNM_USB_MAX_BURST_SIZE;
        if (len < CNM_USB_MAX_BURST_SIZE)
            req_len = len;

        pos += CNM_USB_MAX_BURST_SIZE;

    } while(len > 0);

    return 1;
}

int usb_write_endpoint(int base, unsigned char *buf, int len)
{
    int retval;
    int i;
    int bytes_transferred;
    unsigned char *bufRef;

    if ((len % CNM_USB_BULK_OUT_SIZE) > 0)
    {
        VLOG(ERR, "[VDI] usb_write_endpoint error transfer length is not aligned by %d, length=%d\n", CNM_USB_BULK_OUT_SIZE, len);	
        return -1;
    }

    retval = 0;
    bytes_transferred = 0;
    bufRef = (unsigned char *)buf;

    for (i=0; i<len; i=i+CNM_USB_BULK_OUT_SIZE)
    {
        retval = write((int)base, bufRef, CNM_USB_BULK_OUT_SIZE);
        if (retval != CNM_USB_BULK_OUT_SIZE)
        {
            VLOG(ERR, "[VDI] usb_write_endpoint error write fail retval=%d\n", retval);	
            return retval;
        }
        bytes_transferred += CNM_USB_BULK_OUT_SIZE;
        bufRef = buf + bytes_transferred;
    }	

    return bytes_transferred;
}

int usb_read_endpoint(int base, unsigned char *buf, int len)
{
    int retval;
    int i;
    int bytes_transferred;
    unsigned char *bufRef;

    if ((len % CNM_USB_BULK_IN_SIZE) > 0)
    {
        VLOG(ERR, "[VDI] usb_read_endpoint error transfer length is not aligned by %d, length=%d\n", CNM_USB_BULK_IN_SIZE, len);	
        return -1;
    }

    retval = 0;
    bytes_transferred = 0;
    bufRef = (unsigned char *)buf;

    for (i=0; i<len; i=i+CNM_USB_BULK_IN_SIZE)
    {
        retval = read((int)base, bufRef, CNM_USB_BULK_IN_SIZE);
        if (retval != CNM_USB_BULK_IN_SIZE)
        {
            VLOG(ERR, "[VDI] usb_read_endpoint error write fail retval=%d\n", retval);	
            return retval;
        }
        bytes_transferred += CNM_USB_BULK_IN_SIZE;
        bufRef = buf + bytes_transferred;
    }	

    return bytes_transferred;
}

#endif

#ifdef CNM_FPGA_PCI_INTERFACE

static void * s_hpi_base;
static unsigned long s_dram_base;
static unsigned long s_hpi_bus_length;

static unsigned int hpi_read_reg_limit(unsigned long core_idx, unsigned int addr, unsigned int *data);
static unsigned int hpi_write_reg_limit(unsigned long core_idx,unsigned int addr, unsigned int data);
static unsigned int pci_read_reg(unsigned int addr);
static void pci_write_reg(unsigned int addr, unsigned int data);
static int pci_write_memory(unsigned int addr, unsigned char *buf, int size);
static int pci_read_memory(unsigned int addr, unsigned char *buf, int size);
static void pci_read_cmd();
static void pci_write_cmd();

int vdi_load_prom_via_usb(unsigned long core_idx, int need_init, const char* path)
{
    return -1;
}

int vdi_ready_change_clock(unsigned long core_idx)
{
    UNREFERENCED_PARAMETER(core_idx);
    pci_write_reg(CLOCK_MUX_REG<<2, CLOCK_MASK_ALL);

    return 1;
}

int vdi_set_change_clock(unsigned long core_idx, unsigned long clock_mask)
{
    UNREFERENCED_PARAMETER(core_idx);
    pci_write_reg(CLOCK_MUX_REG<<2, clock_mask);

    return 1;
}

int vdi_done_change_clock(unsigned long core_idx)
{
    UNREFERENCED_PARAMETER(core_idx);
    pci_write_reg(CLOCK_MUX_REG<<2, 0);

    return 1;
}

int hpi_init(unsigned long core_idx, unsigned long dram_base)
{
    if (core_idx >= MAX_NUM_VPU_CORE)
        return -1;
    s_dram_base = dram_base;
    if (s_hpi_bus_length == 0)
        s_hpi_bus_length = 8;

    return 1;
}

void hpi_release(unsigned long core_idx)
{
}

int hpi_write_register(unsigned long core_idx, void * base, unsigned int addr, unsigned int data)
{
    int status, cnt = 0;
    int ret = 0;

    if (io_lock(core_idx) < 0)
        return -1;

    s_hpi_base = base;

    pci_write_reg(HPI_ADDR_ADDR_H, (addr >> 16));
    pci_write_reg(HPI_ADDR_ADDR_L, (addr & 0xffff));

    pci_write_reg(HPI_ADDR_DATA, ((data >> 16) & 0xFFFF));
    pci_write_reg(HPI_ADDR_DATA + 4, (data & 0xFFFF));

    pci_write_cmd();

    do {
        usleep(0);
        status = pci_read_reg(HPI_ADDR_STATUS);
        status = (status>>1) & 1;
        cnt++;
    } while (status == 0 && cnt < HPI_WAIT_TIMEOUT);

    if (cnt == HPI_WAIT_TIMEOUT) {
        ret = -200;
    }

    if (s_EnableTrace == TRUE) {
        VLOG(INFO, "APB_WRITE : %s(addr:0x%08x, data:0x%08x) [%s]\n", __FUNCTION__, addr, data, (ret==0)?"OK":"FAIL");
    }

    io_unlock(core_idx);

    return ret;
}

unsigned int hpi_read_register(unsigned long core_idx, void * base, unsigned int addr)
{
    int             status, cnt = 0;
    unsigned int    data;
    BOOL            success = TRUE;

    if (io_lock(core_idx) < 0) {
        return -1;
    }

    s_hpi_base = base;

    pci_write_reg(HPI_ADDR_ADDR_H, ((addr >> 16)&0xffff));
    pci_write_reg(HPI_ADDR_ADDR_L, (addr & 0xffff));

    pci_read_cmd();

    do {
        usleep(0);
        status = pci_read_reg(HPI_ADDR_STATUS);
        status = status & 1;
        cnt++;
    } while (status == 0 && cnt < HPI_WAIT_TIMEOUT);

    if (cnt == HPI_WAIT_TIMEOUT) {
        data = (unsigned int)-200;
    }

    data = pci_read_reg(HPI_ADDR_DATA) << 16;
    data |= pci_read_reg(HPI_ADDR_DATA + 4);

    if (s_EnableTrace == TRUE) {
        VLOG(INFO, "APB_READ  : %s (addr:0x%08x) ----> 0x%08x [%s]\n", __FUNCTION__, addr, data, (success==TRUE)?"OK":"FAIL");
    }

    io_unlock(core_idx);	

    return data;
}

int hpi_write_memory(unsigned long core_idx, void * base, uint32_t busLength, unsigned int addr, unsigned char *data, int len, int endian)
{	
    unsigned int    next4Kaddr;
    int             sizeToWrite, remainSize;
    unsigned char*  pBuf;
    unsigned char   lsBuf[HPI_MAX_BUS_LENGTH];
    unsigned int     alignSize = busLength;
    unsigned int     alignMask = busLength-1, alignedAddr;
    unsigned int     offset;

    if (addr < s_dram_base) {
        fprintf(stderr, "[HPI-w] invalid address base address is 0x%lu\n", addr);
        return 0;
    }

    if (io_lock(core_idx) < 0)
        return 0;

    if (len==0) {
        io_unlock(core_idx);
        return 0;
    }

    addr = addr - s_dram_base;
    s_hpi_base = base; 

    alignedAddr = addr&~alignMask;
    offset      = addr - alignedAddr;
    pBuf        = (BYTE*)osal_malloc((len+offset+alignMask)&~alignMask);
    if (offset) {
        pci_read_memory(alignedAddr, lsBuf, (offset+alignMask)&~alignMask);
        swap_endian(core_idx, lsBuf, alignSize, endian);
        memcpy(pBuf, lsBuf, offset);
    }
    addr       = alignedAddr;
    remainSize = len;
    next4Kaddr = (addr+4095)&~4095;
    if (addr != next4Kaddr && (addr+len) > next4Kaddr) {
        sizeToWrite = next4Kaddr - addr - offset;
        memcpy(pBuf+offset, data, sizeToWrite);
        swap_endian(core_idx, pBuf, (sizeToWrite+offset+alignMask)&~alignMask, endian);
        pci_write_memory(addr, (unsigned char *)pBuf, (sizeToWrite+offset+alignMask)&~alignMask);	

        data       += sizeToWrite;
        remainSize -= sizeToWrite;
        addr        = next4Kaddr;
        offset      = 0;
    } 

    sizeToWrite = remainSize+offset;
    memcpy(pBuf+offset, data, remainSize);
    swap_endian(core_idx, pBuf, (sizeToWrite+alignMask)&~alignMask, endian);

    pci_write_memory(addr, pBuf, (sizeToWrite+alignMask)&~alignMask);

    free(pBuf);

    io_unlock(core_idx);	

    return len;	
}

int hpi_read_memory(unsigned long core_idx, void * base, uint32_t busLength, unsigned int addr, unsigned char *data, int len, int endian)
{
    unsigned int     numberOf4KBlocks;
    unsigned int     next4Kaddr;
    int              sizeToRead, remainSize;
    unsigned char*   pBuf;
    unsigned int     alignSize = busLength;
    unsigned int     alignMask = busLength-1, alignedAddr;
    unsigned int     offset;

	UNREFERENCED_PARAMETER(alignSize);
    if (addr < s_dram_base) {
        fprintf(stderr, "[HPI-r] invalid address base address is 0x%lu\n", addr);
        return 0;
    }

    if (io_lock(core_idx) < 0)
        return 0;

    if (len==0) {
        io_unlock(core_idx);
        return 0;
    }

    addr = addr - s_dram_base;
    s_hpi_base = base; 

    numberOf4KBlocks = ((len+4095)&~4095)>>12;

    alignedAddr = addr&~alignMask;
    offset      = addr - alignedAddr;
    pBuf        = (BYTE*)osal_malloc((len+offset+alignMask)&~alignMask);
    addr        = alignedAddr;
    remainSize  = len+offset;
    next4Kaddr  = (addr+4095)&~4095;
    if (addr != next4Kaddr && (addr+len) > next4Kaddr) {
        sizeToRead  = next4Kaddr - addr;
        pci_read_memory(addr, pBuf, (sizeToRead+alignMask)&~alignMask);
        swap_endian(core_idx, pBuf, (sizeToRead+offset+alignMask)&~alignMask, endian);
        memcpy(data, pBuf+offset, sizeToRead-offset);

        data       += sizeToRead-offset;
        remainSize -= sizeToRead;
        addr        = next4Kaddr;
        offset      = 0;
        //numberOf4KBlocks--;
    } 

    if (numberOf4KBlocks > 0) {
        pci_read_memory(addr, pBuf, (remainSize+alignMask)&~alignMask);
        swap_endian(core_idx, pBuf, (remainSize+alignMask)&~alignMask, endian);
        memcpy(data, pBuf+offset, remainSize-offset);
    }

    free(pBuf);

    io_unlock(core_idx);

    return len;
}

int hpi_hw_reset(void * base)
{
    s_hpi_base = base;
	io_lock(0);
    pci_write_reg(DEVICE_ADDR_SW_RESET<<2, 1);		// write data 1	
    io_unlock(0);
    usleep(3000*1000);
    return 0;
}

unsigned int hpi_write_reg_limit(unsigned long core_idx, unsigned int addr, unsigned int data)
{
    int status;
    int i;

    if (io_lock(core_idx) < 0)
        return 0;

    pci_write_reg(HPI_ADDR_ADDR_H, (addr >> 16));
    pci_write_reg(HPI_ADDR_ADDR_L, (addr & 0xffff));

    pci_write_reg(HPI_ADDR_DATA, ((data >> 16) & 0xFFFF));
    pci_write_reg(HPI_ADDR_DATA + 4, (data & 0xFFFF));

    pci_write_cmd();

    i = 0;
    do {
        status = pci_read_reg(HPI_ADDR_STATUS);
        status = (status>>1) & 1;
        if (i++ > 10000)
        {
            io_unlock(core_idx);
            return 0;
        }
    } while (status == 0);

    io_unlock(core_idx);	

    return 1;
}

unsigned int hpi_read_reg_limit(unsigned long core_idx, unsigned int addr, unsigned int *data)
{
    int status;
    int i;

    if (io_lock(core_idx) < 0)
        return 0;

    pci_write_reg(HPI_ADDR_ADDR_H, ((addr >> 16)&0xffff));
    pci_write_reg(HPI_ADDR_ADDR_L, (addr & 0xffff));

    pci_read_cmd();

    i=0;
    do {
        status = pci_read_reg(HPI_ADDR_STATUS);
        status = status & 1;
        if (i++ > 10000)
        {
            io_unlock(core_idx);
            return 0;
        }
    } while (status == 0);

    *data = pci_read_reg(HPI_ADDR_DATA) << 16;
    *data |= pci_read_reg(HPI_ADDR_DATA + 4);

    io_unlock(core_idx);	

    return 1;
}

/*------------------------------------------------------------------------
Usage : used to program output frequency of ICS307M
Artument :
Device		: first device selected if 0, second device if 1.
OutFreqMHz	: Target output frequency in MHz.
Return : TRUE if success, FALSE if invalid OutFreqMHz.
------------------------------------------------------------------------*/
int hpi_ics307m_set_clock_freg(void * base, int Device, int OutFreqMHz)
{
    int		VDW, RDW, OD, SDW, tmp;
    int		min_clk ; 
    int		max_clk ;

    s_hpi_base = base;
    if ( Device == 0 )
    {   
        min_clk = ACLK_MIN ;
        max_clk = ACLK_MAX ;
    }
    else
    {   
        min_clk = CCLK_MIN ;
        max_clk = CCLK_MAX ;
    }

    if (OutFreqMHz < min_clk || OutFreqMHz > max_clk) {
        // printf ("Target Frequency should be from %2d to %2d !!!\n", min_clk, max_clk);
        return 0;
    }

    if (OutFreqMHz >= min_clk && OutFreqMHz < 14) {
        switch (OutFreqMHz) {
        case 6: VDW=4; RDW=2; OD=10; break;
        case 7: VDW=6; RDW=2; OD=10; break;
        case 8: VDW=8; RDW=2; OD=10; break;
        case 9: VDW=10; RDW=2; OD=10; break;
        case 10: VDW=12; RDW=2; OD=10; break;
        case 11: VDW=14; RDW=2; OD=10; break;
        case 12: VDW=16; RDW=2; OD=10; break;
        case 13: VDW=18; RDW=2; OD=10; break;
        } 
    } else {
        VDW = OutFreqMHz - 8;	// VDW
        RDW = 3;				// RDW
        OD = 4;					// OD
    } 

    switch (OD) {			// change OD to SDW: s2:s1:s0 
    case 0: SDW = 0; break;
    case 1: SDW = 0; break;
    case 2: SDW = 1; break;
    case 3: SDW = 6; break;
    case 4: SDW = 3; break;
    case 5: SDW = 4; break;
    case 6: SDW = 7; break;
    case 7: SDW = 4; break;
    case 8: SDW = 2; break;
    case 9: SDW = 0; break;
    case 10: SDW = 0; break;
    default: SDW = 0; break;
    }

	io_lock(0);
    tmp = 0x20 | SDW;
    pci_write_reg((CLOCK_GEN_PARAM0(Device))<<2, tmp);		// write data 0
    tmp = ((VDW << 7)&0xff80) | RDW;
    pci_write_reg((CLOCK_GEN_PARAM1(Device))<<2, tmp);		// write data 1
    tmp = 1;
    pci_write_reg((CLOCK_GEN_COMMAND(Device))<<2, tmp);		// write command set
    tmp = 0;
    pci_write_reg((CLOCK_GEN_COMMAND(Device))<<2, tmp);		// write command reset
    io_unlock(0);

    return 1;
}

#define AUTO_HPI_SET_TIMING_OPT
int hpi_set_timing_opt(unsigned long core_idx, void * base)
{
    int i;
    unsigned int iAddr;
    unsigned int uData;
    unsigned int uuData;
    int iTemp;
    int testFail;
    unsigned int startAddr;
    unsigned int endAddr;

    hpi_read_reg_limit(core_idx, VPU_BIT_REG_BASE+0x1044, &uData);

#ifdef CODA7Q
    startAddr = (VPU_BIT_REG_BASE + 0x120);
    endAddr   = (VPU_BIT_REG_BASE + 0x128);
#else
    startAddr = (VPU_BIT_REG_BASE+0x07000000)+0x010; 
    endAddr   = (VPU_BIT_REG_BASE+0x07000000)+0x018;
#endif

    s_hpi_base = base;
	i=2;
#ifdef AUTO_HPI_SET_TIMING_OPT
    
    io_lock(0);
    pci_write_reg(0x70<<2, 25);
    pci_write_reg(0x71<<2, 25);
    pci_write_reg(0x72<<2, 6);
    io_unlock(0);
    iAddr    = startAddr;
    uData    = 0x12345678;
    testFail = 0;
    for (iTemp=0; iTemp<10000; iTemp++) {
        if (hpi_write_reg_limit(core_idx, iAddr, uData)==FALSE) {
            testFail = 1;
            break;
        }
        if (hpi_read_reg_limit(core_idx,iAddr, &uuData)==FALSE) {
            testFail = 1;
            break;
        } 
        if (uuData != uData) {
            testFail = 1;
            break;
        }
        else {
            if (hpi_write_reg_limit(core_idx, iAddr, 0)==FALSE) {
                testFail = 1;
                break;
            }
        }

        iAddr += 4;
        if (iAddr == endAddr)
            iAddr = startAddr;
        uData++;
    }
#endif

#ifdef AUTO_HPI_SET_TIMING_OPT
    if (testFail) {
        // find HPI maximum timing register value
        while (testFail && i < HPI_SET_TIMING_MAX) {
            i++;
            iAddr = startAddr;
            uData = 0x12345678;
            testFail = 0;
            printf ("HPI Tw, Tr value: %d\r", i);

            pci_write_reg(0x70<<2, i);
            pci_write_reg(0x71<<2, i);
            if (i<15) 
                pci_write_reg(0x72<<2, 0);
            else
                pci_write_reg(0x72<<2, i*20/100);

            for (iTemp=0; iTemp<10000; iTemp++) {
                if (hpi_write_reg_limit(core_idx, iAddr, uData)==FALSE) {
                    testFail = 1;
                    break;
                }
                if (hpi_read_reg_limit(core_idx,iAddr, &uuData)==FALSE) {
                    testFail = 1;
                    break;
                } 
                if (uuData != uData) {
                    testFail = 1;
                    break;
                }
                else {
                    if (hpi_write_reg_limit(core_idx, iAddr, 0)==FALSE) {
                        testFail = 1;
                        break;
                    }
                }

                iAddr += 4;
                if (iAddr == endAddr)
                    iAddr = startAddr;
                uData++;
            }
        }
    	io_lock(0);
        pci_write_reg(0x70<<2, i);
        pci_write_reg(0x71<<2, i+i*40/100);
        pci_write_reg(0x72<<2, i*20/100);
    	io_unlock(0);
        printf("----------------------------------------------------------------\n");
        printf("TIMING VALUE CHANGED!!\n");
        printf("----------------------------------------------------------------\n");
        vdi_hw_reset(core_idx);
    }
#else
	io_lock(0);
	pci_write_reg(0x70<<2, 25);
	pci_write_reg(0x71<<2, 25);
	pci_write_reg(0x72<<2, 6);
	io_unlock(0);
#endif

    printf ("\nOptimized HPI Tw value : %d\n", pci_read_reg(0x70<<2));
    printf ("Optimized HPI Tr value : %d\n", pci_read_reg(0x71<<2));
    printf ("Optimized HPI Te value : %d\n", pci_read_reg(0x72<<2));

    return i;
}

void pci_write_reg(unsigned int addr, unsigned int data)
{
    unsigned long *reg_addr = (unsigned long *)(addr + s_hpi_base);
    *(volatile unsigned int *)reg_addr = data;	
}

unsigned int pci_read_reg(unsigned int addr)
{
    unsigned long *reg_addr = (unsigned long *)(addr + s_hpi_base);
    return *(volatile unsigned int *)reg_addr;
}

void pci_write_cmd(void)
{
    pci_write_reg(HPI_ADDR_CMD, (s_hpi_bus_length<<4)|2);
}

void pci_read_cmd(void)
{
    pci_write_reg(HPI_ADDR_CMD, (s_hpi_bus_length<<4)|1);
}

int pci_read_memory(unsigned int addr, unsigned char *buf, int size)
{

    int status;
    int i, j, k;
    int data = 0, cnt;

    i = j = k = 0;

    for (i=0; i < size / HPI_MAX_PKSIZE; i++) 
    {
        pci_write_reg(HPI_ADDR_ADDR_H, (addr >> 16));		
        pci_write_reg(HPI_ADDR_ADDR_L, (addr & 0xffff));

        pci_write_reg(HPI_ADDR_CMD, (((HPI_MAX_PKSIZE) << 4) + 1));

        cnt = 0;
        do 
        {
            usleep(0);
            status = 0;
            status = pci_read_reg(HPI_ADDR_STATUS);
            status = status & 1;
            cnt++;
        } while (status==0 && cnt < HPI_WAIT_TIMEOUT);

        if (cnt == HPI_WAIT_TIMEOUT)
            return -200;

        for (j=0; j<HPI_MAX_PKSIZE/2; j++) 
        {
            data = pci_read_reg(HPI_ADDR_DATA + j * 4);
            buf[k  ] = (data >> 8) & 0xFF;
            buf[k+1] = data & 0xFF;
            k = k + 2;
        }

        addr += HPI_MAX_PKSIZE;
    }

    size = size % HPI_MAX_PKSIZE;

    if ( ((addr + size) & 0xFFFFFF00) != (addr & 0xFFFFFF00))
        size = size;

    if (size) 
    {
        pci_write_reg(HPI_ADDR_ADDR_H, (addr >> 16));
        pci_write_reg(HPI_ADDR_ADDR_L, (addr & 0xffff));

        pci_write_reg(HPI_ADDR_CMD, (((size) << 4) + 1));

        do 
        {
            status = 0;
            status = pci_read_reg(HPI_ADDR_STATUS);
            status = status & 1;

        } while (status==0);

        for (j = 0; j < size / 2; j++) 
        {
            data = pci_read_reg(HPI_ADDR_DATA + j*4);
            buf[k  ] = (data >> 8) & 0xFF;
            buf[k+1] = data & 0xFF;
            k = k + 2;
        }
    }

    return 1;
}

int  pci_write_memory(unsigned int addr, unsigned char *buf, int size)
{
    int status;
    int i, j, k;
    int data = 0;
    int cnt;

    i = j = k = 0;

    for (i = 0; i < size/HPI_MAX_PKSIZE; i++)
    {
        pci_write_reg(HPI_ADDR_ADDR_H, (addr >> 16));
        pci_write_reg(HPI_ADDR_ADDR_L, (addr & 0xffff));

        for (j=0; j < HPI_MAX_PKSIZE/2; j++) 
        {            
            data = (buf[k] << 8) | buf[k+1];
            pci_write_reg(HPI_ADDR_DATA + j * 4, data);
            k = k + 2;
        }

        pci_write_reg(HPI_ADDR_CMD, (((HPI_MAX_PKSIZE) << 4) + 2));
        cnt = 0;
        do {
            usleep(0);
            status = pci_read_reg(HPI_ADDR_STATUS);
            status = (status>>1) & 1;
            cnt++;
        } while (status == 0 && cnt < 10000);

        if (cnt == 1000000)
            return -200;

        addr += HPI_MAX_PKSIZE;
    }

    size = size % HPI_MAX_PKSIZE;

    if (size) 
    {
        pci_write_reg(HPI_ADDR_ADDR_H, (addr >> 16));
        pci_write_reg(HPI_ADDR_ADDR_L, (addr & 0xffff));

        for (j = 0; j< size / 2; j++) 
        {
            data = (buf[k] << 8) | buf[k+1];
            pci_write_reg(HPI_ADDR_DATA + j * 4, data);
            k = k + 2;
        }

        pci_write_reg(HPI_ADDR_CMD, (((size) << 4) + 2));

        do 
        {
            status = 0;
            status = pci_read_reg(HPI_ADDR_STATUS);
            status = (status>>1) & 1;

        } while (status==0);
    }

    return 1;
}

void hpi_set_bus_length(int length)
{
    s_hpi_bus_length = length;
}
#endif //#ifdef CNM_FPGA_PCI_INTERFACE

#endif //#include CNM_FPGA_PLATFORM

#endif	//#if defined(linux) || defined(__linux) || defined(ANDROID)
 
