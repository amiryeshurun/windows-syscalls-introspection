.model flat, stdcall
.code

DisableWriteProtection proc
	mov eax, cr0
	and eax, 0ffff7fffh
	mov cr0, eax
	ret
DisableWriteProtection endp

EnableWriteProtection proc
	mov eax, cr0
	or eax, 0ffff7fffh
	mov cr0, eax
	ret
EnableWriteProtection endp

GetCurrentKTHREAD proc
	ASSUME FS:NOTHING
	mov eax, fs:[124h]
	ASSUME FS:ERROR
	ret
GetCurrentKTHREAD endp
end