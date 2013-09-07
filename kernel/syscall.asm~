%include "sconst.inc"

        ;; _NR_get_ticks equ 0
        _NR_send_receive equ 0
        ;; _NR_write equ 1
        _NR_printx equ 1
        INT_VECTOR_SYS_CALL equ 0x90

        ;; global get_ticks
        global sendreceive
        global write
        global printx

        extern disp_int
bits 32
[section .text]
        
;;; 说明
;;; 1.参考中断向量表的初始化（protect_mode中的init_protect_mode()函数）
;;; 2.参考kernel.asm中的sys_call函数
;;; 3.参考global.c中的sys_call_table数组
        
;;; int get_ticks();
;; get_ticks:
;;         mov eax,_NR_get_ticks
;;         int INT_VECTOR_SYS_CALL
;;         ret

;;; void sendreceive(int function,int src_dest,MESSAGE *p_m)
sendreceive:
        push ebp
        mov ebp,esp
        
        push ebx
        push ecx
        push edx
        
        mov eax,_NR_send_receive
        mov ebx,[ebp+8]
        mov ecx,[ebp+12]
        mov edx,[ebp+16]
        int INT_VECTOR_SYS_CALL

        pop edx
        pop ecx
        pop ebx
        pop ebp
        ret
;;; int write(char *buf,int len);
;; write:
;;         push ebp
;;         mov ebp,esp
        
;;         push ecx
;;         push edx
        
;;         mov eax,_NR_write
;;         mov ecx,[ebp+8]
;;         mov edx,[ebp+12]
;;         int INT_VECTOR_SYS_CALL

;;         pop edx
;;         pop ecx
;;         pop ebp
;;         ret     
;;; int printx(char *buf)
printx:
        push ebp
        mov ebp,esp
        push edx
        
        mov eax,_NR_printx
        mov edx,[ebp+8]
        int INT_VECTOR_SYS_CALL

        pop edx
        pop ebp
        ret
