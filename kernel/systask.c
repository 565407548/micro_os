#include "stdlib.h"

#include "message.h"
#include "driver.h"
#include "protect_mode.h"
#include "../fs/include/fs.h"
#include "process.h"
#include "tty.h"
#include "console.h"
#include "global.h"

#include "misc.h"

void task_sys(){
    MESSAGE message;

    while(1){
        reset_message(&message);
        send_receive(RECEIVE,ANY,&message);

        int src=message.source_pid;
        switch(message.type){
        case INFO_OTHER_TICKS: 
            /* message.return_type=t_int; */
            message.res_int=ticks;
            //必须在消息发送前，指定发送消息的源
            message.source_pid=process2pid(p_process_ready);
            break;
        default:
            panic("unkonw message type");
            break;
        }
        send_receive(SEND,src,&message);
    }
}

