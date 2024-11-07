#include <stdbool.h>

#ifdef _WIN32
#include <SDL/SDL.h> /* Windows-specific SDL2 library */
#else
#include <SDL2/SDL.h> /* macOS- and GNU/Linux-specific */
#endif

#define WIDTH 500
#define HEIGHT 500
#define DELAY 3000

// int main(int argc, char *argv[]){
//     SDL_Window *window = NULL;
    
//     SDL_Renderer *ren = NULL;
    

//     if (SDL_Init(SDL_INIT_VIDEO) != 0) {
//     fprintf(stderr, "SDL failed to initialise: %s\n", SDL_GetError());
//     return 1;
//   }

//   /* Creates a SDL window */
//   SDL_CreateWindowAndRenderer(500, 500, 0, &window, &ren); /* Additional flag(s) */
//   SDL_SetWindowTitle(window, "GBAEmulator");
//   unsigned int pixels[WIDTH * HEIGHT];
//   SDL_Texture* tex = SDL_CreateTexture(ren, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, 640, 480);

//   /* Checks if window has been created; if not, exits program */
//   if (window == NULL) {
//     fprintf(stderr, "SDL window failed to initialise: %s\n", SDL_GetError());
//     return 1;
//   }

//   SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
//     SDL_RenderClear(ren);
//     SDL_RenderPresent(ren);
//     bool quit = false;
//     //Event handler
//     SDL_Event e;
//     //While application is running
//     while( !quit )
//     {
//         //Handle events on queue
//         while( SDL_PollEvent( &e ) != 0 ) // poll for event
//         {
//             //User requests quit
//             if( e.type == SDL_QUIT ) // unless player manually quits
//             {
//                 quit = true;
//             }
//         }
//     }

//   /* Pauses all SDL subsystems for a variable amount of milliseconds */
//   SDL_Delay(DELAY);

//   SDL_DestroyTexture(tex);

//   SDL_DestroyRenderer(ren);

//   /* Frees memory */
//   SDL_DestroyWindow(window);
  
//   /* Shuts down all SDL subsystems */
//   SDL_Quit(); 
  
//   return 0;
// }