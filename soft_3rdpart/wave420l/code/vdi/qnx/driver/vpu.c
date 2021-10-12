/*
 * vpu.c
 *
 * linux device driver for VPU.
 *
 * Copyright (C) 2006 - 2013  CHIPS&MEDIA INC.
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301  USA
 *
 */

#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/dma-mapping.h>
#include <linux/wait.h>
#include <linux/list.h>
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/uaccess.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/sched.h>

#include "../../../vpuapi/vpuconfig.h"
#ifdef CNM_FPGA_PLATFORM
#include <linux/workqueue.h>
#endif

#include "vpu.h"

//#define ENABLE_DEBUG_MSG
#ifdef ENABLE_DEBUG_MSG
#define DPRINTK(args...)		printk(KERN_INFO args);
#else
#define DPRINTK(args...)
#endif

/* definitions to be changed as customer  configuration */
/* if you want to have clock gating scheme frame by frame */
/* #define VPU_SUPPORT_CLOCK_CONTROL */

/* if the driver want to use interrupt service from kernel ISR */
#define VPU_SUPPORT_ISR

/* if the platform driver knows the name of this driver */
/* VPU_PLATFORM_DEVICE_NAME */
/* #define VPU_SUPPORT_PLATFORM_DRIVER_REGISTER	*/

/* if this driver knows the dedicated video memory address */
#define VPU_SUPPORT_RESERVED_VIDEO_MEMORY

#define VPU_PLATFORM_DEVICE_NAME "vdec"
#define VPU_CLK_NAME "vcodec"
#define VPU_DEV_NAME "vpu"

/* if the platform driver knows this driver */
/* the definition of VPU_REG_BASE_ADDR and VPU_REG_SIZE are not meaningful */

#define VPU_REG_BASE_ADDR 0x75000000
#define VPU_REG_SIZE (0x4000*MAX_NUM_VPU_CORE)

#ifdef VPU_SUPPORT_ISR
#define VPU_IRQ_NUM (23+32)
#endif

/* this definition is only for chipsnmedia FPGA board env */
/* so for SOC env of customers can be ignored */
#ifdef CNM_FPGA_PLATFORM

#if defined(CODA960) || defined(CODA980)
#define VPU_CNM_REG_BASE_ADDR		0x10000000
#else
#define VPU_CNM_REG_BASE_ADDR		0x40000000
#endif
#define VPU_SUPPORT_PLATFORM_DRIVER_REGISTER

#undef VPU_SUPPORT_ISR

#ifdef CNM_FPGA_PCI_INTERFACE
#define CNM_DEV_VENDOR_ID			   0xCEDA
#define CNM_DEV_DEVICE_ID			   0x4311
#define CNM_DEV_REGISTER_SIZE				0x2000

#include <linux/pci.h>

static struct pci_device_id ids[] = {
	/*{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82801AA_3), },*/
	{ PCI_DEVICE(CNM_DEV_VENDOR_ID, CNM_DEV_DEVICE_ID), },
	{ 0, }
};
MODULE_DEVICE_TABLE(pci, ids);
#endif	/* #ifdef CNM_FPGA_PCI_INTERFACE */
#endif  /* CNM_FPGA_PLATFORM */

#ifndef VM_RESERVED	/*for kernel up to 3.7.0 version*/
# define  VM_RESERVED   (VM_DONTEXPAND | VM_DONTDUMP)
#endif

typedef struct vpu_drv_context_t {
	struct fasync_struct *async_queue;
	unsigned long interrupt_reason;
	u32 open_count;					 /*!<< device reference count. Not instance count */
#ifdef CNM_FPGA_PLATFORM
	pid_t current_lock_pid;				/* PID aquiring the IO lock */

#endif /* CNM_FPGA_PLATFORM */
} vpu_drv_context_t;

/* To track the allocated memory buffer */
typedef struct vpudrv_buffer_pool_t {
	struct list_head list;
	struct vpudrv_buffer_t vb;
	struct file *filp;
} vpudrv_buffer_pool_t;

/* To track the instance index and buffer in instance pool */
typedef struct vpudrv_instanace_list_t {
	struct list_head list;
	unsigned long inst_idx;
	unsigned long core_idx;
	struct file *filp;
} vpudrv_instanace_list_t;

typedef struct vpudrv_instance_pool_t {
	unsigned char codecInstPool[MAX_NUM_INSTANCE][MAX_INST_HANDLE_SIZE];
} vpudrv_instance_pool_t;

#ifdef VPU_SUPPORT_RESERVED_VIDEO_MEMORY
#ifdef CNM_FPGA_PLATFORM
#	if defined(WAVE420) /* -128MB for MJPEG */
#		define VPU_INIT_VIDEO_MEMORY_SIZE_IN_BYTE (1024*1024*(1023-128))
#elif defined(CODA960) || defined(CODA980)
#		define VPU_INIT_VIDEO_MEMORY_SIZE_IN_BYTE (128*1024*1024)
#	else
#		define VPU_INIT_VIDEO_MEMORY_SIZE_IN_BYTE (1024*1024*1024)
#	endif
#	define VPU_DRAM_PHYSICAL_BASE 0x80000000
#else
#	define VPU_INIT_VIDEO_MEMORY_SIZE_IN_BYTE (62*1024*1024)
#	define VPU_DRAM_PHYSICAL_BASE 0x86C00000
#endif /* CNM_FPGA_PLATFORM */
#include "vmm.h"
static video_mm_t s_vmem;
static vpudrv_buffer_t s_video_memory = {0};
#endif /*VPU_SUPPORT_RESERVED_VIDEO_MEMORY*/

static int vpu_hw_reset(void);
static void vpu_clk_disable(struct clk *clk);
static int vpu_clk_enable(struct clk *clk);
static struct clk *vpu_clk_get(struct device *dev);
static void vpu_clk_put(struct clk *clk);

/* end customer definition */
static vpudrv_buffer_t s_instance_pool = {0};
static vpudrv_buffer_t s_common_memory = {0};
static vpu_drv_context_t s_vpu_drv_context;
static int s_vpu_major;
static struct cdev s_vpu_cdev;

static struct clk *s_vpu_clk;
static int s_vpu_open_ref_count;
#ifdef VPU_SUPPORT_ISR
static int s_vpu_irq = VPU_IRQ_NUM;
#endif

static vpudrv_buffer_t s_vpu_register = {0};

static int s_interrupt_flag;
static wait_queue_head_t s_interrupt_wait_q;

static spinlock_t s_vpu_lock = __SPIN_LOCK_UNLOCKED(s_vpu_lock);
static DEFINE_SEMAPHORE(s_vpu_sem);
static struct list_head s_vbp_head = LIST_HEAD_INIT(s_vbp_head);
static struct list_head s_inst_list_head = LIST_HEAD_INIT(s_inst_list_head);

static vpu_bit_firmware_info_t s_bit_firmware_info[MAX_NUM_VPU_CORE];

#ifdef CONFIG_PM
/* implement to power management functions */
#define BIT_BASE			0x0000
#define BIT_CODE_RUN				(BIT_BASE + 0x000)
#define BIT_CODE_DOWN			   (BIT_BASE + 0x004)
#define BIT_INT_CLEAR			   (BIT_BASE + 0x00C)
#define BIT_INT_STS				 (BIT_BASE + 0x010)
#define BIT_CODE_RESET			(BIT_BASE + 0x014)
#define BIT_INT_REASON			  (BIT_BASE + 0x174)
#define BIT_BUSY_FLAG			   (BIT_BASE + 0x160)
#define BIT_RUN_COMMAND			 (BIT_BASE + 0x164)
#define BIT_RUN_INDEX			   (BIT_BASE + 0x168)
#define BIT_RUN_COD_STD			 (BIT_BASE + 0x16C)

/* WAVE4 registers */
#define W4_REG_BASE					 0x0000
#define W4_VPU_BUSY_STATUS			  (W4_REG_BASE + 0x0070)
#define W4_VPU_INT_REASON_CLEAR		 (W4_REG_BASE + 0x0034)
#define W4_VPU_INT_CLEAR				(W4_REG_BASE + 0x003C)
#define W4_VPU_INT_STS				  (W4_REG_BASE + 0x0044)
#define W4_VPU_INT_REASON			   (W4_REG_BASE + 0x004c)

#define W4_RET_SUCCESS				  (W4_REG_BASE + 0x0110)
#define W4_RET_FAIL_REASON			  (W4_REG_BASE + 0x0114)

/* WAVE4 INIT, WAKEUP */
#define W4_PO_CONF					  (W4_REG_BASE + 0x0000)
#define W4_VCPU_CUR_PC				  (W4_REG_BASE + 0x0004)

#define W4_VPU_INT_ENABLE			   (W4_REG_BASE + 0x0048)

#define W4_RESET_REQ					(W4_REG_BASE + 0x0050)
#define W4_RESET_STATUS				 (W4_REG_BASE + 0x0054)

#define W4_VPU_REMAP_CTRL			   (W4_REG_BASE + 0x0060)
#define W4_VPU_REMAP_VADDR			  (W4_REG_BASE + 0x0064)
#define W4_VPU_REMAP_PADDR			  (W4_REG_BASE + 0x0068)
#define W4_VPU_REMAP_PROC_START		 (W4_REG_BASE + 0x006C)
#define W4_VPU_BUSY_STATUS			  (W4_REG_BASE + 0x0070)

