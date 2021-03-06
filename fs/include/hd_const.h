/*named const parameter*/
/* #define MINOR_BOOT 0 */
/* #define MAJOR_SHIFT 8 */
#define HD_PART_INDEX_SHIFT 0
#define HD_INDEX_SHIFT 16

#define HD_PART_INDEX_MASK 0xFFFF
#define HD_INDEX_MASK 0xFF

/*0:保留(记录整块硬盘信息)；1-4：主分区；5--16：逻辑分区*/
#define HD_PART_PRIM_MIN 1
#define HD_PART_PRIM_MAX 4
#define HD_PART_PRIM_COUNT (HD_PART_PRIM_MAX-HD_PART_PRIM_MIN+1)
#define HD_PART_LOGICAL_MIN 5
#define HD_PART_LOGICAL_MAX 16
#define HD_PART_LOGICAL_COUNT (HD_PART_LOGICAL_MAX-HD_PART_LOGICAL_MIN+1)
//每个分区中有效分区表项最大个数
#define HD_ENT_PER_PRIM_PART 4
#define HD_ENT_PER_LOGICAL_PART 2

//硬盘主分区表所在扇区
#define HD_PRIM_TABLE_SECTOR 0 


#define STATUS_BUSY_MASK 0x80 /*获得status register中busy位的掩码(P334)*/

#define STATUS_DRQ_MASK 0x08 /*data request(ready to transfer data)*/
#define STATUS_DRQ 0x08

#define HD_TIME_OUT 500000 /*单位ms*/

#define PARTITION_TABLE_OFFSET (0x1BE)

/*
分区可否启动状态值
只有两种可能：
80h：可引导
00h：不可引导
其他：不合法
*/
#define BOOT_YES 0x80
#define BOOT_NO 0x00

#define MAX_DRIVERS 2
#define NR_PART_PER_DRIVER 4
#define NR_SUB_PER_PART 16
#define NR_SUB_PER_DRIVER (NR_PART_PER_DRIVER*NR_SUB_PER_PART)
#define NR_PRIM_PER_DRIVER (NR_PART_PER_DRIVER)
#define MAX_PRIM (MAX_DRIVERS*NR_PRIM_PER_DRIVER)
#define MAX_SUBPARTITIONS (NR_SUB_PER_DRIVER*MAX_DRIVERS)

#define MINOR_hd0a 0x0
#define MINOR_hd1a (MINOR_hd1a+NR_SUB_PER_PART)

/*primary*/
#define REG_P_DATA 0x1F0
#define REG_P_ERROR 0x1F1
#define REG_P_FEATURES 0x1F1
#define REG_P_SECTOR_COUNT 0x1F2
#define REG_P_LBA_LOW 0x1F3
#define REG_P_LBA_MID 0x1F4
#define REG_P_LBA_HIGH 0x1F5
#define REG_P_DEVICE 0x1F6
#define REG_P_STATUS 0x1F7
#define REG_P_COMMAND 0x1F7
#define REG_P_ALT_STATUS 0x3F6
#define REG_P_DEVICE_CONTROL 0x3F6

/*secondary*/
#define REG_S_DATA 0x1F0-0x80
#define REG_S_ERROR 0x1F1-0x80
#define REG_S_FEATURES 0x1F1-0x80
#define REG_S_SECTOR_COUNT 0x1F2-0x80
#define REG_S_LBA_LOW 0x1F3-0x80
#define REG_S_LBA_MID 0x1F4-0x80
#define REG_S_LBA_HIGH 0x1F5-0x80
#define REG_S_DEVICE 0x1F6-0x80
#define REG_S_STATUS 0x1F7-0x80
#define REG_S_COMMAND 0x1F7-0x80
#define REG_S_ALT_STATUS 0x3F6-0x80
#define REG_S_DEVICE_CONTROL 0x3F6-0x80

/*命令码*/ 
#define ATA_IDENTIFY 0xEC
#define ATA_INITIAL 0x91
#define ATA_READ 0x20
#define ATA_WRITE 0x30
