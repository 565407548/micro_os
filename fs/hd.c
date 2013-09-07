#include "stdlib.h"

#include "include/fs.h"
#include "include/hd.h"

#include "message.h"
#include "driver.h"
#include "i8259a.h"
#include "protect_mode.h"
#include "process.h"

#include "syscall.h"
#include "misc.h"
#include "kliba.h"
#include "clock.h"
#include "error.h"
#include "string.h"
#include "stdio.h"

static char hdbuf[SECTOR_SIZE];
static struct s_hd_info hd_info[1];

static void init_hd();
static void hd_handler(int irq);

static void hd_device_info(MESSAGE *message);
static void hd_open(MESSAGE *message);
static void hd_close(MESSAGE *message);
static void hd_rdwt(MESSAGE *message);
static void hd_ioctl(MESSAGE *message);

static void get_part_table(int index,int sector_index,struct s_hd_part_entry *entry,int entry_count);
static void partition(int index);
static HD_PART_INFO* get_part_info(HD_INFO *hdi,int part_index);
/* static void set_part_info(HD_INFO *hdi, int part_index,HD_PART_INFO *hd_part_info); */
/* static void print_hdinfo(struct s_hd_info *hdi); */

static void hd_identify(int driver);

static void set_command(HD_COMMAND *cmd,u8 hd_index,u8 data,u8 features,int sector_count,int sector_index,u8 command);
static void hd_command_out(struct s_hd_command *cmd);
static void interrupt_wait();
static int wait_for(int mask,int val,int timeout);

static void print_identify_info(u16*  hdinfo);

/* static void hd_rdwt_test(); */
/*
功能：
    处理硬盘相关请求的任务进程。初始化后，永久运行接收有关硬盘请求的信息
参数：
    （无）
返回值：
    （无）
*/
void task_hd(){
    MESSAGE message;
#ifdef DEBUG_HD
    printl("in task_hd\n");
#endif 
   //初始化硬盘
    init_hd();

    while(1){
        reset_message(&message);
        send_receive(RECEIVE,ANY,&message);
        
        int src=message.source_pid;
        switch(message.type){
        case INFO_FS_OPEN:
            hd_open(&message);
            break;
        case INFO_FS_CLOSE:
            hd_close(&message);
            break;
        case INFO_FS_READ:
        case INFO_FS_WRITE:
            hd_rdwt(&message);
            break;
        case INFO_FS_IOCTL:
            hd_ioctl(&message);
            break;
        case INFO_FS_DEVICE:
            hd_device_info(&message);
            break;
        default:
            //panic("unkonw message type(%d) in task_hd",src);
            break;
        }
        
#ifdef DEBUG_HD
        printl("in task_hd\n");
#endif
        /* message.source_pid=process2pid(p_process_ready); */
        send_receive(SEND,src,&message);
    }
}

/*<ring 1>
功能：
    初始化硬盘，其中包括显示主机硬盘数、启动硬盘驱动程序及初始化hd_info
参数：
    （无）
返回值：
    （无）
*/
static void init_hd(){
    int i;
    u8 *p_drivers_count=(u8*)(0x475);
#ifdef DEBUG_HD
    printl("Drivers_count:%d.\n",*p_drivers_count);
#endif
    assert(*p_drivers_count,"");

    /*启动硬盘中断，由于硬盘中断接在级联从片上，所以需要同时启动主片上的对应驱动*/
    put_irq_handler(AT_WINI_IRQ,hd_handler);
    enable_irq(CASCADE_IRQ);
    enable_irq(AT_WINI_IRQ);
    
    /*hd_info初始化*/
    for(i=0;i<ARRAY_LENGTH(hd_info);i++){
        memset(&hd_info[i],0,sizeof(hd_info[0]));
    }
    hd_info[0].open_count=0;
}

