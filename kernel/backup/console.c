#include "stdlib.h"
#include "kliba.h"
#include "stdio.h"

#include "message.h"
#include "driver.h"
#include "screen.h"
#include "protect_mode.h"
#include "../fs/include/fs.h"
#include "process.h"
#include "tty.h"
#include "console.h"
#include "global.h"

static void set_char(u8 *position,char ch/* ,u8 color */);

/*
初始化tty下标为nr_tty的console
*/
void init_console(CONSOLE *p_console,int nr_tty){
    /* int nr_tty=p_tty-tty_table; */
    /* p_tty->p_console=console_table+nr_tty; */

    int v_mem_lines=V_MEM_SIZE/NR_BYTES_PER_LINE;//行数
 
    int console_v_mem_lines=(v_mem_lines/NR_CONSOLES);//占用行数
    p_console->original_addr=nr_tty*(console_v_mem_lines*NR_BYTES_PER_LINE);
    p_console->v_mem_limit=console_v_mem_lines*NR_BYTES_PER_LINE;
    p_console->current_start_addr=p_console->original_addr;

    p_console->cursor=p_console->original_addr;
    /* set_color(p_console,RED_COLOR,BLACK_COLOR); */
    disp_str("disp_str");
    
    out_char(p_console,nr_tty+'0');
    out_char(p_console,'#');
    printf("test");
}

/*
该函数引用了全局变量console_table,nr_current_cosole
*/
int is_current_console(CONSOLE *p_console){
    return (p_console==&console_table[nr_current_console]);
}

void set_color(CONSOLE *p_console,u8 fore_color,u8 back_color){
    p_console->color=((fore_color&FORE_COLOR_MASK)<<FORE_COLOR_SHIFT) |
        ((back_color&BACK_COLOR_MASK)<<FORE_COLOR_SHIFT);
}
void set_back_color(CONSOLE *p_console,u8 back_color){
    set_color(p_console,get_fore_color(p_console),back_color);
}
void set_fore_color(CONSOLE *p_console,u8 fore_color){
    set_color(p_console,fore_color,get_back_color(p_console));
}
u8 get_back_color(CONSOLE *p_console){
    return (p_console->color&BACK_COLOR_MASK)>>BACK_COLOR_SHIFT;
}
u8 get_fore_color(CONSOLE *p_console){
    return (p_console->color&FORE_COLOR_MASK)>>FORE_COLOR_SHIFT;
}



void out_char(CONSOLE *p_console,char ch){
    int position=p_console->cursor;
    switch(ch){
    case '\n':
        next_line(p_console);
        break;
    case '\b':
        backspace(p_console);
        break;
    default:
        if(position>=p_console->original_addr &&
           position<p_console->original_addr+p_console->v_mem_limit){
            position=V_MEM_BASE+position;
            *(u8*)position++=ch;
            *(u8*)position++=0x0F/* (BLACK_COLOR<<BACK_COLOR_SHIFT) | (RED_COLOR<<FORE_COLOR_SHIFT) */;
            
            //set_char((u8*)position,ch/* ,p_console->color */);

            if(position+NR_BYTES_PER_CHAR < p_console->original_addr+p_console->v_mem_limit){
                p_console->cursor=position+NR_BYTES_PER_CHAR;
                while(p_console->cursor >= p_console->current_start_addr
                   +NR_BYTES_PER_SCREEN){
                    p_console->current_start_addr+=NR_BYTES_PER_LINE;
                }
            }
        }
        break;
    }
    flush();
}

void select_console(int nr_console){
    if(nr_console<0 || nr_console>=NR_CONSOLES){
        return;
    }
    nr_current_console=nr_console;

    flush();
}

//addr是相对地址，而不是绝对地址。（字为单位，即2字节）
void set_video_start_addr(u32 addr){
        disable_int();
        out_byte(CRTC_ADDR_REG,START_ADDR_H);
        out_byte(CRTC_DATA_REG,(addr/2>>8)&0xFF);
        out_byte(CRTC_ADDR_REG,START_ADDR_L);
        out_byte(CRTC_DATA_REG,(addr/2)&0xFF);
        enable_int();    
}

