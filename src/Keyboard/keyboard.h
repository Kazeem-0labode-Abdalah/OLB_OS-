#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "utils/util.h"
#include "Interupts/idt.h"

void irq1_OnKeyboard(struct InterruptRegisters * reg);
void initKeyboard();

#endif