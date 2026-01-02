
#include <iostream>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

typedef struct 
{
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Event event;

} sdl_t;

typedef struct 
{
    uint32_t window_width; // SDL Window width
    uint32_t window_height; // SDL Window height
    uint32_t fg_color; // Foreground colour
    uint32_t bg_color; // background colour
    uint32_t scale_factor; // scale factor
} config_t;




// Set up init emulate config from passed in argu
bool set_config(config_t *config, const int argc, const char **argv) {

    // Set default
    *config = {
        640, // Origin X
        320, // Origin Y
        0xFFFFFFFF, // White
        0x00000000, // Black
        20 // Scale Factor
    };

    // Override default values
    for (int i = 1; i <  argc; i ++) {
        (void)argv[i]; // Prevent compiler error from unused variables
    }
    return true;
}


// Init SDL
bool init_sdl(sdl_t *sdl, const config_t config) {

    SDL_SetMainReady();

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
        // SDL3 returns true on success, false on failure
        SDL_Log("Could not init SDL subsystem: %s", SDL_GetError());
        return false;
    }

    sdl -> window = SDL_CreateWindow(
        "SDL 3 Window ", // Window title
        config.window_width * config.scale_factor, // Width, in pixel
        config.window_height * config.scale_factor, // Height, in pixel
        0 // Flags
    );

    if (!sdl -> window) {
        SDL_Log("Could not create SDL Window %s", SDL_GetError());
        return false;
    }

    // Renderer
    sdl -> renderer = SDL_CreateRenderer(sdl -> window, NULL);
    if (!sdl -> renderer) {
        SDL_Log("Could not create SDL renderer %s", SDL_GetError());
        return false;
    }
    return true;
}

// Final clean up program
void final_cleanup(sdl_t *sdl) {
    SDL_DestroyWindow(sdl -> window);
    SDL_DestroyRenderer(sdl -> renderer);
    SDL_Quit(); // Shut up subsystem
}

// Clear screen
void clear_screen(const config_t config, sdl_t sdl) {

    // Int screen clear
    const uint8_t r = (config.bg_color >> 24) & 0xFF; // Shift to 24 bits then mask it off
    const uint8_t g = (config.bg_color >> 16) & 0xFF; // Shift to 16 bits then mask it off
    const uint8_t b = (config.bg_color >> 8) & 0xFF; // Shift to 8 bits then mask it off
    const uint8_t a = (config.bg_color >> 0) & 0xFF; // Shift to 0 bit then mask it off

    SDL_SetRenderDrawColor(sdl.renderer, r, g, b, a);
    SDL_RenderClear(sdl.renderer);
}

// Update screen with changes
void update_screen(const sdl_t sdl) {
    SDL_RenderPresent(sdl.renderer);
}


// Main method
int main(int argc, char **argv) {

    (void)argc;
    (void)argv;

    sdl_t sdl = {};
    
    // Init emulater configs
    config_t config = {};
    if (!set_config(&config, argc, (const char**)argv)) {
        exit(EXIT_FAILURE);
    }

    // Init SDL
    if (!init_sdl(&sdl, config)) {
        exit(EXIT_FAILURE);
    }

    // Init the function the clear screen / sdl window to background colour
    clear_screen(config, sdl);


    // Main emulator loop
    bool running = true;
    while (running) {
        // Event
        SDL_Event event;
        while (SDL_PollEvent(&event)) {  // Poll events from SDL
            if (event.type == SDL_EVENT_QUIT) {  // Check for close window event
                running = false;
            }
        }

        // Delay for 60fps
        SDL_Delay(60);

        // Clear screen
        clear_screen(config, sdl);

        // Update the window with changes
        update_screen(sdl);
    }

    // Final Cleanup
    final_cleanup(&sdl);

    exit(EXIT_SUCCESS);
}
