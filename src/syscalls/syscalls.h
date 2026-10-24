#ifndef SYSCALL_H
#define SYSCALL_H

#include "stdint.h"
uint32_t write(uint32_t fd,char *buffer,uint32_t len);

#endif