;************************************************
;������ 64bit
;�÷�:	Descriptor base,limit,attr
;		base 	dd (24+8 bit)
;		limit 	dd (16+4 bit)
;		attr 	dw (8+4 bit)
;************************************************
%macro Descriptor 3
	dw %2&0FFFFH					;limit
	dw %1&0FFFFH					;base
	db %1>>16&0FFH					;base
	db %3&0FFH						;attr
	db (%3>>8&0F0H)|(%2>>16&0FH)	;attr,limit
	db %1>>24&0FF					;base
%endmacro	
;******************-----END-----*****************
;************************************************
;�������� 64bit
;�÷�:	Descriptor selector,offset,attr1,attr2
;		selector	dw	(16 bit)
;		offset 		dq 	(16+16 bit)
;		attr1 		db 	(8 bit)
;		attr2		db 	(8 bit)
;************************************************
%macro Gate 4
	dw	(%2&0FFFFh)							;offset
	dw	%1									;selector
	dw	(%3 & 1Fh) | ((%4 << 8) & 0FF00h)	;attr
	dw	((%2 >> 16) & 0FFFFh)				;offset
%endmacro 
;******************-----END-----*****************
;************************************************
; ѡ��������ֵ˵��
; ����:
;       SA_  : Selector Attribute
;************************************************
SA_RPL0		EQU	0	; ��
SA_RPL1		EQU	1	; �� RPL
SA_RPL2		EQU	2	; ��
SA_RPL3		EQU	3	; ��

SA_TIG		EQU	0	; ��TI
SA_TIL		EQU	4	; ��
;******************-----END-----*****************
;************************************************
; ����������ֵ�����У�
;       DA_  : Descriptor Attribute
;       D    : ���ݶ�
;       C    : �����
;       S    : ϵͳ��
;       R    : ֻ��
;       RW   : ��д
;       A    : �ѷ���
;       ���� : �ɰ���������˼����
;************************************************
; ����������
DA_32		EQU	4000h	; 32 λ��

DA_DPL0		EQU	00h	; DPL = 0
DA_DPL1		EQU	20h	; DPL = 1
DA_DPL2		EQU	40h	; DPL = 2
DA_DPL3		EQU	60h	; DPL = 3

; �洢������������
DA_DR		EQU	90h	; ���ڵ�ֻ�����ݶ�����ֵ
DA_DRW		EQU	92h	; ���ڵĿɶ�д���ݶ�����ֵ
DA_DRWA		EQU	93h	; ���ڵ��ѷ��ʿɶ�д���ݶ�����ֵ
DA_C		EQU	98h	; ���ڵ�ִֻ�д��������ֵ
DA_CR		EQU	9Ah	; ���ڵĿ�ִ�пɶ����������ֵ
DA_CCO		EQU	9Ch	; ���ڵ�ִֻ��һ�´��������ֵ
DA_CCOR		EQU	9Eh	; ���ڵĿ�ִ�пɶ�һ�´��������ֵ

; ϵͳ������������
DA_LDT		EQU	82h	; �ֲ���������������ֵ
DA_TaskGate	EQU	85h	; ����������ֵ
DA_386TSS	EQU	89h	; ���� 386 ����״̬������ֵ
DA_386CGate	EQU	8Ch	; 386 ����������ֵ
DA_386IGate	EQU	8Eh	; 386 �ж�������ֵ
DA_386TGate	EQU	8Fh	; 386 ����������ֵ
;******************-----END-----*****************

;�������λ�ַ�޸�
;���﷨����
;�÷���	setBase desriptor,base
;		descriptor	address
;		base	dd (24+8 bit)
;%macro setBase %1,%2
;	MOV	WORD [%1+2],%2&0FFFFH
;	MOV BYTE [%1+4],(%2>>16)&0FFH
;	MOV BYTE [%1+7],(%2>>24)&0FFH
;%endmacro