#define W4_REMAP_CODE_INDEX			 0
enum {
	W4_INT_INIT_VPU		  = 0,
	W4_INT_DEC_PIC_HDR	   = 1,
	W4_INT_FINI_SEQ		  = 2,
	W4_INT_DEC_PIC		   = 3,
	W4_INT_SET_FRAMEBUF	  = 4,
	W4_INT_FLUSH_DEC		 = 5,
	W4_INT_GET_FW_VERSION	= 9,
	W4_INT_QUERY_DEC		 = 10,
	W4_INT_SLEEP_VPU		 = 11,
	W4_INT_WAKEUP_VPU		= 12,
	W4_INT_CHANGE_INT		= 13,
	W4_INT_CREATE_INSTANCE   = 14,
	W4_INT_BSBUF_EMPTY	   = 15,   /*!<< Bitstream buffer empty */
	W4_INT_ENC_SLICE_INT     = 15,
};

#define W4_HW_OPTION					(W4_REG_BASE + 0x0124)
#define W4_CODE_SIZE					(W4_REG_BASE + 0x011C)
/* Note: W4_INIT_CODE_BASE_ADDR should be aligned to 4KB */
#define W4_ADDR_CODE_BASE			   (W4_REG_BASE + 0x0118)
#define W4_CODE_PARAM				   (W4_REG_BASE + 0x0120)
/* Note: W4_INIT_STACK_BASE_ADDR should be aligned to 4KB */
#define W4_ADDR_STACK_BASE			  (W4_REG_BASE + 0x012C)
#define W4_STACK_SIZE				   (W4_REG_BASE + 0x0130)
#define W4_INIT_VPU_TIME_OUT_CNT		(W4_REG_BASE + 0x0134)

/* WAVE4 Wave4BitIssueCommand */
#define W4_CORE_INDEX			(W4_REG_BASE + 0x0104)
#define W4_INST_INDEX			(W4_REG_BASE + 0x0108)
#define W4_COMMAND			   (W4_REG_BASE + 0x0100)
#define W4_VPU_HOST_INT_REQ	  (W4_REG_BASE + 0x0038)

/* Product register */
#define VPU_PRODUCT_CODE_REGISTER   (BIT_BASE + 0x1044)

static u32	s_vpu_reg_store[MAX_NUM_VPU_CORE][64];
#if defined(CODA7L) || defined(CODA851)
static u32 s_run_index;
static u32 s_run_codstd;
#endif
#endif

#ifdef CNM_FPGA_PLATFORM
#define CNM_USB_CMD_READ			(0<<15)
#define CNM_USB_CMD_WRITE		   (1<<15)
#define CNM_USB_CMD_APB			 (0<<14)
#define CNM_USB_CMD_AXI			 (1<<14)
#define CNM_USB_CMD_SIZE(SIZE)	  (SIZE&0x3FFF)
#include <linux/mutex.h>

#ifdef CNM_FPGA_PLATFORM
#ifdef CNM_FPGA_PCI_INTERFACE
static int vpu_resume(struct pci_dev *pdev);
static int vpu_suspend(struct pci_dev *pdev, pm_message_t state);
#endif
static DEFINE_MUTEX(s_io_lock);
#else
static int vpu_resume(struct platform_device *pdev);
static int vpu_suspend(struct platform_device *pdev, pm_message_t state);
#endif

#ifdef CNM_FPGA_PCI_INTERFACE
#define HPI_CHECK_STATUS			1
#define HPI_WAIT_TIME				0x100000
/*#define HPI_WAIT_TIMEOUT						1000000*/
#define HPI_WAIT_TIMEOUT						100
#define HPI_ADDR_CMD				(0x00<<2)
#define HPI_ADDR_STATUS				(0x01<<2)
#define HPI_ADDR_ADDR_H				(0x02<<2)
#define HPI_ADDR_ADDR_L				(0x03<<2)
#define HPI_ADDR_ADDR_M				(0x06<<2)
#define HPI_ADDR_DATA				(0x80<<2)

#ifdef SUPPORT_128BIT_BUS
#define HPI_CMD_WRITE_VALUE		 ((16 << 4) + 2)
#define HPI_CMD_READ_VALUE		  ((16 << 4) + 1)
#else
#define HPI_CMD_WRITE_VALUE		 ((8 << 4) + 2)
#define HPI_CMD_READ_VALUE		  ((8 << 4) + 1)
#endif

static unsigned long s_hpi_base = 0;
static unsigned long s_hpi_bus_length = 16;

void pci_write_reg(unsigned int addr, unsigned int data)
{
	unsigned long *reg_addr = (unsigned long *)(addr + (unsigned long)s_hpi_base);
	/*printk(KERN_INFO "%s reg_addr: %p, data=%x\n", __func__, reg_addr, data);*/
	*(volatile unsigned int *)reg_addr = data;
}

static unsigned int pci_read_reg(unsigned int addr)
{
	unsigned long *reg_addr = (unsigned long *)(addr + (unsigned long)s_hpi_base);
	return *(volatile unsigned int *)reg_addr;
}

static void pci_write_cmd(void)
{
	pci_write_reg(HPI_ADDR_CMD, (s_hpi_bus_length<<4)|2);
}

static void pci_read_cmd(void)
{
	pci_write_reg(HPI_ADDR_CMD, (s_hpi_bus_length<<4)|1);
}

static int ldd_hpi_write_register(unsigned int addr, unsigned int data, int reg_base)
{
	int status, cnt = 0;
	int ret;

	if ((ret = mutex_lock_interruptible(&s_io_lock)) == 0) {
		if (reg_base)
			s_hpi_base = (unsigned long)s_vpu_register.virt_addr;
		else
			s_hpi_base = 0;

		addr += VPU_CNM_REG_BASE_ADDR;
		pci_write_reg(HPI_ADDR_ADDR_H, (addr >> 16));
		pci_write_reg(HPI_ADDR_ADDR_L, (addr & 0xffff));

		pci_write_reg(HPI_ADDR_DATA, ((data >> 16) & 0xFFFF));
		pci_write_reg(HPI_ADDR_DATA + 4, (data & 0xFFFF));

		pci_write_cmd();

		do {
			msleep(10);
			status = pci_read_reg(HPI_ADDR_STATUS);
			status = (status>>1) & 1;
			cnt++;
		} while (status == 0 && cnt < HPI_WAIT_TIMEOUT);

		mutex_unlock(&s_io_lock);
	}

	if (cnt == HPI_WAIT_TIMEOUT)
		return -200;

	return 0;
}

static unsigned int ldd_hpi_read_register(unsigned int addr)
{
	int status, cnt = 0;
	int ret = 0;
	unsigned int data = 0;

	if ((ret = mutex_lock_interruptible(&s_io_lock)) == 0) {
		s_hpi_base = (unsigned long)s_vpu_register.virt_addr;

		addr += VPU_CNM_REG_BASE_ADDR;
		pci_write_reg(HPI_ADDR_ADDR_H, ((addr >> 16)&0xffff));
		pci_write_reg(HPI_ADDR_ADDR_L, (addr & 0xffff));

		pci_read_cmd();

		do {
			status = pci_read_reg(HPI_ADDR_STATUS);
			status = status & 1;
			cnt++;
			msleep(10);
		} while (status == 0 && cnt < HPI_WAIT_TIMEOUT);

		if (cnt == HPI_WAIT_TIMEOUT) {
			mutex_unlock(&s_io_lock);
			return -200;
		}

		data = pci_read_reg(HPI_ADDR_DATA) << 16;
		data |= pci_read_reg(HPI_ADDR_DATA + 4);
		mutex_unlock(&s_io_lock);
	}

	return data;
}

#endif

#ifdef CNM_FPGA_PCI_INTERFACE
#define	ReadVpuRegister(addr)		   ldd_hpi_read_register(addr)
#define	WriteVpuRegister(addr, val)	ldd_hpi_write_register(addr, val, 1)
#define	WriteVpu(addr, val)			ldd_hpi_write_register(addr, val, 0)
#endif
#else
#define	ReadVpuRegister(addr)		*(volatile unsigned int *)(s_vpu_register.virt_addr + s_bit_firmware_info[core].reg_base_offset + addr)
#define	WriteVpuRegister(addr, val)	*(volatile unsigned int *)(s_vpu_register.virt_addr + s_bit_firmware_info[core].reg_base_offset + addr) = (unsigned int)val
#define	WriteVpu(addr, val)			*(volatile unsigned int *)(addr) = (unsigned int)val;
#endif
#ifdef CNM_FPGA_PLATFORM
static irqreturn_t vpu_irq_handler(int irq, void *dev_id);
static void polling_interrupt_work(struct work_struct *work);
static DECLARE_WORK(wq, polling_interrupt_work);
static void polling_interrupt_work(struct work_struct *work)
{
	unsigned long product_code;
	unsigned long int_sts_register;
	unsigned long core;
	unsigned long regval = 0;

	DPRINTK("[VPUDRV][+]%s -> %d\n", __func__, HZ);

	for (core = 0; core < MAX_NUM_VPU_CORE; core++) {
		if (s_bit_firmware_info[core].size == 0) /* it means that we didn't get an information the current core from API layer. No core activated.*/
			  continue;
		product_code = ReadVpuRegister(VPU_PRODUCT_CODE_REGISTER);

        if (PRODUCT_CODE_W_SERIES(product_code))
			int_sts_register = W4_VPU_INT_STS;
		else
			int_sts_register = BIT_INT_STS;

		regval = ReadVpuRegister(int_sts_register);
		DPRINTK("[VPUDRV] product_code: 0x%08lx int_sts: %ld\n", product_code, regval);

		if (regval) {
			vpu_irq_handler(0, (void *)&s_vpu_drv_context);
            regval = 0;
        }

	}

	if (regval == 0) {
		udelay(1*1000);	// delay more to give idle time to OS;
		schedule_work(&wq);
	}
	DPRINTK("[VPUDRV][-]%s\n", __func__);
}
#endif

