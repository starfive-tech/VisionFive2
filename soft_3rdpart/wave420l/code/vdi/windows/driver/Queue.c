/*++

Module Name:

    queue.c

Abstract:

    This file contains the queue entry points and callbacks.

Environment:

    Kernel-mode Driver Framework

--*/

#include "driver.h"
#include "queue.tmh"

#ifdef ALLOC_PRAGMA
#pragma alloc_text (PAGE, vpudrvQueueInitialize)
#pragma alloc_text (PAGE, vpudrvMapUserMemory)
#pragma alloc_text (PAGE, vpudrvWaitInterrupt)
#pragma alloc_text (PAGE, vpudrvGetInstancePool)
#pragma alloc_text (PAGE, vpudrvClockGate)
#pragma alloc_text (PAGE, vpudrvReadWriteRegister)

#endif

NTSTATUS 
	MapMemory(
	_In_ WDFDEVICE hDevice,
	_Inout_ vpudrv_buffer_t *vb,
	_In_ MEMORY_CACHING_TYPE cacheMode
	);

NTSTATUS
	UnMapMemory(
	_In_ WDFDEVICE hDevice,
	_Inout_ vpudrv_buffer_t *vb
	);

ULONG 
	ReadVpuRegister(
	_In_ WDFDEVICE hDevice,
	_In_ ULONG Addr
	);

VOID 
	WriteVpuRegister(
	_In_ WDFDEVICE hDevice,
	_In_ ULONG Addr, 
	_In_ ULONG Data
	);

NTSTATUS
vpudrvQueueInitialize(
    _In_ WDFDEVICE Device
    )
/*++

Routine Description:

     The I/O dispatch callbacks for the frameworks device object
     are configured in this function.

     A single default I/O Queue is configured for parallel request
     processing, and a driver context memory allocation is created
     to hold our structure QUEUE_CONTEXT.

Arguments:

    Device - Handle to a framework device object.

Return Value:

    VOID

--*/
{
    WDFQUEUE queue;
    NTSTATUS status;
    WDF_IO_QUEUE_CONFIG    queueConfig;
	WDF_OBJECT_ATTRIBUTES attributes;
	PDEVICE_CONTEXT deviceContext;

    PAGED_CODE();
    
	deviceContext = DeviceGetContext(Device);

    //
    // Configure a default queue so that requests that are not
    // configure-fowarded using WdfDeviceConfigureRequestDispatching to goto
    // other queues get dispatched here.
    //
    WDF_IO_QUEUE_CONFIG_INIT_DEFAULT_QUEUE(
         &queueConfig,
        WdfIoQueueDispatchParallel
        );

    queueConfig.EvtIoDeviceControl = vpudrvEvtIoDeviceControl;
	queueConfig.EvtIoRead = vpudrvEvtIoRead;
	queueConfig.EvtIoWrite = vpudrvEvtIoWrite;
    queueConfig.EvtIoStop = vpudrvEvtIoStop;

    status = WdfIoQueueCreate(
                 Device,
                 &queueConfig,
                 WDF_NO_OBJECT_ATTRIBUTES,
                 &queue
                 );

    if( !NT_SUCCESS(status) ) {
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_QUEUE, "%!FUNC! WdfIoQueueCreate failed %!STATUS!", status);
        return status;
    }

	WDF_OBJECT_ATTRIBUTES_INIT (&attributes);
	attributes.ParentObject = Device;

	status = WdfSpinLockCreate (&attributes, &deviceContext->HPILock);
	if (!NT_SUCCESS(status)) {
		return status;
	}
	
    return status;
}
VOID
	vpudrvEvtIoRead(
	_In_ WDFQUEUE Queue,
	_In_ WDFREQUEST Request,
	_In_ size_t Length
	)
{
	UNREFERENCED_PARAMETER(Queue);
	UNREFERENCED_PARAMETER(Length);

	TraceEvents(TRACE_LEVEL_INFORMATION, 
		TRACE_QUEUE, 
		"!FUNC! Queue 0x%p, Request 0x%p Length %d ", 
		Queue, Request, (int) Length);

	WdfRequestComplete(Request, STATUS_SUCCESS);
}
VOID
	vpudrvEvtIoWrite(
	_In_ WDFQUEUE Queue,
	_In_ WDFREQUEST Request,
	_In_ size_t Length
	)
{
	UNREFERENCED_PARAMETER(Queue);
	UNREFERENCED_PARAMETER(Length);

	TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_QUEUE, "!FUNC! Queue 0x%p, Request 0x%p Length %d ", Queue, Request, (int) Length);

	WdfRequestComplete(Request, STATUS_SUCCESS);
}
VOID
vpudrvEvtIoDeviceControl(
    _In_ WDFQUEUE Queue,
    _In_ WDFREQUEST Request,
    _In_ size_t OutputBufferLength,
    _In_ size_t InputBufferLength,
    _In_ ULONG IoControlCode
    )
