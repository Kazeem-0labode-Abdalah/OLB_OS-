;Final execution flow
;Stage2
; │
; ├── Read FAT
; ├── Read root directory
; ├── Initailly load KERNEL.BIN to 0x20000 
; │
; ▼
;Read mmap from the BIOS
;  │
; ▼
;Load GDT
; │
; ▼
;Protected Mode
 ;│
; ▼
;Copy the kernel from 0x2000 to 0x00100000
 ;│
; ▼
;Far jump
;jmp 0x08:0x00100000
; │
; ▼
;Kernel begins executing at 1 MiB


BITS 16;we are still oprating in 16 bit mode
;ORG 0x10000
ORG 0
bdb_byte_per_sector: DW 512
bdb_reserved_sectors: DW  1
bdb_fat_count: DB 2
bdb_dir_entries_count: DW 0E0h
bdb_sectors_per_fat: DW 9
bdb_sectors_per_track: DW 18
bdb_heads: DW 2
ebr_drive_number: DB 0
  
start:
cli
    mov ax,cs ;cs is 0x1000
    mov ds,ax; ds must be 0x100 too
    mov es,ax
    mov ss,ax
    mov sp,0x00007000
sti

;create the TSS
align 4
tss_start:
    dd 0                  ; Previous task link

    dd 0                  ; ESP0
    dw KERNEL_DATA_SEL     ; SS0
    dw 0                  ; Reserved

    dd 0                  ; ESP1
    dw 0                  ; SS1
    dw 0

    dd 0                  ; ESP2
    dw 0                  ; SS2
    dw 0

    dd 0                  ; CR3
    dd 0                  ; EIP
    dd 0                  ; EFLAGS

    dd 0                  ; EAX
    dd 0                  ; ECX
    dd 0                  ; EDX
    dd 0                  ; EBX
    dd 0                  ; ESP
    dd 0                  ; EBP
    dd 0                  ; ESI
    dd 0                  ; EDI

    dw 0                  ; ES
    dw 0

    dw 0                  ; CS
    dw 0

    dw 0                  ; SS
    dw 0

    dw 0                  ; DS
    dw 0

    dw 0                  ; FS
    dw 0

    dw 0                  ; GS
    dw 0

    dw 0                  ; LDT
    dw 0

    dw 0                  ; Trap
    dw tss_end - tss_start ;I/O bitmap offset 0x68 for tss current sizeS

tss_end:



;To load the kernel the following steps must be observed
MOV ax,0
MOV es,ax

;step 1: get the lba of the root directory
MOV ax,[bdb_sectors_per_fat]
MOV bx,[bdb_fat_count]
MUL bx
ADD ax,[bdb_reserved_sectors]
PUSH ax; the value ax holds is 19

;step 2: determine the size of the directory (how many sector it contains)
MOV ax,[bdb_dir_entries_count];
SHL ax,5; ax*32
XOR dx,dx
DIV word[bdb_byte_per_sector];will provide the amount of sectors to read
TEST dx,dx
JZ .store
INC ax

.store:
MOV [root_dir_sector_count],al
JMP read_root_directory

;step 3: read root directory into the memory

read_root_directory:
;set the stage to read the root directory 
    mov al,[root_dir_sector_count]
    mov [sector_count],al
    POP ax
    mov ah,0
    MOV bx,0000
    MOV es,bx
    MOV bx,buffer
    CALL read_disk
    JC faliure_loading_rootdir_in_mem
    JNC success_loading_rootdir_in_mem  

    faliure_loading_rootdir_in_mem:
   ; MOV si,read_dir_faliure_msg
   ; CALL print
    JMP halt

    success_loading_rootdir_in_mem:
   ; MOV si,read_dir_success_msg
   ; CALL print
   
    XOR bx,bx
    MOV di,buffer
    CLD


;step 4 :search for the kernel in the root directory that has been loaded into memory
search_kernel:
    MOV si,file_kernel_bin_name
    MOV cx,11
    PUSH di
    CLD
    REPE CMPSB
    POP di
    JE found_kernel
    ADD di,32
    INC bx
    CMP bx,[bdb_dir_entries_count]
    JL search_kernel
    JMP kernel_not_found

kernel_not_found:
  ;  MOV si,kernel_not_found_msg
  ;  CALL print
    HLT
    JMP halt

found_kernel:
    ;MOV si,kernel_found_msg
    ;CALL print

    MOV ax,[es:di +26]
    MOV [kernel_cluster],ax

   ;set the stage to load the fat table into memory
    mov ax,[bdb_sectors_per_fat]
    mov [sector_count],al
    MOV ax,[bdb_reserved_sectors]
    ; load the fat table to 0000:0x7E00 make es 0000 temparily to use used read disk to read fat then change it back to its origuinal value
    PUSH es
    MOV cx,0
    MOV es,cx
    MOV bx,buffer
    CALL read_disk
    JC fail_disk_read
    POP es
    ;MOV si,read_fat_table_success_msg
    ;CALL print
  
    ;now set the stage to load the kernel into memory
    MOV bx,kernel_load_segment
    MOV es,bx
    MOV bx,kernel_load_offset

    mov dword [KERNEL_SIZE],0
