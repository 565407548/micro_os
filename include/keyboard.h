#include "keyboard_const.h"

typedef struct s_kb{
    char *p_head;
    char *p_tail;
    int count;
    char buf[KB_IN_BYTES];
}KB_INPUT;

/*kernel/keyboard.c*/
void init_keyboard();
/*
键盘中断程序，当键盘有按键按下时，会执行该程序，该程序会把按键的扫描码记录于键盘缓冲区中。
*/
void keyboard_handler(int irq);
BOOL get_key_pressed();
void set_key_pressed(BOOL b);

/*
当返回值为>=0时，才是有效值；其他值为无效值，应该忽略
*/
int/* void */ keyboard_read(/* TTY *p_tty */);
/**/
