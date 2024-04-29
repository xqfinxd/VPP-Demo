#include <iostream>
#include <SDL2/SDL.h>

#undef main
int main(int argc, char* argv[])
{
    SDL_SetMainReady();

    SDL_assert(SDL_Init(SDL_INIT_EVERYTHING) == 0);
    SDL_Window* window = SDL_CreateWindow("VPP-Demo",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        1280, 900, SDL_WINDOW_SHOWN | SDL_WINDOW_VULKAN);

    SDL_Delay(2000);

    SDL_DestroyWindow(window);

    return 0;
}
