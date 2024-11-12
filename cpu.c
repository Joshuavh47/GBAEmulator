#include "cpu.h"
#include "rom.h"
#include "memory.h"

Registers registers;
unsigned char* rom_data;
long int rom_size;
unsigned char *rom_bank;
unsigned char *ram_bank;
unsigned char *io_bank;
unsigned char *high_ram;
unsigned char interrupt_register;

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

inline unsigned char memory_read_pc_byte(){
    return mem_read_byte(registers.pc);    
}

inline unsigned short memory_read_pc_word(){
    unsigned char b1 = rom_bank[registers.pc];
    unsigned char b2 = rom_bank[registers.pc+1];
    return (b2<<8)|b1;

}

inline int half_carry_addition_8bit(unsigned char b1, unsigned char b2){
    return (((b1 & 0x0F) + (b2 & 0x0F)) & 0x10) == 0x10;
}

inline int half_carry_addition_16bit(unsigned short w1, unsigned short w2){
    return (((w1 & 0x00FF) + (w2 & 0x00FF)) & 0x0100) == 0x0100;
}

inline int half_carry_subtraction_8bit(unsigned char b1, unsigned char b2){
    return (int)(b1 & 0x0F) - (int)(b2 & 0x0F) < 0;
}

inline int half_carry_subtraction_16bit(unsigned short w1, unsigned short w2){
    return (int)(w1 & 0x00FF) - (int)(w2 & 0x00FF) < 0;
}

inline int carry_addition_8bit(unsigned char b1, unsigned char b2){
    return (int)b1 + (int)b2 > 0xFF;
}

inline int carry_addition_16bit(unsigned short w1, unsigned short w2){
    return (int)w1 + (int)w2 > 0xFFFF;
}

inline int carry_subtraction_8bit(unsigned char b1, unsigned char b2){
    return (int)b1 - (int)b2 < 0;
}

inline int carry_subtraction_16bit(unsigned short w1, unsigned short w2){
    return (int)w1 - (int)w2 < 0;
}

inline void jmp_a16(){
    registers.pc = memory_read_pc_word();
    //printf("Jump to %X\n", registers.pc);
    add_cycles(12);
}

inline void jmp_hl(){
    registers.pc = registers.hl;
    add_cycles(4);
}

inline void jr_nz_r8(){
    signed char offset = memory_read_pc_byte();
    registers.pc++;
    if(get_flag(ZERO)){
        add_cycles(8);
    }
    else{
        registers.pc += offset;
        add_cycles(12);
    }
}

inline void jr_z_d8(){
    signed char offset = memory_read_pc_byte();
    registers.pc++;
    if(get_flag(ZERO)){
        registers.pc += offset;
        add_cycles(12);
    }
    else{
        add_cycles(8);
    }
}

inline void cp_d8(){
    unsigned char other_byte = memory_read_pc_byte();
    printf("A: %#X D8: %#X\n", registers.a, other_byte);
    registers.pc++;
    registers.a == other_byte ? set_flag(ZERO) : unset_flag(ZERO);
    half_carry_subtraction_8bit(registers.a, other_byte) ? set_flag(HALF_CARY) : unset_flag(HALF_CARY);
    registers.a < other_byte ? set_flag(CARRY) : unset_flag(CARRY);
    set_flag(SUBTRACTION);
    add_cycles(8);
}

inline void xor_a(){
    registers.a ^= registers.a;
    clear_flags();
    set_flag(ZERO);
    add_cycles(4);
}

inline void xor_c(){
    registers.a ^= registers.c;
    clear_flags();
    if(!registers.a){
        set_flag(ZERO);
    }
    add_cycles(4);
}

inline void ld_a_d8(){
    registers.a = memory_read_pc_byte();
    registers.pc++;
    add_cycles(8);
}

inline void ld_b_d8(){
    registers.b = memory_read_pc_byte();
    registers.pc++;
    add_cycles(8);
}

inline void ld_c_d8(){
    registers.c = memory_read_pc_byte();
    registers.pc++;
    add_cycles(8);
}

inline void ld_a_b(){
    registers.a = registers.b;
    add_cycles(4);
}

inline void ld_a_c(){
    registers.a = registers.c;
    add_cycles(4);
}

inline void ld_b_a(){
    registers.b = registers.a;
    add_cycles(4);
}

inline void ld_c_a(){
    registers.c = registers.a;
    add_cycles(4);
}

inline void ld_e_a(){
    registers.e = registers.a;
    add_cycles(4);
}

inline void ld_d_d8(){
    registers.d = memory_read_pc_byte();
    registers.pc++;
    add_cycles(8);
}

inline void ldh_a8_a(){
    unsigned char offset = memory_read_pc_byte();
    mem_write_byte(0xFF00 + offset, registers.a);
    registers.pc++;
    add_cycles(12);
}

