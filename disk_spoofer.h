#pragma once

#include <ntddk.h>
#include <ntddscsi.h>
#include <ntddstor.h>
#include <scsi.h>

// Driver information
#define DRIVER_NAME "DiskSpoofer"
#define DRIVER_VERSION "1.0"

// Maximum serial number length
#define MAX_SERIAL_LENGTH 32

// Function prototypes
NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath);
VOID DriverUnload(PDRIVER_OBJECT DriverObject);
NTSTATUS HookedDeviceControl(PDEVICE_OBJECT DeviceObject, PIRP Irp);
NTSTATUS SetupHooks(void);
VOID RemoveHooks(void);
VOID GenerateRandomSerial(PCHAR Buffer, ULONG Length);
NTSTATUS ModifyStorageDeviceDescriptor(PVOID Buffer, ULONG BufferLength);

// Global variables declarations
extern PDRIVER_OBJECT g_OriginalDriverObject;
extern PDRIVER_DISPATCH g_OriginalDeviceControl;
extern CHAR g_SpoofedSerial[];

// Debug macros
#ifdef DBG
#define DebugPrint(x) DbgPrint x
#else
#define DebugPrint(x)
#endif

// Utility macros
#define POOL_TAG 'pooS'  // 'Soop' in little endian

// Status codes
#define STATUS_HOOK_FAILED          ((NTSTATUS)0xC0000001L)
#define STATUS_DRIVER_NOT_FOUND     ((NTSTATUS)0xC0000002L)