#ifndef UPL_H
#define UPL_H

#include "utils/util.h"
#define SUCCESS 1
#define FAILURE 0
#define PROGRAM_NAME_WAS_NOT_FOUND_IN_ROOT_DIR 2
#define NO_MORE_FRAME_TO_ALLOCATE 3
#define KERNEL_PCB_HOLDER_IS_FILLED_UP 100
#define ROOT_DIR_START 19
#define DATA_START_SECTOR 33
#define SECTORS_PER_FRAME 8
#define BUFFER_ADDR 0x00007E00
#define TEMP_PHYSICAL_FRAME_WINDOW 0xE0000000;
#define STACK_VIRT_ADDR 0xBFFFF000
#define UPPER_STACK_VIRT_ADDR 0xC0000000
#define TOTAL_AMOUNT_ROOT_DIR_BLOCKS 14
#define TOTAL_AMOUNT_OF_FAT_BLOCKS 9
#define SECTOR_SIZE 512
uint16_t user_program_loader(uint32_t kernel_pd,PCB_entry *kernel_pcb_holder,char * program_name);
#endif