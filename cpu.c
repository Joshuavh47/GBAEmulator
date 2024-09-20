#include "cpu.h"

Registers registers;

void set_flag(unsigned char flag){ //sets a flag
    registers.f|=0x1<<flag;
}

void unset_flag(unsigned char flag){ //unsets a flag
    registers.f&=~(0x1<<flag);
}

void clear_flags(){ //clears all flags
    registers.f&=0x0F;
}

unsigned char toggle_flag(unsigned char flag){ //toggles a flag
    if((registers.f)>>flag&&0x01){
        unset_flag(flag);
        return 0x1;
    }
    else{
        set_flag(flag);
        return 0x0;
    }
}

unsigned char get_flag(unsigned char flag){ //check the value of a flag
    return registers.f>>flag&&0x1;
}

int main(int argc, char *argv[]){
    set_flag(ZERO);
    set_flag(CARRY);
    registers.a = 0x12;
    printf("%x\n", registers.f);
    printf("%x\n", registers.af);
    clear_flags();
    printf("%x\n", registers.af);
    return 0;
}