//cursor是相对地址，而不是绝对地址。（字节为单位）
void set_cursor(u32 cursor){
    disable_int();
    out_byte(CRTC_ADDR_REG,CURSOR_H);
    out_byte(CRTC_DATA_REG,((cursor/2)>>8)&0xFF);
    out_byte(CRTC_ADDR_REG,CURSOR_L);
    out_byte(CRTC_DATA_REG,(cursor/2)&0xFF);
    enable_int();
}

//console1，console2有问题
void scroll_screen(int row){
    int original=console_table[nr_current_console].original_addr;
    int start_addr=console_table[nr_current_console].current_start_addr;
    int current_cursor=console_table[nr_current_console].cursor;
    int limit_addr=console_table[nr_current_console].v_mem_limit;

    int tmp_start_addr=start_addr+NR_BYTES_PER_LINE*row;
    if(tmp_start_addr >= original &&
       tmp_start_addr <= original+limit_addr-NR_BYTES_PER_SCREEN)
    {
        console_table[nr_current_console].current_start_addr=tmp_start_addr;
        if(current_cursor<tmp_start_addr){
            console_table[nr_current_console].cursor=tmp_start_addr;
        }
        else if(current_cursor>=tmp_start_addr+NR_BYTES_PER_SCREEN){
            console_table[nr_current_console].cursor=tmp_start_addr+NR_BYTES_PER_SCREEN-NR_BYTES_PER_LINE;
        }
        flush();
    }
}

void scroll_screen_down(int row){
    scroll_screen(row);
}

void scroll_screen_up(int row){
    scroll_screen(-1*row);
}

//console2,console2开始有问题
//开始求current_cursor方法：
//     current_cursor=(current_cursor/80+1)*80
//现在问题已经解决
//问题原因为：consolei的original_addr不一定是80的倍数。如果original_addr=243，而current_cursor=350,即在该console的第二行，其下一行行首的位置为（第三行）：243+80*2=403.而如果按之前的方法求解的结果为：400
void next_line(CONSOLE *p_console){
    int original_addr=p_console->original_addr;
    int current_start=p_console->current_start_addr;
    int limit=p_console->v_mem_limit;
    int current_cursor=p_console->cursor;
    int current_line=(current_cursor-original_addr)/NR_BYTES_PER_LINE;
    current_cursor=(current_line+1)*NR_BYTES_PER_LINE+original_addr;
    if(current_cursor>= original_addr+limit){
        return;
    }
    if(current_cursor>=current_start+NR_BYTES_PER_SCREEN && 
       current_start+NR_BYTES_PER_SCREEN<original_addr+limit){
        console_table[nr_current_console].current_start_addr=
            current_start+NR_BYTES_PER_LINE;
    }
    p_console->cursor=current_cursor;
    flush();
} 

void backspace(CONSOLE *p_console){
    int position=p_console->cursor;
    position=position-NR_BYTES_PER_CHAR;
    if(position>=p_console->original_addr &&
       position<p_console->original_addr+p_console->v_mem_limit){
        if(position<p_console->current_start_addr){
            p_console->current_start_addr-=NR_BYTES_PER_LINE;
        }
        set_char((u8*)position,' '/* ,p_console->color */);
        
        p_console->cursor=position;
    }
}

void flush(){
    set_cursor(console_table[nr_current_console].cursor);
    set_video_start_addr(console_table[nr_current_console].current_start_addr);
}

/*
直接向显存中写入字符
*/
static void set_char(u8 *position,char ch/* ,u8 color */){
    position=(u8*)(V_MEM_BASE+position);
    *position++=ch;
    *position++=(BLACK_COLOR<<BACK_COLOR_SHIFT) | (RED_COLOR<<FORE_COLOR_SHIFT);
}
