//#define DEBUG
//#define DEBUG_PROCESS
//#define DEBUG_HD
//#define DEBUG_FS
//#define DEBUG_MESSAGE
//#define DEBUG_TTY
//#define DEBUG_MAIN

//BOOL值
#define FALSE 0
#define TRUE (!FALSE)

//空指针
#define NULL 0

#define EXIT_SUCCESS 1
#define EXIT_FAIL 0

#define BITS_PER_BYTE 8

#define SECTOR_SIZE 512
#define SECTOR_SIZE_SHIFT 9

#define BITS_PER_SECTOR (BITS_PER_BYTE*SECTOR_SIZE)

#define GDT_SIZE 128
#define IDT_SIZE 256

#define PRIVILIGE_KERNEL 0
#define PRIVILIGE_TASK 1
#define PRIVILIGE_USER 3

#define RPL_KERNEL SA_RPL0
#define RPL_TASK SA_RPL1
#define RPL_USER SA_RPL3


#define INT_M_CTL 0x20
#define INT_M_CTLMASK 0x21
#define INT_S_CTL 0xA0
#define INT_S_CTLMASK 0xA1

#define NR_IRQ 16

#define CLOCK_IRQ 0
#define KEYBOARD_IRQ 1
#define CASCADE_IRQ 2
#define FLOPPY_IRQ 6
#define MOUSE_IRQ 12
#define EXCEPTION_IRQ 13
#define AT_WINI_IRQ 14

#define TIMER0 0x40
#define TIMER_MODE 0x43
#define RATE_GENERATOR 0x34

#define TIMER_FREQ 1193182L/*输入频率，即1s产生TIMER_FREQ个信号*/
#define HZ 100/*输出频率，即1s有HZ个时钟中断发生(全局变量ticks的数值)*/

#define NR_CONSOLES 3


/*特殊字符，用于标记输出字符串是assert、panic还是normal*/
#define MAG_CH_ASSERT 'a' 
#define MAG_CH_PANIC 'p'
#define MAG_CH_NORMAL 'n'

/*assert宏*/
#define ASSERT
#ifdef ASSERT
void assertion_failure(char *con,char *exp,char *file,char *base_file,int line);
#define assert(con,exp) {                          \
        if(!(con))\
            assertion_failure(#con,exp,__FILE__,__BASE_FILE__,__LINE__); \
}
#else
#define assert(con,exp)
#endif

#define ARRAY_LENGTH(x) (sizeof(x)/sizeof(x[0]))
#define min(x,y) ((x<y)?(x):(y))
#define max(x,y) ((x>y)?(x):(y))
