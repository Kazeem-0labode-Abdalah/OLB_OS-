#pragma once
#include "stdint.h"

#define COLOUR_BLACK 0
#define COLOUR_LIGHT_GREY 7

#define width 80
#define height 25

void print(const char *s);
void scrollUp();
void newLine();
void reset();
