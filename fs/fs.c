#include "stdlib.h"
#include "message.h"
#include "driver.h"
#include "protect_mode.h"
#include "include/fs.h"
#include "include/hd.h"
#include "process.h"
#include "tty.h"
#include "console.h"
#include "global.h"

#include "syscall.h"
#include "misc.h"

#include "string.h"
#include "error.h"
#include "stdio.h"

static SUPER_BLOCK super_block;

static void init_fs();
static void mkfs();

static int get_boot_block_first_index();
static int get_boot_block_length();
static int get_super_block_first_index();
static int get_super_block_length();
static int get_imap_first_index(SUPER_BLOCK sb);
static int get_imap_length(SUPER_BLOCK sb);
static int get_smap_first_index(SUPER_BLOCK super_block);
static int get_smap_length(SUPER_BLOCK sb);
static int get_inode_first_index(SUPER_BLOCK super_block);
/* static int get_inode_length(SUPER_BLOCK sb); */
static int get_data_block_first_index(SUPER_BLOCK super_block);
/* static int get_data_block_length(SUPER_BLOCK sb); */

static void init_super_block(SUPER_BLOCK *sb,int super_block_first_index,int part_size);
static void init_imap(int imap_first_index,int imap_sectors_length);
static void init_smap(int smap_first_index,int smap_sectors_length);
static void init_inode(int inode_first_index,int root_dir_start_index);
static void init_data_block(int data_block_first_index);

static BOOL do_create(MESSAGE *message);
static BOOL do_unlink(MESSAGE *message);
static int do_ls(MESSAGE *message);
static int do_open(MESSAGE *message);
static int do_reopen(MESSAGE *message);
static void/* int */ do_read(MESSAGE *message);
static void/* int */ do_write(MESSAGE *message);
static void do_seek(MESSAGE *message);
static int do_close(MESSAGE *message);

static int create_file(const char *dir_name,const char *file_name,int file_type);
static int alloc_imap_bit(int dev);
static void free_imap_bit(int bit_index);
static int alloc_smap_bit(int dev,int data_sector_length);
static void free_smap_bit(int bits_index,int bits_length);
static BOOL new_inode(int inode_index,int file_type,int first_sector_index,int sector_length);
static BOOL new_dir_entry(const char *dir_name,int inode_index,const char *filename);
static BOOL remove_dir_entry(const char *dir_name,int inode_index);

static BOOL strip_path(const char *path,char *dir_name,char *file_name);
static int search_file(const char *dir_name,const char *file_name,int file_type);
/* static BOOL get_parent_dir_inode(char *path,INODE *dir_inode,char *filename); */
static int get_inode_by_index(int inode_index,INODE *p_inode);
static int get_inode_by_name(const char *dir_name,const char *file_name,int file_type,INODE *p_inode);
static int get_filename_in_dir(const char* dir_name,char * files);
static int get_dir_inode_by_index(int dir_inode_index,INODE *p_dir_inode);
static int get_dir_inode_by_name(const char *dir_name,INODE *p_dir_inode);
/* static BOOL get_file_inode(INODE dir_inode,char *filename,INODE *inode); */
/* static void release_inode(INODE *pinode); */
static void sync_inode(int inode_index,INODE inode);
static void sync_sectors(int sector_index,int sector_length,u8 *data);
/* static void read_super_block(int dev); */
static void get_super_block(int dev,SUPER_BLOCK *super_block);

/*第一块硬盘第一个逻辑分区（逻辑分区编号为5-16,第一个逻辑分区编号为5）*/
#define ROOT_DEVICE MAKE_DRIVER_DEVICE(DRIVER_HD,MAKE_DEVICE_HD(0,5))

#define WRITE_SECTOR(device,buffer,sector_index) rw_sector(INFO_FS_WRITE,\
                                                           device,      \
                                                           (sector_index)*SECTOR_SIZE, \
                                                           SECTOR_SIZE, \
                                                           TASK_FS,     \
                                                           buffer)
#define READ_SECTOR(device,buffer,sector_index) rw_sector(INFO_FS_READ, \
                                                          device,       \
                                                          (sector_index)*SECTOR_SIZE, \
                                                          SECTOR_SIZE,  \
                                                          TASK_FS,      \
                                                          buffer)



void task_fs(){
#ifdef DEBUG_FS
    printl("in task_fs\n");
#endif
    init_fs();
    MESSAGE message;
    memset(&message,0,sizeof(message));
    while(TRUE){
        send_receive(RECEIVE,ANY,&message);
        
        int source_pid=message.source_pid;
        int fd;
	
        switch(message.type){
        case INFO_FS_CREATE:
            message.res_bool=do_create(&message);
            break;
        case INFO_FS_UNLINK:
            message.res_bool=do_unlink(&message);
            break;
        case INFO_FS_LS:
            message.res_int=do_ls(&message);
            break;
        case INFO_FS_OPEN:
            fd=do_open(&message);
            message.fd=fd;
            break;
        case INFO_FS_READ:
            do_read(&message);
            break;
        case INFO_FS_WRITE:
            do_write(&message);
            break;
        case INFO_FS_SEEK:
            do_seek(&message);
            break;
        case INFO_FS_CLOSE:
            message.res_int=do_close(&message);
            break;
        default:
            printl("\n\n\nunknown message type:%d\n",message.type);
            assert(FALSE,"unknown message type!");
        }
        if(message.type!=INFO_SUSPEND_PROCESS){
            send_receive(SEND,source_pid,&message);
        }else{
            printl("inof_suspend_process\n");
        }
    }
#ifndef _FS_H_
#define _FS_H_
    
#endif /* _FS_H_ */
    while(1)
        ;
    spin("never here");
}

/*
注意：读取的起始位置是扇区的整数倍
*/
int rw_sector(int type,int device,u64 position,int length,int pid,void *buffer){
    MESSAGE message;
    message.type=type;
    message.device=device;
    message.position=position;
    message.length=length;
    message.arg_pointer=buffer;
    message.source_pid=pid;
    assert(dd_map[DRIVER(device)].driver_pid!=PID_INVALID,"");

    send_receive(BOTH,dd_map[DRIVER(device)].driver_pid,&message);
    return 0;
}

