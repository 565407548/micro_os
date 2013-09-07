#include "stdlib.h"

#include "kliba.h"

#include "stdio.h"

void disp_int(int input){
    char output[16];
    itoa(output,input,16);
    disp_str(output);
}