/*++

Routine Description:

    This event is invoked when the framework receives IRP_MJ_DEVICE_CONTROL request.

Arguments:

    Queue -  Handle to the framework queue object that is associated with the
             I/O request.

    Request - Handle to a framework request object.

    OutputBufferLength - Size of the output buffer in bytes

    InputBufferLength - Size of the input buffer in bytes

    IoControlCode - I/O control code.

Return Value:

    VOID

--*/
{
	WDFDEVICE hDevice;
	PDEVICE_CONTEXT deviceContext;
	NTSTATUS status = STATUS_UNSUCCESSFUL;

    TraceEvents(TRACE_LEVEL_INFORMATION, 
                TRACE_QUEUE, 
                "%!FUNC! Queue 0x%p, Request 0x%p OutputBufferLength %d InputBufferLength %d IoControlCode %d", 
                Queue, Request, (int) OutputBufferLength, (int) InputBufferLength, (int)IoControlCode);

	hDevice = WdfIoQueueGetDevice(Queue);
	deviceContext = DeviceGetContext(hDevice);

	UNREFERENCED_PARAMETER(OutputBufferLength);
	UNREFERENCED_PARAMETER(InputBufferLength);

	switch(IoControlCode){
	
	case VDI_IOCTL_GET_RESERVED_VIDEO_MEMORY_INFO:
		status = vpudrvGetReservedVideoMemoryInfo(hDevice, Request, OutputBufferLength);
		break;
	case VDI_IOCTL_MAP_PHYSICAL_MEMORY:
		status = vpudrvMapUserMemory(hDevice, Request, OutputBufferLength, TRUE);
		break;
	case VDI_IOCTL_UNMAP_PHYSICALMEMORY:
		status = vpudrvMapUserMemory(hDevice, Request, OutputBufferLength, FALSE);
		break;			
	case VDI_IOCTL_WAIT_INTERRUPT:
		status = vpudrvWaitInterrupt(hDevice, Request, InputBufferLength);
		break;
	case VDI_IOCTL_SET_CLOCK_GATE:
		status = vpudrvClockGate(hDevice, Request, InputBufferLength);
		break;
	case VDI_IOCTL_RESET:
		status = vpudrvHardwareReset(hDevice, Request);
		break;

	case VDI_IOCTL_GET_INSTANCE_POOL:
		status = vpudrvGetInstancePool(hDevice, Request, OutputBufferLength);
		break;
	
	case VDI_IOCTL_READ_HPI_REGISTER:
	case VDI_IOCTL_WRITE_HPI_REGISTER:
		status = vpudrvReadWriteRegister(hDevice, Request, OutputBufferLength);
		break;

	default:
		status = STATUS_INVALID_PARAMETER;
		WdfRequestComplete(Request, status);
		break;
	}

	TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_QUEUE, "%!FUNC!: status %!STATUS!", status);

    return;
}

