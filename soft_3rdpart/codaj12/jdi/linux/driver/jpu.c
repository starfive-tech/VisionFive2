//--=========================================================================--
//  This file is linux device driver for JPU.
//-----------------------------------------------------------------------------
//
//       This confidential and proprietary software may be used only
//     as authorized by a licensing agreement from Chips&Media Inc.
//     In the event of publication, the following notice is applicable:
//
//            (C) COPYRIGHT 2006 - 2016  CHIPS&MEDIA INC.
//                      ALL RIGHTS RESERVED
//
//       The entire notice above must be reproduced on all authorized
//       copies.
//
//--=========================================================================-

#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/mm.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/dma-mapping.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/wait.h>
#include <linux/list.h>
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/uaccess.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/sched/signal.h>
#include <linux/reset.h>
#include <linux/version.h>
#include <soc/sifive/sifive_l2_cache.h>

#include "../../../jpuapi/jpuconfig.h"
#include "jpu.h"

//#define ENABLE_DEBUG_MSG
#ifdef ENABLE_DEBUG_MSG
#define DPRINTK(args...)            printk(KERN_INFO args);
#else
#define DPRINTK(args...)
#endif

/* definitions to be changed as customer  configuration */
/* if you want to have clock gating scheme frame by frame */
//#define JPU_SUPPORT_CLOCK_CONTROL
#define JPU_SUPPORT_ISR
//#define JPU_IRQ_CONTROL

/* jpu pmu */
#define STARFIVE_JPU_PMU

#ifdef STARFIVE_JPU_PMU
#include <soc/starfive/jh7110_pmu.h>
#endif

/* if clktree is work,try this...*/
#define STARFIVE_JPU_SUPPORT_CLOCK_CONTROL

/* if the platform driver knows the name of this driver */
/* JPU_PLATFORM_DEVICE_NAME */
#define JPU_SUPPORT_PLATFORM_DRIVER_REGISTER

/* if this driver knows the dedicated video memory address */

//#define JPU_SUPPORT_RESERVED_VIDEO_MEMORY        //if this driver knows the dedicated video memory address

static void starfive_flush_dcache(unsigned long start, unsigned long len)
{
	sifive_l2_flush64_range(start, len);
}

#define JPU_PLATFORM_DEVICE_NAME    "cnm_jpu"
#define JPU_CLK_NAME                "jpege"
#define JPU_DEV_NAME                "jpu"

#define JPU_REG_BASE_ADDR           0x11900000
#define JPU_REG_SIZE                0x300


#ifdef JPU_SUPPORT_ISR
#define JPU_IRQ_NUM                 24
/* if the driver want to disable and enable IRQ whenever interrupt asserted. */
/*#define JPU_IRQ_CONTROL*/
#endif


#ifndef VM_RESERVED	/*for kernel up to 3.7.0 version*/
#define VM_RESERVED   (VM_DONTEXPAND | VM_DONTDUMP)
#endif

struct device *jpu_dev;

typedef struct jpu_drv_context_t {
    struct fasync_struct *async_queue;
    u32 open_count;                     /*!<< device reference count. Not instance count */
    u32 interrupt_reason[MAX_NUM_INSTANCE];
} jpu_drv_context_t;


/* To track the allocated memory buffer */
typedef struct jpudrv_buffer_pool_t {
    struct list_head        list;
    struct jpudrv_buffer_t  jb;
    struct file*            filp;
} jpudrv_buffer_pool_t;

/* To track the instance index and buffer in instance pool */
typedef struct jpudrv_instance_list_t {
    struct list_head    list;
    unsigned long       inst_idx;
    struct file*        filp;
} jpudrv_instance_list_t;

typedef struct jpudrv_instance_pool_t {
	unsigned char codecInstPool[MAX_NUM_INSTANCE][MAX_INST_HANDLE_SIZE];
} jpudrv_instance_pool_t;

#ifndef STARFIVE_JPU_SUPPORT_CLOCK_CONTROL
typedef struct jpu_clkgen_t {
	void __iomem *en_ctrl;
	uint32_t rst_mask;
} jpu_clkgen_t;
#endif

struct clk_bulk_data jpu_clks[] = {
		{ .id = "axi_clk" },
		{ .id = "core_clk" },
		{ .id = "apb_clk" },
		{ .id = "noc_bus" },
};

typedef struct jpu_clk_t {
#ifndef STARFIVE_JPU_SUPPORT_CLOCK_CONTROL
	void __iomem *clkgen;
	void __iomem *rst_ctrl;
	void __iomem *rst_status;
	uint32_t en_shift;
	uint32_t en_mask;
	jpu_clkgen_t apb_clk;
	jpu_clkgen_t axi_clk;
	jpu_clkgen_t core_clk;
#else
	struct clk_bulk_data *clks;
	struct reset_control *resets;
	int nr_clks;
#endif
	struct device *dev;
} jpu_clk_t;

#ifdef JPU_SUPPORT_RESERVED_VIDEO_MEMORY
#include "jmm.h"
static jpu_mm_t         s_jmem;
static jpudrv_buffer_t  s_video_memory = {0};
#endif /* JPU_SUPPORT_RESERVED_VIDEO_MEMORY */


static int jpu_hw_reset(void);
static void jpu_clk_disable(jpu_clk_t *clk);
static int jpu_clk_enable(jpu_clk_t *clk);
static jpu_clk_t *jpu_clk_get(struct platform_device *pdev);
static void jpu_clk_put(jpu_clk_t *clk);
// end customer definition

static jpudrv_buffer_t s_instance_pool = {0};
static jpu_drv_context_t s_jpu_drv_context;
static int s_jpu_major;
static struct cdev s_jpu_cdev;
static jpu_clk_t *s_jpu_clk;
static int s_jpu_open_ref_count;
#ifdef JPU_SUPPORT_ISR
static int s_jpu_irq = JPU_IRQ_NUM;
#endif

static jpudrv_buffer_t s_jpu_register = {0};

static int s_interrupt_flag[MAX_NUM_INSTANCE];
static wait_queue_head_t s_interrupt_wait_q[MAX_NUM_INSTANCE];


