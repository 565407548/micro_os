#include "stdlib.h"
#include "string.h"

static char *itoa_10_to_2(char *str,int num);
static char *itoa_10_to_10(char *str,int num);
static char *itoa_10_to_16(char *str,int num);

/* static char *htoa(char *str,char *hexs); */
/* static char *otoa(char *str,char *octs); */

/* char *itoa(char *str,int num){ */
/*     return itoa_10_to_2(str,num); */
/* } */

char *itoa(char *str,int num,int base){
    char *p_char=0;
    switch(base){
    case 2:
        p_char=itoa_10_to_2(str,num);
        break;
    case 16:
        p_char=itoa_10_to_16(str,num);
        break;
    case 10:
    default:
        p_char=itoa_10_to_10(str,num);
        break;
    }
    return p_char;
}

void delay(int time){
    volatile int i,j,k;
    for(i=0;i<time;i++){
        for(j=0;j<10;j++){
            for(k=0;k<1000;k++){}
        }
    }
}

static char *itoa_10_to_2(char *str,int num){
    char *p=str;
    char ch;
    int i;
    int flag=0;

    if(num==0){
        *p++='0';
    }else{
        for(i=31;i>=0;i--){
            ch=(num>>i)&1;
            if(flag || ch>0){
                flag=1;
                ch+='0';
                *p++=ch;
            }
        }
    }
    *p++='b';
    *p=0;
    return str;
}

static char *base_convert(char *str,int num,int base)
{
    char *p=str;
    int q,r;
    q=num;
    do{
        r=q%base;
        q=q/base;

        r=r+'0';
        if(r>'9'){
            r=r+7;
        }
        *p++=r;
    }while(q>0);
    *p='\0';

    strreverse(str);
    return str;
}

static char *itoa_10_to_10(char *str,int num){
    return base_convert(str,num,10);
}

static char *itoa_10_to_16(char *str, int num){
    char *p=str;
    char ch;
    int i;
    int flag=0;

    *p++='0';
    *p++='x';

    if(num==0){
        *p++='0';
    }else{
        for(i=28;i>=0;i-=4){
            ch=(num>>i)&0xF;
            if(flag || ch>0){
                flag=1;
                ch+='0';
                if(ch>'9'){
                    ch+=7;
                }
                *p++=ch;
            }
        }
    }
    *p=0;
    return str;
}
