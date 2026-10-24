// Complete interrupt flow using a keyboard interupt example

// When you press a key, the sequence is:

// Keyboard

// ↓

// IRQ1

// ↓

// PIC

// ↓

// Interrupt 33

// ↓

// CPU

// ↓

// IDT Entry 33

// ↓

// irq1 (assembly)

// ↓

// irq_common_stub

// ↓

// irq_handler()

// ↓

// keyboard_handler()

// ↓

// Send EOI to PIC

// ↓

// iret

// ↓

// CPU resumes the interrupted program

#include "idt.h"
#include "utils/util.h"
#include "vga/vga.h"
#include "syscalls/syscalls.h"

struct idt_entry_struct idt_entries[AMOUNT_OF_IDT_ENTRIES];
struct idt_ptr_struct idt_entries_ptr;
extern void flush_idt(uint32_t);

void initIdt(){
   //  print("Idt is being initiated\n");
    idt_entries_ptr.limit = (uint16_t)sizeof(idt_entries[0]) * AMOUNT_OF_IDT_ENTRIES -1;
    idt_entries_ptr.base = (uint32_t)&idt_entries;
    memset(&idt_entries,0,sizeof(idt_entries));

   //some programming for programable interface computer

    outPortB(0x20,0x11);
    outPortB(0xA0,0x11);

    outPortB(0x21,0x20);
    outPortB(0xA1,0x28);

    outPortB(0x21,0x04);
    outPortB(0xA1,0x02);

    outPortB(0x21,0x01);
    outPortB(0xA1,0x01);

    outPortB(0x21,0x0);
    outPortB(0xA1,0x0);


    //cpu interupts
    setIdtGate(0,(uint32_t)isr0,0x08,0x8E);
    setIdtGate(1,(uint32_t)isr1,0x08,0x8E);
    setIdtGate(2,(uint32_t)isr2,0x08,0x8E);
    setIdtGate(3,(uint32_t)isr3,0x08,0x8E);
    setIdtGate(4,(uint32_t)isr4,0x08,0x8E);
    setIdtGate(5,(uint32_t)isr5,0x08,0x8E);
    setIdtGate(6,(uint32_t)isr6,0x08,0x8E);
    setIdtGate(7,(uint32_t)isr7,0x08,0x8E);
    setIdtGate(8,(uint32_t)isr8,0x08,0x8E);
    setIdtGate(9,(uint32_t)isr9,0x08,0x8E);
    setIdtGate(10,(uint32_t)isr10,0x08,0x8E);
    setIdtGate(11,(uint32_t)isr11,0x08,0x8E);
    setIdtGate(12,(uint32_t)isr12,0x08,0x8E);
    setIdtGate(13,(uint32_t)isr13,0x08,0x8E);
    setIdtGate(14,(uint32_t)isr14,0x08,0x8E);
    setIdtGate(15,(uint32_t)isr15,0x08,0x8E);
    setIdtGate(16,(uint32_t)isr16,0x08,0x8E);
    setIdtGate(17,(uint32_t)isr17,0x08,0x8E);
    setIdtGate(18,(uint32_t)isr18,0x08,0x8E);
    setIdtGate(19,(uint32_t)isr19,0x08,0x8E);
    setIdtGate(20,(uint32_t)isr20,0x08,0x8E);
    setIdtGate(21,(uint32_t)isr21,0x08,0x8E);
    setIdtGate(22,(uint32_t)isr22,0x08,0x8E);
    setIdtGate(23,(uint32_t)isr23,0x08,0x8E);
    setIdtGate(24,(uint32_t)isr24,0x08,0x8E);
    setIdtGate(25,(uint32_t)isr25,0x08,0x8E);
    setIdtGate(26,(uint32_t)isr26,0x08,0x8E);
    setIdtGate(27,(uint32_t)isr27,0x08,0x8E);
    setIdtGate(28,(uint32_t)isr28,0x08,0x8E);
    setIdtGate(29,(uint32_t)isr29,0x08,0x8E);
    setIdtGate(30,(uint32_t)isr30,0x08,0x8E);
    setIdtGate(31,(uint32_t)isr31,0x08,0x8E);
    //for interupt request from hadware
     setIdtGate(32,(uint32_t)timer_irq0,0x08,0x8E);
     setIdtGate(33,(uint32_t)irq1,0x08,0x8E);
     setIdtGate(34,(uint32_t)irq2,0x08,0x8E);
     setIdtGate(35,(uint32_t)irq3,0x08,0x8E);
     setIdtGate(36,(uint32_t)irq4,0x08,0x8E);
     setIdtGate(37,(uint32_t)irq5,0x08,0x8E);
     setIdtGate(38,(uint32_t)irq6,0x08,0x8E);
     setIdtGate(39,(uint32_t)irq7,0x08,0x8E);
     setIdtGate(40,(uint32_t)irq8,0x08,0x8E);
     setIdtGate(41,(uint32_t)irq9,0x08,0x8E);
     setIdtGate(42,(uint32_t)irq10,0x08,0x8E);
     setIdtGate(43,(uint32_t)irq11,0x08,0x8E);
     setIdtGate(44,(uint32_t)irq12,0x08,0x8E);
     setIdtGate(45,(uint32_t)irq13,0x08,0x8E);
     setIdtGate(46,(uint32_t)irq14,0x08,0x8E);
    //for system calls note that  in flag bit 5-6 should be 11 so ring 3 can call 0xEE
    setIdtGate(128,(uint32_t)isr128,0x08,0x8E | 0x60);
    setIdtGate(177,(uint32_t)isr177,0x08,0x8E | 0x60);

    flush_idt((uint32_t)&idt_entries_ptr);
}

