//--=========================================================================--
//  This file is linux device driver for VPU.
//-----------------------------------------------------------------------------
//
//       This confidential and proprietary software may be used only
//     as authorized by a licensing agreement from Chips&Media Inc.
//     In the event of publication, the following notice is applicable:
//
//            (C) COPYRIGHT 2006 - 2015  CHIPS&MEDIA INC.
//                      ALL RIGHTS RESERVED
//
//       The entire notice above must be reproduced on all authorized
//       copies.
//
//--=========================================================================-

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
#include <linux/reset.h>
#include <linux/version.h>
#include <linux/of.h>
#include <linux/pm_runtime.h>
#include <soc/sifive/sifive_l2_cache.h>

#include "../../../vpuapi/vpuconfig.h"

#include "vpu.h"

#ifndef  CONFIG_PM
#define CONFIG_PM
#endif

//#define ENABLE_DEBUG_MSG
#ifdef ENABLE_DEBUG_MSG
#define DPRINTK(args...)		printk(KERN_INFO args);
#else
#define DPRINTK(args...)
#endif

/* definitions to be changed as customer  configuration */
/* if you want to have clock gating scheme frame by frame */
//#define VPU_SUPPORT_CLOCK_CONTROL

/* if clktree is work,try this...*/
#define STARFIVE_VPU_SUPPORT_CLOCK_CONTROL

/* if the driver want to use interrupt service from kernel ISR */
#define VPU_SUPPORT_ISR

/* if the platform driver knows the name of this driver */
/* VPU_PLATFORM_DEVICE_NAME */
#define VPU_SUPPORT_PLATFORM_DRIVER_REGISTER

/* if this driver knows the dedicated video memory address */
//#define VPU_SUPPORT_RESERVED_VIDEO_MEMORY

static void starfive_flush_dcache(unsigned long start, unsigned long len)
{
	sifive_l2_flush64_range(start, len);
}

#define VPU_PLATFORM_DEVICE_NAME "venc"
#define VPU_CLK_NAME "vcoenc"
#define VPU_DEV_NAME "venc"

/* if the platform driver knows this driver */
/* the definition of VPU_REG_BASE_ADDR and VPU_REG_SIZE are not meaningful */

//#define VPU_REG_BASE_ADDR 0x75000000
#define VPU_REG_BASE_ADDR 0x130B0000

#define VPU_REG_SIZE (0x4000*MAX_NUM_VPU_CORE)

#ifdef VPU_SUPPORT_ISR
#define VPU_IRQ_NUM (15)
#endif

/* this definition is only for chipsnmedia FPGA board env */
/* so for SOC env of customers can be ignored */

#ifndef VM_RESERVED	/*for kernel up to 3.7.0 version*/
# define  VM_RESERVED   (VM_DONTEXPAND | VM_DONTDUMP)
#endif

#ifdef STARFIVE_VPU_SUPPORT_CLOCK_CONTROL

typedef struct vpu_clkgen_t {
	void __iomem *en_ctrl;
	uint32_t rst_mask;
} vpu_clkgen_t;

struct clk_bulk_data vpu_clks[] = {
		{ .id = "apb_clk" },
		{ .id = "axi_clk" },
		{ .id = "bpu_clk" },
		{ .id = "vce_clk" },
		{ .id = "noc_bus" },
};

typedef struct vpu_clk_t{
#ifndef STARFIVE_VPU_SUPPORT_CLOCK_CONTROL
		void __iomem *clkgen;
		void __iomem *rst_ctrl;
		void __iomem *rst_status;
		void __iomem *noc_bus;
		uint32_t en_shift;
		uint32_t en_mask;
		vpu_clkgen_t apb_clk;
		vpu_clkgen_t axi_clk;
		vpu_clkgen_t bpu_clk;
		vpu_clkgen_t vce_clk;
		vpu_clkgen_t aximem_128b;
#else
		struct clk_bulk_data *clks;
		struct reset_control *resets;
		int nr_clks;
#endif
		struct device *dev;
		bool noc_ctrl;

}vpu_clk_t;
#endif /*STARFIVE_VPU_SUPPORT_CLOCK_CONTROL*/

struct device *vpu_dev;

typedef struct vpu_drv_context_t {
	struct fasync_struct *async_queue;
	unsigned long interrupt_reason;
	u32 open_count;					 /*!<< device reference count. Not instance count */
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
#	define VPU_INIT_VIDEO_MEMORY_SIZE_IN_BYTE (62*1024*1024)
#	define VPU_DRAM_PHYSICAL_BASE 0x86C00000
#include "vmm.h"
static video_mm_t s_vmem;
static vpudrv_buffer_t s_video_memory = {0};
#endif /*VPU_SUPPORT_RESERVED_VIDEO_MEMORY*/

static int vpu_hw_reset(void);
static void vpu_clk_disable(vpu_clk_t *clk);
static int vpu_clk_enable(vpu_clk_t *clk);
static vpu_clk_t *vpu_clk_get(struct platform_device *pdev);
static void vpu_clk_put(vpu_clk_t *clk);

/* end customer definition */
static vpudrv_buffer_t s_instance_pool = {0};
static vpudrv_buffer_t s_common_memory = {0};
static vpu_drv_context_t s_vpu_drv_context;
static int s_vpu_major;
static struct cdev s_vpu_cdev;

static struct vpu_clk_t *s_vpu_clk;
static int s_vpu_open_ref_count;
#ifdef VPU_SUPPORT_ISR
static int s_vpu_irq = VPU_IRQ_NUM;
#endif

static vpudrv_buffer_t s_vpu_register = {0};

static int s_interrupt_flag;
static wait_queue_head_t s_interrupt_wait_q;

static spinlock_t s_vpu_lock = __SPIN_LOCK_UNLOCKED(s_vpu_lock);
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,36)
static DECLARE_MUTEX(s_vpu_sem);
#else
static DEFINE_SEMAPHORE(s_vpu_sem);
#endif
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
#define W4_VPU_VINT_CLEAR				(W4_REG_BASE + 0x003C)
#define W4_VPU_VPU_INT_STS				  (W4_REG_BASE + 0x0044)
#define W4_VPU_INT_REASON			   (W4_REG_BASE + 0x004c)

