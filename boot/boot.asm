        org 07c00h
        
        BaseOfStack     equ     07c00h

        %include "load.inc"

        ;; 以下整体结构参考FAT12引导扇区格式说明（P104）
        jmp short LABEL_START
        nop

        %include "fat12hdr.inc"

LABEL_START:
        mov ax,cs
        mov ds,ax
        mov es,ax
        mov ss,ax
        mov sp,BaseOfStack

        ;; 清屏
        mov ax,0600h            ;ah=06h,al=0h
        mov bx,0700h            ;bh=07h,bl=0h(黑底白字)
        mov cx,0                ;左上角（0，0）
        mov dx,02580h           ;右下角(25,80)
        int 10h

        ;; 软区复位（P109）
        ;; ah=0,dl=驱动器号，int 13h
        xor ah,ah
        xor dl,dl
        int 13h
      
        mov word[ReadingSectorNo],19
.Label_Search_In_Root_Directory: 
        cmp word[SectorsForLoop],0
        jz .Label_No_Loader
        dec word[SectorsForLoop]

        ;; es:bx
        mov ax,Loader_Seg
        mov es,ax
        mov bx,Loader_Off
        
        mov ax,[ReadingSectorNo]
        mov cl,1
        call ReadSector

        ;; mov dh,0
        ;; call DispStr
        
        mov si,LoaderFileName   ;ds:si
        mov di,Loader_Off       ;es:di
        cld

        mov dx,16               ;512/32=16,一个扇区有记录16个目录项
.Label_Search_Loader: 
        cmp dx,0
        jz .Label_Goto_Next_Sector
        dec dx
        mov cx,11
.Label_Compare_Filename:
        cmp cx,0
        jz .Label_Loader_Find
        dec cx

        ;; lodsb                   ;mov al,byte[ds:si]，si自增（d=0）/自减（d=1）
        mov al,byte[ds:si]
        inc si
        
        cmp al,byte[es:di]
        jz .Label_Go_On
        jmp .Label_Different
.Label_Go_On:
        inc di
        jmp .Label_Compare_Filename
.Label_Different:
        and di,0FFE0h
        add di,32               ;add di,20h；每个目录项32字节
        mov si,LoaderFileName
        jmp .Label_Search_Loader
.Label_Goto_Next_Sector:
        inc word[ReadingSectorNo]
        jmp .Label_Search_In_Root_Directory
.Label_No_Loader:
        mov dh,1
        call DispStr
        jmp $
.Label_Loader_Find:
        mov dh,0
        call DispStr
        
        and di,0FFE0h    
        
        add di,0x1A
        mov cx,word[es:di]

        mov ax,Loader_Seg
        mov es,ax
        mov bx,Loader_Off

.Label_Goon_Loading_File:
        push cx
        mov al,'.'
        call DispALInChar
      
        mov ax,cx
        call GetLogicSectorNoFromFatSectorNo
        mov cx,ax
        
        mov ax,cx
        mov cl,1
        call ReadSector

        pop cx
        
        push cx
        call GetFATEntry
        pop cx
        
        cmp ax,0FFFh
        jz .Label_File_Loaded
        mov cx,ax
        add bx,[BPB_BytsPerSec]
        jmp .Label_Goon_Loading_File
.Label_File_Loaded:
        mov al,'o'
        call DispALInChar
        
        jmp Loader_Seg:Loader_Off                   ;停在此处

;;;变量
        CurrentRow      db 0     
        SectorsForLoop  dw 14   ;224*32/512=14
        ReadingSectorNo dw 0
        SectorNoOfFAT1  equ BPB_RsvdSecCnt
        
;;;字符串
        LoaderFileName  db "LOADER  BIN" ;fat目录项存储的文件名始终为大写，即便用户创建的用户名是小写也如此
        MessageLength   dw 9
StringStart:
        BootMessage     db "Booting  " ;0
        NoLoader        db "No Loader" ;1
        Loading         db "Loading  " ;2
                 

        
;;; 读取文件（P109）
;;; 参数：
;;;     ax=读取的扇区号(相对与整个硬盘的逻辑扇区号) cl=即将读取的扇区数 es:bx=扇区将读如的内存首地址
;;; 返回：
;;; 
ReadSector:
        push bp
        mov bp,sp
        sub esp,2

        mov byte[bp-2],cl
        push bx
        mov bl,[BPB_SecPerTrk]
        div bl
        inc ah
        mov cl,ah
        mov dh,al
        shr al,1
        mov ch,al
        and dh,1
        pop bx
        mov dl,[BS_DrvNum]
.Go_On_Reading:
        mov ah,2
        mov al,byte[bp-2]
        int 13h
        jc .Go_On_Reading   
      
        add esp,2
        pop bp
        ret