static int vpu_alloc_dma_buffer(vpudrv_buffer_t *vb)
{
	if (!vb)
		return -1;

#ifdef VPU_SUPPORT_RESERVED_VIDEO_MEMORY
	vb->phys_addr = (unsigned long)vmem_alloc(&s_vmem, vb->size, 0);
	if ((unsigned long)vb->phys_addr  == (unsigned long)-1) {
		printk(KERN_ERR "[VPUDRV] Physical memory allocation error size=%d\n", vb->size);
		return -1;
	}

	vb->base = (unsigned long)(s_video_memory.base + (vb->phys_addr - s_video_memory.phys_addr));
#else
	vb->base = (unsigned long)dma_alloc_coherent(NULL, PAGE_ALIGN(vb->size), (dma_addr_t *) (&vb->phys_addr), GFP_DMA | GFP_KERNEL);
	if ((void *)(vb->base) == NULL)	{
		printk(KERN_ERR "[VPUDRV] Physical memory allocation error size=%d\n", vb->size);
		return -1;
	}
#endif
	return 0;
}

static void vpu_free_dma_buffer(vpudrv_buffer_t *vb)
{
	if (!vb)
		return;

#ifdef VPU_SUPPORT_RESERVED_VIDEO_MEMORY
	if (vb->base)
		vmem_free(&s_vmem, vb->phys_addr, 0);
#else
	if (vb->base)
		dma_free_coherent(0, PAGE_ALIGN(vb->size), (void *)vb->base, vb->phys_addr);
#endif
}

static int vpu_free_instances(struct file *filp)
{
	vpudrv_instanace_list_t *vil, *n;
	vpudrv_instance_pool_t *vip;
	void *vip_base;
	int instance_pool_size_per_core;
	void *vdi_mutexes_base;
	const int PTHREAD_MUTEX_T_DESTROY_VALUE = 0xdead10cc;

	DPRINTK("[VPUDRV] vpu_free_instances\n");

	instance_pool_size_per_core = (s_instance_pool.size/MAX_NUM_VPU_CORE); /* s_instance_pool.size  assigned to the size of all core once call VDI_IOCTL_GET_INSTANCE_POOL by user. */

	list_for_each_entry_safe(vil, n, &s_inst_list_head, list)
	{
		if (vil->filp == filp) {
			vip_base = (void *)(s_instance_pool.base + (instance_pool_size_per_core*vil->core_idx));
			DPRINTK("[VPUDRV] vpu_free_instances detect instance crash instIdx=%d, coreIdx=%d, vip_base=%p, instance_pool_size_per_core=%d\n", (int)vil->inst_idx, (int)vil->core_idx, vip_base, (int)instance_pool_size_per_core);
			vip = (vpudrv_instance_pool_t *)vip_base;
			if (vip) {
				memset(&vip->codecInstPool[vil->inst_idx], 0x00, 4);	/* only first 4 byte is key point(inUse of CodecInst in vpuapi) to free the corresponding instance. */
#define PTHREAD_MUTEX_T_HANDLE_SIZE 4
				vdi_mutexes_base = (vip_base + (instance_pool_size_per_core - PTHREAD_MUTEX_T_HANDLE_SIZE*4));
				DPRINTK("[VPUDRV] vpu_free_instances : force to destroy vdi_mutexes_base=%p in userspace \n", vdi_mutexes_base);
				if (vdi_mutexes_base) {
					int i;
					for (i = 0; i < 4; i++) {
						memcpy(vdi_mutexes_base, &PTHREAD_MUTEX_T_DESTROY_VALUE, PTHREAD_MUTEX_T_HANDLE_SIZE);
						vdi_mutexes_base += PTHREAD_MUTEX_T_HANDLE_SIZE;
					}
				}
			}
			s_vpu_open_ref_count--;
			list_del(&vil->list);
			kfree(vil);
		}
	}
	return 1;
}

static int vpu_free_buffers(struct file *filp)
{
	vpudrv_buffer_pool_t *pool, *n;
	vpudrv_buffer_t vb;

	DPRINTK("[VPUDRV] vpu_free_buffers\n");

	list_for_each_entry_safe(pool, n, &s_vbp_head, list)
	{
		if (pool->filp == filp) {
			vb = pool->vb;
			if (vb.base) {
				vpu_free_dma_buffer(&vb);
				list_del(&pool->list);
				kfree(pool);
			}
		}
	}

	return 0;
}

static irqreturn_t vpu_irq_handler(int irq, void *dev_id)
{
	vpu_drv_context_t *dev = (vpu_drv_context_t *)dev_id;

	/* this can be removed. it also work in VPU_WaitInterrupt of API function */
	int core;
	int product_code;

	DPRINTK("[VPUDRV][+]%s\n", __func__);

	for (core = 0; core < MAX_NUM_VPU_CORE; core++) {
		if (s_bit_firmware_info[core].size == 0) {/* it means that we didn't get an information the current core from API layer. No core activated.*/
			printk(KERN_ERR "[VPUDRV] :  s_bit_firmware_info[core].size is zero\n");
			continue;
		}
		product_code = ReadVpuRegister(VPU_PRODUCT_CODE_REGISTER);

        if (PRODUCT_CODE_W_SERIES(product_code)) {
			if (ReadVpuRegister(W4_VPU_INT_STS)) {
				dev->interrupt_reason = ReadVpuRegister(W4_VPU_INT_REASON);
				WriteVpuRegister(W4_VPU_INT_REASON_CLEAR, dev->interrupt_reason);
				WriteVpuRegister(W4_VPU_INT_CLEAR, 0x1);
			}
		} else {
			if (ReadVpuRegister(BIT_INT_STS)) {
				dev->interrupt_reason = ReadVpuRegister(BIT_INT_REASON);
				WriteVpuRegister(BIT_INT_CLEAR, 0x1);
			}
		}
		DPRINTK("[VPUDRV] product: 0x%08x intr_reason: 0x%08lx\n", product_code, dev->interrupt_reason);
	}

	if (dev->async_queue)
		kill_fasync(&dev->async_queue, SIGIO, POLL_IN);	/* notify the interrupt to user space */

	s_interrupt_flag = 1;

	wake_up_interruptible(&s_interrupt_wait_q);
	DPRINTK("[VPUDRV][-]%s\n", __func__);

	return IRQ_HANDLED;
}

static int vpu_open(struct inode *inode, struct file *filp)
{
	DPRINTK("[VPUDRV][+] %s\n", __func__);
	spin_lock(&s_vpu_lock);

	s_vpu_drv_context.open_count++;

	filp->private_data = (void *)(&s_vpu_drv_context);
	spin_unlock(&s_vpu_lock);

	DPRINTK("[VPUDRV][-] %s\n", __func__);

	return 0;
}

