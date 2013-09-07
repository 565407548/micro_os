        org 0100h
        
        jmp Label_Start
        
        %include "fat12hdr.inc"
        %include "load.inc"
        %include "pm.inc"
;; GDT
Label_Gdt:      Descriptor    0,      0,      0
Label_Desc_Flat_C:     Descriptor      0,      0fffffh,        DA_CR|DA_32|DA_LIMIT_4K
Label_Desc_Flat_RW:     Descriptor      0,      0fffffh,        DA_DRW|DA_32|DA_LIMIT_4K
Label_Desc_Video:     Descriptor      0B8000h,      0ffffh,   DA_DRW|DA_DPL_3

GdtLen  equ     $-Label_Gdt
GdtPtr  dw      GdtLen-1
        dd      Loader_Phy_Addr+Label_Gdt

;; Gdt selector
SelectorFlatC   equ     Label_Desc_Flat_C       -       Label_Gdt
SelectorFlatRW  equ     Label_Desc_Flat_RW      -       Label_Gdt
SelectorVideo   equ     Label_Desc_Video        -       Label_Gdt

BaseOfStack     equ     0100h

Label_Start:

        mov ax,cs
        mov ds,ax
        mov es,ax
        mov ss,ax
        mov sp,BaseOfStack

        
        mov al,'s'
        push ax
        call DispChar
        pop ax
        
        mov di,_MemoryInfoBuffer
        mov si,_dwMemoryARDSNumber
        push si
        push di
        call GetMemoryInfo
        pop di
        pop si

        mov ax,word[_dwMemoryARDSNumber]
        push ax
        call Disp16InHex
        pop ax
        
        ;; call GetRootDirEntryCount
        ;; call GetSectorNoOfBoot
        ;; call GetSectorNoOfFat
        ;; call GetSectorNoOfRootDir
        ;; call GetSectorNoOfDataArea

        ;; 软驱复位
        xor ah,ah
        xor dl,dl
        int 13h
        
        call GetRootDirSectors
        mov word[RootDirSectors],ax
        
        call GetSectorNoOfRootDir
        mov word[ReadingSectorNo],ax

Label_Search_Kernel_In_Root_Dir:
        cmp word[RootDirSectors],0
        jz Label_No_Kernel
        dec word[RootDirSectors]
        
        mov ax,Kernel_Seg
        mov es,ax
        mov bx,Kernel_Off
        mov ax,[ReadingSectorNo]
        mov cx,1
        
        push bx
        push es
        push cx
        push ax
        call ReadSector
        pop ax
        pop cx
        pop es
        pop bx
        
        mov si,Kernel_File_Name
        mov di,Kernel_Off

        mov cx,16               ;512/32=16 DirEntry
Label_Next_Entry_0:
        cmp cx,0
        jz Label_Goon_Next_Sector
        dec cx
        
        mov dx,11
Label_Next_Char_0:
        cmp dx,0
        jz Label_Kernel_Found
        dec dx
        
        mov al,byte[es:di]
        cmp al,byte[ds:si]
        jnz Label_Next_Entry_1
        inc di
        inc si
        jmp Label_Next_Char_0
Label_Next_Entry_1:
        mov si,Kernel_File_Name
        and di,0FFE0h
        add di,32
        jmp Label_Next_Entry_0
Label_Goon_Next_Sector:
        inc word[ReadingSectorNo]
        jmp Label_Search_Kernel_In_Root_Dir
Label_No_Kernel:
        mov al,'n'
        push ax
        call DispChar
        jmp $
Label_Kernel_Found:
        ;; Kernel found
        mov al,'k'
        push ax
        call DispChar
        pop ax
       
        and di,0FFE0h
        mov eax,[es:di+01Ch]
        mov dword[KernelSize],eax

        add di,01Ah
        mov cx,word[es:di]
        
        mov ax,Kernel_Seg
        mov es,ax
        mov bx,Kernel_Off

