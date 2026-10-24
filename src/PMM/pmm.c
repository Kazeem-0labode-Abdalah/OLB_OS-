#include "stdint.h"
#include "pmm.h"
#include "utils/util.h"

uint8_t * bitmap;
uint64_t highest_frame = 0;
uint64_t bitmap_size = 0;

void init_pmm(mmap_entry_t *mmap, uint32_t mmap_entries){
    uint64_t frame_start = 0; 
    uint64_t amount_of_frames = 0;
    
    //get the highest frame 
    for (uint64_t i = 0; i < mmap_entries; i++){
    uint64_t end = mmap[i].base + mmap[i].length;
    uint64_t frame = (end + FRAME_SIZE - 1) / FRAME_SIZE;

    if (frame > highest_frame)
        highest_frame = frame;
   }

    //detrmine bitmap size
    bitmap_size = (highest_frame + 7) / 8;

    //place bitmap at 0x00103000
    bitmap = (uint8_t*) BITMAP_ADDR;

    //mark all frames as used in bitmap
    memset(bitmap, 0xFF, bitmap_size);

    //mark usable regions as free
    for(uint64_t i = 0; i < mmap_entries; i++){
        if(mmap[i].type == 1){
            frame_start = mmap[i].base/FRAME_SIZE;
            amount_of_frames = mmap[i].length/FRAME_SIZE;
            
        for(uint64_t j = frame_start; j < frame_start + amount_of_frames;j++){
            
           uint32_t byte = j / 8;
           uint32_t bit = j % 8;

            bitmap[byte] &= ~(1 << bit);
        }
        }

    }

//set the bitmap as used 
//set stage2 as used cause it contains gdt
//set mmap,mmap_entries,kernel stack downward,kernel, pd_and pt,kernel heap 

// 0x00003000     E820 mmap
// 0x00003500     mmap count
// 0x00007C00     Stage1
// 0x00001000     Stage2
// 0x00020000     kernel temporary load
// 0x00100000 to  0x00500000 is kernel pd,pt,bitmap,stack,heap 
// 0x00400000     PMM bitmap
// 0x005F0000     kernel stack
// 0x00600001     kernel heap
// 0x001031F5 PCB_ENTRIES_VADDR 
// 0x0010AF95 PCB_CUR_INDEX_VADDR
//  0x0010AF99 AMOUNT_OF_USED_PCB

//first MB: BIOS / stage1 / stage2 / mmap /mmap num holder and kernel temporary location 
reserve_region(0x0, 0x100000);

// reserve kernel pd,pt,bitmap,stack,heap and all it other properties
reserve_region(0x100000,KERNEL_SIZE);
}

uint32_t alloc_frame(){
    for(uint32_t i = 0; i < highest_frame; i++){
       uint32_t byte = i / 8;
       uint32_t bit = i % 8;

        if((bitmap[byte] & (1 << bit)) == 0){
            bitmap[byte] |= (1 << bit);
            return i * FRAME_SIZE;
        }
    }
        return PMM_INVALID_FRAME;
}

void dalloc_frame(uint32_t frame_addr){
    if(frame_addr % FRAME_SIZE != 0)
        return;

    uint32_t frame = frame_addr / FRAME_SIZE;
    uint32_t byte = frame / 8;
    uint32_t bit  = frame % 8;
    if(bitmap[byte] & (1 << bit)){
         bitmap[byte] &= ~(1<< bit);
    }
}

void reserve_region(uint64_t addr, uint64_t size)
{
    uint64_t start = addr / FRAME_SIZE;
    uint64_t frames = (size + FRAME_SIZE - 1) / FRAME_SIZE;

    for(uint64_t i = 0; i < frames; i++)
        reserve_frame((start+i) * FRAME_SIZE);
}

void reserve_frame(uint64_t frame_addr)
{
    uint64_t frame = frame_addr / FRAME_SIZE;

    uint32_t byte = frame / 8;
    uint32_t bit = frame % 8;

    bitmap[byte] |= (1 << bit);
}