load_kernel_loop:
    MOV ax,[kernel_cluster]
    ADD ax,31
    mov byte [sector_count],1
    CALL read_disk;read the current sector
    JC fail_disk_read
    
    add dword [KERNEL_SIZE],512
    ADD bx,[bdb_byte_per_sector]
    MOV ax,[kernel_cluster];Following the FAT12 cluster chain
    MOV cx,3
    MUL cx
    MOV cx,2
    DIV cx; (current kernel cluster * 3)/2

    ;set up our buffer
    MOV si,buffer
    ADD si,ax
    ; FAT is at 0000:7E00
    PUSH ds
    MOV ax,0
    MOV ds,ax
    MOV ax,[ds:si]
    POP ds
    OR dx,dx
    JZ even

odd:
    SHR ax,4;You shift right because odd FAT12 entries start 4 bits into the two-byte value.
    JMP next_cluster_after

even:
    AND ax,0x0FFF; remove four upper bit

next_cluster_after:
    ;check to see if weve gotten to the end of the last fat entry entry of kernel file
    CMP ax,0xFF8
    JAE read_finish
    MOV [kernel_cluster],ax
    JMP load_kernel_loop

read_finish:
    ; finished loading kernel
  ; mov si,finished_loading_kernel_msg
   ;call print
    jmp init_read_mmap_from_bios


halt:
    JMP halt


print:
    PUSH si
    PUSH ax
    PUSH bx

print_loop:
    LODSB
    OR al,al
    JZ done_print    
    MOV ah,0x0E
    INT 0x10
    JMP print_loop

done_print:
    POP bx
    POP ax
    POP si
    RET


print_hex:
    PUSH AX
    PUSH BX
    PUSH CX
    PUSH DX

    MOV CX,4

.next:
    ROL BX,4
    MOV AL,BL
    AND AL,0x0F

    CMP AL,9
    JBE .digit
    ADD AL,'A'-10
    JMP .print

.digit:
    ADD AL,'0'

.print:
    MOV AH,0x0E
    INT 10h
    LOOP .next

    POP DX
    POP CX
    POP BX
    POP AX
    RET


read_disk:
; read_disk
; IN:
;   AX = starting LBA
;   sector count = sectors to read
;   ES:BX = destination buffer
; OUT:
;   CF clear on success
;   CF set on failure

;PUSHA
lba_to_chs:
    ;calculation for the sector
    XOR dx,dx
    DIV word [bdb_sectors_per_track]
    INC dx
    MOV cl,dl;sector

    ;calculation for the head
    XOR dx,dx
    DIV word[bdb_heads]
    MOV dh,dl;head

    ;calculation for the cylinder
    MOV ch,al
    SHL ah,6
    OR cl,ah;cynlinder

    MOV di,3 ;for retry loop

retry:

    XOR ax,ax
    MOV ah,02h
    MOV al,[sector_count]
    mov dl,[ebr_drive_number]
    INT 13h
    JNC done_read
    CALL disk_reset
    DEC di
    TEST di,di
    JNZ retry

fail_disk_read:
    ;MOV si,general_read_faliure_msg
    ;CALL print
    HLT
    JMP halt

disk_reset:
    mov dl,[ebr_drive_number]
    MOV ah,0
    STC
    INT 13h
    JC fail_disk_read
    RET

done_read:
 ;MOV si,done_read_m
 ;CALL print
  RET


init_read_mmap_from_bios:
;MOV si,reading_the_mmap_from_bios_msg
;CALL print
MOV di,MMAP_LOCATION;store the mmap at this location in memory
XOR ebx,ebx; start 0
MOV es,bx
MOV edx,0x534D4150; SMAP signature
XOR bp,bp

read_mmap_from_bios:
MOV eax,0xE820
MOV ecx,24
INT 0x15
JC read_mmap_from_bios_done
INC bp
ADD di,24
test ebx,ebx
JNZ read_mmap_from_bios

read_mmap_from_bios_done:
MOV [MMAP_ENTRIES_COUNT_MEMORY_LOCATION], bp
;mov bx,bp
;call print_hex
;mov si,amount_of_mmap_entries_gotten_from_the_bios_msg
;call print

;Each descriptor is 8 bytes in size and  tells the CPU
;Where the segment starts
;How big it is
;Whether it contains code or data
;Whether it can be written
;Whether it can be executed
;Whether it is 16-bit or 32-bit

jmp gdt_setup_code
;Create a Global Descriptor Table (GDT)
align 8

gdt_start:

dq 0;first gdt entry which is a  null descriptor

;kernel code segment
dw 0xFFFF;Lower 16 bits of the segment limit.
dw 0;Lower 16 bits of the base address.
db 0;Middle 8 bits of the base.
db 10011010b; P=1 (present), DPL=00 (Ring 0),S=1 (code/data descriptor),TYPE=1010 (executable, readable, not accessed),
db 11001111b; G=1  (4 KB granularity),D=1  (32-bit segment),L=0  (not 64-bit),AVL=0,Limit high = 1111.
db 0;final byte == Highest 8 bits of the base address.in this case 0s