static spinlock_t s_jpu_lock = __SPIN_LOCK_UNLOCKED(s_jpu_lock);
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,36)
static DECLARE_MUTEX(s_jpu_sem);
#else
static DEFINE_SEMAPHORE(s_jpu_sem);
#endif
static struct list_head s_jbp_head = LIST_HEAD_INIT(s_jbp_head);
static struct list_head s_inst_list_head = LIST_HEAD_INIT(s_inst_list_head);

#ifdef CONFIG_PM
/* implement to power management functions */
#endif

#define NPT_BASE                                0x0000
#define NPT_REG_SIZE                            0x300
#define MJPEG_PIC_STATUS_REG(_inst_no)          (NPT_BASE + (_inst_no*NPT_REG_SIZE) + 0x004)

#define ReadJpuRegister(addr)           *(volatile unsigned int *)(s_jpu_register.virt_addr + addr)
#define WriteJpuRegister(addr, val)     *(volatile unsigned int *)(s_jpu_register.virt_addr + addr) = (unsigned int)val
#define WriteJpu(addr, val)             *(volatile unsigned int *)(addr) = (unsigned int)val;


static int jpu_alloc_dma_buffer(jpudrv_buffer_t *jb)
{
    if (!jb)
        return -1;
#ifdef JPU_SUPPORT_RESERVED_VIDEO_MEMORY
    jb->phys_addr = (unsigned long long)jmem_alloc(&s_jmem, jb->size, 0);
    if ((unsigned long)jb->phys_addr  == (unsigned long)-1) {
        printk(KERN_ERR "[JPUDRV] Physical memory allocation error size=%d\n", jb->size);
        return -1;
    }

    jb->base = (unsigned long)(s_video_memory.base + (jb->phys_addr - s_video_memory.phys_addr));
#else
    jb->base = (unsigned long)dma_alloc_coherent(jpu_dev, PAGE_ALIGN(jb->size), (dma_addr_t *) (&jb->phys_addr), GFP_DMA | GFP_KERNEL);
    if ((void *)(jb->base) == NULL) {
        printk(KERN_ERR "[JPUDRV] Physical memory allocation error size=%d\n", jb->size);
        return -1;
    }
	starfive_flush_dcache(jb->phys_addr,PAGE_ALIGN(jb->size));
#endif /* JPU_SUPPORT_RESERVED_VIDEO_MEMORY */
    return 0;
}

static void jpu_free_dma_buffer(jpudrv_buffer_t *jb)
{
    if (!jb) {
        return;
    }

    if (jb->base)
#ifdef JPU_SUPPORT_RESERVED_VIDEO_MEMORY
        jmem_free(&s_jmem, jb->phys_addr, 0);
#else
        dma_free_coherent(jpu_dev, PAGE_ALIGN(jb->size), (void *)jb->base, jb->phys_addr);
#endif /* JPUR_SUPPORT_RESERVED_VIDEO_MEMORY */
}

static int jpu_free_instances(struct file *filp)
{
    jpudrv_instance_list_t *vil, *n;
    jpudrv_instance_pool_t *vip;
    void *vip_base;
    int instance_pool_size_per_core;
    void *jdi_mutexes_base;
    const int PTHREAD_MUTEX_T_DESTROY_VALUE = 0xdead10cc;

    DPRINTK("[JPUDRV] jpu_free_instances\n");

    instance_pool_size_per_core = (s_instance_pool.size/MAX_NUM_JPU_CORE); /* s_instance_pool.size  assigned to the size of all core once call JDI_IOCTL_GET_INSTANCE_POOL by user. */

    list_for_each_entry_safe(vil, n, &s_inst_list_head, list)
    {
        if (vil->filp == filp) {
            vip_base = (void *)(s_instance_pool.base + instance_pool_size_per_core);
            DPRINTK("[JPUDRV] jpu_free_instances detect instance crash instIdx=%d, vip_base=%p, instance_pool_size_per_core=%d\n", (int)vil->inst_idx, vip_base, (int)instance_pool_size_per_core);
            vip = (jpudrv_instance_pool_t *)vip_base;
            if (vip) {
                memset(&vip->codecInstPool[vil->inst_idx], 0x00, 4);    /* only first 4 byte is key point(inUse of CodecInst in jpuapi) to free the corresponding instance. */
#define PTHREAD_MUTEX_T_HANDLE_SIZE 4
                jdi_mutexes_base = (vip_base + (instance_pool_size_per_core - PTHREAD_MUTEX_T_HANDLE_SIZE*4));
                DPRINTK("[JPUDRV] jpu_free_instances : force to destroy jdi_mutexes_base=%p in userspace \n", jdi_mutexes_base);
                if (jdi_mutexes_base) {
                    int i;
                    for (i = 0; i < 4; i++) {
                        memcpy(jdi_mutexes_base, &PTHREAD_MUTEX_T_DESTROY_VALUE, PTHREAD_MUTEX_T_HANDLE_SIZE);
                        jdi_mutexes_base += PTHREAD_MUTEX_T_HANDLE_SIZE;
                    }
                }
            }
            s_jpu_open_ref_count--;
            list_del(&vil->list);
            kfree(vil);
        }
    }
    return 1;
}

static int jpu_free_buffers(struct file *filp)
{
    jpudrv_buffer_pool_t *pool, *n;
    jpudrv_buffer_t jb;

    DPRINTK("[JPUDRV] jpu_free_buffers\n");

    list_for_each_entry_safe(pool, n, &s_jbp_head, list)
    {
        if (pool->filp == filp) {
            jb = pool->jb;
            if (jb.base) {
                jpu_free_dma_buffer(&jb);
                list_del(&pool->list);
                kfree(pool);
            }
        }
    }

    return 0;
}


static irqreturn_t jpu_irq_handler(int irq, void *dev_id)
{
    jpu_drv_context_t*  dev = (jpu_drv_context_t *)dev_id;
    int i;
    u32 flag;

    DPRINTK("[JPUDRV][+]%s\n", __func__);

#ifdef JPU_IRQ_CONTROL
    disable_irq_nosync(s_jpu_irq);
#endif

    for (i=0; i<MAX_NUM_INSTANCE; i++) {
        flag = ReadJpuRegister(MJPEG_PIC_STATUS_REG(i));
        if (flag != 0) {
            break;
        }
    }

    dev->interrupt_reason[i] = flag;
    s_interrupt_flag[i] = 1;
    DPRINTK("[JPUDRV][%d] INTERRUPT FLAG: %08x, %08x\n", i, dev->interrupt_reason[i], MJPEG_PIC_STATUS_REG(i));

    if (dev->async_queue)
        kill_fasync(&dev->async_queue, SIGIO, POLL_IN);    // notify the interrupt to userspace

#ifndef JPU_IRQ_CONTROL
    WriteJpuRegister(MJPEG_PIC_STATUS_REG(i),flag);  	//clear interrut
#endif

    wake_up_interruptible(&s_interrupt_wait_q[i]);

    DPRINTK("[JPUDRV][-]%s\n", __func__);

    return IRQ_HANDLED;
}

