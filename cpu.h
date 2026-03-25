#ifndef __CPU_H__
#define __CPU_H__

#include <stdio.h>
#include <stdlib.h>

#define ZERO (unsigned char) 7
#define SUBTRACTION (unsigned char) 6
#define HALF_CARY (unsigned char) 5
#define CARRY (unsigned char) 4

#define CLOCK_SPEED 4194304
#define TIMA 0xFF05
#define TMA 0xFF06
#define TMC 0xFF07


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
    unsigned char interrupts;
    unsigned short sp;
    unsigned short pc;
    int machine_cycles;

} Registers;

extern Registers registers;
extern unsigned char current_opcode;

void set_flag(unsigned char flag);
void unset_flag(unsigned char flag);
void clear_flags();
unsigned char toggle_flag(unsigned char flag);
unsigned char get_flag(unsigned char flag);
unsigned char memory_read_pc_byte();
unsigned short memory_read_pc_word();

int half_carry_addition_8bit(unsigned char b1, unsigned char b2);
int half_carry_addition_16bit(unsigned short w1, unsigned short w2);
int half_carry_subtraction_8bit(unsigned char b1, unsigned char b2);
int half_carry_subtraction_16bit(unsigned short w1, unsigned short w2);

int carry_addition_8bit(unsigned char b1, unsigned char b2);
int carry_addition_16bit(unsigned short w1, unsigned short w2);
int carry_subtraction_8bit(unsigned char b1, unsigned char b2);
int carry_subtraction_16bit(unsigned short w1, unsigned short w2);

void add_cycles(int cycles);
void unimplemented_cb_instruction(unsigned char code);
void unimplemented_instruction(unsigned char code);
void cb_prefix();
int execute_opcode();



void cb_swap_a();
void cb_bit_0_c();


#endif