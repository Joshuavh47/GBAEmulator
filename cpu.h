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
    unsigned char interupts;
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
void add_cycles(int cycles);
void unimplemented_instruction(unsigned char code);
int execute_opcode();

void cp_d8();

void jmp_a16();
void jmp_nz_r8();

void xor_a();

void ld_a_d8();
void ld_c_d8();
void ld_b_d8();
void ld_a_b();
void ldd_hl_a();
void ld_hl_d16();
void ldh_a8_a();
void ldh_a_a8();
void ldh_indirect_c_a();
void ld_indirect_hl_d8();
void ld_indirect_a16_a();
void ld_sp_d16();
void ld_a_hl_inc();
void ld_bc_d16();

void call_a16();

void or_c();

void inc_c();

void dec_b();
void dec_c();
void dec_bc();

void ret();

void di();


#endif