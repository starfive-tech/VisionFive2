//------------------------------------------------------------------------------
// File: vdi.c
//
// Copyright (c) 2006, Chips & Media.  All rights reserved.
//------------------------------------------------------------------------------
#if defined(_WIN32) || defined(__WIN32__) || defined(_WIN64) || defined(WIN32) || defined(__MINGW32__)

//lint -e10
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <initguid.h>
#include <setupapi.h>
#include <malloc.h>
//lint +e10
#include "driver/Public.h"

#include "vpuconfig.h"
#include "vdi.h"
#include "vdi_osal.h"
#include "coda9/coda9_regdefine.h"
#include "wave/common/common_regdefine.h"
#include "wave/wave4/wave4_regdefine.h"
#include "wave/coda7q/coda7q_regdefine.h"
#include "wave/wave5/wave5_regdefine.h"
typedef HANDLE MUTEX_HANDLE;


//#define SUPPORT_INTERRUPT
#	define VDI_SRAM_BASE_ADDR               0x00	    // if we can know the sram address in SOC directly for vdi layer. it is possible to set in vdi layer without allocation from driver
#	define VDI_WAVE410_SRAM_SIZE		    0x25000     // 8Kx8K MAIN10 MAX size
#	define VDI_WAVE412_SRAM_SIZE			0x80000
#	define VDI_WAVE512_SRAM_SIZE			0x80000
#	define VDI_WAVE515_SRAM_SIZE			0x80000
#	define VDI_WAVE520_SRAM_SIZE			0x25000     // 8Kx8X MAIN10 MAX size
#	define VDI_WAVE420_SRAM_SIZE		    0x2E000     // 8Kx8X MAIN10 MAX size
#	define VDI_WAVE420L_SRAM_SIZE		    0x2E000     // 8Kx8X MAIN10 MAX size
#	define VDI_CODA9_SRAM_SIZE			    0x34600     // FHD MAX size, 0x17D00  4K MAX size 0x34600
#	define VDI_SYSTEM_ENDIAN 	        VDI_LITTLE_ENDIAN
#	define VDI_128BIT_BUS_SYSTEM_ENDIAN		        VDI_128BIT_LITTLE_ENDIAN

#ifdef SUPPORT_MULTI_CORE_IN_ONE_DRIVER
#define VPU_CORE_BASE_OFFSET 0x4000
#endif

#ifndef INFINITE
#define INFINITE            0xFFFFFFFF  // Infinite timeout
#endif 

typedef struct vpu_buffer_pool_t
{
    vpudrv_buffer_t vdb;
    int inuse;
} vpu_buffer_pool_t;

typedef struct _vdi_info_t {
    unsigned long       coreIdx;
	unsigned int		product_code;
    HANDLE              vpu_fd;	
	BYTE *vip;          // for old hpi driver
    vpu_instance_pool_t *pvip;
    int                 task_num;
    int                 clock_state;
    vpudrv_buffer_t     vdb_video_memory;
    vpudrv_buffer_t     vdb_register;
    vpu_buffer_t        vpu_common_memory;
    vpu_buffer_pool_t   vpu_buffer_pool[MAX_VPU_BUFFER_POOL];
    int                 vpu_buffer_pool_count;
    MUTEX_HANDLE        vpu_mutex;
    MUTEX_HANDLE        vpu_disp_mutex;
    MUTEX_HANDLE        vmem_mutex;

    HDEVINFO            hDevInfo;
    PSP_DEVICE_INTERFACE_DETAIL_DATA pDeviceInterfaceDetail;	
} vdi_info_t;

static vdi_info_t s_vdi_info[MAX_NUM_VPU_CORE];


static int s_use_old_hpi_driver;

static int swap_endian(unsigned long coreIdx, unsigned char *data, int len, int endian);
static int allocate_common_memory(unsigned long coreIdx);
#pragma comment(lib,"setupapi.lib")
static int GetDevicePath(unsigned long coreIdx);

// for old hpi driver
#define VPU_DEVICE_NAME                 "\\\\.\\HPI"
#define FILE_DEVICE_HPI					0x8000
#define IOCTL_CREATE_SHARE_MEMORY		CTL_CODE(FILE_DEVICE_HPI, 0x802, METHOD_OUT_DIRECT, FILE_READ_ACCESS)
typedef struct _HPI_LIST_ITEM
{
	ULONG						Ibuffer;
	ULONG						InSize;
	ULONG						OutBufferL[2];
	ULONG                       OutSize;
} HPI_LIST_ITEM, *PHPI_LIST_ITEM;

HPI_LIST_ITEM					ioItem;

int vdi_probe(unsigned long coreIdx)
{
    int ret;

    ret = vdi_init(coreIdx);
    vdi_release(coreIdx);
    return ret;
}

