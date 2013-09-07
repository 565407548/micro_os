#include "process_const.h"

typedef struct s_stackframe{
    u32 gs;
    u32 fs;
    u32 es;
    u32 ds;
    u32 edi;
    u32 esi;
    u32 ebp;
    u32 kernel_esp;
    u32 ebx;
    u32 edx;
    u32 ecx;
    u32 eax;
    u32 retaddr;
    u32 eip;
    u32 cs;
    u32 eflags;
    u32 esp;
    u32 ss;
}STACK_FRAME;


typedef struct s_process{
    struct s_stackframe registers;

    u16 ldt_sel;
    DESCRIPTOR ldts[LDT_SIZE];
    
    int ticks;
    int priority;

    u32 pid;
    char name[16];

    int nr_tty;

    //进程通信
    int sendto;//发送消息给pid=process_sendto的进程
    struct s_process *q_sending;//发消息给该进程的进程pointer
    struct s_process *next_sending;//同该进程一样，要发送消息给同样进程的进程pointer
    int receivefrom;//从pid=process_receivefrom的进程接收消息
    int flags;//进程状态，接收，发送或无消息
    MESSAGE  *message;
    int has_int_msg;

    //file
    struct s_file_descriptor* file_descriptor[FILE_COUNT];
}PROCESS;

typedef struct s_tack{
    task_f initial_eip;
    int stacksize;
    char name[32];
}TASK;

/*kernel/process.c*/
void schedule();
void init_process(PROCESS *p_process);
int process2pid(PROCESS *p_process);
char* process2pname(PROCESS *p_process);
PROCESS* pid2process(int pid);
char* pid2pname(int pid);
int ldt_seg_linear(PROCESS *p_process,int index);
void *va2la(int pid,const void *va);

int sys_send_receive(int function, int src_dest,MESSAGE *p_message,PROCESS *p_process);
/**/