static void init_fs(){
    /* u8 fsbuf[SECTOR_SIZE]; */
    MESSAGE message;
    message.type=INFO_FS_DEVICE;
    message.device=ROOT_DEVICE;
    assert(dd_map[DRIVER(ROOT_DEVICE)].driver_pid!=PID_INVALID,"");
    send_receive(BOTH,dd_map[DRIVER(ROOT_DEVICE)].driver_pid,&message);

    //如果系统已经是要求的系统，就不需要在格式化系统了
    get_super_block(ROOT_DEVICE,&super_block);
    /* if(super_block.magic!=MAGIC_V1) */{
        mkfs();
        get_super_block(ROOT_DEVICE,&super_block);
    }
    
    init_inode_table();
    init_file_descriptor_table();
#ifdef DEBUG_FS
    //write test
    /* memset(fsbuf,0x23,SECTOR_SIZE); */
    /* WRITE_SECTOR(ROOT_DEVICE,fsbuf,1); */
    //read test
    u8 fsbuf[SECTOR_SIZE];
    READ_SECTOR(ROOT_DEVICE,fsbuf,1);
    printl("read test:\nfsbuf[0]=%x fsbuf[1]=%x fsbuf[2]=%x fsbuf[3]=%x\n",fsbuf[0],fsbuf[1],fsbuf[2],fsbuf[3]);
#endif
}

static void mkfs(){
    MESSAGE message;
    
    HD_PART_INFO part_info;
    message.type=INFO_FS_IOCTL;
    message.subtype=DIOCTL_GET_PART_INFO;
    message.device=ROOT_DEVICE;//-------------需要指定到具体分区--------------------
    message.arg_pointer=(void*)&part_info;
    message.source_pid=TASK_FS;
    assert(dd_map[DRIVER(message.device)].driver_pid!=PID_INVALID,"driver not exist!");
    send_receive(BOTH,dd_map[DRIVER(message.device)].driver_pid,&message);
    
#ifdef DEBUG_FS
    printl("device=%d base=%d size=%d (in sector)\n",DEVICE(message.device),part_info.base,part_info.size);
#endif

    SUPER_BLOCK sb;
    //super block
    int super_block_first_index=get_super_block_first_index();
    init_super_block(&sb,super_block_first_index,part_info.size);
#ifdef DEBUG_FS
    printl("init_super_block ok(start_sector=%d)\n",super_block_first_index);
#endif
    //imap
    /* int imap_first_index=super_block_first_index+SUPER_SECTORS_LENGTH; */
    int imap_first_index=get_imap_first_index(sb);
    int imap_sectors_length=get_imap_length(sb)/* sb.imap_sectors_length */;
    init_imap(imap_first_index,imap_sectors_length);
#ifdef DEBUG_FS
    printl("init_imap ok(start_sector=%d)\n",imap_first_index);
#endif
    //smap
    /* int smap_first_index=imap_first_index+imap_sectors_length; */
    int smap_first_index=get_smap_first_index(sb);
    int smap_sectors_length=get_smap_length(sb)/* sb.smap_sectors_length */;
    init_smap(smap_first_index,smap_sectors_length);
#ifdef DEBUG_FS
    printl("init_smap ok(start_sector=%d)\n",smap_first_index);
#endif
    //inode
    /* int inode_first_index=smap_first_index+smap_sectors_length; */
    int inode_first_index=get_inode_first_index(sb);
    /* int inode_sectors_length=get_inode_length(sb)/\* sb.inodes_sectors_length *\/; */
    int root_dir_start_index=get_data_block_first_index(sb)/* sb.data_first_sector_index */;
    init_inode(inode_first_index,root_dir_start_index);    
#ifdef DEBUG_FS
    printl("init_inode ok(start_sector=%d)\n",inode_first_index);
#endif
    //data
    /* int data_block_first_index=inode_first_index+inode_sectors_length; */
    int data_block_first_index=get_data_block_first_index(sb);
    init_data_block(data_block_first_index);
#ifdef DEBUG_FS
    printl("init_data_block ok(start_sector=%d)\n",data_block_first_index);
#endif
}

static int get_boot_block_first_index(){
    return 0;
}
static int get_boot_block_length(){
    return BOOT_SECTORS_LENGTH;
}
static int get_super_block_first_index(){
    return get_boot_block_first_index()+get_boot_block_length();
}
static int get_super_block_length(){
    return SUPER_SECTORS_LENGTH;
}
static int get_imap_first_index(SUPER_BLOCK sb){
    return get_super_block_first_index()+get_super_block_length();
}
static int get_imap_length(SUPER_BLOCK sb){
    return sb.imap_sectors_length;
}

static int get_smap_first_index(SUPER_BLOCK sb){
    return get_imap_first_index(sb)+get_imap_length(sb);
}
static int get_smap_length(SUPER_BLOCK sb){
    return sb.smap_sectors_length;
}
static int get_inode_first_index(SUPER_BLOCK sb){
    return get_smap_first_index(sb)+get_smap_length(sb);
}
/* static int get_inode_length(SUPER_BLOCK sb){ */
/*     return sb.inodes_sectors_length; */
/* } */
static int get_data_block_first_index(SUPER_BLOCK sb){
    /* return get_inode_first_index(sb)+get_inode_length(sb); */
    return sb.data_first_sector_index;
}
/* static int get_data_block_length(SUPER_BLOCK sb){ */
/*     return -1; */
/* } */

static void init_super_block(SUPER_BLOCK *sb,int super_block_first_index,int part_size){
    u8 fsbuf[SECTOR_SIZE];
    sb->magic=MAGIC_V1;
    sb->inodes_length=BITS_PER_SECTOR;
    sb->inodes_sectors_length=(sb->inodes_length*INODE_SIZE+SECTOR_SIZE-1)/SECTOR_SIZE;
    sb->sectors_length=part_size;
    sb->imap_sectors_length=sb->inodes_length/(SECTOR_SIZE*BITS_PER_BYTE);
    sb->smap_sectors_length=sb->sectors_length/BITS_PER_SECTOR+1;
    sb->data_first_sector_index=BOOT_SECTORS_LENGTH+SUPER_SECTORS_LENGTH+sb->imap_sectors_length+sb->smap_sectors_length+sb->inodes_sectors_length;
    sb->root_dir_inode_index=ROOT_DIR_INODE_INDEX;
    INODE inode;
    sb->inode_size=INODE_SIZE;
    sb->inode_isize_off=(int)&inode.i_size-(int)&inode;
    sb->inode_start_off=(int)&inode.i_start_sector_index-(int)&inode;
    DIR_ENTRY dir_entry;
    sb->dir_entry_size=DIR_ENTRY_SIZE;
    sb->dir_entry_inode_off=(int)&dir_entry.inode_index-(int)&dir_entry;
    sb->dir_entry_fname_off=(int)&dir_entry.name-(int)&dir_entry;

    memset(fsbuf,0x80,SECTOR_SIZE);
    memcpy(fsbuf,sb,SUPER_BLOCK_SIZE);

#ifdef DEBUG_FS
    printl("super_block_first_index=%d fsbuf[0]=%d\n",super_block_first_index,*(int*)fsbuf);
#endif
    WRITE_SECTOR(ROOT_DEVICE,fsbuf,super_block_first_index);
}

