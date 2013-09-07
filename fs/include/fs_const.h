//分区类型（0表示没有分区）
#define NO_PART 0
#define EXTENDED_PART 5
#define LINUX_PART 83
#define ZHENG_PART 92

/*
31---24：保存驱动程序编号，即设备类型识别编
23---0：保存设备编号，具体设备具体设计
号*/
#define DEVICE_SHIFT 0
#define DRIVER_SHIFT 24
/*驱动程序编号用8位二进制表示*/
#define DRIVER_MASK 0xFF
/*设备编号用24位二进制表示*/
#define DEVICE_MASK 0xFFFFFF

#define MAGIC_V1 0x12345678
#define BOOT_SECTORS_LENGTH 1
#define SUPER_SECTORS_LENGTH 1
#define ROOT_DIR_INODE_INDEX 1
#define MAX_FILENAME_LENGTH 12
#define DEFAULT_FILE_SECTOR_LENGTH 1

/*用一位整数表示文件的类型和文件操作方式*/
/*文件类型,[0-8)位表示文件类型*/
/* #define FILE_TYPE_SHIFT 0 */
#define FILE_TYPE_MASK 0xFF
#define I_DIRECTORY 0x01
#define I_CHAR_SPECIAL 0x02
#define I_FILE 0x04
/*文件打开方式,[8，16)表示文件操作方式*/
/* #define FILE_OP_SHIFT 8 */
#define FILE_OP_MASK 0xFF00
#define O_RDONLY 0x0100
#define O_WRONLY 0x0200
#define O_RDWR 0x0400
#define O_CREATE 0x0800

#define GET_FILE_TYPE(flags) ((flags) & FILE_TYPE_MASK)
#define GET_FILE_OP(flags) ((flags) & FILE_OP_MASK)
#define SET_FILE_FLAGS(file_type,file_op) (((file_type)&FILE_TYPE_MASK) | \
                                           ((file_op)&FILE_OP_MASK))

/*设置文件当前处理位置的方式*/
#define SEEK_SET 0
#define SEEK_CURRENT 1
#define SEEK_END 2

#define FILE_COUNT 32
#define FILE_DESCRIPTOR_COUNT 200
#define INODE_COUNT 100
