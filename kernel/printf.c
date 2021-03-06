
#include "stdlib.h"
#include "stdio.h"

#include "message.h"
#include "protect_mode.h"
#include "../fs/include/fs.h"
#include "../fs/include/file.h"
#include "process.h"
#include "syscall.h"

int printf(const char *fmt,...){
    int length=0;
    char buf[IO_BUFFER_SIZE];

    va_list args=(va_list)((char*)(&fmt)+4);
    length=vsprintf(buf,fmt,args);
    write(STDOUT,(void*)buf,length);

    return length;
}

int sprintf(char *str, const char *format, ...){
    int length=0;

    va_list args=(va_list)((char*)(&format)+4);
    length=vsprintf(str,format,args);

    return length;
}

int printl(const char *fmt,...){
    int length=0;
    char buf1[IO_BUFFER_SIZE];
    char buf2[IO_BUFFER_SIZE];
    va_list args=(va_list)((char*)(&fmt)+4);
    length=vsprintf(buf1,fmt,args);
    sprintf(buf2,"%c%s",MAG_CH_NORMAL,buf1);
    
    printx(buf2);

    return length;
}