void setIdtGate(uint8_t num,uint32_t base,uint16_t sel,uint8_t flags){
    idt_entries[num].base_low =  base & 0xFFFF;
    idt_entries[num].base_high = (base >> 16) & 0xFFFF;
    idt_entries[num].sel = sel;
    idt_entries[num].always0 = 0;
    idt_entries[num].flags = flags;
}

const char * cpu_interrupts_messages[] = {
     "Division by zero",
    "Debug Exception",
    "Non maskable interupt",
    "Breakpoint",
    "Into Detected overflow",
    "Out of bounds",
    "Invalid opcode",
    "Device not available",
    "Double fault",
    "Co processor segment overrun",
    "Invalid tss",
    "Segment not present",
    "Stack segment fault",
    "General protection fault",
    "Page fault",
    "Reserved",
    "X87 FPU Error",
    "Aligment check",
    "Machine check",
    "SIMD floating point Execption",
    "Virtualization exeption",
    "Control protection exception",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Hypervisor injection",
    "VMM communication on",
    "Security exception",
    "Reserved"
};



void isr_handler(struct InterruptRegisters * reg){
    if(reg-> int_no< 32){
        print("A cpu exception has occured, the exception is: ");
        print(cpu_interrupts_messages[reg->int_no]);
        print("\n");
        print("The system has halted\n");
        for(;;);
    }else if(reg->int_no == 128){
        //validate the user buffer addr making sure it not malicius(i'll do this later)
        uint32_t amount_of_bytes_written = 0;
        for(uint32_t i = 0 ; i < reg->ecx;i++){
            print((char*)(i + reg->ebx));
            amount_of_bytes_written++;
        }
       //how do i return amount_of_bytes_written back to the user stack
    }
    return;
};

void *irq_routines[16] ={
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};

void irq_installer(int irq_no,void(*handler)(struct InterruptRegisters * reg)){
    irq_routines[irq_no] = handler;
}

void irq_uninstaller(int irq_no){
    irq_routines[irq_no] = 0;
}

void irq_handler(struct InterruptRegisters * reg){
    void(*handler)(struct InterruptRegisters * reg);
    handler = irq_routines[reg->int_no - 32];
    if(handler){
        handler(reg);
    }
    if(reg->int_no > 30){
        outPortB(0xA0,0x20); //EOI to Slave PIC
        outPortB(0x20, 0x20); //EOI to Master PIC
    }

     outPortB(0x20,0x20);
     return;
}

