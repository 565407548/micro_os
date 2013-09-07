enum e_direction{
    l2r,
    r2l,
    u2d,
    d2u
}DIRECTION;

/*lib/string.asm*/
void* memcpy(void *p_dst,const void *s_dst,int i_size);
void memset(void *p_dst,u8 c_char,int i_size);

int strlen(const char *p_str);
int strcpy(char *p_dest,const char *p_src);
int strncpy(char *p_dest,const char *p_src,int len);
int strcmp(const char *str1,const char *str2);

/*kernel/string.c*/
void *strreverse(char *p_str);
int strchr(const char *str,char ch);