;kernel data segment
dw 0xFFFF
dw 0
db 0
db 10010010b
db 11001111b
db 0


;user code segment
dw 0xFFFF
dw 0
db 0
db 11111010b 
db 11001111b
db 0

;user data segment
dw 0xFFFF
dw 0
db 0
db 11110010b
db 11001111b
db 0

gdt_tss:
dw 0                  
dw 0                  
db 0                  
db 10001001b           
db 00000000b          
db 0     

gdt_end:


;Load the GDT,cpu loads a 6 byte structure hence the below
gdt_setup_code:

gdt_descriptor:
dw gdt_end - gdt_start-1; size (limit)
dd gdt_start + 0x10000;base

; Set up the TSS descriptor in the GDT
mov eax, tss_start + 0x10000

; Set the TSS segment limit
mov word [gdt_tss + 0], tss_end - tss_start - 1

; Set the lower 16 bits of the TSS base address
mov word [gdt_tss + 2], ax

; Set the upper 16 bits of the TSS base address
shr eax, 16
mov byte [gdt_tss + 4], al
mov byte [gdt_tss + 7], ah

; Set flags and upper limit bits to zero
mov byte [gdt_tss + 6], 0



;Disable interrupts and load the GDT
cli
lgdt [gdt_descriptor]


;Turn on Protected Mode Setting bit 0 (PE) enables protected mode.
mov eax,cr0
or eax,1
mov cr0,eax


;the CPU is technically in protected mode, but:
;CS still contains the old real-mode value.
;The instruction queue may still contain old instructions.
;The CPU does not yet know which GDT code segment to use.therefore
;Do a Far Jump This is required to flush the instruction queue.


JMP  dword KERNEL_CODE_SEL:(protected_mode + 0x10000);this will make cs = 0x08

BITS 32;Switch to 32-bit Assembly this tells NASM to encode the following instructions as 32-bit instructions.note that it does not switch the CPU into protected mode. CR0.PE + the far jump did that
protected_mode:
MOV ax,KERNEL_DATA_SEL
MOV ds,ax
MOV es,ax
MOV fs,ax
MOV gs,ax
MOV ss,ax
MOV esp,0x00600000
;Now every instruction assembled is 32-bit.,we are now oprating in 32-bit mode


MOV dword[tss_start+ 0x10000 + 4],0x00600000; update the SP of the tss nd move intp LTR the descriotor of the tss in the gdt
MOV ax,TSS_SEL
LTR ax;this load TR = 0x28

;copy the kernel from where it was initially loaded 0x00020000 to 0x00100000
MOV esi,0x00020000
MOV edi,0x00100000
MOV ecx, [KERNEL_SIZE + 0x10000]
MOV eax, ecx
SHR ecx, 2; copy dwords
cld
REP MOVSD

AND EAX,3;remaining bytes
MOV ECX, EAX
REP MOVSB

;jump to the new location of the kernel 0x00100000
JMP KERNEL_CODE_SEL:0x00100000


read_faliure_msg: DB "Reading the root directory failed!",0x0D,0x0A,0
MMAP_ENTRIES_COUNT_MEMORY_LOCATION equ 0x03500
MMAP_LOCATION equ 0x03000
sector_count db 0
root_dir_sector_count db 0
KERNEL_SIZE dd 0
file_kernel_bin_name: DB 'KERNEL  BIN'
kernel_found_msg: DB "KERNEL has been found in root dir",0x0D,0x0A,0
kernel_not_found_msg : DB "KERNEL.BIN was not found in the root directory",0x0D,0x0A,0
read_dir_success_msg: DB "Reading the root directory to search for the kernel was successful",0x0D,0x0A,0
read_fat_table_success_msg: DB "Reading the FAT table was successful",0x0D,0x0A,0
read_dir_faliure_msg: DB "Reading the root directory to search for the kernel failed",0x0D,0x0A,0
general_read_faliure_msg: DB "disk Read fail",0x0D,0x0A,0
done_read_m: DB "disk read done",0x0D,0x0A,0
reading_the_mmap_from_bios_msg:DB "Reading the mmap from the bios",0x0D,0x0A,0
finished_loading_kernel_msg : DB "The kernel has been sucessfully read into the memory",0x0D,0x0A,0
amount_of_mmap_entries_gotten_from_the_bios_msg:DB" mmap entries has been obtained from the bios",0x0D,0x0A,0
before_gdt_msg db 0x0D, 0x0A, "Reached GDT setup", 0
kernel_cluster: DW 0
kernel_load_segment EQU 0x2000
kernel_load_offset EQU 0  
buffer equ 0x7E00
KERNEL_CODE_SEL equ 0x08
KERNEL_DATA_SEL equ 0x10
USER_CODE_SEL equ 0x18
USER_DATA_SEL equ 0x20
TSS_SEL equ 0x28