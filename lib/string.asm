        global memcpy
        global memset
        global strlen
        global strcpy
        
;;; void *memcpy(void *p_dst,void* p_src,int i_size);
memcpy:
        push ebp
        mov ebp,esp
        push ecx
        push edi
        push esi
        
        mov edi,[ebp+8]
        mov esi,[ebp+12]
        mov ecx,[ebp+16]

.1:
        cmp ecx,0
        jz .2
        mov al,[ds:esi]
        inc esi
        mov byte[es:edi],al
        inc edi

        dec ecx
        jmp .1

.2:
        mov eax,[ebp+8]
        
        pop esi
        pop edi
        pop ecx
        pop ebp
        ret

;;; void memset(void *p_dst,char c_char,int i_size);
memset:
        push ebp
        mov ebp,esp
        push ecx
        push edx
        push edi
        
        mov edi,[ebp+8]
        mov edx,[ebp+12]
        mov ecx,[ebp+16]

.1: 
        cmp ecx,0
        jz .2
        mov byte[es:edi],dl
        inc edi

        dec ecx
        jmp .1
.2:
        
        pop edi
        pop edx
        pop ecx
        pop ebp
        ret

;;; int strlen(char *p_str)
strlen:
        push ebp
        mov ebp,esp
        push esi
        
        mov eax,0
        mov esi,[ebp+8]

.1: 
        cmp byte[ds:esi],0
        jz .2
        inc esi
        inc eax
        jmp .1
.2: 
        pop esi
        pop ebp
        ret

;;; char *strcpy(char *p_dest,char *p_src)
strcpy:
        push ebp
        mov ebp,esp
        push edi
        push esi
        
        mov esi,[ebp+12]
        mov edi,[ebp+8]
.1:
        mov al,[esi]
        inc esi
        mov byte[edi],al
        inc edi
        cmp al,0
        jnz .1

        mov eax,[ebp+8]

        pop esi
        pop edi
        pop ebp
        ret
