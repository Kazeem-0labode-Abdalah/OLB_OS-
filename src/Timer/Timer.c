// initTimer()
//       |
//       v
// Configure PIT hardware
//       |
//       v
// PIT sends IRQ0 (Interupt 32) every 1/frequency seconds
//       |
//       v
// CPU receives interrupt
//       |
//       v
// IDT dispatches IRQ0
//       |
//       v
// onIrq0() which is timer_irq0() in our case executes
//       |
//       v
// round robbin scheduler is executed and user program is switched
//       |
//       v
//

// the timer interrupts the CPU  100 times every second which is  once every 10 milliseconds.


#include "stdint.h"
#include "Interupts/idt.h"
#include "Kernel/kernel.h"
#include "utils/util.h"
#include "vga/vga.h"


void load_new_pcb_into_cpu(struct InterruptRegisters * reg,PCB_entry *next,uint32_t *pcb_entries_cur_index,uint32_t next_index){
   next->state = RUNNING; 
    reg->ss = next->ss;
    reg->useresp = next->stack_pointer;
    reg->eflags = next->Eflags;
     reg->cs = next->cs;
    reg->eip = next->eip;
    reg->err_code = (uint32_t)0; // this is a dummy err_code just to keep  the stack same
    reg->int_no = (uint32_t)0; // this is a dummy interupt number same reason as above since both errcode and intterupt number are not gonna be used in the cpu then them being dummy values is not an issues.
    reg->eax = next->eax;
    reg->ecx = next->ecx;
    reg->edx = next->edx;
    reg->ebx = next->ebx;
    reg->esp = next->esp_pusha;
    reg->ebp = next->ebp;
    reg->esi = next->esi;
    reg->edi = next->edi;
    reg->ds = next->ds;
    reg->cr3 = next->page_directory; //is this neccesarry im just doing it to bit it back in the stack in mem im already putting next directry of the pcb into the computer cr3
      asm volatile("mov %0,%%cr3" :: "r"(next->page_directory));
      *pcb_entries_cur_index = next_index;
}

 uint64_t ticks = 0;// this will keep track of how many time our clock has ticks
 const uint32_t frequency = 100;
 uint32_t *amount_of_used_pcb = (uint32_t*)AMOUNT_OF_USED_PCB;

 void irq0_Ontimer(struct InterruptRegisters * reg){
   print("Timer clicked ");
  ticks++;

  PCB_entry pcb;
  pcb.state = READY;//write now this is set to running cause it ready to run the othr state it can be in is blocked ,running or waiting since syscalls like taking user input or waiting of data from network so waiting cant be used now its blocked or runnung or ready and it will be ready to run again for now
  pcb.page_directory = reg->cr3;
  pcb.ds = reg->ds;
  pcb.edi = reg->edi;
  pcb.esi = reg->esi;
  pcb.ebp = reg->ebp;
  pcb.esp_pusha = reg->esp;
  pcb.ebx = reg->ebx;
  pcb.edx = reg->edx;
  pcb.ecx = reg->ecx;
  pcb.eax = reg->eax;
  pcb.int_no = reg->int_no;
  pcb.err_code = reg->err_code;
  pcb.eip = reg->eip;
  pcb.cs = reg->cs;
  pcb.Eflags = reg->eflags;
  pcb.stack_pointer = reg->useresp;
  pcb.ss = reg->ss;

    PCB_entry *kernel_pcb_entries = (PCB_entry*)PCB_ENTRIES_VADDR;
    uint32_t *pcb_entries_cur_index = (uint32_t*)PCB_CUR_INDEX_VADDR;
    pcb.id = (uint32_t )*pcb_entries_cur_index;
   //save the previous process state to the kernel pcb entries holder array
   kernel_pcb_entries[*pcb_entries_cur_index] = pcb;
   //get the next pcb index
   uint32_t next_index = (*pcb_entries_cur_index + 1) % *amount_of_used_pcb;
   uint32_t local_copy_of_next_index = next_index;
  // note that AMOUNT_OF_USED_PCB is initial set to 1 in kernel.c and is incremented by one evertime a new user program is ran
  
   for(uint32_t i = next_index; i < *amount_of_used_pcb; i++){
    if(kernel_pcb_entries[local_copy_of_next_index].state != READY){
      local_copy_of_next_index = (local_copy_of_next_index  + 1) % *amount_of_used_pcb;
      continue;
    }else{
       load_new_pcb_into_cpu(reg,&kernel_pcb_entries[local_copy_of_next_index],pcb_entries_cur_index,local_copy_of_next_index);
        return;
    }
      
   }
  
  //incease of the occurence where all pcb are either not ready or waiting or loop doesnt run at all, load the pcb that just executed back into the cpu
   load_new_pcb_into_cpu(reg,&kernel_pcb_entries[*pcb_entries_cur_index],pcb_entries_cur_index,pcb.id);
   return;
  
}

 void initTimer(){
    ticks = 0;
    irq_installer(0,irq0_Ontimer);
    uint32_t divisor = 1193180/frequency;
    outPortB(0x43,0x36);
    //The PIT only accepts one byte at a time, so the divisor is split into two 8-bit values and sent twice
    outPortB(0x40,(uint8_t)divisor & 0xFF);
    outPortB(0x40,(uint8_t)(divisor >> 8) & 0xFF);
 }

void get_next_pcb(PCB_entry *next_pcb,uint32_t *next_pcb_index,PCB_entry *kernel_pcb_entries){
  *next_pcb = kernel_pcb_entries[*next_pcb_index];
}




 