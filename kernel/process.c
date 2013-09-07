#include "stdlib.h"

#include "i8259a.h"
#include "message.h"
#include "protect_mode.h"
#include "../fs/include/fs.h"
#include "process.h"
#include "tty.h"
#include "console.h"
#include "driver.h"
#include "kliba.h"
#include "global.h"

void schedule(){
    PROCESS *p;
    int greatest_ticks=0;
    while(!greatest_ticks){
        for(p=process_table;p<process_table+NR_TASKS+NR_PROCS;p++){
            if(p->flags==NORMAL){
                if(greatest_ticks<p->ticks){
                    greatest_ticks=p->ticks;
                    p_process_ready=p;
                }
            }
        }

        if(!greatest_ticks){
            for(p=process_table;p<process_table+NR_TASKS+NR_PROCS;p++){
                if(p->flags==NORMAL)
                    p->ticks=p->priority;
            }
        }
    }
}

void init_process(PROCESS *p_process){
    int i=0;
    for(i=0;i<FILE_COUNT;i++){
        p_process->file_descriptor[i]=NULL;
    }
    
    p_process->sendto=NO_TASK;
    p_process->receivefrom=NO_TASK;
    p_process->q_sending=NULL;
    p_process->next_sending=NULL;
    p_process->message=NULL;
    p_process->flags=NORMAL;
    p_process->has_int_msg=0;
}

int process2pid(PROCESS *p_process){
    return p_process->pid;
}

char* process2pname(PROCESS *p_process){
    return p_process->name;
}

PROCESS* pid2process(int pid){
    return &process_table[pid];
}

char* pid2pname(int pid){
    return process2pname(pid2process(pid));
}

int ldt_seg_linear(PROCESS *p_process,int index){
    DESCRIPTOR *p_desc=&p_process->ldts[index];
    return p_desc->base_high << 24 | p_desc->base_mid <<16 | p_desc->base_low;
}

void* va2la(int pid,const void *va){
    PROCESS *p_process=&process_table[pid];
   
    u32 seg_base=ldt_seg_linear(p_process,INDEX_LDT_DRW);
    u32 la=seg_base+(u32)va;

    if(pid<NR_TASKS+NR_PROCS){
        assert(la==(u32)va,"");
    }

    return (void*)la;
}


