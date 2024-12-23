#include <stdio.h>
#include "cpu.h"
#include "rom.h"
#include "memory.h"
#include "vram.h"

Registers registers;
unsigned char* rom_data;
long int rom_size;

unsigned char *io_bank;
unsigned char *working_ram;
unsigned char *high_ram;
unsigned char interrupt_enable;
unsigned char interrupt_flags;
unsigned char *rom_bank;
unsigned char *ram_bank;
unsigned char *vram_bank;

int didUpdate;


int init_memory(){
    rom_bank = rom_data;
    registers.pc = 0x0;
    registers.machine_cycles = 0;
    if(rom_size >= 0xC000){
        ram_bank = rom_data + 0xA000;
    }

    io_bank = malloc(0x4C);
    working_ram = malloc(0x4000);
    vram_bank = malloc(0x2000);

    registers.sp = 0xCFFF;

    high_ram = malloc(0x7F);

    mem_write_byte(0xFF00, 0xFF); // Stub joystick IO. 
    printf("Joystick: %#X\n",mem_read_byte(0xFF00));
    registers.interupts = 1;

    

    return 0;
}


inline void mem_write_byte(unsigned short addr, unsigned char data){
    if(addr >= 0x8000 && addr < 0x9800){
        vram_bank[addr-0x8000] = data;
        didUpdate = 1;
        printf("%s\n", "WROTE TO BACKGROUND MEM");
    }
    else if(addr >= 0xC000 && addr <= 0xCFFF){
        working_ram[addr - 0xC000] = data;
    }
    else if(addr >= 0xFF00 && addr <= 0xFF4B){
        io_bank[addr & 0x00FF] = data;
        if(addr == 0xFF46){
            printf("*** START DMA/OAM TRANSFER %#04X\n", data);
        }
    }
    else if(addr >= 0xFF80 && addr <= 0xFFFE){
        high_ram[addr - 0xFF80] = data;
        printf("***WROTE %#04X TO ADDRESS %#06X***\n", data, addr);
    }
    else if(addr == 0xFF0F){
        interrupt_flags = data;
    }
    else if(addr == 0xFFFF){
        interrupt_enable = data;
    }
}

inline unsigned char mem_read_byte(unsigned short addr){
    if(addr<0x8000){
        return rom_bank[addr];
    }
    else if(addr >= 0xC000 && addr <= 0xCFFF){
        return working_ram[addr - 0xC000];
    }
    else if(addr >= 0xFF00 && addr <= 0xFF4B){
        return io_bank[addr & 0x00FF];
    }
    else if(addr >= 0xFF80 && addr <= 0xFFFE){
        return high_ram[addr - 0xFF80];
    }
    else if(addr == 0xFF0F){
        return interrupt_flags;
    }
    else if(addr == 0xFFFF){
        return interrupt_enable;
    }
    return 0;
}

inline unsigned short stack_pop_word(){
    unsigned char lsb = mem_read_byte(registers.sp++);
    unsigned char msb = mem_read_byte(registers.sp++);
    return (msb << 8) | lsb;
}

inline void stack_write_word(unsigned short data){
    registers.sp -= 2;
    mem_write_byte(registers.sp, data & 0xFF);
    mem_write_byte(registers.sp + 1, data >> 8);
}


