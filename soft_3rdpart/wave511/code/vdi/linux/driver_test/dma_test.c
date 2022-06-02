#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/dma-mapping.h>
#include <linux/of.h>
#include <linux/wait.h>
#include <linux/list.h>
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/uaccess.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/of_address.h>
#include <linux/of_reserved_mem.h>
#include <linux/sched.h>
#include <linux/sched/signal.h>
#include <linux/version.h>
#include <linux/kfifo.h>
#include <linux/kthread.h>
#include <asm/io.h>
#include <mach-u74/starfive_u74.h>

#define VPU_PLATFORM_DEVICE_NAME "dma_test"


struct device *vpu_dev;

static int vpu_mmap(struct file *fp, struct vm_area_struct *vm)
{
	return 0;
}

static int vpu_open(struct inode *inode, struct file *filp)
{
    return 0;
}

/*static int vpu_ioctl(struct inode *inode, struct file *filp, u_int cmd, u_long arg) // for kernel 2.6.9 of C&M*/
static long vpu_ioctl(struct file *filp, u_int cmd, u_long arg)
{
   
    return 0;
}

static ssize_t vpu_read(struct file *filp, char __user *buf, size_t len, loff_t *ppos)
{

    return -1;
}

static ssize_t vpu_write(struct file *filp, const char __user *buf, size_t len, loff_t *ppos)
{
    return -1;
}

static int vpu_release(struct inode *inode, struct file *filp)
{
   return 0;
}

static int vpu_fasync(int fd, struct file *filp, int mode)
{
	return 0;
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


struct dma_addr_t{
	unsigned long phy;
	unsigned long base;
	int size;
};
struct dma_addr_t dma_addr[10] = {0};

static int vpu_probe(struct platform_device *pdev)
{
    int err = 0;
	int i = 0;

    printk("[VPUDRV] dma_test vpu_probe\n");
	if(pdev){
		vpu_dev = &pdev->dev;
		dev_info(vpu_dev,"device init.\n");
	}
    
#if 0   //use reserved dma space
	err = of_reserved_mem_device_init(vpu_dev);
	if(err) {
		dev_err(vpu_dev, "Could not get reserved memory\n");
		goto ERROR_PROVE_DEVICE;
	  }
	 
	  /* Allocate memory */
	  dma_set_coherent_mask(vpu_dev, 0xFFFFFFFF);
#endif
	 for(i = 0;i<10;i++)
	 {
		 dma_addr[i].size = 10485760;	 
	 }

	 //alloc
	for(i=0;i<10;i++){
		dma_addr[i].base = (unsigned long)dma_alloc_coherent(vpu_dev, PAGE_ALIGN(dma_addr[i].size), (dma_addr_t *) (&dma_addr[i].phy), GFP_DMA | GFP_KERNEL);
		printk("dma_alloc_coherent, physaddr:0x%lx,  virtual_addr:0x%lx size=%d \n",dma_addr[i].phy,dma_addr[i].base,dma_addr[i].size);
	}
	  
	  //free
	for(i=0;i<10;i++){	
		dma_free_coherent(vpu_dev, PAGE_ALIGN(dma_addr[i].size), (void *)dma_addr[i].base, dma_addr[i].phy);
		printk(" dma_free_coherent, physaddr:0x%lx,  virtual_addr:0x%lx size=%d \n",dma_addr[i].phy,dma_addr[i].base,dma_addr[i].size);
	}

    return 0;
ERROR_PROVE_DEVICE:
	printk("ERROR_PROVE_DEVICE\n");
	return -1;
}


static int vpu_remove(struct platform_device *pdev)
{
	return 0;
}

static int vpu_suspend(struct platform_device *pdev, pm_message_t state)
{
	return 0;
}

static int vpu_suspend(struct device *dev)
{
	return 0;
}
static int  vpu_resume(struct device *dev)
{
	return 0;
}

static int vpu_resume(struct platform_device *pdev)
{
	return 0;
}

static const struct of_device_id cm_vpu_match[] = {
    {
        .compatible = "stf,dma_test",
    },
    {
    },
};
MODULE_DEVICE_TABLE(of, cm_vpu_match);

static const struct dev_pm_ops vpu_pm_ops = {
	SET_RUNTIME_PM_OPS(vpu_suspend, vpu_resume, NULL)
};

static const struct of_device_id dw_dma_of_id_table[] = {
	{ .compatible = "stf,dma_test"},
	{}
};
MODULE_DEVICE_TABLE(of, dw_dma_of_id_table);

static struct platform_driver vpu_driver = {
    .driver = {
           .name = VPU_PLATFORM_DEVICE_NAME,
           .of_match_table = cm_vpu_match,
		   	.pm = &vpu_pm_ops,
    },
    .probe = vpu_probe,
    .remove = vpu_remove,
    .suspend = vpu_suspend,
    .resume = vpu_resume,
};

module_platform_driver(vpu_driver);

MODULE_AUTHOR("dma test.");
MODULE_DESCRIPTION("dma test.");
MODULE_LICENSE("GPL");