/*static int vpu_ioctl(struct inode *inode, struct file *filp, u_int cmd, u_long arg) // for kernel 2.6.9 of C&M*/
static long vpu_ioctl(struct file *filp, u_int cmd, u_long arg)
{
	int ret = 0;
	struct vpu_drv_context_t *dev = (struct vpu_drv_context_t *)filp->private_data;

	switch (cmd) {
	case VDI_IOCTL_ALLOCATE_PHYSICAL_MEMORY:
		{
			vpudrv_buffer_pool_t *vbp;

			DPRINTK("[VPUDRV][+]VDI_IOCTL_ALLOCATE_PHYSICAL_MEMORY\n");

			if ((ret = down_interruptible(&s_vpu_sem)) == 0) {
				vbp = kzalloc(sizeof(*vbp), GFP_KERNEL);
				if (!vbp) {
					up(&s_vpu_sem);
					return -ENOMEM;
				}

				ret = copy_from_user(&(vbp->vb), (vpudrv_buffer_t *)arg, sizeof(vpudrv_buffer_t));
				if (ret) {
					kfree(vbp);
					up(&s_vpu_sem);
					return -EFAULT;
				}

				ret = vpu_alloc_dma_buffer(&(vbp->vb));
				if (ret == -1) {
					ret = -ENOMEM;
					kfree(vbp);
					up(&s_vpu_sem);
					break;
				}
				ret = copy_to_user((void __user *)arg, &(vbp->vb), sizeof(vpudrv_buffer_t));
				if (ret) {
					kfree(vbp);
					ret = -EFAULT;
					up(&s_vpu_sem);
					break;
				}

				vbp->filp = filp;
				spin_lock(&s_vpu_lock);
				list_add(&vbp->list, &s_vbp_head);
				spin_unlock(&s_vpu_lock);

				up(&s_vpu_sem);
			}
			DPRINTK("[VPUDRV][-]VDI_IOCTL_ALLOCATE_PHYSICAL_MEMORY\n");
		}
		break;
	case VDI_IOCTL_FREE_PHYSICALMEMORY:
		{
			vpudrv_buffer_pool_t *vbp, *n;
			vpudrv_buffer_t vb;
			DPRINTK("[VPUDRV][+]VDI_IOCTL_FREE_PHYSICALMEMORY\n");

			if ((ret = down_interruptible(&s_vpu_sem)) == 0) {

				ret = copy_from_user(&vb, (vpudrv_buffer_t *)arg, sizeof(vpudrv_buffer_t));
				if (ret) {
					up(&s_vpu_sem);
					return -EACCES;
				}

				if (vb.base)
					vpu_free_dma_buffer(&vb);

				spin_lock(&s_vpu_lock);
				list_for_each_entry_safe(vbp, n, &s_vbp_head, list)
				{
					if (vbp->vb.base == vb.base) {
						list_del(&vbp->list);
						kfree(vbp);
						break;
					}
				}
				spin_unlock(&s_vpu_lock);

				up(&s_vpu_sem);
			}
			DPRINTK("[VPUDRV][-]VDI_IOCTL_FREE_PHYSICALMEMORY\n");

		}
		break;
	case VDI_IOCTL_GET_RESERVED_VIDEO_MEMORY_INFO:
		{
#ifdef VPU_SUPPORT_RESERVED_VIDEO_MEMORY
			DPRINTK("[VPUDRV][+]VDI_IOCTL_GET_RESERVED_VIDEO_MEMORY_INFO\n");
			if (s_video_memory.base != 0) {
				ret = copy_to_user((void __user *)arg, &s_video_memory, sizeof(vpudrv_buffer_t));
				if (ret != 0)
					ret = -EFAULT;
			} else {
				ret = -EFAULT;
			}
			DPRINTK("[VPUDRV][-]VDI_IOCTL_GET_RESERVED_VIDEO_MEMORY_INFO\n");
#endif
		}
		break;

	case VDI_IOCTL_WAIT_INTERRUPT:
		{
			vpudrv_intr_info_t info;
			DPRINTK("[VPUDRV][+]VDI_IOCTL_WAIT_INTERRUPT\n");
			ret = copy_from_user(&info, (vpudrv_intr_info_t *)arg, sizeof(vpudrv_intr_info_t));
			if (ret != 0)
				return -EFAULT;

#ifdef CNM_FPGA_PLATFORM
			schedule_work(&wq);			
#endif /* CNM_FPGA_PLATFORM */
			ret = wait_event_interruptible_timeout(s_interrupt_wait_q, s_interrupt_flag != 0, msecs_to_jiffies(info.timeout));
#ifdef CNM_FPGA_PLATFORM
			cancel_work_sync(&wq);
			flush_scheduled_work();
#endif /* CNM_FPGA_PLATFORM */
			if (!ret) {
				ret = -ETIME;
				break;
			}

			if (signal_pending(current)) {
				ret = -ERESTARTSYS;
				break;
			}

			DPRINTK("[VPUDRV] s_interrupt_flag(%d), reason(0x%08lx)\n", s_interrupt_flag, dev->interrupt_reason);

			info.intr_reason = dev->interrupt_reason;
			s_interrupt_flag = 0;
			dev->interrupt_reason = 0;
			ret = copy_to_user((void __user *)arg, &info, sizeof(vpudrv_intr_info_t));
			DPRINTK("[VPUDRV][-]VDI_IOCTL_WAIT_INTERRUPT\n");
			if (ret != 0)
				return -EFAULT;
		}
		break;

	case VDI_IOCTL_SET_CLOCK_GATE:
		{
			u32 clkgate;

			DPRINTK("[VPUDRV][+]VDI_IOCTL_SET_CLOCK_GATE\n");
			if (get_user(clkgate, (u32 __user *) arg))
				return -EFAULT;
#ifdef VPU_SUPPORT_CLOCK_CONTROL
			if (clkgate)
				vpu_clk_enable(s_vpu_clk);
			else
				vpu_clk_disable(s_vpu_clk);
#endif
			DPRINTK("[VPUDRV][-]VDI_IOCTL_SET_CLOCK_GATE\n");

		}
		break;
	case VDI_IOCTL_GET_INSTANCE_POOL:
		{
			DPRINTK("[VPUDRV][+]VDI_IOCTL_GET_INSTANCE_POOL\n");
			if ((ret = down_interruptible(&s_vpu_sem)) == 0) {
				if (s_instance_pool.base != 0) {
					ret = copy_to_user((void __user *)arg, &s_instance_pool, sizeof(vpudrv_buffer_t));
					if (ret != 0)
						ret = -EFAULT;
				} else {
					ret = copy_from_user(&s_instance_pool, (vpudrv_buffer_t *)arg, sizeof(vpudrv_buffer_t));
					if (ret == 0) {
	#ifdef USE_VMALLOC_FOR_INSTANCE_POOL_MEMORY
					s_instance_pool.size = PAGE_ALIGN(s_instance_pool.size);
					s_instance_pool.base = (unsigned long)vmalloc(s_instance_pool.size);
					s_instance_pool.phys_addr = s_instance_pool.base;

					if (s_instance_pool.base != 0)
	#else
					
	#ifdef CNM_FPGA_PLATFORM
						s_instance_pool.base = (unsigned long)dma_alloc_coherent(NULL, PAGE_ALIGN(s_instance_pool.size), (dma_addr_t *) (&s_instance_pool.phys_addr), GFP_DMA | GFP_KERNEL);
						if (s_instance_pool.base != 0)
	#else
						if (vpu_alloc_dma_buffer(&s_instance_pool) != -1)
	#endif /* CNM_FPGA_PLATFORM */
	#endif
	
						{
							memset((void *)s_instance_pool.base, 0x0, s_instance_pool.size); /*clearing memory*/
							ret = copy_to_user((void __user *)arg, &s_instance_pool, sizeof(vpudrv_buffer_t));
							if (ret == 0) {
								/* success to get memory for instance pool */
								up(&s_vpu_sem);
								break;
							}
						}

					}
					ret = -EFAULT;
				}

				up(&s_vpu_sem);
			}

			DPRINTK("[VPUDRV][-]VDI_IOCTL_GET_INSTANCE_POOL\n");
		}
		break;
	case VDI_IOCTL_GET_COMMON_MEMORY:
		{
			DPRINTK("[VPUDRV][+]VDI_IOCTL_GET_COMMON_MEMORY\n");
			if (s_common_memory.base != 0) {
				ret = copy_to_user((void __user *)arg, &s_common_memory, sizeof(vpudrv_buffer_t));
				if (ret != 0)
					ret = -EFAULT;
			} else {
				ret = copy_from_user(&s_common_memory, (vpudrv_buffer_t *)arg, sizeof(vpudrv_buffer_t));
				if (ret == 0) {
					if (vpu_alloc_dma_buffer(&s_common_memory) != -1) {
						ret = copy_to_user((void __user *)arg, &s_common_memory, sizeof(vpudrv_buffer_t));
						if (ret == 0) {
							/* success to get memory for common memory */
							break;
						}
					}
				}

				ret = -EFAULT;
			}
			DPRINTK("[VPUDRV][-]VDI_IOCTL_GET_COMMON_MEMORY\n");
		}
		break;
	case VDI_IOCTL_OPEN_INSTANCE:
		{
			vpudrv_inst_info_t inst_info;
			vpudrv_instanace_list_t *vil, *n;

			vil = kzalloc(sizeof(*vil), GFP_KERNEL);
			if (!vil)
				return -ENOMEM;

			if (copy_from_user(&inst_info, (vpudrv_inst_info_t *)arg, sizeof(vpudrv_inst_info_t)))
				return -EFAULT;

			vil->inst_idx = inst_info.inst_idx;
			vil->core_idx = inst_info.core_idx;
			vil->filp = filp;

			spin_lock(&s_vpu_lock);
			list_add(&vil->list, &s_inst_list_head);

			 inst_info.inst_open_count = 0; /* counting the current open instance number */
			list_for_each_entry_safe(vil, n, &s_inst_list_head, list)
			{
				if (vil->core_idx == inst_info.core_idx)
					inst_info.inst_open_count++;
			}
			spin_unlock(&s_vpu_lock);

			s_vpu_open_ref_count++; /* flag just for that vpu is in opened or closed */

			if (copy_to_user((void __user *)arg, &inst_info, sizeof(vpudrv_inst_info_t))) {
                                kfree(vil);
				return -EFAULT;
                        }

			DPRINTK("[VPUDRV] VDI_IOCTL_OPEN_INSTANCE core_idx=%d, inst_idx=%d, s_vpu_open_ref_count=%d, inst_open_count=%d\n", (int)inst_info.core_idx, (int)inst_info.inst_idx, s_vpu_open_ref_count, inst_info.inst_open_count);
		}
		break;
	case VDI_IOCTL_CLOSE_INSTANCE:
		{
			vpudrv_inst_info_t inst_info;
			vpudrv_instanace_list_t *vil, *n;

			DPRINTK("[VPUDRV][+]VDI_IOCTL_CLOSE_INSTANCE\n");
			if (copy_from_user(&inst_info, (vpudrv_inst_info_t *)arg, sizeof(vpudrv_inst_info_t)))
				return -EFAULT;

			spin_lock(&s_vpu_lock);
			list_for_each_entry_safe(vil, n, &s_inst_list_head, list)
			{
				if (vil->inst_idx == inst_info.inst_idx && vil->core_idx == inst_info.core_idx) {
					list_del(&vil->list);
					kfree(vil);
					break;
				}
			}

			inst_info.inst_open_count = 0; /* counting the current open instance number */
			list_for_each_entry_safe(vil, n, &s_inst_list_head, list)
			{
				if (vil->core_idx == inst_info.core_idx)
					inst_info.inst_open_count++;
			}
			spin_unlock(&s_vpu_lock);

			s_vpu_open_ref_count--; /* flag just for that vpu is in opened or closed */

			if (copy_to_user((void __user *)arg, &inst_info, sizeof(vpudrv_inst_info_t)))
				return -EFAULT;

			DPRINTK("[VPUDRV] VDI_IOCTL_CLOSE_INSTANCE core_idx=%d, inst_idx=%d, s_vpu_open_ref_count=%d, inst_open_count=%d\n", (int)inst_info.core_idx, (int)inst_info.inst_idx, s_vpu_open_ref_count, inst_info.inst_open_count);
		}
		break;
	case VDI_IOCTL_GET_INSTANCE_NUM:
		{
			vpudrv_inst_info_t inst_info;
			vpudrv_instanace_list_t *vil, *n;
			DPRINTK("[VPUDRV][+]VDI_IOCTL_GET_INSTANCE_NUM\n");

			ret = copy_from_user(&inst_info, (vpudrv_inst_info_t *)arg, sizeof(vpudrv_inst_info_t));
			if (ret != 0)
				break;

			inst_info.inst_open_count = 0;

			spin_lock(&s_vpu_lock);
			list_for_each_entry_safe(vil, n, &s_inst_list_head, list)
			{
				if (vil->core_idx == inst_info.core_idx)
					inst_info.inst_open_count++;
			}
			spin_unlock(&s_vpu_lock);

			ret = copy_to_user((void __user *)arg, &inst_info, sizeof(vpudrv_inst_info_t));

			DPRINTK("[VPUDRV] VDI_IOCTL_GET_INSTANCE_NUM core_idx=%d, inst_idx=%d, open_count=%d\n", (int)inst_info.core_idx, (int)inst_info.inst_idx, inst_info.inst_open_count);

		}
		break;
	case VDI_IOCTL_RESET:
		{
			vpu_hw_reset();
		}
		break;
	case VDI_IOCTL_GET_REGISTER_INFO:
		{
			DPRINTK("[VPUDRV][+]VDI_IOCTL_GET_REGISTER_INFO\n");
			ret = copy_to_user((void __user *)arg, &s_vpu_register, sizeof(vpudrv_buffer_t));
			if (ret != 0)
				ret = -EFAULT;
			DPRINTK("[VPUDRV][-]VDI_IOCTL_GET_REGISTER_INFO s_vpu_register.phys_addr=0x%x, s_vpu_register.virt_addr=0x%lx, s_vpu_register.size=%d\n", s_vpu_register.phys_addr , s_vpu_register.virt_addr, s_vpu_register.size);
		}
		break;
#ifdef CNM_FPGA_PLATFORM
	case VDI_IOCTL_IO_LOCK:
		if ((ret = mutex_lock_interruptible(&s_io_lock)) == 0) {
			s_vpu_drv_context.current_lock_pid = current->pid;
		}
		break;
	case VDI_IOCTL_IO_UNLOCK:
		s_vpu_drv_context.current_lock_pid = 0;
		mutex_unlock(&s_io_lock);
		break;
#if 0 /*only for PM test*/
#ifdef CONFIG_PM
	case VDI_IOCTL_SLEEP:
		{
			pm_message_t null;
			vpu_suspend(NULL, null);
			break;
		}
	case VDI_IOCTL_WAKEUP:
		{
			vpu_resume(NULL);
			break;
		}
#endif
#endif
#endif
	default:
		{
			printk(KERN_ERR "[VPUDRV] No such IOCTL, cmd is %d\n", cmd);
		}
		break;
	}
	return ret;
}

