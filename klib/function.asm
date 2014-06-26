Seg_Inner_Offset	EQU		2
Seg_Outer_Offset	EQU		4
Sec_PerTrack		EQU		18 	;每磁道扇区数
num_retries			db		0	;读取出错次数

;****************************************************
;功    能：	read扇区
;入口参数：	保存在堆栈中，依次为
;		 	1.扇区号
;		 	2.扇区数
;		 	3.缓冲区地址BX
;出口参数：	无	
;****************************************************
read_sector:
	push	bp
	mov 	bp,sp
	pusha
	mov 	[num_retries],byte 0	
.loop:		
;扇区号(AX)	
	mov 	ax,	[bp+Seg_Outer_Offset]
;扇区(CL)：sector=L%18	
	div		Sec_PerCylinder
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
	cmp		byte [num_retries],3	;最多只重复读取3次
	jne		.loop

.done:
	popa
	pop bp
	ret
;****************read扇区功能结束********************


;****************************************************
;功    能：	转换HEX为字符串
;入口参数：	1.地址：DS[BX]
;			2.长度	CX
;出口参数：	ES[DI]
;****************************************************	
HexToString:
	push 	ax
	push	si
	mov 	si,bx
	xor		bx,bx
.loop1:					;外层循环
	dec		cx
	mov		al,ds[si+cx]
	mov		bl,al
	shr		al,4		;高4位
	and 	bl,01111b	;低4位
	mov 	bh,2		;内层循环次数(2次）
.loop2:					;内层循环	
	dec		bh
	and		al,01111b	
	cmp 	al,9
	ja		.1
	add		al,'0'
	jmp 	.2
.1:
	sub		al,0Ah
	add		al,'A'
.2:
	mov		es[di],al
	mov		al,bl
	add		di,2
	cmp 	bh,0
	ja		.loop2		;内层循环	
	cmp 	cx,0
	ja		.loop1		;外层循环
	pop		ax
	pop 	si
	ret
;*************转换HEX为字符串功能结束****************	




;****************************************************
;功    能：	显示字符串
;入口参数：	1.地址：DS[BX]
;			2.长度	CX
;出口参数：	无
;****************************************************
DisplayStr:
	push 	ax
	push 	es
	push	bp
	mov		ax,ds
	mov 	es,ax
	xor		ax	
	mov 	bp,bx
	xor 	bx
	mov 	bx,000ch	;BH=00(页号为0),BL=0CH(黑底红字)
	mov 	ax,1301h	;AH=13,AL=01
	mov 	dx,0000h
	int 	10h
	pop 	ax
	pop 	es
	pop		bp
	ret
;****************显示字符串功能结束******************

;****************************************************
;功    能：	打开A20地址总线
;入口参数：	无
;出口参数：	无
;****************************************************
Enable_A20:
	mov		al, 0xD1
	out		0x64, al
	call	Delay
	mov		al, 0xDF
	out		0x60, al
	call	Delay
	ret
;*************打开A20地址总线功能结束****************

;****************************************************
;功    能：	时间延迟
;入口参数：	无
;出口参数：	无
;****************************************************
Delay:
	jmp		.done
.done:	
	ret
;******************延迟功能结束**********************

;****************************************************
;功    能：	初始化8259A
;入口参数：	无
;出口参数：	无
;****************************************************
Init_8259A:
	push ax
	; Initialize master and slave PIC!
	mov		al, ICW1
	out		0x20, al		; ICW1 to master
	call	Delay
	out		0xA0, al		; ICW1 to slave
	call	Delay
	mov		al, ICW2_MASTER
	out		0x21, al		; ICW2 to master
	call	Delay
	mov		al, ICW2_SLAVE
	out		0xA1, al		; ICW2 to slave
	call	Delay
	mov		al, ICW3_MASTER
	out		0x21, al		; ICW3 to master
	call	Delay
	mov		al, ICW3_SLAVE
	out		0xA1, al		; ICW3 to slave
	call	Delay
	mov		al, ICW4
	out		0x21, al		; ICW4 to master
	call	Delay
	out		0xA1, al		; ICW4 to slave
	call	Delay
	mov		al, 0xff		; mask all ints in slave
	out		0xA1, al		; OCW1 to slave
	call	Delay
	mov		al, 0xfb		; mask all ints but 2 in master
	out		0x21, al		; OCW1 to master
	call	Delay
	pop 	ax
	ret
;***************初始化8259A功能结束******************