static int jpu_open(struct inode *inode, struct file *filp)
{
    DPRINTK("[JPUDRV][+] %s\n", __func__);

    spin_lock(&s_jpu_lock);

    s_jpu_drv_context.open_count++;

    filp->private_data = (void *)(&s_jpu_drv_context);
    spin_unlock(&s_jpu_lock);

    DPRINTK("[JPUDRV][-] %s\n", __func__);

    return 0;
}

static long jpu_ioctl(struct file *filp, u_int cmd, u_long arg)
{
    int ret = 0;

    switch (cmd)
    {
    case JDI_IOCTL_ALLOCATE_PHYSICAL_MEMORY:
        {
            jpudrv_buffer_pool_t *jbp;

            DPRINTK("[JPUDRV][+]JDI_IOCTL_ALLOCATE_PHYSICAL_MEMORY\n");
            if ((ret = down_interruptible(&s_jpu_sem)) == 0) {
                jbp = kzalloc(sizeof(jpudrv_buffer_pool_t), GFP_KERNEL);
                if (!jbp) {
                    up(&s_jpu_sem);
                    return -ENOMEM;
                }

                ret = copy_from_user(&(jbp->jb), (jpudrv_buffer_t *)arg, sizeof(jpudrv_buffer_t));
                if (ret)
                {
                    kfree(jbp);
                    up(&s_jpu_sem);
                    return -EFAULT;
                }

                ret = jpu_alloc_dma_buffer(&(jbp->jb));
                if (ret == -1)
                {
                    ret = -ENOMEM;
                    kfree(jbp);
                    up(&s_jpu_sem);
                    break;
                }
                ret = copy_to_user((void __user *)arg, &(jbp->jb), sizeof(jpudrv_buffer_t));
                if (ret)
                {
                    kfree(jbp);
                    ret = -EFAULT;
                    up(&s_jpu_sem);
                    break;
                }

                jbp->filp = filp;
                spin_lock(&s_jpu_lock);
                list_add(&jbp->list, &s_jbp_head);
                spin_unlock(&s_jpu_lock);

                up(&s_jpu_sem);
            }
            DPRINTK("[JPUDRV][-]JDI_IOCTL_ALLOCATE_PHYSICAL_MEMORY\n");
        }
        break;
    case JDI_IOCTL_FREE_PHYSICALMEMORY:
        {
            jpudrv_buffer_pool_t *jbp, *n;
            jpudrv_buffer_t jb;

            DPRINTK("[JPUDRV][+]VDI_IOCTL_FREE_PHYSICALMEMORY\n");
            if ((ret = down_interruptible(&s_jpu_sem)) == 0) {
                ret = copy_from_user(&jb, (jpudrv_buffer_t *)arg, sizeof(jpudrv_buffer_t));
                if (ret) {
                    up(&s_jpu_sem);
                    return -EACCES;
                }

                if (jb.base)
                    jpu_free_dma_buffer(&jb);

                spin_lock(&s_jpu_lock);
                list_for_each_entry_safe(jbp, n, &s_jbp_head, list) {
                    if (jbp->jb.base == jb.base) {
                        list_del(&jbp->list);
                        kfree(jbp);
                        break;
                    }
                }
                spin_unlock(&s_jpu_lock);

                up(&s_jpu_sem);
            }
            DPRINTK("[JPUDRV][-]VDI_IOCTL_FREE_PHYSICALMEMORY\n");
        }
        break;
    case JDI_IOCTL_GET_RESERVED_VIDEO_MEMORY_INFO:
        {
#ifdef JPU_SUPPORT_RESERVED_VIDEO_MEMORY
            if (s_video_memory.base != 0) {
                ret = copy_to_user((void __user *)arg, &s_video_memory, sizeof(jpudrv_buffer_t));
                if (ret != 0)
                    ret = -EFAULT;
            } else {
                ret = -EFAULT;
            }
#endif /* JPU_SUPPORT_RESERVED_VIDEO_MEMORY */
        }
        break;


    case JDI_IOCTL_WAIT_INTERRUPT:
        {
            jpudrv_intr_info_t info;
            struct jpu_drv_context_t *dev = (struct jpu_drv_context_t *)filp->private_data;
            u32 instance_no;

            DPRINTK("[JPUDRV][+]JDI_IOCTL_WAIT_INTERRUPT\n");
            ret = copy_from_user(&info, (jpudrv_intr_info_t *)arg, sizeof(jpudrv_intr_info_t));
            if (ret != 0)
                return -EFAULT;

            instance_no = info.inst_idx;
            DPRINTK("[JPUDRV] INSTANCE NO: %d\n", instance_no);
            ret = wait_event_interruptible_timeout(s_interrupt_wait_q[instance_no], s_interrupt_flag[instance_no] != 0, msecs_to_jiffies(info.timeout));
            if (!ret) {
                DPRINTK("[JPUDRV] INSTANCE NO: %d ETIME\n", instance_no);
                ret = -ETIME;
                break;
            }

            if (signal_pending(current)) {
                ret = -ERESTARTSYS;
                DPRINTK("[JPUDRV] INSTANCE NO: %d ERESTARTSYS\n", instance_no);
                break;
            }

            DPRINTK("[JPUDRV] INST(%d) s_interrupt_flag(%d), reason(0x%08x)\n", instance_no, s_interrupt_flag[instance_no], dev->interrupt_reason[instance_no]);

            info.intr_reason = dev->interrupt_reason[instance_no];
            s_interrupt_flag[instance_no] = 0;
            dev->interrupt_reason[instance_no] = 0;
            ret = copy_to_user((void __user *)arg, &info, sizeof(jpudrv_intr_info_t));
#ifdef JPU_IRQ_CONTROL
            enable_irq(s_jpu_irq);
#endif
            DPRINTK("[VPUDRV][-]VDI_IOCTL_WAIT_INTERRUPT\n");
            if (ret != 0)
                return -EFAULT;
        }
        break;
    case JDI_IOCTL_SET_CLOCK_GATE:
        {
            u32 clkgate;

            if (get_user(clkgate, (u32 __user *) arg))
                return -EFAULT;

#ifdef JPU_SUPPORT_CLOCK_CONTROL
            if (clkgate)
                jpu_clk_enable(s_jpu_clk);
            else
                jpu_clk_disable(s_jpu_clk);
#endif /* JPU_SUPPORT_CLOCK_CONTROL */
        }
        break;
    case JDI_IOCTL_GET_INSTANCE_POOL:
        DPRINTK("[JPUDRV][+]JDI_IOCTL_GET_INSTANCE_POOL\n");

        if ((ret = down_interruptible(&s_jpu_sem)) == 0) {
            if (s_instance_pool.base != 0) {
                ret = copy_to_user((void __user *)arg, &s_instance_pool, sizeof(jpudrv_buffer_t));
            } else {
                ret = copy_from_user(&s_instance_pool, (jpudrv_buffer_t *)arg, sizeof(jpudrv_buffer_t));
                if (ret == 0) {
                    s_instance_pool.size      = PAGE_ALIGN(s_instance_pool.size);
                    s_instance_pool.base      = (unsigned long)vmalloc(s_instance_pool.size);
                    s_instance_pool.phys_addr = s_instance_pool.base;

                    if (s_instance_pool.base != 0) {
                        memset((void *)s_instance_pool.base, 0x0, s_instance_pool.size); /*clearing memory*/
                        ret = copy_to_user((void __user *)arg, &s_instance_pool, sizeof(jpudrv_buffer_t));
                        if (ret == 0) {
                            /* success to get memory for instance pool */
                            up(&s_jpu_sem);
                            break;
                        }
                    }
                    ret = -EFAULT;
                }
            }
            up(&s_jpu_sem);
        }

        DPRINTK("[JPUDRV][-]JDI_IOCTL_GET_INSTANCE_POOL: %s base: %lx, size: %d\n",
                (ret==0 ? "OK" : "NG"), s_instance_pool.base, s_instance_pool.size);
        break;
    case JDI_IOCTL_OPEN_INSTANCE:
        {
            jpudrv_inst_info_t inst_info;

            if (copy_from_user(&inst_info, (jpudrv_inst_info_t *)arg, sizeof(jpudrv_inst_info_t)))
                return -EFAULT;

            spin_lock(&s_jpu_lock);
            s_jpu_open_ref_count++; /* flag just for that jpu is in opened or closed */
            inst_info.inst_open_count = s_jpu_open_ref_count;
            spin_unlock(&s_jpu_lock);

            if (copy_to_user((void __user *)arg, &inst_info, sizeof(jpudrv_inst_info_t))) {
                return -EFAULT;
            }

            DPRINTK("[JPUDRV] JDI_IOCTL_OPEN_INSTANCE inst_idx=%d, s_jpu_open_ref_count=%d, inst_open_count=%d\n",
                    (int)inst_info.inst_idx, s_jpu_open_ref_count, inst_info.inst_open_count);
        }
        break;
    case JDI_IOCTL_CLOSE_INSTANCE:
        {
            jpudrv_inst_info_t inst_info;

            DPRINTK("[JPUDRV][+]JDI_IOCTL_CLOSE_INSTANCE\n");
            if (copy_from_user(&inst_info, (jpudrv_inst_info_t *)arg, sizeof(jpudrv_inst_info_t)))
                return -EFAULT;

            spin_lock(&s_jpu_lock);
            s_jpu_open_ref_count--; /* flag just for that jpu is in opened or closed */
            inst_info.inst_open_count = s_jpu_open_ref_count;
            spin_unlock(&s_jpu_lock);

            if (copy_to_user((void __user *)arg, &inst_info, sizeof(jpudrv_inst_info_t)))
                return -EFAULT;

            DPRINTK("[JPUDRV] JDI_IOCTL_CLOSE_INSTANCE inst_idx=%d, s_jpu_open_ref_count=%d, inst_open_count=%d\n",
                    (int)inst_info.inst_idx, s_jpu_open_ref_count, inst_info.inst_open_count);
        }
        break;
    case JDI_IOCTL_GET_INSTANCE_NUM:
        {
            jpudrv_inst_info_t inst_info;

            DPRINTK("[JPUDRV][+]JDI_IOCTL_GET_INSTANCE_NUM\n");

            ret = copy_from_user(&inst_info, (jpudrv_inst_info_t *)arg, sizeof(jpudrv_inst_info_t));
            if (ret != 0)
                break;

            spin_lock(&s_jpu_lock);
            inst_info.inst_open_count = s_jpu_open_ref_count;
            spin_unlock(&s_jpu_lock);

            ret = copy_to_user((void __user *)arg, &inst_info, sizeof(jpudrv_inst_info_t));

            DPRINTK("[JPUDRV] JDI_IOCTL_GET_INSTANCE_NUM inst_idx=%d, open_count=%d\n", (int)inst_info.inst_idx, inst_info.inst_open_count);
        }
        break;
    case JDI_IOCTL_RESET:
        jpu_hw_reset();
        break;

    case JDI_IOCTL_GET_REGISTER_INFO:
        DPRINTK("[JPUDRV][+]JDI_IOCTL_GET_REGISTER_INFO\n");
        ret = copy_to_user((void __user *)arg, &s_jpu_register, sizeof(jpudrv_buffer_t));
        if (ret != 0)
            ret = -EFAULT;
        DPRINTK("[JPUDRV][-]JDI_IOCTL_GET_REGISTER_INFO s_jpu_register.phys_addr=0x%lx, s_jpu_register.virt_addr=0x%lx, s_jpu_register.size=%d\n",
                s_jpu_register.phys_addr , s_jpu_register.virt_addr, s_jpu_register.size);
        break;
    case JDI_IOCTL_FLUSH_DCACHE:
        {
            jpudrv_flush_cache_t cache_info;

            //DPRINTK("[JPUDRV][+]JDI_IOCTL_FLUSH_DCACHE\n");
            ret = copy_from_user(&cache_info, (jpudrv_flush_cache_t *)arg, sizeof(jpudrv_flush_cache_t));
            if (ret != 0)
                ret = -EFAULT;

            if(cache_info.flag)
                starfive_flush_dcache(cache_info.start,cache_info.size);

            //DPRINTK("[JPUDRV][-]JDI_IOCTL_FLUSH_DCACHE\n");
            break;
        }
    default:
        {
            printk(KERN_ERR "No such IOCTL, cmd is %d\n", cmd);
        }
        break;
    }
    return ret;
}