;;; 把文件在FATEntry中的扇区号转化为实际所处软盘的扇区号
;;; 输入：
;;;     ax=FATEntry中的扇区号
;;; 输出：
;;;     ax=实际所处软盘的扇区号
GetLogicSectorNoFromFatSectorNo:
        ;; mov cx,ax              
        ;; ;; Boot占用扇区数(1)
        ;; mov ax,[BPB_RsvdSecCnt]
        ;; add cx,ax

        ;; ;; FATEntry占用扇区数(18)
        ;; mov ax,[BPB_FATSz16]
        ;; mov dl,byte[BPB_NumFATs]
        ;; mul dl
        ;; add cx,ax

        ;; ;; 目录项占用扇区数(224*32/512=14)
        ;; mov ax,[BPB_RootEntCnt]
        ;; mov dl,32
        ;; mul dl
        ;; ;; ax=ax+[BPB_BytsPerSec]-1
        ;; add ax,[BPB_BytsPerSec]
        ;; sub ax,1
        ;; mov dl,[BPB_BytsPerSec]
        ;; div dl

        ;; ;; FATEntry中扇区号（i）对应的数据域中的扇区号为（i-2），因为FATEntry前两项保留，第三项开始表示数据域中的项
        ;; add cx,ax
        ;; sub ax,2
        
        add ax,1+18+14-2
        ret

;;; int GetFATEntry(int ax) 
;;; 获得文件对应的FATEntry号
;;; 参数：
;;;     ax=文件当前FATEntry号
;;; 返回：
;;;     ax=文件下一个FATEntry号
GetFATEntry:
        push bp
        mov bp,sp
        push bx
        push cx
        push dx
        push es
        
        ;; 在Loader_Seg前开辟4K空间存放读取的FAT中的数据
        mov ax,Loader_Seg
        sub ax,0100h
        mov es,ax
        
        mov ax,[bp+4]
        mov bx,3
        mul bx
        mov bx,2
        div bx

        xor dx,dx               ;此句不可省略
        mov bx,[BPB_BytsPerSec]
        div bx
        push dx                 ;ax扇区偏移量，dx字节偏移量
        
        mov bx,0
        add ax,[SectorNoOfFAT1] ;要读取的扇区数
        mov cl,2                ;读取2字节
        call ReadSector         ;将指定扇区数据读入内存es:bx中
        
        pop dx
        add bx,dx
        mov ax,[es:bx]
        
        mov cx,[bp+4]
        test cx,1               ;cx&1
        jz .Label_EvenFATEntry
        
        shr ax,4
        jmp .Label_Exit
.Label_EvenFATEntry:
        and ax,0FFFh
.Label_Exit:
        pop es
        pop dx
        pop cx
        pop bx
        pop bp
        
        ret
        
;;; 显示字符串
;;; 参数：
;;;     dh=字符串序号
;;; 返回：
;;; 
;;; output string
;;; es:bp=string's starting addr
;;; cx=string's length
;;; ah=13h,al=01h
;;; bh=page numbe,bl=color
;;; dh=row,dl=column
;;; int 10h
DispStr:
        push ax
        push bx
        push cx
        push dx
        push bp
        push es
        
        mov ax,word[MessageLength]
        mul dh
        add ax,StringStart
        
        mov bp,ax
        mov ax,ds
        mov es,ax
        mov cx,word[MessageLength]
        mov ax,01301h
        mov bx,0007h
        mov dh,byte[CurrentRow]
        mov dl,0
        int 10h
        inc byte[CurrentRow]

        pop es
        pop bp
        pop dx
        pop cx
        pop bx
        pop ax
        ret
        
;;; 以字符串方式输出al
;;; ah=0Eh, al=输出值, bh=page, bl=color, int 10h
;;; 输入：
;;;     al=待输出的内容
DispALInChar:
        push ax
        push bx
        mov ah,0Eh
        mov bl,0Fh
        int 10h
        pop bx
        pop ax
        ret
;;; 以十六进制方式输出al的值
;;; ah=0Eh, al=输出值, bh=page, bl=color, int 10h
;;; 输入：
;;;     al=待输出的内容
;; DispALInHex:
;;         push ax
;;         push bx
;;         push cx
;;         push dx
        
;;         mov ah,0Eh
;;         mov bl,0Fh
;;         mov dl,al
;;         mov cx,2
;;         shr al,4
;; .2:
;;         and al,0Fh
;;         cmp al,10
;;         add al,'0'
;;         cmp al,'9'
;;         jle .1
;;         add al,'A'-'0'-10       ;65-48-10=7
;; .1:
;;         int 10h
        
;;         mov al,dl
;;         loop .2

        ;; pop dx
        ;; pop cx
        ;; pop bx
        ;; pop ax
        ;; ret

;;; 以16进制的方式输出AX的值
;;; ah=0Eh, al=输出值, bh=page, bl=color, int 10h
;;; 输入：
;;;     ax=待输出的内容
;; DispAXInHex:
;;         push cx
        
;;         mov cx,ax
;;         shr ax,8
;;         call DispALInHex
;;         mov ax,cx
;;         call DispALInHex

;;         pop cx
;;         ret
        
;;; 引导扇区结束标志
        times 510-($-$$) db 0
        dw 0xaa55
        
        
