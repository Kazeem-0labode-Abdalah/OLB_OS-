global _start
extern kernel_main

_start:
;mov dword [0xB8004], 0x0F4B0F4B  ; 'K'
call kernel_main
mov word [0xB800C], 0x0F52 ; R: kernel_main returned

halt:
    hlt
    jmp halt