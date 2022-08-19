#ifndef __VPU_DRV_H__
#define __VPU_DRV_H__

#include <linux/fs.h>
#include <linux/types.h>
#include "../../../vpuapi/vpuconfig.h"

#define USE_VMALLOC_FOR_INSTANCE_POOL_MEMORY

#define VDI_IOCTL_MAGIC  'V'
#define VDI_IOCTL_ALLOCATE_PHYSICAL_MEMORY  _IO(VDI_IOCTL_MAGIC, 0)
#define VDI_IOCTL_FREE_PHYSICALMEMORY       _IO(VDI_IOCTL_MAGIC, 1)
#define VDI_IOCTL_WAIT_INTERRUPT            _IO(VDI_IOCTL_MAGIC, 2)
#define VDI_IOCTL_SET_CLOCK_GATE            _IO(VDI_IOCTL_MAGIC, 3)
#define VDI_IOCTL_RESET                     _IO(VDI_IOCTL_MAGIC, 4)
#define VDI_IOCTL_GET_INSTANCE_POOL         _IO(VDI_IOCTL_MAGIC, 5)
#define VDI_IOCTL_GET_COMMON_MEMORY         _IO(VDI_IOCTL_MAGIC, 6)
#define VDI_IOCTL_GET_RESERVED_VIDEO_MEMORY_INFO _IO(VDI_IOCTL_MAGIC, 8)
#define VDI_IOCTL_OPEN_INSTANCE             _IO(VDI_IOCTL_MAGIC, 9)
#define VDI_IOCTL_CLOSE_INSTANCE            _IO(VDI_IOCTL_MAGIC, 10)
#define VDI_IOCTL_GET_INSTANCE_NUM          _IO(VDI_IOCTL_MAGIC, 11)
#define VDI_IOCTL_GET_REGISTER_INFO         _IO(VDI_IOCTL_MAGIC, 12)
#define VDI_IOCTL_GET_FREE_MEM_SIZE         _IO(VDI_IOCTL_MAGIC, 13)
#define VDI_IOCTL_FLUSH_DCACHE              _IO(VDI_IOCTL_MAGIC, 14)
#define VDI_IOCTL_GET_PHYSICAL_MEMORY       _IO(VDI_IOCTL_MAGIC, 15)
#define VDI_IOCTL_CPUFREQ_SAVEENV           _IO(VDI_IOCTL_MAGIC, 16)
#define VDI_IOCTL_CPUFREQ_PUTENV            _IO(VDI_IOCTL_MAGIC, 17)

#define DRAM_MEM2SYS(addr) ((addr) > 0x40000000 && (addr) < 0x43FFFFFFF ? ((addr)+0x400000000):(addr))
#define ioremap_nocache ioremap

typedef struct vpudrv_flush_cache_t {
    unsigned long start;
    unsigned long size;
    unsigned char flag;
} vpudrv_flush_cache_t;

typedef struct vpudrv_buffer_t {
    unsigned int size;
    unsigned long phys_addr;
    unsigned long base;                     /* kernel logical address in use kernel */
    unsigned long virt_addr;                /* virtual user space address */
} vpudrv_buffer_t;

typedef struct vpu_bit_firmware_info_t {
    unsigned int size;                      /* size of this structure*/
    unsigned int core_idx;
    unsigned long reg_base_offset;
    unsigned short bit_code[512];
} vpu_bit_firmware_info_t;

typedef struct vpudrv_inst_info_t {
    unsigned int core_idx;
    unsigned int inst_idx;
    int inst_open_count;    /* for output only*/
} vpudrv_inst_info_t;

typedef struct vpudrv_intr_info_t {
    unsigned int timeout;
    int         intr_reason;
#ifdef SUPPORT_MULTI_INST_INTR
    int         intr_inst_index;
#endif
} vpudrv_intr_info_t;
#endif

