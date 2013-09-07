/*任务进程，level_1*/
#define NR_TASKS 4
#define STACK_SIZE_TTY 0x8000
#define STACK_SIZE_SYS 0x8000
#define STACK_SIZE_HD 0x8000
#define STACK_SIZE_FS 0x8000

#define TASK_START 0
#define TASK_TTY (TASK_START)
#define TASK_SYS (TASK_TTY+1)
#define TASK_HD (TASK_SYS+1)
#define TASK_FS (TASK_HD+1)
#define PID_INVALID (-1)

/*用户进程，level_3*/
#define NR_PROCS 4
//#define STACK_SIZE_TASK_FS 0x8000
#define STACK_SIZE_TESTA 0x8000
#define STACK_SIZE_TESTB 0x8000
#define STACK_SIZE_TESTC 0x8000
#define STACK_SIZE_TESTD 0x8000

#define PROCS_START (TASK_START+NR_TASKS)
//#define TASK_FS (PROCS_START)
#define TESTA (PROCS_START)
#define TESTB (TESTA+1)
#define TESTC (TESTB+1)
#define TESTD (TESTC+1)

/* #define STACK_SIZE_TOTAL (STACK_SIZE_TTY +      \ */
/*                           STACK_SIZE_SYS +      \ */
/*                           STACK_SIZE_TESTA +    \ */
/*                           STACK_SIZE_TESTB +    \ */
/*                           STACK_SIZE_TESTC +    \ */
/*                           STACK_SIZE_TESTD      \ */
/*         ) */

#define STACK_SIZE_TOTAL (STACK_SIZE_TTY +      \
                          STACK_SIZE_SYS +      \
                          STACK_SIZE_HD +       \
                          STACK_SIZE_FS +  \
                          STACK_SIZE_TESTA +    \
                          STACK_SIZE_TESTB +    \
                          STACK_SIZE_TESTC +    \
                          STACK_SIZE_TESTD      \
        )
