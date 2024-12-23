#ifndef __VRAM_H__
#define __VRAM_H__



extern unsigned int *screen;
extern unsigned int *screen2;
extern int didUpdate;

void get_tile_row_color_ids(int arr[], unsigned char b1, unsigned char b2);
void get_tile_color_ids();
void init_screen();
void render_scanline();
void update_screen();

void update_screen2();
void test_tiles();

#endif