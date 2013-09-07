/*user process api*/
BOOL create(const char *pathname,int flag);
BOOL unlink(const char *pathname,int flag);
int ls(const char *dirname,char *files);
int open(const char *pathname,int flags);
int write(int fd,const void *buf,int length);
int read(int fd,void *buf,int length);
void seek(int fd,int offset,int whence);
int close(int fd);
