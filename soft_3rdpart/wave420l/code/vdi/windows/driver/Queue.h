/*++

Module Name:

    queue.h

Abstract:

    This file contains the queue definitions.

Environment:

    Kernel-mode Driver Framework

--*/

//
// This is the context that can be placed per queue
// and would contain per queue information.
//
typedef struct _QUEUE_CONTEXT {

    ULONG PrivateDeviceData;  // just a placeholder

} QUEUE_CONTEXT, *PQUEUE_CONTEXT;

WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(QUEUE_CONTEXT, QueueGetContext)

NTSTATUS
vpudrvQueueInitialize(
    _In_ WDFDEVICE hDevice
    );

NTSTATUS
	vpudrvHardwareReset(
	_In_ WDFDEVICE hDevice,
	_In_ WDFREQUEST Request
	);

NTSTATUS
	vpudrvCreateInitMemory(
	_In_ WDFDEVICE hDevice
	);

NTSTATUS 
	vpudrvMapUserMemory(
	_In_ WDFDEVICE hDevice,
	_In_ WDFREQUEST Request, 
	_In_ size_t OutputBufferSize, 
	_In_ BOOLEAN bMap
	);

NTSTATUS 
	vpudrvWaitInterrupt(
	_In_ WDFDEVICE hDevice,
	_In_ WDFREQUEST Request, 
	_In_ size_t InputBufferSize
	);

NTSTATUS 
	vpudrvClockGate(
	_In_ WDFDEVICE hDevice,
	_In_ WDFREQUEST Request, 
	_In_ size_t InputBufferSize
	);

NTSTATUS 
	vpudrvGetInstancePool(
	_In_ WDFDEVICE hDevice,
	_In_ WDFREQUEST Request, 
	_In_ size_t OutputBufferSize
	);

NTSTATUS 
	vpudrvGetReservedVideoMemoryInfo(
	_In_ WDFDEVICE hDevice,
	_In_ WDFREQUEST Request, 
	_In_ size_t OutputBufferSize
	);

NTSTATUS
	vpudrvReadWriteRegister(
	_In_ WDFDEVICE hDevice,
	_In_ WDFREQUEST Request, 
	_In_ size_t OutputBufferSize
	);

//
// Events from the IoQueue object
//
EVT_WDF_IO_QUEUE_IO_DEVICE_CONTROL vpudrvEvtIoDeviceControl;
EVT_WDF_IO_QUEUE_IO_STOP vpudrvEvtIoStop;
EVT_WDF_IO_QUEUE_IO_READ vpudrvEvtIoRead;
EVT_WDF_IO_QUEUE_IO_WRITE vpudrvEvtIoWrite;

