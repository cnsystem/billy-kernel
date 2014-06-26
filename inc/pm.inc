;************************************************
;描述符 64bit
;用法:	Descriptor base,limit,attr
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
;门描述符 64bit
;用法:	Descriptor selector,offset,attr1,attr2
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
; 选择子类型值说明
; 其中:
;       SA_  : Selector Attribute
;************************************************
SA_RPL0		EQU	0	; ┓
SA_RPL1		EQU	1	; ┣ RPL
SA_RPL2		EQU	2	; ┃
SA_RPL3		EQU	3	; ┛

SA_TIG		EQU	0	; ┓TI
SA_TIL		EQU	4	; ┛
;******************-----END-----*****************
;************************************************
; 在下列类型值命名中：
;       DA_  : Descriptor Attribute
;       D    : 数据段
;       C    : 代码段
;       S    : 系统段
;       R    : 只读
;       RW   : 读写
;       A    : 已访问
;       其它 : 可按照字面意思理解
;************************************************
; 描述符类型
DA_32		EQU	4000h	; 32 位段

DA_DPL0		EQU	00h	; DPL = 0
DA_DPL1		EQU	20h	; DPL = 1
DA_DPL2		EQU	40h	; DPL = 2
DA_DPL3		EQU	60h	; DPL = 3

; 存储段描述符类型
DA_DR		EQU	90h	; 存在的只读数据段类型值
DA_DRW		EQU	92h	; 存在的可读写数据段属性值
DA_DRWA		EQU	93h	; 存在的已访问可读写数据段类型值
DA_C		EQU	98h	; 存在的只执行代码段属性值
DA_CR		EQU	9Ah	; 存在的可执行可读代码段属性值
DA_CCO		EQU	9Ch	; 存在的只执行一致代码段属性值
DA_CCOR		EQU	9Eh	; 存在的可执行可读一致代码段属性值

; 系统段描述符类型
DA_LDT		EQU	82h	; 局部描述符表段类型值
DA_TaskGate	EQU	85h	; 任务门类型值
DA_386TSS	EQU	89h	; 可用 386 任务状态段类型值
DA_386CGate	EQU	8Ch	; 386 调用门类型值
DA_386IGate	EQU	8Eh	; 386 中断门类型值
DA_386TGate	EQU	8Fh	; 386 陷阱门类型值
;******************-----END-----*****************

;描述符段基址修改
;有语法错误
;用法：	setBase desriptor,base
;		descriptor	address
;		base	dd (24+8 bit)
;%macro setBase %1,%2
;	MOV	WORD [%1+2],%2&0FFFFH
;	MOV BYTE [%1+4],(%2>>16)&0FFH
;	MOV BYTE [%1+7],(%2>>24)&0FFH
;%endmacro