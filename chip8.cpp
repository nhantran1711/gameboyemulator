
#include <iostream>
#include <SDL3/SDL.h>

// Init SDL
bool init_sdl(void) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
        SDL_Log("Could not init SDL subsystem: %s" , SDL_GetError());
        return false;
    }
    return true;
}

// Final clean up program
void final_cleanup(void) {
    SDL_Quit();
}

// Main method
int main() {

    // Init SDL
    if (!init_sdl()) {
        exit(EXIT_FAILURE);
    }

    // Final Cleanup
    final_cleanup();

    exit(EXIT_SUCCESS);
}