static ssize_t jpu_read(struct file *filp, char __user *buf, size_t len, loff_t *ppos)
{

    return -1;
}

static ssize_t jpu_write(struct file *filp, const char __user *buf, size_t len, loff_t *ppos)
{

    /* DPRINTK("[VPUDRV] vpu_write len=%d\n", (int)len); */
    if (!buf) {
        printk(KERN_ERR "[VPUDRV] vpu_write buf = NULL error \n");
        return -EFAULT;
    }

    return -1;
}

static int jpu_release(struct inode *inode, struct file *filp)
{
    int ret = 0;
    u32 open_count;

    DPRINTK("[JPUDRV][+] jpu_release\n");

    if ((ret = down_interruptible(&s_jpu_sem)) == 0) {

        /* found and free the not handled buffer by user applications */
        jpu_free_buffers(filp);

        /* found and free the not closed instance by user applications */
        jpu_free_instances(filp);
        DPRINTK("[JPUDRV] open_count: %d\n", s_jpu_drv_context.open_count);
        spin_lock(&s_jpu_lock);
        s_jpu_drv_context.open_count--;
        open_count = s_jpu_drv_context.open_count;
        spin_unlock(&s_jpu_lock);
        if (open_count == 0) {
            if (s_instance_pool.base) {
                DPRINTK("[JPUDRV] free instance pool\n");
                vfree((const void *)s_instance_pool.base);
                s_instance_pool.base = 0;
            }


        }
    }
    up(&s_jpu_sem);

    DPRINTK("[JPUDRV][-] jpu_release\n");

    jpu_hw_reset();

    return 0;
}