inline void ldh_a_a8(){
    unsigned char offset = memory_read_pc_byte();
    registers.a = 0xFF00 + offset;

    /*
    * TEMPORARY PATCH BECAUSE I HAVE NO INTERRUPTS/TIMER/VIDEO/AUDIO            *
    * In this section of rom, the LCD Y coordinate gets loaded into a.          *
    * Since this increases automatically every scanline, the conditional jump   *
    * will always happen. Once I implement video, this will be taken out.       *
    */

    if(registers.pc == 0x234){  
        registers.a = 0x94;     
    }                           
                                
                                
                                

    registers.pc++;
    add_cycles(12);

}

inline void ldh_indirect_c_a(){
    mem_write_byte(registers.c | 0xFF00, registers.a); // write register a to address stored in register c | 0xFF00
    add_cycles(8);
}

inline void ld_d_indirect_hl(){
    registers.d = mem_read_byte(registers.hl);
    add_cycles(8);
}

inline void ld_e_indirect_hl(){
    registers.e = mem_read_byte(registers.hl);
    add_cycles(8);
}

inline void ld_hl_d16(){
    registers.hl = memory_read_pc_word();
    registers.pc += 2;
    add_cycles(12);
}

inline void ldd_hl_a(){ // do you set carry and hc with this instruction??
    registers.hl = registers.a;
    registers.hl--;
    add_cycles(8);
}

inline void ld_indirect_hl_d8(){
    mem_write_byte(registers.hl, memory_read_pc_byte());
    registers.pc++;
    add_cycles(12);
}

inline void ld_indirect_a16_a(){
    mem_write_byte(memory_read_pc_word(), registers.a);
    registers.pc += 2;
    add_cycles(16);
}

inline void ld_sp_d16(){
    registers.sp = memory_read_pc_word();
    registers.pc += 2;
    add_cycles(12);
}

inline void ld_a_hl_inc(){
    registers.a = mem_read_byte(registers.hl++);
    add_cycles(8);
}

inline void ld_bc_d16(){
    registers.bc = memory_read_pc_word();
    registers.pc += 2;
    add_cycles(12);
}

inline void ld_a_indirect_a16(){
    unsigned short addr = memory_read_pc_word();
    registers.pc += 2;
    registers.a = mem_read_byte(addr);
    add_cycles(16);
}

inline void call_a16(){
    unsigned short addr = memory_read_pc_word(); // get address of called function
    registers.pc += 2; // move pc past the address spot
    stack_write_word(registers.pc); // write this spot onto stack
    registers.pc = addr;
    printf("***CALL*** PC: %#06X\n\n", registers.pc);
    add_cycles(24);
}

inline void add_a_a(){
    clear_flags();
    if(carry_addition_8bit(registers.a, registers.a)){
        set_flag(CARRY);
    }
    if(half_carry_addition_8bit(registers.a, registers.a)){
        set_flag(HALF_CARY);
    }
    registers.a += registers.a;
    if(!registers.a){
        set_flag(ZERO);
    }
    add_cycles(4);
}

inline void add_hl_de(){
    unset_flag(SUBTRACTION);
    if(half_carry_addition_16bit(registers.hl, registers.de)){
        set_flag(HALF_CARY);
    }
    if(carry_addition_16bit(registers.hl, registers.de)){
        set_flag(CARRY);
    }
    registers.hl += registers.de;
    add_cycles(8);
}

inline void inc_c(){
    half_carry_addition_8bit(registers.c, 1) ? set_flag(HALF_CARY) : unset_flag(HALF_CARY);
    registers.c++;
    registers.c ? unset_flag(ZERO) : set_flag(ZERO);
    unset_flag(SUBTRACTION);
    add_cycles(4);
}

inline void inc_hl(){
    registers.hl++;
    add_cycles(8);
}

inline void dec_b(){
    half_carry_subtraction_8bit(registers.b, 1) ? set_flag(HALF_CARY) : unset_flag(HALF_CARY);
    --registers.b == 0 ? set_flag(ZERO) : unset_flag(ZERO);
    set_flag(SUBTRACTION);
    //printf("B: %#X\n", registers.b);
    add_cycles(4);
}

inline void dec_c(){
    half_carry_subtraction_8bit(registers.c, 1) ? set_flag(HALF_CARY) : unset_flag(HALF_CARY);
    --registers.c == 0 ? set_flag(ZERO) : unset_flag(ZERO);
    set_flag(SUBTRACTION);
    add_cycles(4);
}

inline void dec_bc(){
    registers.bc--;
    add_cycles(8);
}

