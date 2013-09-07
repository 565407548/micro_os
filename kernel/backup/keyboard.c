#include "stdlib.h"
#include "string.h"

#include "i8259a.h"

#include "keyboard.h"
#include "keymap.h"

static KB_INPUT kb_in;
static int code_with_E0=0;
static int shift_l;
static int shift_r;
static int alt_l;
static int alt_r;
static int ctrl_l;
static int ctrl_r;
static int caps_lock;
static int num_lock;
static int scroll_lock;
static int column;

static void kb_wait();
static void kb_ack();
static void set_leds();
static u8 get_byte_from_kbuf();

void init_keyboard(){
//局部变量必须在函数中进行初始化，否则是系统任意指定的一个值
    code_with_E0=0;
    
    shift_l=0;
    shift_r=0;
    alt_l=0;
    alt_r=0;
    ctrl_l=0;
    ctrl_r=0;
    
    caps_lock=0;
    scroll_lock=0;
    num_lock=0;
    set_leds();

    /* column=0; */

    kb_in.count=0;
    kb_in.p_head=kb_in.p_tail=kb_in.buf;

    //注册键盘中断
    put_irq_handler(KEYBOARD_IRQ,keyboard_handler);
    enable_irq(KEYBOARD_IRQ);
}

/*
键盘中断处理程序
当键盘有按键按下时，发生键盘中断，会执行键盘中断程序，该程序会读取键盘按键产生的扫描码
*/
void keyboard_handler(int irq){
    u8 scan_code=in_byte(KB_DATA);

#ifdef DEBUG
    disp_int(scan_code);
#endif

    if(kb_in.count<KB_IN_BYTES){
        *(kb_in.p_head)=scan_code;
        kb_in.p_head++;
        if(kb_in.p_head==kb_in.buf+KB_IN_BYTES)
        {
            kb_in.p_head=kb_in.buf;
        }
        kb_in.count++;
    }
}

