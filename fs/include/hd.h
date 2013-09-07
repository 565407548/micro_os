#include "hd_const.h"
/*struct*/
typedef struct s_hd_part_info{
    u32 base;
    u32 size;
}HD_PART_INFO;

typedef struct s_hd_command{
    u8 data;
    u8 features;
    u8 count;
    u8 lba_low;
    u8 lba_mid;
    u8 lba_high;
    u8 device;/*P331*/
    u8 command;
}HD_COMMAND;

//一个分区表项信息
typedef struct s_hd_part_entry{
    u8 flag;/*80h=可引导，00h=不可引导，其他=不合法*/
    u8 begin_header_index;
    u8 begin_sector_index;/*只用了低6位，高2位为起始柱面号的第8，9位*/
    u8 begin_cylinder_index_low8;
    u8 part_type;
    u8 end_header_index;
    u8 end_sector_index;/*只用了低6位，高2位为结束柱面号的第8，9位*/
    u8 end_cylinder_index_low8;
    u32 begin_sector_lba;
    u32 sectors_count;
}HD_PART_ENTRY;

typedef struct s_hd_info{
    int open_count;
    struct s_hd_part_info primary[HD_PART_PRIM_COUNT+1];//记录整个硬盘和硬盘主分区的信息
    struct s_hd_part_info logical[HD_PART_LOGICAL_COUNT];//记录逻辑分区的信息
}HD_INFO;


/*macro*/
/*
先查阅fs.h可知：0-23表示设备编号；24-31表示设备驱动程序编号，即主设备号
硬盘表示方法：
思想：硬盘的设备编号由两部分构成
       0-16：记录硬盘分区序号，1-4表示主分区，其余表示逻辑分区
       16-23记录硬盘编号。如有两块硬盘，其编号分别为0，1

device=(driver<<24) | (sdevice<<16) | (prim<<8) | (logical)
*/
#define MAKE_DEVICE_HD(hd_index,hd_part_index) ( (hd_index & HD_INDEX_MASK)<<HD_INDEX_SHIFT | \
                                              (hd_part_index & HD_PART_INDEX_MASK)<<HD_PART_INDEX_SHIFT)
#define HD_INDEX(x) ( (x>>HD_INDEX_SHIFT) & HD_INDEX_MASK )
#define HD_PART_INDEX(x) ( (x>>HD_PART_INDEX_SHIFT) & HD_PART_INDEX_MASK ) 

#define IS_PRIM(hd_part_index) (HD_PART_PRIM_MIN<=hd_part_index && hd_part_index<=HD_PART_PRIM_MAX)
#define IS_LOGICAL(hd_part_index) (HD_PART_LOGICAL_MIN<=hd_part_index && hd_part_index<=HD_PART_LOGICAL_MAX)

#define MAKE_DEVICE_REG(lba,sdev,lba_high4) (((lba)<<6)|\
                                            ((sdev)<<4)|\
                                            ((lba_high4) & 0xF)|0xA0)

/*../hd.c*/
void task_hd();
/**/
