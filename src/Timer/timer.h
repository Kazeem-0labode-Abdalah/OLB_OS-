#ifndef TIMER_H
#define TIMER_H

#include "utils/util.h"
void initTimer();
void irq0_Ontimer(struct InterruptRegisters * reg);
#endif