VOID
vpudrvEvtIoStop(
    _In_ WDFQUEUE Queue,
    _In_ WDFREQUEST Request,
    _In_ ULONG ActionFlags
)
/*++

Routine Description:

    This event is invoked for a power-managed queue before the device leaves the working state (D0).

Arguments:

    Queue -  Handle to the framework queue object that is associated with the
             I/O request.

    Request - Handle to a framework request object.

    ActionFlags - A bitwise OR of one or more WDF_REQUEST_STOP_ACTION_FLAGS-typed flags
                  that identify the reason that the callback function is being called
                  and whether the request is cancelable.

Return Value:

    VOID

--*/
{
    TraceEvents(TRACE_LEVEL_INFORMATION, 
                TRACE_QUEUE, 
                "!FUNC! Queue 0x%p, Request 0x%p ActionFlags %d", 
                Queue, Request, ActionFlags);

    //
    // In most cases, the EvtIoStop callback function completes, cancels, or postpones
    // further processing of the I/O request.
    //
    // Typically, the driver uses the following rules:
    //
    // - If the driver owns the I/O request, it calls WdfRequestUnmarkCancelable
    //   (if the request is cancelable) and either calls WdfRequestStopAcknowledge
    //   with a Requeue value of TRUE, or it calls WdfRequestComplete with a
    //   completion status value of STATUS_SUCCESS or STATUS_CANCELLED.
    //
    //   Before it can call these methods safely, the driver must make sure that
    //   its implementation of EvtIoStop has exclusive access to the request.
    //
    //   In order to do that, the driver must synchronize access to the request
    //   to prevent other threads from manipulating the request concurrently.
    //   The synchronization method you choose will depend on your driver's design.
    //
    //   For example, if the request is held in a shared context, the EvtIoStop callback
    //   might acquire an internal driver lock, take the request from the shared context,
    //   and then release the lock. At this point, the EvtIoStop callback owns the request
    //   and can safely complete or requeue the request.
    //
    // - If the driver has forwarded the I/O request to an I/O target, it either calls
    //   WdfRequestCancelSentRequest to attempt to cancel the request, or it postpones
    //   further processing of the request and calls WdfRequestStopAcknowledge with
    //   a Requeue value of FALSE.
    //
    // A driver might choose to take no action in EvtIoStop for requests that are
    // guaranteed to complete in a small amount of time.
    //
    // In this case, the framework waits until the specified request is complete
    // before moving the device (or system) to a lower power state or removing the device.
    // Potentially, this inaction can prevent a system from entering its hibernation state
    // or another low system power state. In extreme cases, it can cause the system
    // to crash with bugcheck code 9F.
    //

    return;
}

NTSTATUS 
	vpudrvGetReservedVideoMemoryInfo(
	_In_ WDFDEVICE hDevice,
	_In_ WDFREQUEST Request, 
	_In_ size_t OutputBufferSize
	)
{
	PDEVICE_CONTEXT deviceContext;
	NTSTATUS status = STATUS_SUCCESS;
	PVOID   pvOutputBuffer;
	
	PAGED_CODE();
	UNREFERENCED_PARAMETER(OutputBufferSize);
	deviceContext = DeviceGetContext(hDevice);

	if (OutputBufferSize < sizeof(vpudrv_buffer_t))
	{
		status = STATUS_INSUFFICIENT_RESOURCES;
		TraceEvents(TRACE_LEVEL_ERROR, TRACE_QUEUE, "%!FUNC! :OutputBufferSize  is not sufficient OutputBufferSize=%d\n", (int)OutputBufferSize);
		goto exit;
	};

	status = WdfRequestRetrieveOutputBuffer(Request, sizeof(vpudrv_buffer_t), &pvOutputBuffer, NULL);
	if (!NT_SUCCESS(status))
		goto exit;

	RtlCopyMemory(pvOutputBuffer, &deviceContext->reserved_video_memory, OutputBufferSize/*sizeof(deviceContext->reserved_video_memory)*/);

	TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_QUEUE, "%!FUNC! reserved memory size=%d, virt=0x%I64X, mdl=0x%I64X, base=0x%I64X, physic=0x%I64X\n", 
		(int)deviceContext->reserved_video_memory.size, (ULONG_PTR)deviceContext->reserved_video_memory.virt_addr, (ULONG_PTR)deviceContext->reserved_video_memory.mdl, (ULONG_PTR)deviceContext->reserved_video_memory.base, deviceContext->reserved_video_memory.phys_addr.QuadPart);	

	WdfRequestCompleteWithInformation(Request, status, sizeof(vpudrv_buffer_t));

	return status;
