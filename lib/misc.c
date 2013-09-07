#include "stdlib.h"
#include "misc.h"
#include "stdio.h"

void panic(const char* fmt,...){
    /* int i; */
    char buf[256];

    va_list args=(va_list)((char*)&fmt+4);

    vsprintf(buf,fmt,args);

    printl("%c!!panic!! (%s)",MAG_CH_PANIC,buf);
}

void assertion_failure(char *con,char *exp,char *file,char *base_file,int line){
    printl("%c%s [assert(%s) failed: file: %s, base_file: %s, line: %d]",
           MAG_CH_ASSERT,
           exp,con,file,base_file,line);
    /* printf("%cassert(%s) failed: file: %s, base_file: %s, line: %d", */
    /*        MAG_CH_ASSERT, */
    /*        exp,file,base_file,line); */

    //forever loop to prevent the proc from going on
    spin("assertion_failure()");

    //should never reach here
    __asm__ __volatile__("ud2");
}

void spin(char* function_name){
    printl("In the function:%s\n\n",function_name);
    while(1){}
}
