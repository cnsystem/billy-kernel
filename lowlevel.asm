
[BITS 32]

INTERRUPT_STATE_SIZE equ 64

%macro Save_Registers 0
	push	eax
	push	ebx
	push	ecx
	push	edx
	push	esi
	push	edi
	push	ebp
	push	ds
	push	es
	push	fs
	push	gs
%endmacro

%macro Restore_Registers 0
	pop	gs
	pop	fs
	pop	es
	pop	ds
	pop	ebp
	pop	edi
	pop	esi
	pop	edx
	pop	ecx
	pop	ebx
	pop	eax
	add	esp, 8	; skip int num and error code
%endmacro

%macro Activate_User_Context 0

	push	esp			
	push	dword [g_currentThread]	
	call	Switch_User_Context
	add	esp, 8			
%endmacro

REG_SKIP equ (11*4)

%macro Int_With_Err 1
align 8
	push	dword %1	; push interrupt number
	jmp	Handle_Interrupt ; jump to common handler
%endmacro

%macro Int_No_Err 1
align 8
	push	dword 0		; fake error code
	push	dword %1	; push interrupt number
	jmp	Handle_Interrupt ; jump to common handler
%endmacro

IMPORT g_interruptTable
IMPORT g_currentThread
IMPORT g_needReschedule
IMPORT g_killCurrentThread
IMPORT Get_Next_Runnable
IMPORT Make_Runnable
IMPORT Switch_User_Context
EXPORT g_handlerSizeNoErr
EXPORT g_handlerSizeErr
EXPORT Load_IDTR
EXPORT Load_GDTR
EXPORT Load_LDTR
EXPORT g_entryPointTableStart
EXPORT g_entryPointTableEnd
EXPORT Switch_To_Thread
EXPORT Restore_Thread
EXPORT Get_Current_EFLAGS

[SECTION .text]
align 8
Load_IDTR:
	mov	eax, [esp+4]
	lidt	[eax]
	ret

align 8
Load_GDTR:
	mov	eax, [esp+4]
	lgdt	[eax]
	mov	ax, KERNEL_DS
	mov	ds, ax
	mov	es, ax
	mov	fs, ax
	mov	gs, ax
	mov	ss, ax
	jmp	KERNEL_CS:.here
.here:
	ret

align 8
Load_LDTR:
	mov	eax, [esp+4]
	lldt	ax
	ret

align 8
Handle_Interrupt:
	Save_Registers
	mov	ax, KERNEL_DS
	mov	ds, ax
	mov	es, ax
	mov	eax, g_interruptTable	
	mov	esi, [esp+REG_SKIP]	
	mov	ebx, [eax+esi*4]	 
	push	esp
	call	ebx
	add	esp, 4			 
	cmp	[g_needReschedule], dword 0
	je	.restore 
	cmp	[g_killCurrentThread], dword 0
	jne	.switch
	push	dword [g_currentThread]
	call	Make_Runnable
	add	esp, 4			 

.switch: 
	mov	eax, [g_currentThread]
	mov	[eax+0], esp		 
	mov	[eax+4], dword 0	 
	call	Get_Next_Runnable
	mov	[g_currentThread], eax
	mov	esp, [eax+0]		 
	mov	[g_needReschedule], dword 0
	mov	[g_killCurrentThread], dword 0

.restore:	
	Activate_User_Context 
	Restore_Registers 
	iret

align 16
Switch_To_Thread:
	push	eax		 
	mov	eax, [esp+4]	 
	mov	[esp-4], eax	 
	add	esp, 8		 
	pushfd			 
	mov	eax, [esp-4]	 
	push	dword KERNEL_CS	 
	sub	esp, 4	  
	push	dword 0
	push	dword 0 
	Save_Registers
 
	mov	eax, [g_currentThread]
	mov	[eax+0], esp 
	mov	[eax+4], dword 0 
	mov	eax, [esp+INTERRUPT_STATE_SIZE] 
	jmp	Restore_Thread

align 16
Restore_Thread: 
	mov	[g_currentThread], eax
	mov	esp, [eax+0] 
	Activate_User_Context 
	Restore_Registers 
	iret
 
align 16
Get_Current_EFLAGS:
	pushfd			 
	pop	eax		 
	ret 
	
align 8
g_entryPointTableStart: 
Int_No_Err 0
align 8
Before_No_Err:
Int_No_Err 1
align 8
After_No_Err:
Int_No_Err 2	 
Int_No_Err 3
Int_No_Err 4
Int_No_Err 5
Int_No_Err 6
Int_No_Err 7
align 8
Before_Err:
Int_With_Err 8
align 8
After_Err:
Int_No_Err 9	 
Int_With_Err 10
Int_With_Err 11
Int_With_Err 12
Int_With_Err 13
Int_With_Err 14
Int_No_Err 15	 
Int_No_Err 16
Int_With_Err 17 
%assign intNum 18
%rep (256 - 18)
Int_No_Err intNum
%assign intNum intNum+1
%endrep

align 8
g_entryPointTableEnd:

[SECTION .data]
 
align 4
g_handlerSizeNoErr: dd (After_No_Err - Before_No_Err)
align 4
g_handlerSizeErr: dd (After_Err - Before_Err)