static void init_imap(int imap_first_index,int imap_sectors_length){
    int bit_index,sector_index;
    u8 fsbuf[SECTOR_SIZE];

    memset(fsbuf,0,SECTOR_SIZE);
    sector_index=0;
    //0:reserved;1:/;2:tty0;3:tty1;4:tty2
    for(bit_index=0;bit_index<(NR_CONSOLES+2);bit_index++){
        fsbuf[0]|=1<<bit_index;
    }
    assert(fsbuf[0]==0x1F,"");
#ifdef DEBUG_FS
    printl("imap_first_index=%d fsbuf[0]=%d\n",imap_first_index,*(int*)fsbuf);
#endif
    WRITE_SECTOR(ROOT_DEVICE,fsbuf,imap_first_index+sector_index);

    memset(fsbuf,0,SECTOR_SIZE);
    for(sector_index=1;sector_index<imap_sectors_length;sector_index++){
        WRITE_SECTOR(ROOT_DEVICE,fsbuf,imap_first_index+sector_index);
    }
}

static void init_smap(int smap_first_index,int smap_sectors_length){
    int byte_index,bit_index,sector_index;
    //[0,1):reserved;[1,DEFAULT_FILE_SECTOR_LENGTH):(for root directory);
    int sectors_length=DEFAULT_FILE_SECTOR_LENGTH+1;
    u8 fsbuf[SECTOR_SIZE];

    memset(fsbuf,0,SECTOR_SIZE);
    sector_index=0;
    for(byte_index=0;byte_index<sectors_length/BITS_PER_BYTE;byte_index++){
        fsbuf[byte_index]=0xFF;
    }
    for(bit_index=0;bit_index<sectors_length%BITS_PER_BYTE;bit_index++){
        fsbuf[byte_index]|=(1<<bit_index);
    }
#ifdef DEBUG_FS
    printl("smap_first_index=%d fsbuf[0]=%d\n",smap_first_index,*(int*)fsbuf);
#endif
    WRITE_SECTOR(ROOT_DEVICE,fsbuf,smap_first_index+sector_index);
    
    memset(fsbuf,0,SECTOR_SIZE);
    for(sector_index=1;sector_index<smap_sectors_length;sector_index++){
        WRITE_SECTOR(ROOT_DEVICE,fsbuf,smap_first_index+sector_index);
    }
}


static void init_inode(int inode_first_index,int root_dir_start_index){
    int inode_index;
    u8 fsbuf[SECTOR_SIZE];

    memset(fsbuf,0,SECTOR_SIZE);
    INODE *pinode=NULL;
    
    inode_index=1;// inode_index=1:.;inode_index=0:reserved
    pinode=(INODE*)(fsbuf+inode_index*INODE_SIZE);
    pinode->i_mode=I_DIRECTORY;
    pinode->i_size=DIR_ENTRY_SIZE*4;//.,dev_tty0,dev_tty1,dev_tty2
    pinode->i_start_sector_index=root_dir_start_index;
    //pinode->i_sectors_length=DEFAULT_FILE_SECTOR_LENGTH*(1+NR_CONSOLES);
    pinode->i_sectors_length=DEFAULT_FILE_SECTOR_LENGTH;
    assert(inode_index==ROOT_DIR_INODE_INDEX,"");    

    for(inode_index=2;inode_index<2+NR_CONSOLES;inode_index++){
        pinode=(INODE*)(fsbuf+inode_index*INODE_SIZE);
        pinode->i_mode=I_CHAR_SPECIAL;
        pinode->i_size=0;
        pinode->i_start_sector_index=MAKE_DRIVER_DEVICE(DRIVER_TTY,inode_index-2);
        pinode->i_sectors_length=0;
    }
    WRITE_SECTOR(ROOT_DEVICE,fsbuf,inode_first_index);
}

static void init_data_block(int data_block_first_index){
    int dir_entry_index;
    u8 fsbuf[SECTOR_SIZE];
    memset(fsbuf,0,SECTOR_SIZE);
    
    dir_entry_index=0;
    DIR_ENTRY *dir_entry=((DIR_ENTRY*)fsbuf)+dir_entry_index;
    dir_entry->inode_index=ROOT_DIR_INODE_INDEX;
    strcpy(dir_entry->name,".");
#ifdef DEBUG_FS
    printl("dir_name=%s inode_index=%d\n",dir_entry->name,*(int *)fsbuf);
#endif

    for(dir_entry_index=1;dir_entry_index<=NR_CONSOLES;dir_entry_index++){
        dir_entry=((DIR_ENTRY*)fsbuf)+dir_entry_index; 
        dir_entry->inode_index=ROOT_DIR_INODE_INDEX+dir_entry_index;
        sprintf(dir_entry->name,"dev_tty%d",dir_entry_index-1);
#ifdef DEBUG_FS
        printl("dir_name=%s\n",dir_entry->name);
#endif
    }

#ifdef DEBUG_FS
    printl("fsbuf[0]=%d fsbuf[1]=%d fsbuf[2]=%d fsbuf[3]=%d\n",*(int*)(fsbuf),*(int*)(fsbuf+16),*(int*)(fsbuf+32),*(int*)(fsbuf+48));
#endif

    WRITE_SECTOR(ROOT_DEVICE,fsbuf,data_block_first_index);
}

static BOOL do_create(MESSAGE *message){
    const char *path;
    char dirname[MAX_FILENAME_LENGTH]={0};
    char filename[MAX_FILENAME_LENGTH]={0};
    int flags,inode_index;

    path=message->arg_pointer;
    flags=message->flags;

    //分离父目录路径和文件名
    if(!strip_path(path,dirname,filename)){
        return FALSE;
    }

    inode_index=search_file(dirname,filename,GET_FILE_TYPE(flags));
    if(inode_index<0){
        inode_index=create_file(dirname,filename,GET_FILE_TYPE(flags));
        if(inode_index<0){
            return FALSE;
        }
    }
    else{
        set_error_index(FILE_EXIST);
        return FALSE;
    }
    return TRUE;
}

