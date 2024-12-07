#ifndef __VRAM_H__
#define __VRAM_H__


extern int full_tile_map[256][256];
unsigned int palette[4] = {0xFFFFFFFF, 0xFFA0A0A0, 0xFF606060, 0xFF000000};
extern unsigned int *screen;

void get_tile_row_color_ids(int arr[], unsigned char b1, unsigned char b2);

#endif