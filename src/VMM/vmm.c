#include "vmm.h"
#include "utils/util.h"
#include "stdint/stdint.h"
#include "vga/vga.h"
#include "PMM/pmm.h"
#include "Interupts/idt.h"
extern void asm_init_vmm(void);

void init_vmm(){
    asm_init_vmm();
    irq_installer(14,&IRQ14_Onpage_fault_handler);
}

uint32_t vmm_map_page(void * page_dir,void * virt,void * phys,uint32_t flags){
    uint32_t virtual_addr = (uint32_t)virt;
    uint32_t physical_addr = (uint32_t)phys;
    uint32_t *page_dir_addr = (uint32_t *)page_dir;
    bool was_new_pt_created = false;
    uint32_t new_pt;

    //break the virtual addr and get the page dir index and the page table index
    uint32_t pd_index = virtual_addr >> 22; //top 10 bits
    uint32_t pt_index = (virtual_addr >> 12) & 0x3FF; // middle 10 bits

    //get the page dir entry for the virtual address
    uint32_t pd_entry = page_dir_addr[pd_index];

    //does the table exist in the page dir?
    if(!(pd_entry & 0x1)){//check present bit which is bit 0
    //if page table does not exist, we create it
    new_pt = (uint32_t) alloc_frame();
    if(new_pt == PMM_INVALID_FRAME)
    {
          print("Out of physical memory nwhile using vmm_map_page\n");
          return ERROR;
    }
    was_new_pt_created = true;
    memset((void *)new_pt,0,FRAME_SIZE);
    // enter the newly created table into the directory
    page_dir_addr[pd_index] = (uint32_t)new_pt | PAGE_PRESENT|PAGE_RW;
    //reload pd_entry
    pd_entry = page_dir_addr[pd_index];
    }

    //get the pointer to the page table
    uint32_t *pt_pointer = (uint32_t*)(0xFFC00000 + pd_index * 0x1000);
    //put the physical addr that is to be mapped to the virtual address into the page table
    pt_pointer[pt_index] = (physical_addr & ~0xFFF)| (flags & 0xFFF);
    //flush the cpu
    asm volatile("invlpg (%0)":: "r"(virt):"memory");

    if(was_new_pt_created){
        return new_pt;
    }
    return SUCCESS;
}

void IRQ14_Onpage_fault_handler(struct InterruptRegisters * reg){
    //what types of error are there which are in the reg struct error_no
    uint32_t fault_addr;
    asm volatile("mov %%cr2,%0":"=r"(fault_addr));
    print("A page fault has occured the error code \n");

if (reg->err_code & 1){
    print("Protection violation\n");
}else{
    print("Non-present page\n");
}

if (reg->err_code & 2){
    print("Write\n");
}else{
    print("Read\n");
}
if (reg->err_code & 4){
    print("User mode\n");
}else{
    print("Kernel mode\n");
    print("\n");
}
}//end of function