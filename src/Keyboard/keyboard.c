#include "utils/util.h"
#include "Interupts/idt.h"
#include "stdint/stdint.h"
#include "vga/vga.h"

bool capsOn;
bool capsLock;


const char uppercase[89] = {
  '!','!', '1','2','3','4','5','6','7','8','9','0','-','=', '!', // 0-14
  '!', 'Q','W','E','R','T','Y','U','I','O','P','[',']', '!', '!', // 15-29
  'A','S','D','F','G','H','J','K','L',';','\'','`', 0, '\\', // 30-43
  'Z','X','C','V','B','N','M',',','.','/', '!', '*', '!', ' ', // 44-57
  '!', '!', '!', '!', '!', '!', '!', '!', '!', '!', '!', '!', '!', '!', '!', '!', // 58-73 F-keys + keypad
  '!', '!', '!', '!', '!', '!', '!', '!', '!', '!', '!', '!', '!', '!', '!' // 74-88
};


const char lowercase[89] = {
  '!','!', '1','2','3','4','5','6','7','8','9','0','-','=', '!', // 0-14
  '!', 'q','w','e','r','t','y','u','i','o','p','[',']', '!', '!', // 15-29
  'a','s','d','f','g','h','j','k','l',';','\'','`', 0, '\\', // 30-43
  'z','x','c','v','b','n','m',',','.','/', '!', '*', '!', ' ', // 44-57
  '!', '!', '!', '!', '!', '!', '!', '!', '!', '!', '!', '!', '!', '!', '!', '!', // 58-73 F-keys + keypad
  '!', '!', '!', '!', '!', '!', '!', '!', '!', '!', '!', '!', '!', '!', '!' // 74-88
};



void irq1_OnKeyboard(struct InterruptRegisters * reg){
    uint8_t data = importB(0x60);
    uint8_t scan_code = data & 0x7F;
    uint8_t pressed = data & 0x80;

    switch (scan_code)
    {
        // case 1:
        // case 29:
        // case 56:
        // case 59:
        // case 60:
        // case 61:
        // case 62:
        // case 63:
        // case 64:
        // case 65:
        // case 66:
        // case 67:
        // case 68:
        // case 87:
        // case 88:
        // break;
        case 42://left shift
            if(pressed==0){
                capsOn = true;
            }else{
                capsOn = false;
            }
        break;
         case 54://right shift
            if(pressed==0){
                capsOn = true;
            }else{
                capsOn = false;
            }
        break;
        case 58://caplock
            if(capsLock && pressed == 0){
                capsLock = false;
            }else if(!capsLock && pressed == 0){
                capsLock = true;
            }
        break;
    
    default:
            if(capsOn || capsLock){
                print(&uppercase[scan_code]);
            }else{
                print(&lowercase[scan_code]);
            }
        break;
    }

     outPortB(0x20,0x20);

}
void initKeyboard(){
    capsLock = false;
    capsOn = false;
    irq_installer(1,&irq1_OnKeyboard);
};