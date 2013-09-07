//major device numbers
//DRIVER_[device name]
#define DRIVER_NO 0
#define DRIVER_FLOPPY 1
#define DRIVER_CDROM 2
#define DRIVER_HD 3
#define DRIVER_TTY 4
#define DRIVER_SCSI 5


typedef struct s_driver_device_map{
    int driver_pid;
}DRIVER_DEVICE_MAP;