static BOOL do_unlink(MESSAGE *message){
    int inode_index,flags,sector_index,sector_length;
    const char *path;
    char dir_name[MAX_FILENAME_LENGTH]={0};
    char file_name[MAX_FILENAME_LENGTH]={0};
    INODE inode;

    flags=message->flags;
    path=message->arg_pointer;
    //分离父目录路径和文件名
    if(!strip_path(path,dir_name,file_name)){
        return FALSE;
    }
    
    //1.get inode_index,and remove the dir_entry
    inode_index=search_file(dir_name,file_name,GET_FILE_TYPE(flags));
#ifdef DEBUG_FS
    printl("inode_index=%d(in do_unlink)\n",inode_index);
#endif
    if(inode_index<0){
        set_error_index(FILE_NOT_EXIST);
        return FALSE;
    }
    if(!remove_dir_entry(dir_name,inode_index)){
        set_error_index(FILE_NOT_EXIST);
        return FALSE;
    }
    //2.get file inode by inode_index
    get_inode_by_index(inode_index,&inode);
    
    //3.clear imap by inode_index
    free_imap_bit(inode_index);
    
    //4.clear smap inode
    sector_index=inode.i_start_sector_index-get_data_block_first_index(super_block);
    sector_length=inode.i_sectors_length;
    /* printl("sector_index=%d sector_length=%d data_first_index=%d\n",inode.i_start_sector_index,sector_length,get_data_block_first_index(super_block)); */
    free_smap_bit(sector_index,sector_length);
    
    return TRUE;
}

static int do_ls(MESSAGE *message){
    const char *dir_name=message->arg_pointer;
    char* files=(char*)message->res_pointer;

#ifdef DEBUG_FS
    printl("dir_name=%s (in do_ls)\n",dir_name);
#endif 

    return get_filename_in_dir(dir_name,files);
}

static int do_open(MESSAGE *message){
    int i,fd=-1;
    const char *path;
    char dirname[MAX_FILENAME_LENGTH]={0};
    char filename[MAX_FILENAME_LENGTH]={0};
    int flags;
    int inode_index;
    struct s_file_descriptor *pfd=NULL;
    struct s_inode *pinode=NULL;
    /* struct s_inode parent_dir_inode; */
    PROCESS *process;

    path=message->arg_pointer;
    flags=message->flags;
    process=pid2process(message->source_pid);
    
    pfd=get_free_fd_from_table();
    pinode=get_free_inode_from_table();
    if(pfd==NULL || pinode==NULL){
        set_error_index(NO_FD_INODE);
        return -1;
    }
    
    //分离父目录路径和文件名
    if(!strip_path(path,dirname,filename)){
        return -1;
    }
#ifdef DEBUG_FS
    printl("dir_name=%s file_name=%s\n",dirname,filename);
#endif

    //搜索文件
    inode_index=search_file(dirname,filename,GET_FILE_TYPE(flags));
    if(inode_index<0){
        if((flags & O_CREATE)==0){
            return -1;
        }
        inode_index=create_file(dirname,filename,GET_FILE_TYPE(flags));
        if(inode_index<0){
            return -1;
        }
    }
    assert(inode_index>=0,"");
    get_inode_by_index(inode_index,pinode);
#ifdef DEBUG_FS
    printl("pinode.start_index=%d(in do_open)",pinode->i_start_sector_index);
#endif
    /*SDTIN=0,STDOUT=1,STDERROR=2*/
    for(i=3;i<FILE_COUNT;i++){
        if(process->file_descriptor[i]==NULL){
            fd=i;
            break;
        }
    }
    
    if(fd>=0){
        process->file_descriptor[fd]=pfd;
    
        pfd->fd_inode=pinode;
        pfd->fd_op_mode=GET_FILE_OP(flags);
        pfd->fd_position=0;

        pinode->i_inode_index=inode_index;
#ifdef DEBUG_FS
        printl("inode_index=%d(in do_open)\n",pinode->i_inode_index);
        printl("data_sector_index=%d data_sector_length=%d\n",process->file_descriptor[fd]->fd_inode->i_start_sector_index,process->file_descriptor[fd]->fd_inode->i_sectors_length);
#endif
        return fd;
    }else{
        set_error_index(PROCESS_FD_POINTER_NOT_ENOUGH);
        return -1;
    }
}

static int do_reopen(MESSAGE *message){
    return -1;
}

static /* int */void do_write(MESSAGE *message){
    u8 fsbuf[SECTOR_SIZE*DEFAULT_FILE_SECTOR_LENGTH];
    const char *buf;
    int start_position,sector_length,length,data_sector_index;
    PROCESS *process;
    struct s_file_descriptor* pfd;

    process=pid2process(message->source_pid);
    if(message->fd==STDOUT && process->file_descriptor[message->fd]==NULL){
#ifdef DEBUG_FS
        printl("pid=%d (in do_write)\n",message->source_pid);
#endif
/*pfd->fd_inode->i_mode & I_CHAR_SPECIAL !=0*/
        /* int device=pfd->fd_inode->i_start_sector_index; */
        message->process_index=message->source_pid;
        send_receive(SEND,TASK_TTY/* dd_map[DRIVER(device)] */,message);
        /* message->type=INFO_SUSPEND_PROCESS; */
    }else{
        
        pfd=process->file_descriptor[message->fd];
        if((pfd->fd_op_mode & O_WRONLY)==0 && (pfd->fd_op_mode & O_RDWR)==0){
            set_error_index(FILE_CANNOT_WRITE);
            message->length=-1;
            return;
        }
        buf=message->arg_pointer;
        
        start_position=pfd->fd_position;
        if(start_position >= pfd->fd_inode->i_sectors_length*SECTOR_SIZE){
            set_error_index(FILE_ALLOC_MEM_USEUP);
            message->length=-1;
            return;
        }
        length=min(start_position+message->length,pfd->fd_inode->i_sectors_length*SECTOR_SIZE);
        sector_length=(length+SECTOR_SIZE-1)/SECTOR_SIZE;
        data_sector_index=pfd->fd_inode->i_start_sector_index;
        
#ifdef DEBUG_FS
        /* printl("1 %d %d length=%d\n",start_position,message->length,length); */
        printl("sector_index=%d sector_length=%d(in do_write)\n",data_sector_index,sector_length);
#endif
        rw_sector(INFO_FS_READ,
                  ROOT_DEVICE,
                  (data_sector_index)*SECTOR_SIZE,
                  sector_length*SECTOR_SIZE,
                  TASK_FS,
                  fsbuf);
        memcpy(fsbuf+start_position,buf,length-start_position);
        /* printl("2 %d %d\n",data_sector_index,length); */
        rw_sector(INFO_FS_WRITE,
                  ROOT_DEVICE,
                  (data_sector_index)*SECTOR_SIZE,
                  sector_length*SECTOR_SIZE,
                  TASK_FS,
                  fsbuf);
        
        /* printl("3 inode.i_size=%d\n",pfd->fd_inode->i_size); */
        pfd->fd_inode->i_size=length;
        /* printl("inode_index=%d\n",pfd->fd_inode->i_inode_index); */
        sync_inode(pfd->fd_inode->i_inode_index,*(pfd->fd_inode));
        
        pfd->fd_position=length;  
        message->length=length-start_position;
    }
}