static int jpu_fasync(int fd, struct file *filp, int mode)
{
    struct jpu_drv_context_t *dev = (struct jpu_drv_context_t *)filp->private_data;
    return fasync_helper(fd, filp, mode, &dev->async_queue);
}




static int jpu_map_to_register(struct file *fp, struct vm_area_struct *vm)
{
    unsigned long pfn;

    vm->vm_flags |= VM_IO | VM_RESERVED;
    vm->vm_page_prot = pgprot_noncached(vm->vm_page_prot);
    pfn = s_jpu_register.phys_addr >> PAGE_SHIFT;

    return remap_pfn_range(vm, vm->vm_start, pfn, vm->vm_end-vm->vm_start, vm->vm_page_prot) ? -EAGAIN : 0;
}

static int jpu_map_to_physical_memory(struct file *fp, struct vm_area_struct *vm)
{
    vm->vm_flags |= VM_IO | VM_RESERVED;
    vm->vm_page_prot = pgprot_noncached(vm->vm_page_prot);

    return remap_pfn_range(vm, vm->vm_start, vm->vm_pgoff, vm->vm_end-vm->vm_start, vm->vm_page_prot) ? -EAGAIN : 0;
}

static int jpu_map_to_instance_pool_memory(struct file *fp, struct vm_area_struct *vm)
{
    int ret;
    long length = vm->vm_end - vm->vm_start;
    unsigned long start = vm->vm_start;
    char *vmalloc_area_ptr = (char *)s_instance_pool.base;
    unsigned long pfn;

    vm->vm_flags |= VM_RESERVED;

    /* loop over all pages, map it page individually */
    while (length > 0) {
        pfn = vmalloc_to_pfn(vmalloc_area_ptr);
        if ((ret = remap_pfn_range(vm, start, pfn, PAGE_SIZE, PAGE_SHARED)) < 0) {
            return ret;
        }
        start += PAGE_SIZE;
        vmalloc_area_ptr += PAGE_SIZE;
        length -= PAGE_SIZE;
    }


    return 0;
}

/*!
* @brief memory map interface for jpu file operation
* @return  0 on success or negative error code on error
*/
static int jpu_mmap(struct file *fp, struct vm_area_struct *vm)
{
    if (vm->vm_pgoff == 0)
        return jpu_map_to_instance_pool_memory(fp, vm);

    if (vm->vm_pgoff == (s_jpu_register.phys_addr>>PAGE_SHIFT))
        return jpu_map_to_register(fp, vm);

    return jpu_map_to_physical_memory(fp, vm);
}

struct file_operations jpu_fops = {
    .owner = THIS_MODULE,
    .open = jpu_open,
    .read = jpu_read,
    .write = jpu_write,
    .unlocked_ioctl = jpu_ioctl,
    .release = jpu_release,
    .fasync = jpu_fasync,
    .mmap = jpu_mmap,
};



