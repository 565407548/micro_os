#include "stdlib.h"
#include "stdio.h"
#include "string.h"

#include "i8259a.h"
#include "message.h"
#include "protect_mode.h"
#include "../fs/include/fs.h"
#include "process.h"
#include "tty.h"
#include "console.h"
#include "keyboard.h"
#include "driver.h"
#include "kliba.h"
#include "global.h"

static void tty_dev_write(TTY *p_tty);
static void tty_dev_read(TTY *p_tty);
static void tty_do_write(MESSAGE *msg);
static void tty_do_read(MESSAGE *msg);
static void tty_write(TTY *p_tty, const char *str);
static void init_tty(TTY *p_tty);
static void put_key(TTY *p_tty,u32 key);
static void in_process(TTY *p_tty, u32 key);
static TTY* get_tty(int tty_index);

void task_tty(){
    TTY *p_tty;

#ifdef DEBUG_TTY
    /* int i; */
    /*     char buf[10]; */
    /*     for(i=0;i*160<V_MEM_SIZE;i++) */
    /*     { */
    /*         display_position=i*160; */
    /*         disp_str(itoa(buf,i+1,10)); */
    /*     } */
    /*     display_position=0; */
#endif

    init_keyboard();

    for(p_tty=TTY_FIRST;p_tty<TTY_END;p_tty++){
        init_tty(p_tty);
    }

    select_console(0);

    MESSAGE message;
    while(TRUE){
        reset_message(&message);
        send_receive(RECEIVE,ANY,&message);
        
        /* int source_pid=message.source_pid; */
        switch(message.type){
        case INFO_FS_READ:
            tty_do_read(&message);
            break;
        case INFO_FS_WRITE:
            assert(message.fd==STDOUT,"");
            tty_do_write(&message);
            break;
        case INFO_INT_KEYPRESSED:
            key_pressed=FALSE;
            break;
        default:
            assert(FALSE,"unknown message type");
        }
        /* if(message.type!=INFO_SUSPEND_PROCESS){ */
        /*     send_receive(SEND,source_pid,&message); */
        /* } */
        for(p_tty=TTY_FIRST;p_tty<TTY_END;p_tty++){
            do{
                tty_dev_read(p_tty);
                tty_dev_write(p_tty);
            }while(p_tty->inbuf_count>0);
        }
    }

    /* screen_start_addr=V_MEM_BASE; */
    /* while(1){ */
    /*     keyboard_read(); */
    /* } */
}

static void in_process(TTY *p_tty, u32 key){
    if(!(key & FLAG_NP)){
        put_key(p_tty,key);
    }
    else{
        u32 i_key=key & FLAG_AUX_MASK;
        switch(i_key){
        case UP:
            /* scroll_screen_up(1); */
            move_cursor_up(1);
            break;
        case DOWN:
            /* scroll_screen_down(1); */
            move_cursor_down(1);
            break;
        case LEFT:
            move_cursor_left(1);
            break;
        case RIGHT:
            move_cursor_right(1);
            break;
        case ENTER:
            put_key(p_tty,'\n');
            break;
        case BACKSPACE:
            put_key(p_tty,'\b');
            break;
        case F1:
        case F2:
        case F3:
            if(1){
                select_console(i_key-F1);//目前F1-F10连续，F11,F12不连续
            }
            break;
        default:
            break;
        }
    }
}

void sys_printx(int _unused1, int _unused2, char *s, PROCESS *p_process){
    const char *p;
    /* char ch; */

    char reenter_error[]="?k_reenter is incorrect for unknown reason.";
    reenter_error[0]=MAG_CH_PANIC;

    if(k_reenter==0)/*发生重入*/
        p=va2la(process2pid(p_process),s);
    else if(k_reenter>0)
        p=s;
    else
        p=reenter_error;

    /*
      panic或在level0，level1发生assert。
      注意：输出字符串不能识别特殊字符，如\b,\n等
    */
    if((*p==MAG_CH_PANIC) ||
       (*p==MAG_CH_ASSERT && p_process_ready < &process_table[NR_TASKS])){
        disable_int();
        char *v=(char*)V_MEM_BASE;
        const char *q=p+1;//*q不能变
/* #ifdef DEBUG_TTY */
        v=(char*)(10*NR_BYTES_PER_LINE);
/* #endif */
        /* while(v<(char*)(V_MEM_BASE+V_MEM_SIZE)){ */
        /*     *v++=*q++; */
        /*     *v++=RED_COLOR; */
        /*     if(!*q){//q为空，即完整输出一次后，换行，之后接着重新输出 */
        /*         while(((int)v-V_MEM_BASE)%NR_BYTES_PER_LINE){ */
        /*             *v++=' '; */
        /*             *v++=BLACK_COLOR; */
        /*         } */
        /*         q=p+1; */
        /*     } */
        /* } */
        while(*q){
            *v++=*q++;
            *v++=RED_COLOR;
        }
        
        __asm__ __volatile__("hlt");
    }
    else if(*p==MAG_CH_ASSERT || *p==MAG_CH_NORMAL){
        ++p;
        tty_write(&tty_table[p_process->nr_tty],p);
    }
}