static void/* int */ do_read(MESSAGE *message){
    char fsbuf[SECTOR_SIZE*DEFAULT_FILE_SECTOR_LENGTH];
    char *buf;
    int start_position,sector_length,length,data_sector_index;
    PROCESS *process;
    struct s_file_descriptor* pfd;

    process=pid2process(message->source_pid);
    pfd=process->file_descriptor[message->fd];
    if((pfd->fd_op_mode & O_RDONLY)==0 && (pfd->fd_op_mode & O_RDWR)==0){
        set_error_index(FILE_CANNOT_READ);
        message->length=-1;
        return;
    }
    buf=message->res_pointer;
    start_position=pfd->fd_position;
    if(start_position > pfd->fd_inode->i_size){
        set_error_index(END_OF_FILE);
        message->length=-1;
        return;
    }
    assert(start_position<=pfd->fd_inode->i_size,"file read over");
    length=min(start_position+message->length,pfd->fd_inode->i_size);
    sector_length=(length+SECTOR_SIZE-1)/SECTOR_SIZE;
    data_sector_index=pfd->fd_inode->i_start_sector_index;
    
    /* printl("data_sector_index=%d sector_length=%d\n",data_sector_index,sector_length); */

    rw_sector(INFO_FS_READ,
              ROOT_DEVICE,
              (data_sector_index)*SECTOR_SIZE,
              sector_length*SECTOR_SIZE,
              TASK_FS,
              fsbuf);
    strncpy(buf,fsbuf+start_position,length-start_position);

    pfd->fd_position=length;
    
    message->length=length-start_position;  
}

static void do_seek(MESSAGE *message){
    int offset,whence;
    PROCESS *process;
    struct s_file_descriptor* pfd;
    
    offset=message->offset;
    whence=message->whence;
    process=pid2process(message->source_pid);
    pfd=process->file_descriptor[message->fd];
    switch(whence){
    case SEEK_SET:
        pfd->fd_position=min(offset,pfd->fd_inode->i_size);
        break;
    case SEEK_CURRENT:
        pfd->fd_position=min(pfd->fd_position+offset,pfd->fd_inode->i_size);
        break;
    case SEEK_END:
        pfd->fd_position=pfd->fd_inode->i_size;
        break;
    }
}

static int do_close(MESSAGE *message){
    PROCESS *process=pid2process(message->source_pid);
    int fd=message->fd;
    
    if(process->file_descriptor[fd]!=NULL &&
       process->file_descriptor[fd]->fd_inode!=NULL &&
       process->file_descriptor[fd]->fd_inode->i_share_count>0
        ){
        free_inode(process->file_descriptor[fd]->fd_inode);
        free_fd(process->file_descriptor[fd]);
        process->file_descriptor[fd]=NULL;
        
        return 0;
    }
    else{
        set_error_index(FILE_NOT_OPEN);
        return -1;
    }
}

/*
新建文件：
1.分配inode，设置imap
2.分配data_sector,设置smap
3.新建目录项，更新目录项内容（inode_index,filename）
4.新建inode，更新inode的内容
*/
static int create_file(const char *dir_name,const char *file_name,int file_type){
    int inode_index=alloc_imap_bit(0);
    if(inode_index<0){
        set_error_index(IMAP_NOT_ENOUGH);
        return -1;
    }
#ifdef DEBUG_FS
    printl("inode_index=%d(in create_file)\n",inode_index);
#endif

    int data_index=alloc_smap_bit(0,DEFAULT_FILE_SECTOR_LENGTH);//相对于数据区的sector_index
    if(data_index<0){
        set_error_index(SMAP_NOT_ENOUGH);
        return -1;
    }
#ifdef DEBUG_FS
    printl("data_index=%d(in create_file)\n",data_index);
#endif
    data_index+=get_data_block_first_index(super_block);//绝对sector_index
#ifdef DEBUG_FS
    printl("data_index=%d(in create_file)\n",data_index);
#endif


    if(!new_dir_entry(dir_name,inode_index,file_name)){
        set_error_index(DIR_CREATE_ERROR);
        return -1;
    }

    if(!new_inode(inode_index,file_type,data_index,DEFAULT_FILE_SECTOR_LENGTH)){
        set_error_index(INODE_CREATE_ERROR);
        return -1;
    }
    return inode_index;
}

static int alloc_imap_bit(int dev){
    int imap_first_index=get_imap_first_index(super_block);
    int imap_length=get_imap_length(super_block);
    u8 fsbuf[SECTOR_SIZE];
    int off, byte_index,bit_index;

    for(off=0;off<imap_length;off++){
        memset(fsbuf,0,SECTOR_SIZE);
        
        int sector_index=(imap_first_index+off);
        rw_sector(INFO_FS_READ,
                  ROOT_DEVICE,
                  sector_index*SECTOR_SIZE,
                  SECTOR_SIZE,
                  TASK_FS,
                  fsbuf);
        for(byte_index=0;byte_index<SECTOR_SIZE;byte_index++){
            if(fsbuf[byte_index]!=0xFF)
                break;
        }
        for(bit_index=0;bit_index<BITS_PER_BYTE;bit_index++){
            if((fsbuf[byte_index] & (1<<bit_index))==0){
                fsbuf[byte_index]=fsbuf[byte_index] | (1<<bit_index);
                sync_sectors(sector_index,1,fsbuf);
                return (off*SECTOR_SIZE+byte_index)*BITS_PER_BYTE+bit_index;
            }
        }
    }
    return -1;
}

static void free_imap_bit(int bits_index){

    u8 fsbuf[SECTOR_SIZE];
    int sector_index,sector_length,bits_off;

    sector_index=get_imap_first_index(super_block)+bits_index/BITS_PER_SECTOR;
    sector_length=1;
    bits_off=bits_index%BITS_PER_SECTOR;

    //读取smap数据
    rw_sector(INFO_FS_READ,
              ROOT_DEVICE,
              sector_index*SECTOR_SIZE,
              sector_length*SECTOR_SIZE,
              TASK_FS,
              fsbuf);

    /* printl("sector_index=%d sector_length=%d fsbuf=%c (in free_imap_bit)\n",sector_index,sector_length,*(char*)fsbuf[bits_off/BITS_PER_BYTE]); */
    fsbuf[bits_off/BITS_PER_BYTE] &=~(1<<bits_index%BITS_PER_BYTE); 
/* #ifdef DEBUG_FS */
    /* printl("bits_off/BITS_PER_BYTE=%d bits_index%BITS_PER_BYTE=%d fsbuf=%c(in free_imap_bit)\n",bits_off/BITS_PER_BYTE,bits_index%BITS_PER_BYTE,*(char*)fsbuf[bits_off/BITS_PER_BYTE]); */
/* #endif */
    rw_sector(INFO_FS_WRITE,
              ROOT_DEVICE,
              sector_index*SECTOR_SIZE,
              sector_length*SECTOR_SIZE,
              TASK_FS,
              fsbuf);
}