#define W4_RET_SUCCESS				  (W4_REG_BASE + 0x0110)
#define W4_RET_FAIL_REASON			  (W4_REG_BASE + 0x0114)

/* WAVE4 INIT, WAKEUP */
#define W4_PO_CONF					  (W4_REG_BASE + 0x0000)
#define W4_VCPU_CUR_PC				  (W4_REG_BASE + 0x0004)

#define W4_VPU_VINT_ENABLE			   (W4_REG_BASE + 0x0048)

#define W4_VPU_RESET_REQ					(W4_REG_BASE + 0x0050)
#define W4_VPU_RESET_STATUS				 (W4_REG_BASE + 0x0054)

#define W4_VPU_REMAP_CTRL			   (W4_REG_BASE + 0x0060)
#define W4_VPU_REMAP_VADDR			  (W4_REG_BASE + 0x0064)
#define W4_VPU_REMAP_PADDR			  (W4_REG_BASE + 0x0068)
#define W4_VPU_REMAP_CORE_START		 (W4_REG_BASE + 0x006C)
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

enum {
    W5_INT_INIT_VPU          = 0,
    W5_INT_WAKEUP_VPU        = 1,
    W5_INT_SLEEP_VPU         = 2,
    W5_INT_CREATE_INSTANCE   = 3, 
    W5_INT_FLUSH_INSTANCE    = 4,
    W5_INT_DESTORY_INSTANCE  = 5,
    W5_INT_INIT_SEQ          = 6,
    W5_INT_SET_FRAMEBUF      = 7,
    W5_INT_DEC_PIC           = 8,
    W5_INT_ENC_PIC           = 8,
    W5_INT_ENC_SET_PARAM     = 9,
    W5_INT_DEC_QUERY         = 14,
    W5_INT_BSBUF_EMPTY       = 15,
};

#define W4_HW_OPTION					(W4_REG_BASE + 0x0124)
#define W4_CODE_SIZE					(W4_REG_BASE + 0x011C)
/* Note: W4_INIT_CODE_BASE_ADDR should be aligned to 4KB */
#define W4_ADDR_CODE_BASE			   (W4_REG_BASE + 0x0118)
#define W4_CODE_PARAM				   (W4_REG_BASE + 0x0120)
#define W4_INIT_VPU_TIME_OUT_CNT		(W4_REG_BASE + 0x0134)


/* WAVE5 registers */
#define W5_ADDR_CODE_BASE               (W4_REG_BASE + 0x0110) 
#define W5_CODE_SIZE				    (W4_REG_BASE + 0x0114)
#define W5_CODE_PARAM				    (W4_REG_BASE + 0x0128)
#define W5_INIT_VPU_TIME_OUT_CNT		(W4_REG_BASE + 0x0130)

#define W5_HW_OPTION					(W4_REG_BASE + 0x012C)

#define W5_RET_SUCCESS                (W4_REG_BASE + 0x0108)


/* WAVE4 Wave4BitIssueCommand */
#define W4_CORE_INDEX			(W4_REG_BASE + 0x0104)
#define W4_INST_INDEX			(W4_REG_BASE + 0x0108)
#define W4_COMMAND			   (W4_REG_BASE + 0x0100)
#define W4_VPU_HOST_INT_REQ	  (W4_REG_BASE + 0x0038)

/* Product register */
#define VPU_PRODUCT_CODE_REGISTER   (BIT_BASE + 0x1044)

static u32	s_vpu_reg_store[MAX_NUM_VPU_CORE][64];
#endif

