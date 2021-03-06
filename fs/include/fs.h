#include "fs_const.h"
/*
分区类型
可通过以下命令查看：
1.>> fdisk 80m.img
2.>> l
DRIVER:驱动程序/驱动程序编号
DEVICE:设备编号，具体设备具体设计
注意：尽量保持相关结构体大小可被SECTOR_SIZE整除，确保读取的数据在一个扇区内。
*/

//超级块结构体
typedef struct s_super_block{
    u32 magic;
    u32 inodes_length;
    u32 sectors_length;
    u32 imap_sectors_length;
    u32 smap_sectors_length;
    u32 data_first_sector_index;
    u32 inodes_sectors_length;
    u32 root_dir_inode_index;
    u32 inode_size;
    u32 inode_isize_off;
    u32 inode_start_off;
    u32 dir_entry_size;
    u32 dir_entry_inode_off;
    u32 dir_entry_fname_off;


    int super_block;
}SUPER_BLOCK;
#define SUPER_BLOCK_SIZE 56


//inode结构体
typedef struct s_inode{
    u32 i_mode;
    u32 i_size;
    u32 i_start_sector_index;
    u32 i_sectors_length;
    u8 res[16];

    int i_device;
    int i_share_count;
    int i_length;
    int i_inode_index;
}INODE;
#define INODE_SIZE 32/*bytes*/

/*
注意：
inode_index=0:表示该目录项为空
*/
typedef struct s_dir_entry{
    u32 inode_index;
    char name[MAX_FILENAME_LENGTH];
}DIR_ENTRY;

#define DIR_ENTRY_SIZE sizeof(struct s_dir_entry)

typedef struct s_file_descriptor{
    int fd_op_mode;//主要是记录操作模式，RDONLY,WRONLY,RDWR,CREATE
    int fd_position;
    struct s_inode *fd_inode;
}FILE_DESCRIPTOR;

#define MAKE_DRIVER_DEVICE(driver,device) ((((driver) & DRIVER_MASK) << DRIVER_SHIFT) | \
                                           (((device) & DEVICE_MASK) << DEVICE_SHIFT))
#define DRIVER(x) ( (x>>DRIVER_SHIFT) & DRIVER_MASK )
#define DEVICE(x) ( x & DEVICE_MASK )


//function
void init_inode_table();
INODE* get_inode_from_table(int inode_index);
INODE* get_free_inode_from_table();
void free_inode(INODE *inode);

void init_file_descriptor_table();
FILE_DESCRIPTOR* get_free_fd_from_table();
void free_fd(FILE_DESCRIPTOR* file_descriptor);

void init_file_descriptor_table();
int get_file_descriptor_table_length();
FILE_DESCRIPTOR* get_file_descriptor_table(int index);
void set_file_descriptor_table(int index,FILE_DESCRIPTOR* file_descriptor);

void task_fs();
int rw_sector(int type,int device,u64 position,int length,int pid,void *buffer);


