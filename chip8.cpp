
#include <iostream>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

// SDL Container object
typedef struct 
{
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Event event;

} sdl_t;

// Configuration object
typedef struct 
{
    uint32_t window_width; // SDL Window width
    uint32_t window_height; // SDL Window height
    uint32_t fg_color; // Foreground colour
    uint32_t bg_color; // background colour
    uint32_t scale_factor; // scale factor
} config_t;

// Emulator states
typedef enum 
{
    QUIT,
    RUNNING,
    PAUSE
} emulator_state_t;

// Chip 8 Object
typedef struct 
{
    emulator_state_t state;
    uint8_t ram[4096];
    bool display[64 * 32]; // original chip 8 resolution
    uint16_t stack[12]; // Subroutine stack
    uint8_t V[16]; // Data Register
    uint16_t I; // Index Register
    uint8_t delay_timer; // Decrement at 60hz wheb > 0
    uint8_t sound_timer; // Decrement at 60hz and play tones > 0
    uint16_t PC; // Program Counter
    bool keypad[16]; // Key pad 0x0-0xF
    char *rom_name; // Current rom name
    
} chip8_t;



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

// Hanlde user input
void handle_input(chip8_t *chip8) {
    // Event
    SDL_Event event;
    while (SDL_PollEvent(&event)) {  // Poll events from SDL
        switch (event.type)
        {
        case SDL_EVENT_QUIT:
            chip8 -> state = QUIT;
            return;
        
        case SDL_EVENT_KEY_DOWN:
            if (event.key.key == SDLK_ESCAPE) {
                chip8->state = QUIT;
                return;
            }

            break;

        case SDL_EVENT_KEY_UP:
            break;

        default:
            break;
        }
    }
}

// Handle chip 8 init
bool init_chip8(chip8_t *chip8, const char rom_name[]) {
    // Entry point
    const uint32_t entry_point = 0x200;
    const uint8_t font[] = {
        0xF0, 0x90, 0x90, 0x90, 0xF0,		// 0
        0x20, 0x60, 0x20, 0x20, 0x70,		// 1
        0xF0, 0x10, 0xF0, 0x80, 0xF0,		// 2
        0xF0, 0x10, 0xF0, 0x10, 0xF0,		// 3
        0x90, 0x90, 0xF0, 0x10, 0x10,		// 4
        0xF0, 0x80, 0xF0, 0x10, 0xF0,		// 5
        0xF0, 0x80, 0xF0, 0x90, 0xF0,		// 6
        0xF0, 0x10, 0x20, 0x40, 0x40,		// 7
        0xF0, 0x90, 0xF0, 0x90, 0xF0,		// 8
        0xF0, 0x90, 0xF0, 0x10, 0xF0,		// 9
        0xF0, 0x90, 0xF0, 0x90, 0x90,		// A
        0xE0, 0x90, 0xE0, 0x90, 0xE0,		// B
        0xF0, 0x80, 0x80, 0x80, 0xF0,		// C
        0xE0, 0x90, 0x90, 0x90, 0xE0,		// D
        0xF0, 0x80, 0xF0, 0x80, 0xF0,		// E
        0xF0, 0x80, 0xF0, 0x80, 0x80		// F
    }; 

    // Load font
    memcpy(&chip8->ram[0], font, sizeof(font));

    // Load ROM to chip8
    FILE *rom = fopen(rom_name, "rb");
    if (!rom ) {
        SDL_Log("Could not open rom file");
        return false;
    }

    // Get/check rom size
    fseek(rom, 0, SEEK_END);
    const size_t rom_size = ftell(rom);
    const size_t max_size = sizeof chip8_>ram - entry_point;
    rewind(rom);

    // Checking rom size
    if (rom_size > max_size) {
        SDL_Log("Too big of rom file");
        return false;
    }

    // Load Rom
    if (fread(&chip8->ram[entry_point], rom_size, 1, rom) != 1) {
        SDL_Log("Can not read rom file to chip 8 memory");
        return false;
    };

    fclose(rom)

    // Default as running
    chip8->state = RUNNING; 
    chip8->PC = entry_point;
    chip8->rom_name = rom_name;

    return true;
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

    // Init chip 8 machine
    chip8_t chip8 = {};
    if (!init_chip8(&chip8)) {
        exit(EXIT_FAILURE);
    }

    // Init SDL
    if (!init_sdl(&sdl, config)) {
        exit(EXIT_FAILURE);
    }

    // Init the function the clear screen / sdl window to background colour
    clear_screen(config, sdl);


    // Main emulator loop
    while (chip8.state != QUIT) {
        
        handle_input(&chip8);

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
