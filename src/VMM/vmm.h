#ifndef VMM_H
#define VMM_H

#include "stdint.h"
#include "utils/util.h"
#define PAGE_PRESENT 0x1
#define PAGE_RW      0x2
#define PAGE_USER    0x4
#define PMM_INVALID_FRAME 0xFFFFFFFF
struct InterruptRegisters;

void IRQ14_Onpage_fault_handler(struct InterruptRegisters * reg);
void init_vmm();
extern uint32_t page_dir[1024];
extern uint32_t page_table1[1024];
extern void init_vmm();
uint32_t vmm_map_page(void * page_dir,void * virt,void * phys,uint32_t flags);


#endif