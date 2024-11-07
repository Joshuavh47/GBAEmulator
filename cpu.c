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
    return rom_bank[registers.pc];    
}

inline unsigned short memory_read_pc_word(){
    unsigned char b1 = rom_bank[registers.pc];
    unsigned char b2 = rom_bank[registers.pc+1];
    return (b2<<8)|b1;

}


inline void jmp_a16(){
    registers.pc = memory_read_pc_word();
    //printf("Jump to %X\n", registers.pc);
    add_cycles(12);
}

inline void jmp_nz_r8(){
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

inline void cp_d8(){
    unsigned char other_byte = memory_read_pc_byte();
    printf("A: %#X D8: %#X\n", registers.a, other_byte);
    registers.pc++;
    signed short result = registers.a - other_byte;
    registers.a == other_byte ? set_flag(ZERO) : unset_flag(ZERO);
    ~(registers.a ^ other_byte ^ result) ? set_flag(HALF_CARY) : unset_flag(HALF_CARY);
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

inline void ldh_a8_a(){
    unsigned char offset = memory_read_pc_byte();
    mem_write_byte(0xFF00 + offset, registers.a);
    registers.pc++;
    add_cycles(12);
}

inline void ldh_a_a8(){
    unsigned char offset = memory_read_pc_byte();
    registers.a = 0xFF00 + offset;

    if(registers.pc == 0x234){ // TEMPORARY PATCH BECAUSE I HAVE NO INTERRUPTS/TIMER/VIDEO/AUDIO
        registers.a = 0x94;
    }

    registers.pc++;
    add_cycles(12);

}

inline void ldh_indirect_c_a(){
    mem_write_byte(registers.c | 0xFF00, registers.a); // write register a to address stored in register c | 0xFF00
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

inline void call_a16(){
    unsigned short addr = memory_read_pc_word();
    registers.pc += 2;
    stack_write_word(memory_read_pc_word());
    registers.pc = addr;
    add_cycles(24);
}

inline void inc_c(){
    unsigned short result = registers.c + 1;
    ~(registers.c ^ 0x1 ^ result) ? set_flag(HALF_CARY) : unset_flag(HALF_CARY);
    registers.c++;
    registers.c ? unset_flag(ZERO) : set_flag(ZERO);
    unset_flag(SUBTRACTION);
    add_cycles(4);
}

inline void dec_b(){
    unsigned short result = registers.b - 0x1;
    ~(registers.b ^ 0x1 ^ result) ? set_flag(HALF_CARY) : unset_flag(HALF_CARY);
    --registers.b == 0 ? set_flag(ZERO) : unset_flag(ZERO);
    set_flag(SUBTRACTION);
    printf("B: %#X\n", registers.b);
    add_cycles(4);
}

inline void dec_c(){
    unsigned short result = registers.c - 0x1;
    ~(registers.c ^ 0x1 ^ result) ? set_flag(HALF_CARY) : unset_flag(HALF_CARY);
    --registers.c == 0 ? set_flag(ZERO) : unset_flag(ZERO);
    set_flag(SUBTRACTION);
    add_cycles(4);
}

inline void dec_bc(){
    registers.bc--;
    add_cycles(8);
}

inline void or_c(){
    registers.a |= registers.c;
    clear_flags();
    !registers.a ? set_flag(ZERO) : unset_flag(ZERO);
    add_cycles(4);
}

inline void ret(){
    registers.pc = stack_pop_word();
    add_cycles(16);
}

inline void di(){
    registers.interupts = 0;
    add_cycles(4);
}

inline void add_cycles(int cycles){
    registers.machine_cycles += cycles;
}

void unimplemented_instruction(unsigned char code){
    printf("%s 0x%02X\n", "Unimplemented opcode: ", code);
    exit(1);
}

inline int execute_opcode(){

    
    unsigned char current_opcode = memory_read_pc_byte();
    
    printf("PC: %#X Inst: %#X ", registers.pc, current_opcode);

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
        case 0x20:
            jmp_nz_r8();
            break;
        case 0x21:
            ld_hl_d16();
            break;
        case 0x2A:
            ld_a_hl_inc();
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
        case 0x78:
            ld_a_b();
            break;
        case 0xAF:
            xor_a();
            break;
        case 0xB1:
            or_c();
            break;
        case 0xC3:
            jmp_a16();
            break;
        case 0xC9:
            ret();
            break;
        case 0xCD:
            call_a16();
            break;
        case 0xE0:
            ldh_a8_a();
            break;
        case 0xE2:
            ldh_indirect_c_a();
            break;
        case 0xEA:
            ld_indirect_a16_a();
            break;
        case 0xF0:
            ldh_a_a8();
            break;
        case 0xF3:
            di();
            break;
        case 0xFE:
            cp_d8();
            break;
        default:
            unimplemented_instruction(current_opcode);
            break;


    }
    printf("A: %#X MC: %d\n", registers.a, registers.machine_cycles);
    

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
    while(1){
        
        execute_opcode();
        
        
    }

}
