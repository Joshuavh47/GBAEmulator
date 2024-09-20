#ifndef __CPU_H__
#define __CPU_H__

#include <stdio.h>
#include <stdlib.h>

#define ZERO (unsigned char) 7
#define SUBTRACTION (unsigned char) 6
#define HALF_CARY (unsigned char) 5
#define CARRY (unsigned char) 4

/*
* Used anonymous structs and tagged unions to implement combined registers. 
* This allows for registers to be accessed explicitly instead of in chains, 
* allowing for easier readability. 
* Ex: registers.a and registers.af instead of registers->AF.b.a to access
*     higher and lower parts of the combined register.
*/
typedef struct Registers{ //Gameboy Registers
    struct{
        union{
            struct{
                unsigned char f;
                unsigned char a;
            };
            unsigned short af;
        };
    };
    struct{
        union{
            struct{
                unsigned char c;
                unsigned char b;
            };
            unsigned short bc;
        };
    };
    struct{
        union{
            struct{
                unsigned char e;
                unsigned char d;
            };
            unsigned short de;
        };
    };
    struct{
        union{
            struct{
                unsigned char l;
                unsigned char h;
            };
            unsigned short hl;
        };
    };

    unsigned short sp;
    unsigned short pc;

} Registers;

extern Registers registers;

void set_flag(unsigned char flag);
void unset_flag(unsigned char flag);
void clear_flags();
unsigned char toggle_flag(unsigned char flag);
unsigned char get_flag(unsigned char flag);

#endif