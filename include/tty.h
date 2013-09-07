#include "tty_const.h"

typedef struct s_tty{
    u32 inbuf[TTY_IN_BYTES];
    u32 *p_inbuf_head;
    u32 *p_inbuf_tail;
    int inbuf_count;

    struct s_console *p_console;
}TTY;

/*kernel/tty.c*/
void task_tty();
/* void in_process(TTY *p_tty,u32 key); */
/* int sys_write(int _unused1,char *buf,int len,PROCESS *p_process); */
void sys_printx(int _unused1, int _unused2, char *s, PROCESS *p_process);
/**/
