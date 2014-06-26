
BOOTSEG 	equ 0x07C0	
INITSEG 	equ 0x9000
SETUPSEG 	equ 0x9020
KERNSEG 	equ 0x1000
VIDSEG 		equ 0xb800
ICW1 		equ 0x11		
ICW2_MASTER equ 0x20		; 
ICW2_SLAVE 	equ 0x28		; 
ICW3_MASTER equ 0x04		; 
ICW3_SLAVE 	equ 0x02		; 
ICW4 		equ 0x01		; 
KERNEL_CS_Selector 		equ 1<<3		
KERNEL_DS_Selector 		equ 2<<3		
Video_Selector			equ 3<<3
KERN_THREAD_OBJ 		equ (1024*1024)
KERN_STACK 				equ KERN_THREAD_OBJ + 4096
ENTRY_POINT				

[BITS 16]
[ORG 0x0]
start_setup:
	mov		ax, SETUPSEG
	mov		ds, ax
	xor		eax, eax
	mov		ah, 0x88
	int		0x15
	add		eax, 1024	; 1024 KB == 1 MB
	mov		[mem_size_kbytes], eax
	call	Kill_Motor	
	cli	
	lidt	[IDT_Pointer]
	lgdt	[GDT_Pointer]
	call	Init_PIC
	call	Enable_A20
	mov		eax,cr0
	or		eax,1
	mov 	cr0,eax
	jmp		dword	KERNEL_CS_Selector:((SETUPSEG<<4)+setup_32)
[BITS 32]
setup_32:
	mov		ax, KERNEL_DS_Selector
	mov		ds, ax
	mov		es, ax
	mov		fs, ax
	mov		ss, ax
	mov 	ax, Video_Selector
	mov		gs, ax	
	
	mov	esp, KERN_STACK + 4096
	mov	eax, [(SETUPSEG<<4)+mem_size_kbytes]
	push	eax		; memSizeKB
	push	dword 8		; bootInfoSize
	; Pass pointer to Boot_Info struct as argument to kernel
	; entry point.
	push	esp
	; Push return address to make this look like a call
	; XXX - untested
	push	dword (SETUPSEG<<4)+.returnAddr
	; Far jump into kernel
	jmp		KERNEL_CS_Selector:ENTRY_POINT

GDT:
						Descriptor			0,			0, 			0										; ¿ÕÃèÊö·û
LABEL_DESC_FLAT_C:		Descriptor			0,			0fffffh, 	DA_CR  | DA_32 | DA_LIMIT_4K			; 0 ~ 4G
LABEL_DESC_FLAT_RW:		Descriptor			0,			0fffffh, 	DA_DRW | DA_32 | DA_LIMIT_4K			; 0 ~ 4G
LABEL_DESC_VIDEO:		Descriptor			0B8000h,	0ffffh, 	DA_DRW | DA_DPL3						; ÏÔ´æÊ×µØÖ·
GDT_SIZE 	equ ($-GDT)
GDT_Pointer:
	dw 	GDT_SIZE-1				; limit
	dd 	(SETUPSEG<<4) + GDT		; base address
IDT_Pointer:
	dw 	0
	dd 	00