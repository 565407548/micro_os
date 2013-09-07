#Makefile for Zheng's OS

#Entry point of Zheng's
#It must have the same value with 'KernelEntryPointPhyAddr' in load.inc
ENTRYPOINT=0x30400 #等号两边可以有或者没有空格
ENTRYOFFSET=0x400
FD=a.img

#Programs, flags, etc
#如果一行显示不下，可以用‘\’作为行扩展
ASM=nasm
DASM=objdump
CC=gcc
LD=ld
ASMBFLAGS=-I boot/include/
ASMELFFLAGS=-I include/ -f elf
#-s:strip all symbol
LDFLAGS= -Ttext $(ENTRYPOINT) -m elf_i386 #-m elf_i386指明把代码链接成i386架构的代码
CFLAGS=-I include/ -c -O -g -fomit-frame-pointer -fno-stack-protector -fno-builtin -m32 -Wall

BINS=bin/boot.bin bin/loader.bin bin/kernel.bin
OBJS=obj/kernel/kernel.o obj/lib/kliba.o obj/lib/klib.o obj/kernel/string.o obj/kernel/i8259.o obj/kernel/start.o obj/kernel/protect_mode.o obj/kernel/main.o obj/kernel/global.o obj/kernel/clock.o obj/kernel/keyboard.o obj/kernel/syscall.o obj/kernel/process.o obj/kernel/message.o obj/kernel/tty.o obj/kernel/console.o obj/kernel/printf.o obj/kernel/vsprintf.o obj/kernel/stdio.o obj/kernel/systask.o obj/kernel/hd.o obj/kernel/fs_accessor.o obj/kernel/fs.o obj/kernel/open.o obj/kernel/error.o obj/lib/misc.o

#LOBJS=

#This Program
#ZHENGSBOOT=bin/boot.bin bin/loader.bin

#All Phony Targets
.PHONY:everything final image clean realclean all building

#Default starting position
nop:
	@echo "why not \'make image' huh?:)"

everything:$(BINS) $(OBJS)

all:realclean everything

image:realclean everything building

#clean:
#	rm -f $(OBJS) $(LOBJS)

realclean:
	rm -f  $(BINS) $(OBJS)

#disasm:
#	$(DASM) $(DASMFLAGS) $(ZHENGSKERNEL) > $(DASMOUTPUT)

building:
	dd if=bin/boot.bin of=$(FD) bs=512 count=1 conv=notrunc
	sudo mount -o loop $(FD) /mnt/floppy/
	sudo cp -fv bin/loader.bin /mnt/floppy
	sudo cp -fv bin/kernel.bin /mnt/floppy
	sleep 1
	sudo umount /mnt/floppy

bin/boot.bin : boot/boot.asm boot/include/load.inc boot/include/fat12hdr.inc
	$(ASM) $(ASMBFLAGS) -o $@ $<

bin/loader.bin : boot/loader.asm boot/include/load.inc boot/include/fat12hdr.inc boot/include/pm.inc
	$(ASM) $(ASMBFLAGS) -o $@ $<

bin/kernel.bin : $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $(OBJS)

obj/kernel/kernel.o : kernel/kernel.asm include/sconst.inc
	$(ASM) $(ASMELFFLAGS) -o $@ $<

obj/kernel/start.o : kernel/start.c  
	$(CC) $(CFLAGS) -o $@ $<

obj/kernel/protect_mode.o:kernel/protect_mode.c 
	$(CC) $(CFLAGS) -o $@ $<

obj/kernel/i8259.o : kernel/i8259.c 
	$(CC) $(CFLAGS) -o $@ $<

obj/kernel/main.o : kernel/main.c 
	$(CC) $(CFLAGS) -o $@ $<

obj/kernel/global.o : kernel/global.c 
	$(CC) $(CFLAGS) -o $@ $<

obj/kernel/clock.o : kernel/clock.c
	$(CC) $(CFLAGS) -o $@ $<

obj/kernel/keyboard.o : kernel/keyboard.c 
	$(CC) $(CFLAGS) -o $@ $<

obj/kernel/process.o : kernel/process.c 
	$(CC) $(CFLAGS) -o $@ $<

obj/kernel/message.o : kernel/message.c
	$(CC) $(CFLAGS) -o $@ $<

obj/kernel/tty.o : kernel/tty.c
	$(CC) $(CFLAGS) -o $@ $<

obj/kernel/console.o : kernel/console.c
	$(CC) $(CFLAGS) -o $@ $<

obj/kernel/vsprintf.o : kernel/vsprintf.c
	$(CC) $(CFLAGS) -o $@ $< 

obj/kernel/printf.o : kernel/printf.c
	$(CC) $(CFLAGS) -o $@ $<

obj/kernel/stdio.o : kernel/stdio.c
	$(CC) $(CFLAGS) -o $@ $<

obj/kernel/systask.o : kernel/systask.c 
	$(CC) $(CFLAGS) -o $@ $<

obj/kernel/error.o : kernel/error.c
	$(CC) $(CFLAGS) -o $@ $<

obj/kernel/hd.o : fs/hd.c
	$(CC) $(CFLAGS) -o $@ $<

obj/kernel/fs.o : fs/fs.c
	$(CC) $(CFLAGS) -o $@ $<

obj/kernel/fs_accessor.o : fs/fs_accessor.c
	$(CC) $(CFLAGS) -o $@ $<

obj/kernel/open.o : fs/open.c
	$(CC) $(CFLAGS) -o $@ $<

obj/lib/misc.o : lib/misc.c
	$(CC) $(CFLAGS) -o $@ $<

# obj/lib/string.o : lib/string.asm
# 	$(ASM) $(ASMELFFLAGS) -o $@ $<

obj/kernel/string.o : kernel/string.c
	$(CC) $(CFLAGS) -o $@ $<

obj/kernel/syscall.o : kernel/syscall.asm include/sconst.inc
	$(ASM) $(ASMELFFLAGS) -o $@ $<

obj/lib/kliba.o : lib/kliba.asm include/sconst.inc
	$(ASM) $(ASMELFFLAGS) -o $@ $<

obj/lib/klib.o : lib/klib.c include/type.h
	$(CC) $(CFLAGS) -o $@ $<

