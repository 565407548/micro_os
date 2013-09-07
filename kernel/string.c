#include "stdlib.h"
#include "string.h"

void *strreverse(char *p_str){
    int len=strlen(p_str);
    char *p_lower=p_str;
    char *p_upper=p_str+len-1;
    char *p_limit=p_str+len;
    while(p_lower<p_upper){
        *p_limit=*p_lower;
        *p_lower=*p_upper;
        *p_upper=*p_limit;
        p_lower++;
        p_upper--;
    }
    *p_limit='\0';
    
    return p_str;
}

int strchr(const char *str,char ch){
    const char *p=str;
    while(*p!=NULL){
        if(*p==ch){
            return p-str;
        }
        ++p;
    }
    return -1;
}


int strcmp(const char *str1,const char *str2){
    const char *p=str1;
    const char *q=str2;
    /* //有问题 */
    /* while(*str1!=0 && *str2!=0 && *str1++==*str2++){ */
    /* } */
    while(*p!=0 && *q!=0 && *p==*q){
        ++p;
        ++q;
    }
    return *p-*q;
}

int strlen(const char *p_str){
    const char *p=p_str;
    while(*p)
        ++p;
    return p-p_str;
}

int strcpy(char *p_dest,const char *p_src){
    int length=0;
    while(*p_src!=0){
        *p_dest=*p_src;
        ++p_dest;
        ++p_src;
        ++length;
    }
    *p_dest=0;
    return length;
}

int strncpy(char *p_dest,const char *p_src,int len){
    int length=0;
    while(*p_src!=0 && length<len){
        *p_dest=*p_src;
        ++p_dest;
        ++p_src;
        ++length;
    }
    *p_dest=0;
    return length;
}

void* memcpy(void *p_dest,const void *p_src,int i_size){
    int i;
    for(i=0;i<i_size;i++){
        *(u8*)(p_dest+i)=*(u8*)(p_src+i);
    }
    return p_dest;
}

void memset(void *p_dest,u8 c_char,int i_size){
    int i;
    for(i=0;i<i_size;i++){
        *(u8*)(p_dest+i)=c_char;
    }
}


