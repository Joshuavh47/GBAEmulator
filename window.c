#include <stdbool.h>
#include <stdlib.h>
#include "vram.h"
#include "window.h"

SDL_Window *window;
SDL_Renderer *renderer;
SDL_Texture *texture;

SDL_Window *window2;
SDL_Renderer *renderer2;
SDL_Texture *texture2;

unsigned int *screen;
unsigned int *screen2;

unsigned int *test_bitmap;

unsigned int *test_bitmap2;

unsigned char *copy_buff;
unsigned char *copy_buff2;
int pitch;
int pitch2;

void test(){
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't initialize SDL: %s", SDL_GetError());
        
    }
    pitch = WIDTH*4;
    window = SDL_CreateWindow("TEST", WIDTH, HEIGHT, SDL_WINDOW_RESIZABLE);
    renderer = SDL_CreateRenderer(window, NULL);
    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, WIDTH, HEIGHT);
    copy_buff = (unsigned char*)malloc(pitch * HEIGHT);
    copy_buff2 = (unsigned char*)malloc(pitch * HEIGHT);
    for(int i=0;i<pitch*HEIGHT;i+=4){
        copy_buff[i+0] = 0xFF;
        copy_buff[i+1] = 0x00;
        copy_buff[i+2] = 0x00;
        copy_buff[i+3] = 0xFF;
    }
    for(int i=0;i<pitch*HEIGHT;i+=4){
        copy_buff2[i+0] = 0x00;
        copy_buff2[i+1] = 0x00;
        copy_buff2[i+2] = 0xFF;
        copy_buff2[i+3] = 0xFF;
    }

    // for(int i=0;i<pitch*HEIGHT;i++){
    //     if(i%pitch == 0){
    //         printf("\n***NEW LINE***\n");
    //     }
    //     printf("%08X ", copy_buff[i]);
    // }
    
    while(1){
        SDL_Event e;
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
        int ret = SDL_LockTexture(texture, NULL, (void**)(&test_bitmap), &pitch);
        if(!ret){
            printf("%d %s\n",ret,SDL_GetError());
        }
        SDL_RenderClear(renderer);
        SDL_memcpy(test_bitmap, copy_buff, WIDTH * HEIGHT * sizeof(uint32_t));
        SDL_UnlockTexture(texture);
        SDL_RenderTexture(renderer,texture, NULL, NULL);
        SDL_RenderPresent(renderer);
        //printf("\n%d\n", ret);
        //printf("%s\n",SDL_GetError());
    }
}

void init_sdl(){
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't initialize SDL: %s", SDL_GetError());
    }
    pitch = TEXTURE_WIDTH*4;
    window = SDL_CreateWindow("TEST", WIDTH, HEIGHT, SDL_WINDOW_RESIZABLE);
    renderer = SDL_CreateRenderer(window, NULL);
    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, TEXTURE_WIDTH, TEXTURE_HEIGHT);

    pitch2 = 256*4;
    window2 = SDL_CreateWindow("debug", 256, 256, SDL_WINDOW_RESIZABLE);
    renderer2 = SDL_CreateRenderer(window2, NULL);
    texture2 = SDL_CreateTexture(renderer2, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 256, 256);
}


void update_screen(){
    
    
    
    
    //SDL_Event e;
    // while(SDL_PollEvent(&e)){
    //     if(e.type == SDL_EVENT_QUIT){
    //         SDL_DestroyTexture(texture);
    //         SDL_DestroyRenderer(renderer);
    //         SDL_DestroyWindow(window);
    //         SDL_Quit();
    //         exit(1);
    //     }
    //     if(e.type == SDL_EVENT_WINDOW_RESIZED){
    //         SDL_SetWindowAspectRatio(window, 1.111111111111f, 1.111111111111f);
    //     }
    // }
    int ret = SDL_LockTexture(texture, NULL, (void**)(&test_bitmap), &pitch);
    // if(!ret){
    //     printf("%d %s\n",ret,SDL_GetError());
    // }
    SDL_RenderClear(renderer);
    SDL_memcpy(test_bitmap, screen, TEXTURE_WIDTH * TEXTURE_HEIGHT * sizeof(int));
    SDL_UnlockTexture(texture);
    SDL_RenderTexture(renderer,texture, NULL, NULL);
    SDL_RenderPresent(renderer);

    
    
}
void update_screen2(){
    int ret2 = SDL_LockTexture(texture2, NULL, (void**)(&test_bitmap2), &pitch2);
    // if(!ret){
    //     printf("%d %s\n",ret,SDL_GetError());
    // }
    SDL_RenderClear(renderer2);
    SDL_memcpy(test_bitmap2, screen2, 256 * 256 * sizeof(int));
    SDL_UnlockTexture(texture2);
    SDL_RenderTexture(renderer2,texture2, NULL, NULL);
    SDL_RenderPresent(renderer2);
}
    