Label_Goon_Loading_Kernel:
        push cx                 ;保存当前读取的扇区在FATEntry中的编号
        
        mov al,'.'
        push ax
        call DispChar
        pop ax

        ;; 调用FATEntry中编号转化成数据域中的编号函数，结果保存与ax中
        push cx
        call GetLogicSectorNoFromFatSectorNo
        pop cx

        mov cx,1                ;一次读取一个扇区
        
        push bx
        push es
        push cx
        push ax
        call ReadSector
        pop ax
        pop cx
        pop es
        pop bx

        pop cx                  ;Label_Goon_Loading_Kernel中第一次入栈的cx出栈

        ;; 获得下一个要读取的扇区的FATEntry编号
        push cx
        call GetFATEntry
        pop cx
        
        cmp ax,0FFFh
        
        jz Label_File_Loaded
        mov cx,ax
        add bx,[BPB_BytsPerSec] ;由于bx为16位，所以Kernel必须小于64KB
        jmp Label_Goon_Loading_Kernel

Label_File_Loaded:
        
        mov al,'o'
        push ax
        call DispChar
        pop ax
        
        call KillMotor

        mov ax,ReadingStr
        mov cx,word[ReadingStrLength]
        push cx
        push ax
        call DispStrInRealMode1
        pop ax
        pop cx

        lgdt [GdtPtr]

        cli

        in al,92h
        or al,02h
        out 92h,al

        mov eax,cr0
        or eax,1
        mov cr0,eax
        
        jmp dword SelectorFlatC:(Loader_Phy_Addr+Label_PM_Start)
        
 ;;; 函数（实模式）
;;; int GetSectorNoOfBoot()
GetSectorNoOfBoot:
        mov ax,0
        ret

GetBootSectors:
        mov ax,word[BPB_RsvdSecCnt]
        ret
        
;; int GetSectorNoOfFAT()
GetSectorNoOfFat:
        mov ax,word[BPB_RsvdSecCnt]
        ret

GetFatSectors:
        push bx
        push dx
        
        mov ax,word[BPB_FATSz16]
        mov bl,byte[BPB_NumFATs]
        mul bl

        pop dx
        pop bx

        ret
        
;;; int GetSectorNoOfRootDir()
GetSectorNoOfRootDir:
        push bx
        push dx
        
        ;; FAT
        mov ax,word[BPB_FATSz16]
        mov bl,byte[BPB_NumFATs]
        mul bl
        ;; Boot
        add ax,word[BPB_RsvdSecCnt]

        pop dx
        pop bx

        ret

;; int GetRootDirSectors()
;; ;; ([BPB_RootEntCnt]*32+[BPB_BytsPerSec]-1)/[BPB_BytsPerSec]
;;; 每个目录项占用32字节，该函数求解目录项占用扇区数（计算结果为14扇区）
GetRootDirSectors:
        push bx
        push dx
        
        mov ax,word[BPB_RootEntCnt]
        mov bx,32               ;
        mul bx                  ;结果保存与dx:ax中
        add ax,word[BPB_BytsPerSec]
        dec ax
        mov bx,word[BPB_BytsPerSec]
        div bx

        pop dx
        pop bx
        ret
        
;; int GetSectorNoOfDataArea()
GetSectorNoOfDataArea:
        push cx

        call GetSectorNoOfRootDir
        mov cx,ax
        call GetRootDirSectors
        add ax,cx
        
        pop cx
        ret

GetRootDirEntryCount:
        mov ax,word[BPB_RootEntCnt]
        ;; mov ax,224
        ret

;;; int GetLogicSectorNoFromFatSectorNo(int ax)
GetLogicSectorNoFromFatSectorNo:
        push bp
        mov bp,sp
        push cx

        ;; mov ax,[bp+4]
        ;; add ax,1+18+14-2

        call GetSectorNoOfDataArea ;ax=33
        sub ax,2                     ;ax=31
        mov cx,[bp+4]
        add ax,cx

        pop cx
        pop bp
        ret
        
;;; void GetMemoryInfo(void *di,void *si)
;;; di指定保存ARDS信息的首地址，si指定保存ARDS个数的地址空间
GetMemoryInfo:
        push bp
        mov bp,sp

        mov si,[bp+6]
        
        mov ebx,0
        mov di,[bp+4]
.memoryInfoLoop:
        mov eax,0E820h          ;eax=0000E820h
        mov ecx,20
        mov edx,0534D4150h      ;edx='SMAP'
        int 15h
        jc .memoryInfoFail
        add di,20
        inc dword[si]
        cmp ebx,0
        jne .memoryInfoLoop
        jmp .return
.memoryInfoFail:
        mov dword[si],0
.return:
        pop bp
        ret
        
