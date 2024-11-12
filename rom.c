#include <stdio.h>
#include <stdlib.h>
#include "rom.h"

unsigned char* rom_data;
long int rom_size;
FILE* fp;
int load_rom(char* filename){
    if((fp = fopen(filename, "rb"))==NULL){
        perror("File not found");
        return 1;
    }
    fseek(fp, 0, SEEK_END);
    rom_size = ftell(fp);
    printf("%##lX\n", rom_size);
    rewind(fp);
    rom_data = malloc(rom_size);
    fread(rom_data, rom_size, 1, fp);
    printf("%ld\n", rom_size);
    fclose(fp);
    
    
    return 0;
}

