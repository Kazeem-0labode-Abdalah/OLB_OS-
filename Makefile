# Variables
BUILD_DIR := build

USER_P := user_programs

SRC_DIR := src

ASM := nasm

COMPILER := i686-elf-gcc

OBJCOPY := i686-elf-objcopy

CFLAGS = -m32 -ffreestanding -fno-pic -nostdlib -nostartfiles -Wall -Wextra -Isrc

LINKER := i686-elf-ld

LINKER_LD := linker.ld

ASM_FLAGS := -f elf32


# Default target

all: clean always floppy_image bootloader stage2 kernel user_program1 user_program2


# Floppy image

floppy_image: $(BUILD_DIR)/main.img

$(BUILD_DIR)/main.img: bootloader stage2 kernel user_program1 user_program2
	dd if=/dev/zero of=$(BUILD_DIR)/main.img bs=512 count=2880
	mkfs.fat -F 12 -n "OLBOS" $(BUILD_DIR)/main.img
	dd if=$(BUILD_DIR)/bootloader.bin of=$(BUILD_DIR)/main.img conv=notrunc
	mcopy -i $(BUILD_DIR)/main.img $(BUILD_DIR)/kernel.bin ::KERNEL.BIN
	mcopy -i $(BUILD_DIR)/main.img $(BUILD_DIR)/stage2.bin ::STAGE2.BIN
	mcopy -i $(BUILD_DIR)/main.img $(BUILD_DIR)/user_programs/pgram1/pgram1.bin ::PGRAM1.BIN
	mcopy -i $(BUILD_DIR)/main.img $(BUILD_DIR)/user_programs/pgram2/pgram2.bin ::PGRAM2.BIN
	#qemu-system-i386 -machine isapc -m 32M -drive format=raw,file=build/main.img

# Bootloader

bootloader: $(BUILD_DIR)/bootloader.bin

$(BUILD_DIR)/bootloader.bin:
	$(ASM) $(SRC_DIR)/Bootloader/boot.asm -f bin -o $(BUILD_DIR)/bootloader.bin


# Stage2

stage2: $(BUILD_DIR)/stage2.bin

$(BUILD_DIR)/stage2.bin:
	$(ASM) $(SRC_DIR)/Bootloader/stage2.asm -f bin -o $(BUILD_DIR)/stage2.bin


# Kernel

kernel: $(BUILD_DIR)/kernel.bin

$(BUILD_DIR)/kernel.bin:

	$(ASM) $(ASM_FLAGS) -o $(BUILD_DIR)/kernel/asm/main.o $(SRC_DIR)/Kernel/kernel.asm

	$(ASM) $(ASM_FLAGS) -o $(BUILD_DIR)/kernel/asm/idt.o $(SRC_DIR)/Interupts/idt.asm

	$(ASM) $(ASM_FLAGS) -o $(BUILD_DIR)/kernel/asm/vmm.o $(SRC_DIR)/VMM/vmm.asm

	$(ASM) $(ASM_FLAGS) -o $(BUILD_DIR)/kernel/asm/syscalls.o $(SRC_DIR)/syscalls/syscalls.asm

	$(COMPILER) $(CFLAGS) -c $(SRC_DIR)/Kernel/kernel.c -o $(BUILD_DIR)/kernel/c/main.o

	$(COMPILER) $(CFLAGS) -c $(SRC_DIR)/Interupts/idt.c -o $(BUILD_DIR)/kernel/c/idt.o

	$(COMPILER) $(CFLAGS) -c $(SRC_DIR)/Timer/Timer.c -o $(BUILD_DIR)/kernel/c/Timer.o

	$(COMPILER) $(CFLAGS) -c $(SRC_DIR)/Keyboard/keyboard.c -o $(BUILD_DIR)/kernel/c/keyboard.o

	$(COMPILER) $(CFLAGS) -c $(SRC_DIR)/PMM/pmm.c -o $(BUILD_DIR)/kernel/c/pmm.o

	$(COMPILER) $(CFLAGS) -c $(SRC_DIR)/VMM/vmm.c -o $(BUILD_DIR)/kernel/c/vmm.o

	$(COMPILER) $(CFLAGS) -c $(SRC_DIR)/syscalls/syscalls.c -o $(BUILD_DIR)/kernel/c/syscalls.o

	$(COMPILER) $(CFLAGS) -c $(SRC_DIR)/user_program_loader/upl.c -o $(BUILD_DIR)/kernel/c/upl.o

	$(COMPILER) $(CFLAGS) -c $(SRC_DIR)/utils/util.c -o $(BUILD_DIR)/kernel/c/util.o

	$(COMPILER) $(CFLAGS) -c $(SRC_DIR)/vga/vga.c -o $(BUILD_DIR)/kernel/c/vga.o

	$(LINKER) -T $(LINKER_LD) -o $(BUILD_DIR)/kernel.elf \
	$(BUILD_DIR)/kernel/asm/main.o \
	$(BUILD_DIR)/kernel/asm/idt.o \
	$(BUILD_DIR)/kernel/asm/vmm.o \
	$(BUILD_DIR)/kernel/asm/syscalls.o \
	$(BUILD_DIR)/kernel/c/main.o \
	$(BUILD_DIR)/kernel/c/idt.o \
	$(BUILD_DIR)/kernel/c/Timer.o \
	$(BUILD_DIR)/kernel/c/keyboard.o \
	$(BUILD_DIR)/kernel/c/pmm.o \
	$(BUILD_DIR)/kernel/c/vmm.o \
	$(BUILD_DIR)/kernel/c/syscalls.o \
	$(BUILD_DIR)/kernel/c/upl.o \
	$(BUILD_DIR)/kernel/c/util.o \
	$(BUILD_DIR)/kernel/c/vga.o

	$(OBJCOPY) -O binary $(BUILD_DIR)/kernel.elf $(BUILD_DIR)/kernel.bin


# User program 1

user_program1: $(BUILD_DIR)/user_programs/pgram1/pgram1.bin

$(BUILD_DIR)/user_programs/pgram1/pgram1.bin:

	$(COMPILER) $(CFLAGS) -c $(SRC_DIR)/user_programs/pgram1.c -o $(BUILD_DIR)/user_programs/pgram1/pgram1.o

	$(OBJCOPY) -O binary $(BUILD_DIR)/user_programs/pgram1/pgram1.o $(BUILD_DIR)/user_programs/pgram1/pgram1.bin


# User program 2

user_program2: $(BUILD_DIR)/user_programs/pgram2/pgram2.bin

$(BUILD_DIR)/user_programs/pgram2/pgram2.bin:

	$(COMPILER) $(CFLAGS) -c $(SRC_DIR)/user_programs/pgram2.c -o $(BUILD_DIR)/user_programs/pgram2/pgram2.o

	$(OBJCOPY) -O binary $(BUILD_DIR)/user_programs/pgram2/pgram2.o $(BUILD_DIR)/user_programs/pgram2/pgram2.bin


# Create directories

always:
	mkdir -p $(BUILD_DIR)
	mkdir -p $(BUILD_DIR)/kernel
	mkdir -p $(BUILD_DIR)/kernel/asm
	mkdir -p $(BUILD_DIR)/kernel/c
	mkdir -p $(BUILD_DIR)/user_programs
	mkdir -p $(BUILD_DIR)/user_programs/pgram1
	mkdir -p $(BUILD_DIR)/user_programs/pgram2


# Clean

.PHONY: all bootloader stage2 kernel user_program1 user_program2 clean floppy_image always

clean:
	rm -rf $(BUILD_DIR)/*