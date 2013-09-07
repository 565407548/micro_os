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

#include "clock.h"

void init_clock(){
    //8253,8254
    out_byte(TIMER_MODE,RATE_GENERATOR);
    out_byte(TIMER0,(u8)(TIMER_FREQ/HZ));
    out_byte(TIMER0,(u8)((TIMER_FREQ/HZ)>>8));

    put_irq_handler(CLOCK_IRQ,clock_handler);
    enable_irq(CLOCK_IRQ);
}

void clock_handler(int irq){
    ticks++;
    p_process_ready->ticks--;

    if(k_reenter!=0){
        return;
    }
    
    if(key_pressed){
        inform_int(TASK_TTY,INFO_INT_KEYPRESSED);
    }

    schedule();
}

void milli_delay(int milli_sec){
    int t=get_ticks();
    while(((get_ticks()-t)*1000/HZ)<milli_sec){}
}

/*
功能：
    获得系统当前的tick数，由时钟中断控制器产生的
参数：
    （无）
返回值：
    返回系统启动至今的tick数，即发生时钟中断的次数
*/

int get_ticks(){
    MESSAGE message;
    reset_message(&message);
    message.source_pid=process2pid(p_process_ready);
    message.type=INFO_OTHER_TICKS;
    /* message.return_type=t_int; */
    send_receive(BOTH,TASK_SYS,&message);
    return message.res_int;
}

/*
功能：获得系统当前的启动的至今的秒数（单位：ms），根据ticks转换而来。
参数：
    （无）
返回值：
    返回系统启动至今的秒数（单位：ms）
*/
int get_milli_seconds(){
    return get_ticks()*1000/HZ;
}
