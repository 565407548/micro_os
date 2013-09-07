#include "protect_mode_const.h"

typedef struct s_descriptor{
    u16 limit_low;
    u16 base_low;
    u8 base_mid;
    u8 attr1;
    u8 limit_high_and_attr2;
    u8 base_high;
}DESCRIPTOR;

typedef struct s_gate{
    u16 offset_low;
    u16 selector;
    u8 dcount;
    u8 attr;
    u16 offset_high;
}GATE;

typedef struct s_tss{
    u32 backlink;
    u32 esp0;
    u32 ss0;
    u32 esp1;
    u32 ss1;
    u32 esp2;
    u32 ss2;
    u32 cr3;
    u32 eip;
    u32 flags;
    u32 eax;
    u32 ecx;
    u32 edx;
    u32 ebx;
    u32 esp;
    u32 ebp;
    u32 esi;
    u32 edi;
    u32 es;
    u32 cs;
    u32 ss;
    u32 ds;
    u32 fs;
    u32 gs;
    u32 ldt;
    u16 trap;
    u16 iobase;
}TSS;


#define vir2phys(seg_base,vir) (u32)(((u32)(seg_base))+(u32)(vir))

/*kernel/protect_mode.c*/
void init_protect_mode();
u32 seg2phys(u16 seg);
/**/
