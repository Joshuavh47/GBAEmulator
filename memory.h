#ifndef __MEMORY_H__
#define __MEMORY_H__



//extern unsigned char *gb_memory[16];
extern unsigned char *rom_bank;
extern unsigned char *ram_bank;
extern unsigned char *io_bank;
extern unsigned char *working_ram;
extern unsigned char *high_ram;
extern unsigned char interrupt_enable;
unsigned char interrupt_flags;
extern unsigned char joypad_input;



int init_memory();
void mem_write_byte(unsigned short addr, unsigned char data);
unsigned char mem_read_byte(unsigned short addr);
void stack_write_word(unsigned short data);
unsigned short stack_pop_word();

#endif