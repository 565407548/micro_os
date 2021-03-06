#include "message_const.h"

typedef enum e_return_type{
    t_char,
    t_int,
    t_point,
    t_float,
    t_double
}RETURN_TYPE,ARGUMENT_TYPE;

/*自定义结构体*/
typedef struct s_message{
    int source_pid;/*pid*/
    int process_index;
    int type;
    int subtype;
    int device;

    RETURN_TYPE return_type;
    //res:result
    union{
        char res_char;
        int res_int;
        BOOL res_bool;
        void *res_pointer;
        float res_float;
        double res_double;
    };
    //arg:argument
    ARGUMENT_TYPE argument_type;
    union{
        char arg_char;
        int arg_int;
        BOOL arg_bool;
        const void *arg_pointer;
        float arg_float;
        double arg_double;
    };
    int length;/*value_point表示缓冲区或其他相关内存指针，length表示对应长度*/
    int position;

    int fd;
    int flags;/*文件操作(读写)*/
    int mode;/*文件操作（不同用户不同权限）*/
    int offset;
    int whence;
}MESSAGE;


void reset_message(MESSAGE *p_message);
void inform_int(int receiver,int msgtype);
int send_receive(int function,int src_dest,MESSAGE *p_message);