//目前仅支持文件长度为1扇区固定大小，即data_sector_length=1
static int alloc_smap_bit(int dev,int data_sector_length){
    assert(data_sector_length==1,"only for data_sector_length=1 currently!");

    int smap_first_index=get_smap_first_index(super_block);
    int smap_length=get_smap_length(super_block);
    u8 fsbuf[SECTOR_SIZE];
    int off, byte_index,bit_index;

    for(off=0;off<smap_length;off++){
        memset(fsbuf,0,SECTOR_SIZE);
        
        int sector_index=(smap_first_index+off);
        rw_sector(INFO_FS_READ,
                  ROOT_DEVICE,
                  sector_index*SECTOR_SIZE,
                  SECTOR_SIZE,
                  TASK_FS,
                  fsbuf);
        for(byte_index=0;byte_index<SECTOR_SIZE;byte_index++){
            if(fsbuf[byte_index]!=0xFF)
                break;
        }
        for(bit_index=0;bit_index<BITS_PER_BYTE;bit_index++){
            if((fsbuf[byte_index] & (1<<bit_index))==0){
                fsbuf[byte_index]=fsbuf[byte_index] | (1<<bit_index);
                sync_sectors(sector_index,1,fsbuf);
                return (off*SECTOR_SIZE+byte_index)*BITS_PER_BYTE+bit_index;
            }
        }
    }
    return -1;
}

static void free_smap_bit(int bits_index,int bits_length){
    assert(bits_length==1,"only for sector_length=1 currently!");

#ifdef DEBUG_FS
    printl("bits_index=%d bits_length=%d(int free_smap_bit)\n",bits_index,bits_length);
#endif

    u8 fsbuf[SECTOR_SIZE];
    int sector_index,sector_length,bits_off, bytes_index,bytes_start,bytes_end;

    sector_index=get_smap_first_index(super_block)+bits_index/BITS_PER_SECTOR;
    sector_length=(bits_index+bits_length+BITS_PER_SECTOR-1)/BITS_PER_SECTOR;
    bits_off=bits_index%BITS_PER_SECTOR;
    bytes_start=(bits_off+BITS_PER_BYTE-1)/BITS_PER_BYTE;
    bytes_end=(bits_off+bits_length)/BITS_PER_BYTE;
#ifdef DEBUG_FS
    printl("bytes_start=%d bytes_end=%d (in fre_smap_bit)\n",bytes_start,bytes_end);
#endif    

    //读取smap数据
    rw_sector(INFO_FS_READ,
              ROOT_DEVICE,
              sector_index*SECTOR_SIZE,
              sector_length*SECTOR_SIZE,
              TASK_FS,
              fsbuf);
    /*分为三步处理：
      bits:           -------------------------------------------------
                         |  |  |                          |   |   |
      bits_index         8m ^   8(m+1)                   8n   ^   8(n+1)
      step：                | 1|            2             | 3 |
     */
    for(bits_index=bits_off;bits_index%BITS_PER_BYTE!=0 && bits_index<bits_off+bits_length;bits_index++){
#ifdef DEBUG_FS
        printl("bits_index=%d(step 1. in fre_smap_bit)",bits_index);
#endif
        fsbuf[bits_index/BITS_PER_BYTE] &=~(1<<bits_index%BITS_PER_BYTE); 
    }
#ifdef DEBUG_FS
    printl("\n");
#endif
    for(bytes_index=bytes_start;bytes_index<bytes_end;bytes_index++){
#ifdef DEBUG_FS
        printl("bytes_index=%d(step 2. in fre_smap_bit)",bytes_index);
#endif
        fsbuf[bytes_index]=0;
    }
#ifdef DEBUG_FS
    printl("\n");
#endif
    if(bytes_start<bytes_end){/*需要step3操作的情况*/
        for(bits_index=bytes_end*BITS_PER_BYTE;bits_index<bits_off+bits_length;bits_index++){
#ifdef DEBUG_FS
            printl("bits_index=%d(step 3. in fre_smap_bit)",bits_index);
#endif
            fsbuf[bits_index/BITS_PER_BYTE] &=~(1<<bits_index%BITS_PER_BYTE); 
        }
    }
/* #ifdef DEBUG_FS */
    printl("\n");
/* #endif */

    //更新smap数据
    rw_sector(INFO_FS_WRITE,
              ROOT_DEVICE,
              sector_index*SECTOR_SIZE,
              sector_length*SECTOR_SIZE,
              TASK_FS,
              fsbuf);
}

/*
first_sector_index:该文件的起始扇区编号（相对于data_block起始位置的偏移）
sector_length:分配给该文件的扇区数
*/
static BOOL new_inode(int inode_index,int file_type,int first_sector_index,int sector_length){
    int new_inode_sector_index=get_inode_first_index(super_block)+(inode_index*INODE_SIZE)/SECTOR_SIZE;
    int off=(inode_index*INODE_SIZE)%SECTOR_SIZE;
    u8 fsbuf[SECTOR_SIZE];

    /* printl("data_index=%d(in new_inode)\n",new_inode_sector_index); */

    rw_sector(INFO_FS_READ,
              ROOT_DEVICE,
              (new_inode_sector_index)*SECTOR_SIZE,
              SECTOR_SIZE,
              TASK_FS,
              fsbuf);

    INODE *pinode=(INODE*)(fsbuf+off);
    pinode->i_mode=file_type;
    pinode->i_size=0;
    pinode->i_start_sector_index=first_sector_index;
    pinode->i_sectors_length=sector_length;
    /* printl("first_sector_index=%d sector_length=%d (in new_inode)\n",first_sector_index,sector_length); */

    sync_sectors(new_inode_sector_index,1,fsbuf); 
    
    return TRUE;
}

