#include "stdlib.h"

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

static void tty_do_write(TTY *p_tty);
static void tty_do_read(TTY *p_tty);
static void tty_write(TTY *p_tty, const char *str);
static void init_tty(TTY *p_tty);
static void put_key(TTY *p_tty,u32 key);

/*
终端服务程序
1.接受键盘输出
2.处理输出
*/
void task_tty(){

#ifdef DEBUG_TTY
    /* printf("tty"); */

    /* int i; */
    /* char buf[10]; */
    /* for(i=0;i*160<V_MEM_SIZE;i++) */
    /* { */
    /*     display_position=i*160; */
    /*     disp_str(itoa(buf,i+1,10)); */
    /* } */
    display_position=0;
    disp_str("test");
#endif

    TTY *p_tty;

    init_keyboard();

    for(p_tty=TTY_FIRST;p_tty<TTY_END;p_tty++){
        init_tty(p_tty);
    }

    select_console(0);//默认为0号终端

    while(1){
        //轮询处理各个终端的输入输出
        for(p_tty=TTY_FIRST;p_tty<TTY_END;p_tty++){
            tty_do_write(p_tty);
            tty_do_read(p_tty);
        }
    }
}

/*
终端接受的但哥数据处理方式，如何处理
*/
void in_process(TTY *p_tty, u32 key){
    if(!(key & FLAG_NP)){
        put_key(p_tty,key);
    }
    else{
        u32 i_key=key & FLAG_AUX_MASK;
        switch(i_key){
        case UP:
#ifdef DEBUG_TTY
            disp_str("up");
#endif
            scroll_screen_up(1);
            break;
        case DOWN:
#ifdef DEBUG_TTY
                disp_str("down");
#endif
            scroll_screen_down(1);
            break;
        case ENTER:
#ifdef DEBUG_TTY
            disp_str("enter");
#endif
            put_key(p_tty,'\n');
            break;
        case BACKSPACE:
#ifdef DEBUG_TTY
                disp_str("backspace");
#endif
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
#ifdef DEBUG_TTY
                disp_str("default");
#endif
            break;
        }
    }
}

/* int sys_write(int _unused1,char *buf,int len,PROCESS *p_process){ */
/*     tty_write(&tty_table[p_process->nr_tty],buf,len); */
/*     return 0; */
/* } */

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
#ifdef DEBUG_TTY
        v=(char*)(10*NR_BYTES_PER_LINE);
#endif
        while(v<(char*)(V_MEM_BASE+V_MEM_SIZE)){
            *v++=*q++;
            *v++=RED_COLOR;
            if(!*q){//q为空，即完整输出一次后，换行，之后接着重新输出
                while(((int)v-V_MEM_BASE)%NR_BYTES_PER_LINE){
                    *v++=' ';
                    *v++=BLACK_COLOR;
                }
                q=p+1;
            }
        }
        /* while(*q){ */
        /*     *v++=*q++; */
        /*     *v++=RED_CHAR; */
        /* } */
        
        __asm__ __volatile__("hlt");
    }
    else if(*p==MAG_CH_ASSERT || *p==MAG_CH_NORMAL){
        ++p;
        tty_write(&tty_table[p_process->nr_tty],p);
    }
    
    /* while((ch=*p++)!=0){ */
    /*     if(ch==MAG_CH_PANIC || ch==MAG_CH_ASSERT) */
    /*         continue; */
    /*     out_char(tty_table[p_process->nr_tty].p_console,ch); */
    /* } */
}

/*
终端从键盘读取输入数据
*/
static void tty_do_read(TTY *p_tty){
    if(is_current_console(p_tty->p_console)){
        int key=keyboard_read(p_tty);
        if(key>=0){
            in_process(p_tty,key);
        }
    }
}

static void tty_do_write(TTY *p_tty){
    if(p_tty->inbuf_count){
        char ch=*(p_tty->p_inbuf_tail);
        p_tty->p_inbuf_tail++;
        if(p_tty->p_inbuf_tail==p_tty->inbuf+TTY_IN_BYTES){
            p_tty->p_inbuf_tail=p_tty->inbuf;
        }
        p_tty->inbuf_count--;

        out_char(p_tty->p_console,ch);
    }
}

static void tty_write(TTY *p_tty,const char *buf){
    const char *p=buf;
    /* int i=len; */
    while(*p){
        out_char(p_tty->p_console,*p++);
        /* i--; */
    }
}

static void init_tty(TTY *p_tty){
    p_tty->inbuf_count=0;
    p_tty->p_inbuf_head=p_tty->p_inbuf_tail=p_tty->inbuf;

    init_console(p_tty->p_console,p_tty-tty_table);
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