#define	ReadVpuRegister(addr)		*(volatile unsigned int *)(s_vpu_register.virt_addr + s_bit_firmware_info[core].reg_base_offset + addr)
#define	WriteVpuRegister(addr, val)	*(volatile unsigned int *)(s_vpu_register.virt_addr + s_bit_firmware_info[core].reg_base_offset + addr) = (unsigned int)val
#define	WriteVpu(addr, val)			*(volatile unsigned int *)(addr) = (unsigned int)val;


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
	vb->base = (unsigned long)dma_alloc_coherent(vpu_dev, PAGE_ALIGN(vb->size), (dma_addr_t *) (&vb->phys_addr), GFP_DMA | GFP_KERNEL);
	if ((void *)(vb->base) == NULL)	{
		printk(KERN_ERR "[VPUDRV] Physical memory allocation error size=%d\n", vb->size);
		return -1;
	}
	starfive_flush_dcache(vb->phys_addr,PAGE_ALIGN(vb->size));
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
		dma_free_coherent(vpu_dev, PAGE_ALIGN(vb->size), (void *)vb->base, vb->phys_addr);
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
			if (ReadVpuRegister(W4_VPU_VPU_INT_STS)) {
				dev->interrupt_reason = ReadVpuRegister(W4_VPU_INT_REASON);
				WriteVpuRegister(W4_VPU_INT_REASON_CLEAR, dev->interrupt_reason);
				WriteVpuRegister(W4_VPU_VINT_CLEAR, 0x1);
			}
		} 
        else if (PRODUCT_CODE_NOT_W_SERIES(product_code)) {
			if (ReadVpuRegister(BIT_INT_STS)) {
				dev->interrupt_reason = ReadVpuRegister(BIT_INT_REASON);
				WriteVpuRegister(BIT_INT_CLEAR, 0x1);
			}
		}
        else {
            DPRINTK("[VPUDRV] Unknown product id : %08x\n", product_code);
            continue;
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

			ret = wait_event_interruptible_timeout(s_interrupt_wait_q, s_interrupt_flag != 0, msecs_to_jiffies(info.timeout));
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
					
						if (vpu_alloc_dma_buffer(&s_instance_pool) != -1)
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
			DPRINTK("[VPUDRV][-]VDI_IOCTL_GET_REGISTER_INFO s_vpu_register.phys_addr==0x%lx, s_vpu_register.virt_addr=0x%lx, s_vpu_register.size=%d\n", s_vpu_register.phys_addr , s_vpu_register.virt_addr, s_vpu_register.size);
		}
		break;
	case VDI_IOCTL_FLUSH_DCACHE:
        {
            vpudrv_flush_cache_t cache_info;

            DPRINTK("[JPUDRV][+]VDI_IOCTL_FLUSH_DCACHE\n");
            ret = copy_from_user(&cache_info, (vpudrv_flush_cache_t *)arg, sizeof(vpudrv_flush_cache_t));
            if (ret != 0)
                ret = -EFAULT;

            if(cache_info.flag)
                starfive_flush_dcache(cache_info.start,cache_info.size);

            DPRINTK("[JPUDRV][-]VDI_IOCTL_FLUSH_DCACHE\n");
            break;
        }
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
        s_vpu_drv_context.open_count--;
        if (s_vpu_drv_context.open_count == 0) {
            if (s_instance_pool.base) {
                DPRINTK("[VPUDRV] free instance pool\n");
#ifdef USE_VMALLOC_FOR_INSTANCE_POOL_MEMORY
                vfree((const void *)s_instance_pool.base);
#else			
                vpu_free_dma_buffer(&s_instance_pool);
#endif
                s_instance_pool.base = 0;
            }
            
            if (s_common_memory.base) {
                DPRINTK("[VPUDRV] free common memory\n");
                vpu_free_dma_buffer(&s_common_memory);
                s_common_memory.base = 0;
            }

        }
    }
    up(&s_vpu_sem);

	vpu_hw_reset();
	
    return 0;
}

static int vpu_fasync(int fd, struct file *filp, int mode)
{
	struct vpu_drv_context_t *dev = (struct vpu_drv_context_t *)filp->private_data;
	return fasync_helper(fd, filp, mode, &dev->async_queue);
}


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
	vm->vm_page_prot = pgprot_noncached(vm->vm_page_prot);

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
	

	return 0;
#else
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


