#include "stdlib.h"

#include "message.h"
#include "protect_mode.h"
#include "../fs/include/fs.h"
#include "process.h"
#include "tty.h"
#include "console.h"
#include "driver.h"
#include "kliba.h"
#include "global.h"

#include "string.h"

void cstart(){
    /* disp_str("cstart---begin\n"); */

    memcpy(&gdt,(void *)(*((u32*)(&gdt_ptr[2]))),*((u16*)(&gdt_ptr[0]))+1);

    u16* p_gdt_limit=(u16 *)(&gdt_ptr[0]);
    u32* p_gdt_base=(u32 *)(&gdt_ptr[2]);

    *p_gdt_limit=GDT_SIZE*sizeof(DESCRIPTOR)-1;
    *p_gdt_base=(u32)&gdt;

    u16* p_idt_limit=(u16*)(&idt_ptr[0]);
    u32* p_idt_base=(u32*)(&idt_ptr[2]);
    *p_idt_limit=IDT_SIZE*sizeof(GATE)-1;
    *p_idt_base=(u32)&idt;

    init_protect_mode();

    disp_str("cstart---end");

    display_position=0;
    int i=0;
    for(i=0;i<80*20;i++){
        disp_str(" ");
    }
    display_position=0;
}
