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

#include "syscall.h"
#include "misc.h"
#include "string.h"

static void block(PROCESS *p_process);
static void unblock(PROCESS *p_process);
static int deadlock(int src,int dest);
static int message_send(PROCESS *p_process,int dest,MESSAGE *p_message);
static int message_receive(PROCESS *p_process,int src,MESSAGE *p_message);

/*
<ring0>
当中断发生时，中断处理程序发送，告之特定程序中断已发生
运行在<ring0>不能调用<ring3>的函数send_receive
由于中断运行在内核态，直接可以给对应进程发送消息，
它在本质上区别与<ring1><ring3>态的进程，他们必须通过软中断，进入内核给进程发送消息(sys_send_receive)。
*/
void inform_int(int receiver,int msgtype){
    PROCESS *p=pid2process(receiver);
    if((p->flags & RECEIVING)&&
       ((p->receivefrom==INTERRUPT) || (p->receivefrom==ANY))){
        p->message->source_pid=INTERRUPT;
        p->message->type=msgtype;
        p->has_int_msg=0;
        p->flags &=~RECEIVING;
        p->receivefrom=NO_TASK;

        assert(p->flags==0,"");
        /* assert(p->message==NULL,""); */
        assert(p->receivefrom==NO_TASK,"");
        assert(p->sendto==NO_TASK,"");
    }
    else{
        p->has_int_msg=1;
    }
}

/*
<>
该函数调用sendreceive（软中断）进入内核态
*/
int send_receive(int function,int src_dest,MESSAGE *p_message){
    int ret=0;

    switch(function){
    case BOTH:
        ret=sendreceive(SEND,src_dest,p_message);
        if(ret==0){
            ret=sendreceive(RECEIVE,src_dest,p_message);
        }
        else{
            panic("want both(send but not receive)\n");
        }
        break;
    case SEND:
    case RECEIVE:
        ret=sendreceive(function,src_dest,p_message);
        break;
    default:
        assert((function==BOTH) ||
               (function==SEND) || (function==RECEIVE),"");
        break;
    }
    return ret;
}

/*
<ring 0>
*/
int sys_send_receive(int function, int src_dest,MESSAGE *p_message,PROCESS *p_process){

    assert(k_reenter==0,"reenter kernel");
    assert((0 <= src_dest && src_dest<NR_TASKS+NR_PROCS)||
           src_dest==ANY||
           src_dest==INTERRUPT,"");

    int result=0;
    int caller=process2pid(p_process);
    p_process->message=p_message;//指定消息属于消息发送者
    p_message->source_pid=caller;//必须在此指定消息的所有者

    MESSAGE *mla=(MESSAGE*)va2la(caller,p_message);
    
#ifdef DEBUG_MESSAGE
    if(0<=src_dest && src_dest<=NR_TASKS+NR_PROCS){
        printf("sys_send_receive: p_process=%s src_dest=%s msg_type=%d %d->%d\n",process2pname(p_process),pid2pname(src_dest),p_message->type,mla->source,src_dest);
    }else if(src_dest==ANY){
        printf("sys_send_receive: p_process=%s src_dest=%s msg_type=%d %d->%d\n",process2pname(p_process),"ANY",p_message->type,mla->source,src_dest);
    }else{
        printf("sys_send_receive: p_process=%s src_dest=%s msg_type=%d %d->%d\n",process2pname(p_process),"INTERRUPT",p_message->type,mla->source,src_dest);
    }
#endif

    assert(mla->source_pid!=src_dest,"");//MASSAGE初始化的时候必须指定其拥有者
    if(function == SEND){
        result=message_send(p_process,src_dest,p_message);
    }
    else if(function==RECEIVE){
        result=message_receive(p_process,src_dest,p_message);
    }
    else{
#ifdef DEBUG
        disp_str("invalid\n");
#endif
        panic("{sys_send_receive} invalid function/infotype"
            "%d (SEND:%d, RECEIVE:%d).",function,SEND,RECEIVE);
    }
    return result;
}