static ssize_t vpu_read(struct file *filp, char __user *buf, size_t len, loff_t *ppos)
{
	return -1;
}

static ssize_t vpu_write(struct file *filp, const char __user *buf, size_t len, loff_t *ppos)
{

	/* DPRINTK("[VPUDRV] vpu_write len=%d\n", (int)len); */
	if (!buf) {
		printk(KERN_ERR "[VPUDRV] vpu_write buf = NULL error \n");
		return -EFAULT;
	}

	if (len == sizeof(vpu_bit_firmware_info_t))	{
		vpu_bit_firmware_info_t *bit_firmware_info;

		bit_firmware_info = kmalloc(sizeof(vpu_bit_firmware_info_t), GFP_KERNEL);
		if (!bit_firmware_info) {
			printk(KERN_ERR "[VPUDRV] vpu_write  bit_firmware_info allocation error \n");
			return -EFAULT;
		}

		if (copy_from_user(bit_firmware_info, buf, len)) {
			printk(KERN_ERR "[VPUDRV] vpu_write copy_from_user error for bit_firmware_info\n");
			return -EFAULT;
		}

		if (bit_firmware_info->size == sizeof(vpu_bit_firmware_info_t)) {
			DPRINTK("[VPUDRV] vpu_write set bit_firmware_info coreIdx=0x%x, reg_base_offset=0x%x size=0x%x, bit_code[0]=0x%x\n",
			bit_firmware_info->core_idx, (int)bit_firmware_info->reg_base_offset, bit_firmware_info->size, bit_firmware_info->bit_code[0]);

			if (bit_firmware_info->core_idx > MAX_NUM_VPU_CORE) {
				printk(KERN_ERR "[VPUDRV] vpu_write coreIdx[%d] is exceeded than MAX_NUM_VPU_CORE[%d]\n", bit_firmware_info->core_idx, MAX_NUM_VPU_CORE);
				return -ENODEV;
			}

			memcpy((void *)&s_bit_firmware_info[bit_firmware_info->core_idx], bit_firmware_info, sizeof(vpu_bit_firmware_info_t));
			kfree(bit_firmware_info);

			return len;
		}

		kfree(bit_firmware_info);
	}

	return -1;
}

static int vpu_release(struct inode *inode, struct file *filp)
{
    int ret = 0;

    DPRINTK("[VPUDRV] vpu_release\n");

    if ((ret = down_interruptible(&s_vpu_sem)) == 0) {

        /* found and free the not handled buffer by user applications */
        vpu_free_buffers(filp);

        /* found and free the not closed instance by user applications */
        vpu_free_instances(filp);
#ifdef CNM_FPGA_PLATFORM
        DPRINTK("CURRENT_LOCK_PID: %d, current->pid: %d\n", s_vpu_drv_context.current_lock_pid, current->pid);
        if (s_vpu_drv_context.current_lock_pid == current->pid) {
            /* Abnormal exit. release lock */
            DPRINTK("[VPUDRV] abnormal exit....%d\n", current->pid);
            s_vpu_drv_context.current_lock_pid = 0;
            mutex_unlock(&s_io_lock);
        }
#endif /* CNM_FPGA_PLATFORM */
        s_vpu_drv_context.open_count--;
        if (s_vpu_drv_context.open_count == 0) {
            if (s_instance_pool.base) {
                DPRINTK("[VPUDRV] free instance pool\n");
#ifdef USE_VMALLOC_FOR_INSTANCE_POOL_MEMORY
                vfree((const void *)s_instance_pool.base);
#else			
#ifdef CNM_FPGA_PLATFORM
                dma_free_coherent(0, PAGE_ALIGN(s_instance_pool.size), (void *)s_instance_pool.base, s_instance_pool.phys_addr);
#else
                vpu_free_dma_buffer(&s_instance_pool);
#endif
#endif
                s_instance_pool.base = 0;
            }

            if (s_common_memory.base) {
                DPRINTK("[VPUDRV] free common memory\n");
                vpu_free_dma_buffer(&s_common_memory);
                s_common_memory.base = 0;
            }

#ifdef CNM_FPGA_PLATFORM
            if (s_vpu_drv_context.current_lock_pid != 0) {
                DPRINTK("release IO LOCK\n");
                mutex_unlock(&s_io_lock);
                s_vpu_drv_context.current_lock_pid = 0;
            }
#endif /* CNM_FPGA_PLATFORM */
        }
    }
    up(&s_vpu_sem);

    return 0;
}

static int vpu_fasync(int fd, struct file *filp, int mode)
{
	struct vpu_drv_context_t *dev = (struct vpu_drv_context_t *)filp->private_data;
	return fasync_helper(fd, filp, mode, &dev->async_queue);
}

#ifdef CNM_FPGA_PLATFORM
/*
 * FOR GDB TO ACCESS MMAPPED MEMORY, OVERRIDED vm_operations_struct::access method.
 * THIS PATCH IS QUOTED FROM THE BELOW LINK.
 * http://stackoverflow.com/questions/654393/examining-mmaped-addresses-using-gdb/7558859#7558859
 */

static int custom_generic_access_phys(struct vm_area_struct *vma, unsigned long addr,
    void *buf, int len, int write)
{
    void __iomem *maddr;
    //int offset = (addr & (PAGE_SIZE-1)) - vma->vm_start;
    int offset = (addr) - vma->vm_start;

    maddr = phys_to_virt(__pa(s_instance_pool.base));
    if (write)
        memcpy_toio(maddr + offset, buf, len);
    else
        memcpy_fromio(buf, maddr + offset, len);

    return len;
}

static inline int custom_vma_access(struct vm_area_struct *vma, unsigned long addr,
    void *buf, int len, int write)
{
    return custom_generic_access_phys(vma, addr, buf, len, write);
}

static struct vm_operations_struct custom_vm_ops = {
    .access = custom_vma_access,
};
#endif /* CNM_FPGA_PLATFORM */

static int vpu_map_to_register(struct file *fp, struct vm_area_struct *vm)
{
	unsigned long pfn;

	vm->vm_flags |= VM_IO | VM_RESERVED;
	vm->vm_page_prot = pgprot_noncached(vm->vm_page_prot);
	pfn = s_vpu_register.phys_addr >> PAGE_SHIFT;

	return remap_pfn_range(vm, vm->vm_start, pfn, vm->vm_end-vm->vm_start, vm->vm_page_prot) ? -EAGAIN : 0;
}