int vdi_init(unsigned long coreIdx)
{
    int ret;
    vdi_info_t *vdi;

    if (coreIdx > MAX_VPU_CORE_NUM)
        return 0;

    vdi = &s_vdi_info[coreIdx];

    if (vdi->vpu_fd != (HANDLE)-1 && vdi->vpu_fd != (HANDLE)0x00)
    {
        vdi->task_num++;		
        return 0;
    }

    if (GetDevicePath(coreIdx))
	{
		vdi->vpu_fd = CreateFile(vdi->pDeviceInterfaceDetail->DevicePath,
			GENERIC_READ|GENERIC_WRITE,
			FILE_SHARE_READ|FILE_SHARE_WRITE,
			NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			NULL);

		if (vdi->vpu_fd == INVALID_HANDLE_VALUE) {
			VLOG(ERR, "CreateFile failed.  Error:%d", GetLastError());
			goto ERR_VDI_INIT;
		}

		s_use_old_hpi_driver = 0;
	}
	else
	{
        vdi->vpu_fd = CreateFile((LPCWSTR)VPU_DEVICE_NAME, GENERIC_READ | GENERIC_WRITE, 0,  NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (vdi->vpu_fd == INVALID_HANDLE_VALUE) 
		{
			VLOG(ERR, "Fail to open old hpi driver.  Error:%d", GetLastError());
			VLOG(ERR, "Please Check Windows device driver");
			return -1;
		}

		s_use_old_hpi_driver = 1;	
		vdi->hDevInfo = NULL;
		vdi->pDeviceInterfaceDetail = NULL;
	}

	if (s_use_old_hpi_driver == 0)
	{
		vdi->vdb_register.phys_addr.QuadPart = -1; // -1 means let driver to map a memory of VPU Register.
        if (!DeviceIoControl(vdi->vpu_fd, (DWORD)VDI_IOCTL_MAP_PHYSICAL_MEMORY, NULL, 0, 
			&vdi->vdb_register, sizeof(vpudrv_buffer_t), (LPDWORD)&ret,  NULL))
		{		
			VLOG(ERR, "Fail to map vpu register error:%d", GetLastError());
			goto ERR_VDI_INIT;
		}
	}
	else
	{
		unsigned long ReturnedDataSize;
		ioItem.OutBufferL[0] = 0;
		ioItem.OutSize = 4;
		ioItem.Ibuffer = 0x0;
		ioItem.InSize = 0;

        if (!DeviceIoControl((HANDLE)vdi->vpu_fd, (DWORD)IOCTL_CREATE_SHARE_MEMORY, NULL, 0, ioItem.OutBufferL, ioItem.OutSize,  &ReturnedDataSize, NULL))
		{		
			VLOG(ERR, "Fail to map vpu register error:%d", GetLastError());
			if (vdi->vpu_fd)
				CloseHandle(vdi->vpu_fd);
			return 0;
		}

		vdi->vdb_register.virt_addr = (ULONGLONG)ioItem.OutBufferL[0];
	}

    memset(&vdi->vpu_buffer_pool, 0x00, sizeof(vpu_buffer_pool_t)*MAX_VPU_BUFFER_POOL);

    if (!vdi_get_instance_pool(coreIdx))
    {
        VLOG(ERR, "[VDI] fail to create shared info for saving context \n");
        goto ERR_VDI_INIT;
    }

#ifdef UNICODE
	vdi->vpu_mutex = CreateEvent(NULL, FALSE, TRUE, L"VPU_MUTEX");
#else
	vdi->vpu_mutex = CreateEvent(NULL, FALSE, TRUE, "VPU_MUTEX");
#endif
    if (!vdi->vpu_mutex)
    {
        VLOG(ERR, "[VDI] fail to create mutex for vpu instance\n");
        goto ERR_VDI_INIT;
    }
#ifdef UNICODE
	vdi->vpu_disp_mutex = CreateEvent(NULL, FALSE, TRUE, L"VPU_DISP_MUTEX");
#else
	vdi->vpu_disp_mutex = CreateEvent(NULL, FALSE, TRUE, "VPU_DISP_MUTEX");
#endif
    if (!vdi->vpu_disp_mutex)
    {
        VLOG(ERR, "[VDI] fail to create mutext for vpu display lockl\n");
        goto ERR_VDI_INIT;
    }

	if (s_use_old_hpi_driver == 0)
	{
        if (!DeviceIoControl(vdi->vpu_fd, (DWORD)VDI_IOCTL_GET_RESERVED_VIDEO_MEMORY_INFO, NULL, 0,
            (LPVOID)&vdi->vdb_video_memory, sizeof(vpudrv_buffer_t), (LPDWORD)&ret,  NULL))
        {		

            VLOG(ERR, "[VDI] fail to get video memory which is allocated from driver\n");
            goto ERR_VDI_INIT;
        }
    }

#if 0
    if (REQUIRED_VPU_MEMORY_SIZE > vdi->vdb_video_memory.size)
    {
        VLOG(ERR, "[VDI] Warning : VPU memory will be overflow\n");
    }
#endif

    if (allocate_common_memory(coreIdx) < 0) 
    {
        VLOG(ERR, "[VDI] fail to get vpu common buffer from driver\n");
        goto ERR_VDI_INIT;
    }

    if (!vdi->pvip->instance_pool_inited) 
        osal_memset(&vdi->pvip->vmem, 0x00, sizeof(video_mm_t));

#ifdef SUPPORT_MULTI_CORE_IN_ONE_DRIVER
	ret = vmem_init(&vdi->pvip->vmem, (unsigned long)vdi->vdb_video_memory.phys_addr.QuadPart + (vdi->pvip->vpu_common_buffer.size*MAX_VPU_CORE_NUM),
		vdi->vdb_video_memory.size - (vdi->pvip->vpu_common_buffer.size*MAX_VPU_CORE_NUM));		
#else
    ret = vmem_init(&vdi->pvip->vmem, (unsigned long)vdi->vdb_video_memory.phys_addr.QuadPart + (vdi->pvip->vpu_common_buffer.size),
        vdi->vdb_video_memory.size - vdi->pvip->vpu_common_buffer.size);
#endif

#ifdef UNICODE
	vdi->vmem_mutex = CreateEvent(NULL, FALSE, TRUE, L"VMEM_MUTEX");        
#else
	vdi->vmem_mutex = CreateEvent(NULL, FALSE, TRUE, "VMEM_MUTEX");
#endif

    if (ret < 0)
    {
        VLOG(ERR, "[VDI] fail to init vpu memory management logic\n");
        goto ERR_VDI_INIT;
    }

	vdi_set_clock_gate(coreIdx, 1);


	vdi->product_code = vdi_read_register(coreIdx, VPU_PRODUCT_CODE_REGISTER);
    if (PRODUCT_CODE_W_SERIES(vdi->product_code)) {
		if (vdi_read_register(coreIdx, W4_VCPU_CUR_PC) == 0) { // if BIT processor is not running.
			int i;
			for (i=0; i<64; i++)
				vdi_write_register(coreIdx, (i*4) + 0x100, 0x0); 
		}
	}
	else if (PRODUCT_CODE_NOT_W_SERIES(vdi->product_code)) { // CODA9XX
		if (vdi_read_register(coreIdx, BIT_CUR_PC) == 0) { // if BIT processor is not running.
			int i;
			for (i=0; i<64; i++)
				vdi_write_register(coreIdx, (i*4) + 0x100, 0x0); 
		}
	}
    else {
        VLOG(ERR, "Unknown product id : %08x\n", vdi->product_code);
        goto ERR_VDI_INIT;
    }

    if (vdi_lock(coreIdx) < 0)
    {
        VLOG(ERR, "[VDI] fail to handle lock function\n");
        goto ERR_VDI_INIT;
    }

    vdi->coreIdx = coreIdx;
    vdi->task_num++;	
    vdi_unlock(coreIdx);
	if (s_use_old_hpi_driver == 0)
		VLOG(INFO, "[VDI] success to init driver with new hpi\n");
	else
		VLOG(INFO, "[VDI] success to init driver with old hpi\n");

    return 0;

ERR_VDI_INIT:
    vdi_unlock(coreIdx);
    vdi_release(coreIdx);
    return -1;
}

int vdi_set_bit_firmware_to_pm(unsigned long coreIdx, const unsigned short *code)
{
    UNREFERENCED_PARAMETER(coreIdx);
    UNREFERENCED_PARAMETER(code);
    return 0;
}

int vdi_release(unsigned long coreIdx)
{
    int i;
    vpudrv_buffer_t vdb;
    vdi_info_t *vdi = &s_vdi_info[coreIdx];
    int ret;

    if (!vdi || vdi->vpu_fd == (HANDLE)-1 || vdi->vpu_fd == (HANDLE)0x00)
        return 0;

    if (vdi_lock(coreIdx) < 0)
    {
        VLOG(ERR, "[VDI] fail to handle lock function\n");
        return -1;
    }

    if (vdi->task_num > 1) // means that the opened instance remains 
    {
        vdi->task_num--;
        vdi_unlock(coreIdx);
        return 0;
    }


    CloseHandle((HANDLE)vdi->vmem_mutex);	// that is OK for windows API
    vmem_exit(&vdi->pvip->vmem);
    if (s_use_old_hpi_driver == 0)
    {
		int ret;
        if (!DeviceIoControl(vdi->vpu_fd, (DWORD)VDI_IOCTL_UNMAP_PHYSICALMEMORY, NULL, 0,
            &vdi->vdb_register, sizeof(vpudrv_buffer_t), (LPDWORD)&ret,  NULL))
        {		
            VLOG(ERR, "Fail to unmap vpu register error:%d", GetLastError());
            return 0;
        }
    }

    vdb.size = 0;
    // get common memory information to free virtual address
    for (i=0; i<MAX_VPU_BUFFER_POOL; i++)
    {
        if (vdi->vpu_common_memory.phys_addr >= vdi->vpu_buffer_pool[i].vdb.phys_addr.QuadPart &&
            vdi->vpu_common_memory.phys_addr < (vdi->vpu_buffer_pool[i].vdb.phys_addr.QuadPart + vdi->vpu_buffer_pool[i].vdb.size))
        {
            vdi->vpu_buffer_pool[i].inuse = 0;
            vdi->vpu_buffer_pool_count--;
            vdb = vdi->vpu_buffer_pool[i].vdb;
            break;
        }
    }

    if (vdb.size > 0)
    {
        if (!DeviceIoControl(vdi->vpu_fd, VDI_IOCTL_UNMAP_PHYSICALMEMORY, NULL, 0,
            &vdb, sizeof(vpudrv_buffer_t), (LPDWORD)&ret,  NULL))
        {		
			VLOG(ERR, "[VDI] fail to unmap physical memory virtual address = 0x%llu\n", (int)vdb.virt_addr);
        }
        memset(&vdi->vpu_common_memory, 0x00, sizeof(vpu_buffer_t));
    }

    vdi->task_num--;

    if (vdi->vpu_fd != (HANDLE)-1 && vdi->vpu_fd != (HANDLE)0x00)
    {      

		vdi->vpu_fd = (HANDLE)-1;
        if (vdi->hDevInfo) {
            SetupDiDestroyDeviceInfoList(vdi->hDevInfo);
        }

        if (vdi->pDeviceInterfaceDetail) {
            free(vdi->pDeviceInterfaceDetail);
        }

        if (vdi->vip)
            free(vdi->vip);
    }

    vdi_unlock(coreIdx);

    if (vdi->vpu_mutex)
    {
        CloseHandle(vdi->vpu_mutex);
        vdi->vpu_mutex = NULL;
    }

    if (vdi->vpu_disp_mutex)
    {
        CloseHandle(vdi->vpu_disp_mutex);
        vdi->vpu_disp_mutex = NULL;
    }

    memset(vdi, 0x00, sizeof(vdi_info_t));

    return 0;
}

int vdi_get_common_memory(unsigned long coreIdx, vpu_buffer_t *vb)
{
    vdi_info_t *vdi = &s_vdi_info[coreIdx];

    if(!vdi || vdi->vpu_fd==(HANDLE)-1 || vdi->vpu_fd==(HANDLE)0x00)
        return -1;

    osal_memcpy(vb, &vdi->vpu_common_memory, sizeof(vpu_buffer_t));

    return 0;
}

int allocate_common_memory(unsigned long coreIdx)
{
    vdi_info_t *vdi = &s_vdi_info[coreIdx];
    vpudrv_buffer_t vdb = {0};
    int i;
    int ret;
    if(!vdi || vdi->vpu_fd==(HANDLE)-1 || vdi->vpu_fd==(HANDLE)0x00)
        return -1;

    if (vdi->pvip->vpu_common_buffer.size == 0)
    {
		vdb.size = SIZE_COMMON*MAX_VPU_CORE_NUM;
        vdb.phys_addr = vdi->vdb_video_memory.phys_addr; // set at the beginning of base address
        vdb.base =  vdi->vdb_video_memory.base;

        if (!DeviceIoControl(vdi->vpu_fd, VDI_IOCTL_MAP_PHYSICAL_MEMORY, NULL, 0, 
            &vdb, sizeof(vpudrv_buffer_t), (LPDWORD)&ret,  NULL))
        {
            return -1;
        }

        VLOG(INFO, "[VDI] allocate_common_memory, physaddr=0x%x, virtaddr=0x%x\n", vdb.phys_addr.QuadPart, vdb.virt_addr);
        // convert os driver buffer type to vpu buffer type
#ifdef SUPPORT_MULTI_CORE_IN_ONE_DRIVER
        vdi->pvip->vpu_common_buffer.size = SIZE_COMMON;
        vdi->pvip->vpu_common_buffer.phys_addr = (unsigned long)(vdb.phys_addr.QuadPart + (coreIdx*SIZE_COMMON));
        vdi->pvip->vpu_common_buffer.base = (unsigned long)(vdb.base + (coreIdx*SIZE_COMMON));
        vdi->pvip->vpu_common_buffer.virt_addr = (unsigned long)(vdb.virt_addr + (coreIdx*SIZE_COMMON));
#else
        vdi->pvip->vpu_common_buffer.size = SIZE_COMMON;
        vdi->pvip->vpu_common_buffer.phys_addr = (unsigned long)(vdb.phys_addr.QuadPart);
        vdi->pvip->vpu_common_buffer.base = (unsigned long)(vdb.base);
        vdi->pvip->vpu_common_buffer.virt_addr = (unsigned long)(vdb.virt_addr);
#endif

        osal_memcpy(&vdi->vpu_common_memory, &vdi->pvip->vpu_common_buffer, sizeof(vpudrv_buffer_t));
    }	
    else
    {

		vdb.size = SIZE_COMMON*MAX_VPU_CORE_NUM;
        vdb.phys_addr = vdi->vdb_video_memory.phys_addr; // set at the beginning of base address
        vdb.base =  vdi->vdb_video_memory.base;			

        if (!DeviceIoControl(vdi->vpu_fd, VDI_IOCTL_MAP_PHYSICAL_MEMORY, NULL, 0, 
            &vdb, sizeof(vpudrv_buffer_t), (LPDWORD)&ret,  NULL))
        {
            return -1;
        }		
#ifdef SUPPORT_MULTI_CORE_IN_ONE_DRIVER
        vdi->pvip->vpu_common_buffer.virt_addr = (unsigned long)(vdb.virt_addr + (coreIdx*SIZE_COMMON)); 
#else
		vdi->pvip->vpu_common_buffer.virt_addr = (unsigned long)vdb.virt_addr; 
#endif

        osal_memcpy(&vdi->vpu_common_memory, &vdi->pvip->vpu_common_buffer, sizeof(vpudrv_buffer_t));

        VLOG(INFO, "[VDI] allocate_common_memory, physaddr=0x%x, virtaddr=0x%x\n", (int)vdi->pvip->vpu_common_buffer.phys_addr, (int)vdi->pvip->vpu_common_buffer.virt_addr);

    }

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

    VLOG(TRACE, "[VDI] vdi_get_common_memory physaddr=0x%x, size=%d, virtaddr=0x%x\n", vdi->vpu_common_memory.phys_addr, vdi->vpu_common_memory.size, vdi->vpu_common_memory.virt_addr);

    return 0;
}

vpu_instance_pool_t *vdi_get_instance_pool(unsigned long coreIdx)
{
    vdi_info_t *vdi;
    vpudrv_buffer_t vdb = {0};
   
	if (coreIdx > MAX_VPU_CORE_NUM)
        return NULL;

    vdi = &s_vdi_info[coreIdx];

    if(!vdi || vdi->vpu_fd == (HANDLE)-1 || vdi->vpu_fd == (HANDLE)0x00 )
        return NULL;

    if (!vdi->pvip)
    {
        vdb.size = sizeof(vpu_instance_pool_t) + sizeof(MUTEX_HANDLE)*3;
#ifdef SUPPORT_MULTI_CORE_IN_ONE_DRIVER
		vdb.size  *= MAX_VPU_CORE_NUM;
#endif
		if (s_use_old_hpi_driver)
		{
            vdi->vip = (BYTE *)osal_malloc(vdb.size); //&vdi->vip;
            vdi->pvip = (vpu_instance_pool_t *)(vdi->vip + (coreIdx*(sizeof(vpu_instance_pool_t) + sizeof(MUTEX_HANDLE)*2)));
		}
		else
		{
			unsigned long ret;
            if (!DeviceIoControl(vdi->vpu_fd, (DWORD)VDI_IOCTL_GET_INSTANCE_POOL, NULL, 0,
				(LPVOID)&vdb, sizeof(vpudrv_buffer_t), (LPDWORD)&ret,  NULL))
			{		

				VLOG(ERR, "[VDI] fail to allocate get instance pool physical space=%d, error=0x%x\n", vdb.size, GetLastError());
				return NULL;
			}

			vdi->pvip = (vpu_instance_pool_t *)(vdb.virt_addr + (coreIdx*(sizeof(vpu_instance_pool_t) + sizeof(MUTEX_HANDLE)*3)));
		}
        VLOG(INFO, "[VDI] instance pool physaddr=0x%x, virtaddr=0x%x, base=0x%x, size=%d\n", (int)vdb.phys_addr.QuadPart, (int)vdb.virt_addr, (int)vdb.base, (int)vdb.size);
    }

    return (vpu_instance_pool_t *)vdi->pvip;
}

int vdi_open_instance(unsigned long coreIdx, unsigned long instIdx)
{
	vdi_info_t *vdi = &s_vdi_info[coreIdx];

    UNREFERENCED_PARAMETER(instIdx);

	if(!vdi || vdi->vpu_fd == (HANDLE)-1 || vdi->vpu_fd == (HANDLE)0x00)
		return -1;

	vdi->pvip->vpu_instance_num++;

	return 0;
}

int vdi_close_instance(unsigned long coreIdx, unsigned long instIdx)
{
	vdi_info_t *vdi = &s_vdi_info[coreIdx];

    UNREFERENCED_PARAMETER(instIdx);

	if(!vdi || vdi->vpu_fd == (HANDLE)-1 || vdi->vpu_fd == (HANDLE)0x00)
		return -1;

	vdi->pvip->vpu_instance_num--;

	return 0;
}

int vdi_get_instance_num(unsigned long coreIdx)
{
    vdi_info_t *vdi = &s_vdi_info[coreIdx];

    if(!vdi || vdi->vpu_fd == (HANDLE)-1 || vdi->vpu_fd == (HANDLE)0x00)
        return -1;

    return vdi->pvip->vpu_instance_num;
}

int vdi_hw_reset(unsigned long coreIdx) // DEVICE_ADDR_SW_RESET
{
    vdi_info_t *vdi = &s_vdi_info[coreIdx];
    int ret;

    if(!vdi || vdi->vpu_fd == (HANDLE)-1 || vdi->vpu_fd == (HANDLE)0x00)
        return -1;

    DeviceIoControl(vdi->vpu_fd, (DWORD)VDI_IOCTL_RESET, NULL, 0, NULL, 0, (LPDWORD)&ret, NULL);


    return 0;
}

int vdi_lock(unsigned long coreIdx)
{
    vdi_info_t *vdi = &s_vdi_info[coreIdx];
    const unsigned int MUTEX_TIMEOUT = INFINITE;	// ms

    if(!vdi || !vdi->vpu_mutex)
        return -1;

    if (WaitForSingleObject(vdi->vpu_mutex, MUTEX_TIMEOUT) != WAIT_OBJECT_0) {
        return -1;
    }
    return 0;
}

int vdi_lock_check(unsigned long coreIdx)
{
    vdi_info_t *vdi = &s_vdi_info[coreIdx];
    int ret;

    ret = WaitForSingleObject((MUTEX_HANDLE *)vdi->vpu_mutex, 0);
    if(ret == 0) 
    {
        vdi_unlock(coreIdx);
        return -1;
    }
    else
    {
        return 0;
    }
}

void vdi_unlock(unsigned long coreIdx)
{
    vdi_info_t *vdi = &s_vdi_info[coreIdx];

    if(!vdi || !vdi->vpu_mutex)
        return;

    SetEvent(vdi->vpu_mutex);	

}

int vdi_disp_lock(unsigned long coreIdx)
{
    vdi_info_t *vdi = &s_vdi_info[coreIdx];
    const int MUTEX_TIMEOUT = 10000;    // ms

    if(!vdi || !vdi->vpu_disp_mutex)
        return -1;

    if (WaitForSingleObject(vdi->vpu_disp_mutex, MUTEX_TIMEOUT) != WAIT_OBJECT_0)
        return -1;

    return 0;
}

void vdi_disp_unlock(unsigned long coreIdx)
{
    vdi_info_t *vdi = &s_vdi_info[coreIdx];

    if(!vdi || !vdi->vpu_disp_mutex)
        return ;

    SetEvent(vdi->vpu_disp_mutex);

}

int vmem_lock(unsigned long coreIdx)
{
    vdi_info_t *vdi = &s_vdi_info[coreIdx];
    const unsigned int MUTEX_TIMEOUT = INFINITE;	// ms

    if (WaitForSingleObject((HANDLE)vdi->vmem_mutex, MUTEX_TIMEOUT) != WAIT_OBJECT_0) {
        return -1;
    }
    return 0;
}

void vmem_unlock(unsigned long coreIdx)
{
    vdi_info_t *vdi = &s_vdi_info[coreIdx];

    SetEvent((HANDLE)vdi->vmem_mutex);	
}

void vdi_write_register(unsigned long coreIdx, unsigned int addr, unsigned int data)
{
    vdi_info_t *vdi = &s_vdi_info[coreIdx];
    unsigned long *reg_addr = NULL;
    if(!vdi || vdi->vpu_fd==(HANDLE)-1 || vdi->vpu_fd == (HANDLE)0x00)
		return;

#if 1//def CNM_FPGA_PLATFORM		
	if(!vdi->clock_state)
	{ 
		VLOG(ERR, "[VDI] vdi_write_register clock is in off. enter infinite loop\n");
		while (1); 
	}
#endif

#ifdef SUPPORT_MULTI_CORE_IN_ONE_DRIVER
    reg_addr = (unsigned long *)(addr + (unsigned long)vdi->vdb_register.virt_addr + (coreIdx*VPU_CORE_BASE_OFFSET));	
#else
    reg_addr = (unsigned long *)(addr + (unsigned long)vdi->vdb_register.virt_addr);
#endif
    *(volatile unsigned long *)reg_addr = data;	
}

unsigned int vdi_read_register(unsigned long coreIdx, unsigned int addr)
{
    vdi_info_t *vdi = &s_vdi_info[coreIdx];
    unsigned long *reg_addr = NULL;

    if(!vdi || vdi->vpu_fd==(HANDLE)-1 || vdi->vpu_fd == (HANDLE)0x00)
        return (unsigned int)-1;

#if 1//def CNM_FPGA_PLATFORM		
	if(!vdi->clock_state)
	{ 
		VLOG(ERR, "[VDI] vdi_read_register clock is in off. enter infinite loop\n");
		while (1); 
	}
#endif

#ifdef SUPPORT_MULTI_CORE_IN_ONE_DRIVER
    reg_addr = (unsigned long *)(addr + (unsigned long)vdi->vdb_register.virt_addr + (coreIdx*VPU_CORE_BASE_OFFSET));	
#else
    reg_addr = (unsigned long *)(addr + (unsigned long)vdi->vdb_register.virt_addr);
#endif
    return *(volatile unsigned long *)reg_addr;
}

#define FIO_TIMEOUT         10000

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

    while ((int)vdi_fio_read_register(0, VCORE_DBG_READY(vce_core_idx)) < 0) {
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

unsigned int vdi_fio_read_register(unsigned long coreIdx, unsigned int addr)
{
    unsigned int ctrl;
    unsigned int count = 0;
    unsigned int data  = 0xffffffff;

    ctrl  = (addr&0xffff);
    ctrl |= (0<<16);    /* read operation */
    vdi_write_register(coreIdx, W4_VPU_FIO_CTRL_ADDR, ctrl);
    count = FIO_TIMEOUT;
    while (count--) {
        ctrl = vdi_read_register(coreIdx, W4_VPU_FIO_CTRL_ADDR);
        if (ctrl & 0x80000000) {
            data = vdi_read_register(coreIdx, W4_VPU_FIO_DATA);
            break;
        }
    }

    return data;
}

void vdi_fio_write_register(unsigned long coreIdx, unsigned int addr, unsigned int data)
{
    unsigned int ctrl;

    vdi_write_register(coreIdx, W4_VPU_FIO_DATA, data);
    ctrl  = (addr&0xffff);
    ctrl |= (1<<16);    /* write operation */
    vdi_write_register(coreIdx, W4_VPU_FIO_CTRL_ADDR, ctrl);
}

int vdi_clear_memory(unsigned long coreIdx, unsigned int addr, int len, int endian)
{	
    vdi_info_t*     vdi;
    vpudrv_buffer_t vdb = {0};
    Uint32          i;
    Uint8*          zero;

#ifdef SUPPORT_MULTI_CORE_IN_ONE_DRIVER
    coreIdx = 0;
#endif
    vdi = &s_vdi_info[coreIdx];

    if(!vdi || vdi->vpu_fd==(HANDLE)-1 || vdi->vpu_fd == (HANDLE)0x00)
        return -1;

    for (i=0; i<MAX_VPU_BUFFER_POOL; i++)
    {
        if (vdi->vpu_buffer_pool[i].inuse == 1)
        {
            vdb = vdi->vpu_buffer_pool[i].vdb;
            if (addr >= vdb.phys_addr.QuadPart && addr < (vdb.phys_addr.QuadPart + vdb.size))
                break;
        }
    }

    if (!vdb.size) {
        VLOG(ERR, "address 0x%08x is not mapped address!!!\n", addr);
        return -1;
    }

    zero = (Uint8*)osal_malloc(len);
    osal_memset((void*)zero, 0x00, len);

    osal_memcpy((void *)(vdb.virt_addr), zero, len);	

    osal_free(zero);

    return len;

}

int vdi_write_memory(unsigned long coreIdx, unsigned int addr, unsigned char *data, int len, int endian)
{	
    vdi_info_t *vdi;
    vpudrv_buffer_t vdb = {0};
    int i;

#ifdef SUPPORT_MULTI_CORE_IN_ONE_DRIVER
    coreIdx = 0;
#endif
    vdi = &s_vdi_info[coreIdx];

    if(!vdi || vdi->vpu_fd==(HANDLE)-1 || vdi->vpu_fd == (HANDLE)0x00)
        return -1;

    for (i=0; i<MAX_VPU_BUFFER_POOL; i++)
    {
        if (vdi->vpu_buffer_pool[i].inuse == 1)
        {
            vdb = vdi->vpu_buffer_pool[i].vdb;
            if (addr >= vdb.phys_addr.QuadPart && addr < (vdb.phys_addr.QuadPart + vdb.size))
                break;
        }
    }

    if (!vdb.size) {
        VLOG(ERR, "address 0x%08x is not mapped address!!!\n", addr);
        return -1;
    }


	swap_endian(coreIdx, data, len, endian);
	osal_memcpy((void *)(vdb.virt_addr), data, len);	

    return len;

}

int vdi_read_memory(unsigned long coreIdx, unsigned int addr, unsigned char *data, int len, int endian)
{
    vdi_info_t *vdi;
    vpudrv_buffer_t vdb = {0};
    int i;

#ifdef SUPPORT_MULTI_CORE_IN_ONE_DRIVER
    coreIdx = 0;
#endif
    vdi = &s_vdi_info[coreIdx];

    if(!vdi || vdi->vpu_fd==(HANDLE)-1 || vdi->vpu_fd == (HANDLE)0x00)
        return -1;

    for (i=0; i<MAX_VPU_BUFFER_POOL; i++)
    {
        if (vdi->vpu_buffer_pool[i].inuse == 1)
        {
            vdb = vdi->vpu_buffer_pool[i].vdb;
            if (addr >= vdb.phys_addr.QuadPart && addr < (vdb.phys_addr.QuadPart + vdb.size))
                break;		
        }
    }

    if (!vdb.size)
        return -1;

	osal_memcpy(data, (const void *)(vdb.virt_addr), len);
	swap_endian(coreIdx, data, len,  endian);

    return len;
}

int vdi_allocate_dma_memory(unsigned long coreIdx, vpu_buffer_t *vb)
{
    vdi_info_t *vdi;
    int i;
    int ret = 0;
    Uint32        addr;
    unsigned long offset;
    vpudrv_buffer_t vdb = {0};

#ifdef SUPPORT_MULTI_CORE_IN_ONE_DRIVER
    coreIdx = 0;
#endif
    vdi = &s_vdi_info[coreIdx];

    if(!vdi || vdi->vpu_fd==(HANDLE)-1 || vdi->vpu_fd == (HANDLE)0x00)
        return -1;	

    vdb.size = vb->size;
    vmem_lock(coreIdx);
    addr = vmem_alloc(&vdi->pvip->vmem, vdb.size, 0);
    if ((Uint32)addr == (Uint32)-1)
    {
        vmem_unlock(coreIdx);
        return -1; // not enough memory
    }

    vdb.phys_addr.QuadPart = (ULONGLONG)addr;

    offset = (unsigned long)(vdb.phys_addr.QuadPart - vdi->vdb_video_memory.phys_addr.QuadPart);
    vdb.base = (unsigned long )vdi->vdb_video_memory.base + offset;

    vb->phys_addr = (unsigned long)vdb.phys_addr.QuadPart;
    vb->base = (unsigned long)vdb.base;
    if (!DeviceIoControl(vdi->vpu_fd, VDI_IOCTL_MAP_PHYSICAL_MEMORY, NULL, 0,
        &vdb, sizeof(vpudrv_buffer_t), (LPDWORD)&ret,  NULL))
    {
        vmem_unlock(coreIdx);
        return -1;
    }
    vb->virt_addr = (unsigned long)vdb.virt_addr;
    vmem_unlock(coreIdx);

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

    VLOG(INFO, "[VDI] vdi_allocate_dma_memory, physaddr=0x%p, virtaddr=0x%p~0x%p, size=%d\n", 
        vb->phys_addr, vb->virt_addr, vb->virt_addr+vb->size, vb->size);

    return 0;
}


int vdi_attach_dma_memory(unsigned long coreIdx, vpu_buffer_t *vb)
{
    vdi_info_t *vdi;
    int i;
    int ret = 0;
    unsigned long offset;
    vpudrv_buffer_t vdb = {0};

#ifdef SUPPORT_MULTI_CORE_IN_ONE_DRIVER
    coreIdx = 0;
#endif
    vdi = &s_vdi_info[coreIdx];

    if(!vdi || vdi->vpu_fd==(HANDLE)-1 || vdi->vpu_fd == (HANDLE)0x00)
        return -1;	

    vdb.size = vb->size;
    vdb.phys_addr.QuadPart = vb->phys_addr;
    offset = (unsigned long)(vdb.phys_addr.QuadPart - vdi->vdb_video_memory.phys_addr.QuadPart);
    vdb.base = (unsigned long )vdi->vdb_video_memory.base + offset;
	vdb.virt_addr = vb->virt_addr;

    for (i=0; i<MAX_VPU_BUFFER_POOL; i++)
    {
        if (vdi->vpu_buffer_pool[i].vdb.phys_addr.QuadPart == vb->phys_addr)
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

    return 0;
}

int vdi_dettach_dma_memory(unsigned long coreIdx, vpu_buffer_t *vb)
{
    vdi_info_t *vdi;
    int i;
    int ret = 0;

#ifdef SUPPORT_MULTI_CORE_IN_ONE_DRIVER
    coreIdx = 0;
#endif
    vdi = &s_vdi_info[coreIdx];

    if(!vb || !vdi || vdi->vpu_fd==(HANDLE)-1 || vdi->vpu_fd == (HANDLE)0x00)
        return -1;

    if (vb->size == 0)
        return -1;

    for (i=0; i<MAX_VPU_BUFFER_POOL; i++)
    {
        if (vdi->vpu_buffer_pool[i].vdb.phys_addr.QuadPart == vb->phys_addr)
        {
            vdi->vpu_buffer_pool[i].inuse = 0;
            vdi->vpu_buffer_pool_count--;
            break;
        }
    }

    return 0;
}

void vdi_free_dma_memory(unsigned long coreIdx, vpu_buffer_t *vb)
{
    vdi_info_t *vdi;
    int i;
    int ret = 0;
    vpudrv_buffer_t vdb = {0};

    VLOG(INFO, "[VDI] %s, physaddr=0x%p, virtaddr=0x%p, size=%d\n", __FUNCTION__, vb->phys_addr, vb->virt_addr, vb->size);
#ifdef SUPPORT_MULTI_CORE_IN_ONE_DRIVER
    coreIdx = 0;
#endif
    vdi = &s_vdi_info[coreIdx];
    if(!vb || !vdi || vdi->vpu_fd==(HANDLE)-1 || vdi->vpu_fd == (HANDLE)0x00)
        return ;

    if (vb->size == 0)
        return ;

    for (i=0; i<MAX_VPU_BUFFER_POOL; i++)
    {
        if (vdi->vpu_buffer_pool[i].vdb.phys_addr.QuadPart == vb->phys_addr)
        {
            vdi->vpu_buffer_pool[i].inuse = 0;
            vdi->vpu_buffer_pool_count--;
            vdb = vdi->vpu_buffer_pool[i].vdb;
            break;
        }
    }

    if (!vdb.size)
    {
        VLOG(ERR, "[VDI] invalid buffer to free address = 0x%x\n", (int)vdb.virt_addr);
        return ;
    }
    vmem_lock(coreIdx);
    vmem_free(&vdi->pvip->vmem, (unsigned long)vdb.phys_addr.QuadPart, 0);
    if (!DeviceIoControl(vdi->vpu_fd, VDI_IOCTL_UNMAP_PHYSICALMEMORY, NULL, 0,
        &vdb, sizeof(vpudrv_buffer_t), (LPDWORD)&ret,  NULL))
    {		
        VLOG(ERR, "[VDI] fail to unmap physical memory virtual address = 0x%x\n", (int)vdb.virt_addr);
        vmem_unlock(coreIdx);
        return;
    }
    vmem_unlock(coreIdx);

    osal_memset(vb, 0, sizeof(vpu_buffer_t));
}

int vdi_get_sram_memory(unsigned long coreIdx, vpu_buffer_t *vb)
{
    vdi_info_t *vdi = &s_vdi_info[coreIdx];
    unsigned int sram_size = 0;

    if(!vb || !vdi || vdi->vpu_fd==(HANDLE)-1 || vdi->vpu_fd == (HANDLE)0x00)
        return -1;

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
        vb->phys_addr = VDI_SRAM_BASE_ADDR+(coreIdx*sram_size);		
        vb->size = sram_size;
        return 0;
    }

    return 0;
}

int vdi_set_clock_gate(unsigned long coreIdx, int enable)
{
    vdi_info_t *vdi = &s_vdi_info[coreIdx];
    int ret;
    if(!vdi || vdi->vpu_fd==(HANDLE)-1 || vdi->vpu_fd == (HANDLE)0x00)
        return -1;

    if (vdi->product_code == WAVE510_CODE || vdi->product_code == WAVE512_CODE || vdi->product_code == WAVE515_CODE || vdi->product_code == WAVE520_CODE) {
        return 0;
    }

    vdi->clock_state = enable;

    DeviceIoControl(vdi->vpu_fd, VDI_IOCTL_SET_CLOCK_GATE, &vdi->clock_state, 
        sizeof(unsigned long), NULL, 0, (LPDWORD)&ret, NULL);

    return 0;	
}

int vdi_get_clock_gate(unsigned long coreIdx)
{
    vdi_info_t *vdi = &s_vdi_info[coreIdx];

    if(!vdi || vdi->vpu_fd==(HANDLE)-1 || vdi->vpu_fd == (HANDLE)0x00)
        return -1;

    return vdi->clock_state;
}

int vdi_wait_bus_busy(unsigned long coreIdx, int timeout, unsigned int gdi_busy_flag)
{
    vdi_info_t *vdi = &s_vdi_info[coreIdx];

    LONGLONG  elapsed;
    LONGLONG  tick_per_sec;
    LONGLONG  tick_start;
    LONGLONG  tick_end;
    LARGE_INTEGER  li;

    if(!vdi || vdi->vpu_fd==(HANDLE)-1 || vdi->vpu_fd == (HANDLE)0x00)
        return -1;

    QueryPerformanceFrequency(&li);
    tick_per_sec = (li.QuadPart);

    QueryPerformanceCounter(&li);
    tick_start = li.QuadPart;

    while(1)
    {

        if (vdi->product_code == WAVE520_CODE) {
            if (vdi_fio_read_register(coreIdx, gdi_busy_flag) == 0x3f) break;
        }
        else if (PRODUCT_CODE_W_SERIES(vdi->product_code)) {
            if (vdi_fio_read_register(coreIdx, gdi_busy_flag) == 0x738) break;
        }
        else if (PRODUCT_CODE_NOT_W_SERIES(vdi->product_code)) {
            if (vdi_read_register(coreIdx, gdi_busy_flag) == 0x77) break;
        }
        else {
            VLOG(ERR, "Unknown product id : %08x\n", vdi->product_code);
            return -1;
        }
        if (tick_per_sec && timeout > 0)
        {
            QueryPerformanceCounter(&li);
            tick_end = li.QuadPart;
            elapsed = (LONGLONG)((tick_end - tick_start)/(tick_per_sec/1000.0));
            if (elapsed > timeout) {
                VLOG(ERR, "[VDI] vdi_wait_bus_busy timeout, PC=0x%x\n", vdi_read_register(coreIdx, 0x018));
                return -1;
            }
        }
    }
    return 0;

}

int vdi_wait_vpu_busy(unsigned long coreIdx, int timeout, unsigned int addr_bit_busy_flag)
{
    vdi_info_t *vdi = &s_vdi_info[coreIdx];

    LONGLONG  elapsed;
    LONGLONG  tick_per_sec;
    LONGLONG  tick_start;
    LONGLONG  tick_end;
    LARGE_INTEGER  li;
    Uint32    pc;
    Uint32    normalReg = TRUE;

    if(!vdi || vdi->vpu_fd==(HANDLE)-1 || vdi->vpu_fd == (HANDLE)0x00)
        return -1;

    if (PRODUCT_CODE_W_SERIES(vdi->product_code)) {
        pc = W4_VCPU_CUR_PC;
        if (addr_bit_busy_flag&0x8000) normalReg = FALSE;
    }
    else if (PRODUCT_CODE_NOT_W_SERIES(vdi->product_code)) {
        pc = BIT_CUR_PC;
    }
    else {
        VLOG(ERR, "Unknown product id : %08x\n", vdi->product_code);
        return -1;
    }

    QueryPerformanceFrequency(&li);
    tick_per_sec = (li.QuadPart);

    QueryPerformanceCounter(&li);
    tick_start = li.QuadPart;

    while(1)
    {
        if (normalReg == TRUE) {
            if (vdi_read_register(coreIdx, addr_bit_busy_flag) == 0) break;
        }
        else {
            if (vdi_fio_read_register(coreIdx, addr_bit_busy_flag) == 0) break;
        }
        if (tick_per_sec && timeout > 0)
        {
            QueryPerformanceCounter(&li);
            tick_end = li.QuadPart;
            elapsed = (LONGLONG)((tick_end - tick_start)/(tick_per_sec/1000.0));
            if (elapsed > timeout) {
                VLOG(ERR, "[VDI] vdi_wait_vpu_busy timeout, PC=0x%x\n", vdi_read_register(coreIdx, pc));
                return -1;
            }
        }
    }
    return 0;

}
int vdi_wait_interrupt(unsigned long coreIdx, int timeout, unsigned int addr_bit_int_reason)
{
    vdi_info_t *vdi = &s_vdi_info[coreIdx];
    int intr_reason;
#ifdef SUPPORT_INTERRUPT
#error "Need to modify windows device driver"
    if(!vdi || vdi->vpu_fd==(HANDLE)-1 || vdi->vpu_fd == (HANDLE)0x00)
        return -1;

    DeviceIoControl(vdi->vpu_fd, VDI_IOCTL_WAIT_INTERRUPT, &timeout, sizeof(unsigned long),
        NULL, 0, NULL, NULL);
#else
    LONGLONG  elapsed;
    LONGLONG  tick_per_sec;
    LONGLONG  tick_start;
    LONGLONG  tick_end;
    LARGE_INTEGER  li;
    int int_sts_reg;

    if(!vdi || vdi->vpu_fd==(HANDLE)-1 || vdi->vpu_fd == (HANDLE)0x00)
        return -1;

    if (PRODUCT_CODE_W_SERIES(vdi->product_code)) {
        int_sts_reg = W4_VPU_VPU_INT_STS;
    }
    else if (PRODUCT_CODE_NOT_W_SERIES(vdi->product_code)) {
        int_sts_reg = BIT_INT_STS;
    }
    else {
        VLOG(ERR, "Unknown product id : %08x\n", vdi->product_code);
        return -1;
    }

    QueryPerformanceFrequency(&li);
    tick_per_sec = (li.QuadPart);

    QueryPerformanceCounter(&li);

    tick_start = li.QuadPart;

    while(1)
    {
        //if (coreIdx == 1)
        //	VLOG(INFO, "coreIdx=%d, reason=0x%x PC=0x%x, CMD=0x%x\n", coreIdx, vdi_read_register(coreIdx, 0x174), vdi_read_register(coreIdx, 0x018), vdi_read_register(coreIdx, 0x164));
        if (vdi_read_register(coreIdx, int_sts_reg))
        {
            if ((intr_reason=vdi_read_register(coreIdx, addr_bit_int_reason)))
                break;
        }
        if (timeout > 0 && tick_per_sec)
        {
            QueryPerformanceCounter(&li);
            tick_end = li.QuadPart;
            elapsed = (LONGLONG)((tick_end - tick_start)/(tick_per_sec/1000.0));
            if (elapsed > timeout) {
                return -1;
            }
        }
    }
#endif
    return intr_reason;
}

static int read_pinfo_buffer(int coreIdx, int addr)
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
    vdi_write_register(coreIdx, VDI_LOG_GDI_PINFO_ADDR, addr);
    vdi_write_register(coreIdx, VDI_LOG_GDI_PINFO_REQ, 1);

    ack = 0;
    while (ack == 0)
    {
        ack = vdi_read_register(coreIdx, VDI_LOG_GDI_PINFO_ACK);
    }

    rdata = vdi_read_register(coreIdx, VDI_LOG_GDI_PINFO_DATA);

    //VLOG(INFO, "[READ PINFO] ADDR[%x], DATA[%x]", addr, rdata);
    return rdata;
}

enum {
    VDI_PRODUCT_ID_980,
    VDI_PRODUCT_ID_960
};

static void printf_gdi_info(int coreIdx, int num, int reset)
{
    int i;
    int bus_info_addr;
    int tmp;	
    int val;
    int productId;

    val = vdi_read_register(coreIdx, VPU_PRODUCT_CODE_REGISTER);
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
            vdi_write_register(coreIdx, bus_info_addr, 0x00);
            bus_info_addr += 4;
            vdi_write_register(coreIdx, bus_info_addr, 0x00);
            bus_info_addr += 4;
            vdi_write_register(coreIdx, bus_info_addr, 0x00);
            bus_info_addr += 4;
            vdi_write_register(coreIdx, bus_info_addr, 0x00);
            bus_info_addr += 4;
            vdi_write_register(coreIdx, bus_info_addr, 0x00);

            if (productId == VDI_PRODUCT_ID_980)
            {
                bus_info_addr += 4;
                vdi_write_register(coreIdx, bus_info_addr, 0x00);

                bus_info_addr += 4;
                vdi_write_register(coreIdx, bus_info_addr, 0x00);

                bus_info_addr += 4;
                vdi_write_register(coreIdx, bus_info_addr, 0x00);
            }

        }
        else 
        {
            VLOG(INFO, "index = %02d", i);

            tmp = read_pinfo_buffer(coreIdx, bus_info_addr);	//TiledEn<<20 ,GdiFormat<<17,IntlvCbCr,<<16 GdiYuvBufStride
            VLOG(INFO, " control = 0x%08x", tmp);

            bus_info_addr += 4;
            tmp = read_pinfo_buffer(coreIdx, bus_info_addr);
            VLOG(INFO, " pic_size = 0x%08x", tmp);

            bus_info_addr += 4;
            tmp = read_pinfo_buffer(coreIdx, bus_info_addr);
            VLOG(INFO, " y-top = 0x%08x", tmp);

            bus_info_addr += 4;
            tmp = read_pinfo_buffer(coreIdx, bus_info_addr);
            VLOG(INFO, " cb-top = 0x%08x", tmp);

            bus_info_addr += 4;
            tmp = read_pinfo_buffer(coreIdx, bus_info_addr);
            VLOG(INFO, " cr-top = 0x%08x", tmp);
            if (productId == VDI_PRODUCT_ID_980)
            {
                bus_info_addr += 4;
                tmp = read_pinfo_buffer(coreIdx, bus_info_addr);
                VLOG(INFO, " y-bot = 0x%08x", tmp);

                bus_info_addr += 4;
                tmp = read_pinfo_buffer(coreIdx, bus_info_addr);
                VLOG(INFO, " cb-bot = 0x%08x", tmp);

                bus_info_addr += 4;
                tmp = read_pinfo_buffer(coreIdx, bus_info_addr);
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
	
    if (PRODUCT_CODE_W_SERIES(product_code)) {
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

static void vdi_make_log(unsigned long coreIdx, const char *str, int step)
{
	UINT32 val;

	val = VpuReadReg(coreIdx, W4_INST_INDEX);
	val &= 0xffff;
	if (step == 1)
		VLOG(INFO, "\n**%s start(%d)\n", str, val);
	else if (step == 2)	// 
		VLOG(INFO, "\n**%s timeout(%d)\n", str, val);		
	else
		VLOG(INFO, "\n**%s end(%d)\n", str, val);		
}

void vdi_log(unsigned long coreIdx, int cmd, int step)
{
    vdi_info_t *vdi = &s_vdi_info[coreIdx];
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

    if(!vdi || vdi->vpu_fd==(HANDLE)-1 || vdi->vpu_fd == (HANDLE)0x00)
        return ;

    if (PRODUCT_CODE_W_SERIES(vdi->product_code))
    {
        switch(cmd)
        {
         case INIT_VPU:
             vdi_make_log(coreIdx, "INIT_VPU", step);			 
             break;
         case DEC_PIC_HDR: //SET_PARAM for ENC
             vdi_make_log(coreIdx, "SET_PARAM(ENC), DEC_PIC_HDR(DEC)", step);
             break;
         case FINI_SEQ:
             vdi_make_log(coreIdx, "FINI_SEQ", step);
             break;
         case DEC_PIC://ENC_PIC for ENC
             vdi_make_log(coreIdx, "DEC_PIC, ENC_PIC", step);
             break;
         case SET_FRAMEBUF:
             vdi_make_log(coreIdx, "SET_FRAMEBUF", step);
             break;
         case FLUSH_DECODER:
             vdi_make_log(coreIdx, "FLUSH_DECODER", step);
             break;
         case GET_FW_VERSION:
             vdi_make_log(coreIdx, "GET_FW_VERSION", step);
             break;
         case QUERY_DECODER:
             vdi_make_log(coreIdx, "QUERY_DECODER", step);
             break;
         case SLEEP_VPU:
             vdi_make_log(coreIdx, "SLEEP_VPU", step);
             break;
         case CREATE_INSTANCE:
             vdi_make_log(coreIdx, "CREATE_INSTANCE", step);			 
             break;
		 case RESET_VPU:
			 vdi_make_log(coreIdx, "RESET_VPU", step);
			 break;			 
         default:
             vdi_make_log(coreIdx, "ANY_CMD", step);
             break;
        }
    }
    else if (PRODUCT_CODE_NOT_W_SERIES(vdi->product_code))
    {
        switch(cmd)
        {
        case SEQ_INIT:
             vdi_make_log(coreIdx, "SEQ_INIT", step);	
            break;
        case SEQ_END:
             vdi_make_log(coreIdx, "SEQ_END", step);		
            break;
        case PIC_RUN:
             vdi_make_log(coreIdx, "PIC_RUN", step);	
            break;
        case SET_FRAME_BUF:
             vdi_make_log(coreIdx, "SET_FRAME_BUF", step);	
            break;
        case ENCODE_HEADER:
             vdi_make_log(coreIdx, "ENCODE_HEADER", step);	
            break;
        case RC_CHANGE_PARAMETER:
             vdi_make_log(coreIdx, "RC_CHANGE_PARAMETER", step);	
            break;
        case DEC_BUF_FLUSH:
             vdi_make_log(coreIdx, "DEC_BUF_FLUSH", step);	
            break;
        case FIRMWARE_GET:
             vdi_make_log(coreIdx, "FIRMWARE_GET", step);	
            break;
        case VPU_RESET:
             vdi_make_log(coreIdx, "VPU_RESET", step);
            break;
        case ENC_PARA_SET:
             vdi_make_log(coreIdx, "ENC_PARA_SET", step);
            break;
        case DEC_PARA_SET:
             vdi_make_log(coreIdx, "DEC_PARA_SET", step);
            break;
        default:
             vdi_make_log(coreIdx, "ANY_CMD", step);
            break;
        }
    }
    else {
        VLOG(ERR, "Unknown product id : %08x\n", vdi->product_code);
        return;
    }

    for (i=0; i<0x200; i=i+16) {
        VLOG(INFO, "0x%04xh: 0x%08x 0x%08x 0x%08x 0x%08x\n", i,
            vdi_read_register(coreIdx, i), vdi_read_register(coreIdx, i+4),
            vdi_read_register(coreIdx, i+8), vdi_read_register(coreIdx, i+0xc));
    }
	   
    if (PRODUCT_CODE_W_SERIES(vdi->product_code)) {
		if (cmd == INIT_VPU || cmd == VPU_RESET || cmd == CREATE_INSTANCE)
		{
			vdi_print_vpu_status(coreIdx);
		}
    }
    else if (PRODUCT_CODE_NOT_W_SERIES(vdi->product_code)) {
        //if ((cmd == PIC_RUN && step== 0) || cmd == VPU_RESET)
		if (cmd == VPU_RESET)
        {
            printf_gdi_info(coreIdx, 32, 0);

#define VDI_LOG_MBC_BUSY 0x0440
#define VDI_LOG_MC_BASE	 0x0C00
#define VDI_LOG_MC_BUSY	 0x0C04
#define VDI_LOG_GDI_BUS_STATUS (0x10F4)
#define VDI_LOG_ROT_SRC_IDX	 (0x400 + 0x10C)
#define VDI_LOG_ROT_DST_IDX	 (0x400 + 0x110)

            VLOG(INFO, "MBC_BUSY = %x\n", vdi_read_register(coreIdx, VDI_LOG_MBC_BUSY));
            VLOG(INFO, "MC_BUSY = %x\n", vdi_read_register(coreIdx, VDI_LOG_MC_BUSY));
            VLOG(INFO, "MC_MB_XY_DONE=(y:%d, x:%d)\n", (vdi_read_register(coreIdx, VDI_LOG_MC_BASE) >> 20) & 0x3F, (vdi_read_register(coreIdx, VDI_LOG_MC_BASE) >> 26) & 0x3F);
            VLOG(INFO, "GDI_BUS_STATUS = %x\n", vdi_read_register(coreIdx, VDI_LOG_GDI_BUS_STATUS));

            VLOG(INFO, "ROT_SRC_IDX = %x\n", vdi_read_register(coreIdx, VDI_LOG_ROT_SRC_IDX));
            VLOG(INFO, "ROT_DST_IDX = %x\n", vdi_read_register(coreIdx, VDI_LOG_ROT_DST_IDX));

            VLOG(INFO, "P_MC_PIC_INDEX_0 = %x\n", vdi_read_register(coreIdx, MC_BASE+0x200));
            VLOG(INFO, "P_MC_PIC_INDEX_1 = %x\n", vdi_read_register(coreIdx, MC_BASE+0x20c));
            VLOG(INFO, "P_MC_PIC_INDEX_2 = %x\n", vdi_read_register(coreIdx, MC_BASE+0x218));
            VLOG(INFO, "P_MC_PIC_INDEX_3 = %x\n", vdi_read_register(coreIdx, MC_BASE+0x230));
            VLOG(INFO, "P_MC_PIC_INDEX_3 = %x\n", vdi_read_register(coreIdx, MC_BASE+0x23C));
            VLOG(INFO, "P_MC_PIC_INDEX_4 = %x\n", vdi_read_register(coreIdx, MC_BASE+0x248));
            VLOG(INFO, "P_MC_PIC_INDEX_5 = %x\n", vdi_read_register(coreIdx, MC_BASE+0x254));
            VLOG(INFO, "P_MC_PIC_INDEX_6 = %x\n", vdi_read_register(coreIdx, MC_BASE+0x260));
            VLOG(INFO, "P_MC_PIC_INDEX_7 = %x\n", vdi_read_register(coreIdx, MC_BASE+0x26C));
            VLOG(INFO, "P_MC_PIC_INDEX_8 = %x\n", vdi_read_register(coreIdx, MC_BASE+0x278));
            VLOG(INFO, "P_MC_PIC_INDEX_9 = %x\n", vdi_read_register(coreIdx, MC_BASE+0x284));
            VLOG(INFO, "P_MC_PIC_INDEX_a = %x\n", vdi_read_register(coreIdx, MC_BASE+0x290));
            VLOG(INFO, "P_MC_PIC_INDEX_b = %x\n", vdi_read_register(coreIdx, MC_BASE+0x29C));
            VLOG(INFO, "P_MC_PIC_INDEX_c = %x\n", vdi_read_register(coreIdx, MC_BASE+0x2A8));
            VLOG(INFO, "P_MC_PIC_INDEX_d = %x\n", vdi_read_register(coreIdx, MC_BASE+0x2B4));

            VLOG(INFO, "P_MC_PICIDX_0 = %x\n", vdi_read_register(coreIdx, MC_BASE+0x028));
            VLOG(INFO, "P_MC_PICIDX_1 = %x\n", vdi_read_register(coreIdx, MC_BASE+0x02C));
        }
    }
    else {
        VLOG(ERR, "Unknown product id : %08x\n", vdi->product_code);
        return;
    }
}

//static void swap_bytes(int swap_len, unsigned char* data, int len)
//{
//    int count;
//    int i, j;
//    unsigned char  temp[128];
//    unsigned char* p = data;
//
//    count = len / swap_len;
//
//    for (i=0; i<count; i++) {
//        osal_memcpy(temp, p, swap_len);
//        for (j=(swap_len-1); j>=0; j--) {
//            p[swap_len-1-j] = temp[j];
//        }
//        p += swap_len;
//    }
//}

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

int vdi_get_system_endian(unsigned long coreIdx)
{
	vdi_info_t *vdi = &s_vdi_info[coreIdx];

	if(!vdi || vdi->vpu_fd==(HANDLE)-1 || vdi->vpu_fd == (HANDLE)0x00)
		return -1;

    if (PRODUCT_CODE_W_SERIES(vdi->product_code)) {
        return VDI_128BIT_BUS_SYSTEM_ENDIAN;
    }
    else if (PRODUCT_CODE_NOT_W_SERIES(vdi->product_code)) {
        return VDI_SYSTEM_ENDIAN;
    }
    else {
        VLOG(ERR, "Unknown product id : %08x\n", vdi->product_code);
        return -1;
    }
}

int vdi_convert_endian(unsigned long coreIdx, unsigned int endian)
{
	vdi_info_t *vdi = &s_vdi_info[coreIdx];

	if(!vdi || vdi->vpu_fd==(HANDLE)-1 || vdi->vpu_fd == (HANDLE)0x00)
		return -1;
	
    if (PRODUCT_CODE_W_SERIES(vdi->product_code)) {
        switch (endian) {
        case VDI_LITTLE_ENDIAN:       endian = 0x00; break;
        case VDI_BIG_ENDIAN:          endian = 0x0f; break;
        case VDI_32BIT_LITTLE_ENDIAN: endian = 0x04; break;
        case VDI_32BIT_BIG_ENDIAN:    endian = 0x03; break;
        }
    }
    else if (PRODUCT_CODE_NOT_W_SERIES(vdi->product_code)) {
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


int swap_endian(unsigned long coreIdx,unsigned char *data, int len, int endian)
{
    vdi_info_t *vdi = &s_vdi_info[coreIdx];
    int changes;
    int sys_endian;
    BOOL byteChange, wordChange, dwordChange, lwordChange;

    if(!vdi || vdi->vpu_fd==(HANDLE)-1 || vdi->vpu_fd == (HANDLE)0x00)
        return -1;

    if (PRODUCT_CODE_W_SERIES(vdi->product_code)) {
        sys_endian = VDI_128BIT_BUS_SYSTEM_ENDIAN;
    }
    else if (PRODUCT_CODE_NOT_W_SERIES(vdi->product_code)) {
        sys_endian = VDI_SYSTEM_ENDIAN;
    }
    else {
        VLOG(ERR, "Unknown product id : %08x\n", vdi->product_code);
        return -1;
    }

    endian     = vdi_convert_endian(coreIdx, endian);
    sys_endian = vdi_convert_endian(coreIdx, sys_endian);
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

int GetDevicePath(unsigned long coreIdx)
{
    vdi_info_t *vdi = &s_vdi_info[coreIdx];
    SP_DEVICE_INTERFACE_DATA DeviceInterfaceData;
    SP_DEVINFO_DATA DeviceInfoData;
    ULONG size;
    int count, i, index;
    BOOL status = TRUE;
    TCHAR *DeviceName = NULL;
    TCHAR *DeviceLocation = NULL;

    //
    //  Retrieve the device information for all PLX devices.
    //
    vdi->hDevInfo = SetupDiGetClassDevs(&GUID_DEVINTERFACE_vpudrv,
        NULL,
        NULL,
        DIGCF_DEVICEINTERFACE |
        DIGCF_PRESENT);

    //
    //  Initialize the SP_DEVICE_INTERFACE_DATA Structure.
    //
    DeviceInterfaceData.cbSize  = sizeof(SP_DEVICE_INTERFACE_DATA);

    //
    //  Determine how many devices are present.
    //
    count = 0;
    while(SetupDiEnumDeviceInterfaces(vdi->hDevInfo,
        NULL,
        &GUID_DEVINTERFACE_vpudrv,
        count++,  //Cycle through the available devices.
        &DeviceInterfaceData)
        );

#ifdef SUPPORT_MULTI_CORE_IN_ONE_DRIVER
#else
    if (coreIdx+1 >= (unsigned long)count)
        return 0;
#endif
    //
    // Since the last call fails when all devices have been enumerated,
    // decrement the count to get the true device count.
    //
    count--;

    //
    //  If the count is zero then there are no devices present.
    //
    if (count == 0) {
        VLOG(INFO, "No VPU devices are present and enabled in the system.\n");
        return FALSE;
    }

    //
    //  Initialize the appropriate data structures in preparation for
    //  the SetupDi calls.
    //
    DeviceInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
    DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

    //
    //  Loop through the device list to allow user to choose
    //  a device.  If there is only one device, select it
    //  by default.
    //
    i = 0;
    while (SetupDiEnumDeviceInterfaces(vdi->hDevInfo,
        NULL,
        (LPGUID)&GUID_DEVINTERFACE_vpudrv,
        i,
        &DeviceInterfaceData)) 
    {

        //
        // Determine the size required for the DeviceInterfaceData
        //
        SetupDiGetDeviceInterfaceDetail(vdi->hDevInfo,
            &DeviceInterfaceData,
            NULL,
            0,
            &size,
            NULL);

        if (GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
            VLOG(INFO, "SetupDiGetDeviceInterfaceDetail failed, Error: %d", GetLastError());
            return FALSE;
        }

        vdi->pDeviceInterfaceDetail = (PSP_DEVICE_INTERFACE_DETAIL_DATA) osal_malloc(size);

        if (!vdi->pDeviceInterfaceDetail) {
            VLOG(INFO, "Insufficient memory.\n");
            return FALSE;
        }

        //
        // Initialize structure and retrieve data.
        //
        vdi->pDeviceInterfaceDetail->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
        status = SetupDiGetDeviceInterfaceDetail(vdi->hDevInfo,
            &DeviceInterfaceData,
            vdi->pDeviceInterfaceDetail,
            size,
            NULL,
            &DeviceInfoData);

        free(vdi->pDeviceInterfaceDetail);

        if (!status) {
            VLOG(INFO, "SetupDiGetDeviceInterfaceDetail failed, Error: %d", GetLastError());
            return status;
        }

        //
        //  Get the Device Name
        //  Calls to SetupDiGetDeviceRegistryProperty require two consecutive
        //  calls, first to get required buffer size and second to get
        //  the data.
        //
        SetupDiGetDeviceRegistryProperty(vdi->hDevInfo,
            &DeviceInfoData,
            SPDRP_DEVICEDESC,
            NULL,
            (PBYTE)DeviceName,
            0,
            &size);

        if (GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
            VLOG(INFO, "SetupDiGetDeviceRegistryProperty failed, Error: %d", GetLastError());
            return FALSE;
        }

        DeviceName = (TCHAR*) osal_malloc(size);
        if (!DeviceName) {
            VLOG(INFO, "Insufficient memory.\n");
            return FALSE;
        }

        status = SetupDiGetDeviceRegistryProperty(vdi->hDevInfo,
            &DeviceInfoData,
            SPDRP_DEVICEDESC,
            NULL,
            (PBYTE)DeviceName,
            size,
            NULL);
        if (!status) {
            VLOG(INFO, "SetupDiGetDeviceRegistryProperty failed, Error: %d",
                GetLastError());
            free(DeviceName);
            return status;
        }

        //
        //  Now retrieve the Device Location.
        //
        SetupDiGetDeviceRegistryProperty(vdi->hDevInfo,
            &DeviceInfoData,
            SPDRP_LOCATION_INFORMATION,
            NULL,
            (PBYTE)DeviceLocation,
            0,
            &size);

        if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
            DeviceLocation = (TCHAR*) osal_malloc(size);

            if (DeviceLocation != NULL) {

                status = SetupDiGetDeviceRegistryProperty(vdi->hDevInfo,
                    &DeviceInfoData,
                    SPDRP_LOCATION_INFORMATION,
                    NULL,
                    (PBYTE)DeviceLocation,
                    size,
                    NULL);
                if (!status) {
                    free(DeviceLocation);
                    DeviceLocation = NULL;
                }
            }

        } else {
            DeviceLocation = NULL;
        }

        //
        // If there is more than one device print description.
        //
        if (count > 1) {
            VLOG(INFO, "%d- ", i);
        }

        VLOG(INFO, "%s\n", DeviceName);

        if (DeviceLocation) {
            VLOG(INFO, "        %s\n", DeviceLocation);
        }

        free(DeviceName);
        DeviceName = NULL;

        if (DeviceLocation) {
            free(DeviceLocation);
            DeviceLocation = NULL;
        }

        i++; // Cycle through the available devices.
    }

    //
    //  Select device.
    //
    index = 0;
    if (count > 1) {
        index = coreIdx;
    }

    //
    //  Get information for specific device.
    //
    status = SetupDiEnumDeviceInterfaces(vdi->hDevInfo,
        NULL,
        (LPGUID)&GUID_DEVINTERFACE_vpudrv,
        index,
        &DeviceInterfaceData);

    if (!status) {
        VLOG(INFO, "SetupDiEnumDeviceInterfaces failed, Error: %d", GetLastError());
        return status;
    }

    //
    // Determine the size required for the DeviceInterfaceData
    //
    SetupDiGetDeviceInterfaceDetail(vdi->hDevInfo,
        &DeviceInterfaceData,
        NULL,
        0,
        &size,
        NULL);

    if (GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
        VLOG(INFO, "SetupDiGetDeviceInterfaceDetail failed, Error: %d", GetLastError());
        return FALSE;
    }

    vdi->pDeviceInterfaceDetail = (PSP_DEVICE_INTERFACE_DETAIL_DATA) osal_malloc(size);

    if (!vdi->pDeviceInterfaceDetail) {
        VLOG(INFO, "Insufficient memory.\n");
        return FALSE;
    }

    //
    // Initialize structure and retrieve data.
    //
    vdi->pDeviceInterfaceDetail->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

    status = SetupDiGetDeviceInterfaceDetail(vdi->hDevInfo,
        &DeviceInterfaceData,
        vdi->pDeviceInterfaceDetail,
        size,
        NULL,
        &DeviceInfoData);
    if (!status) {
        VLOG(INFO, "SetupDiGetDeviceInterfaceDetail failed, Error: %d", GetLastError());
        return status;
    }

    return status;
}

void vdi_set_sdram(unsigned long coreIdx, unsigned int addr, int len, unsigned char data, int endian)
{
}


#endif
 
