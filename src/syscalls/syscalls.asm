global asm_write
asm_write:
    push ebp;
    MOV ebp,esp

    MOV eax,[ebp + 8];fd
    MOV ebx,[ebp + 12];buffer
    MOV ecx,[ebp + 16];len

    int 0x80;
    pop ebx
    RET