exit:
	WdfRequestComplete(Request, status);

	return status;
}

NTSTATUS 
	vpudrvGetInstancePool(
	_In_ WDFDEVICE hDevice,
	_In_ WDFREQUEST Request, 
	_In_ size_t OutputBufferSize
	)
{
	NTSTATUS status = STATUS_SUCCESS;
	PDEVICE_CONTEXT deviceContext;

	PVOID   pvOutputBuffer;
	vpudrv_buffer_t *pVpuBufferOutput;

	PAGED_CODE();
	
	UNREFERENCED_PARAMETER(Request);
	UNREFERENCED_PARAMETER(OutputBufferSize);

	deviceContext = DeviceGetContext(hDevice);

	if (OutputBufferSize < sizeof(vpudrv_buffer_t))
	{
		status = STATUS_INSUFFICIENT_RESOURCES;
		TraceEvents(TRACE_LEVEL_ERROR, TRACE_QUEUE, "%!FUNC! :OutputBufferSize  is not sufficient OutputBufferSize=%I64d\n", (LONGLONG)OutputBufferSize);
		goto exit;
	};
	
	status = WdfRequestRetrieveOutputBuffer(Request, sizeof(vpudrv_buffer_t), &pvOutputBuffer, NULL);
	if (!NT_SUCCESS(status))
		goto exit;

	pVpuBufferOutput = (vpudrv_buffer_t *)pvOutputBuffer;
		
	if (!deviceContext->instance_pool.base)
	{
		status = WdfMemoryCreate(WDF_NO_OBJECT_ATTRIBUTES, NonPagedPool, 0, pVpuBufferOutput->size, 
			&deviceContext->InstancePoolBuffer, &(PVOID)(ULONG_PTR)pVpuBufferOutput->base);
	
		if (!NT_SUCCESS(status) || !pVpuBufferOutput->base)
		{
			TraceEvents(TRACE_LEVEL_ERROR, TRACE_QUEUE, " VPUGetInstancePool :Fail to create memory for Instance pool %!STATUS!\n", status);
			goto exit;
		}

		RtlZeroMemory((PVOID)(ULONG_PTR)pVpuBufferOutput->base, pVpuBufferOutput->size);

		RtlCopyMemory(&deviceContext->instance_pool, pvOutputBuffer, OutputBufferSize/*sizeof(deviceContext->instance_pool)*/);
	}
	else
		RtlCopyMemory(pvOutputBuffer, &deviceContext->instance_pool , OutputBufferSize/*sizeof(deviceContext->instance_pool)*/);	

	status = MapMemory(hDevice, pVpuBufferOutput, MmCached);
	if (!NT_SUCCESS(status))
	{
		TraceEvents(TRACE_LEVEL_ERROR, TRACE_QUEUE, "%!FUNC! Fail to map instance-pool to user status = 0x%x\n", status);
		goto exit;
	}

	WdfRequestCompleteWithInformation(Request, status, sizeof(vpudrv_buffer_t));	

	return status;
exit:

	WdfRequestComplete(Request, status);

	return status;
}

