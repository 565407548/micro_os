#include "type.h"
#include "const.h"
#include "./include/hd_const.h"
#include "./include/hd.h"
#include "./include/fs_const.h"
#include "./include/fs.h"

static FILE_DESCRIPTOR file_descriptor_table[FILE_DESCRIPTOR_COUNT];
static INODE inode_table[INODE_COUNT];

void init_inode_table(){
    int index;
    for(index=0;index<INODE_COUNT;index++){
        inode_table[index].i_share_count=0;
    }
}

/* int get_inode_table_length(){ */
/*     return INODE_COUNT; */
/* } */

INODE* get_free_inode_from_table(){
    int index=0;
    for(index=0;index<INODE_COUNT;index++){
        if(inode_table[index].i_share_count==0){
            inode_table[index].i_share_count++;
            return &inode_table[index];
        }
    }
    return NULL;
}

INODE* get_inode_from_table(int inode_index){
    int index=0;
    for(index=0;index<INODE_COUNT;index++){
        if(inode_table[index].i_inode_index==inode_index){
            inode_table[index].i_share_count++;
            return &inode_table[index];
        }
    }
    return NULL;
}

void free_inode(INODE *inode){
    inode->i_share_count--;
}

void init_file_descriptor_table(){
    int index;
    for(index=0;index<FILE_DESCRIPTOR_COUNT;index++){
        file_descriptor_table[index].fd_inode=NULL;
    }
}
/* int get_file_descriptor_table_length(){ */
/*     return FILE_DESCRIPTOR_COUNT; */
/* } */

FILE_DESCRIPTOR* get_free_fd_from_table(){
    int index;
    for(index=0;index<FILE_DESCRIPTOR_COUNT;index++){
        if(file_descriptor_table[index].fd_inode==NULL)
            return &file_descriptor_table[index];
    }
    return NULL;
}
void free_fd(FILE_DESCRIPTOR* file_descriptor){
    file_descriptor->fd_inode=NULL;
}
