#include "Interupts/idt.h"
#include "Timer/timer.h"
#include "Keyboard/keyboard.h"
#include "PMM/pmm.h"
#include "VMM/vmm.h"
#include "kernel.h"
#include "utils/util.h"
#include "user_program_loader/upl.h"
#include "vga/vga.h"

void kernel_main(void){
    print("Welcome to the kernel of OLB_OS, focus on the text with red background\n");
    uint32_t *amount_of_used_pcb = (uint32_t*)AMOUNT_OF_USED_PCB;
    *amount_of_used_pcb = 1;

     mmap_entry_t *mmap_location = (mmap_entry_t*)MMAP_LOCATION;
     uint32_t *mmap_entries_num = (uint32_t*)MMAP_ENTRIES_NUM;
     initTimer();
     print("Timer has been initiated\n");
     initIdt();
     print("IDT has been initiated\n");
     //initKeyboard();
     // print("Keyboard has been initiated\n");
   //  init_pmm(mmap_location,(uint32_t)mmap_entries_num);
//      init_vmm();
      PCB_entry *kernel_pcb_holder = (PCB_entry *)PCB_ENTRIES_VADDR;
      uint32_t *pcb_entries_cur_index = (uint32_t*)PCB_CUR_INDEX_VADDR;
      *pcb_entries_cur_index = 0;
      memset(kernel_pcb_holder,0,sizeof(PCB_entry) * MAX_AMOUNT_OF_PCB_ENTRIES); 
      for (uint32_t i = 0; i < MAX_AMOUNT_OF_PCB_ENTRIES; i++) {
      kernel_pcb_holder[i].state = PCB_UNUSED;
     }

//   char program1[11] = {
//      'P','G','R','A','M','1',' ',' ',
//      'B','I','N'
//  };

//  char program2[11] = {
//      'P','G','R','A','M','2',' ',' ',
//      'B','I','N'
//  };
//    user_program_loader((uint32_t)page_dir,kernel_pcb_holder,program1);
  //  user_program_loader((uint32_t)page_dir,kernel_pcb_holder,program2);
    //return;

for(;;);
 }