inline void cpl(){
    registers.a = ~registers.a;
    set_flag(SUBTRACTION);
    set_flag(HALF_CARY);
    add_cycles(4);
}

inline void and_a(){
    clear_flags();
    set_flag(HALF_CARY);
    if(!registers.a){
        set_flag(ZERO);
    }
    add_cycles(4);
}

inline void and_c(){
    registers.a &= registers.c;
    clear_flags();
    set_flag(HALF_CARY);
    if(!registers.a){
        set_flag(ZERO);
    }
    add_cycles(4);
}

inline void and_d8(){
    registers.a &= memory_read_pc_byte();
    registers.pc++;
    unset_flag(CARRY);
    unset_flag(SUBTRACTION);
    set_flag(HALF_CARY);
    registers.a ? unset_flag(ZERO) : set_flag(ZERO);
    add_cycles(8);
}

inline void or_b(){
    registers.a |= registers.b;
    clear_flags();
    if(!registers.a){
        set_flag(ZERO);
    }
    add_cycles(4);
}

inline void or_c(){
    registers.a |= registers.c;
    clear_flags();
    if(!registers.a){
        set_flag(ZERO);
    }
    add_cycles(4);
}

inline void push_af(){
    stack_write_word(registers.af);
    add_cycles(16);
}

inline void push_bc(){
    stack_write_word(registers.bc);
    add_cycles(16);
}

inline void push_de(){
    stack_write_word(registers.de);
    add_cycles(16);
}

inline void push_hl(){
    stack_write_word(registers.hl);
    add_cycles(16);
}

inline void pop_hl(){
    registers.hl = stack_pop_word();
    add_cycles(12);
}

inline void ret(){
    registers.pc = stack_pop_word();
    printf("***RETURN*** PC: %04X\n\n", registers.pc);
    add_cycles(16);
}

inline void ret_nz(){
    if(!get_flag(ZERO)){
        registers.pc = stack_pop_word();
        printf("***RETURN*** PC: %04X\n\n", registers.pc);
        add_cycles(20);
    }
    else{
        add_cycles(8);
    }
}

inline void ret_z(){
    if(get_flag(ZERO)){
        registers.pc = stack_pop_word();
        printf("***RETURN*** PC: %04X\n\n", registers.pc);
        add_cycles(20);
    }
    else{
        add_cycles(8);
    }
}

inline void rst_28h(){
    stack_write_word(registers.pc);
    registers.pc = 0x28;
    add_cycles(16);
}

inline void ei(){
    registers.interupts = 1;
    add_cycles(4);
}

inline void di(){
    registers.interupts = 0;
    add_cycles(4);
}

inline void cb_swap_a(){
    registers.a = ((registers.a & 0xF0) >> 4) | ((registers.a & 0x0F) << 4); // Swap upper and lower nibbles of register A
    clear_flags();
    if(!registers.a){
        set_flag(ZERO);
    }
    add_cycles(8);
}

inline void add_cycles(int cycles){
    registers.machine_cycles += cycles;
}

void unimplemented_cb_instruction(unsigned char code){
    printf("%s %#04X\n", "Unimplemented CB opcode: ", code);
    exit(1);
}

void unimplemented_instruction(unsigned char code){
    printf("%s %#04X\n", "Unimplemented opcode: ", code);
    exit(1);
}

inline void cb_prefix(){
    unsigned char cb_opcode = memory_read_pc_byte();
    registers.pc++;
    switch (cb_opcode){
        case 0x37:
            cb_swap_a();
            break;
        default:
            unimplemented_cb_instruction(cb_opcode);
            break;
    }
}


