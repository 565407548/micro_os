#define FILE_NOT_EXIST 0
#define FILE_NAME_ERROR 1
#define DIR_NOT_EXIST 2
#define NO_FD_INODE 3
#define IMAP_NOT_ENOUGH 4
#define SMAP_NOT_ENOUGH 5
#define DIR_CREATE_ERROR 6
#define INODE_CREATE_ERROR 7
#define PROCESS_FD_POINTER_NOT_ENOUGH 8
#define FILE_NOT_OPEN 9
#define FILE_CANNOT_READ 10
#define FILE_CANNOT_WRITE 11
#define FILE_EXIST 12
#define END_OF_FILE 13
#define FILE_ALLOC_MEM_USEUP 14/*写文件时，写入内容过多，用完了分配给文件的存储空间*/

void set_error_index(int index);

char* get_error_info();
