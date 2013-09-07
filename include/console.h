#include "console_const.h"

typedef struct s_console{
    /*以下四个数据都是以bytes为单位，而不是以char为单位。也就是说它们都是2的倍数*/
    u32 current_start_addr;
    u32 original_addr;
    u32 v_mem_limit;
    u32 cursor;
    
    u8 color;/*前景背景色*/
}CONSOLE;

/*kernel/console.c*/

/* void init_console(TTY *p_tty); */
void init_console(CONSOLE *p_console,int nr_tty);
int is_current_console(CONSOLE *p_console);

void out_char(CONSOLE *p_console,char ch);
void select_console(int nr_console);

void scroll_screen_down(int row);
void scroll_screen_up(int row);

void move_cursor_left(int moves);
void move_cursor_right(int moves);
void move_cursor_up(int moves);
void move_cursor_down(int moves);

void next_line(CONSOLE *p_console);
void backspace(CONSOLE *p_console);

void set_color(CONSOLE *p_console,u8 fore_color,u8 back_color);
void set_back_color(CONSOLE *p_console,u8 back_color);
void set_fore_color(CONSOLE *p_console,u8 fore_color);
u8 get_back_color(CONSOLE *p_console);
u8 get_fore_color(CONSOLE *p_console);
/* void flush(); */
/**/
