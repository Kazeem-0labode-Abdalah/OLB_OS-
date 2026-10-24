#include "syscalls.h"
extern void asm_write(void);
uint32_t write(uint32_t fd,char *buffer,uint32_t len){
    asm_write();
}