#define MESSAGE_BUFFER_SIZE 512
/*信息接收，信息发送进程的PID，以下是特殊PID*/
#define ANY -1
#define NO_TASK -2
#define INTERRUPT -3
/*
//重构时，可以用下面三个宏替换上面三个宏
#define PID_ANY -1
#define PID_NO -2
#define PID_INTERRUPT -3
*/

/*信息发送，接收动作*/
#define SEND 1
#define RECEIVE 2
#define BOTH 4

/*消息类型*/
/*起始消息*/
#define INFO_START 1
/*中断消息*/
#define INFO_INT_START (INFO_START)/*1*/
#define INFO_INT_HD (INFO_INT_START)
#define INFO_INT_TICK (INFO_INT_HD+1)
#define INFO_INT_KEYPRESSED (INFO_INT_TICK+1)
#define INFO_INT_END (INFO_INT_KEYPRESSED)/*2*/
/*文件操作消息*/
#define INFO_FS_START (INFO_INT_END+1)/*3*/
#define INFO_FS_CREATE (INFO_FS_START)
#define INFO_FS_UNLINK (INFO_FS_CREATE+1)
#define INFO_FS_OPEN (INFO_FS_UNLINK+1)/*5*/
#define INFO_FS_CLOSE  (INFO_FS_OPEN+1)
#define INFO_FS_READ (INFO_FS_CLOSE+1)
#define INFO_FS_WRITE (INFO_FS_READ+1)
#define INFO_FS_IOCTL (INFO_FS_WRITE+1)
#define INFO_FS_DEVICE (INFO_FS_IOCTL+1)
#define INFO_FS_SEEK (INFO_FS_DEVICE+1)
#define INFO_FS_LS (INFO_FS_SEEK+1) /*获得当前目录下的所有项*/
#define INFO_FS_END (INFO_FS_LS)/*6*/
/*其他消息*/
#define INFO_OTHER_START (INFO_FS_END+1)/*7*/
#define INFO_OTHER_TICKS (INFO_OTHER_START)
#define INFO_SUSPEND_PROCESS (INFO_OTHER_TICKS+1)
#define INFO_SYSCALL_RET (INFO_SUSPEND_PROCESS+1)
#define INFO_OTHER_END (INFO_SYSCALL_RET)/*7*/
/*结束消息*/
#define INFO_END INFO_OTHER_END

/*进程标志，正在接受消息还是正在发送消息*/
#define NORMAL 0x0/*正常运行标志*/
#define SENDING 0x01/*阻塞，发送消息标志*/
#define RECEIVING 0x02/*阻塞，接收消息标志*/
#define NOACTION 0x04

#define DEVICE_IOCTL_GET 0x0
#define DIOCTL_GET_PART_INFO 0x01

#define REQUEST 0x0
