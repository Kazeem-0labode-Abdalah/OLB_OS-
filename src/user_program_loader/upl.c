#include "../Kernel/kernel.h"
#include "utils/util.h"
#include "upl.h"
#include "stdint/stdint.h"
#include "PMM/pmm.h"
#include "VMM/vmm.h"

//not that since this is an educational tiny kernel user process dont have a frame allocated for heap
uint16_t next_pcb_entry(PCB_entry *kernel_pcb_holder){
      for(uint16_t i = 0; i < MAX_AMOUNT_OF_PCB_ENTRIES; i++){
        if(kernel_pcb_holder[i].state  == PCB_UNUSED){
            return i;
        }
    }

    return KERNEL_PCB_HOLDER_IS_FILLED_UP;
}


uint16_t user_program_loader(uint32_t kernel_pd, PCB_entry *kernel_pcb_holder,char * program_name){
    uint32_t *amount_of_used_pcb = (uint32_t*)AMOUNT_OF_USED_PCB;

    FAT12DirEntry dirEntry;
    memset(&dirEntry,0,sizeof(dirEntry));

    bool has_program_name_been_found = false;    
    uint16_t program_first_cluster;
    uint32_t program_size;
     uint32_t amount_of_frames;
     uint32_t first_frame_phys_addr;
    uint32_t second_frame_phys_addr;
    uint32_t third_frame_phys_addr;
    uint32_t fourth_frame_phys_addr;

    //step 1 check to see if there is space to enter a new process tlb
    uint16_t available_pcb_index = next_pcb_entry(kernel_pcb_holder);
    if(available_pcb_index == KERNEL_PCB_HOLDER_IS_FILLED_UP){
        return KERNEL_PCB_HOLDER_IS_FILLED_UP;
    }

    //step2 load root directory into memory
    uint8_t *buffer = (uint8_t*)BUFFER_ADDR;
    for(uint16_t i = ROOT_DIR_START; i < ROOT_DIR_START + TOTAL_AMOUNT_ROOT_DIR_BLOCKS;i++){
        ata_read_sector(i,buffer);
        buffer += SECTOR_SIZE;
    }

    buffer = (uint8_t*)BUFFER_ADDR;
    //step 3 locate the file name in the root dir that has been loaded into the memory
    for(uint32_t i = 0; i < (SECTOR_SIZE * TOTAL_AMOUNT_ROOT_DIR_BLOCKS/ sizeof(FAT12DirEntry));i++){
           memcpy(&dirEntry,(void *)buffer,sizeof(dirEntry));

    if (dirEntry.name[0] == 0x00){
    break;          // no more entries
    }

    if ((uint8_t)dirEntry.name[0] == 0xE5)
    {
    buffer += sizeof(FAT12DirEntry);
    continue;
    }


        if(memcmp(dirEntry.name, program_name, 11) == 0){
            program_first_cluster = dirEntry.first_cluster;
            program_size = dirEntry.file_size;
            amount_of_frames = (program_size + FRAME_SIZE - 1) / FRAME_SIZE;
            if(program_first_cluster < 2){
                return FAILURE;
            }
            has_program_name_been_found = true;
            break;
        }else{
            buffer +=sizeof(dirEntry);
            has_program_name_been_found = false;
        }
    }

    if(!has_program_name_been_found){
        return PROGRAM_NAME_WAS_NOT_FOUND_IN_ROOT_DIR;
    }

    //load the fat into memory
      buffer = (uint8_t*)BUFFER_ADDR;
    for(uint16_t i = 1; i <= TOTAL_AMOUNT_OF_FAT_BLOCKS;i++){
        ata_read_sector(i,buffer);
        buffer += SECTOR_SIZE;
    }
    buffer = (uint8_t*)BUFFER_ADDR;//set it back to the begining of memory where fat is loaded
   uint16_t *buffer_as_16bit = (uint16_t*)BUFFER_ADDR;

    //step 4 allocate a frame to the the process directory
    uint32_t *pd =(uint32_t*)alloc_frame();

    if(pd == (uint32_t *)PMM_INVALID_FRAME){
        return NO_MORE_FRAME_TO_ALLOCATE;
    }
    first_frame_phys_addr = vmm_map_page((void *)kernel_pd,(void *)0xE0000000,(void *)pd,PAGE_PRESENT | PAGE_RW);
   if(first_frame_phys_addr == ERROR){//unmap the virt addr or later give it out(map it to anpother frame)
    dalloc_frame((uint32_t)pd);
    return FAILURE;
   }
    memset((void *)0xE0000000, 0, FRAME_SIZE);
    pd[1023] = ((uint32_t)pd & ~0xFFF) | PAGE_PRESENT | PAGE_RW;// for recursive

    //step 5 load_the_program into memory
    uint32_t allocated_program_frames = 0;
    uint32_t program_frames[amount_of_frames];//this will be in the kernel stack the program size should therefore be somthing to consider
    uint32_t virt_Addr = 0x1000;
    uint32_t current_cluster = program_first_cluster;
    uint32_t current_sector = DATA_START_SECTOR + current_cluster - 2;
    uint32_t current_frame = alloc_frame();
    uint8_t *p_buffer = (uint8_t *)current_frame;
     if(current_frame == PMM_INVALID_FRAME){
        dalloc_frame((uint32_t)pd);
        return NO_MORE_FRAME_TO_ALLOCATE;
    }
    program_frames[allocated_program_frames++] = current_frame;
    second_frame_phys_addr = vmm_map_page((void *)pd,(void *)virt_Addr,(void *)current_frame, 0x07);
    if(second_frame_phys_addr==ERROR){
         dalloc_frame((uint32_t)pd);
        for(uint32_t i = 0; i < allocated_program_frames; i++) {
            dalloc_frame(program_frames[i]);
            }
        if(first_frame_phys_addr != ERROR || first_frame_phys_addr !=SUCCESS){
            dalloc_frame(first_frame_phys_addr);
        }
        return FAILURE;
    }
    virt_Addr += FRAME_SIZE;
    uint32_t amount_of_written_sectors =0;

    while(current_cluster < 0xFF8){
        if(amount_of_written_sectors == SECTORS_PER_FRAME && current_cluster < 0xFF8){
        current_frame = alloc_frame();
            if(current_frame == PMM_INVALID_FRAME){
                 dalloc_frame((uint32_t)pd);
                for(uint32_t i = 0; i < allocated_program_frames; i++) {
                  dalloc_frame(program_frames[i]);
                    }
               return NO_MORE_FRAME_TO_ALLOCATE;
            }
            program_frames[allocated_program_frames++] = current_frame;
            p_buffer = (uint8_t *)current_frame;
            third_frame_phys_addr = vmm_map_page((void *)pd,(void *)virt_Addr,(void *)current_frame, 0x07);
            if(third_frame_phys_addr == ERROR){
                 dalloc_frame((uint32_t)pd);
                for(uint32_t i = 0; i < allocated_program_frames; i++) {
                  dalloc_frame(program_frames[i]);
                    }

             if(first_frame_phys_addr != ERROR || first_frame_phys_addr !=SUCCESS){
            dalloc_frame(first_frame_phys_addr);
            }

             if(second_frame_phys_addr != ERROR || second_frame_phys_addr !=SUCCESS){
            dalloc_frame(second_frame_phys_addr);
        }
        
                    
                return FAILURE;
            }
            virt_Addr += FRAME_SIZE;
            ata_read_sector(current_sector,p_buffer);
            current_cluster = get_next_cluster(current_cluster,buffer_as_16bit);
            current_sector = DATA_START_SECTOR + current_cluster - 2;
            amount_of_written_sectors = 1;
            p_buffer +=SECTOR_SIZE;
        }else{
            ata_read_sector(current_sector,p_buffer);
            current_cluster = get_next_cluster(current_cluster,buffer_as_16bit);
            current_sector =  DATA_START_SECTOR + current_cluster - 2;
            amount_of_written_sectors++;
            p_buffer += SECTOR_SIZE;
        }
    }
    
    //step6 set up the user stack
    uint32_t process_stack = alloc_frame();
    if(process_stack == PMM_INVALID_FRAME){
         dalloc_frame((uint32_t)pd);
        for(uint32_t i = 0; i < allocated_program_frames; i++) {
                  dalloc_frame(program_frames[i]);
            }
        return NO_MORE_FRAME_TO_ALLOCATE;
    }
    fourth_frame_phys_addr = vmm_map_page((void *)pd,(void *)STACK_VIRT_ADDR,(void *)process_stack,0x07);
    if(fourth_frame_phys_addr == ERROR){
        dalloc_frame(process_stack);
         dalloc_frame((uint32_t)pd);
                for(uint32_t i = 0; i < allocated_program_frames; i++) {
                  dalloc_frame(program_frames[i]);
                    }

                    
             if(first_frame_phys_addr != ERROR || first_frame_phys_addr !=SUCCESS){
            dalloc_frame(first_frame_phys_addr);
            }

             if(second_frame_phys_addr != ERROR || second_frame_phys_addr !=SUCCESS){
            dalloc_frame(second_frame_phys_addr);
        }

           if(third_frame_phys_addr != ERROR || third_frame_phys_addr !=SUCCESS){
            dalloc_frame(third_frame_phys_addr);
        }
        
        return FAILURE;
    }

    //step 7 build and initialize the pcb
    PCB_entry process_pcb = {0};
    process_pcb.stack_pointer = process_stack;//store the physical addr of the stack
    process_pcb.id = available_pcb_index;
    process_pcb.page_directory = (uint32_t)pd;
    process_pcb.state = READY;
    process_pcb.eax = 0;
    process_pcb.ebx = 0x23;
    process_pcb.ecx = 0;
    process_pcb.edx = 0;
    process_pcb.esp_pusha = UPPER_STACK_VIRT_ADDR;
    process_pcb.ebp = process_pcb.stack_pointer;
    process_pcb.esi = 0;
    process_pcb.edi = 0;
    process_pcb.eip = 0x1000;// the start virtual addres is needed here
    process_pcb.Eflags = 0x202;
    process_pcb.cs = 0x1B;
    process_pcb.ss = 0x23;

    //step 8 enter the pcb into the kernel pcb holder
    kernel_pcb_holder[available_pcb_index] = process_pcb;
    kernel_pcb_holder[available_pcb_index].state = PCB_USED;

    //update the amount of pcb that has been used
      *amount_of_used_pcb++;

    return SUCCESS;   
}

// uint16_t user_program_destroyer(){

//     // destroy_process() // │ 
//     // ├── free stack frame
//     // ├── free program frames
//     // ├── free page tables
//     // └── free page directory
//     //mark pcb as unused 
//     //remove from kernel pcb holder
// }