NTSTATUS 
	vpudrvClockGate(
	_In_ WDFDEVICE hDevice,
	IN WDFREQUEST Request, 
	IN size_t InputBufferSize
	)
{
	NTSTATUS status = STATUS_SUCCESS;
	PDEVICE_CONTEXT deviceContext;
	ULONG   Ret = 0;
	PVOID   pvInputBuffer;
	ULONG	ClockGate;

	PAGED_CODE();

	deviceContext = DeviceGetContext(hDevice);

	if (InputBufferSize < sizeof(ULONG))
	{
		status = STATUS_INSUFFICIENT_RESOURCES;
		TraceEvents(TRACE_LEVEL_ERROR, TRACE_QUEUE, "%!FUNC! Clock status Parameter Clock Flag size=%I64d\n", (LONGLONG)InputBufferSize);
		goto exit;
	};

	status = WdfRequestRetrieveInputBuffer(Request, sizeof(ULONG), &pvInputBuffer, NULL);
	if (!NT_SUCCESS(status))
		goto exit;

	ClockGate = (LONG)*((PULONG)pvInputBuffer);

	Ret = ClockGate;

exit:

	WdfRequestCompleteWithInformation(Request, status, Ret);

	return status;
};

NTSTATUS 
	vpudrvWaitInterrupt(
	_In_ WDFDEVICE hDevice,
	_In_ WDFREQUEST Request, 
	_In_ size_t InputBufferSize
	)
{
	NTSTATUS status = STATUS_SUCCESS;
	PDEVICE_CONTEXT deviceContext;
	ULONG   Ret = 0;
	PVOID   pvInputBuffer;
	ULONG timeoutms;
	LONGLONG timeout;

	PAGED_CODE();

	deviceContext = DeviceGetContext(hDevice);

	if (InputBufferSize < sizeof(ULONG))
	{
		status = STATUS_INSUFFICIENT_RESOURCES;
		TraceEvents(TRACE_LEVEL_ERROR, TRACE_QUEUE, "%!FUNC! Invalid timeout Parameter timeout size=%I64d\n", (LONGLONG)InputBufferSize);
		goto exit;
	};

	status = WdfRequestRetrieveInputBuffer(Request, sizeof(ULONG), &pvInputBuffer, NULL);
	if (!NT_SUCCESS(status))
		goto exit;

	timeoutms = (ULONG)*((PULONG)pvInputBuffer);
	timeout = timeoutms * 1000 * 10;

	status = WdfWaitLockAcquire(deviceContext->InterruptWaitlock, &timeout);
	if (status == STATUS_TIMEOUT)
		Ret = (ULONG)-1;		
	else
		Ret = 0;
exit:

	WdfRequestCompleteWithInformation(Request, status, Ret);

	return status;
};