static int vpu_map_to_physical_memory(struct file *fp, struct vm_area_struct *vm)
{
	vm->vm_flags |= VM_IO | VM_RESERVED;
	vm->vm_page_prot = pgprot_writecombine(vm->vm_page_prot);

	return remap_pfn_range(vm, vm->vm_start, vm->vm_pgoff, vm->vm_end-vm->vm_start, vm->vm_page_prot) ? -EAGAIN : 0;
}
static int vpu_map_to_instance_pool_memory(struct file *fp, struct vm_area_struct *vm)
{
#ifdef USE_VMALLOC_FOR_INSTANCE_POOL_MEMORY
	int ret;
	long length = vm->vm_end - vm->vm_start;  
	unsigned long start = vm->vm_start;  
	char *vmalloc_area_ptr = (char *)s_instance_pool.base;  
	unsigned long pfn;  

	vm->vm_flags |= VM_RESERVED;

	/* loop over all pages, map it page individually */  
	while (length > 0) 
	{  
		pfn = vmalloc_to_pfn(vmalloc_area_ptr);  
		if ((ret = remap_pfn_range(vm, start, pfn, PAGE_SIZE, PAGE_SHARED)) < 0) {  
				return ret;  
		}  
		start += PAGE_SIZE;  
		vmalloc_area_ptr += PAGE_SIZE;  
		length -= PAGE_SIZE;  
	}  
	
#ifdef CNM_FPGA_PLATFORM
    vm->vm_ops = &custom_vm_ops;
#endif /* CNM_FPGA_PLATFORM */

	return 0;
#else
#ifdef CNM_FPGA_PLATFORM
    vm->vm_ops = &custom_vm_ops;
#endif /* CNM_FPGA_PLATFORM */
	vm->vm_flags |= VM_RESERVED;
	return remap_pfn_range(vm, vm->vm_start, vm->vm_pgoff, vm->vm_end-vm->vm_start, vm->vm_page_prot) ? -EAGAIN : 0;
#endif
}

/*!
 * @brief memory map interface for vpu file operation
 * @return  0 on success or negative error code on error
 */
static int vpu_mmap(struct file *fp, struct vm_area_struct *vm)
{
#ifdef USE_VMALLOC_FOR_INSTANCE_POOL_MEMORY
	if (vm->vm_pgoff == 0)
		return vpu_map_to_instance_pool_memory(fp, vm);
	
	if (vm->vm_pgoff == (s_vpu_register.phys_addr>>PAGE_SHIFT))
		return vpu_map_to_register(fp, vm);

	return vpu_map_to_physical_memory(fp, vm);
#else
	if (vm->vm_pgoff) {
		if (vm->vm_pgoff == (s_instance_pool.phys_addr>>PAGE_SHIFT))
			return vpu_map_to_instance_pool_memory(fp, vm);

		return vpu_map_to_physical_memory(fp, vm);
	} else {
		return vpu_map_to_register(fp, vm);
	}
#endif
}

struct file_operations vpu_fops = {
	.owner = THIS_MODULE,
	.open = vpu_open,
	.read = vpu_read,
	.write = vpu_write,
	/*.ioctl = vpu_ioctl, // for kernel 2.6.9 of C&M*/
	.unlocked_ioctl = vpu_ioctl,
	.release = vpu_release,
	.fasync = vpu_fasync,
	.mmap = vpu_mmap,
};

#ifdef CNM_FPGA_PLATFORM
#ifdef CNM_FPGA_PCI_INTERFACE
static int vpu_probe(struct pci_dev *pdev, const struct pci_device_id *id)
#endif
#else
static int vpu_probe(struct platform_device *pdev)
#endif
{
	int err = 0;
#ifdef CNM_FPGA_PLATFORM

#ifdef CNM_FPGA_PCI_INTERFACE
	int i;

	DPRINTK("[VPUDRV] vpu_probe vendor=0x%x, device=0x%x\n", (int)id->vendor, (int)id->device);

	if (id->vendor != CNM_DEV_VENDOR_ID && id->device != CNM_DEV_DEVICE_ID)
		return -ENODEV;

	pci_enable_device(pdev);

	for (i = 0; i < 6; i++)	{
		if ((pci_resource_flags(pdev, i) & IORESOURCE_MEM))	{
			s_vpu_register.phys_addr = pci_resource_start(pdev, i);
			break;
		}
	}

	if (!s_vpu_register.phys_addr) {
		printk(KERN_ERR "C&M HPI controller has no memory regions defined.\n");
		return -EINVAL;
	}

	s_vpu_register.virt_addr = (unsigned long)ioremap_nocache(s_vpu_register.phys_addr, CNM_DEV_REGISTER_SIZE);
	s_vpu_register.size = CNM_DEV_REGISTER_SIZE;		
	DPRINTK("[VPUDRV] : vpu base address get from PCI bus driver physical idx=%d, s_vpu_register.phys_addr=0x%x, s_vpu_register.virt_addr=0x%lx, vpu_reg_len=%d\n", i, s_vpu_register.phys_addr , s_vpu_register.virt_addr, CNM_DEV_REGISTER_SIZE);
    printk(KERN_INFO "REGISTER BASE: 0x%08x\n", VPU_CNM_REG_BASE_ADDR);
    printk(KERN_INFO "MEMORY SIZE  : 0x%08x\n", VPU_INIT_VIDEO_MEMORY_SIZE_IN_BYTE);
#endif

#else
	struct resource *res = NULL;

	DPRINTK("[VPUDRV] vpu_probe\n");
	if (pdev)
		res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (res) {/* if platform driver is implemented */
		s_vpu_register.phys_addr = res->start;
		s_vpu_register.virt_addr = (unsigned long)ioremap_nocache(res->start, res->end - res->start);
		s_vpu_register.size = res->end - res->start;
		DPRINTK("[VPUDRV] : vpu base address get from platform driver physical base addr=0x%x, virtual base=0x%lx\n", s_vpu_register.phys_addr , s_vpu_register.virt_addr);
	} else {
		s_vpu_register.phys_addr = VPU_REG_BASE_ADDR;
		s_vpu_register.virt_addr = (unsigned long)ioremap_nocache(s_vpu_register.phys_addr, VPU_REG_SIZE);
		s_vpu_register.size = VPU_REG_SIZE;
		DPRINTK("[VPUDRV] : vpu base address get from defined value physical base addr=0x%x, virtual base=0x%lx\n", s_vpu_register.phys_addr, s_vpu_register.virt_addr);

	}
#endif

	/* get the major number of the character device */
	if ((alloc_chrdev_region(&s_vpu_major, 0, 1, VPU_DEV_NAME)) < 0) {
		err = -EBUSY;
		printk(KERN_ERR "could not allocate major number\n");
		goto ERROR_PROVE_DEVICE;
	}
	printk(KERN_INFO "SUCCESS alloc_chrdev_region\n");

	/* initialize the device structure and register the device with the kernel */
	cdev_init(&s_vpu_cdev, &vpu_fops);
	if ((cdev_add(&s_vpu_cdev, s_vpu_major, 1)) < 0) {
		err = -EBUSY;
		printk(KERN_ERR "could not allocate chrdev\n");

		goto ERROR_PROVE_DEVICE;
	}

	if (pdev)
		s_vpu_clk = vpu_clk_get(&pdev->dev);
	else
		s_vpu_clk = vpu_clk_get(NULL);

	if (!s_vpu_clk)
		printk(KERN_ERR "[VPUDRV] : not to support clock controller.\n");
	else
		DPRINTK("[VPUDRV] : get clock controller s_vpu_clk=%p\n", s_vpu_clk);

#ifdef VPU_SUPPORT_CLOCK_CONTROL
#else
	vpu_clk_enable(s_vpu_clk);
#endif

#ifdef VPU_SUPPORT_ISR
	if (pdev)
		res = platform_get_resource(pdev, IORESOURCE_IRQ, 0);
	if (res) {/* if platform driver is implemented */
		s_vpu_irq = res->start;
		DPRINTK("[VPUDRV] : vpu irq number get from platform driver irq=0x%x\n", s_vpu_irq);
	} else {
		DPRINTK("[VPUDRV] : vpu irq number get from defined value irq=0x%x\n", s_vpu_irq);
	}

	err = request_irq(s_vpu_irq, vpu_irq_handler, 0, "VPU_CODEC_IRQ", (void *)(&s_vpu_drv_context));
	if (err) {
		printk(KERN_ERR "[VPUDRV] :  fail to register interrupt handler\n");
		goto ERROR_PROVE_DEVICE;
	}
#endif

#ifdef VPU_SUPPORT_RESERVED_VIDEO_MEMORY
	s_video_memory.size = VPU_INIT_VIDEO_MEMORY_SIZE_IN_BYTE;
	s_video_memory.phys_addr = VPU_DRAM_PHYSICAL_BASE;
#ifdef CNM_FPGA_PLATFORM
	s_video_memory.base = VPU_DRAM_PHYSICAL_BASE;
#else
	s_video_memory.base = (unsigned long)ioremap_nocache(s_video_memory.phys_addr, PAGE_ALIGN(s_video_memory.size));
#endif
	if (!s_video_memory.base) {
		printk(KERN_ERR "[VPUDRV] :  fail to remap video memory physical phys_addr=0x%x, base=0x%x, size=%d\n", (int)s_video_memory.phys_addr, (int)s_video_memory.base, (int)s_video_memory.size);
		goto ERROR_PROVE_DEVICE;
	}

	if (vmem_init(&s_vmem, s_video_memory.phys_addr, s_video_memory.size) < 0) {
		printk(KERN_ERR "[VPUDRV] :  fail to init vmem system\n");
		goto ERROR_PROVE_DEVICE;
	}
	DPRINTK("[VPUDRV] success to probe vpu device with reserved video memory phys_addr=0x%x, base = 0x%x\n", (int) s_video_memory.phys_addr, (int)s_video_memory.base);
#else
	DPRINTK("[VPUDRV] success to probe vpu device with non reserved video memory\n");
#endif

	return 0;

ERROR_PROVE_DEVICE:

	if (s_vpu_major)
		unregister_chrdev_region(s_vpu_major, 1);

	if (s_vpu_register.virt_addr)
		iounmap((void *)s_vpu_register.virt_addr);

	return err;
}

