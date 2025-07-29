#include <ntddk.h>
#include <ntddscsi.h>
#include <ntddstor.h>
#include <scsi.h>

// Forward declarations
NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath);
VOID DriverUnload(PDRIVER_OBJECT DriverObject);
NTSTATUS HookedDeviceControl(PDEVICE_OBJECT DeviceObject, PIRP Irp);
NTSTATUS SetupHooks(void);
VOID RemoveHooks(void);

// Global variables
PDRIVER_OBJECT g_OriginalDriverObject = NULL;
PDRIVER_DISPATCH g_OriginalDeviceControl = NULL;
UNICODE_STRING g_TargetDeviceName;
PDEVICE_OBJECT g_TargetDeviceObject = NULL;
PFILE_OBJECT g_TargetFileObject = NULL;

// Custom serial number to spoof
CHAR g_SpoofedSerial[] = "SPOOF1234567890ABCD";

// Function to generate random serial
VOID GenerateRandomSerial(PCHAR Buffer, ULONG Length) {
    LARGE_INTEGER TickCount;
    ULONG Seed;
    ULONG i;
    
    KeQueryTickCount(&TickCount);
    Seed = TickCount.LowPart;
    
    for (i = 0; i < Length - 1; i++) {
        Seed = (Seed * 1103515245 + 12345) & 0x7fffffff;
        Buffer[i] = 'A' + (Seed % 26);
    }
    Buffer[Length - 1] = '\0';
}

// Function to modify storage device descriptor
NTSTATUS ModifyStorageDeviceDescriptor(PVOID Buffer, ULONG BufferLength) {
    PSTORAGE_DEVICE_DESCRIPTOR DeviceDescriptor = (PSTORAGE_DEVICE_DESCRIPTOR)Buffer;
    PCHAR SerialNumberOffset;
    ULONG NewSerialLength = strlen(g_SpoofedSerial);
    
    if (BufferLength < sizeof(STORAGE_DEVICE_DESCRIPTOR)) {
        return STATUS_BUFFER_TOO_SMALL;
    }
    
    // Check if there's a serial number offset
    if (DeviceDescriptor->SerialNumberOffset != 0) {
        SerialNumberOffset = (PCHAR)Buffer + DeviceDescriptor->SerialNumberOffset;
        
        // Ensure we don't overflow the buffer
        if ((DeviceDescriptor->SerialNumberOffset + NewSerialLength + 1) <= BufferLength) {
            RtlCopyMemory(SerialNumberOffset, g_SpoofedSerial, NewSerialLength + 1);
            DbgPrint("[DiskSpoofer] Serial number spoofed: %s\n", g_SpoofedSerial);
        }
    }
    
    return STATUS_SUCCESS;
}

// Hooked DeviceControl handler
NTSTATUS HookedDeviceControl(PDEVICE_OBJECT DeviceObject, PIRP Irp) {
    PIO_STACK_LOCATION IoStack = IoGetCurrentIrpStackLocation(Irp);
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG IoControlCode = IoStack->Parameters.DeviceIoControl.IoControlCode;
    
    // Check if this is a storage query property request
    if (IoControlCode == IOCTL_STORAGE_QUERY_PROPERTY) {
        PSTORAGE_PROPERTY_QUERY Query = (PSTORAGE_PROPERTY_QUERY)Irp->AssociatedIrp.SystemBuffer;
        
        if (Query && Query->PropertyId == StorageDeviceProperty) {
            // Call the original handler first
            Status = g_OriginalDeviceControl(DeviceObject, Irp);
            
            // If successful, modify the response
            if (NT_SUCCESS(Status) && Irp->AssociatedIrp.SystemBuffer) {
                ModifyStorageDeviceDescriptor(
                    Irp->AssociatedIrp.SystemBuffer,
                    IoStack->Parameters.DeviceIoControl.OutputBufferLength
                );
            }
            
            return Status;
        }
    }
    
    // For all other requests, call the original handler
    return g_OriginalDeviceControl(DeviceObject, Irp);
}

// Setup hooks for disk devices
NTSTATUS SetupHooks(void) {
    NTSTATUS Status;
    PDRIVER_OBJECT DiskDriverObject = NULL;
    UNICODE_STRING DiskDriverName;
    
    // Get reference to disk driver
    RtlInitUnicodeString(&DiskDriverName, L"\\Driver\\Disk");
    Status = ObReferenceObjectByName(
        &DiskDriverName,
        OBJ_CASE_INSENSITIVE,
        NULL,
        0,
        *IoDriverObjectType,
        KernelMode,
        NULL,
        (PVOID*)&DiskDriverObject
    );
    
    if (!NT_SUCCESS(Status)) {
        DbgPrint("[DiskSpoofer] Failed to get disk driver object: 0x%X\n", Status);
        return Status;
    }
    
    // Save original handler and install hook
    g_OriginalDriverObject = DiskDriverObject;
    g_OriginalDeviceControl = DiskDriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL];
    DiskDriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = HookedDeviceControl;
    
    DbgPrint("[DiskSpoofer] Hooks installed successfully\n");
    return STATUS_SUCCESS;
}

// Remove hooks
VOID RemoveHooks(void) {
    if (g_OriginalDriverObject && g_OriginalDeviceControl) {
        g_OriginalDriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = g_OriginalDeviceControl;
        ObDereferenceObject(g_OriginalDriverObject);
        DbgPrint("[DiskSpoofer] Hooks removed\n");
    }
}

// Driver entry point
NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath) {
    UNREFERENCED_PARAMETER(RegistryPath);
    
    DbgPrint("[DiskSpoofer] Driver loaded\n");
    
    // Generate random serial number
    GenerateRandomSerial(g_SpoofedSerial, sizeof(g_SpoofedSerial));
    DbgPrint("[DiskSpoofer] Generated serial: %s\n", g_SpoofedSerial);
    
    // Set unload routine (though kdmapper doesn't use it)
    DriverObject->DriverUnload = DriverUnload;
    
    // Setup hooks
    NTSTATUS Status = SetupHooks();
    if (!NT_SUCCESS(Status)) {
        DbgPrint("[DiskSpoofer] Failed to setup hooks: 0x%X\n", Status);
        return Status;
    }
    
    DbgPrint("[DiskSpoofer] Driver initialization complete\n");
    return STATUS_SUCCESS;
}

// Driver unload routine
VOID DriverUnload(PDRIVER_OBJECT DriverObject) {
    UNREFERENCED_PARAMETER(DriverObject);
    
    DbgPrint("[DiskSpoofer] Driver unloading\n");
    RemoveHooks();
    DbgPrint("[DiskSpoofer] Driver unloaded\n");
}