static void init_tty(TTY *p_tty){
    p_tty->inbuf_count=0;
    p_tty->p_inbuf_head=p_tty->p_inbuf_tail=p_tty->inbuf;

    int nr_tty=p_tty-tty_table;
    p_tty->p_console=console_table+nr_tty;
    init_console(p_tty->p_console,nr_tty);
    /* init_console(p_tty->p_console,p_tty-tty_table); */
}

/*
从键盘缓冲区中读取一个数据入tty缓冲区
*/
static void tty_dev_read(TTY *p_tty){
    if(is_current_console(p_tty->p_console)){
        int key=keyboard_read();
        if(key>=0){
            in_process(p_tty,key);
        }
    }
}

/*
把tty缓冲区中的一个数据写入console
*/
static void tty_dev_write(TTY *p_tty){
    if(p_tty->inbuf_count){
        char ch=*(p_tty->p_inbuf_tail);
        p_tty->p_inbuf_tail++;
        if(p_tty->p_inbuf_tail==p_tty->inbuf+TTY_IN_BYTES){
            p_tty->p_inbuf_tail=p_tty->inbuf;
        }
        p_tty->inbuf_count--;

        out_char(p_tty->p_console,ch);
        /* if(p_tty->tty_left_count>0){ */
        /*     if(ch>=' ' && ch<='~'){/\*printable*\/ */
        /*         out_char(tty->p_console,ch); */
        /*         void *p=tty->tty_req_buf+tty->tty_trans_count; */
        /*         memcpy(p,(void*)va2la(TASK_TTY,&ch),1); */
        /*         tty->tty_trans_count++; */
        /*         tty->tty_left_count--; */
        /*     } */
        /*     else if(ch=='\b' && tty->tty_trans_count){ */
        /*         out_char(tty->console,ch); */
        /*         tty->tty_trans_count--; */
        /*         tty->tty_left_count++; */
        /*     } */
        /*     if(ch=='\n' || tty->tty_trans_count==0){ */
        /*         out_char(tty->p_console,ch); */
        /*         MESSAGE msg; */
        /*         msg.type=INFO_RESUME_PROCESS; */
        /*         msg.process_index=tty->process_index; */
        /*         msg.length=tty->tty_trans_count; */
        /*         send_receive(SEND,tty->tty_caller,&msg); */
        /*         tty->tty_left_count=0; */
        /*     } */
        /* } */
    }
}

static void tty_do_read(MESSAGE *msg){
    /* tty->tty_caller=msg->source_pid;/\*usually fs*\/ */
    /* tty->tty_process_index=msg->process_index;/\*the process who want to read*\/ */
    /* tty->tty_req_buf=va2la(tty->tty_process_index,msg->res_pointer); */
    
    /* tty->tty_left_count=msg->length; */
    /* tty->tty_trans_count=0; */
    
    /* msg->type=INFO_SUSPEND_PROCESS; */
    /* msg->length=tty->tty_left_count; */
    /* send_recv(SEND,tty->tty_caller,msg); */
}

static void tty_do_write(MESSAGE *msg){
    char buf[TTY_OUT_BUF_LEN];
    char *p=(char*)va2la(msg->process_index,msg->arg_pointer);
    int i=msg->length;
    int j;
    /* int tty_index= DEVICE(message.device) ; */
    PROCESS *process=pid2process(msg->process_index);
    CONSOLE *p_console=(get_tty(process->nr_tty))->p_console;

    while(i>0){
#ifdef DEBUG_TTY
        out_char((get_tty(process->nr_tty))->p_console,i%10+'0');
#endif
        int bytes=min(TTY_OUT_BUF_LEN,i);
        memcpy(va2la(TASK_TTY,buf),(void*)p,bytes);
        for(j=0;j<bytes;j++){
            out_char(p_console,buf[j]);
        }
        i-=bytes;
        p+=bytes;
    }

    /* msg->type=INFO_SYSCALL_RET; */
    /* send_receive(SEND,msg->process_index,msg); */
}

/*
把buf中的数据写入tty的console
*/
static void tty_write(TTY *p_tty,const char *buf){
    const char *p=buf;
    while(*p){
        out_char(p_tty->p_console,*p++);
    }
}

static void put_key(TTY *p_tty,u32 key){
    if(p_tty->inbuf_count<TTY_IN_BYTES){
        *(p_tty->p_inbuf_head)=key;
        p_tty->p_inbuf_head++;
        if(p_tty->p_inbuf_head==p_tty->inbuf+TTY_IN_BYTES)
        {
            p_tty->p_inbuf_head=p_tty->inbuf;
        }
        p_tty->inbuf_count++;
    }
}

static TTY* get_tty(int tty_index){
    return tty_table+tty_index;
}
