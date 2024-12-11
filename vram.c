#include "vram.h"
#include "cpu.h"
#include "memory.h"

int didUpdate = 1;
unsigned int* screen;
int full_tile_map[256][256];

unsigned int palette[4] = {0xFFFFFFFF, 0xFFA0A0A0, 0xFF606060, 0xFF000000};

void init_screen(){
    screen = malloc(sizeof(int) * 160 * 144);
}

inline void get_tile_color_ids(){
    unsigned short origin;  // Middle of two memory blocks
    if(mem_read_byte(0xFF40)&0x10){   // LCDC bit 4 is set
        origin = 0x8000;
        unsigned short tile_index;
        unsigned short tile_addr;
        int curr_x = 0; // current x position in the 256 x 256 pixel grid (left)
        int curr_y = 0; // current y position in the 256 x 256 picel grid (top)
        int tile_row[8];
        unsigned short start_addr = mem_read_byte(0xFF40) & 0x8 ? 0x9C00 : 0x9800;  // IF LCDC bit 3 is set, use tilemap at $9C00, otherwise use $9800
        for(unsigned short current_addr = start_addr;current_addr<start_addr + 0x400;current_addr++){   // Loop through each byte starting from origin until end
            tile_index = mem_read_byte(current_addr);   // Index of the tile in the memory blocks
            tile_addr = mem_read_byte(origin + tile_index * 16);    // Calculate the address at the begining of the tile
            for(int i=0;i<8;i++){
                get_tile_row_color_ids(tile_row, mem_read_byte(tile_addr + i * 2), mem_read_byte(tile_addr + i * 2 + 1)); // Calculate a row of tile color IDs
                for(int j=0;j<8;j++){
                    full_tile_map[curr_y + i][curr_x + j] = tile_row[j];    // Copy to the big tile map
                }
            }
            curr_x += 8;    // Move 8 pixels to the right
            if(curr_x >= 256){  // If we hit the end, go down 8 pixels
                curr_x = 0;
                curr_y += 8;
            }
        }
    }
    else{
        origin = 0x9000;
        signed short tile_index;    // INDEX SIGNED IF LCDC BIT 4 IS NOT SET
        unsigned short tile_addr;
        int curr_x = 0; // current x position in the 256 x 256 pixel grid (left)
        int curr_y = 0; // current y position in the 256 x 256 picel grid (top)
        int tile_row[8];
        unsigned short start_addr = mem_read_byte(0xFF40) & 0x8 ? 0x9C00 : 0x9800;  // IF LCDC bit 3 is set, use tilemap at $9C00, otherwise use $9800
        for(unsigned short current_addr = start_addr;current_addr<start_addr + 0x400;current_addr++){   // Loop through each byte starting from origin until end
            tile_index = mem_read_byte(current_addr);   // Index of the tile in the memory blocks
            tile_addr = mem_read_byte(origin + tile_index * 16);    // Calculate the address at the begining of the tile
            for(int i=0;i<8;i++){
                get_tile_row_color_ids(tile_row, mem_read_byte(tile_addr + i * 2), mem_read_byte(tile_addr + i * 2 + 1)); // Calculate a row of tile color IDs
                for(int j=0;j<8;j++){
                    full_tile_map[curr_y + i][curr_x + j] = tile_row[j];    // Copy to the big tile map
                }
            }
            curr_x += 8;    // Move 8 pixels (1 tile) to the right
            if(curr_x >= 256){  // If we hit the end, go down 8 pixels (1 tile)
                curr_x = 0;
                curr_y += 8;
            }
        }
    }
    didUpdate = 0;
}


inline void get_tile_row_color_ids(int arr[], unsigned char b1, unsigned char b2){
    for(int i=0;i<8;i++){
        arr[i] = ((b2 >> (7-i)) << 1) | (b2 >> (7-i));
    }
}

inline void render_scanline(){
    unsigned char scanline = mem_read_byte(0xFF44);
    unsigned char SCY = mem_read_byte(0xFF42);
    unsigned char SCX = mem_read_byte(0xFF43);
    unsigned char interrupt_flags = mem_read_byte(0xFF0F);
    if(scanline < 144){
        mem_write_byte(0xFF0F, interrupt_flags & 0xFE);
        for(int i=0;i<160;i++){
            screen[(scanline * 160) + i] = palette[full_tile_map[(SCY + scanline) % 256][(SCX + i) % 256]];  // Wrap around when SCY/SCX goes past the tilemap
        }
        scanline++; // Update scanline value
        mem_write_byte(0xFF44, scanline);   // Put new value in memory
        update_screen();
    }
    else if(scanline >= 144 && scanline < 153){
        if(!(interrupt_flags & 0x1)){
            mem_write_byte(0xFF0F, interrupt_flags | 0x1);
        }
        scanline++; // Update scanline value
        mem_write_byte(0xFF44, scanline);   // Put new value in memory
    }
    else{
        scanline = 0;
        mem_write_byte(0xFF44, scanline);
    }
}