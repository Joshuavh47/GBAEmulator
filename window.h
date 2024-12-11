#ifndef __WINDOW_H__
#define __WINDOW_H__

#ifdef _WIN32
#include <SDL/SDL.h> /* Windows-specific SDL2 library */
#else
#include <SDL3/SDL.h> /* macOS- and GNU/Linux-specific */
#endif


#define WIDTH 600
#define HEIGHT 540
#define DELAY 3000
#define TEXTURE_WIDTH 160
#define TEXTURE_HEIGHT 144


extern SDL_Window *window;
extern SDL_Renderer *renderer;
extern SDL_Texture *texture;

void init_sdl();



#endif