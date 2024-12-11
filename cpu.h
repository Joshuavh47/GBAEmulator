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

void cp_d8();

void jmp_a16();
void jmp_hl();
void jr_r8();
void jr_nz_r8();
void jr_nc_r8();
void jr_z_r8();

void xor_a();
void xor_c();

void ld_a_d8();
void ld_c_d8();
void ld_b_d8();
void ld_a_b();
void ld_a_c();
void ld_b_a();
void ld_b_c();
void ld_c_a();
void ld_d_a();
void ld_e_a();
void ldi_indirect_hl_a();
void ldd_hl_a();
void ld_hl_d16();
void ld_d_d8();
void ldh_a8_a();
void ldh_a_a8();
void ldh_indirect_c_a();
void ld_a_indirect_de();
void ld_d_indirect_hl();
void ld_e_indirect_hl();
void ld_indirect_hl_d8();
void ld_indirect_a16_a();
void ld_de_d16();
void ld_sp_d16();
void ld_indirect_a16_sp();
void ld_a_hl_inc();
void ld_bc_d16();
void ld_a_indirect_a16();

void call_a16();

void cpl();

void and_a();
void and_c();
void and_d8();
void or_b();
void or_c();

void add_a_a();
void add_a_d();
void add_hl_de();
void inc_a();
void inc_c();
void inc_e();
void inc_l();
void inc_hl();
void inc_indirect_hl();

void dec_a();
void dec_b();
void dec_c();
void dec_bc();

void push_af();
void push_bc();
void push_de();
void push_hl();

void pop_af();
void pop_bc();
void pop_de();
void pop_hl();

void ret();
void reti();
void ret_nz();
void ret_z();

void rst_28h();
void rst_38h();

void ei();
void di();

void cb_swap_a();
void cb_bit_0_c();


#endif