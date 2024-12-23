#include "cpu.h"
#include "vram.h"
#include "window.h"
#include "rom.h"

#include "memory.h"

int main(int argc, char* argv[]){
    load_rom("/Users/joshuaeres/Downloads/Tetris (JUE) (V1.1) [!].gb");
    //load_rom("/Users/joshuaeres/Downloads/cpu_instrs.gb");
    init_memory();
    init_screen();
    init_sdl();
    SDL_Event e;
    // int test[8];
    // get_tile_row_color_ids(test, 0x7e, 0x0a);
    // for(int i=0;i<8;i++){
    //     printf("%d ", test[i]);
    // }
    // printf("%s", "\n");

    while(1){
        while(SDL_PollEvent(&e)){
            if(e.type == SDL_EVENT_QUIT){
                SDL_DestroyTexture(texture);
                SDL_DestroyRenderer(renderer);
                SDL_DestroyWindow(window);
                SDL_Quit();
                exit(1);
            }
            if(e.type == SDL_EVENT_WINDOW_RESIZED){
                SDL_SetWindowAspectRatio(window, 1.111111111111f, 1.111111111111f);
            }
        }
        execute_opcode();
        
    }


}