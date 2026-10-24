#include "stdint.h"
#include "vga/vga.h"

uint16_t row = 0;
uint16_t column = 0;
uint16_t *vga = (uint16_t *)0xB8000;
uint8_t default_colour = 0x47;


void reset(){
    row = 0;
    column = 0;
    for(uint16_t current_row = 0; current_row < height; current_row++){
        for(uint16_t current_column = 0; current_column < width; current_column++ ){
            vga[current_row * width + current_column] = (default_colour << 8) |' ';
        }
    }
}

void scrollUp(){
        for(uint16_t current_row = 1; current_row < height; current_row++){
        for(uint16_t current_column = 0; current_column < width; current_column++ ){
            vga[(current_row - 1) * width + current_column] =  vga[current_row * width + current_column];
        }
    }

    for(uint16_t current_column = 0; current_column < width; current_column++ ){
         vga[(height - 1) * width + current_column]  =  (default_colour << 8) |' ';
    }
}

void newLine(){
    if(row < height -1){
        row++;
        column = 0;
    }else{
        scrollUp();
    }
}
void print(const char *s){
     while(*s){

        switch (*s)
        {
        case '\n':
            newLine();
            break;

        case '\r' :
            column = 0;
        break;

        case '\b' :
            if(column == 0 && row !=0){
                row --;
                column = width;
            }
            vga[row * width + (--column)] =(default_colour << 8) |' ';
        break;
        
         case '\t' :
            if(column ==  width){
                newLine();
            }
            uint16_t tablen = 4 -(column % 4);
            while (tablen != 0)
            {
                vga[row * width + (column++)] = (default_colour << 8) |' ' ;
                tablen --;
            }
            break;
        
        default:
            if(column == width){
                newLine();
            }
            vga[row * width +(column++)] = (default_colour << 8) | *s ;
            break;
        }

        s++; 
    }
    
}
