#include "stdlib.h"

#include "message.h"
#include "protect_mode.h"
#include "../fs/include/fs.h"
#include "process.h"
#include "tty.h"
#include "console.h"
#include "driver.h"
#include "kliba.h"
#include "global.h"

#include "clock.h"
/* #include "keyboard.h"*/
#include "kernel.h"

#include "string.h"
#include "error.h"
#include "../fs/include/file.h"
#include "misc.h"
#include "stdio.h"

int kernel_main(){

    TASK *p_task=task_table;
    PROCESS *p_process=process_table;
    char *p_task_stack=task_stack+STACK_SIZE_TOTAL;
    u16 selector_ldt=SELECTOR_LDT_FIRST;

    u8 privilige;
    u8 rpl;
    int eflags;
    int i;
    for(i=0;i<NR_TASKS+NR_PROCS;i++){
        init_process(p_process);
        /*特权级引发GP错误，暂时如下处理。留待后续解决
          该问题主要是由于把PRIVILIGE_USER定义为2，而RPL_USER为3，
          所以会出现第优先级的访问高优先级的空间，出现GP错误。
          经过把PRIVILIGE_USER也修改为3，解决了问题
         */
        if(i<NR_TASKS){
            p_task=task_table+i;
            privilige=PRIVILIGE_TASK;
            rpl=RPL_TASK;
            eflags=0x1202;//IF=1,IOPL=1,bit 2 is always 1
        }
        else{
            p_task=user_proc_table+(i-NR_TASKS);
            privilige=PRIVILIGE_USER;
            rpl=RPL_USER;
            eflags=0x202;//IF=1,IOPL=0,bit 2 is always 1
        }

        strcpy(p_process->name,p_task->name);
        p_process->pid=i;

        p_process->ticks=p_process->priority=30;
        p_process->nr_tty=0;

        p_process->ldt_sel=selector_ldt;

        memcpy(&p_process->ldts[INDEX_LDT_C],&gdt[SELECTOR_KERNEL_CS>>3],sizeof(DESCRIPTOR));
        p_process->ldts[INDEX_LDT_C].attr1=DA_C|privilige<<5;

        memcpy(&p_process->ldts[INDEX_LDT_DRW],&gdt[SELECTOR_KERNEL_DS>>3],sizeof(DESCRIPTOR));
        p_process->ldts[INDEX_LDT_DRW].attr1=DA_DRW|privilige<<5;

        p_process->registers.cs=(8*0 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
        p_process->registers.ds=(8*1 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
        p_process->registers.es=(8*1 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
        p_process->registers.fs=(8*1 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
        p_process->registers.ss=(8*1 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
        p_process->registers.gs=(SELECTOR_KERNEL_GS & SA_RPL_MASK)|rpl;

        p_process->registers.eip=(u32)p_task->initial_eip;
        p_process->registers.esp=(u32)p_task_stack;/* 开始错误写成：p_process->registers.esp=(u32)task_stack，结果导致进程指挥循环执行一次。原因：所有进程的堆栈都指向相同的起始地址，进程第一次执行时，进程的eip时直接指定的，是正确的。但是当执行完第一遍后，之后寄存器的值（eip，esp等）都是从堆栈中获得的，而这会获得错误的eip，此时eip实际指向的是第一次循环最后一个执行的进程的eip，所以外在表现就是最后一个执行的程序会一直执行下去。 */

        p_process->registers.eflags=eflags;

        /* p_process->sendto=NO_TASK; */
        /* p_process->receivefrom=NO_TASK; */
        /* p_process->q_sending=0; */
        /* p_process->next_sending=0; */
        /* p_process->message=0; */
        /* p_process->flags=NORMAL; */
        /* p_process->has_int_msg=0; */

        p_task_stack-=p_task->stacksize;
        p_process++;
        p_task++;
        selector_ldt += 1<<3;
    }

    process_table[0].nr_tty=0;
    process_table[1].nr_tty=0;

    process_table[2].nr_tty=0;
    /* process_table[2].flags=3; */

    process_table[NR_TASKS+0].nr_tty=0;
    process_table[NR_TASKS+1].nr_tty=1;
    process_table[NR_TASKS+2].nr_tty=2;
    process_table[NR_TASKS+3].nr_tty=2;
    process_table[NR_TASKS+4].nr_tty=2;

    ticks=0;
    k_reenter=0;

    init_clock();
    /* init_keyboard(); *//*在tty初始化时，会初始化keyboard*/

    p_process_ready=process_table;
    restart();

    while(1){}
}



/*
<ring 3>
*/
/* void Task_fs(){ */
/*     int index; */
/*     MESSAGE driver_msg; */
/*     reset_message(&driver_msg); */

/*     driver_msg.type=INFO_FS_OPEN; */
/*     index=MAKE_DRIVER_DEVICE(DRIVER_HD,MAKE_DEVICE_HD(0,0)); */
/*     driver_msg.device=index; */
/*     assert(dd_map[DRIVER(index)].driver_pid!=PID_INVALID,""); */
/*     send_receive(BOTH,dd_map[DRIVER(index)].driver_pid,&driver_msg); */
/* #ifdef DEBUG_MAIN */
/*     printf("file open\n"); */
/* #endif */

/*     driver_msg.type=INFO_FS_CLOSE; */
/*     index=MAKE_DRIVER_DEVICE(DRIVER_HD,MAKE_DEVICE_HD(0,0)); */
/*     driver_msg.device=index; */
/*     assert(dd_map[DRIVER(index)].driver_pid!=PID_INVALID,""); */
/*     send_receive(BOTH,dd_map[DRIVER(index)].driver_pid,&driver_msg); */
/* #ifdef DEBUG_MAIN */
/*     printf("file close\n"); */
/* #endif */

/*     spin("Task_fs"); */
/* } */

/*
<ring 3>
*/
void TestA(){
    printf("1.this is a test for printf\n");
    /* printl("printl\n"); */
/* char str[]="abcdf"; */
/* char str1[10]; */
/* char str2[]="test4"; */
/* char ch='e'; */
/* char *p=strchr(str,ch); */
/* printf("%s\n%s\n",str,p!=NULL?p:"NULL"); */
/* printf("(str==str2)=%d\n",strcmp(str,str2)); */
/* printf("length(str)=%d %d\n",strlen(str),strlen(str)); */
/* printf("len=%d str1=%s\n",strcpy(str1,str),str1); */
/* memset(str1,'1',4); */
/* str1[4]=0; */
/* printf("str1=%s\n",str1); */

/*     while(1){ */
/*         printf(" A(%ds) ",get_ticks()); */
/*         delay(2000); */
/*     } */
    /* char pathname[20]; */
    /* char buf[SECTOR_SIZE*2]; */
    /* int flag=O_RDWR | O_CREATE | I_FILE; */
    /* int fd=0; */
    /* int length=0; */
    
    /* if(close(fd)<0){ */
    /* printf("%s\n",get_error_info()); */
    /* }else{ */
    /*     printf("file close\n"); */
    /* } */

    /* strcpy(pathname,"/test.txt"); */
    /* fd=open(pathname,flag); */
    /* if(fd<0) */
    /*     printf("open(%s) error:fd=%d error=%s\n",pathname,fd,get_error_info()); */
    /* else */
    /*     printf("open(%s) ok:fd=%d\n",pathname,fd); */

    /* strcpy(buf,"this is a test for hard disk write and read"); */
    /* length=write(fd,buf,strlen(buf)/\* 512 *\/); */
    /* if(length<0){ */
    /*     printf("error=%s\n",get_error_info()); */
    /* } */
    /* else{ */
    /*     printf("write(%d) ok!\n",length); */
    /* } */
    /* strcpy(buf,"12345678910"); */
    /* length=write(fd,buf,strlen(buf)/\* 512 *\/); */
    /* if(length<0){ */
    /*     printf("error=%s\n",get_error_info()); */
    /* } */
    /* else{ */
    /*     printf("write(%d) ok!\n",length); */
    /* } */
    /* seek(fd,0,SEEK_SET); */

    /* length=read(fd,buf,15); */
    /* if(length<0){ */
    /*     printf("error=%s\n",get_error_info()); */
    /* } */
    /* else{ */
    /*     printf("read from file:%s(%d)\n",buf,length); */
    /* } */
    /* length=read(fd,buf,15); */
    /* if(length<0){ */
    /*     printf("error=%s\n",get_error_info()); */
    /* } */
    /* else{ */
    /*     printf("read from file:%s(%d)\n",buf,length); */
    /* } */

    /* if(close(fd)<0){ */
    /*     printf("%s\n",get_error_info()); */
    /* }else{ */
    /*     printf("file close\n"); */
    /* } */
    
    /* if(!create(pathname,flag)){ */
    /*     printf("create file(%s) error:%s\n",pathname,get_error_info()); */
    /* } */
    /* else{ */
    /*     printf("create file(%s) ok%s\n",pathname); */
    /* } */
    
    /* strcpy(pathname,"/t2.txt"); */
    /* if(!create(pathname,flag)){ */
    /*     printf("create file(%s) error:%s\n",pathname,get_error_info()); */
    /* } */
    /* else{ */
    /*     printf("create file(%s) ok\n",pathname); */
    
    /*     fd=open(pathname,flag); */
    /*     while((length=write(fd,buf,strlen(buf)))>0){ */
    /*         strcpy(buf,"123456789"); */
    /*         /\* printf("write(%d) ok!\n",length); *\/ */
    /*     } */
    /*     if(length<0){ */
    /*         printf ("error=%s\n",get_error_info()); */
    /*     } */
    /* } */

    /* strcpy(pathname,"/t1.txt"); */
    /* fd=open(pathname,flag); */
    /* if(fd<0) */
    /*     printf("fd=%d error=%s\n",fd,get_error_info()); */
    /* else */
    /*     printf("fd=%d\n",fd); */

    /* strcpy(pathname,"/test.txt"); */
    /* if(!unlink(pathname,flag)) */
    /*     printf("unlink(%s) error:%s\n",pathname,get_error_info()); */
    /* else */
    /*     printf("unlink(pathname) ok\n"); */

    /* strcpy(pathname,"/t2.txt"); */
    /* if(!create(pathname,flag)){ */
    /*     printf("create file(%s) error:%s\n",pathname,get_error_info()); */
    /* } */
    /* else{ */
    /*     printf("create file(%s) ok\n",pathname); */
    /* } */
    
    /* char files[10*20]; */
    /* memset(files,0,200); */
    /* int file_count=ls("/",files); */
    /* if(file_count>0){ */
    /*     printf("files in dir:/\n"); */
    /*     //数据已经获得，文件名之间用'\0'隔开 */
    /*     int files_off=0,file_length; */
    /*     do{ */
    /*         printf("   "); */
    /*         file_length=printf("%s",files+files_off); */
    /*         printf("\n"); */
    /*         files_off+=file_length+1; */
    /*     }while(files_off<200 && file_length>0); */
    /* } */
    /* else{ */
    /*     printf("no files in dir:/\n"); */
    /* } */

    /* strcpy(buf,"10987654321"); */
    /* length=write(fd,buf,strlen(buf)/\* 512 *\/); */
    /* if(length<0){ */
    /*     printf("error=%s\n",get_error_info()); */
    /* } */
    /* else{ */
    /*     printf("write(%d) ok!\n",length); */
    /* } */
    /* if(close(fd)<0){ */
    /*     printf("%s\n",get_error_info()); */
    /* }else{ */
    /*     printf("file close\n"); */
    /* } */
    spin("TestA");
}

/*
  <ring 3>
*/
void TestB(){
while(1){
        printl("(B) ");
        delay(500);
    }
}

/*
<ring 3>
*/
void TestC(){
    while(1){
        /* get_ticks(); */
        printf(" C(%ds) ",get_ticks());
        delay(2000);
    }
}

/*
<ring 3>
*/
void TestD(){
    while(1){
        /* disp_str("[D] "); */
        printf("(D) ");
        delay(2000);
    }
}