inline int execute_opcode(){

    
    unsigned char current_opcode = memory_read_pc_byte();
    
    printf("PC: %#06X Inst: %#04X ", registers.pc, current_opcode);
    printf("A: %#06X B: %#06X  C: %#06X  D: %#06X  E: %#06X  F: %#06X  H: %#06X  L: %#06X MC: %d\n\n", registers.a, registers.b,
         registers.c, registers.d, registers.e, registers.f, registers.h, registers.l, registers.machine_cycles);

    //printf("PC:%X PC: %02X Flags: %X Interrupts: %X\n", registers.pc, current_opcode, registers.f, interrupt_register);
    registers.pc++;
    switch(current_opcode){
        case 0x00:
            add_cycles(4);
            break;
        case 0x01:
            ld_bc_d16();
            break;
        case 0x05:
            dec_b();
            break;
        case 0x06:
            ld_b_d8();
            break;
        case 0x0B:
            dec_bc();
            break;
        case 0x0C:
            inc_c();
            break;
        case 0x0D:
            dec_c();
            break;
        case 0x0E:
            ld_c_d8();
            break;
        case 0x16:
            ld_d_d8();
            break;
        case 0x19:
            add_hl_de();
            break;
        case 0x20:
            jr_nz_r8();
            break;
        case 0x21:
            ld_hl_d16();
            break;
        case 0x23:
            inc_hl();
            break;
        case 0x28:
            jr_z_d8();
            break;
        case 0x2A:
            ld_a_hl_inc();
            break;
        case 0x2F:
            cpl();
            break;
        case 0x31:
            ld_sp_d16();
            break;
        case 0x32:
            ldd_hl_a();
            break;
        case 0x36:
            ld_indirect_hl_d8();
            break;
        case 0x3E:
            ld_a_d8();
            break;
        case 0x47:
            ld_b_a();
            break;
        case 0x4F:
            ld_c_a();
            break;
        case 0x56:
            ld_d_indirect_hl();
            break;
        case 0x5E:
            ld_e_indirect_hl();
            break;
        case 0x5F:
            ld_e_a();
            break;
        case 0x78:
            ld_a_b();
            break;
        case 0x79:
            ld_a_c();
            break;
        case 0x87:
            add_a_a();
            break;
        case 0xA1:
            and_c();
            break;
        case 0xA7:
            and_a();
            break;
        case 0xA9:
            xor_c();
            break;
        case 0xAF:
            xor_a();
            break;
        case 0xB0:
            or_b();
            break;
        case 0xB1:
            or_c();
            break;
        case 0xC0:
            ret_nz();
            break;
        case 0xC3:
            jmp_a16();
            break;
        case 0xC5:
            push_bc();
            break;
        case 0xC8:
            ret_z();
            break;
        case 0xC9:
            ret();
            break;
        case 0xCB:
            cb_prefix();
            break;
        case 0xCD:
            call_a16();
            break;
        case 0xD5:
            push_de();
            break;
        case 0xE0:
            ldh_a8_a();
            break;
        case 0xE1:
            pop_hl();
            break;
        case 0xE2:
            ldh_indirect_c_a();
            break;
        case 0xE6:
            and_d8();
            break;
        case 0xE5:
            push_hl();
            break;
        case 0xE9:
            jmp_hl();
            break;
        case 0xEA:
            ld_indirect_a16_a();
            break;
        case 0xEF:
            rst_28h();
            break;
        case 0xF0:
            ldh_a_a8();
            break;
        case 0xF3:
            di();
            break;
        case 0xF5:
            push_af();
            break;
        case 0xFA:
            ld_a_indirect_a16();
            break;
        case 0xFB:
            ei();
            break;
        case 0xFE:
            cp_d8();
            break;
        default:
            unimplemented_instruction(current_opcode);
            break;


    }
    
    /* Interupt Handling */
    
    unsigned char interrupts_allowed = mem_read_byte(0xFFFF);
    unsigned char queued_interrupts = mem_read_byte(0xFF0F);
    unsigned char masked_interrupts = interrupts_allowed & queued_interrupts;

    if(registers.interupts){
        if(masked_interrupts & 0x01){
            registers.interupts = 0;
            stack_write_word(registers.pc);
            registers.machine_cycles += 20;
            registers.pc = 0x40;
        }
        if(masked_interrupts & 0x02){
            registers.interupts = 0;
            stack_write_word(registers.pc);
            registers.machine_cycles += 20;
            registers.pc = 0x48;
        }
        if(masked_interrupts & 0x04){
            registers.interupts = 0;
            stack_write_word(registers.pc);
            registers.machine_cycles += 20;
            registers.pc = 0x50;
        }
        if(masked_interrupts & 0x08){
            registers.interupts = 0;
            stack_write_word(registers.pc);
            registers.machine_cycles += 20;
            registers.pc = 0x58;
        }
        if(masked_interrupts & 0x10){
            registers.interupts = 0;
            stack_write_word(registers.pc);
            registers.machine_cycles += 20;
            registers.pc = 0x60;
        }
    }

    return 0;
}



int main(int argc, char *argv[]){
    
    // load_rom("/Users/joshuaeres/Downloads/Pokemon Red (UE) [S][!].gb");
    // init_memory();
    // while(1){
    //     execute_opcode();
        
    // }
    
    //return 0;
    
   
    // load_rom("/Users/joshuaeres/Downloads/Tetris (JUE) (V1.1) [!].gb");
    // init_memory();
    // for(int i=0;i<rom_size;i++){
        
    //     printf("%u:%02X ", registers.pc,rom_data[i]);

    // }
    load_rom("/Users/joshuaeres/Downloads/Tetris (JUE) (V1.1) [!].gb");
    init_memory();
    printf("%X\n", registers.pc);
    //void test();
    //test();
    //printf("%#02X\n", rom_bank[0xFFBA]);
    while(1){
        
        execute_opcode();
        
        
    }

}
