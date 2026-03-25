#include "cpu.h"
#include "rom.h"
#include "memory.h"
#include "vram.h"

Registers registers;
unsigned char* rom_data;
long int rom_size;
unsigned char *rom_bank;
unsigned char *ram_bank;
unsigned char *io_bank;
unsigned char *high_ram;
unsigned char interrupt_register;
unsigned short video_cycles = 0;
int didUpdate;

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
    if((registers.f >> flag) & 0x01){
        unset_flag(flag);
        return 0x1;
    }
    else{
        set_flag(flag);
        return 0x0;
    }
}

unsigned char get_flag(unsigned char flag){ //check the value of a flag
    return (registers.f >> flag) & 0x1;
}

inline unsigned char memory_read_pc_byte(){
    return mem_read_byte(registers.pc);    
}

inline unsigned short memory_read_pc_word(){
    unsigned char b1 = mem_read_byte(registers.pc);
    unsigned char b2 = mem_read_byte(registers.pc+1);
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


inline void cb_swap_a(){
    registers.a = ((registers.a & 0xF0) >> 4) | ((registers.a & 0x0F) << 4); // Swap upper and lower nibbles of register A
    clear_flags();
    if(!registers.a){
        set_flag(ZERO);
    }
    add_cycles(8);
}

inline void cb_bit_0_c(){
    registers.c & 0x1 ? set_flag(ZERO) : unset_flag(ZERO);
    unset_flag(SUBTRACTION);
    set_flag(HALF_CARY);
    add_cycles(8);
}

static inline void cb_zero_a(){
    registers.a &= 0xFE;
    add_cycles(2);
}

inline void add_cycles(int cycles){
    registers.machine_cycles += cycles;
    video_cycles += cycles;
}

void unimplemented_cb_instruction(unsigned char code){
    printf("%s %#04X\n", "Unimplemented CB opcode: ", code);
    exit(1);
}

void unimplemented_instruction(unsigned char code){
    printf("%s %#04X\n", "Unimplemented opcode: ", code);
    exit(1);
}


int execute_opcode(){

    
    

    static void* dispatch[256] = {
        // Default all opcodes to unimplemented
        [0 ... 255] = &&unimplemented_instruction,

        // --- Load / NOP ---
        [0x00] = &&op_00,   // NOP
        [0x01] = &&op_01,   // LD BC,d16
        [0x02] = &&unimplemented_instruction, // LD (BC),A
        [0x03] = &&unimplemented_instruction, // INC BC
        [0x04] = &&unimplemented_instruction, // INC B
        [0x05] = &&op_05,   // DEC B
        [0x06] = &&op_06,   // LD B,d8
        [0x07] = &&unimplemented_instruction, // RLCA
        [0x08] = &&op_08,   // LD (a16),SP
        [0x09] = &&unimplemented_instruction, // ADD HL,BC
        [0x0A] = &&unimplemented_instruction, // LD A,(BC)
        [0x0B] = &&op_0B,   // DEC BC
        [0x0C] = &&op_0C,   // INC C
        [0x0D] = &&op_0D,   // DEC C
        [0x0E] = &&op_0E,   // LD C,d8
        [0x0F] = &&unimplemented_instruction, // RRCA

        [0x10] = &&unimplemented_instruction, // STOP
        [0x11] = &&op_11,   // LD DE,d16
        [0x12] = &&op_12,   // LD (DE),A
        [0x13] = &&unimplemented_instruction, // INC DE
        [0x14] = &&unimplemented_instruction, // INC D
        [0x15] = &&unimplemented_instruction, // DEC D
        [0x16] = &&op_16,   // LD D,d8
        [0x17] = &&unimplemented_instruction, // RLA
        [0x18] = &&op_18,   // JR r8
        [0x19] = &&op_19,   // ADD HL,DE
        [0x1A] = &&op_1A,   // LD A,(DE)
        [0x1B] = &&unimplemented_instruction, // DEC DE
        [0x1C] = &&op_1C,   // INC E
        [0x1D] = &&unimplemented_instruction, // DEC E
        [0x1E] = &&unimplemented_instruction, // LD E,d8
        [0x1F] = &&unimplemented_instruction, // RRA

        [0x20] = &&op_20,   // JR NZ,r8
        [0x21] = &&op_21,   // LD HL,d16
        [0x22] = &&op_22,   // LDI (HL),A
        [0x23] = &&op_23,   // INC HL
        [0x24] = &&unimplemented_instruction, // INC H
        [0x25] = &&unimplemented_instruction, // DEC H
        [0x26] = &&unimplemented_instruction, // LD H,d8
        [0x27] = &&unimplemented_instruction, // DAA
        [0x28] = &&op_28,   // JR Z,r8
        [0x29] = &&unimplemented_instruction, // ADD HL,HL
        [0x2A] = &&op_2A,   // LD A,(HL+)
        [0x2B] = &&unimplemented_instruction, // DEC HL
        [0x2C] = &&op_2C,   // INC L
        [0x2D] = &&unimplemented_instruction, // DEC L
        [0x2E] = &&unimplemented_instruction, // LD L,d8
        [0x2F] = &&op_2F,   // CPL

        [0x30] = &&op_30,   // JR NC,r8
        [0x31] = &&op_31,   // LD SP,d16
        [0x32] = &&op_32,   // LDD (HL),A
        [0x33] = &&unimplemented_instruction, // INC SP
        [0x34] = &&op_34,   // INC (HL)
        [0x35] = &&unimplemented_instruction, // DEC (HL)
        [0x36] = &&op_36,   // LD (HL),d8
        [0x37] = &&unimplemented_instruction, // SCF
        [0x38] = &&unimplemented_instruction, // JR C,r8
        [0x39] = &&unimplemented_instruction, // ADD HL,SP
        [0x3A] = &&unimplemented_instruction, // LD A,(HL-)
        [0x3B] = &&unimplemented_instruction, // DEC SP
        [0x3C] = &&op_3C,   // INC A
        [0x3D] = &&op_3D,   // DEC A
        [0x3E] = &&op_3E,   // LD A,d8
        [0x3F] = &&unimplemented_instruction, // CCF

        // --- Register loads ---
        [0x40] = &&unimplemented_instruction, // LD B,B
        [0x41] = &&op_41,   // LD B,C
        [0x42] = &&unimplemented_instruction, // LD B,D
        [0x43] = &&unimplemented_instruction, // LD B,E
        [0x44] = &&unimplemented_instruction, // LD B,H
        [0x45] = &&unimplemented_instruction, // LD B,L
        [0x46] = &&unimplemented_instruction, // LD B,(HL)
        [0x47] = &&op_47,   // LD B,A
        [0x48] = &&unimplemented_instruction, // LD C,B
        [0x49] = &&unimplemented_instruction, // LD C,C
        [0x4A] = &&unimplemented_instruction, // LD C,D
        [0x4B] = &&unimplemented_instruction, // LD C,E
        [0x4C] = &&unimplemented_instruction, // LD C,H
        [0x4D] = &&unimplemented_instruction, // LD C,L
        [0x4E] = &&unimplemented_instruction, // LD C,(HL)
        [0x4F] = &&op_4F,   // LD C,A

        [0x50] = &&unimplemented_instruction, // LD D,B
        [0x51] = &&unimplemented_instruction, // LD D,C
        [0x52] = &&unimplemented_instruction, // LD D,D
        [0x53] = &&unimplemented_instruction, // LD D,E
        [0x54] = &&unimplemented_instruction, // LD D,H
        [0x55] = &&unimplemented_instruction, // LD D,L
        [0x56] = &&op_56,   // LD D,(HL)
        [0x57] = &&op_57,   // LD D,A
        [0x58] = &&unimplemented_instruction, // LD E,B
        [0x59] = &&unimplemented_instruction, // LD E,C
        [0x5A] = &&unimplemented_instruction, // LD E,D
        [0x5B] = &&unimplemented_instruction, // LD E,E
        [0x5C] = &&unimplemented_instruction, // LD E,H
        [0x5D] = &&unimplemented_instruction, // LD E,L
        [0x5E] = &&op_5E,   // LD E,(HL)
        [0x5F] = &&op_5F,   // LD E,A

        [0x60] = &&unimplemented_instruction, // LD H,B
        [0x61] = &&unimplemented_instruction, // LD H,C
        [0x62] = &&unimplemented_instruction, // LD H,D
        [0x63] = &&unimplemented_instruction, // LD H,E
        [0x64] = &&unimplemented_instruction, // LD H,H
        [0x65] = &&unimplemented_instruction, // LD H,L
        [0x66] = &&unimplemented_instruction, // LD H,(HL)
        [0x67] = &&unimplemented_instruction, // LD H,A
        [0x68] = &&unimplemented_instruction, // LD L,B
        [0x69] = &&unimplemented_instruction, // LD L,C
        [0x6A] = &&unimplemented_instruction, // LD L,D
        [0x6B] = &&unimplemented_instruction, // LD L,E
        [0x6C] = &&unimplemented_instruction, // LD L,H
        [0x6D] = &&unimplemented_instruction, // LD L,L
        [0x6E] = &&unimplemented_instruction, // LD L,(HL)
        [0x6F] = &&unimplemented_instruction, // LD L,A

        [0x70] = &&unimplemented_instruction, // LD (HL),B
        [0x71] = &&unimplemented_instruction, // LD (HL),C
        [0x72] = &&unimplemented_instruction, // LD (HL),D
        [0x73] = &&unimplemented_instruction, // LD (HL),E
        [0x74] = &&unimplemented_instruction, // LD (HL),H
        [0x75] = &&unimplemented_instruction, // LD (HL),L
        [0x76] = &&unimplemented_instruction, // HALT
        [0x77] = &&unimplemented_instruction, // LD (HL),A
        [0x78] = &&op_78,   // LD A,B
        [0x79] = &&op_79,   // LD A,C
        [0x7A] = &&unimplemented_instruction, // LD A,D
        [0x7B] = &&unimplemented_instruction, // LD A,E
        [0x7C] = &&unimplemented_instruction, // LD A,H
        [0x7D] = &&unimplemented_instruction, // LD A,L
        [0x7E] = &&unimplemented_instruction, // LD A,(HL)
        [0x7F] = &&unimplemented_instruction, // LD A,A

        // --- ALU ---
        [0x80] = &&unimplemented_instruction, // ADD A,B
        [0x81] = &&unimplemented_instruction, // ADD A,C
        [0x82] = &&op_82,   // ADD A,D
        [0x83] = &&unimplemented_instruction, // ADD A,E
        [0x84] = &&unimplemented_instruction, // ADD A,H
        [0x85] = &&unimplemented_instruction, // ADD A,L
        [0x86] = &&unimplemented_instruction, // ADD A,(HL)
        [0x87] = &&op_87,   // ADD A,A
        [0xA0] = &&unimplemented_instruction, // AND B
        [0xA1] = &&op_A1,   // AND C
        [0xA2] = &&unimplemented_instruction, // AND D
        [0xA3] = &&unimplemented_instruction, // AND E
        [0xA4] = &&unimplemented_instruction, // AND H
        [0xA5] = &&unimplemented_instruction, // AND L
        [0xA6] = &&unimplemented_instruction, // AND (HL)
        [0xA7] = &&op_A7,   // AND A
        [0xA8] = &&unimplemented_instruction, // XOR B
        [0xA9] = &&op_A9,   // XOR C
        [0xAA] = &&unimplemented_instruction, // XOR D
        [0xAB] = &&unimplemented_instruction, // XOR E
        [0xAC] = &&unimplemented_instruction, // XOR H
        [0xAD] = &&unimplemented_instruction, // XOR L
        [0xAE] = &&unimplemented_instruction, // XOR (HL)
        [0xAF] = &&op_AF,   // XOR A
        [0xB0] = &&op_B0,   // OR B
        [0xB1] = &&op_B1,   // OR C
        [0xB2] = &&unimplemented_instruction, // OR D
        [0xB3] = &&unimplemented_instruction, // OR E
        [0xB4] = &&unimplemented_instruction, // OR H
        [0xB5] = &&unimplemented_instruction, // OR L
        [0xB6] = &&unimplemented_instruction, // OR (HL)
        [0xB7] = &&unimplemented_instruction, // OR A

        // --- INC / DEC ---
        // (handled above with 0x04-0x3D etc.)

        // --- Memory ---
        // (handled above with 0x1A,0x22,0x2A,0x32,0x34,0x36,0xEA,0xFA)

        // --- Jumps ---
        [0xC3] = &&op_C3,   // JP a16
        [0xE9] = &&op_E9,   // JP (HL)

        // --- Stack / Calls ---
        [0xC0] = &&op_C0,   // RET NZ
        [0xC1] = &&op_C1,   // POP BC
        [0xC5] = &&op_C5,   // PUSH BC
        [0xC8] = &&op_C8,   // RET Z
        [0xC9] = &&op_C9,   // RET
        [0xCD] = &&op_CD,   // CALL a16
        [0xD1] = &&op_D1,   // POP DE
        [0xD5] = &&op_D5,   // PUSH DE
        [0xE1] = &&op_E1,   // POP HL
        [0xE5] = &&op_E5,   // PUSH HL
        [0xF1] = &&op_F1,   // POP AF
        [0xF5] = &&op_F5,   // PUSH AF

        // --- Interrupts ---
        [0xD9] = &&op_D9,   // RETI
        [0xF3] = &&op_F3,   // DI
        [0xFB] = &&op_FB,   // EI

        // --- Misc ---
        [0xE0] = &&op_E0,   // LDH (a8),A
        [0xE2] = &&op_E2,   // LD (C),A
        [0xE6] = &&op_E6,   // AND d8
        [0xEA] = &&op_EA,   // LD (a16),A
        [0xEF] = &&op_EF,   // RST 28H
        [0xF0] = &&op_F0,   // LDH A,(a8)
        [0xFA] = &&op_FA,   // LD A,(a16)
        [0xFE] = &&op_FE,   // CP d8
        [0xFF] = &&op_FF,   // RST 38H

        // --- Prefix ---
        [0xCB] = &&op_CB    // CB prefix handler
    };

    //printf("PC:%X PC: %02X Flags: %X Interrupts: %X\n", registers.pc, current_opcode, registers.f, interrupt_register);
    
fetch: 
    unsigned char current_opcode = memory_read_pc_byte();
    registers.pc++;
    
    printf("PC: %#06X Inst: %#04X ", registers.pc, current_opcode);
    printf("A: %#06X B: %#06X  C: %#06X  D: %#06X  E: %#06X  F: %#06X  H: %#06X  L: %#06X MC: %d\n\n", registers.a, registers.b, registers.c, registers.d, registers.e, registers.f, registers.h, registers.l, registers.machine_cycles);
    
    goto *dispatch[current_opcode];


    
    op_00:
        add_cycles(4);
        goto post;
    op_01:
        registers.bc = memory_read_pc_word();
        registers.pc += 2;
        add_cycles(12);
        goto post;
    op_05:
        half_carry_subtraction_8bit(registers.b, 1) ? set_flag(HALF_CARY) : unset_flag(HALF_CARY);
        --registers.b == 0 ? set_flag(ZERO) : unset_flag(ZERO);
        set_flag(SUBTRACTION);
        //printf("B: %#X\n", registers.b);
        add_cycles(4);
        goto post;
    op_06:
        registers.b = memory_read_pc_byte();
        registers.pc++;
        add_cycles(8);
        goto post;
    op_08:
    {
        unsigned short addr = memory_read_pc_word();
        registers.pc += 2;
        mem_write_byte(addr, registers.sp & 0xFF);
        mem_write_byte(addr + 1, registers.sp >> 8);
        add_cycles(20);
        goto post;
    }
    op_0B:
        registers.bc--;
        add_cycles(8);
        goto post;
    op_0C:
        half_carry_addition_8bit(registers.c, 1) ? set_flag(HALF_CARY) : unset_flag(HALF_CARY);
        registers.c++;
        registers.c ? unset_flag(ZERO) : set_flag(ZERO);
        unset_flag(SUBTRACTION);
        add_cycles(4);
        goto post;
    op_0D:
        half_carry_subtraction_8bit(registers.c, 1) ? set_flag(HALF_CARY) : unset_flag(HALF_CARY);
        --registers.c == 0 ? set_flag(ZERO) : unset_flag(ZERO);
        set_flag(SUBTRACTION);
        add_cycles(4);
        goto post;
    op_0E:
        registers.c = memory_read_pc_byte();
        registers.pc++;
        add_cycles(8);
        goto post;
    op_11:
        registers.de = memory_read_pc_word();
        registers.pc += 2;
        add_cycles(12);
        goto post;
    op_12:
        mem_write_byte(registers.de, registers.a);
        add_cycles(2);
        goto post;
    op_16:
        registers.d = memory_read_pc_byte();
        registers.pc++;
        add_cycles(8);
        goto post;
    op_18:
    {
        signed char offset = memory_read_pc_byte();
        registers.pc++;
        registers.pc += offset;
        add_cycles(12);
        goto post;
    }
    op_19:
        unset_flag(SUBTRACTION);
        if(half_carry_addition_16bit(registers.hl, registers.de)){
            set_flag(HALF_CARY);
        }
        if(carry_addition_16bit(registers.hl, registers.de)){
            set_flag(CARRY);
        }
        registers.hl += registers.de;
        add_cycles(8);
        goto post;
    op_1A:
        registers.a = mem_read_byte(registers.de);
        add_cycles(8);
        goto post;
    op_1C:
        half_carry_addition_8bit(registers.e, 1) ? set_flag(HALF_CARY) : unset_flag(HALF_CARY);
        registers.e++;
        registers.e ? unset_flag(ZERO) : set_flag(ZERO);
        unset_flag(SUBTRACTION);
        add_cycles(4);
        goto post;
    op_20:
    {
        signed char offset = memory_read_pc_byte();
        registers.pc++;
        if(get_flag(ZERO)){
            add_cycles(8);
        }
        else{
            registers.pc += offset;
            add_cycles(12);
        }
        goto post;
    }
    op_21:
        registers.hl = memory_read_pc_word();
        registers.pc += 2;
        add_cycles(12);
        goto post;
    op_22:
        mem_write_byte(registers.hl++, registers.a);
        add_cycles(8);
        goto post;
    op_23:
        registers.hl++;
        add_cycles(8);
        goto post;
    op_28:
    {
        signed char offset = memory_read_pc_byte();
        registers.pc++;
        if(get_flag(ZERO)){
            registers.pc += offset;
            add_cycles(12);
        }
        else{
            add_cycles(8);
        }
        goto post;
    }
    op_2A:
        registers.a = mem_read_byte(registers.hl++);
        add_cycles(8);
        goto post;
    op_2C:
        half_carry_addition_8bit(registers.l, 1) ? set_flag(HALF_CARY) : unset_flag(HALF_CARY);
        registers.l++;
        registers.l ? unset_flag(ZERO) : set_flag(ZERO);
        unset_flag(SUBTRACTION);
        add_cycles(4);
        goto post;
    op_2F:
        registers.a = ~registers.a;
        set_flag(SUBTRACTION);
        set_flag(HALF_CARY);
        add_cycles(4);
        goto post;
    op_30:
    {
        signed char offset = memory_read_pc_byte();
        registers.pc++;
        if(get_flag(CARRY)){
            add_cycles(8);
        }
        else{
            registers.pc += offset;
            add_cycles(12);
        }
        goto post;
    }
    op_31:
        registers.sp = memory_read_pc_word();
        registers.pc += 2;
        add_cycles(12);
        goto post;
    op_32:
        mem_write_byte(registers.hl, registers.a);
        registers.hl--;
        add_cycles(8);
        goto post;
    op_34:
        unsigned char before_inc = mem_read_byte(registers.hl);
        half_carry_addition_8bit(before_inc, 1) ? set_flag(HALF_CARY) : unset_flag(HALF_CARY);
        mem_write_byte(registers.hl, ++before_inc);
        before_inc ? unset_flag(ZERO) : set_flag(ZERO);
        unset_flag(SUBTRACTION);
        add_cycles(12);
        goto post;
    op_36:
        mem_write_byte(registers.hl, memory_read_pc_byte());
        registers.pc++;
        add_cycles(12);
        goto post;
    op_3C:
        half_carry_addition_8bit(registers.a, 1) ? set_flag(HALF_CARY) : unset_flag(HALF_CARY);
        registers.a++;
        registers.a ? unset_flag(ZERO) : set_flag(ZERO);
        unset_flag(SUBTRACTION);
        add_cycles(4);
        goto post;
    op_3D:
        half_carry_subtraction_8bit(registers.a, 1) ? set_flag(HALF_CARY) : unset_flag(HALF_CARY);
        --registers.a == 0 ? set_flag(ZERO) : unset_flag(ZERO);
        set_flag(SUBTRACTION);
        add_cycles(4);
        goto post;
    op_3E:
        registers.a = memory_read_pc_byte();
        registers.pc++;
        add_cycles(8);
        goto post;
    op_41:
        registers.b = registers.c;
        add_cycles(4);
        goto post;
    op_47:
        registers.b = registers.a;
        add_cycles(4);
        goto post;
    op_4F:
        registers.c = registers.a;
        add_cycles(4);
        goto post;
    op_56:
        registers.d = mem_read_byte(registers.hl);
        add_cycles(8);
        goto post;
    op_57:
        registers.d = registers.a;
        add_cycles(4);
        goto post;
    op_5E:
        registers.e = mem_read_byte(registers.hl);
        add_cycles(8);
        goto post;
    op_5F:
        registers.e = registers.a;
        add_cycles(4);
        goto post;
    op_78:
        registers.a = registers.b;
        add_cycles(4);
        goto post;
    op_79:
        registers.a = registers.c;
        add_cycles(4);
        goto post;
    op_82:
        clear_flags();
        if(carry_addition_8bit(registers.a, registers.d)){
            set_flag(CARRY);
        }
        if(half_carry_addition_8bit(registers.a, registers.d)){
            set_flag(HALF_CARY);
        }
        registers.a += registers.d;
        if (!registers.a) {
            set_flag(ZERO);
        }
        add_cycles(4);
        goto post;
    op_87:
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
        goto post;
    op_A1:
        registers.a &= registers.c;
        clear_flags();
        set_flag(HALF_CARY);
        if(!registers.a){
            set_flag(ZERO);
        }
        add_cycles(4);
        goto post;
    op_A7:
        clear_flags();
        set_flag(HALF_CARY);
        if(!registers.a){
            set_flag(ZERO);
        }
        add_cycles(4);
        goto post;
    op_A9:
        registers.a ^= registers.c;
        clear_flags();
        if(!registers.a){
            set_flag(ZERO);
        }
        add_cycles(4);
        goto post;
    op_AF:
        registers.a ^= registers.a;
        clear_flags();
        set_flag(ZERO);
        add_cycles(4);
        goto post;
    op_B0:
        registers.a |= registers.b;
        clear_flags();
        if(!registers.a){
            set_flag(ZERO);
        }
        add_cycles(4);
        goto post;
    op_B1:
        registers.a |= registers.c;
        clear_flags();
        if(!registers.a){
            set_flag(ZERO);
        }
        add_cycles(4);
        goto post;
    op_C0:
        if(!get_flag(ZERO)){
            registers.pc = stack_pop_word();
            printf("***RETURN*** PC: %04X\n\n", registers.pc);
            add_cycles(20);
        }
        else{
            add_cycles(8);
        }
        goto post;
    op_C1:
        registers.bc = stack_pop_word();
        add_cycles(12);
        goto post;
    op_C3:
        registers.pc = memory_read_pc_word();
        //printf("Jump to %X\n", registers.pc);
        add_cycles(12);
        goto post;
    op_C5:
        stack_write_word(registers.bc);
        add_cycles(16);
        goto post;
    op_C8:
        if(get_flag(ZERO)){
            registers.pc = stack_pop_word();
            printf("***RETURN*** PC: %04X\n\n", registers.pc);
            add_cycles(20);
        }
        else{
            add_cycles(8);
        }
        goto post;
    op_C9:
        registers.pc = stack_pop_word();
        printf("***RETURN*** PC: %#06X\n\n", registers.pc);
        add_cycles(16);
        goto post;
    op_CB:
        unsigned char cb_opcode = memory_read_pc_byte();
        registers.pc++;
        switch (cb_opcode){
            case 0x37:
                cb_swap_a();
                break;
            case 0x41:
                cb_bit_0_c();
                break;
            case 0x87:
                cb_zero_a();
                break;
            default:
                unimplemented_cb_instruction(cb_opcode);
                break;
        }
        goto post;
    op_CD:
    {
        unsigned short addr = memory_read_pc_word(); // get address of called function
        registers.pc += 2; // move pc past the address spot
        stack_write_word(registers.pc); // write this spot onto stack
        registers.pc = addr;
        printf("***CALL*** PC: %#06X\n\n", registers.pc);
        add_cycles(24);
        goto post;
    }
    op_D1:
        registers.de = stack_pop_word();
        add_cycles(12);
        goto post;
    op_D5:
        stack_write_word(registers.de);
        add_cycles(16);
        goto post;
    op_D9:
        registers.pc = stack_pop_word();
        printf("***RETURN FROM INTERRUPT*** PC: %#06X\n\n", registers.pc);
        registers.interrupts = 1;
        add_cycles(16);
        goto post;
    op_E0: {
        unsigned char offset = memory_read_pc_byte();
        mem_write_byte(0xFF00 + offset, registers.a);
        registers.pc++;
        add_cycles(12);
        goto post;
    }
    op_E1:
        registers.hl = stack_pop_word();
        add_cycles(12);
        goto post;
    op_E2:
        mem_write_byte(registers.c | 0xFF00, registers.a); // write register a to address stored in register c | 0xFF00
        add_cycles(8);
        goto post;
    op_E6:
        registers.a &= memory_read_pc_byte();
        registers.pc++;
        unset_flag(CARRY);
        unset_flag(SUBTRACTION);
        set_flag(HALF_CARY);
        registers.a ? unset_flag(ZERO) : set_flag(ZERO);
        add_cycles(8);
        goto post;
    op_E5:
        stack_write_word(registers.hl);
        add_cycles(16);
        goto post;
    op_E9:
        registers.pc = registers.hl;
        add_cycles(4);
        goto post;
    op_EA:
        mem_write_byte(memory_read_pc_word(), registers.a);
        registers.pc += 2;
        add_cycles(16);
        goto post;
    op_EF:
        stack_write_word(registers.pc);
        registers.pc = 0x28;
        add_cycles(16);
        goto post;
    op_F0: {
        unsigned char offset = memory_read_pc_byte();
        registers.a = mem_read_byte(0xFF00 + offset);

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
        goto post;
    }
    op_F1:
        registers.af = stack_pop_word();
        add_cycles(12);
        goto post;
    op_F3:
        registers.interrupts = 0;
        add_cycles(4);
        goto post;
    op_F5:
        stack_write_word(registers.af);
        add_cycles(16);
        goto post;
    op_FA:
    {
        unsigned short addr = memory_read_pc_word();
        registers.pc += 2;
        registers.a = mem_read_byte(addr);
        add_cycles(16);
        goto post;
    }
    op_FB:
        registers.interrupts = 1;
        add_cycles(4);
        goto post;
    op_FE:
        unsigned char other_byte = memory_read_pc_byte();
        printf("A: %#X D8: %#X\n", registers.a, other_byte);
        registers.pc++;
        registers.a == other_byte ? set_flag(ZERO) : unset_flag(ZERO);
        half_carry_subtraction_8bit(registers.a, other_byte) ? set_flag(HALF_CARY) : unset_flag(HALF_CARY);
        registers.a < other_byte ? set_flag(CARRY) : unset_flag(CARRY);
        set_flag(SUBTRACTION);
        add_cycles(8);
        goto post;
    op_FF:
        stack_write_word(registers.pc);
        registers.pc = 0x38;
        add_cycles(16);
        goto post;
    unimplemented_instruction:
        printf("%s %#04X\n", "Unimplemented opcode: ", current_opcode);
        exit(1);
    

post:

    /* Video Timer */

    if(video_cycles >= 456){
        if(didUpdate){
            get_tile_color_ids();
        }
        render_scanline();
        test_tiles();
        video_cycles = 0;
    }
    
    /* Interupt Handling */
    
    unsigned char interrupts_allowed = mem_read_byte(0xFFFF);
    unsigned char queued_interrupts = mem_read_byte(0xFF0F);
    unsigned char masked_interrupts = interrupts_allowed & queued_interrupts;

    if(registers.interrupts){
        if(masked_interrupts & 0x01){
            registers.interrupts = 0;
            stack_write_word(registers.pc);
            registers.machine_cycles += 20;
            registers.pc = 0x40;
        }
        if(masked_interrupts & 0x02){
            registers.interrupts = 0;
            stack_write_word(registers.pc);
            registers.machine_cycles += 20;
            registers.pc = 0x48;
        }
        if(masked_interrupts & 0x04){
            registers.interrupts = 0;
            stack_write_word(registers.pc);
            registers.machine_cycles += 20;
            registers.pc = 0x50;
        }
        if(masked_interrupts & 0x08){
            registers.interrupts = 0;
            stack_write_word(registers.pc);
            registers.machine_cycles += 20;
            registers.pc = 0x58;
        }
        if(masked_interrupts & 0x10){
            registers.interrupts = 0;
            stack_write_word(registers.pc);
            registers.machine_cycles += 20;
            registers.pc = 0x60;
        }
    }
    goto fetch;
    return 0;
}


/*
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
    init_screen();
    printf("%X\n", registers.pc);
    //void test();
    //test();
    //printf("%#02X\n", rom_bank[0xFFBA]);
    while(1){
        
        execute_opcode();
        
        
    }

}
*/