/*
功能：
    硬盘打开命令
参数：
    index：完整的设备编号，由设备驱动程序编号和设备编号构成，具体含义参考fs.h,hd.h
返回值：
    （无）
*/
static void hd_device_info(MESSAGE *message){
    int index=message->device;
    int hd_index=HD_INDEX(index);
    assert(hd_index==0,"");

    hd_identify(hd_index);/*驱动器号，目前只可能是0*/

    if((hd_info[hd_index].open_count++)==0){
        partition(index);
#ifdef DEBUG
        print_hdinfo(&hd_info[hd_index]);
#endif
    }
}

static void hd_open(MESSAGE *message){
    
}

static void hd_close(MESSAGE *message){
    int hd_index=HD_INDEX(message->device);
    assert(hd_index==0,"");
    assert(hd_info[hd_index].open_count>0,"");

    hd_info[hd_index].open_count--;
}

/*
1.函数内部需要注意：while(1),while(-1)都会执行循环体，只有while(0)时才跳出循环体
2.目前只能处理读写数据长度为SECTOR_SIZE整数倍的请求，否则会出错
*/
static void hd_rdwt(MESSAGE *message){
    int hd_index=HD_INDEX(message->device);
    int hd_part_index=HD_PART_INDEX(message->device);

    u64 position=message->position;
#ifdef DEBUG_HD
    printl("position=%d\n",position);
#endif
    assert((position>>SECTOR_SIZE_SHIFT)<(1<<31),"");
    assert(position%SECTOR_SIZE==0,"hd readwrite start position must be the multiple of SECTOR_SIZE");//读取的起始位置必须在扇区的起始位置

    HD_COMMAND hd_command;
    u32 base=get_part_info(&hd_info[hd_index],hd_part_index)->base;
    u32 sector_index=base+(u32)(position>>SECTOR_SIZE_SHIFT);
    int sector_length=(message->length+SECTOR_SIZE-1)/SECTOR_SIZE;
    int cmd=(message->type==INFO_FS_READ)?ATA_READ:ATA_WRITE;
    set_command(&hd_command,hd_index,0,0,sector_length,sector_index,cmd);
    hd_command_out(&hd_command);

    int bytes_left=message->length;
#ifdef DEBUG_HD
    printl("sector_index=%d sector_length=%d bytes_left=%d\n",sector_index,sector_length,bytes_left);
#endif
    void *la=(void *)va2la(message->source_pid,message->arg_pointer);
#ifdef DEBUG_HD
    printl("source_pid=%d la[0]=%d\n",message->source_pid,*(int*)la);
#endif
//此处需要注意：while(1),while(-1)都会执行循环体，只有while(0)时才跳出循环体
    while(bytes_left>0){
        int bytes=min(SECTOR_SIZE,bytes_left);
        if(message->type==INFO_FS_READ){
            interrupt_wait();
            port_read(REG_P_DATA,hdbuf,SECTOR_SIZE);
            memcpy(la,(void *)va2la(TASK_HD,hdbuf),bytes);
        }else{
            if(!wait_for(STATUS_DRQ_MASK,STATUS_DRQ,HD_TIME_OUT))
                panic("hd writing error!\n");
            /* printl("bytes=%d",bytes); */
            port_write(REG_P_DATA,la,bytes);
            /* printl("test1"); */
            interrupt_wait();
            /* printl("test2"); */
        }
        bytes_left-=bytes;
        la+=bytes;
        /* printl("(bytes_left>0)=%d\n",bytes_left>0); */
    }
#ifdef DEBUG_HD
    printl("hd_rdwt ok!\n");
#endif
}
static void hd_ioctl(MESSAGE *message){
    int hd_index=HD_INDEX(message->device);
    int hd_part_index=HD_PART_INDEX(message->device);

    struct s_hd_info *hdi=&hd_info[hd_index];

#ifdef DEBUG_HD
    printl("hd_index=%d hd_part_index=%d\n",hd_index,hd_part_index);
#endif

    if(message->subtype==DIOCTL_GET_PART_INFO){
        void *dest=va2la(message->source_pid,message->arg_pointer);
        /* void *src=va2la(TASK_HD, */
        /*     hd_part_index<=HD_PART_PRIM_COUNT? */
        /*                 &hdi->primary[hd_part_index]: */
        /*                 &hdi->logical[hd_part_index-HD_PART_LOGICAL_MIN]); */
        void *src=va2la(TASK_HD,get_part_info(&hdi[hd_index],hd_part_index));
        
        memcpy(dest,src,sizeof(struct s_hd_part_info));
    }else{
        assert(FALSE,"unknown message subtype in hd_ioctl");
    }
}

