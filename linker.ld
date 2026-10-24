ENTRY(_start)

SECTIONS
{
    . = 0x00100000;

    .multiboot ALIGN(4K) : {
        *(.multiboot)
    }

    .boot ALIGN(4K) : {
        *(.boot)
    }

    .text ALIGN(4K) : {
        *(.text)
    }

    .rodata ALIGN(4K) : {
        *(.rodata)
    }

    .data ALIGN(4K) : {
        *(.data)
    }

    .bss ALIGN(4K) : {
        *(COMMON)
        *(.bss)
    }

    _kernel_end = .;
}