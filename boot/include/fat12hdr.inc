;; fat12文件系统的说明：
;; 1.该文件系统组成：引导扇区、fat1，fat2，根目录区（长度非固定，需计算），数据区（长度非固定）
;; 2.扇区：磁盘上最小的数据单元；簇：一个或者多个扇区；分区：通常指整个文件系统
;; 3.根目录中每一个条目占用32字节，具体含义如下：
;;   DIR_Name      0     0xB  文件名8字节，扩展名3字节
;;   DIR_Attr      0xB   1    文件属性
;;   保留位         0xC   10   保留位
;;   DIR_WrtTime   0x16  2    最后一次写入时间
;;   DIR_WrtDate   0x18  2    最后一次写入日期
;;   DIR_FstClus   0x1A  2    此条目对应开始簇号
;;   DIR_FileSize  0x1c  4    文件大小
;; 4.fat表中每12位表示一个数据项，代表一个簇，第0个和第1个fat项始终不使用，从第2个开始表示数据区的每个簇
;;   FAT项的值表示的下一个簇号，但如果大于或者等于0xff8，则表示当前文件是最后一个簇。但如果值为0xff7，表示这是一个坏簇
;; 5.DirEntry（x）中表示的文件开始簇号对应与FATEntry（x），对应与数据区的簇号（x-2）

BS_OEMName	DB 'ZhengsOS'	; OEM String, 必须 8 个字节

BPB_BytsPerSec	DW 512		; 每扇区字节数
BPB_SecPerClus	DB 1		; 每簇多少扇区
BPB_RsvdSecCnt	DW 1		; Boot 记录占用多少扇区
BPB_NumFATs	DB 2		; 共有多少 FAT 表
BPB_RootEntCnt	DW 224		; 根目录文件数最大值
BPB_TotSec16	DW 2880		; 逻辑扇区总数
BPB_Media	DB 0xF0		; 媒体描述符
BPB_FATSz16	DW 9		; 每FAT扇区数
BPB_SecPerTrk	DW 18		; 每磁道扇区数
BPB_NumHeads	DW 2		; 磁头数(面数)
BPB_HiddSec	DD 0		; 隐藏扇区数
BPB_TotSec32	DD 0		; 如果 wTotalSectorCount 是 0 由这个值记录扇区数

BS_DrvNum	DB 0		; 中断 13 的驱动器号
BS_Reserved1	DB 0		; 未使用
BS_BootSig	DB 29h		; 扩展引导标记 (29h)
BS_VolID	DD 0		; 卷序列号
BS_VolLab	DB 'ZhengsOSv01'; 卷标, 必须 11 个字节
BS_FileSysType	DB 'FAT12   '	; 文件系统类型, 必须 8个字节  

SectorsOfRootDir   dw 14    ;14=[BPB_RootEntCnt]*32/512

;SectorNoOfBoot dw   0
SectorNoOfFAT  dw   1  ;BPB_RsvdSecCnt
SectorNoOfRootDir  dw  19 ;[BPB_RsvdSecCnt]+ [BPB_NumFATs]*[BPB_FATSz16]
SectorNoOfDatazone dw  33 ;14+19,14=[BPB_RootEntCnt]*32/512