;;; void read(int ax,int cx,void *es,void *bx)
;;; 从ax扇区开始，读取cx个扇区到内存地址es：bx上
ReadSector:
        push bp
        mov bp,sp
        push cx
        push dx

        mov ax,[bp+4]
        mov bl,byte[BPB_SecPerTrk]
        div bl
        inc ah
        mov cl,ah
        mov dh,al
        shr al,1
        mov ch,al
        and dh,1
        mov dl,[BS_DrvNum]
.GoOnReading:
        mov al,byte[bp+6]
        mov ah,2
        mov es,[bp+8]
        mov bx,[bp+10]
        int 13h
        
        jc .GoOnReading

        pop dx
        pop cx 
        pop bp
        ret

;;; int GetFATEntry(int ax)
;;; 文件的当前扇区序号为ax（在FAT中序号），获得其下一个扇区号。临时读如内存的数据存放于es:bx中
GetFATEntry:
        push bp
        mov bp,sp
        push bx
        push cx
        push dx
        push es


        
        mov ax,[bp+4]
        mov bx,3
        mul bx
        mov bx,2
        div bx                  ;此时，ax保存的是对应FATEntry的整体字节偏移量

        xor dx,dx               ;开始少了该句，当FATEntry为偶数时，可以正确得到结果；但是如果是奇数，dx就不为零，以下部分数据都会出问题，所以一定要注意乘法/除法运算
        mov bx,[BPB_BytsPerSec] ;ax/512
        div bx
        push dx                 ;ax扇区数，dx字节偏移

        mov cx,ax
        mov ax,BufferArea_Seg
        mov es,ax               ;
        mov bx,BufferArea_Off   ;
        call GetSectorNoOfFat
        add ax,cx
        mov cx,2
        
        push bx
        push es
        push cx
        push ax
        call ReadSector
        pop ax
        pop cx
        pop es
        pop bx

        pop dx
        add bx,dx
        mov ax,word[es:bx]

        ;; push ax
        ;; call Disp16InHex
        ;; pop ax
        
        mov cx,[bp+4]
        test cx,1               ;cx&1
        jz .even
        shr ax,4
        jmp .return
.even:
        and ax,0fffh
.return:
        pop es
        pop dx
        pop cx
        pop bx
        pop bp
        ret

;;; int GetStringLength(void *str)
GetStringLength:
        push bp
        mov bp,sp
        push cx
        push si
        
        mov cx,0
        mov si,[bp+4]
.loop: 
        mov al,[si]
        test al,al
        jz .return
        inc si
        jmp .loop
.return:
        mov ax,cx

        push si
        pop cx
        pop bp
        ret
        
;;; DispStrInRealMode(void *str,int length)
DispStrInRealMode1:
        push di
        mov di,sp
        push es
        push bp
        push cx
        push bx
        push dx
        push ax

        mov ax,ds
        mov es,ax
        mov bp,[di+4]
        mov cx,[di+6]
        mov ax,01301h
        mov bx,0007h
        mov dl,0
        add dh,3
        int 10h

        pop ax
        pop dx
        pop bx
        pop cx
        pop bp
        pop es
        pop di
        ret
        
;;; void DispStrInReadMode2(void *str)
DispStrInRealMode2:
        push bp
        mov bp,sp
        push ax
        push cx
        push si
        
        mov si,[bp+4]
        push si
        call GetStringLength
        mov cx,ax
        pop si

        push cx
        mov ax,[bp+4]
        push ax
        call DispStrInRealMode1
        pop ax
        pop cx

        pop si
        pop cx
        pop ax
        pop bp
        ret
        
        
;; ;;; DispChar(char al)
DispChar:
        push bp
        mov bp,sp
        push ax
        push bx

        mov al,byte[bp+4]
        mov ah,0Eh
        mov bl,0Fh
        int 10h
        
        pop bx
        pop ax
        pop bp
        ret

;;; 以十六进制方式输出al的值
;;; ah=0Eh, al=输出值, bh=page, bl=color, int 10h
;;; 输入：
;;;     al=待输出的内容
;;; void Disp8InHex(int al)
Disp8InHex:
        push bp
        mov bp,sp
        push ax
        push cx
        push dx
        
        mov ah,0Eh
        mov bl,0Fh
        
        mov al,byte[bp+4]
        mov dl,al
        mov cx,2
        shr al,4