/*<ring 0>
功能：
    硬盘驱动程序。每次发生硬盘中断都通知TASK_HD进程
参数：
    （无）
返回值：
    （无）
*/
static void hd_handler(int irq){
    /* u8 hd_status=in_byte(REG_P_STATUS); */
    in_byte(REG_P_STATUS);

    inform_int(TASK_HD,INFO_INT_HD);
}

static void set_command(HD_COMMAND *cmd,u8 hd_index,u8 data,u8 features,int sector_count,int sector_index,u8 command){
#ifdef DEBUG_HD
    printl("secotr_count=%d sector_index=%d\n",sector_count,sector_index);
#endif
    cmd->data=data;
    cmd->features=features;
    cmd->count=sector_count;    
    cmd->lba_low=sector_index&0xFF;
    cmd->lba_mid=(sector_index>>8)&0xFF;
    cmd->lba_high=(sector_index>>16)&0xFF;
    cmd->device=MAKE_DEVICE_REG(1,hd_index,(sector_index>>24)&0xF);
    cmd->command=command;
}

/*
功能：
    获得一个分区内的完整分区表，即包含四项分区表信息    
参数：
    entry：
返回值：
    （无）
*/
static void get_part_table(int index,int sector_index,struct s_hd_part_entry *entry,int entry_count){
    int hd_index=HD_INDEX(DEVICE(index));
    struct s_hd_command cmd;

    set_command(&cmd,hd_index,0,0,1,sector_index,ATA_READ);

#ifdef DEBUG
    printl("feathures=%d   count=%d   lba_low=%d lba_mid=%d lba_high=%d device=%d command=%d\n",cmd.features,cmd.count,cmd.lba_low,cmd.lba_mid,cmd.lba_high,cmd.device,cmd.command);
#endif

    hd_command_out(&cmd);//命令发出
    interrupt_wait();//等待硬盘中断

    port_read(REG_P_DATA,hdbuf,SECTOR_SIZE);

    memcpy(entry,
           hdbuf+PARTITION_TABLE_OFFSET,
           sizeof(struct s_hd_part_entry)*entry_count);
}