NTSTATUS 
	vpudrvMapUserMemory(
	_In_ WDFDEVICE hDevice,
	_In_ WDFREQUEST Request, 
	_In_ size_t OutputBufferSize, 
	_In_ BOOLEAN bMap
	)
{
	NTSTATUS status = STATUS_SUCCESS;
	PDEVICE_CONTEXT deviceContext;
	PVOID   pvOutputBuffer;
	vpudrv_buffer_t *pVpuBufferOutput;

	PAGED_CODE();

	UNREFERENCED_PARAMETER(Request);
	UNREFERENCED_PARAMETER(bMap);
	UNREFERENCED_PARAMETER(OutputBufferSize);

	deviceContext = DeviceGetContext(hDevice);

	if (OutputBufferSize < sizeof(vpudrv_buffer_t))
	{
		status = STATUS_INSUFFICIENT_RESOURCES;
		TraceEvents(TRACE_LEVEL_ERROR, TRACE_QUEUE, " %!FUNC! :OutputBufferSize is not sufficient OutputBufferSize=0x%I64X, struct size = %I64d \n", (LONGLONG)OutputBufferSize, sizeof(vpudrv_buffer_t));
		goto exit;
	}

	status = WdfRequestRetrieveOutputBuffer(Request, sizeof(vpudrv_buffer_t), &pvOutputBuffer, NULL);
	if (!NT_SUCCESS(status))
		goto exit;

	pVpuBufferOutput = (vpudrv_buffer_t *)pvOutputBuffer;

	if (bMap)
	{
		if (pVpuBufferOutput->phys_addr.QuadPart == -1) // 0x00 means that vpuapi are requesting to get a virtual address of VPU Register Base
		{
			pVpuBufferOutput->phys_addr = deviceContext->regsBasePA;
			pVpuBufferOutput->base = (ULONG_PTR)deviceContext->RegsBase;
			pVpuBufferOutput->size = deviceContext->RegsLength;

			TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_QUEUE, "%!FUNC! read to map physical address size=%d, virt=0x%I64X, mdl=0x%I64X, base=0x%I64X, physic=0x%I64X\n", 
				(int)pVpuBufferOutput->size, (LONGLONG)pVpuBufferOutput->virt_addr, (LONGLONG)pVpuBufferOutput->mdl, (LONGLONG)pVpuBufferOutput->base, pVpuBufferOutput->phys_addr.QuadPart);	

			status = MapMemory(hDevice, pVpuBufferOutput, MmNonCached);
		}
		else
		{
			status = MapMemory(hDevice, pVpuBufferOutput, MmWriteCombined);
		}
	}
	else
		status = UnMapMemory(hDevice, pVpuBufferOutput);

	if (!NT_SUCCESS(status))
	{
		TraceEvents(TRACE_LEVEL_ERROR, TRACE_QUEUE, "%!FUNC! fail to map physical address virt=0x%I64X, mdl=0x%I64X, base=0x%I64X, physic=0x%I64X\n", 
			(LONGLONG)pVpuBufferOutput->virt_addr, (ULONG_PTR)pVpuBufferOutput->mdl, (ULONG_PTR)pVpuBufferOutput->base, pVpuBufferOutput->phys_addr.QuadPart);	
		goto exit;
	}

	TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_QUEUE, "%!FUNC! Success to map physical address size=%d, virt=0x%I64X, mdl=0x%I64X, base=0x%I64X, physic=0x%I64X\n", 
		(int)pVpuBufferOutput->size, (LONGLONG)pVpuBufferOutput->virt_addr, (LONGLONG)(ULONG_PTR)pVpuBufferOutput->mdl, (ULONG_PTR)pVpuBufferOutput->base, pVpuBufferOutput->phys_addr.QuadPart);	

	WdfRequestCompleteWithInformation(Request, status, sizeof(vpudrv_buffer_t));
	return status;

exit:
	WdfRequestComplete(Request, status);

	return status;
}

NTSTATUS
	vpudrvHardwareReset(
	_In_ WDFDEVICE hDevice,
	_In_ WDFREQUEST Request
	)
{
	NTSTATUS status;
	LARGE_INTEGER delay;
	PDEVICE_CONTEXT deviceContext;

	UNREFERENCED_PARAMETER(Request);
	TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_QUEUE, "--> %!FUNC!");

	deviceContext = DeviceGetContext(hDevice);

	// Wait 100 msec.
	//
	delay.QuadPart =  WDF_REL_TIMEOUT_IN_MS(100);

	KeDelayExecutionThread( KernelMode, TRUE, &delay );

	status = STATUS_SUCCESS;
	TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_QUEUE, "<-- %!FUNC!");
	WdfRequestComplete(Request, status);
	return STATUS_SUCCESS;
}