static BOOL new_dir_entry(const char *dir_name,int inode_index,const char *filename){

    INODE dir_inode;
    int dir_inode_index;
    dir_inode_index=get_dir_inode_by_name(dir_name,&dir_inode);

    assert(dir_inode.i_mode==I_DIRECTORY,"must be directory");

    DIR_ENTRY *dir_entry;
    int length=dir_inode.i_size;
    int sector_length=(length+SECTOR_SIZE-1)/SECTOR_SIZE;
    int dir_entry_count=(dir_inode.i_size)/DIR_ENTRY_SIZE;
    int dir_entry_index;
    u8 fsbuf[SECTOR_SIZE*2];
    BOOL find=FALSE;
    
    rw_sector(INFO_FS_READ,
              ROOT_DEVICE,
              (dir_inode.i_start_sector_index)*SECTOR_SIZE,
              sector_length*SECTOR_SIZE,
              TASK_FS,
              fsbuf);
    
    for(dir_entry_index=0;dir_entry_index<dir_entry_count;dir_entry_index++){
        dir_entry=(DIR_ENTRY *)(fsbuf+dir_entry_index*DIR_ENTRY_SIZE);
/* #ifdef DEBUG_FS */
/*         printl("dir_entry->inode_index=%d ",dir_entry->inode_index); */
/* #endif */
        if(dir_entry->inode_index==0){/*目录项inode_index=0表示该目录项为空*/
            find=TRUE;
            break;
        }
    }
    
    if((!find) && 
       ((dir_inode.i_size+DIR_ENTRY_SIZE) <= (dir_inode.i_sectors_length*SECTOR_SIZE))){
        dir_entry=(DIR_ENTRY *)(fsbuf+dir_entry_index*DIR_ENTRY_SIZE);
        dir_inode.i_size=dir_inode.i_size+DIR_ENTRY_SIZE;
        sync_inode(dir_inode_index,dir_inode);
        find=TRUE;
    }
    if(find){
        dir_entry->inode_index=inode_index;
        strcpy(dir_entry->name,filename);
        sync_sectors(dir_inode.i_start_sector_index,sector_length,fsbuf);//更新硬盘数据
        
        return TRUE;
    }
    return FALSE;
}

static BOOL remove_dir_entry(const char *dir_name,int inode_index){
    INODE dir_inode;

    get_dir_inode_by_name(dir_name,&dir_inode);
    assert(dir_inode.i_mode==I_DIRECTORY,"must be directory");

    DIR_ENTRY *dir_entry;
    int length=dir_inode.i_size;
    int sector_length=(length+SECTOR_SIZE-1)/SECTOR_SIZE;
    int dir_entry_count=(dir_inode.i_size)/DIR_ENTRY_SIZE;
    int dir_entry_index;
    u8 fsbuf[SECTOR_SIZE*2];
    
    rw_sector(INFO_FS_READ,
              ROOT_DEVICE,
              (dir_inode.i_start_sector_index)*SECTOR_SIZE,
              sector_length*SECTOR_SIZE,
              TASK_FS,
              fsbuf);
    
    for(dir_entry_index=0;dir_entry_index<dir_entry_count;dir_entry_index++){
        dir_entry=(DIR_ENTRY *)(fsbuf+dir_entry_index*DIR_ENTRY_SIZE);
/* #ifdef DEBUG_FS */
/*         printl("dir_entry->inode_index=%d ",dir_entry->inode_index); */
/* #endif */
        if(dir_entry->inode_index==inode_index){/*目录项inode_index=0表示该目录项为空*/
            dir_entry->inode_index=0;
            rw_sector(INFO_FS_WRITE,
                      ROOT_DEVICE,
                      (dir_inode.i_start_sector_index)*SECTOR_SIZE,
                      sector_length*SECTOR_SIZE,
                      TASK_FS,
                      fsbuf);
            return TRUE;
        }
    }
    return FALSE;
}

static BOOL strip_path(const char *path,char *dir_name,char *file_name){
    if(*path!='/'){
        set_error_index(FILE_NAME_ERROR);
        return FALSE;
    }else{
        //获得父目录地址
        *dir_name++=*path++;
        *dir_name=0;
        
        //获得文件名
        while(*path!=0){
            if(*path=='/'){//目前只处理但目录结构
                set_error_index(FILE_NAME_ERROR);
                return FALSE;
            }
            *file_name++=*path++;
        }
        *file_name=0;
        return TRUE;
    }
}

static int search_file(const char *dir_name,const char *file_name,int file_type){
    /* struct s_inode parent_dir_inode; */
    /* if(!get_dir_inode(dir_name,&parent_dir_inode)){ */
    /*     return FALSE; */
    /* } */
    /* if(!get_file_inode(&parent_dir_inode,filename,p_inode)){ */
    /*     return FALSE; */
    /* } */
    INODE inode;
    return get_inode_by_name(dir_name,file_name,file_type,&inode);
}

/* static BOOL get_parent_dir_inode(const char *dir_name,INODE *p_dir_inode){ */
/*     assert(strcmp(dir_name,"/")==0,"only one directory exist(root dir:/)"); */

/*     //获得父目录 */
/*     if(!get_dir_inode(super_block.root_dir_inode_index,dir_inode)){ */
/*         set_error_index(DIR_NOT_EXIST); */
/*         return FALSE; */
/*     } */
/* #ifdef DEBUG_FS */
/*     printl("dirinode.size=%d\n",dir_inode->i_size); */
/* #endif */
/*     return TRUE; */
/* } */

/*
根据inode_index获得对应的inode
*/
static int get_inode_by_index(int inode_index,INODE *p_inode){
    int inode_first_index=get_inode_first_index(super_block);
    int off=inode_index*INODE_SIZE;
    u8 fsbuf[SECTOR_SIZE];

    inode_first_index+=off/SECTOR_SIZE;
    off=off%SECTOR_SIZE;
    
    rw_sector(INFO_FS_READ,
              ROOT_DEVICE,
              (inode_first_index)*SECTOR_SIZE,
              SECTOR_SIZE,
              TASK_FS,
              fsbuf);
    memcpy(p_inode,fsbuf+off,INODE_SIZE);
    
    return inode_index;
}

//有问题：调用的rw_sector中读取数据长度必须为SECTOR_SIZE的整数倍
//该函数还发现一个问题：如果fsbuf为全局静态变量，由于开始读目录内容填充了fsbuf，然后在比较的时候又读取了硬盘，再次填充了fsbuf，导致之前的数据被覆盖。所以fsbuf还是设定成局部变量。
//切忌：尽量减少全局变量的使用。
static int get_inode_by_name(const char *dir_name,const char *file_name,int file_type,INODE *p_inode){
    INODE dir_inode;
    DIR_ENTRY *dir_entry;
    int dir_entry_index;
    int dir_entry_count;
    u8 fsbuf[SECTOR_SIZE*2];
    
    if(get_dir_inode_by_name(dir_name,&dir_inode)<0){
        set_error_index(DIR_NOT_EXIST);
        return -1;
    }
    dir_entry_count=dir_inode.i_size/DIR_ENTRY_SIZE; 

    rw_sector(INFO_FS_READ,
              ROOT_DEVICE,
              (dir_inode.i_start_sector_index)*SECTOR_SIZE,
              dir_inode.i_size,
              TASK_FS,
              fsbuf);

    for(dir_entry_index=0;dir_entry_index<dir_entry_count;dir_entry_index++){
        dir_entry=(DIR_ENTRY *)(fsbuf+dir_entry_index*DIR_ENTRY_SIZE);

/* #ifdef DEBUG_FS */
/*         printl("dir_index=%d dir_name=%s filename=%s cmp=%d\n", */
/*                dir_entry->inode_index, */
/*                dir_entry->name, */
/*                file_name, */
/*                strcmp(dir_entry->name,file_name)==0); */
/* #endif */
        //此处的dir_entry->inode_index=0:表示该文件建立后删除了
        if(dir_entry->inode_index>0 && strcmp(dir_entry->name,file_name)==0){
            if(get_inode_by_index(dir_entry->inode_index,p_inode)){
                if((p_inode->i_mode & file_type) !=0){
                    /* printl("get inode by name data_sector_index=%d\n",p_inode->i_start_sector_index); */
                    return dir_entry->inode_index;
                }
            }
        }
    }
    set_error_index(FILE_NOT_EXIST);
    return -1;
}