.2:
        and al,0Fh
        add al,'0'
        cmp al,'9'
        jle .1
        add al,7       ;65-48-10='A'-'0'-10=7
.1:
        int 10h

        mov al,dl
        loop .2
        
        pop dx
        pop cx
        pop ax
        pop bp
        ret

;;; 以16进制的方式输出AX的值
;;; ah=0Eh, al=输出值, bh=page, bl=color, int 10h
;;; 输入：
;;;     ax=待输出的内容
;;; void Disp16InHex(int ax)
Disp16InHex:
        push bp
        mov bp,sp

        mov ax,word[bp+4]
        shr ax,8
        push ax
        call Disp8InHex
        pop ax
        
        mov ax,[bp+4]
        push ax
        call Disp8InHex
        pop ax
        
        pop bp
        ret
        
;;; void KillMotor()
KillMotor:
        push ax
        push dx
       
        mov dx,03F2h
        mov al,0
        out dx,al
        
        pop dx
        pop ax
        ret
        
;; ;;; 变量
        RootDirSectors       dw      0
        ReadingSectorNo         dw      0
        KernelSize              dd      0
        
;; ;;; 字符串
        Kernel_File_Name  db      "KERNEL  BIN"
        TestStr db "Test"
        TestLength dw 4
        ReadingStr db "Reading"
        ReadingStrLength dw 7 

;;;当省略以下三行的时候，屏幕会不停的闪硕。为什么会出现这样的情况呢？
[SECTION .s32]
ALIGN 32
[BITS 32]     
Label_PM_Start:
        mov ax,SelectorVideo
        mov gs,ax
        mov ax,SelectorFlatRW
        mov ds,ax
        mov es,ax
        mov fs,ax
        mov ss,ax
        mov esp,TopOfStack

        call GetMemorySize

        mov eax,[dwMemorySize]
        push eax
        call DispInt
        pop eax

        push ProcBar
        call DispInt
        add esp,4
        
        ;; call SetupPaging
        call PagingDemo

        mov ah,0Fh
        mov al,'P'
        mov [gs:((80*0+39)*2)],ax

        call InitKernel
        ;; jmp $
        ;; jmp SelectorFlatC:Kernel_Phy_Addr
        jmp SelectorFlatC:KernelEntryPointPhyAddr

;;; void GetMemorySize()
GetMemorySize:
        push esi
        push ecx
        push edx
        push edi
        mov esi,MemoryInfoBuffer
        mov ecx,[dwMemoryARDSNumber]
.loop:
        mov edx,5
        mov edi,ARDSStruct
.1:
        push dword[esi]
        call DispInt
        pop eax

        stosd                   ;[es:edi]=eax,edi自增4
        add esi,4
        dec edx
        jnz .1

        call DispReturn

        cmp dword[dwType],1
        jne .2
        mov eax,[dwBaseAddrLow]
        add eax,[dwLengthLow]
        cmp eax,[dwMemorySize]
        jb .2
        mov [dwMemorySize],eax
.2:
        loop .loop

        pop edi
        pop edx
        pop ecx
        pop esi
        ret

DispAL:
        push edi
        push eax
        push ecx
        push edx
       
        mov edi,[dwDisplayPosition]
        mov ah,0Fh
        mov dl,al
        shr al,4
        mov ecx,2
.begin:
        and al,0Fh
        add al,'0'
        cmp al,'9'
        ja .1
        jmp .2
.1:
        add al,7                ;al=al-'0'-10+'A'=al-48-10+65=al+7
.2:
        mov [gs:edi],ax
        add edi,2
        mov al,dl
        loop .begin

        mov [dwDisplayPosition],edi

        pop edx
        pop ecx
        pop eax
        pop edi
        ret

;;; void DispInt(int eax)
DispInt:
        mov eax,[esp+4]
        shr eax,24
        call DispAL

        mov eax,[esp+4]
        shr eax,16
        call DispAL

        mov eax,[esp+4]
        shr eax,8
        call DispAL

        mov eax,[esp+4]
        call DispAL

        mov ah,07h
        mov al,'h'
        push edi
        mov edi,[dwDisplayPosition]
        mov [gs:edi],ax
        add edi,4
        mov [dwDisplayPosition],edi
        pop edi
        
        ret
        