/*
<ring 0>
*/
static int message_send(PROCESS *p_process,int dest,MESSAGE *p_message){
    PROCESS *p_sender=p_process;
    PROCESS *p_dest=process_table+dest;

    //panic("error!");
    //assert(1==0,"error\n"); 

    assert(p_sender!=p_dest,"(send info to itself!)");

#ifdef DEBUG_
    disp_str("message_send: sender=");
    disp_str(p_sender->p_name);
    disp_str(" receiver=");
    disp_str(p_dest->p_name);
    disp_str(" msg_type=");
    disp_int(p_message->type);
    disp_str("\n");
#endif

    if(deadlock(process2pid(p_sender),dest)){
        panic(">>DEADLOCK<< %s->%s",p_sender->name,p_dest->name);
    }

    /* BOOL is_info_int=(INFO_INT_START<=p_message->type) &&  */
    /*     (p_message->type <= INFO_INT_END); */
    
    /* if(is_info_int){ */
    /*     //(is_info_int && p_message->type==p_dest->p_message->type) */
    /*     p_dest->n_waiting_int--; */

    //<ring 0> <ring 1>中特定进程给特定进程发送消息
    if((p_dest->flags & RECEIVING) &&
       (p_dest->receivefrom == process2pid(p_sender) ||
        p_dest->receivefrom == ANY)){
        
        assert(p_dest->message,"");
        assert(p_message,"");
        
        // phys_copy
        memcpy(va2la(dest,p_dest->message),
               va2la(process2pid(p_sender),p_message),
               sizeof(MESSAGE));
        
        p_sender->message=NULL;
        p_sender->receivefrom=NO_TASK;
        p_sender->sendto=NO_TASK;
        
        
        p_dest->message=NULL;
        p_dest->receivefrom=NO_TASK;
        p_dest->sendto=NO_TASK;
        p_dest->flags &= ~RECEIVING;//实际unblock位置
#ifdef DEBUG
        disp_str("(unblock)\n");
#endif
        unblock(p_dest);
        
#ifdef DEBUG_
        disp_str(p_dest->p_name);
        disp_str(" (message_send unblock)\n");
#endif
        
        assert(p_dest->flags==NORMAL,"");
        assert(p_dest->message==NULL,"");
        assert(p_dest->receivefrom==NO_TASK,"");
        assert(p_dest->sendto==NO_TASK,"");
        
        assert(p_sender->flags==NORMAL,"");
        assert(p_sender->message==NULL,"");
        assert(p_sender->receivefrom==NO_TASK,"");
        assert(p_sender->sendto==NO_TASK,"");
    }
    else{
        p_sender->flags |=SENDING;//设置进程标志为block
        p_sender->sendto=dest;
        p_sender->message=p_message;
        
	//把发送信息进程添加到接收消息进程的sending队列中
        PROCESS *p;
        if(p_dest->q_sending){
            p=p_dest->q_sending;
            while(p->next_sending){
                p=p->next_sending;
            }
            p->next_sending=p_sender;
        }
        else{
            p_dest->q_sending=p_sender;
        }
        p_sender->next_sending=NULL;
        
        block(p_sender);
#ifdef DEBUG_
        disp_str(p_sender->p_name);
        disp_str(" (message_send unblock)\n");
#endif
        assert(p_sender->flags & SENDING,"");
        assert(p_sender->message!=0,"");
        assert(p_sender->receivefrom==NO_TASK,"");
        assert(p_sender->sendto==dest,"");
    }
    return 0;
}