/*
返回目录下文件/目录的个数
*/
static int get_filename_in_dir(const char* dir_name,char *files){
    INODE dir_inode;
    DIR_ENTRY *dir_entry;
    int dir_entry_index;
    int dir_entry_count;
    int files_index,files_off;
    u8 fsbuf[SECTOR_SIZE*2];
    
    if(get_dir_inode_by_name(dir_name,&dir_inode)<0){
        set_error_index(DIR_NOT_EXIST);
        return -1;
    }
    dir_entry_count=dir_inode.i_size/DIR_ENTRY_SIZE; 

    rw_sector(INFO_FS_READ,
              ROOT_DEVICE,
              (dir_inode.i_start_sector_index)*SECTOR_SIZE,
              dir_inode.i_size,
              TASK_FS,
              fsbuf);

    files_off=0;
    files_index=0;
    for(dir_entry_index=0;dir_entry_index<dir_entry_count;dir_entry_index++){
        dir_entry=(DIR_ENTRY *)(fsbuf+dir_entry_index*DIR_ENTRY_SIZE);
        //此处的dir_entry->inode_index=0:表示该文件建立后删除了
        if(dir_entry->inode_index>0){
            strcpy(files+files_off,dir_entry->name);
#ifdef DEBUG_FS
            printl("%d:%s\n",files_index,files+files_off);
#endif
            files_off+=strlen(dir_entry->name)+1;
            files_index++;
        }
    }
    return files_index;
}
/*

*/
static int get_dir_inode_by_index(int dir_inode_index,INODE *p_dir_inode){
    assert(dir_inode_index==super_block.root_dir_inode_index,"only one directory(root dir:/)");

    if(get_inode_by_index(dir_inode_index,p_dir_inode)>=0){
        if(p_dir_inode->i_mode==I_DIRECTORY)
            return dir_inode_index;
    }
    return -1;
}

static int get_dir_inode_by_name(const char *dir_name,INODE *p_dir_inode){
    assert(strcmp(dir_name,"/")==0,"only one directory(root dir:/)");

    return get_dir_inode_by_index(super_block.root_dir_inode_index,p_dir_inode);
 
}


/* static BOOL get_file_inode(const char *dirname,const char *filename,INODE *p_inode){ */
/*     INODE dir_inode; */

/*     get_dir_inode_by_name(dirname,&dir_inode); */

/*     DIR_ENTRY *dir_entry; */
/*     int dir_entry_count=p_dir_inode->i_size/DIR_ENTRY_SIZE; */
/*     int dir_entry_index; */
/*     u8 fsbuf[SECTOR_SIZE*2]; */
    
/*     /\* READ_SECTOR(ROOT_DEVICE,fsbuf,dir_inode.i_start_sector_index); *\/ */
/*     //只会读取整个扇区 */
/*     rw_sector(INFO_FS_READ, */
/*               ROOT_DEVICE, */
/*               (dir_inode.i_start_sector_index)*SECTOR_SIZE, */
/*               dir_inode.i_size, */
/*               TASK_FS, */
/*               fsbuf); */

/*     for(dir_entry_index=0;dir_entry_index<dir_entry_count;dir_entry_index++){ */
/*         dir_entry=(DIR_ENTRY *)(fsbuf+dir_entry_index*DIR_ENTRY_SIZE); */

/* #ifdef DEBUG_FS */
/*         printl("dir_index=%d dir_name=%s filename=%s cmp=%d\n", */
/*                dir_entry->inode_index, */
/*                dir_entry->name, */
/*                filename, */
/*                strcmp(dir_entry->name,filename)==0); */
/* #endif */

/*         if(strcmp(dir_entry->name,filename)==0){ */
/*             if(get_inode(dir_entry->inode_index,p_inode)){ */
/*                 if((p_inode->i_mode & I_FILE) !=0){ */
/*                     return TRUE; */
/*                 } */
/*             } */
/*         } */
/*     } */
/*     return FALSE; */
/* } */

/* static void release_inode(INODE *pinode){ */
    
/* } */

static void sync_inode(int inode_index,INODE inode){
#ifdef DEBUG_FS
        printl("inode_index=%d(in sync_inode)\n",inode_index);
#endif
        int inode_first_index=get_inode_first_index(super_block);
        int off=inode_index*INODE_SIZE;
        u8 fsbuf[SECTOR_SIZE];
        
        inode_first_index+=off/SECTOR_SIZE;
        off=off%SECTOR_SIZE;
        
        rw_sector(INFO_FS_READ,
            ROOT_DEVICE,
            (inode_first_index)*SECTOR_SIZE,
            SECTOR_SIZE,
            TASK_FS,
            fsbuf);
        
        memcpy(fsbuf+off,&inode,INODE_SIZE);

        rw_sector(INFO_FS_WRITE,
            ROOT_DEVICE,
            (inode_first_index)*SECTOR_SIZE,
            SECTOR_SIZE,
            TASK_FS,
            fsbuf);
}

static void sync_sectors(int sector_index,int sector_length,u8 *data){
    /* printl("sector_index=%d(in sync_sectors)\n",sector_index); */
    rw_sector(INFO_FS_WRITE,
              ROOT_DEVICE,
              (sector_index)*SECTOR_SIZE,
              sector_length*SECTOR_SIZE,
              TASK_FS,
              data);
}

/* static void read_super_block(int dev){ */

/* } */

static void get_super_block(int dev,SUPER_BLOCK *super_block){
    u8 fsbuf[SECTOR_SIZE];
    int super_block_first_index=BOOT_SECTORS_LENGTH;
    /* READ_SECTOR(ROOT_DEVICE,fsbuf,super_block_first_index); */
    rw_sector(INFO_FS_READ,
              ROOT_DEVICE,
              (super_block_first_index)*SECTOR_SIZE,
              SUPER_BLOCK_SIZE,
              TASK_FS,
              fsbuf);
    
    memcpy(super_block,fsbuf,SUPER_BLOCK_SIZE);
}
