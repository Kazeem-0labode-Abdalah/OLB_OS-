;BIOS
; │
; ▼
;boot.asm starts at 0000:7C00
; │
; ▼
;Initialize segments/stack
; │
; ▼
;Enable A20
; │
; ▼
;Load STAGE2.BIN to 1000:0000
; │
; ▼
;Far jump to 1000:0000
; │
; ▼
;Stage2

ORG 0x7C00;load this starting at this memory location
BITS 16;we are currently oprating in 16 bit mode .Generate 16-bit instructions.

JMP SHORT main
nop

bdb_oem: DB 'MSWIN4.1'
bdb_byte_per_sector: DW 512
bdb_sectors_per_cluster: DB 1
bdb_reserved_sectors: DW  1
bdb_fat_count: DB 2
bdb_dir_entries_count: DW 0E0h
bdb_total_sectors: DW 2880
bdb_media_descriptor_type: DB 0F0h
bdb_sectors_per_fat: DW 9
bdb_sectors_per_track: DW 18
bdb_heads: DW 2
bdb_hidden_sectors: DD 0
bdb_large_sectors_count: DD 0

ebr_drive_number: DB 0
                  DB 0
ebr_signature:    DB 29h
ebr_volumn_id:    DB 12h,34h,56h,78h
ebr_volumn_label: DB 'OLBOS      '
ebr_system_id     DB 'FAT12   '

main:
cli
    MOV ax,0
    MOV ss,ax
    MOV ds,ax
    MOV es,ax
    MOV sp,0x00007000
    MOV [ebr_drive_number],dl
sti
    ;enable the A20 line
    in al,0x92
    or al,00000010b
    out 0x92,al

    ;MOV si,mbr_asm_load_msg
    ;CALL print
     CALL load_stage2
    ;MOV si,stage2_asm_load_msg
    ;CALL print
    JMP stage2_load_segment:stage2_load_offset

halt:
    JMP halt

load_stage2:

;step 1: get the lba of the root directory
MOV ax,[bdb_sectors_per_fat]
MOV bx,[bdb_fat_count]
MUL bx
ADD ax,[bdb_reserved_sectors]
PUSH ax; the value ax holds is 19


;step 2: determine the size of the directory (how many sector its madeup of)
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
   ;MOV si,read_dir_faliure_msg
    ;CALL print
    JMP halt

    success_loading_rootdir_in_mem:
    ;MOV si,read_dir_success_msg
    ;CALL print
   
    XOR bx,bx
    MOV di,buffer
    CLD


;step 4 :search for the stage2 in the root directory that has been loaded into memory
search_stage2:
    MOV si,stage2_bin_name
    MOV cx,11
    
    PUSH di;save di cause CMPSB changes DI After comparing DI +=11 
    REPE CMPSB
    POP di
    JE found_stage2
    ADD di,32
    INC bx
    CMP bx,[bdb_dir_entries_count]
    JL search_stage2
    JMP stage2_not_found

stage2_not_found:
   ;MOV si,stage2_not_found_msg
   ;CALL print
    HLT
    JMP halt

found_stage2:
    MOV ax,[di +26]
    MOV [stage2_cluster],ax

    ;set the stage to load the fat table into memory
    mov ax,[bdb_sectors_per_fat]
    mov [sector_count],al
    MOV ax,[bdb_reserved_sectors]
    MOV bx,buffer
    CALL read_disk
    JC fail_disk_read

    ;now set the stage load the stage2 into memory
    MOV bx,stage2_load_segment
    MOV es,bx
    MOV bx ,stage2_load_offset

    load_stage2_loop:
    MOV ax,[stage2_cluster]
    ADD ax,31
    mov byte [sector_count],1
    call read_disk;read the current sector


   ;  MOV di,bx
    ; MOV cx,300

    ;print_11:
    ;MOV ah,0x0E
    ;MOV al,es:[di]
    ;INT 0x10
    ;INC di
    ;LOOP print_11


    ADD bx,[bdb_byte_per_sector]
    MOV ax,[stage2_cluster];Following the FAT12 cluster chain
    MOV cx,3
    MUL cx
    MOV cx,2
    DIV cx; (current stage2 cluster * 3)/2

    ;set up our buffer
    MOV si,buffer
    ADD si,ax
    MOV ax,[ds:si]
    OR dx,dx
    JZ even

odd:
    SHR ax,4;You shift right because odd FAT12 entries start 4 bits into the two-byte value.
    JMP next_cluster_after

even:
    AND ax,0x0FFF; remove four upper bit

next_cluster_after:
    ;check to see if weve gotten to the end of the last fat entry entry of stage2 file
    CMP ax,0xFF8
    JAE read_finish
    MOV [stage2_cluster],ax
    JMP load_stage2_loop

read_finish:
    ;finished loading stage2
    RET

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
; MOV si,done_read_m
;CALL print
  RET

    buffer equ 0x7E00
    stage2_load_segment EQU 0x1000
    stage2_load_offset EQU 0
    sector_count DB 0
    root_dir_sector_count DB 0
    stage2_cluster: DW 0
    stage2_bin_name: DB "STAGE2  BIN"
    mbr_asm_load_msg: DB "BootLD",0x0D,0x0A,0
    stage2_not_found_msg: DB "stg2 not found",0x0D,0x0A,0
    stage2_asm_load_msg: DB "stg2 loaded",0x0D,0x0A,0
    general_read_faliure_msg: DB "disk Read fail",0x0D,0x0A,0
    read_dir_success_msg: DB "S",0x0D,0x0A,0
    read_dir_faliure_msg: DB "F",0x0D,0x0A,0
    done_read_m: DB "disk read done",0x0D,0x0A,0

TIMES 510 -($-$$) DB 0
DW 0xAA55