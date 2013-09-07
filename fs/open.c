#include "stdlib.h"
#include "stdio.h"

#include "message.h"
#include "protect_mode.h"
#include "include/fs.h"
#include "process.h"

#include "syscall.h"


BOOL create(const char *pathname,int flags){
    MESSAGE message;
    message.type=INFO_FS_CREATE;
    message.arg_pointer=pathname;
    message.flags=flags;

    send_receive(BOTH,TASK_FS,&message);
    return message.res_bool;
}

BOOL unlink(const char *pathname,int flags){
    MESSAGE message;
    message.type=INFO_FS_UNLINK;
    message.arg_pointer=pathname;
    message.flags=flags;

    send_receive(BOTH,TASK_FS,&message);
    return message.res_bool;
}

int ls(const char *dirname,char *files){
    MESSAGE message;
    message.type=INFO_FS_LS;
    message.arg_pointer=dirname;
    message.res_pointer=files;

    send_receive(BOTH,TASK_FS,&message);

    return message.res_int;
}

//return file descriptor
int open(const char *pathname,int flags){
    MESSAGE message;
    message.type=INFO_FS_OPEN;
    message.arg_pointer=pathname;
    message.flags=flags;

    send_receive(BOTH,TASK_FS,&message);
    return message.fd;
}

//return file descriptor
int write(int fd,const void *buf,int length){
    MESSAGE message;
    message.type=INFO_FS_WRITE;
    message.fd=fd;
    message.arg_pointer=buf;/*保存写入内容的地址*/
    message.length=length;
    
    send_receive(BOTH,TASK_FS,&message);
    return message.length;
}

//return file descriptor
int read(int fd,void *buf,int length){
    MESSAGE message;
    message.type=INFO_FS_READ;
    message.fd=fd;
    message.res_pointer=buf;/*保存读取结果的地址*/
    message.length=length;
    
    send_receive(BOTH,TASK_FS,&message);
    return message.length;
}

void seek(int fd,int offset,int whence){
    MESSAGE message;
    message.type=INFO_FS_SEEK;
    message.fd=fd;
    message.offset=offset;
    message.whence=whence;
    
    send_receive(BOTH,TASK_FS,&message);
}

//return file descriptor
int close(int fd){
    MESSAGE message;
    message.type=INFO_FS_CLOSE;
    message.fd=fd;

    send_receive(BOTH,TASK_FS,&message);
    return message.res_int;
}
