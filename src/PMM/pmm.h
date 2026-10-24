#ifndef PMM_H
#define PMM_H


#include "stdint.h"
#define TOTAL_AMOUNT_OF_FRAME_IN_MEMORY 1048576
#define BITMAP_SIZE (TOTAL_AMOUNT_OF_FRAME_IN_MEMORY / 8)
#define FRAME_SIZE 4096
#define ERROR 0
#define BITMAP_ADDR 0x00103000
#define MMAP_LOCATION 0x00003000
#define MMAP_ENTRIES_NUM 0x00003500
#define PMM_INVALID_FRAME 0xFFFFFFFF
#define KERNEL_SIZE 0x500000

typedef struct{
uint64_t base;
uint64_t length;
uint32_t type;
uint32_t acpi;
}__attribute__((packed)) mmap_entry_t;

void init_pmm(mmap_entry_t *mmap,uint32_t mmap_entries);

uint32_t alloc_frame();

void dalloc_frame(uint32_t frame_addr);
void reserve_region(uint64_t addr, uint64_t size);
void reserve_frame(uint64_t frame_addr);
#endif