static int jpu_probe(struct platform_device *pdev)
{
    int err = 0;
    struct resource *res = NULL;
#ifdef JPU_SUPPORT_RESERVED_VIDEO_MEMORY
	struct resource res_cma;
	struct device_node *node;
#endif

    DPRINTK("[JPUDRV] jpu_probe\n");
    if (pdev) {
        res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    }

    if (res) {/* if platform driver is implemented */
        s_jpu_register.phys_addr = res->start;
        s_jpu_register.virt_addr = (unsigned long)ioremap(res->start, res->end - res->start);
        s_jpu_register.size      = res->end - res->start;
        DPRINTK("[JPUDRV] : jpu base address get from platform driver physical base addr==0x%lx, virtual base=0x%lx\n", s_jpu_register.phys_addr , s_jpu_register.virt_addr);
    } else {
        s_jpu_register.phys_addr = JPU_REG_BASE_ADDR;
        s_jpu_register.virt_addr = (unsigned long)ioremap(s_jpu_register.phys_addr, JPU_REG_SIZE);
        s_jpu_register.size      = JPU_REG_SIZE;
        DPRINTK("[JPUDRV] : jpu base address get from defined value physical base addr==0x%lx, virtual base=0x%lx\n", s_jpu_register.phys_addr, s_jpu_register.virt_addr);
    }
    if (pdev) {
        jpu_dev = &pdev->dev;
		//jpu_dev->dma_ops = NULL;
        dev_info(jpu_dev,"init device.\n");
    }
    /* get the major number of the character device */
    if ((alloc_chrdev_region(&s_jpu_major, 0, 1, JPU_DEV_NAME)) < 0) {
        err = -EBUSY;
        printk(KERN_ERR "could not allocate major number\n");
        goto ERROR_PROVE_DEVICE;
    }

    /* initialize the device structure and register the device with the kernel */
    cdev_init(&s_jpu_cdev, &jpu_fops);
    if ((cdev_add(&s_jpu_cdev, s_jpu_major, 1)) < 0) {
        err = -EBUSY;
        printk(KERN_ERR "could not allocate chrdev\n");

        goto ERROR_PROVE_DEVICE;
    }

    if (pdev)
        s_jpu_clk = jpu_clk_get(pdev);
    else
        s_jpu_clk = jpu_clk_get(NULL);

    if (!s_jpu_clk) {
        printk(KERN_ERR "[JPUDRV] : not support clock controller.\n");
    }
    else {
        DPRINTK("[JPUDRV] : get clock controller s_jpu_clk=%p\n", s_jpu_clk);
    }

#ifdef JPU_SUPPORT_CLOCK_CONTROL
#else
    jpu_clk_enable(s_jpu_clk);
#endif


#ifdef JPU_SUPPORT_ISR
#ifdef JPU_SUPPORT_PLATFORM_DRIVER_REGISTER
    if(pdev)
        res = platform_get_resource(pdev, IORESOURCE_IRQ, 0);
    if (res) {/* if platform driver is implemented */
        s_jpu_irq = res->start;
        DPRINTK("[JPUDRV] : jpu irq number get from platform driver irq=0x%x\n", s_jpu_irq );
    } else {
        DPRINTK("[JPUDRV] : jpu irq number get from defined value irq=0x%x\n", s_jpu_irq );
    }
#else
    DPRINTK("[JPUDRV] : jpu irq number get from defined value irq=0x%x\n", s_jpu_irq);
#endif


    err = request_irq(s_jpu_irq, jpu_irq_handler, 0, "JPU_CODEC_IRQ", (void *)(&s_jpu_drv_context));
    if (err) {
        printk(KERN_ERR "[JPUDRV] :  fail to register interrupt handler\n");
        goto ERROR_PROVE_DEVICE;
    }
#endif

#ifdef JPU_SUPPORT_RESERVED_VIDEO_MEMORY
	node = of_parse_phandle(jpu_dev->of_node, "memory-region", 0);
	if(node){
		dev_info(jpu_dev, "Get mem form memory-region\n");
		of_address_to_resource(node, 0, &res_cma);
		s_video_memory.size = resource_size(&res_cma);
    	s_video_memory.phys_addr = res_cma.start;
	}else{
        dev_info(jpu_dev, "Get mem form reserved memory failed.please check the dts file.\n");
		return 0;
    }
    s_video_memory.base = (unsigned long)ioremap(MEM2SYS(s_video_memory.phys_addr), PAGE_ALIGN(s_video_memory.size));
	if (!s_video_memory.base) {
        printk(KERN_ERR "[JPUDRV] :  fail to remap video memory physical phys_addr=0x%lx, base=0x%lx, size=%d\n", MEM2SYS(s_video_memory.phys_addr), s_video_memory.base, s_video_memory.size);
        goto ERROR_PROVE_DEVICE;
    }

    if (jmem_init(&s_jmem, s_video_memory.phys_addr, s_video_memory.size) < 0) {
        printk(KERN_ERR "[JPUDRV] :  fail to init vmem system\n");
        goto ERROR_PROVE_DEVICE;
    }
    DPRINTK("[JPUDRV] success to probe jpu device with reserved video memory phys_addr=0x%lx, base=0x%lx\n", s_video_memory.phys_addr, s_video_memory.base);
#else
    DPRINTK("[JPUDRV] success to probe jpu device with non reserved video memory\n");
#endif

    return 0;


ERROR_PROVE_DEVICE:

    if (s_jpu_major)
        unregister_chrdev_region(s_jpu_major, 1);

    if (s_jpu_register.virt_addr)
        iounmap((void *)s_jpu_register.virt_addr);

    return err;
}

static int jpu_remove(struct platform_device *pdev)
{
    DPRINTK("[JPUDRV] jpu_remove\n");
#ifdef JPU_SUPPORT_PLATFORM_DRIVER_REGISTER

    if (s_instance_pool.base) {
        vfree((const void *)s_instance_pool.base);
        s_instance_pool.base = 0;
    }
#ifdef JPU_SUPPORT_RESERVED_VIDEO_MEMORY
    if (s_video_memory.base) {
        iounmap((void *)s_video_memory.base);
        s_video_memory.base = 0;
        jmem_exit(&s_jmem);
    }
#endif

    if (s_jpu_major > 0) {
        cdev_del(&s_jpu_cdev);
        unregister_chrdev_region(s_jpu_major, 1);
        s_jpu_major = 0;
    }

#ifdef JPU_SUPPORT_ISR
    if (s_jpu_irq)
        free_irq(s_jpu_irq, &s_jpu_drv_context);
#endif

    if (s_jpu_register.virt_addr)
        iounmap((void*)s_jpu_register.virt_addr);

    jpu_clk_disable(s_jpu_clk);
    jpu_clk_put(s_jpu_clk);

#endif /* JPU_SUPPORT_PLATFORM_DRIVER_REGISTER */

	return 0;
}

//#ifdef CONFIG_PM
#if 1
static int jpu_suspend(struct platform_device *pdev, pm_message_t state)
{
    jpu_clk_disable(s_jpu_clk);
    return 0;

}
static int jpu_resume(struct platform_device *pdev)
{
    jpu_clk_enable(s_jpu_clk);

    return 0;
}
#else
#define    jpu_suspend    NULL
#define    jpu_resume    NULL
#endif /* !CONFIG_PM */

#ifdef JPU_SUPPORT_PLATFORM_DRIVER_REGISTER
static const struct of_device_id jpu_of_id_table[] = {
	{ .compatible = "cm,codaj12-jpu-1" },
	{ .compatible = "starfive,jpu" },
	{}
};

