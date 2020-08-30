#include "driver.h"

UINT32 SyscallId = 0xe0;
BOOLEAN WriteProtected;
__NtOpenProcess SavedNtOpenProcess;
PTR SystemTable;
UNICODE_STRING DriverName = RTL_CONSTANT_STRING(L"\\Device\\AmirsDriver");

BOOLEAN IsWriteProtected()
{
	/* Get cr0 and check its 16th bit (WP is enables if the value is 1) */
	UINT32 cr0_value;
	__asm
	{
		mov eax, cr0
		mov cr0_value, eax
	}
	return cr0_value & WP_BIT;
}

NTSTATUS MyNtOpenProcess(PHANDLE ProcessHandle, ACCESS_MASK DesiredAccess, 
	POBJECT_ATTRIBUTES ObjectAttributes, PCLIENT_ID ClientId)
{
	/* 
		manipulate the call...
	*/

	return SavedNtOpenProcess(ProcessHandle, DesiredAccess,
		ObjectAttributes, ClientId);
}

VOID OverrideNtOpenProcessHandler()
{
	PTR KTHREAD = GetCurrentKTHREAD(), SSDT, NtOpenProcessAddress;
	/* Locate the SSDT Address in the current KTHREAD structure */
	memcpy(&SSDT, KTHREAD + SERVICE_TABLE_OFFSET, sizeof(PTR)); 
	/* The NT system call list (those in ntoskrnl.exe) are listed in the
		first table. The second table contains addresses of functions insde win32k.sys */
	memcpy(&SystemTable, SSDT, sizeof(PTR));
	/* Each value in the table is 4 bytes long */
	NtOpenProcessAddress = SystemTable + SyscallId * 4;
	/* Save the current handler */
	memcpy(&SavedNtOpenProcess, NtOpenProcessAddress, sizeof(PTR));
	/* Override the handler*/
	_InterlockedExchange((LONG*)NtOpenProcessAddress, (LONG)MyNtOpenProcess);
}

VOID Unload(IN PDRIVER_OBJECT DriverObject)
{
	PTR NtOpenProcessAddress = SystemTable + SyscallId * 4;
	/* Put back the original address */
	_InterlockedExchange((LONG*)NtOpenProcessAddress, (LONG)SavedNtOpenProcess);
	if (WriteProtected)
		EnableWriteProtection();
	IoDeleteDevice(DriverObject->DeviceObject);
}

NTSTATUS UnsupportedDriverOperation(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
	NTSTATUS NtStatus = STATUS_NOT_SUPPORTED;
	// ...
	return NtStatus;
}

NTSTATUS DriverEntry(IN PDRIVER_OBJECT DriverObject, IN PUNICODE_STRING RegistryPath)
{
	NTSTATUS Status = STATUS_SUCCESS;
	PDEVICE_OBJECT DeviceObject = NULL;

	Status = IoCreateDevice(DriverObject, 0, &DriverName, FILE_DEVICE_UNKNOWN, FILE_DEVICE_SECURE_OPEN, FALSE, &DeviceObject);
	if (!NT_SUCCESS(Status))
		return Status;

	for (UINT32 i = 0; i < IRP_MJ_MAXIMUM_FUNCTION; i++)
		DriverObject->MajorFunction[i] = UnsupportedDriverOperation;
	DriverObject->DriverUnload = Unload;

	if (WriteProtected = IsWriteProtected())
		DisableWriteProtection();

	return STATUS_SUCCESS;
}
