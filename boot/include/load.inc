;通用缓冲区4K
;BufferArea 4K
BufferArea_Seg equ 0x8000-0x200
BufferArea_Off equ 0
;BufferArea_Limit equ 0x2000
BufferArea_Phy_Addr  equ BufferArea_Seg*0x10

;Kernel 64K
Kernel_Seg equ 0x8000
Kernel_Off equ 0
;Kernel_Limit equ 0x10000
Kernel_Phy_Addr equ Kernel_Seg*0x10

;Loader 64K
Loader_Seg equ 0x9000
Loader_Off equ 0x100      ;Loader_Off=0x100,如果设置成0，会出错
;Loader_Limit equ 0x10000
Loader_Phy_Addr equ Loader_Seg*0x10

KernelEntryPointPhyAddr equ 030400h

PageDirBase equ 200000h     ;2M
PageTableBase equ 201000h   ;2M+4K
PageDirBase1		equ	210000h	; 页目录开始地址:	2M + 64K
PageTableBase1		equ	211000h	; 页表开始地址:		2M + 64K + 4K

;LinearAddrDemo	equ	00401000h
;ProcFoo		equ	00401000h
;ProcBar		equ	00501000h
;ProcPagingDemo	equ	00301000h