int keyboard_read(){
    u8 scan_code;
    char output[2];
    int make;

    int key=0;

    u32 *keyrow;

    memset(output,0,2);

    if(kb_in.count>0)
    {
        code_with_E0=0;

        scan_code=get_byte_from_kbuf();

        if(scan_code==0xE1){
            int i;
            u8 pausebreak_scode[]={0xE1,0x1D,0x45,
                                   0xE1,0x9D,0xC5};
            int is_pausebreak=1;
            for(i=1;i<6;i++){
                if(get_byte_from_kbuf()!=pausebreak_scode[i]){
                    is_pausebreak=0;
                    break;
                }
            }
            if(is_pausebreak){
                key=PAUSEBREAK;
            }
        }
        else if(scan_code==0xE0){
            scan_code=get_byte_from_kbuf();

            if(scan_code==0x2A){
                if(get_byte_from_kbuf()==0xE0){
                    if(get_byte_from_kbuf()==0x37){
                        key=PRINTSCREEN;
                        make=1;
                    }
                }
            }
            if(scan_code==0xB7){
                if(get_byte_from_kbuf()==0xE0){
                    if(get_byte_from_kbuf()==0xAA){
                        key=PRINTSCREEN;
                        make=0;
                    }
                }
            }
            if(key==0){
                code_with_E0=1;
            }
        }

        if(key!=PRINTSCREEN && key!=PAUSEBREAK){
            make=(scan_code & FLAG_BREAK ? FALSE : TRUE);
            keyrow=&keymap[(scan_code & 0x7F)*MAP_COLS];

            column=0;

            int caps=shift_l || shift_r;
            if(caps_lock){
                if(keyrow[0]>='a' && keyrow[0]<='z'){
                    caps=!caps;
                }
            }
            if(caps){
                column=1;
            }
            if(code_with_E0){
                column=2;
                code_with_E0=0;
            }

            key=keyrow[column];
            switch(key){
            case SHIFT_L:
                shift_l=make;
                break;
            case SHIFT_R:
                shift_r=make;
                break;
            case CTRL_L:
                ctrl_l=make;
                break;
            case CTRL_R:
                ctrl_r=make;
                break;
            case ALT_L:
                alt_l=make;
                break;
            case ALT_R:
                alt_r=make;
                break;
            case CAPS_LOCK:
                if(make){
                    caps_lock=!caps_lock;
                    set_leds();
                }
                break;
            case NUM_LOCK:
                if(make){
                    num_lock=!num_lock;
                    set_leds();
                }
                break;
            case SCROLL_LOCK:
                if(make){
                    scroll_lock=!scroll_lock;
                    set_leds();
                }
                break;
            default:
                break;
            }
            if(make){
                int pad=0;
                int num_flag=shift_l || shift_r;
                if(num_lock){
                    num_flag=!num_flag;
                }
                if(key>=PAD_DIV && key<=PAD_9){
                    pad=1;
                    switch(key){
                    case PAD_DIV:
                        key='/';
                        break;
                    case PAD_MUL:
                        key='*';
                        break;
                    case PAD_MINUS:
                        key='-';
                        break;
                    case PAD_PLUS:
                        key='+';
                        break;
                    case PAD_ENTER:
                        key=ENTER;//key=PAD_ENTER;
                    default:
                        if(num_flag && key>=PAD_0 && key<=PAD_9){
                            key=key-PAD_0+'0';
                        }
                        else if(num_flag && key==PAD_DOT){
                            key='.';
                        }
                        else{
                            switch(key){
                            case PAD_DEL:
                                key=DELETE;
                                break;
                            case PAD_INS:
                                key=INSERT;
                                break;
                            case PAD_END:
                                key=END;
                                break;
                            case PAD_DOWN:
                                key=DOWN;
                                break;
                            case PAD_PAGEDOWN:
                                key=PAGEDOWN;
                                break;
                            case PAD_LEFT:
                                key=LEFT;
                                break;
                            case PAD_MID:
                                key=PAD_MID;
                                break;
                            case PAD_RIGHT:
                                key=RIGHT;
                                break;
                            case PAD_HOME:
                                key=HOME;
                                break;
                            case PAD_UP:
                                key=UP;
                                break;
                            case PAD_PAGEUP:
                                key=PAGEUP;
                                break;
                            }
                        }
                        break;
                    }
                }
                key|=shift_l ? FLAG_SHIFT_L : 0;
                key|=shift_r ? FLAG_SHIFT_R : 0;
                key|=alt_l ? FLAG_ALT_L : 0;
                key|=alt_r ? FLAG_ALT_R : 0;
                key|=ctrl_l ? FLAG_CTRL_L : 0;
                key|=ctrl_r ? FLAG_CTRL_R : 0;
                key|=pad ? FLAG_PAD : 0;

                /* in_process(p_tty,key); */
                return key;
            }
        }
    }
    return -1;
}

/*
等待缓冲区空
当状态码第一位为0时，则表示缓冲区为空
*/
static void kb_wait(){
    u8 kb_stat;
    do{
        kb_stat=in_byte(KB_STATUS);
    }while(kb_stat & 0x02);
}

/*
等待回复ACK
即等待键盘回复一个0xFA
*/
static void kb_ack(){
    u8 kb_read;
    do{
        kb_read=in_byte(KB_DATA);
    }while(kb_read!=KB_ACK);
}

/*
由于是在bochs虚拟机下，不好测试键盘led灯，所以先放一放
设置led灯的过程（往8048发送命令）：
1.向0x60端口0xED
2.等待键盘接受这个命令，键盘接受这个命令后，会回复一个0xFA
3.接到回复后，再向0x60发送led设置参数值
4.当键盘收到设置参数值后，同样会回复一个0xFA，整个设置过程完成。
*/
static void set_leds(){
    u8 leds=(caps_lock<<2) | (num_lock<<1) | scroll_lock;
    kb_wait();
    out_byte(KB_DATA,LED_CODE);
    kb_ack();

    kb_wait();
    out_byte(KB_DATA,leds);
    kb_ack();
}

/*
从键盘缓冲区中获得一个扫描码
由于该过程会读取键盘缓冲区，所以为了避免同时又出现键盘缓冲区写的情况，在处理前，先关中断，处理完毕后，再开中断。
*/
static u8 get_byte_from_kbuf(){
    u8 scan_code;
    while(kb_in.count<=0){}

    disable_int();
    scan_code=*(kb_in.p_tail);
    kb_in.p_tail++;
    if(kb_in.p_tail==kb_in.buf+KB_IN_BYTES){
        kb_in.p_tail=kb_in.buf;
    }
    kb_in.count--;
    enable_int();

    return scan_code;
}
