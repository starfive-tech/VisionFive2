/*++

Module Name:

    device.h

Abstract:

    This file contains the device definitions.

Environment:

    Kernel-mode Driver Framework

--*/

#include "public.h"

//-----------------------------------------------------------------------------   
// PCI Device/Vendor Ids.
//-----------------------------------------------------------------------------   
#define HPI_PCI_VENDOR_ID               0xCEDA
#define HPI_PCI_DEVICE_ID               0x4311
#define HPI_REGISTER_SIZE				0x2000
#define HPI_ADDR_CMD				(0x00<<2)
#define HPI_ADDR_STATUS				(0x01<<2)
#define HPI_ADDR_ADDR_H				(0x02<<2)
#define HPI_ADDR_ADDR_L				(0x03<<2)
#define HPI_ADDR_ADDR_M				(0x06<<2)
#define HPI_ADDR_DATA				(0x80<<2)
#define HPI_CMD_READ_VALUE			0x0081
#define HPI_CMD_WRITE_VALUE			0x0082

//
// The device context performs the same job as
// a WDM device extension in the driver frameworks
//
typedef struct _DEVICE_CONTEXT
{
	WDFDEVICE Device;
	PVOID RegsBase;         // Registers base address
	ULONG RegsLength;       // Registers base length
	PHYSICAL_ADDRESS regsBasePA;
	WDFINTERRUPT Interrupt;     // Returned by InterruptCreate

	WDFWAITLOCK InterruptWaitlock;
	WDFSPINLOCK HPILock;

	vpudrv_buffer_t instance_pool;
	vpudrv_buffer_t reserved_video_memory;
	vpudrv_buffer_t sram_memory;
	WDFMEMORY InstancePoolBuffer;

	ULONG PrivateDeviceData;  // just a placeholder

} DEVICE_CONTEXT, *PDEVICE_CONTEXT;

//
// This macro will generate an inline function called DeviceGetContext
// which will be used to get a pointer to the device context memory
// in a type safe manner.
//
WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(DEVICE_CONTEXT, DeviceGetContext)

//
// Function to initialize the device and its callbacks
//
NTSTATUS
vpudrvCreateDevice(
    _Inout_ PWDFDEVICE_INIT DeviceInit
    );

//
// Performs whatever initialization is needed to setup the device, setting up
// a DMA channel or mapping any I/O port resources.  This will only be called
// as a device starts or restarts, not every time the device moves into the D0
// state.  Consequently, most hardware initialization belongs elsewhere.
NTSTATUS
	vpudrvPrepareDevice(
	_In_ WDFDEVICE Device,
	_In_ WDFCMRESLIST ResourcesTranslated
	);
VOID 
	vpudrvReleaseDevice(
	_In_ WDFDEVICE Device
	);
VOID
	vpudrvShutdownDevice(
	_In_ WDFDEVICE Device
	);
NTSTATUS
	vpudrvCreateReservedMemory(
	_In_ WDFDEVICE Device
	);
NTSTATUS 
	vpudrvInterruptCreate(
	_In_ WDFDEVICE Device
	);

EVT_WDF_INTERRUPT_ISR vpudrvEvtInterruptIsr;
EVT_WDF_INTERRUPT_DPC vpudrvEvtInterruptDpc;
EVT_WDF_INTERRUPT_ENABLE vpudrvEvtInterruptEnable;
EVT_WDF_INTERRUPT_DISABLE vpudrvEvtInterruptDisable; 