NTSTATUS
	vpudrvReadWriteRegister(
	_In_ WDFDEVICE hDevice,
	_In_ WDFREQUEST Request, 
	_In_ size_t OutputBufferSize
	)
{
	NTSTATUS status = STATUS_SUCCESS;
	PDEVICE_CONTEXT deviceContext;
	PVOID   pvOutputBuffer;
	PHPI_REGISTER_INFO pRegisterInfo;

	PAGED_CODE();
	UNREFERENCED_PARAMETER(OutputBufferSize);
	deviceContext = DeviceGetContext(hDevice);
#if 0
	if (OutputBufferSize < sizeof(HPI_REGISTER_INFO))
	{
		status = STATUS_INSUFFICIENT_RESOURCES;
		TraceEvents(TRACE_LEVEL_ERROR, TRACE_QUEUE, " %!FUNC! :OutputBufferSize is not sufficient OutputBufferSize=0x%I64X\n", (LONGLONG)OutputBufferSize);
		goto exit;
	};
#endif

	status = WdfRequestRetrieveOutputBuffer(Request, sizeof(HPI_REGISTER_INFO), &pvOutputBuffer, NULL);
	if (!NT_SUCCESS(status))
		goto exit;

	pRegisterInfo = (PHPI_REGISTER_INFO)pvOutputBuffer;

	if (pRegisterInfo->Address >= deviceContext->RegsLength){
		status = STATUS_ACCESS_VIOLATION;
		TraceEvents(TRACE_LEVEL_ERROR, TRACE_QUEUE, " %!FUNC! : Request Address is over than HPI address range, addr=0x%x\n", pRegisterInfo->Address);
		goto exit;
	}
	
	if (pRegisterInfo->Command == REG_WRITE_REGISTER)
		WRITE_REGISTER_ULONG((PULONG)((ULONG_PTR)deviceContext->RegsBase + pRegisterInfo->Address), (ULONG)pRegisterInfo->Data);
	else
		pRegisterInfo->Data = READ_REGISTER_ULONG((PULONG)((ULONG_PTR)deviceContext->RegsBase + pRegisterInfo->Address));
	
	WdfRequestCompleteWithInformation(Request, status, sizeof(HPI_REGISTER_INFO));

	return status;
exit:
	WdfRequestComplete(Request, status);

	return status;
};

NTSTATUS 
	MapMemory(
	_In_ WDFDEVICE hDevice,
	_Inout_ vpudrv_buffer_t *vb,
	_In_ MEMORY_CACHING_TYPE cacheMode
	)
{
	NTSTATUS status = STATUS_SUCCESS;
	PDEVICE_CONTEXT deviceContext;

	deviceContext = DeviceGetContext(hDevice);

	__try
	{

		try
		{
			vb->mdl = (ULONG_PTR)(PMDL)IoAllocateMdl((PVOID)(ULONG_PTR)vb->base, vb->size, FALSE, FALSE, NULL);
			if (!vb->mdl)
			{
				TraceEvents(TRACE_LEVEL_ERROR, TRACE_QUEUE, "%!FUNC! Fail to IoAllocateMdl vb->size = %d\n", (int)vb->size);
				status = STATUS_INSUFFICIENT_RESOURCES;
				__leave;
			};

			MmBuildMdlForNonPagedPool((PMDL)(ULONG_PTR)vb->mdl);
			vb->virt_addr = (ULONG_PTR)MmMapLockedPagesSpecifyCache((PMDL)(ULONG_PTR)vb->mdl, UserMode, 
				cacheMode, NULL, FALSE, NormalPagePriority);
			if (!vb->virt_addr)
			{
				TraceEvents(TRACE_LEVEL_ERROR, TRACE_QUEUE, "%!FUNC! Fail to MmMapLockedPagesSpecifyCache\n");
				status = STATUS_INSUFFICIENT_RESOURCES;
				__leave;
			};

			status = STATUS_SUCCESS;
		}
		except(EXCEPTION_EXECUTE_HANDLER)
		{
			status = STATUS_INSUFFICIENT_RESOURCES;
			__leave;
		};
	}
	__finally
	{
		if (!NT_SUCCESS(status))
		{
			TraceEvents(TRACE_LEVEL_ERROR, TRACE_QUEUE, "%!FUNC! Fail to UnMapMemory status = 0x%x\n", (int)status);
			UnMapMemory(hDevice, vb);
		};
	};
	return status;
};

NTSTATUS
	UnMapMemory(
	_In_ WDFDEVICE hDevice,
	_Inout_ vpudrv_buffer_t *vb
	)
{
	NTSTATUS status = STATUS_SUCCESS;
	PDEVICE_CONTEXT deviceContext;

	PAGED_CODE();

	deviceContext = DeviceGetContext(hDevice);

	try
	{
		if (vb->virt_addr)
			MmUnmapLockedPages((PVOID)(ULONG_PTR)vb->virt_addr, (PMDL)(ULONG_PTR)vb->mdl);
		if (vb->mdl)
			IoFreeMdl((PMDL)(ULONG_PTR)vb->mdl);
	}
	except(EXCEPTION_EXECUTE_HANDLER)
	{
		status = STATUS_ACCESS_VIOLATION;
		TraceEvents(TRACE_LEVEL_ERROR, TRACE_QUEUE, " Fail to UnMapMemory status = 0x%x\n", status);		
	};

	return status;
}