static int vpu_probe(struct platform_device *pdev)
{
	int err = 0;
	struct resource *res = NULL;

	DPRINTK("[VPUDRV] vpu_probe\n");

	if(pdev){
		vpu_dev = &pdev->dev;
		vpu_dev->coherent_dma_mask = 0xffffffff;
	}
	
	if (pdev)
		res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (res) {/* if platform driver is implemented */
		s_vpu_register.phys_addr = res->start;
		s_vpu_register.virt_addr = (unsigned long)ioremap_nocache(res->start, res->end - res->start);
		s_vpu_register.size = res->end - res->start;
		DPRINTK("[VPUDRV] : vpu base address get from platform driver physical base addr==0x%lx, virtual base=0x%lx\n", s_vpu_register.phys_addr , s_vpu_register.virt_addr);
	} else {
		s_vpu_register.phys_addr = VPU_REG_BASE_ADDR;
		s_vpu_register.virt_addr = (unsigned long)ioremap_nocache(s_vpu_register.phys_addr, VPU_REG_SIZE);
		s_vpu_register.size = VPU_REG_SIZE;
		DPRINTK("[VPUDRV] : vpu base address get from defined value physical base addr==0x%lx, virtual base=0x%lx\n", s_vpu_register.phys_addr, s_vpu_register.virt_addr);

	}

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
		s_vpu_clk = vpu_clk_get(pdev);
	else
		s_vpu_clk = vpu_clk_get(NULL);

	if (!s_vpu_clk)
		printk(KERN_ERR "[VPUDRV] : not support clock controller.\n");
	else
		DPRINTK("[VPUDRV] : get clock controller s_vpu_clk=%p\n", s_vpu_clk);

#ifdef VPU_SUPPORT_CLOCK_CONTROL
#else
	vpu_clk_enable(s_vpu_clk);
#endif

#ifdef VPU_SUPPORT_ISR
#ifdef VPU_SUPPORT_PLATFORM_DRIVER_REGISTER
	if (pdev)
		res = platform_get_resource(pdev, IORESOURCE_IRQ, 0);
	if (res) {/* if platform driver is implemented */
		s_vpu_irq = res->start;
		DPRINTK("[VPUDRV] : vpu irq number get from platform driver irq=0x%x\n", s_vpu_irq);
	} else {
		DPRINTK("[VPUDRV] : vpu irq number get from defined value irq=0x%x\n", s_vpu_irq);
	}
#else
	DPRINTK("[VPUDRV] : vpu irq number get from defined value irq=0x%x\n", s_vpu_irq);
#endif

	err = request_irq(s_vpu_irq, vpu_irq_handler, 0, "VPU_CODEC_IRQ", (void *)(&s_vpu_drv_context));
	if (err) {
		printk(KERN_ERR "[VPUDRV] :  fail to register interrupt handler\n");
		goto ERROR_PROVE_DEVICE;
	}
#endif

#ifdef VPU_SUPPORT_RESERVED_VIDEO_MEMORY
	s_video_memory.size = VPU_INIT_VIDEO_MEMORY_SIZE_IN_BYTE;
	s_video_memory.phys_addr = VPU_DRAM_PHYSICAL_BASE;
	s_video_memory.base = (unsigned long)ioremap_nocache(s_video_memory.phys_addr, PAGE_ALIGN(s_video_memory.size));
	if (!s_video_memory.base) {
		printk(KERN_ERR "[VPUDRV] :  fail to remap video memory physical phys_addr==0x%lx, base==0x%lx, size=%d\n", s_video_memory.phys_addr, s_video_memory.base, (int)s_video_memory.size);
		goto ERROR_PROVE_DEVICE;
	}

	if (vmem_init(&s_vmem, s_video_memory.phys_addr, s_video_memory.size) < 0) {
		printk(KERN_ERR "[VPUDRV] :  fail to init vmem system\n");
		goto ERROR_PROVE_DEVICE;
	}
	DPRINTK("[VPUDRV] success to probe vpu device with reserved video memory phys_addr==0x%lx, base = =0x%lx\n", s_video_memory.phys_addr, s_video_memory.base);
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

static int vpu_remove(struct platform_device *pdev)
{
	DPRINTK("[VPUDRV] vpu_remove\n");
#ifdef VPU_SUPPORT_PLATFORM_DRIVER_REGISTER

	if (s_instance_pool.base) {
#ifdef USE_VMALLOC_FOR_INSTANCE_POOL_MEMORY
		vfree((const void *)s_instance_pool.base);
#else		
		vpu_free_dma_buffer(&s_instance_pool);
#endif
		s_instance_pool.base = 0;
	}

	if (s_common_memory.base) {
		vpu_free_dma_buffer(&s_common_memory);
		s_common_memory.base = 0;
	}

#ifdef VPU_SUPPORT_RESERVED_VIDEO_MEMORY
	if (s_video_memory.base) {
		iounmap((void *)s_video_memory.base);		
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

	vpu_clk_disable(s_vpu_clk);
	vpu_clk_put(s_vpu_clk);

#endif /*VPU_SUPPORT_PLATFORM_DRIVER_REGISTER*/

	return 0;
}

#ifdef CONFIG_PM
#define W4_MAX_CODE_BUF_SIZE	 (512*1024)
#define W4_CMD_INIT_VPU		  (0x0001)
#define W4_CMD_SLEEP_VPU		 (0x0400)
#define W4_CMD_WAKEUP_VPU		(0x0800)
#define W5_CMD_SLEEP_VPU        (0x0004)
#define W5_CMD_WAKEUP_VPU        (0x0002)

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

static int vpu_suspend(struct platform_device *pdev, pm_message_t state)
{
    int i;
    int core;
    unsigned long timeout = jiffies + HZ;	/* vpu wait timeout to 1sec */
    int product_code;

    DPRINTK("[VPUDRV] vpu_suspend\n");

    vpu_clk_enable(s_vpu_clk);

    if (s_vpu_open_ref_count > 0) {
        for (core = 0; core < MAX_NUM_VPU_CORE; core++) {
            if (s_bit_firmware_info[core].size == 0)
                continue;
            product_code = ReadVpuRegister(VPU_PRODUCT_CODE_REGISTER);
            if (PRODUCT_CODE_W_SERIES(product_code)) {
                unsigned long cmd_reg = W4_CMD_SLEEP_VPU;
                unsigned long suc_reg = W4_RET_SUCCESS;

                while (ReadVpuRegister(W4_VPU_BUSY_STATUS)) {
                    if (time_after(jiffies, timeout)) {
                        DPRINTK("SLEEP_VPU BUSY timeout");
                        goto DONE_SUSPEND;
                    }
                }

                if (product_code == WAVE512_CODE || product_code == WAVE520_CODE) {
                    cmd_reg = W5_CMD_SLEEP_VPU;
                    suc_reg = W5_RET_SUCCESS;
                }
                Wave4BitIssueCommand(core, cmd_reg);

                while (ReadVpuRegister(W4_VPU_BUSY_STATUS)) {
                    if (time_after(jiffies, timeout)) {
                        DPRINTK("SLEEP_VPU BUSY timeout");
                        goto DONE_SUSPEND;
                    }
                }
                if (ReadVpuRegister(suc_reg) == 0) {
                    DPRINTK("SLEEP_VPU failed [0x%x]", ReadVpuRegister(W4_RET_FAIL_REASON));
                    goto DONE_SUSPEND;
                }
            }
            else if (PRODUCT_CODE_NOT_W_SERIES(product_code)) {
                while (ReadVpuRegister(BIT_BUSY_FLAG)) {
                    if (time_after(jiffies, timeout))
                        goto DONE_SUSPEND;
                }

                for (i = 0; i < 64; i++)
                    s_vpu_reg_store[core][i] = ReadVpuRegister(BIT_BASE+(0x100+(i * 4)));
            }
            else {
                DPRINTK("[VPUDRV] Unknown product id : %08x\n", product_code);
                goto DONE_SUSPEND;
            }
        }
    }

    vpu_clk_disable(s_vpu_clk);
    return 0;

DONE_SUSPEND:

    vpu_clk_disable(s_vpu_clk);

    return -EAGAIN;

}
static int vpu_resume(struct platform_device *pdev)
{
    int i;
    int core;
    u32 val;
    unsigned long timeout = jiffies + HZ;	/* vpu wait timeout to 1sec */
    int product_code;

    unsigned long   code_base;
    u32		  	code_size;
    u32		  	remap_size;
    int			 regVal;
    u32		  	hwOption  = 0;


    DPRINTK("[VPUDRV] vpu_resume\n");

    vpu_clk_enable(s_vpu_clk);

    for (core = 0; core < MAX_NUM_VPU_CORE; core++) {

        if (s_bit_firmware_info[core].size == 0) {
            continue;
        }

        product_code = ReadVpuRegister(VPU_PRODUCT_CODE_REGISTER);
        if (PRODUCT_CODE_W_SERIES(product_code)) {
            unsigned long addr_code_base_reg = W4_ADDR_CODE_BASE;
            unsigned long code_size_reg = W4_CODE_SIZE;
            unsigned long code_param_reg = W4_CODE_PARAM;
            unsigned long timeout_cnt_reg = W4_INIT_VPU_TIME_OUT_CNT;
            unsigned long hw_opt_reg = W4_HW_OPTION;
            unsigned long suc_reg = W4_RET_SUCCESS;

            if (product_code == WAVE512_CODE || product_code == WAVE520_CODE) {
                addr_code_base_reg = W5_ADDR_CODE_BASE;
                code_size_reg = W5_CODE_SIZE;
                code_param_reg = W5_CODE_PARAM;
                timeout_cnt_reg = W5_INIT_VPU_TIME_OUT_CNT;
                hw_opt_reg = W5_HW_OPTION;
                suc_reg = W5_RET_SUCCESS;
            }

            code_base = s_common_memory.phys_addr;
            /* ALIGN TO 4KB */
            code_size = (W4_MAX_CODE_BUF_SIZE&~0xfff);
            if (code_size < s_bit_firmware_info[core].size*2) {
                goto DONE_WAKEUP;
            }

            regVal = 0;
            WriteVpuRegister(W4_PO_CONF, regVal);

            /* Reset All blocks */
            regVal = 0x7ffffff;
            WriteVpuRegister(W4_VPU_RESET_REQ, regVal);	/*Reset All blocks*/

            /* Waiting reset done */
            while (ReadVpuRegister(W4_VPU_RESET_STATUS)) {
                if (time_after(jiffies, timeout))
                    goto DONE_WAKEUP;
            }

            WriteVpuRegister(W4_VPU_RESET_REQ, 0);

            /* remap page size */
            remap_size = (code_size >> 12) & 0x1ff;
            regVal = 0x80000000 | (W4_REMAP_CODE_INDEX<<12) | (0 << 16) | (1<<11) | remap_size;
            WriteVpuRegister(W4_VPU_REMAP_CTRL,	 regVal);
            WriteVpuRegister(W4_VPU_REMAP_VADDR,	0x00000000);	/* DO NOT CHANGE! */
            WriteVpuRegister(W4_VPU_REMAP_PADDR,	code_base);
            WriteVpuRegister(addr_code_base_reg,	 code_base);
            WriteVpuRegister(code_size_reg,		  code_size);
            WriteVpuRegister(code_param_reg,		 0);
            WriteVpuRegister(timeout_cnt_reg,   timeout);

            WriteVpuRegister(hw_opt_reg, hwOption);

            /* Interrupt */
            if (product_code == WAVE512_CODE) {
                // decoder
                regVal  = (1<<W5_INT_INIT_SEQ);
                regVal |= (1<<W5_INT_DEC_PIC);
                regVal |= (1<<W5_INT_BSBUF_EMPTY);
            }
            else if (product_code == WAVE520_CODE) {
                regVal  = (1<<W5_INT_ENC_SET_PARAM);
                regVal |= (1<<W5_INT_ENC_PIC);
            }
            else {
                regVal  = (1<<W4_INT_DEC_PIC_HDR);
                regVal |= (1<<W4_INT_DEC_PIC);
                regVal |= (1<<W4_INT_QUERY_DEC);
                regVal |= (1<<W4_INT_SLEEP_VPU);
                regVal |= (1<<W4_INT_BSBUF_EMPTY);
            }

            WriteVpuRegister(W4_VPU_VINT_ENABLE,  regVal);

            Wave4BitIssueCommand(core, W4_CMD_INIT_VPU);
            WriteVpuRegister(W4_VPU_REMAP_CORE_START, 1);

            while (ReadVpuRegister(W4_VPU_BUSY_STATUS)) {
                if (time_after(jiffies, timeout))
                    goto DONE_WAKEUP;
            }

            if (ReadVpuRegister(suc_reg) == 0) {
                DPRINTK("WAKEUP_VPU failed [0x%x]", ReadVpuRegister(W4_RET_FAIL_REASON));
                goto DONE_WAKEUP;
            }
        }
        else if (PRODUCT_CODE_NOT_W_SERIES(product_code)) {

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

        }
        else {
            DPRINTK("[VPUDRV] Unknown product id : %08x\n", product_code);
            goto DONE_WAKEUP;
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

#ifdef VPU_SUPPORT_PLATFORM_DRIVER_REGISTER
static const struct of_device_id vpu_of_id_table[] = {
	{ .compatible = "cnm,cnm420l-vpu" },
	{ .compatible = "starfive,venc" },
	{}
};
MODULE_DEVICE_TABLE(of, vpu_of_id_table);

static struct platform_driver vpu_driver = {
	.driver = {
		   .name = VPU_PLATFORM_DEVICE_NAME,
		   .of_match_table = of_match_ptr(vpu_of_id_table),
		   },
	.probe = vpu_probe,
	.remove = vpu_remove,
	.suspend = vpu_suspend,
	.resume = vpu_resume,
};
#endif /* VPU_SUPPORT_PLATFORM_DRIVER_REGISTER */

static int __init vpu_init(void)
{
	int res;

	DPRINTK("[VPUDRV] begin vpu_init\n");

	init_waitqueue_head(&s_interrupt_wait_q);
	s_common_memory.base = 0;
	s_instance_pool.base = 0;
#ifdef VPU_SUPPORT_PLATFORM_DRIVER_REGISTER
	res = platform_driver_register(&vpu_driver);
#else
	res = vpu_probe(NULL);
#endif /* VPU_SUPPORT_PLATFORM_DRIVER_REGISTER */

	DPRINTK("[VPUDRV] end vpu_init result=0x%x\n", res);
	return res;
}

static void __exit vpu_exit(void)
{
#ifdef VPU_SUPPORT_PLATFORM_DRIVER_REGISTER
	DPRINTK("[VPUDRV] vpu_exit\n");

	platform_driver_unregister(&vpu_driver);

#else /* VPU_SUPPORT_PLATFORM_DRIVER_REGISTER */

#ifdef VPU_SUPPORT_CLOCK_CONTROL
#else
	vpu_clk_disable(s_vpu_clk);
#endif
	vpu_clk_put(s_vpu_clk);

	if (s_instance_pool.base) {
#ifdef USE_VMALLOC_FOR_INSTANCE_POOL_MEMORY
		vfree((const void *)s_instance_pool.base);
#else			
		vpu_free_dma_buffer(&s_instance_pool);
#endif
		s_instance_pool.base = 0;
	}

	if (s_common_memory.base) {
		vpu_free_dma_buffer(&s_common_memory);
		s_common_memory.base = 0;
	}

#ifdef VPU_SUPPORT_RESERVED_VIDEO_MEMORY
	if (s_video_memory.base) {
		iounmap((void *)s_video_memory.base);
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

#endif /* VPU_SUPPORT_PLATFORM_DRIVER_REGISTER */

	return;
}

MODULE_AUTHOR("A customer using C&M VPU, Inc.");
MODULE_DESCRIPTION("VPU linux driver");
MODULE_LICENSE("GPL");

module_init(vpu_init);
module_exit(vpu_exit);

static int vpu_pmu_enable(struct device *dev)
{
	int ret;

	pm_runtime_enable(dev);
	ret = pm_runtime_get_sync(dev);
	if (ret < 0)
		dev_err(dev, "failed to get pm runtime: %d\n", ret);

	return ret;
}

static void vpu_pmu_disable(struct device *dev)
{
	pm_runtime_put_sync(dev);
	pm_runtime_disable(dev);
}

#ifndef STARFIVE_VPU_SUPPORT_CLOCK_CONTROL

#define CLK_ENABLE_DATA			1
#define CLK_DISABLE_DATA		0
#define CLK_EN_SHIFT			31
#define CLK_EN_MASK			0x80000000U

#define SAIF_BD_APBS_BASE		0x13020000
#define WAVE420L_CLK_AXI_CTRL		0x138U
#define WAVE420L_CLK_BPU_CTRL		0x13cU
#define WAVE420L_CLK_VCE_CTRL		0x140U
#define WAVE420L_CLK_APB_CTRL		0x144U
#define WAVE420L_CLK_NOCBUS_CTRL	0x148U

#define RSTGEN_SOFTWARE_RESET_ASSERT1	0x2FCU
#define RSTGEN_SOFTWARE_RESET_STATUS1	0x30CU

#define RSTN_AXI_MASK			(0x1 << 22)
#define RSTN_BPU_MASK			(0x1 << 23)
#define RSTN_VCE_MASK			(0x1 << 24)
#define RSTN_APB_MASK			(0x1 << 25)
#define RSTN_128B_AXIMEM_MASK		(0x1 << 26)

static void saif_set_reg(volatile void __iomem *addr, uint32_t data,
			uint32_t shift, uint32_t mask)
{
	uint32_t tmp;

	tmp = readl(addr);
	tmp &= ~mask;
	tmp |= (data << shift) & mask;
	writel(tmp, addr);
}

static void saif_assert_rst(volatile void __iomem *addr,
			const volatile void __iomem *addr_status, uint32_t mask)
{
	uint32_t tmp;

	tmp = readl(addr);
	tmp |= mask;
	writel(tmp, addr);
	do {
		tmp = readl(addr_status);
	} while ((tmp & mask) != 0);
}

static void saif_clear_rst(volatile void __iomem *addr,
			const volatile void __iomem *addr_status, uint32_t mask)
{
	uint32_t tmp;

	tmp = readl(addr);
	tmp &= ~mask;
	writel(tmp, addr);
	do {
		tmp = readl(addr_status);
	} while ((tmp & mask) != mask);
}

static void vpu_noc_vdec_bus_control(vpu_clk_t *clk, bool enable)
{
	if (enable)
		saif_set_reg(clk->noc_bus, CLK_ENABLE_DATA, clk->en_shift, clk->en_mask);
	else
		saif_set_reg(clk->noc_bus, CLK_DISABLE_DATA, clk->en_shift, clk->en_mask);
}

static void vpu_clk_control(vpu_clk_t *clk, bool enable)
{
	if (enable) {
		/*enable*/
		saif_set_reg(clk->apb_clk.en_ctrl, CLK_ENABLE_DATA, clk->en_shift, clk->en_mask);
		saif_set_reg(clk->axi_clk.en_ctrl, CLK_ENABLE_DATA, clk->en_shift, clk->en_mask);
		saif_set_reg(clk->bpu_clk.en_ctrl, CLK_ENABLE_DATA, clk->en_shift, clk->en_mask);
		saif_set_reg(clk->vce_clk.en_ctrl, CLK_ENABLE_DATA, clk->en_shift, clk->en_mask);

		/*clr-reset*/
		saif_clear_rst(clk->rst_ctrl, clk->rst_status, clk->apb_clk.rst_mask);
		saif_clear_rst(clk->rst_ctrl, clk->rst_status, clk->axi_clk.rst_mask);
		saif_clear_rst(clk->rst_ctrl, clk->rst_status, clk->bpu_clk.rst_mask);
		saif_clear_rst(clk->rst_ctrl, clk->rst_status, clk->vce_clk.rst_mask);
		saif_clear_rst(clk->rst_ctrl, clk->rst_status, clk->aximem_128b.rst_mask);
	} else {
		/*assert-reset*/
		saif_assert_rst(clk->rst_ctrl, clk->rst_status, clk->apb_clk.rst_mask);
		saif_assert_rst(clk->rst_ctrl, clk->rst_status, clk->axi_clk.rst_mask);
		saif_assert_rst(clk->rst_ctrl, clk->rst_status, clk->bpu_clk.rst_mask);
		saif_assert_rst(clk->rst_ctrl, clk->rst_status, clk->vce_clk.rst_mask);
		saif_assert_rst(clk->rst_ctrl, clk->rst_status, clk->aximem_128b.rst_mask);

		/*disable*/
		saif_set_reg(clk->apb_clk.en_ctrl, CLK_DISABLE_DATA, clk->en_shift, clk->en_mask);
		saif_set_reg(clk->axi_clk.en_ctrl, CLK_DISABLE_DATA, clk->en_shift, clk->en_mask);
		saif_set_reg(clk->bpu_clk.en_ctrl, CLK_DISABLE_DATA, clk->en_shift, clk->en_mask);
		saif_set_reg(clk->vce_clk.en_ctrl, CLK_DISABLE_DATA, clk->en_shift, clk->en_mask);
	}
}

static int vpu_of_clk_get(struct platform_device *pdev, vpu_clk_t *vpu_clk)
{
	if (!pdev)
		return -ENXIO;

	vpu_clk->clkgen = ioremap(SAIF_BD_APBS_BASE, 0x400);
	if (IS_ERR(vpu_clk->clkgen)) {
		dev_err(dev, "ioremap clkgen failed.\n");
		return PTR_ERR(vpu_clk->clkgen);
	}

	/* clkgen define */
	vpu_clk->axi_clk.en_ctrl = vpu_clk->clkgen + WAVE420L_CLK_AXI_CTRL;
	vpu_clk->bpu_clk.en_ctrl = vpu_clk->clkgen + WAVE420L_CLK_BPU_CTRL;
	vpu_clk->vce_clk.en_ctrl = vpu_clk->clkgen + WAVE420L_CLK_VCE_CTRL;
	vpu_clk->apb_clk.en_ctrl = vpu_clk->clkgen + WAVE420L_CLK_APB_CTRL;
	vpu_clk->noc_bus = vpu_clk->clkgen + WAVE420L_CLK_NOCBUS_CTRL;
	vpu_clk->en_mask = CLK_EN_MASK;
	vpu_clk->en_shift = CLK_EN_SHIFT;

	/* rstgen define */
	vpu_clk->rst_ctrl = vpu_clk->clkgen + RSTGEN_SOFTWARE_RESET_ASSERT1;
	vpu_clk->rst_status = vpu_clk->clkgen + RSTGEN_SOFTWARE_RESET_STATUS1;
	vpu_clk->axi_clk.rst_mask = RSTN_AXI_MASK;
	vpu_clk->bpu_clk.rst_mask = RSTN_BPU_MASK;
	vpu_clk->vce_clk.rst_mask = RSTN_VCE_MASK;
	vpu_clk->apb_clk.rst_mask = RSTN_APB_MASK;
	vpu_clk->aximem_128b.rst_mask = RSTN_128B_AXIMEM_MASK;

	if (device_property_read_bool(&pdev->dev, "starfive,venc_noc_ctrl"))
		vpu_clk->noc_ctrl = true;

	return 0;
}

static void vpu_clk_reset(vpu_clk_t *clk)
{
	/*assert-reset*/
	saif_assert_rst(clk->rst_ctrl, clk->rst_status, clk->apb_clk.rst_mask);
	saif_assert_rst(clk->rst_ctrl, clk->rst_status, clk->axi_clk.rst_mask);
	saif_assert_rst(clk->rst_ctrl, clk->rst_status, clk->bpu_clk.rst_mask);
	saif_assert_rst(clk->rst_ctrl, clk->rst_status, clk->vce_clk.rst_mask);

	/*clr-reset*/
	saif_clear_rst(clk->rst_ctrl, clk->rst_status, clk->apb_clk.rst_mask);
	saif_clear_rst(clk->rst_ctrl, clk->rst_status, clk->axi_clk.rst_mask);
	saif_clear_rst(clk->rst_ctrl, clk->rst_status, clk->bpu_clk.rst_mask);
	saif_clear_rst(clk->rst_ctrl, clk->rst_status, clk->vce_clk.rst_mask);

}

int vpu_hw_reset(void)
{
	if (!s_vpu_clk)
		return -1;

	vpu_clk_reset(s_vpu_clk);

	DPRINTK("[VPUDRV] reset vpu hardware. \n");
	return 0;
}

struct vpu_clk_t *vpu_clk_get(struct platform_device *pdev)
{
	vpu_clk_t *vpu_clk;

	vpu_clk = devm_kzalloc(&pdev->dev, sizeof(*vpu_clk), GFP_KERNEL);
	if (!vpu_clk)
		return NULL;

	if (vpu_of_clk_get(dev, vpu_clk))
		goto err_get_clk;

	return vpu_clk;
err_get_clk:
	devm_kfree(&pdev->dev, vpu_clk);
	return NULL;

}

void vpu_clk_put(struct vpu_clk_t *clk)
{
	if (clk->clkgen) {
		iounmap(clk->clkgen);
		clk->clkgen = NULL;
	}
}

static int vpu_clk_enable(struct vpu_clk_t *clk)
{
	if (clk == NULL || IS_ERR(clk))
		return -1;

	vpu_pmu_enable(clk->dev);
	vpu_clk_control(clk, true);
	if (clk->noc_ctrl == true)
		vpu_noc_vdec_bus_control(clk, true);

	DPRINTK("[VPUDRV] vpu_clk_enable\n");
	return 0;
}

void vpu_clk_disable(struct vpu_clk_t *clk)
{
	if (clk == NULL || IS_ERR(clk))
		return;

	vpu_clk_control(clk, false);
	vpu_pmu_disable(clk->dev);
	if (clk->noc_ctrl == true)
		vpu_noc_vdec_bus_control(clk, false);

	DPRINTK("[VPUDRV] vpu_clk_disable\n");
}

#else /*STARFIVE_VPU_SUPPORT_CLOCK_CONTROL*/

int vpu_hw_reset(void)
{
	DPRINTK("[VPUDRV] reset vpu hardware. \n");
	/* sram do not need reset */
	return reset_control_reset(s_vpu_clk->resets);
}

static int vpu_of_clk_get(struct platform_device *pdev, vpu_clk_t *vpu_clk)
{
	struct device *dev = &pdev->dev;
	int ret;

	vpu_clk->dev = dev;
	vpu_clk->clks = vpu_clks;
	vpu_clk->nr_clks = ARRAY_SIZE(vpu_clks);

	vpu_clk->resets = devm_reset_control_array_get_exclusive(dev);
	if (IS_ERR(vpu_clk->resets)) {
		ret = PTR_ERR(vpu_clk->resets);
		dev_err(dev, "faied to get vpu reset controls\n");
	}

	ret = devm_clk_bulk_get(dev, vpu_clk->nr_clks, vpu_clk->clks);
	if (ret)
		dev_err(dev, "faied to get vpu clk controls\n");

	if (device_property_read_bool(dev, "starfive,venc_noc_ctrl"))
		vpu_clk->noc_ctrl = true;

	return 0;
}

static vpu_clk_t *vpu_clk_get(struct platform_device *pdev)
{
	vpu_clk_t *vpu_clk;

	if (!pdev)
		return NULL;

	vpu_clk = devm_kzalloc(&pdev->dev, sizeof(*vpu_clk), GFP_KERNEL);
	if (!vpu_clk)
		return NULL;

	if (vpu_of_clk_get(pdev, vpu_clk))
		goto err_of_clk_get;

	return vpu_clk;

err_of_clk_get:
	devm_kfree(&pdev->dev, vpu_clk);
	return NULL;
}

static void vpu_clk_put(vpu_clk_t *clk)
{
	clk_bulk_put(clk->nr_clks, clk->clks);
}

static int vpu_clk_enable(vpu_clk_t *clk)
{
	int ret;

	vpu_pmu_enable(clk->dev);
	ret = clk_bulk_prepare_enable(clk->nr_clks, clk->clks);
	if (ret)
		dev_err(clk->dev, "enable clk error.\n");

	ret = reset_control_deassert(clk->resets);
	if (ret)
		dev_err(clk->dev, "deassert vpu error.\n");

	DPRINTK("[VPUDRV] vpu_clk_enable\n");
	return ret;
}

static void vpu_clk_disable(vpu_clk_t *clk)
{
	int ret;

	ret = reset_control_assert(clk->resets);
	if (ret)
		dev_err(clk->dev, "assert vpu error.\n");

	clk_bulk_disable_unprepare(clk->nr_clks, clk->clks);
	vpu_pmu_disable(clk->dev);
}

#endif /*STARFIVE_VPU_SUPPORT_CLOCK_CONTROL*/
