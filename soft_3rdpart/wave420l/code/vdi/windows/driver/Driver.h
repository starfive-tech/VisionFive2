/*++

Module Name:

    driver.h

Abstract:

    This file contains the driver definitions.

Environment:

    Kernel-mode Driver Framework

--*/

#define INITGUID

#include <ntddk.h>
#include <wdf.h>

#include "device.h"
#include "queue.h"
#include "trace.h"
//
// WDFDRIVER Events
//

DRIVER_INITIALIZE DriverEntry;
EVT_WDF_DRIVER_DEVICE_ADD vpudrvEvtDeviceAdd;
EVT_WDF_OBJECT_CONTEXT_CLEANUP vpudrvEvtDriverContextCleanup;

EVT_WDF_DEVICE_D0_ENTRY vpudrvEvtDeviceD0Entry;
EVT_WDF_DEVICE_D0_EXIT vpudrvEvtDeviceD0Exit;
EVT_WDF_DEVICE_PREPARE_HARDWARE vpudrvEvtDevicePrepareHardware;
EVT_WDF_DEVICE_RELEASE_HARDWARE vpudrvEvtDeviceReleaseHardware;
 
