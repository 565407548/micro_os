#include "stdlib.h"

#include "message.h"
#include "protect_mode.h"
#include "../fs/include/fs.h"
#include "process.h"
#include "tty.h"
#include "console.h"
#include "driver.h"
#include "kliba.h"

#include "systask.h"
#include "main.h"
#include "../fs/include/hd.h"

#include "global.h"


char task_stack[STACK_SIZE_TOTAL];
irq_handler irq_table[NR_IRQ];
system_call sys_call_table[NR_SYS_CALL]={sys_send_receive/*,sys_write*/,sys_printx};

TTY tty_table[NR_CONSOLES];
TTY *TTY_FIRST=tty_table;
TTY *TTY_END=tty_table+NR_CONSOLES;
CONSOLE console_table[NR_CONSOLES];

PROCESS process_table[NR_TASKS+NR_PROCS];
TASK task_table[NR_TASKS]={
    {task_tty,STACK_SIZE_TTY,"tty"},
    {task_sys,STACK_SIZE_SYS,"sys"},
    {task_hd,STACK_SIZE_HD,"task_hd"},
    {task_fs,STACK_SIZE_FS,"task_fs"}
 };
TASK user_proc_table[NR_PROCS]={
    /* {Task_fs,STACK_SIZE_TASK_FS,"task_fs"}, */
    {TestA,STACK_SIZE_TESTA,"A"},
    {TestB,STACK_SIZE_TESTB,"B"},
    {TestC,STACK_SIZE_TESTC,"C"},
    {TestD,STACK_SIZE_TESTD,"D"}};

DRIVER_DEVICE_MAP dd_map[]={
    {PID_INVALID},/*unused*/
    {PID_INVALID},/*reserved for floppy*/
    {PID_INVALID},/*reserved for cdrom*/
    {TASK_HD},/*hard disk*/
    {TASK_TTY},/*tty*/
    {PID_INVALID}/*reserved for scsi disk*/
};
