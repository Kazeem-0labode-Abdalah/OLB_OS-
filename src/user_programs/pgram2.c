#include "syscalls/syscalls.h"
#include "stdint/stdint.h"

char msg[] = "user_program2_is_running";


void _start(void)
{
    while (1) {
        write(1, msg, 24);
    }
}


