
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

    // Set default
    *config = (config_t) {
        .window_width = 64, // Origin X
        .window_height = 32 // Origin Y
    };

    // Override default values
    for (int i = 1; i <  argc; i ++) {
        (void)argv[i]; // Prevent compiler error from unused variables
    }
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
        SDL_Window_OPENGL // Flags
    );

    if (!sdl -> window) {
        SDL_Log("Could not create SDL Window %s", SDL_GetError());
        return false;
    }
    return true;
}

// Final clean up program
void final_cleanup(sdl_t sdl) {
    SDL_DestroyWindow(sdl.window);
    SDL_Quit(); // Shut up subsystem
}

// Main method
int main(int argc, char **argv) {

    (void)argc;
    (void)argv;

    sdl_t sdl = {0};
    bool done = false;

    
    // Init emulater configs
    config_t config = {0};
    if (!set_config(&config, argc, argv)) {
        exit(EXIT_FAILURE);
    }

    // Init SDL
    if (!init_sdl(&sdl, config)) {
        exit(EXIT_FAILURE);
    }


    // Final Cleanup
    final_cleanup(&sdl);

    exit(EXIT_SUCCESS);
}