static struct platform_driver jpu_driver = {
    .driver = {
        .name = JPU_PLATFORM_DEVICE_NAME,
        .of_match_table = of_match_ptr(jpu_of_id_table),
    },
    .probe      = jpu_probe,
    .remove     = jpu_remove,
    .suspend    = jpu_suspend,
    .resume     = jpu_resume,
};
#endif /* JPU_SUPPORT_PLATFORM_DRIVER_REGISTER */

static int __init jpu_init(void)
{
    int res = 0;
    u32 i;

    DPRINTK("[JPUDRV] begin jpu_init\n");
    for (i=0; i<MAX_NUM_INSTANCE; i++) {
        init_waitqueue_head(&s_interrupt_wait_q[i]);
    }
    s_instance_pool.base = 0;
#ifdef JPU_SUPPORT_PLATFORM_DRIVER_REGISTER
    res = platform_driver_register(&jpu_driver);
#else
    res = jpu_probe(NULL);
#endif /* JPU_SUPPORT_PLATFORM_DRIVER_REGISTER */

    DPRINTK("[JPUDRV] end jpu_init result=0x%x\n", res);
    return res;
}

static void __exit jpu_exit(void)
{
    DPRINTK("[JPUDRV] [+]jpu_exit\n");
#ifdef JPU_SUPPORT_PLATFORM_DRIVER_REGISTER
    platform_driver_unregister(&jpu_driver);
#else /* JPU_SUPPORT_PLATFORM_DRIVER_REGISTER */
#ifdef JPU_SUPPORT_CLOCK_CONTROL
#else
    jpu_clk_disable(s_jpu_clk);
#endif /* JPU_SUPPORT_CLOCK_CONTROL */
    jpu_clk_put(s_jpu_clk);
    if (s_instance_pool.base) {
        vfree((const void *)s_instance_pool.base);
        s_instance_pool.base = 0;
    }
#ifdef JPU_SUPPORT_RESERVED_VIDEO_MEMORY
    if (s_video_memory.base) {
        iounmap((void *)s_video_memory.base);
        s_video_memory.base = 0;

        jmem_exit(&s_jmem);
    }
#endif /* JPU_SUPPORT_RESERVED_VIDEO_MEMORY */

    if (s_jpu_major > 0) {
        cdev_del(&s_jpu_cdev);
        unregister_chrdev_region(s_jpu_major, 1);
        s_jpu_major = 0;
    }

#ifdef JPU_SUPPORT_ISR
    if (s_jpu_irq)
        free_irq(s_jpu_irq, &s_jpu_drv_context);
#endif /* JPU_SUPPORT_ISR */

    if (s_jpu_register.virt_addr) {
        iounmap((void *)s_jpu_register.virt_addr);
        s_jpu_register.virt_addr = 0x00;
    }

#endif /* JPU_SUPPORT_PLATFORM_DRIVER_REGISTER */
    DPRINTK("[JPUDRV] [-]jpu_exit\n");

    return;
}

MODULE_AUTHOR("A customer using C&M JPU, Inc.");
MODULE_DESCRIPTION("JPU linux driver");
MODULE_LICENSE("GPL");

module_init(jpu_init);
module_exit(jpu_exit);

#ifdef STARFIVE_JPU_PMU
static void jpu_pmu_enable(bool enable)
{
	starfive_power_domain_set(POWER_DOMAIN_JPU, enable);
}
#else
static void jpu_pmu_enable(bool enable)
{
	return;
}
#endif

#ifndef STARFIVE_JPU_SUPPORT_CLOCK_CONTROL
#define CLK_ENABLE_DATA		1
#define CLK_DISABLE_DATA	0
#define CLK_EN_SHIFT		31
#define CLK_EN_MASK		0x80000000U

#define SAIF_BD_APBS_BASE	0x13020000
#define CODAJ12_CLK_AXI_CTRL	0x108U
#define CODAJ12_CLK_APB_CTRL	0x110U
#define CODAJ12_CLK_CORE_CTRL	0x10cU

#define RSTGEN_SOFTWARE_RESET_ASSERT1	0x2FCU
#define RSTGEN_SOFTWARE_RESET_STATUS1	0x30CU

#define RSTN_AXI_MASK			(0x1 << 12)
#define RSTN_CORE_MASK			(0x1 << 13)
#define RSTN_APB_MASK			(0x1 << 14)

static __maybe_unused uint32_t saif_get_reg(
			const volatile void __iomem *addr,
			uint32_t shift, uint32_t mask)
{
	u32 tmp;
	tmp = readl(addr);
	tmp = (tmp & mask) >> shift;
	return tmp;
}

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

static void jpu_clk_control(jpu_clk_t *clk, bool enable)
{
	if (enable) {
		/*enable*/
		saif_set_reg(clk->apb_clk.en_ctrl, CLK_ENABLE_DATA, clk->en_shift, clk->en_mask);
		saif_set_reg(clk->axi_clk.en_ctrl, CLK_ENABLE_DATA, clk->en_shift, clk->en_mask);
		saif_set_reg(clk->core_clk.en_ctrl, CLK_ENABLE_DATA, clk->en_shift, clk->en_mask);

		/*clr-reset*/
		saif_clear_rst(clk->rst_ctrl, clk->rst_status, clk->apb_clk.rst_mask);
		saif_clear_rst(clk->rst_ctrl, clk->rst_status, clk->axi_clk.rst_mask);
		saif_clear_rst(clk->rst_ctrl, clk->rst_status, clk->core_clk.rst_mask);
	} else {
		/*assert-reset*/
		saif_assert_rst(clk->rst_ctrl, clk->rst_status, clk->apb_clk.rst_mask);
		saif_assert_rst(clk->rst_ctrl, clk->rst_status, clk->axi_clk.rst_mask);
		saif_assert_rst(clk->rst_ctrl, clk->rst_status, clk->core_clk.rst_mask);

		/*disable*/
		saif_set_reg(clk->apb_clk.en_ctrl, CLK_DISABLE_DATA, clk->en_shift, clk->en_mask);
		saif_set_reg(clk->axi_clk.en_ctrl, CLK_DISABLE_DATA, clk->en_shift, clk->en_mask);
		saif_set_reg(clk->core_clk.en_ctrl, CLK_DISABLE_DATA, clk->en_shift, clk->en_mask);
	}
}