/*
功能：
    通过系统设备编号和分区类型获得该设备的所有分区信息
参数：
    device：系统设备编号
    type：分区类型（主分区、扩展分区）
返回值：
    （无）

*/
static void partition(int index){
    /* int driver=DRIVER(device); */
    /* int sdevice=SDEVICE_HD(device); */
    /* int prim=PRIM_HD(device); */
    /* int logical=LOGICAL_HD(device); */
    
    /* int devie=DEVICE(index); */
    int hd_index=HD_INDEX(index);
    /* int hd_part_index=HD_PART_INDEX(index); */

    struct s_hd_info *hdi=&hd_info[hd_index];

    /*一个硬盘设备所有分区表项信息*/
    struct s_hd_part_entry prim_part_table[HD_ENT_PER_PRIM_PART];
    struct s_hd_part_entry logical_part_table[HD_ENT_PER_LOGICAL_PART];

    //获得该硬盘分区表信息
    get_part_table(index,HD_PRIM_TABLE_SECTOR,prim_part_table,HD_ENT_PER_PRIM_PART);
    /* get_part_table(index,20001,prim_part_table,HD_ENT_PER_PRIM_PART); */

    int prim_index=0;//用于遍历所有主分区
    int logical_index=0;//用于遍历所有逻辑分区
    int extended_index=0;//记录主分区中对应扩展分区的序号
    /*遍历所有主分区*/
    for(prim_index=1;prim_index<=HD_PART_PRIM_COUNT;prim_index++){
        int prim_table_index=prim_index-1;
        if(prim_part_table[prim_table_index].part_type==NO_PART)
            continue;

        hdi->primary[prim_index].base=prim_part_table[prim_table_index].begin_sector_lba;
        hdi->primary[prim_index].size=prim_part_table[prim_table_index].sectors_count;

#ifdef DEBUG
        printl("prim %d:base=%d length=%d part_type=%d\n",prim_index,hdi->primary[prim_index].base,hdi->primary[prim_index].size,prim_part_table[prim_table_index].part_type);
#endif

        if(prim_part_table[prim_table_index].part_type==EXTENDED_PART){
            assert(extended_index==0,"only one extended part can exist!");
            int extended_index=prim_index;

            int logical_begin_sector=hdi->primary[extended_index].base;
            for(logical_index=0;logical_index<HD_PART_LOGICAL_COUNT;logical_index++){
                get_part_table(index,logical_begin_sector,logical_part_table,HD_ENT_PER_LOGICAL_PART);

                hdi->logical[logical_index].base=logical_begin_sector+logical_part_table[0].begin_sector_lba;
                hdi->logical[logical_index].size=logical_part_table[0].sectors_count;

#ifdef DEBUG
                printl("logical %d: base=%d length=%d part_type=%d \nnext_part_type=%d\n",logical_index,hdi->logical[logical_index].base,logical_index,hdi->logical[logical_index].size,logical_part_table[0],logical_part_table[1].part_type);
#endif
                
                if(logical_part_table[1].part_type==NO_PART){
                    break;
                }
                else{
                    logical_begin_sector=hdi->primary[extended_index].base+logical_part_table[1].begin_sector_lba;
                }
            }
        }
    }
}

static HD_PART_INFO* get_part_info(HD_INFO *hdi,int part_index){
    if(HD_PART_PRIM_MIN<=part_index && part_index<=HD_PART_PRIM_MAX){
        return &hdi->primary[part_index];
    }else if(HD_PART_LOGICAL_MIN<=part_index && part_index<=HD_PART_LOGICAL_MAX){
        return &hdi->logical[part_index-HD_PART_LOGICAL_MIN];
    }else{
#ifdef DEBUG_HD
        printl("part_index=%d\n",part_index);
#endif
        assert(0,"in get_part_info\n");
        return NULL;
    }
}

/* static void set_part_info(HD_INFO *hdi, int part_index,HD_PART_INFO *hd_part_info){ */
/*     HD_PART_INFO *dest_part_info=NULL; */

/*     if(HD_PART_PRIM_MIN<=part_index && part_index<=HD_PART_PRIM_MAX){ */
/*         dest_part_info=&hdi->primary[part_index]; */
/*     }else if(HD_PART_LOGICAL_MIN<=part_index && part_index<=HD_PART_LOGICAL_MAX){ */
/*         dest_part_info=&hdi->logical[part_index-HD_PART_LOGICAL_MIN]; */
/*     } */

/*     assert(dest_part_info!=NULL,"in set_part_info\n"); */
    
/*     memcpy(dest_part_info,hd_part_info,sizeof(HD_PART_INFO)); */
    
/* } */

/*
功能：
    打印硬盘各个分区的详细信息（基址和大小）
参数：
    hdi：硬盘信息结构体的指针
返回值：
    （无）
*/
/* static void print_hdinfo(struct s_hd_info *hdi){ */
/*     int prim_part_index,logical_part_index; */
        
/*     for(prim_part_index=0;prim_part_index<=HD_PART_PRIM_COUNT;prim_part_index++){ */
/*         if(hdi->primary[prim_part_index].size==0) */
/*         continue; */
    
