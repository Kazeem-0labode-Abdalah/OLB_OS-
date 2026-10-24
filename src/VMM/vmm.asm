global asm_init_vmm
;remeber to make the linker enforce 
;0x00100000 = PD
;0x00200000 = PT1
;0x00300000 = PT2
section .bss
align 4096
global page_dir
page_dir:
    resd 1024

align 4096
global page_table1
page_table1:
    resd 1024

align 4096
global page_table2
page_table2:
    resd 1024

section .text
asm_init_vmm:
;fill page_table1

MOV ecx, 0
fill_pt1:
    MOV eax, ecx
    SHL eax, 12
    OR eax, 0x003
    MOV [page_table1 + ecx*4], eax

    INC ecx
    CMP ecx, 1024
    JL fill_pt1


    MOV ecx, 0
fill_pt2:
    MOV eax, ecx
    SHL eax, 12
    ADD eax, 0x00400000
    OR eax, 0x003

    MOV [page_table2 + ecx*4], eax

    INC ecx
    CMP ecx, 512
    JL fill_pt2
    
    ;link page0 table to page directory, make PD0 point to our page 0
    MOV eax,page_table1
    OR eax,0x003;present =1,readwrite =1
    MOV [page_dir + 0],eax

    ;link page1 table to page directory, make PD0 point to our page 1
    MOV eax,page_table2
    OR eax,0x003
    MOV [page_dir + 4],eax

    ;load cr3 by telling it where the page directory is
    MOV eax,page_dir
    MOV cr3,eax

    ;enable pagging  bit in CR0, this is the "on switch" bit 31 of CR0 = PG bit
    MOV eax,CR0
    OR eax,0x80000000
    MOV cr0,eax
    ;after the last instruction above the cpu starts using  virtual address the next instruction  fetch goes through page table

    RET