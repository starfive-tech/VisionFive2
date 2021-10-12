/*++

Module Name:

    driver.c

Abstract:

    This file contains the driver entry points and callbacks.

Environment:

    Kernel-mode Driver Framework

--*/

#include "driver.h"
#include "driver.tmh"

#ifdef ALLOC_PRAGMA
#pragma alloc_text (INIT, DriverEntry)
#pragma alloc_text (PAGE, vpudrvEvtDeviceAdd)
#pragma alloc_text (PAGE, vpudrvEvtDriverContextCleanup)

#pragma alloc_text (PAGE, vpudrvEvtDevicePrepareHardware)
#pragma alloc_text (PAGE, vpudrvEvtDeviceReleaseHardware)
#pragma alloc_text (PAGE, vpudrvEvtDeviceD0Exit)
#pragma alloc_text (PAGE, vpudrvEvtDeviceD0Entry)

#endif

NTSTATUS
DriverEntry(
    _In_ PDRIVER_OBJECT  DriverObject,
    _In_ PUNICODE_STRING RegistryPath
    )
/*++

Routine Description:
    DriverEntry initializes the driver and is the first routine called by the
    system after the driver is loaded. DriverEntry specifies the other entry
    points in the function driver, such as EvtDevice and DriverUnload.

Parameters Description:

    DriverObject - represents the instance of the function driver that is loaded
    into memory. DriverEntry must initialize members of DriverObject before it
    returns to the caller. DriverObject is allocated by the system before the
    driver is loaded, and it is released by the system after the system unloads
    the function driver from memory.

    RegistryPath - represents the driver specific path in the Registry.
    The function driver can use the path to store driver related data between
    reboots. The path does not store hardware instance specific data.

Return Value:

    STATUS_SUCCESS if successful,
    STATUS_UNSUCCESSFUL otherwise.

--*/
{
    WDF_DRIVER_CONFIG config;
    NTSTATUS status;
    WDF_OBJECT_ATTRIBUTES attributes;

    //
    // Initialize WPP Tracing
    //
    WPP_INIT_TRACING( DriverObject, RegistryPath );

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "%!FUNC! Entry5");

    //
    // Register a cleanup callback so that we can call WPP_CLEANUP when
    // the framework driver object is deleted during driver unload.
    //
    WDF_OBJECT_ATTRIBUTES_INIT(&attributes);
    attributes.EvtCleanupCallback = vpudrvEvtDriverContextCleanup;

    WDF_DRIVER_CONFIG_INIT(&config,
                           vpudrvEvtDeviceAdd
                           );

    status = WdfDriverCreate(DriverObject,
                             RegistryPath,
                             &attributes,
                             &config,
                             WDF_NO_HANDLE
                             );

    if (!NT_SUCCESS(status)) {
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_DRIVER, "WdfDriverCreate failed %!STATUS!", status);
        WPP_CLEANUP(DriverObject);
        return status;
    }

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "%!FUNC! Exit");

    return status;
}

NTSTATUS
vpudrvEvtDeviceAdd(
    _In_    WDFDRIVER       Driver,
    _Inout_ PWDFDEVICE_INIT DeviceInit
    )
/*++
Routine Description:

    EvtDeviceAdd is called by the framework in response to AddDevice
    call from the PnP manager. We create and initialize a device object to
    represent a new instance of the device.

Arguments:

    Driver - Handle to a framework driver object created in DriverEntry

    DeviceInit - Pointer to a framework-allocated WDFDEVICE_INIT structure.

Return Value:

    NTSTATUS

--*/
{
    NTSTATUS status;
	WDF_PNPPOWER_EVENT_CALLBACKS pnpPowerCallbacks;

    UNREFERENCED_PARAMETER(Driver);

    PAGED_CODE();

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "%!FUNC! Entry");

	WdfDeviceInitSetIoType(DeviceInit, WdfDeviceIoBuffered);

	//
	// Zero out the PnpPowerCallbacks structure.
	//
	WDF_PNPPOWER_EVENT_CALLBACKS_INIT(&pnpPowerCallbacks);

	//
	// Set Callbacks for any of the functions we are interested in.
	// If no callback is set, Framework will take the default action
	// by itself.
	//
	pnpPowerCallbacks.EvtDevicePrepareHardware = vpudrvEvtDevicePrepareHardware;
	pnpPowerCallbacks.EvtDeviceReleaseHardware = vpudrvEvtDeviceReleaseHardware;

	//
	// These two callbacks set up and tear down hardware state that must be
	// done every time the device moves in and out of the D0-working state.
	//
	pnpPowerCallbacks.EvtDeviceD0Entry         = vpudrvEvtDeviceD0Entry;
	pnpPowerCallbacks.EvtDeviceD0Exit          = vpudrvEvtDeviceD0Exit;

	//
	// Register the PnP Callbacks..
	//
	WdfDeviceInitSetPnpPowerEventCallbacks(DeviceInit, &pnpPowerCallbacks);

    status = vpudrvCreateDevice(DeviceInit);
	if (!NT_SUCCESS(status)) {
		TraceEvents(TRACE_LEVEL_ERROR, TRACE_DRIVER, "vpudrvCreateDevice failed: %!STATUS!", status);
	}
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "%!FUNC! Exit");

    return status;
}