;;; void DispStr(void *str)
DispStr:
        push ebp
        mov ebp,esp
        push ebx
        push edi
        push esi

        mov esi,[ebp+8]
        mov edi,[dwDisplayPosition]
        mov ah,0Fh

.1:
        lodsb                   ;al=[ds:esi],esi自增4
        test al,al
        jz .2
        cmp al,0Ah
        jnz .3
        push eax
        mov eax,edi
        mov bl,160
        div bl
        and eax,0FFh
        inc eax
        mov bl,160
        mul bl
        mov edi,eax
        pop eax
        jmp .1
.3:
        mov [gs:edi],ax
        add edi,2
        jmp .1
.2:
        mov [dwDisplayPosition],edi
        
        pop esi
        pop edi
        pop ebx
        pop ebp
        ret
        
DispReturn:
        push strReturn
        call DispStr
        add esp,4
        ret

;;; void *MemoryCopy(void *dest,void *source,int size)
MemoryCopy:
        push ebp
        mov ebp,esp

        push ecx
        push edi
        push esi
        
        mov edi,[ebp+8]
        mov esi,[ebp+12]
        mov ecx,[ebp+16]

.1:
        cmp ecx,0
        jz .2

        mov al,[ds:esi]
        inc esi

        mov byte[es:edi],al
        inc edi
        dec ecx
        jmp .1
.2: 
        mov eax,[ebp+8]

        pop esi
        pop edi
        pop ecx
        mov esp,ebp
        pop ebp    
        ;;;当栈操作出问题的时候，屏幕会出现闪烁。
        
        ret
        
SetupPaging:
        ;; (edx:eax)/ebx
        xor edx,edx
        mov eax,[dwMemorySize]
        mov ebx,400000h         ;4M
        div ebx

        mov ecx,eax
        test edx,edx
        jz .no_remainder
        inc ecx
.no_remainder: 
        push ecx

        mov ax,SelectorFlatRW
        mov es,ax
        mov edi,PageDirBase
        xor eax,eax
        mov eax,PageTableBase|PG_P|PG_US_U|PG_RW_W
.1: 
        stosd                   ;[es:edi]=eax,edi 自增4
        add eax,4096
        loop .1

        pop eax
        mov ebx,1024
        mul ebx
        mov ecx,eax
        mov edi,PageTableBase
        xor eax,eax
        mov eax,PG_P|PG_US_U|PG_RW_W
.2: 
        stosd                   ;[es:edi]=eax,edi 自增4
        add eax,4096
        loop .2

        mov eax,PageDirBase
        mov cr3,eax
        mov eax,cr0
        or eax,80000000h
        mov cr0,eax
        jmp short .3

.3:
        nop 
        ret
        
InitKernel:
        push ecx
        push esi
        
        xor esi,esi
        mov cx,word[Kernel_Phy_Addr+2Ch]
        movzx ecx,cx
        mov esi,[Kernel_Phy_Addr+1Ch]
        add esi,Kernel_Phy_Addr

.Begin:
        mov eax,[esi+0]
        cmp eax,0
        jz .NoAction
        push dword[esi+010h]
        mov eax,[esi+04h]
        add eax,Kernel_Phy_Addr
        push eax
        push dword[esi+08h]
        call MemoryCopy
        add esp,12        
.NoAction: 
        add esi,020h
        dec ecx
        jnz .Begin

        pop esi
        pop ecx
        ret
        
PageSwitch:
        ;; (edx:eax)/ebx
        xor edx,edx
        mov eax,[dwMemorySize]
        mov ebx,400000h         ;4M
        div ebx

        mov ecx,eax
        test edx,edx
        jz .no_remainder
        inc ecx
.no_remainder: 
        push ecx

        mov ax,SelectorFlatRW
        mov es,ax
        mov edi,PageDirBase1
        xor eax,eax
        mov eax,PageTableBase1|PG_P|PG_US_U|PG_RW_W
.1: 
        stosd                   ;[es:edi]=eax,edi 自增4
        add eax,4096
        loop .1

        pop eax
        mov ebx,1024
        mul ebx
        mov ecx,eax
        mov edi,PageTableBase1
        xor eax,eax
        mov eax,PG_P|PG_US_U|PG_RW_W
