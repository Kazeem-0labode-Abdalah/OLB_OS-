#include "stdint.h"
#include "util.h"
#include "vga/vga.h"

void memset(void *dest,char val,uint32_t count){
    char *temp = (char*) dest;
    for(;count !=0;count--){
        *temp++ = val;
    }   
}

int memcmp(const void *a, const void *b, int n){
     const unsigned char *pa = a;
     const unsigned char *pb = b;
    for(int i = 0; i < n;i++){
        if(pa[i] != pb[i]){
            return 1;
        }
            }
      return 0;
   }

void *memcpy(void *dest,const void *src,uint32_t size){
    unsigned char *temp_dest = (unsigned char*) dest;
    const unsigned char *temp_src = (const unsigned char*) src;

    for(;size !=0;size--){
        *temp_dest++ = *temp_src++;
    }

    return dest;
}

void outPortB(uint16_t port,uint8_t value){
    asm volatile("outb %1, %0" : : "dN" (port), "a" (value));
}

char importB(uint16_t port){
    char rv;
    asm volatile("inb %1, %0" : "=a"(rv):"dN"(port));
    return rv;
}

uint16_t inw(uint16_t port){
    uint16_t data;
    asm volatile("inw %1,%0":"=a"(data):"Nd"(port));
    return data;
};

void outw(uint16_t port,uint16_t data){
    asm volatile("outw %0, %1":: "a"(data), "Nd"(port));
}

uint16_t get_next_cluster(uint16_t cur_cluster,uint16_t *fat){
    uint16_t fat_offset = (cur_cluster * 3) / 2;

    uint16_t two_bytes = fat[fat_offset] | (fat[fat_offset + 1] << 8);

    if ((cur_cluster & 1) == 0)
        return two_bytes & 0x0FFF;
    else
        return two_bytes >> 4;
}

void ata_read_sector(uint32_t lba,uint8_t *buffer){
    outw(0x1F6,(0xE0|(lba >> 24)) & 0x0F);//Drive/head register,master/slave drive,highest four LBA bits
    outPortB(0x1F2,1);//Read exactly one sector.
    outPortB(0x1F3,lba & 0xFF);//send lba, lowest 8 bits 
    outPortB(0x1F4,(lba >>8) & 0xFF);//Next 8 bits.
    outPortB(0x1F5,(lba >> 16) & 0xFF);//Next 8 bits.The upper four bits were already sent to 0x1F6.

    while(importB(0x1F7) & 0x80);//BSY
    while(!(importB(0x1F7) & 0x08))//DRG
    for(uint16_t i = 0; i < 256 ;i++){
        buffer[i] = inw(0x1F0);    
    }

    if(importB(0x1F7) & 0x01){
        print("An error  ocured while loading a sector using the ata_read_Sector function/n");
    }
}