/*         printl("%s%d: base:%d(0x%x), size:%d(0x%x) (in sector)\n", */
/*                prim_part_index==0?"  ":"      ", */
/*                prim_part_index, */
/*                hdi->primary[prim_part_index].base, */
/*                hdi->primary[prim_part_index].base, */
/*                hdi->primary[prim_part_index].size, */
/*                hdi->primary[prim_part_index].size); */
/*     } */
/*     for(logical_part_index=0;logical_part_index<HD_PART_LOGICAL_COUNT;logical_part_index++){ */
/*         if(hdi->logical[logical_part_index].size==0) */
/*             continue; */
/*         printl("            %d: base:%d(0x%x), size:%d(0x%x) (in sector)\n", */
/*                logical_part_index, */
/*                hdi->logical[logical_part_index].base, */
/*                hdi->logical[logical_part_index].base, */
/*                hdi->logical[logical_part_index].size, */
/*                hdi->logical[logical_part_index].size); */
/*     } */
/* } */

/*
功能：
    获取硬盘信息。主要是获取对应硬盘的容量，获取的信息保存在hd_info中
参数：
    device：系统统一设备编号
返回值：
    （无）
*/
static void hd_identify(int index){
    int hd_index=HD_INDEX(index);

    struct s_hd_command cmd;
    cmd.device=MAKE_DEVICE_REG(0,hd_index,0);
    cmd.command=ATA_IDENTIFY;
    hd_command_out(&cmd);

#ifdef DEBUG_HD
    printl("interrupt_wait\n");
#endif

    interrupt_wait();

    port_read(REG_P_DATA,hdbuf,SECTOR_SIZE);

#ifdef DEBUG_HD
    print_identify_info((u16*)hdbuf); 
#endif
    
    u16 *hdinfo=(u16*)hdbuf;
    //硬盘基址 
    hd_info[hd_index].primary[0].base=0;
    //硬盘扇区数
    hd_info[hd_index].primary[0].size=((int)hdinfo[61]<<16)+hdinfo[60];
}

static void print_identify_info(u16*  hdinfo){
    int sectors=((int)hdinfo[61]<<16)+hdinfo[60];
    printl("HD Size:%d MB\n",sectors*512/1000000);
}

/*
功能：
    往硬盘控制器发送命令，操作硬盘
参数：
    cmd：命令
返回值：
    （无）

说明（P326）：
    硬盘操作并不复杂，具体有两步：
       1.往命令块寄存器写入正确的值
       2.往控制块寄存器中发送命令
*/
static void hd_command_out(struct s_hd_command *cmd){
    if(!wait_for(STATUS_BUSY_MASK,0,HD_TIME_OUT))
        panic("hd error!");

    /*往控制块寄存器写入正确的值*/
    out_byte(REG_P_DEVICE_CONTROL,0);
    
    /*往控制块寄存器中发送命令*/
    out_byte(REG_P_FEATURES,cmd->features);
    out_byte(REG_P_SECTOR_COUNT,cmd->count);
    out_byte(REG_P_LBA_LOW,cmd->lba_low);
    out_byte(REG_P_LBA_MID,cmd->lba_mid);
    out_byte(REG_P_LBA_HIGH,cmd->lba_high);
    out_byte(REG_P_DEVICE,cmd->device);

    out_byte(REG_P_COMMAND,cmd->command);
}

/*
功能：
    等待硬盘中断
参数：
    （无）
返回值：
    （无）
*/
static void interrupt_wait(){
    MESSAGE msg;
    msg.type=INFO_INT_HD;

#ifdef DEBUG_HD
    printl("interrupt_wait mst.type=%d\n",msg.type);
#endif

    send_receive(RECEIVE,INTERRUPT,&msg);
}

/*
功能：
     在一定时间内等待某一状态，如果等到，则返回TRUE；否则返回假FALSE.
参数：
    mask:获取对应状态的掩码
    val:想等待的状态值
    time_out:最长等待时间(ms)
返回值：
    
*/
static BOOL wait_for(int mask,int val,int time_out){
    int s=get_milli_seconds();

    while((get_milli_seconds()-s)<time_out){
        if((in_byte(REG_P_STATUS) & mask)==val){
            return TRUE;
        }
    }
    return FALSE;
}
