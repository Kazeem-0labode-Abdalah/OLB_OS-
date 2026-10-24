#ifndef KERNEL_H
#define KERNEL_H

#include "stdint.h"

#define RUNNING 0
#define READY 1
#define BLOCKED 2
#define WAITING 3
#define PCB_USED 4
#define PCB_UNUSED 5
#define MAX_AMOUNT_OF_PCB_ENTRIES 100
#define PCB_ENTRIES_VADDR      0x001031F5
#define PCB_CUR_INDEX_VADDR     0x0010AF95
#define AMOUNT_OF_USED_PCB      0x0010AF99

typedef struct{
    uint32_t id;
    uint32_t stack_pointer;
    uint32_t eax,ebx,ecx,edx,esp_pusha,ebp,esi,edi,eip,Eflags,cs,ss,ds,int_no,err_code;
    uint32_t page_directory;
    uint8_t state;
}PCB_entry;

#endif