//
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long u64;
typedef int int32;
typedef char* va_list;
typedef unsigned int PID;
typedef int BOOL;

typedef void (*int_handler)();
typedef void (*task_f)();

typedef void (*irq_handler)(int irq);
typedef void *system_call;

/*自定义枚举*/
/*消息类型*/
typedef enum e_info_type{
    eit_invalid=0,
    eit_first=1,

    eit_hand_int=1,
    eit_get_ticks=2,
    eit_fs_create=3,
    eit_fs_delete=4,
    eit_fs_open=5,
    eit_fs_close=6,

    eit_last=6
}INFO_TYPE;

/*分区表类型*/
typedef enum e_part_type{
    ept_invalid=0,
    ept_first=1,

    ept_primary=1,
    ept_extended=2,

    ept_last=2
}PART_TYPE;

typedef struct s_dev_driver_map{
    int driver_nr;
}DEV_DRIVER_MAP;