#ifdef CNM_FPGA_PLATFORM
#ifdef CNM_FPGA_PCI_INTERFACE
static void vpu_remove(struct pci_dev *pdev)
#endif
#else
static int vpu_remove(struct platform_device *pdev)
#endif
{
	DPRINTK("[VPUDRV] vpu_remove\n");
#ifdef VPU_SUPPORT_PLATFORM_DRIVER_REGISTER

	if (s_instance_pool.base) {
#ifdef USE_VMALLOC_FOR_INSTANCE_POOL_MEMORY
		vfree((const void *)s_instance_pool.base);
#else		
#ifdef CNM_FPGA_PLATFORM
		dma_free_coherent(0, PAGE_ALIGN(s_instance_pool.size), (void *)s_instance_pool.base, s_instance_pool.phys_addr);
#else
		vpu_free_dma_buffer(&s_instance_pool);
#endif /* CNM_FPGA_PLATFORM */
#endif
		s_instance_pool.base = 0;
	}

	if (s_common_memory.base) {
		vpu_free_dma_buffer(&s_common_memory);
		s_common_memory.base = 0;
	}

#ifdef VPU_SUPPORT_RESERVED_VIDEO_MEMORY
	if (s_video_memory.base) {
#ifdef CNM_FPGA_PLATFORM
#else
		iounmap((void *)s_video_memory.base);		
#endif /* CNM_FPGA_PLATFORM */
		s_video_memory.base = 0;
		vmem_exit(&s_vmem);
	}
#endif

	if (s_vpu_major > 0) {
		cdev_del(&s_vpu_cdev);
		unregister_chrdev_region(s_vpu_major, 1);
		s_vpu_major = 0;
	}

#ifdef VPU_SUPPORT_ISR
	if (s_vpu_irq)
		free_irq(s_vpu_irq, &s_vpu_drv_context);
#endif

	if (s_vpu_register.virt_addr)
		iounmap((void *)s_vpu_register.virt_addr);

	vpu_clk_put(s_vpu_clk);

#endif /*VPU_SUPPORT_PLATFORM_DRIVER_REGISTER*/

#ifdef CNM_FPGA_PLATFORM
#else
	return 0;
#endif
}

#ifdef CONFIG_PM
#define WAVE4_STACK_SIZE		 (8*1024)	   /* DO NOT CHANGE */
#define W4_MAX_CODE_BUF_SIZE	 (512*1024)
#define W4_CMD_INIT_VPU		  (0x0001)
#define W4_CMD_SLEEP_VPU		 (0x0400)
#define W4_CMD_WAKEUP_VPU		(0x0800)

static void Wave4BitIssueCommand(int core, u32 cmd)
{
	WriteVpuRegister(W4_VPU_BUSY_STATUS, 1);
	WriteVpuRegister(W4_CORE_INDEX,  0);
/*	coreIdx = ReadVpuRegister(W4_VPU_BUSY_STATUS);*/
/*	coreIdx = 0;*/
/*	WriteVpuRegister(W4_INST_INDEX,  (instanceIndex&0xffff)|(codecMode<<16));*/
	WriteVpuRegister(W4_COMMAND, cmd);
	WriteVpuRegister(W4_VPU_HOST_INT_REQ, 1);

	return;
}

#ifdef CNM_FPGA_PLATFORM
#ifdef CNM_FPGA_PCI_INTERFACE
static int vpu_suspend(struct pci_dev *pdev, pm_message_t state)
#endif
#else
static int vpu_suspend(struct platform_device *pdev, pm_message_t state)
#endif
{
	int i;
	int core;
	unsigned long timeout = jiffies + HZ;	/* vpu wait timeout to 1sec */
	int product_code;

	DPRINTK("[VPUDRV] vpu_suspend\n");

#ifdef CNM_FPGA_PLATFORM
	return 0;
#endif
	vpu_clk_enable(s_vpu_clk);

	if (s_vpu_open_ref_count > 0) {
		for (core = 0; core < MAX_NUM_VPU_CORE; core++) {
			if (s_bit_firmware_info[core].size == 0)
				continue;
			product_code = ReadVpuRegister(VPU_PRODUCT_CODE_REGISTER);
            if (PRODUCT_CODE_W_SERIES(product_code)) {
				while (ReadVpuRegister(W4_VPU_BUSY_STATUS)) {
					if (time_after(jiffies, timeout)) {
						DPRINTK("SLEEP_VPU BUSY timeout");
						goto DONE_SUSPEND;
					}
				}
				Wave4BitIssueCommand(core, W4_CMD_SLEEP_VPU);

				while (ReadVpuRegister(W4_VPU_BUSY_STATUS)) {
					if (time_after(jiffies, timeout)) {
						DPRINTK("SLEEP_VPU BUSY timeout");
						goto DONE_SUSPEND;
					}
				}
				if (ReadVpuRegister(W4_RET_SUCCESS) == 0) {
					DPRINTK("SLEEP_VPU failed [0x%x]", ReadVpuRegister(W4_RET_FAIL_REASON));
					goto DONE_SUSPEND;
				}
			} else {
				while (ReadVpuRegister(BIT_BUSY_FLAG)) {
					if (time_after(jiffies, timeout))
						goto DONE_SUSPEND;
				}

#if defined(CODA7L) || defined(CODA851)
				s_run_index = ReadVpuRegister(BIT_RUN_INDEX);
				s_run_codstd = ReadVpuRegister(BIT_RUN_COD_STD);
				WriteVpuRegister(BIT_BUSY_FLAG, 1);
				WriteVpuRegister(BIT_RUN_COMMAND, VPU_SLEEP);

				while (ReadVpuRegister(BIT_BUSY_FLAG)) {
					if (time_after(jiffies, timeout))
						goto DONE_SUSPEND;
				}
#endif
				for (i = 0; i < 64; i++)
					s_vpu_reg_store[core][i] = ReadVpuRegister(BIT_BASE+(0x100+(i * 4)));
			}
		}
	}

	vpu_clk_disable(s_vpu_clk);
	return 0;

DONE_SUSPEND:

	vpu_clk_disable(s_vpu_clk);

	return -EAGAIN;

}
#ifdef CNM_FPGA_PLATFORM
#ifdef CNM_FPGA_PCI_INTERFACE
static int vpu_resume(struct pci_dev *pdev)
#endif
#else
static int vpu_resume(struct platform_device *pdev)
#endif
{
	int i;
	int core;
	u32 val;
	unsigned long timeout = jiffies + HZ;	/* vpu wait timeout to 1sec */
	int product_code;

	unsigned long   code_base;
	u32		  	code_size;
	unsigned long   stack_base;
	u32		  	stack_size;
	u32		  	remap_size;
	int			 regVal;
	u32		  	hwOption  = 0;

	DPRINTK("[VPUDRV] vpu_resume\n");

#ifdef CNM_FPGA_PLATFORM
	return 0;
#endif

	vpu_clk_enable(s_vpu_clk);

	for (core = 0; core < MAX_NUM_VPU_CORE; core++) {

		if (s_bit_firmware_info[core].size == 0) {
			continue;
		}

		product_code = ReadVpuRegister(VPU_PRODUCT_CODE_REGISTER);
        if (PRODUCT_CODE_W_SERIES(product_code)) {
			code_base = s_common_memory.phys_addr;
			/* ALIGN TO 4KB */
			code_size = (W4_MAX_CODE_BUF_SIZE&~0xfff);
			if (code_size < s_bit_firmware_info[core].size*2) {
				goto DONE_WAKEUP;
			}
			stack_base = code_base + code_size;
			stack_size = WAVE4_STACK_SIZE;

			/*---- LOAD BOOT CODE*/
			for (i = 0; i < 512; i++) {
				val = s_bit_firmware_info[core].bit_code[i];
				WriteVpu(code_base+(i*4), val);
			}

			regVal = 0;
			WriteVpuRegister(W4_PO_CONF, regVal);

			/* Reset All blocks */
			regVal = 0x7ffffff;
			WriteVpuRegister(W4_RESET_REQ, regVal);	/*Reset All blocks*/

			/* Waiting reset done */
			while (ReadVpuRegister(W4_RESET_STATUS)) {
				if (time_after(jiffies, timeout))
					goto DONE_WAKEUP;
			}

			WriteVpuRegister(W4_RESET_REQ, 0);

			/* remap page size */
			remap_size = (code_size >> 12) & 0x1ff;
			regVal = 0x80000000 | (W4_REMAP_CODE_INDEX<<12) | (0 << 16) | (1<<11) | remap_size;
			WriteVpuRegister(W4_VPU_REMAP_CTRL,	 regVal);
			WriteVpuRegister(W4_VPU_REMAP_VADDR,	0x00000000);	/* DO NOT CHANGE! */
			WriteVpuRegister(W4_VPU_REMAP_PADDR,	code_base);
			WriteVpuRegister(W4_ADDR_CODE_BASE,	 code_base);
			WriteVpuRegister(W4_CODE_SIZE,		  code_size);
			WriteVpuRegister(W4_CODE_PARAM,		 0);
			WriteVpuRegister(W4_INIT_VPU_TIME_OUT_CNT,   timeout);

			/* Stack */
			WriteVpuRegister(W4_ADDR_STACK_BASE, stack_base);
			WriteVpuRegister(W4_STACK_SIZE,	  stack_size);
#ifdef CNM_FPGA_PLATFORM
			{
				int vdi_current_vclk = 20;
				int divisor;
				int baudrate = 38400;
				divisor	= (vdi_current_vclk * 1000000 + baudrate * 8)/(16 * baudrate);
				hwOption  |= (divisor << 16);
			}
#endif
			WriteVpuRegister(W4_HW_OPTION, hwOption);

			/* Interrupt */
			regVal  = (1<<W4_INT_DEC_PIC_HDR);
			regVal |= (1<<W4_INT_DEC_PIC);
			regVal |= (1<<W4_INT_QUERY_DEC);
			regVal |= (1<<W4_INT_SLEEP_VPU);
			regVal |= (1<<W4_INT_BSBUF_EMPTY);

			WriteVpuRegister(W4_VPU_INT_ENABLE,  regVal);

			Wave4BitIssueCommand(core, W4_CMD_INIT_VPU);
			WriteVpuRegister(W4_VPU_REMAP_PROC_START, 1);

			while (ReadVpuRegister(W4_VPU_BUSY_STATUS)) {
				if (time_after(jiffies, timeout))
					goto DONE_WAKEUP;
			}

			if (ReadVpuRegister(W4_RET_SUCCESS) == 0) {
				DPRINTK("WAKEUP_VPU failed [0x%x]", ReadVpuRegister(W4_RET_FAIL_REASON));
				goto DONE_WAKEUP;
			}
		} else {
		
			WriteVpuRegister(BIT_CODE_RUN, 0);

			/*---- LOAD BOOT CODE*/
			for (i = 0; i < 512; i++) {
				val = s_bit_firmware_info[core].bit_code[i];
				WriteVpuRegister(BIT_CODE_DOWN, ((i << 16) | val));
			}

			for (i = 0 ; i < 64 ; i++)
				WriteVpuRegister(BIT_BASE+(0x100+(i * 4)), s_vpu_reg_store[core][i]);

			WriteVpuRegister(BIT_BUSY_FLAG, 1);
			WriteVpuRegister(BIT_CODE_RESET, 1);
			WriteVpuRegister(BIT_CODE_RUN, 1);

			while (ReadVpuRegister(BIT_BUSY_FLAG)) {
				if (time_after(jiffies, timeout))
					goto DONE_WAKEUP;
			}

#if defined(CODA7L) || defined(CODA851)
			WriteVpuRegister(BIT_BUSY_FLAG, 1);
			WriteVpuRegister(BIT_RUN_INDEX,   s_run_index);
			WriteVpuRegister(BIT_RUN_COD_STD, s_run_codstd);
			WriteVpuRegister(BIT_RUN_COMMAND, VPU_WAKE);

			while (ReadVpuRegister(BIT_BUSY_FLAG)) {
				if (time_after(jiffies, timeout))
					goto DONE_WAKEUP;
			}
#endif
		}

	}

	if (s_vpu_open_ref_count == 0)
		vpu_clk_disable(s_vpu_clk);

DONE_WAKEUP:

	if (s_vpu_open_ref_count > 0)
		vpu_clk_enable(s_vpu_clk);

	return 0;
}
#else
#define	vpu_suspend	NULL
#define	vpu_resume	NULL
#endif				/* !CONFIG_PM */