/*
<ring0>
*/
static int message_receive(PROCESS *p_process,int src,MESSAGE *p_message){
    PROCESS *p_wanna_receive=p_process;
    PROCESS *p_from=NULL;
    PROCESS *p_prev=NULL;
    BOOL copyok=FALSE;

    assert(process2pid(p_wanna_receive)!=src,"");

#ifdef DEBUG_
    disp_str("message_receive: receiver=");
    disp_str(p_wanna_receive->p_name);
    disp_str(" sender(pid)=");
    disp_int(src);
    disp_str(" msg_type=");
    disp_int(p_message->type);
    disp_str("\n");
#endif

    if(p_wanna_receive->has_int_msg &&
       ((src==ANY) || (src==INTERRUPT))){//中断消息
        MESSAGE msg;
        reset_message(&msg);
        msg.source_pid=INTERRUPT;

        memcpy(va2la(process2pid(p_wanna_receive),p_message),&msg,sizeof(MESSAGE));

        p_wanna_receive->has_int_msg=0;
        
        p_wanna_receive->flags=NORMAL;

        assert(p_wanna_receive->flags==NORMAL,"");
        assert(p_wanna_receive->message!=NULL,"");
        assert(p_wanna_receive->sendto==NO_TASK,"");
        assert(p_wanna_receive->has_int_msg==0,"");

        return 0;
    }

    if(src==ANY){//从任何进程接收消息（非中断消息）
#ifdef DEBUG_
        disp_str("message_receive(any)\n");
#endif
        if(p_wanna_receive->q_sending){
            p_from=p_wanna_receive->q_sending;
            copyok=TRUE;

            assert(p_wanna_receive->flags==0,"");
            /* assert(p_wanna_receive->p_message==NULL,""); *///在某个时刻没有设置为空，导致该断言不能通过
            assert(p_wanna_receive->receivefrom==NO_TASK,"");
            assert(p_wanna_receive->sendto==NO_TASK,"");
            assert(p_wanna_receive->q_sending!=0,"");
            assert(p_from->flags==SENDING,"");
            assert(p_from->message!=0,"");
            assert(p_from->receivefrom==NO_TASK,"");
            assert(p_from->sendto==process2pid(p_wanna_receive),"");
        }
    }
    else{//从指定的特定进程接收消息
#ifdef DEBUG_
        disp_str("message_receive(particular src)\n");
#endif
        p_from=&process_table[src];
        
        if((p_from->flags & SENDING) &&
           (p_from->sendto==process2pid(p_wanna_receive))){            

            PROCESS *p=p_wanna_receive->q_sending;
            assert(p,"");

            //在正发送消息给p_wanna_receive进程消息的链表中查找src/p_from
            while(p){//注意：p_from有可能是链表中的第一个，后面需要考虑这个问题
                assert(p_from->flags & SENDING,"");
                if(process2pid(p)==src){
                    copyok=TRUE;
                    p_from=p;
                    break;
                }
                p_prev=p;
                p=p->next_sending;
            }

            assert(p_wanna_receive->flags==NORMAL,"");
            /* assert(p_wanna_receive->message==NULL,""); */
            assert(p_wanna_receive->receivefrom==NO_TASK,"");
            assert(p_wanna_receive->sendto==NO_TASK,"");
            assert(p_wanna_receive->q_sending!=0,"");
            assert(p_from->flags==SENDING,"");
            assert(p_from->message!=NULL,"");
            assert(p_from->receivefrom==NO_TASK,"");
            assert(p_from->sendto==process2pid(p_wanna_receive),"");
        }
    }

    if(copyok){
        if(p_from==p_wanna_receive->q_sending){//如果p_from是链表中的第一个进程
            assert(p_prev==0,"");
            p_wanna_receive->q_sending=p_from->next_sending;
            p_from->next_sending=NULL;
        }else{
            assert(p_prev,"");
            p_prev->next_sending=p_from->next_sending;
            p_from->next_sending=NULL;
        }

        assert(p_message,"");
        assert(p_from->message,"");
        // phys_copy
        memcpy(va2la(process2pid(p_wanna_receive),p_message),
               va2la(process2pid(p_from),p_from->message),
               sizeof(MESSAGE));

        p_from->message=NULL;
        p_from->sendto=NO_TASK;
        p_from->flags &=~SENDING;


#ifdef DEBUG_
        disp_str(p_from->p_name);
        disp_str(" (message_receive unblock)\n");
#endif
        unblock(p_from);
    }
    else{
        p_wanna_receive->flags|=RECEIVING;
        p_wanna_receive->message=p_message;

        p_wanna_receive->receivefrom=src;
                
#ifdef DEBUG_
        disp_str(p_wanna_receive->p_name);
        disp_str(" (message_receive block)\n");
#endif
        block(p_wanna_receive);

        assert(p_wanna_receive->flags==RECEIVING,"");
        assert(p_wanna_receive->message!=0,"");
        assert(p_wanna_receive->receivefrom!=NO_TASK,"");
        assert(p_wanna_receive->sendto==NO_TASK,"");
        //assert(p_wanna_receive->n_waiting_int==0,"");
    }
    return 0;
}

/*
<ring0-3>
*/
void reset_message(MESSAGE *p_message){
    memset(p_message,0,sizeof(MESSAGE));
}

/*
<ring 0>
block进程
*/
static void block(PROCESS *p_process){
    assert(p_process->flags,"");
    schedule();
}

/*
<ring 0>
unblock进程
*/
static void unblock(PROCESS *p_process){
    assert(p_process->flags==NORMAL,"");
}

/*
判断是否存在死锁
<ring 0>
*/
static int deadlock(int src,int dest){
    PROCESS *p_process=process_table+dest;
    while(1){
        if(p_process->flags & SENDING){
            if(p_process->sendto==src){
                p_process=process_table+dest;
                /* printl("=_=%s",p_process->name); */
                do{
                    assert(p_process->message,"");
                    p_process=process_table+p_process->sendto;
                    /* printl("->%s",p_process->p_name); */
                }while(p_process!=process_table+src);
                /* printl("=_="); */
                return 1;
            }
            p_process=process_table+p_process->sendto;
        }
        else{
            break;
        }
    }
    return 0;
}