ULONG 
	ReadVpuRegister(
	_In_ WDFDEVICE hDevice,
	_In_ ULONG Addr
	)
{
	ULONG Status;
	ULONG Data;
	PDEVICE_CONTEXT deviceContext;

	deviceContext = DeviceGetContext(hDevice);

	WdfSpinLockAcquire(deviceContext->HPILock);

	WRITE_REGISTER_ULONG((PULONG)((ULONG_PTR)((ULONG_PTR)deviceContext->RegsBase + HPI_ADDR_ADDR_H)),  (ULONG)((Addr >> 16)&0xffff));
	WRITE_REGISTER_ULONG((PULONG)((ULONG_PTR)((ULONG_PTR)deviceContext->RegsBase + HPI_ADDR_ADDR_L)), (ULONG)(Addr & 0xffff));

	WRITE_REGISTER_ULONG((PULONG)((ULONG_PTR)((ULONG_PTR)deviceContext->RegsBase + HPI_ADDR_CMD)), (ULONG)HPI_CMD_READ_VALUE);

	do {
		Status = READ_REGISTER_ULONG((PULONG)((ULONG_PTR)deviceContext->RegsBase + HPI_ADDR_STATUS));
		Status = Status & 1;
	} while (Status == 0);

	Data = READ_REGISTER_ULONG((PULONG)((ULONG_PTR)deviceContext->RegsBase + HPI_ADDR_DATA)) << 16;
	Data |= READ_REGISTER_ULONG((PULONG)((ULONG_PTR)deviceContext->RegsBase + (HPI_ADDR_DATA + 4)));

	WdfSpinLockRelease(deviceContext->HPILock);
	return Data;

}

VOID 
	WriteVpuRegister(
	_In_ WDFDEVICE hDevice,
	_In_ ULONG Addr, 
	_In_ ULONG Data
	)
{
	ULONG Status;
	PDEVICE_CONTEXT deviceContext;

	deviceContext = DeviceGetContext(hDevice);

	WdfSpinLockAcquire(deviceContext->HPILock);

	WRITE_REGISTER_ULONG((PULONG)((ULONG_PTR)((ULONG_PTR)deviceContext->RegsBase + HPI_ADDR_ADDR_H)), (ULONG)(Addr >> 16));
	WRITE_REGISTER_ULONG((PULONG)((ULONG_PTR)((ULONG_PTR)deviceContext->RegsBase + HPI_ADDR_ADDR_L)), (ULONG)(Addr & 0xffff));

	WRITE_REGISTER_ULONG((PULONG)((ULONG_PTR)((ULONG_PTR)deviceContext->RegsBase + HPI_ADDR_DATA)), (ULONG)((Data >> 16) & 0xFFFF));
	WRITE_REGISTER_ULONG((PULONG)((ULONG_PTR)((ULONG_PTR)deviceContext->RegsBase + (HPI_ADDR_DATA + 4))), (ULONG)(Data & 0xFFFF));

	WRITE_REGISTER_ULONG((PULONG)((ULONG_PTR)((ULONG_PTR)deviceContext->RegsBase + HPI_ADDR_CMD)), (ULONG)HPI_CMD_WRITE_VALUE);

	do {
		Status = READ_REGISTER_ULONG((PULONG)((ULONG_PTR)((ULONG_PTR)deviceContext->RegsBase + HPI_ADDR_STATUS)));
		Status = (Status>>1) & 1;
	} while (Status == 0);

	WdfSpinLockRelease(deviceContext->HPILock);
}