#ifdef CNM_FPGA_PLATFORM
#ifdef CNM_FPGA_PCI_INTERFACE
static struct pci_driver vpu_driver = {
	.name = VPU_PLATFORM_DEVICE_NAME,
	.id_table = ids,
	.probe = vpu_probe,
	.remove = vpu_remove,
	.suspend = vpu_suspend,
	.resume = vpu_resume,
};
#endif
#else
static struct platform_driver vpu_driver = {
	.driver = {
		   .name = VPU_PLATFORM_DEVICE_NAME,
		   },
	.probe = vpu_probe,
	.remove = vpu_remove,
	.suspend = vpu_suspend,
	.resume = vpu_resume,
};
#endif

static int __init vpu_init(void)
{
	int res;

	DPRINTK("[VPUDRV] begin vpu_init\n");

	init_waitqueue_head(&s_interrupt_wait_q);
	s_common_memory.base = 0;
	s_instance_pool.base = 0;
#ifdef VPU_SUPPORT_PLATFORM_DRIVER_REGISTER
#ifdef CNM_FPGA_PLATFORM
	mutex_init(&s_io_lock);
	s_vpu_drv_context.current_lock_pid = 0;

#ifdef CNM_FPGA_PCI_INTERFACE
	res = pci_register_driver(&vpu_driver);
#endif
#else
	res = platform_driver_register(&vpu_driver);
#endif
#else
	res = platform_driver_register(&vpu_driver);
	res = vpu_probe(NULL);
#endif

	DPRINTK("[VPUDRV] end vpu_init result=0x%x\n", res);
	return res;
}

static void __exit vpu_exit(void)
{
#ifdef VPU_SUPPORT_PLATFORM_DRIVER_REGISTER
	DPRINTK("[VPUDRV] vpu_exit\n");

#ifdef CNM_FPGA_PLATFORM
#ifdef CNM_FPGA_PCI_INTERFACE
	cancel_work_sync(&wq);
	pci_unregister_driver(&vpu_driver);
	flush_scheduled_work();
#endif
#else
	platform_driver_unregister(&vpu_driver);
#endif

#else

	vpu_clk_put(s_vpu_clk);

	if (s_instance_pool.base) {
#ifdef USE_VMALLOC_FOR_INSTANCE_POOL_MEMORY
		vfree((const void *)s_instance_pool.base);
#else			
#ifdef CNM_FPGA_PLATFORM
		dma_free_coherent(0, PAGE_ALIGN(s_instance_pool.size), (void *)s_instance_pool.base, s_instance_pool.phys_addr);
#else
		vpu_free_dma_buffer(&s_instance_pool);
#endif /* CNM_FPGA_PLATFORM */
#endif
		s_instance_pool.base = 0;
	}

	if (s_common_memory.base) {
		vpu_free_dma_buffer(&s_common_memory);
		s_common_memory.base = 0;
	}

#ifdef VPU_SUPPORT_RESERVED_VIDEO_MEMORY
	if (s_video_memory.base) {
#ifdef CNM_FPGA_PLATFORM
#else
		iounmap((void *)s_video_memory.base);
#endif /* CNM_FPGA_PLATFORM */
		s_video_memory.base = 0;

		vmem_exit(&s_vmem);
	}
#endif

	if (s_vpu_major > 0) {
		cdev_del(&s_vpu_cdev);
		unregister_chrdev_region(s_vpu_major, 1);
		s_vpu_major = 0;
	}

#ifdef VPU_SUPPORT_ISR
	if (s_vpu_irq)
		free_irq(s_vpu_irq, &s_vpu_drv_context);
#endif

	if (s_vpu_register.virt_addr) {
		iounmap((void *)s_vpu_register.virt_addr);
		s_vpu_register.virt_addr = 0x00;
	}

#endif

	return;
}

MODULE_AUTHOR("A customer using C&M VPU, Inc.");
MODULE_DESCRIPTION("VPU linux driver");
MODULE_LICENSE("GPL");

module_init(vpu_init);
module_exit(vpu_exit);

int vpu_hw_reset(void)
{
#ifdef CNM_FPGA_PLATFORM
#endif
	DPRINTK("[VPUDRV] request vpu reset from application. \n");
	return 0;
}

struct clk *vpu_clk_get(struct device *dev)
{
#ifdef CNM_FPGA_PLATFORM
	return NULL;
#else
	return clk_get(dev, VPU_CLK_NAME);
#endif
}
void vpu_clk_put(struct clk *clk)
{
#ifdef CNM_FPGA_PLATFORM
#else
	if (!(clk == NULL || IS_ERR(clk)))
		clk_put(clk);
#endif
}
int vpu_clk_enable(struct clk *clk)
{

#ifdef CNM_FPGA_PLATFORM
	return 1;
#else
		if (!(clk == NULL || IS_ERR(clk))) {
		/* the bellow is for C&M EVB.*/
		/*
		{
			struct clk *s_vpuext_clk = NULL;
			s_vpuext_clk = clk_get(NULL, "vcore");
			if (s_vpuext_clk)
			{
				DPRINTK("[VPUDRV] vcore clk=%p\n", s_vpuext_clk);
				clk_enable(s_vpuext_clk);
			}

			DPRINTK("[VPUDRV] vbus clk=%p\n", s_vpuext_clk);
			if (s_vpuext_clk)
			{
				s_vpuext_clk = clk_get(NULL, "vbus");
				clk_enable(s_vpuext_clk);
			}
		}
		*/
		/* for C&M EVB. */

		DPRINTK("[VPUDRV] vpu_clk_enable\n");
		return clk_enable(clk);
	}

	return 0;
#endif
}

void vpu_clk_disable(struct clk *clk)
{
	if (!(clk == NULL || IS_ERR(clk))) {
		DPRINTK("[VPUDRV] vpu_clk_disable\n");
		clk_disable(clk);
	}
}
 
