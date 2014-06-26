Start:
Seg_Inner_Offset	EQU		2
Seg_Outer_Offset	EQU		4
Sec_PerTrack		EQU		18 	;每磁道扇区数

num_retries			db		0	;读取出错次数
BPB_Byte_PerSec		DW		512	;每扇区字节数
BPB_Sec_PerTrack	DW		18 	;每磁道扇区数
BPB_Trk_PerHead		DW		80	;每磁头磁道数
BPB_NumHead			DW		2 	;磁头数

BPB_FATSz16			DW		9	;每个FAT表所占扇区数
BPB_NumFATs			DW 		2	;FAT表个数

BPB_TotalSec		DW		2880;总扇区数

BPB_RootEntCnt		DW 		224;

FileEN_Len			EQU 	32	;根目录条目长度
Clus_Offset			EQU		0x1A;扇区地址偏移
FATSeg				EQU		0x7E00;
DirSeg				EQU		0x9000;
LoadSeg				EQU		0x8000;
RootBytes			DW		0	;根目录所占字节BPB_RootEntCnt*32
RootSectors			DW		0	;根目录所占扇区数(RootBytes+BPB_Byte_PerSec-1)/BPB_Byte_PerSec

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
;功    能：	加载FAT表到7E00~9000H
;入口参数：	无		 			 	
;出口参数：	无	
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
;功    能：	加载根目录表到9000H
;入口参数：	无		 			 	
;出口参数：	无	
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
;功    能：	搜索文件
;入口参数：	保存在堆栈中，依次为
;		 	1.文件名		 	
;出口参数：	文件第一个扇区号AX
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
;功    能：	搜索FAT条目
;入口参数：	条目ID	AX
;出口参数：	扇区号	AX
;************************************************
GetFATEN:
	;将部分要使用的寄存器压栈
	push 	bx
	push 	dx
	push 	cx
	push 	es
	;ax*1.5
	mov 	bx,	3
	mul 	bx
	mov 	bx,	2
	div 	bx
	;ES:BX->FAT段
	les		bx,	FATSeg
	;加上FAT项所在地址偏移
	add		bx,	ax
	mov		ax,	word [es:bx]
	
	cmp 	dx,	1
	jz		.odd
	
.even:					;偶数，有效位为低12位
	and  	ax,	0x0FFF
	jmp 	.done
.odd:					;奇数，低4位为前一FAT项，无效，向右移位
	shr		ax,	4
.done:
	pop cx
	pop dx
	pop bx	
	pop es
	ret
;************************************************
;功    能：	LoadFile
;入口参数：	起始扇区号	AX
;出口参数：	无
;************************************************
LoadFile:
	push	ds
	push	cx
	push	bx
	push 	ax
	
	mov 	cx,	1;一次性读取扇区数
	lds		bx,	LoadSeg;文件Load位置
.loop:
	;read_sector函数的参数压栈
	push 	ax
	push	cx
	push 	bx
	call read_sector
	;清理堆栈参数
	add		sp,	6
	;查找FAT表，获取下一个扇区
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
;功    能：	read扇区
;入口参数：	保存在堆栈中，依次为
;		 	1.扇区号
;		 	2.扇区数
;		 	3.缓冲区地址BX
;出口参数：	无	
;************************************************
read_sector:
	push	bp
	mov 	bp,sp
	pusha
	mov 	[num_retries],byte 0	
.loop:		
;扇区号(AX)	
	mov 	ax,	[bp+Seg_Outer_Offset]
;扇区(CL)：sector=L%18	
	div		Sec_PerTrack
	mov 	cl,al
;柱面(CH)：cylinder=L/18/2
	mov 	al,ah
	xor 	ah,ah
	div 	2
	mov 	ch,ah
;柱头(DH): head=L/18%2
	mov 	dh,al
;读取扇区数(AL)
	xor 	ax,ax
	mov 	ax,[bp+Seg_Outer_Offset+2]	
;功能号(AH):02H
	xor 	ah,ah
	mov 	ah,02h
;驱动器(DL):00h
	mov 	dl,0
;缓冲区(BX)
	mov 	bx,[bp+Seg_Outer_Offset+4]

	int 	13h
	
	jnc 	.done
	inc		byte [num_retries]
	cmp		byte [num_retries],3;最多只重复读取3次
	jne		.loop

.done:
	popa
	pop bp
	ret
;****************read扇区功能结束********************
LoaderFile	DB 	"Loader.bin"
KernelFile	DB	"Kernel.bin"
StackTop:
	times (510-($-Start)) db 0
StackBase:
	dw 0xaa55