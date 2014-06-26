Seg_Inner_Offset	EQU		2
Seg_Outer_Offset	EQU		4
Sec_PerTrack		EQU		18 	;ÿ�ŵ�������
num_retries			db		0	;��ȡ�������

;****************************************************
;��    �ܣ�	read����
;��ڲ�����	�����ڶ�ջ�У�����Ϊ
;		 	1.������
;		 	2.������
;		 	3.��������ַBX
;���ڲ�����	��	
;****************************************************
read_sector:
	push	bp
	mov 	bp,sp
	pusha
	mov 	[num_retries],byte 0	
.loop:		
;������(AX)	
	mov 	ax,	[bp+Seg_Outer_Offset]
;����(CL)��sector=L%18	
	div		Sec_PerCylinder
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
	cmp		byte [num_retries],3	;���ֻ�ظ���ȡ3��
	jne		.loop

.done:
	popa
	pop bp
	ret
;****************read�������ܽ���********************


;****************************************************
;��    �ܣ�	ת��HEXΪ�ַ���
;��ڲ�����	1.��ַ��DS[BX]
;			2.����	CX
;���ڲ�����	ES[DI]
;****************************************************	
HexToString:
	push 	ax
	push	si
	mov 	si,bx
	xor		bx,bx
.loop1:					;���ѭ��
	dec		cx
	mov		al,ds[si+cx]
	mov		bl,al
	shr		al,4		;��4λ
	and 	bl,01111b	;��4λ
	mov 	bh,2		;�ڲ�ѭ������(2�Σ�
.loop2:					;�ڲ�ѭ��	
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
	ja		.loop2		;�ڲ�ѭ��	
	cmp 	cx,0
	ja		.loop1		;���ѭ��
	pop		ax
	pop 	si
	ret
;*************ת��HEXΪ�ַ������ܽ���****************	




;****************************************************
;��    �ܣ�	��ʾ�ַ���
;��ڲ�����	1.��ַ��DS[BX]
;			2.����	CX
;���ڲ�����	��
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
	mov 	bx,000ch	;BH=00(ҳ��Ϊ0),BL=0CH(�ڵ׺���)
	mov 	ax,1301h	;AH=13,AL=01
	mov 	dx,0000h
	int 	10h
	pop 	ax
	pop 	es
	pop		bp
	ret
;****************��ʾ�ַ������ܽ���******************

;****************************************************
;��    �ܣ�	��A20��ַ����
;��ڲ�����	��
;���ڲ�����	��
;****************************************************
Enable_A20:
	mov		al, 0xD1
	out		0x64, al
	call	Delay
	mov		al, 0xDF
	out		0x60, al
	call	Delay
	ret
;*************��A20��ַ���߹��ܽ���****************

;****************************************************
;��    �ܣ�	ʱ���ӳ�
;��ڲ�����	��
;���ڲ�����	��
;****************************************************
Delay:
	jmp		.done
.done:	
	ret
;******************�ӳٹ��ܽ���**********************

;****************************************************
;��    �ܣ�	��ʼ��8259A
;��ڲ�����	��
;���ڲ�����	��
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
;***************��ʼ��8259A���ܽ���******************