.2: 
        stosd                   ;[es:edi]=eax,edi 自增4
        add eax,4096
        loop .2

        mov eax,LinearAddrDemo
        shr eax,22
        mov ebx,4096
        mul ebx
        mov eax,LinearAddrDemo
        shr eax,12
        and eax,03FFh
        mov ebx,4
        mul ebx
        add eax,ecx
        add eax,PageTableBase1
        ;; mov dword [es:eax],ProcBar|PG_P|PG_US_U|PG_RW_W
        mov dword [es:eax],PG_P|PG_US_U|PG_RW_W
        
        mov eax,PageDirBase1
        mov cr3,eax
        mov eax,cr0
        or eax,80000000h
        mov cr0,eax
        jmp short .3

.3:
        nop 
        ret

PagingDemo:
        ;; mov ax,cs
        ;; mov ds,ax
        ;; mov ax,SelectorFlatRW
        ;; mov es,ax

        push LenFoo
        push OffsetFoo
        push ProcFoo
        call MemoryCopy
        add esp,12

        push LenBar
        push OffsetBar
        push ProcBar
        call MemoryCopy
        add esp,12

        push LenPagingDemoProc
        push OffsetPagingDemoProc
        push ProcPagingDemo
        call MemoryCopy
        add esp,12

        ;; mov ax,SelectorData
        ;; mov ds,ax
        ;; mov es,ax

        ;; call SetupPaging

        
        ;; call SelectorFlatC:ProcPagingDemo
        ;; call PageSwitch
        ;; call SelectorFlatC:ProcPagingDemo
        ret
        
PagingDemoProc:
OffsetPagingDemoProc    equ     PagingDemoProc-$$
        mov eax,LinearAddrDemo
        call eax
        retf
LenPagingDemoProc       equ     $-PagingDemoProc

Foo:
OffsetFoo       equ     Foo-$$
        push strFoo
        call DispStr
        add esp,4
        ret
LenFoo          equ     $-Foo
Bar:
OffsetBar       equ     Bar-$$
        push strBar
        call DispStr
        add esp,4
        ret
LenBar          equ     $-Bar
        
[SECTION .data1]
ALIGN 32
Label_Data:
;;; 实模式下使用
_strReturn: dd 0Ah,0
_strFoo: dd "foo",0
_strBar: dd "bar",0
_dwMemoryARDSNumber: dd 0
_dwDisplayPosition: dd (80*6+0)*2
_dwMemorySize: dd 0
_ARDSStruct:
	_dwBaseAddrLow: dd 0
	_dwBaseAddrHigh: dd 0
	_dwLengthLow: dd 0
	_dwLengthHigh: dd 0
	_dwType: dd 0
_MemoryInfoBuffer: times 256 db 0
_LinearAddrDemo:
_ProcFoo:           times 256 db 0
_ProcBar:           times 256 db 0
_ProcPagingDemo     times 256 db 0
;;;保护模式下使用
        strReturn               equ     Loader_Phy_Addr+_strReturn
        strFoo                  equ     Loader_Phy_Addr+_strFoo
        strBar                  equ     Loader_Phy_Addr+_strBar
        dwMemoryARDSNumber      equ     Loader_Phy_Addr+_dwMemoryARDSNumber
        dwDisplayPosition       equ     Loader_Phy_Addr+_dwDisplayPosition
        dwMemorySize            equ     Loader_Phy_Addr+_dwMemorySize
        ARDSStruct              equ     Loader_Phy_Addr+_ARDSStruct
	        dwBaseAddrLow           equ     Loader_Phy_Addr+_dwBaseAddrLow
	        dwBaseAddrHigh          equ     Loader_Phy_Addr+_dwBaseAddrHigh
	        dwLengthLow             equ     Loader_Phy_Addr+_dwLengthLow
	        dwLengthHigh            equ     Loader_Phy_Addr+_dwLengthHigh
	        dwType                  equ     Loader_Phy_Addr+_dwType
        MemoryInfoBuffer        equ     Loader_Phy_Addr+_MemoryInfoBuffer
        LinearAddrDemo          equ     Loader_Phy_Addr+_LinearAddrDemo
        ProcFoo                 equ     Loader_Phy_Addr+_ProcFoo
        ProcBar                 equ     Loader_Phy_Addr+_ProcBar
        ProcPagingDemo          equ     Loader_Phy_Addr+_ProcPagingDemo
        
        StackSpace      times   1000h   db 0
        TopOfStack      equ     Loader_Phy_Addr+$
;;; SECTION .data1结束
        
        
        
        
        
        
