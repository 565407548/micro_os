#define IO_BUFFER_SIZE 512
#define STDIN 0
#define STDOUT 1
#define STDERROR 2


//kernel/printf.c
int printf(const char *fmt,...);
int sprintf(char *str, const char *format, ...);
int printl(const char *fmt,...);

//kernel/vsprintf.c
int vsprintf(char *buf,const char *fmt,va_list args);

//kernel/stdio.h
void disp_int(int input);
