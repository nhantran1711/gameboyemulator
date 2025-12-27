
#include <iostream>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

typedef struct 
{
    SDL_Window *window;

} sdl_t;

typedef struct 
{
    uint32_t window_height; // SDL Window width
    uint32_t window_width; // SDL Window Height
} config_t;




// Set up init emulate config from passed in argu
bool set_config(config_t *config, const int argc, const char **argv) {

}


// Init SDL
bool init_sdl(sdl_t *sdl, const config_t config) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
        SDL_Log("Could not init SDL subsystem: %s" , SDL_GetError());
        return false;
    }

    sdl -> window = SDL_CreateWindow(
        "SDL 3 Window ", // Window title
        SDL_WINDOWPOS_CENTERED, // X pos
        SDL_WINDOWPOS_CENTERED, // Y pos
        config.window_width, // Width, in pixel
        config.window_height, // Height, in pixel
        SDl_Window_OPENGL // Flags
    )

    if (!sdl -> window) {
        SDL_Log("Could not create SDL Window %s", SDL_GetError())
        return false;
    }
    return true;
}

// Final clean up program
void final_cleanup(void) {
    SDL_DestroyWindow(window);
    SDL_Quit(); // Shut up subsystem
}

// Main method
int main(int argc, char **argv) {

    (void)argc;
    (void)argv;

    sdl_t sdl = {0};
    bool done = false;

    // Init SDL
    if (!init_sdl(&sdl)) {
        exit(EXIT_FAILURE);
    }

    // Init emulater configs
    config_t config = {0};
    if (!set_config(&config, argc, argv)) {
        exit(EXIT_FAILURE);
    }

    // Final Cleanup
    final_cleanup();

    exit(EXIT_SUCCESS);
}
