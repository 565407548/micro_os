#include "stdlib.h"
#include "string.h"
#include "stdio.h"

/*
返回有效字符串加最后一个‘0’（字符串结束标志）的总长度
*/
int vsprintf(char *buf,const char *fmt,va_list args){
    char *p;
    char tmp[IO_BUFFER_SIZE];
    va_list p_next_arg=args;

    for(p=buf;*fmt;fmt++){
        if(*fmt!='%'){
            *p++=*fmt;
            continue;
        }

        fmt++;
        switch(*fmt){
        case 'x':
            itoa(tmp,*(int*)p_next_arg,16);
            strcpy(p,tmp);
            p_next_arg+=4;
            p+=strlen(tmp);
            break;
        case 'd':
            itoa(tmp,*(int*)p_next_arg,10);
            strcpy(p,tmp);
            p_next_arg+=4;
            p+=strlen(tmp);
            break;
        case 'c':
            *p++ = *((char*)p_next_arg);
            p_next_arg += 4;
            break;
        case 's':
            strcpy(p,*(char**)p_next_arg);
            p+=strlen(*(char**)p_next_arg);
            p_next_arg+=4;
            break;
        default:
            break;
        }
    }
    *p=0;

    return (p-buf);
}