NTSTATUS
	vpudrvEvtDevicePrepareHardware (
	WDFDEVICE      Device,
	WDFCMRESLIST   Resources,
	WDFCMRESLIST   ResourcesTranslated
	)
{
	NTSTATUS            status = STATUS_SUCCESS;
	
	UNREFERENCED_PARAMETER(Resources);

	PAGED_CODE();

	status = vpudrvPrepareDevice(Device, ResourcesTranslated);
	if (!NT_SUCCESS (status)){
		TraceEvents(TRACE_LEVEL_ERROR, TRACE_DRIVER,
			"<-- Fail to vpudrvPrepreDevice, status %!STATUS!", status);
		return status;
	}

	TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "%!FUNC! Exit");

	return status;
}

NTSTATUS
	vpudrvEvtDeviceReleaseHardware(
	_In_  WDFDEVICE Device,
	_In_  WDFCMRESLIST ResourcesTranslated
	)
{
	NTSTATUS status = STATUS_SUCCESS;
	UNREFERENCED_PARAMETER(ResourcesTranslated);

	PAGED_CODE();

	vpudrvReleaseDevice(Device);

	TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "%!FUNC! Exit");

	return status;
}

NTSTATUS
	vpudrvEvtDeviceD0Exit(
	_In_  WDFDEVICE Device,
	_In_  WDF_POWER_DEVICE_STATE TargetState
	)
{
	NTSTATUS status = STATUS_SUCCESS;

	PAGED_CODE();

	switch (TargetState) {
	case WdfPowerDeviceD1:
	case WdfPowerDeviceD2:
	case WdfPowerDeviceD3:

		//
		// Fill in any code to save hardware state here.
		//

		//
		// Fill in any code to put the device in a low-power state here.
		//
		break;

	case WdfPowerDevicePrepareForHibernation:

		//
		// Fill in any code to save hardware state here.  Do not put in any
		// code to shut the device off.  If this device cannot support being
		// in the paging path (or being a parent or grandparent of a paging
		// path device) then this whole case can be deleted.
		//

		break;

	case WdfPowerDeviceD3Final:
	default:

		//
		// Reset the hardware, as we're shutting down for the last time.
		//
		vpudrvShutdownDevice(Device);
		break;
	}

	TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "%!FUNC! Exit");

	return status;
}

NTSTATUS
	vpudrvEvtDeviceD0Entry(
	IN  WDFDEVICE Device,
	IN  WDF_POWER_DEVICE_STATE PreviousState
	)
{
	NTSTATUS status = STATUS_SUCCESS;
	UNREFERENCED_PARAMETER(Device);
	UNREFERENCED_PARAMETER(PreviousState);

	PAGED_CODE();

	TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "%!FUNC! Exit");

	return status;
}

VOID
vpudrvEvtDriverContextCleanup(
    _In_ WDFOBJECT DriverObject
    )
/*++
Routine Description:

    Free all the resources allocated in DriverEntry.

Arguments:

    DriverObject - handle to a WDF Driver object.

Return Value:

    VOID.

--*/
{
    UNREFERENCED_PARAMETER(DriverObject);

    PAGED_CODE ();

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "%!FUNC! Entry");

    //
    // Stop WPP Tracing
    //
    WPP_CLEANUP( WdfDriverWdmGetDriverObject(DriverObject) );

}

