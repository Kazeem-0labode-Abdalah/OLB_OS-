#ifndef UTIL_H
#define UTIL_H

#include "stdint.h"
void memset(void *dest,char val,uint32_t count);
void *memcpy(void *dest,const void *src,uint32_t size);
int memcmp(const void *a, const void *b, int n);
void outPortB(uint16_t port,uint8_t value);
char importB(uint16_t port);
uint16_t inw(uint16_t port);
void outw(uint16_t port,uint16_t data);
void ata_read_sector(uint32_t lba,uint8_t *buffer);
uint16_t get_next_cluster(uint16_t cur_cluster,uint16_t *fat);
#define SUCCESS 1
#define ERROR 0

struct InterruptRegisters
{
    uint32_t cr3;
    uint32_t ds;
    uint32_t edi,esi,ebp,esp,ebx,edx,ecx,eax;
    uint32_t int_no,err_code;
    uint32_t eip,cs,eflags,useresp,ss;
};

typedef struct{
    char name[11];
    uint8_t attr;
    uint16_t first_cluster;
    uint32_t file_size;
}FAT12DirEntry;

#endif