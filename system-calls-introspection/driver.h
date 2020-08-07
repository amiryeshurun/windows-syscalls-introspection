#ifndef __DRIVER_H_
#define __DRIVER_H_

#include "ntddk.h"

#define WP_BIT (1 << 16)
#define SERVICE_TABLE_OFFSET 0x3C

typedef unsigned char* PTR;

typedef NTSYSAPI NTSTATUS(NTAPI* __NtOpenProcess)(
	PHANDLE            processHandle,
	ACCESS_MASK        desiredAccess,
	POBJECT_ATTRIBUTES objectAttributes,
	PCLIENT_ID         clientId
	);

extern VOID DisableWriteProtection();
extern VOID EnableWriteProtection();
extern PTR GetCurrentKTHREAD();

#endif