static void jpu_clk_reset(jpu_clk_t *clk)
{
	/*assert-reset*/
	saif_assert_rst(clk->rst_ctrl, clk->rst_status, clk->apb_clk.rst_mask);
	saif_assert_rst(clk->rst_ctrl, clk->rst_status, clk->axi_clk.rst_mask);
	saif_assert_rst(clk->rst_ctrl, clk->rst_status, clk->core_clk.rst_mask);

	/*clr-reset*/
	saif_clear_rst(clk->rst_ctrl, clk->rst_status, clk->apb_clk.rst_mask);
	saif_clear_rst(clk->rst_ctrl, clk->rst_status, clk->axi_clk.rst_mask);
	saif_clear_rst(clk->rst_ctrl, clk->rst_status, clk->core_clk.rst_mask);
}

int jpu_hw_reset(void)
{
	if (!s_jpu_clk)
		return -1;

	jpu_clk_reset(s_jpu_clk);

	DPRINTK("[VPUDRV] reset vpu hardware. \n");
	return 0;
}

static int jpu_of_clk_get(struct platform_device *pdev, jpu_clk_t *jpu_clk)
{
	if (!pdev)
		return -ENXIO;

	jpu_clk->clkgen = ioremap(SAIF_BD_APBS_BASE, 0x400);
	if (IS_ERR(jpu_clk->clkgen)) {
		dev_err(&pdev->dev, "ioremap clkgen failed.\n");
		return PTR_ERR(jpu_clk->clkgen);
	}

	/* clkgen define */
	jpu_clk->axi_clk.en_ctrl = jpu_clk->clkgen + CODAJ12_CLK_AXI_CTRL;
	jpu_clk->apb_clk.en_ctrl = jpu_clk->clkgen + CODAJ12_CLK_APB_CTRL;
	jpu_clk->core_clk.en_ctrl = jpu_clk->clkgen + CODAJ12_CLK_CORE_CTRL;
	jpu_clk->en_mask = CLK_EN_MASK;
	jpu_clk->en_shift = CLK_EN_SHIFT;

	/* rstgen define */
	jpu_clk->rst_ctrl = jpu_clk->clkgen + RSTGEN_SOFTWARE_RESET_ASSERT1;
	jpu_clk->rst_status = jpu_clk->clkgen + RSTGEN_SOFTWARE_RESET_STATUS1;
	jpu_clk->axi_clk.rst_mask = RSTN_AXI_MASK;
	jpu_clk->apb_clk.rst_mask = RSTN_APB_MASK;
	jpu_clk->core_clk.rst_mask = RSTN_CORE_MASK;

	return 0;
}

static jpu_clk_t *jpu_clk_get(struct platform_device *pdev)
{
	jpu_clk_t *jpu_clk;

	jpu_clk = devm_kzalloc(&pdev->dev, sizeof(*jpu_clk), GFP_KERNEL);
	if (!jpu_clk)
		return NULL;

	if (jpu_of_clk_get(pdev, jpu_clk))
		goto err_get_clk;

	return jpu_clk;
err_get_clk:
	devm_kfree(&pdev->dev, jpu_clk);
	return NULL;
}

static void jpu_clk_put(jpu_clk_t *clk)
{
	if (clk->clkgen) {
		iounmap(clk->clkgen);
		clk->clkgen = NULL;
	}
}

static int jpu_clk_enable(jpu_clk_t *clk)
{
	if (clk == NULL || IS_ERR(clk))
		return -1;

	jpu_pmu_enable(true);
	jpu_clk_control(clk, true);

	DPRINTK("[VPUDRV] vpu_clk_enable\n");
	return 0;
}

static void jpu_clk_disable(jpu_clk_t *clk)
{
	if (clk == NULL || IS_ERR(clk))
		return;

	jpu_clk_control(clk, false);
	jpu_pmu_enable(false);

	DPRINTK("[VPUDRV] vpu_clk_disable\n");
}

#else /* STARFIVE_JPU_SUPPORT_CLOCK_CONTROL */

static int jpu_hw_reset(void)
{
    return reset_control_reset(s_jpu_clk->resets);
}

static int jpu_of_clk_get(struct platform_device *pdev, jpu_clk_t *jpu_clk)
{
	struct device *dev = &pdev->dev;
	int ret;

	jpu_clk->dev = dev;
	jpu_clk->clks = jpu_clks;
	jpu_clk->nr_clks = ARRAY_SIZE(jpu_clks);

	jpu_clk->resets = devm_reset_control_array_get_exclusive(dev);
	if (IS_ERR(jpu_clk->resets)) {
		ret = PTR_ERR(jpu_clk->resets);
		dev_err(dev, "faied to get jpu reset controls\n");
	}

	ret = devm_clk_bulk_get(dev, jpu_clk->nr_clks, jpu_clk->clks);
	if (ret)
		dev_err(dev, "faied to get jpu clk controls\n");

	return 0;
}

static jpu_clk_t *jpu_clk_get(struct platform_device *pdev)
{
	jpu_clk_t *jpu_clk;

	if (!pdev)
		return NULL;

	jpu_clk = devm_kzalloc(&pdev->dev, sizeof(*jpu_clk), GFP_KERNEL);
	if (!jpu_clk)
		return NULL;

	if (jpu_of_clk_get(pdev, jpu_clk))
		goto err_of_clk_get;

	return jpu_clk;

err_of_clk_get:
	devm_kfree(&pdev->dev, jpu_clk);
	return NULL;

}

static void jpu_clk_put(jpu_clk_t *clk)
{
	clk_bulk_put(clk->nr_clks, clk->clks);
}

static int jpu_clk_enable(jpu_clk_t *clk)
{
	int ret;

	jpu_pmu_enable(true);
	ret = clk_bulk_prepare_enable(clk->nr_clks, clk->clks);
	if (ret)
		dev_err(clk->dev, "enable clk error.\n");

	ret = reset_control_deassert(clk->resets);
	if (ret)
		dev_err(clk->dev, "deassert jpu error.\n");

	DPRINTK("[VPUDRV] jpu_clk_enable\n");
	return ret;
}

static void jpu_clk_disable(jpu_clk_t *clk)
{
	int ret;

	ret = reset_control_assert(clk->resets);
	if (ret)
		dev_err(clk->dev, "assert jpu error.\n");

	clk_bulk_disable_unprepare(clk->nr_clks, clk->clks);
	jpu_pmu_enable(false);
}
#endif /* STARFIVE_JPU_SUPPORT_CLOCK_CONTROL */
