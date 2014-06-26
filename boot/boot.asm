Start:
Seg_Inner_Offset	EQU		2
Seg_Outer_Offset	EQU		4
Sec_PerTrack		EQU		18 	;ÿ�ŵ�������

num_retries			db		0	;��ȡ�������
BPB_Byte_PerSec		DW		512	;ÿ�����ֽ���
BPB_Sec_PerTrack	DW		18 	;ÿ�ŵ�������
BPB_Trk_PerHead		DW		80	;ÿ��ͷ�ŵ���
BPB_NumHead			DW		2 	;��ͷ��

BPB_FATSz16			DW		9	;ÿ��FAT����ռ������
BPB_NumFATs			DW 		2	;FAT�����

BPB_TotalSec		DW		2880;��������

BPB_RootEntCnt		DW 		224;

FileEN_Len			EQU 	32	;��Ŀ¼��Ŀ����
Clus_Offset			EQU		0x1A;������ַƫ��
FATSeg				EQU		0x7E00;
DirSeg				EQU		0x9000;
LoadSeg				EQU		0x8000;
RootBytes			DW		0	;��Ŀ¼��ռ�ֽ�BPB_RootEntCnt*32
RootSectors			DW		0	;��Ŀ¼��ռ������(RootBytes+BPB_Byte_PerSec-1)/BPB_Byte_PerSec

[bits 16]
[section .text]
[org 0x7c00]
start:
	mov	ax,	cs
	mov	ds,	ax
	mov es,	ax
	mov ss,	ax
	mov sp,	StackTop
	mov bp,	StackBase
	
	xor ax,	ax
	mov	ax, word BPB_RootEntCnt
	and ax,	0Fh
	mov bx,	word FileEN_Len
	xor	bx,	0Fh
	mul	bl
	mov [RootBytes], ax
	add	ax,	BPB_Byte_PerSec
	sub ax,	1
	mov bx,	word BPB_Byte_PerSec
	div bx
	mov RootSectors,ax
	call LoadFAT
	call LoadDirectory
	
	

;************************************************
;��    �ܣ�	����FAT��7E00~9000H
;��ڲ�����	��		 			 	
;���ڲ�����	��	
;************************************************
LoadFAT:
	mov ax,	ds
	mov es,	ax
	mov bx,	DirSeg
	xor ax,	ax
	mov	ax,	1
	add	ax,	BPB_FATSz16
	add ax,	BPB_FATSz16
	push ax
	xor	ax,	ax
	mov ax,	BPB_FATSz16
	push ax
	push bx
	call read_sector
	add	sp,	6
	ret	
;************************************************
;��    �ܣ�	���ظ�Ŀ¼��9000H
;��ڲ�����	��		 			 	
;���ڲ�����	��	
;************************************************
LoadDirectory:
	mov ax,	ds
	mov es,	ax
	mov bx,	FATSeg
	xor ax,	ax
	mov	ax,	1
	push ax
	xor	ax,	ax
	mov ax,	RootSectors
	push ax
	push bx
	call read_sector
	add	sp,	6
	ret	
;************************************************
;��    �ܣ�	�����ļ�
;��ڲ�����	�����ڶ�ջ�У�����Ϊ
;		 	1.�ļ���		 	
;���ڲ�����	�ļ���һ��������AX
;************************************************
SearchFile:
	push	bp
	mov 	bp,sp	
	mov 	si,	[bp+2]
	mov 	di,	DirSeg
	cld 
	mov 	dx,	BPB_RootEntCnt
.loop:
	cmp		dx,	0
	jz		.FileNoFound
	
.CompareName:
	cmp		cx,	0
	jz		.Done
	lodsb
	dec		cx
	cmp 	al,	[es:di]
	jnz		.Different
	inc		di
	jmp		.CompareName
.Different:
	dec		dx
	and 	di,	0xFFFE0
	add 	di,	0x20
	mov 	cx,	11
	jmp		.loop
FileNoFound:
	mov 	ax, 0
	pop		bp
	ret		
.Done
	and 	di,	0xFFFE0
	add 	di,	Clus_Offset
	mov 	ax,	word[di]
	pop 	bp
	ret
;************************************************
;��    �ܣ�	����FAT��Ŀ
;��ڲ�����	��ĿID	AX
;���ڲ�����	������	AX
;************************************************
GetFATEN:
	;������Ҫʹ�õļĴ���ѹջ
	push 	bx
	push 	dx
	push 	cx
	push 	es
	;ax*1.5
	mov 	bx,	3
	mul 	bx
	mov 	bx,	2
	div 	bx
	;ES:BX->FAT��
	les		bx,	FATSeg
	;����FAT�����ڵ�ַƫ��
	add		bx,	ax
	mov		ax,	word [es:bx]
	
	cmp 	dx,	1
	jz		.odd
	
.even:					;ż������ЧλΪ��12λ
	and  	ax,	0x0FFF
	jmp 	.done
.odd:					;��������4λΪǰһFAT���Ч��������λ
	shr		ax,	4
.done:
	pop cx
	pop dx
	pop bx	
	pop es
	ret
;************************************************
;��    �ܣ�	LoadFile
;��ڲ�����	��ʼ������	AX
;���ڲ�����	��
;************************************************
LoadFile:
	push	ds
	push	cx
	push	bx
	push 	ax
	
	mov 	cx,	1;һ���Զ�ȡ������
	lds		bx,	LoadSeg;�ļ�Loadλ��
.loop:
	;read_sector�����Ĳ���ѹջ
	push 	ax
	push	cx
	push 	bx
	call read_sector
	;�����ջ����
	add		sp,	6
	;����FAT����ȡ��һ������
	call GetFATEN
	cmp  ax,0xFF8
	jnb	 .over
	add	 bx,	BPB_Byte_PerSec

	jmp .loop
.over:
	pop cx
	pop ax
	pop bx	
	pop ds 	
	ret
;************************************************
;��    �ܣ�	read����
;��ڲ�����	�����ڶ�ջ�У�����Ϊ
;		 	1.������
;		 	2.������
;		 	3.��������ַBX
;���ڲ�����	��	
;************************************************
read_sector:
	push	bp
	mov 	bp,sp
	pusha
	mov 	[num_retries],byte 0	
.loop:		
;������(AX)	
	mov 	ax,	[bp+Seg_Outer_Offset]
;����(CL)��sector=L%18	
	div		Sec_PerTrack
	mov 	cl,al
;����(CH)��cylinder=L/18/2
	mov 	al,ah
	xor 	ah,ah
	div 	2
	mov 	ch,ah
;��ͷ(DH): head=L/18%2
	mov 	dh,al
;��ȡ������(AL)
	xor 	ax,ax
	mov 	ax,[bp+Seg_Outer_Offset+2]	
;���ܺ�(AH):02H
	xor 	ah,ah
	mov 	ah,02h
;������(DL):00h
	mov 	dl,0
;������(BX)
	mov 	bx,[bp+Seg_Outer_Offset+4]

	int 	13h
	
	jnc 	.done
	inc		byte [num_retries]
	cmp		byte [num_retries],3;���ֻ�ظ���ȡ3��
	jne		.loop

.done:
	popa
	pop bp
	ret
;****************read�������ܽ���********************
LoaderFile	DB 	"Loader.bin"
KernelFile	DB	"Kernel.bin"
StackTop:
	times (510-($-Start)) db 0
StackBase:
	dw 0xaa55