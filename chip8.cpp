
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

// CHIP8 Instruction format
typedef struct {
    uint16_t opcode;
    uint16_t NNN; // 12 bit address
    uint8_t NN;  // 8 bit
    uint8_t N; // 4 bit
    uint8_t X;  // 4 bit register identifier
    uint8_t Y;  // 4 bit register identifier
} instruction_t;

// Chip 8 Object
typedef struct 
{
    emulator_state_t state;
    uint8_t ram[4096];
    bool display[64 * 32]; // original chip 8 resolution
    uint16_t stack[12]; // Subroutine stack
    uint16_t *stack_ptr;
    uint8_t V[16]; // Data Register
    uint16_t I; // Index Register
    uint8_t delay_timer; // Decrement at 60hz wheb > 0
    uint8_t sound_timer; // Decrement at 60hz and play tones > 0
    uint16_t PC; // Program Counter
    bool keypad[16]; // Key pad 0x0-0xF
    const char *rom_name; // Current rom name
    instruction_t inst; // Current chip 8 instruction
    
} chip8_t;





// Set up init emulate config from passed in argu
bool set_config(config_t *config, const int argc, const char **argv) {

    // Set default
    *config = {
        64, // Origin X
        32, // Origin Y
        0xFFFFFFFF, // White
        0x00000000, // Black
        10 // Scale Factor
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
void update_screen(const sdl_t sdl, const config_t config, const chip8_t chip8) {
    SDL_FRect rect = {0, 0, (float)config.scale_factor, (float)config.scale_factor};

    // Colours values foregrounds
    const uint8_t fg_r = (config.fg_color >> 24) & 0xFF; // Shift to 24 bits then mask it off
    const uint8_t fg_g = (config.fg_color >> 16) & 0xFF; // Shift to 16 bits then mask it off
    const uint8_t fg_b = (config.fg_color >> 8) & 0xFF; // Shift to 8 bits then mask it off
    const uint8_t fg_a = (config.fg_color >> 0) & 0xFF; // Shift to 0 bit then mask it off

    // Colours values background
    const uint8_t bg_r = (config.bg_color >> 24) & 0xFF; // Shift to 24 bits then mask it off
    const uint8_t bg_g = (config.bg_color >> 16) & 0xFF; // Shift to 16 bits then mask it off
    const uint8_t bg_b = (config.bg_color >> 8) & 0xFF; // Shift to 8 bits then mask it off
    const uint8_t bg_a = (config.bg_color >> 0) & 0xFF; // Shift to 0 bit then mask it off

    // loop through display pixel, draw a rect pre pixel to sdl window
    for (uint32_t i = 0; i < sizeof chip8.display; i++) {
        // translate index i to 2d x/y coords
        // x = i % widdow_width
        // y = i / window_width
        rect.x = i % config.window_width;
        rect.y = i / config.window_width;

        // If the pixel is on, draw foreground
        if (chip8.display[i]) {
            SDL_SetRenderDrawColor(sdl.renderer, fg_r, fg_g, fg_b, fg_a);
            SDL_RenderFillRect(sdl.renderer, &rect);
        }
        // pixel is off, draw background
        else {
            SDL_SetRenderDrawColor(sdl.renderer, bg_r, bg_g, bg_b, bg_a);
            SDL_RenderFillRect(sdl.renderer, &rect);
        }

    }
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
            // Space bar
            else if (event.key.key == SDLK_SPACE) {
                if (chip8->state == RUNNING) {
                    chip8->state = PAUSE;
                    puts("=====PAUSED=====");
                }
                else {
                    chip8->state = RUNNING;
                }
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

#ifdef DEBUG
void print_debug_info(chip8_t *chip8) {
    printf("Address: 0x%04X, Opcode: 0x%04X Desc: ", chip8->PC - 2, chip8->inst.opcode);
    switch ((chip8->inst.opcode >> 12) & 0x0F)
    {
        case 0x00:
            if (chip8->inst.NN == 0xE0) {
                // clear screen 0x00E0
                printf("clear screen");
                
            }
            else if (chip8->inst.NN == 0xEE) {
                // Return from subroutine
                printf("Return from subroutine from address 0x%04X\n", *(chip8->stack_ptr - 1));
                chip8->PC = *--chip8->stack_ptr;
            }
            else {
                printf("Unimplemented opcode. \n");
            }
            break;
        case 0x02:
            // call subroutine 0x2NNN at NNN
            *chip8->stack_ptr = chip8->PC; // store current address to return on subroutine address
            chip8->PC = chip8->inst.NNN; // set program ciunter to subroutine address

            break;
        
        case 0x06:
            // 0x6XNN: Set reigster VX to NN
            printf("Set register V%X to NN (0x%02X)\n", chip8->inst.X, chip8->inst.NN);
            break;

        case 0x0A:
            // 0xANNN: Set index register I to NNN
            printf("Set I to NNN (0x%04X)\n", chip8->inst.NNN);

            break;

        case 0x0D:
            // Draw at N coords
            printf("Draw N (%d) at height sprite at coords V%X (0x%02X), V%X (0x%02X) from memory location I (0x%04X)", 
                chip8->inst.N, chip8->inst.X, chip8->V[chip8->inst.X], chip8->inst.Y, chip8->V[chip8->inst.Y], chip8->I
            );
            break;

    default:
        printf("Unimplemented opcode. \n");
        break; // Invalid opcode
    }
}
#endif



// Emulate 1 chip 8 instuctions
void emulator_instructions(chip8_t *chip8, const config_t config) {
    // Get the next opcode from RAM
    chip8->inst.opcode = (chip8->ram[chip8 -> PC] << 8) | (chip8 -> ram[chip8->PC + 1]);

    // Pre increment program counter
    chip8->PC += 2; 

    // Fill out cur instructionf format
    chip8->inst.NNN = chip8->inst.opcode & 0x0FFF;
    chip8->inst.NN = chip8->inst.opcode & 0x0FF;
    chip8->inst.N = chip8->inst.opcode & 0x0F;
    chip8->inst.X = (chip8->inst.opcode >> 8) & 0x0F;
    chip8->inst.Y = (chip8->inst.opcode >> 4) & 0x0F;


    switch ((chip8->inst.opcode >> 12) & 0x0F)
    {
        case 0x00:
            if (chip8->inst.NN == 0xE0) {
                // clear screen 0x00E0
                memset(chip8->display, 0, sizeof chip8->display);
            }
            else if (chip8->inst.NN == 0xEE) {
                // Return from subroutine
                chip8->PC = *--chip8->stack_ptr;
            }
            break;
        case 0x02:
            // call subroutine 0x2NNN at NNN
            *chip8->stack_ptr = chip8->PC; // store current address to return on subroutine address
            chip8->PC = chip8->inst.NNN; // set program ciunter to subroutine address

            break;
        
        case 0x06:
            // 0x6XNN: Set reigster VX to NN
            chip8->V[chip8->inst.X] = chip8->inst.NN;
            break;
        
        case 0x0A:
            // 0xANNN: Set index register I to NNN
            chip8->I = chip8->inst.NNN;
            break;
        
        case 0x0D:
            // 0xDXYN: Draw N height sprite at coords X and Y
            // Read from memory location I
            // VF (Carry Flag) is set if any
            // Screen pixels are XOR with sprite bits
            {uint8_t X_coord = chip8-> V[chip8->inst.X] % config.window_width;
            uint8_t Y_coord = chip8-> V[chip8->inst.Y] % config.window_height;
            uint8_t orig_X = X_coord;

            chip8->V[0xF] = 0; // Init carry to 0

            // Loop through instructions
            for (uint8_t i = 0; i < chip8->inst.N; i++) {
                // Get the next byte/row of sprite data
                const uint8_t sprite_data = chip8->ram[chip8->I + i];
                X_coord = orig_X; // Reset X for the next row


                for (int j = 7; j >= 0; j --) {
                    // If sprite pixil nits is on and display pixel is on, set carry flage
                    uint8_t x = X_coord % config.window_width;
                    uint8_t y = Y_coord % config.window_height;

                    bool *pixel = &chip8->display[y * config.window_width + x];
                    const bool sprite_bit = (sprite_data & (1 << j));

                    if (sprite_bit && *pixel) {
                        chip8->V[0xF] = 1;
                    }

                    *pixel ^= sprite_bit;
                    X_coord++;

                    // Stop drawing if hit right edge
                    if (++X_coord >= config.window_width) {
                        break;
                    }
                }
                // Stop drawing when we hit the bottom edge
                if (++Y_coord >= config.window_height) {
                    break;
                }
            }
            break;
        }
    default:
        break; // Invalid opcode
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
    const size_t max_size = sizeof chip8->ram - entry_point;
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

    fclose(rom);

    // Default as running
    chip8->state = RUNNING; 
    chip8->PC = entry_point;
    chip8->rom_name = rom_name;
    chip8->stack_ptr = chip8->stack;

    return true;
}






// Main method
int main(int argc, char **argv) {
    // Default usage message for args
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <rom>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

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
    const char *rom_name =  argv[1];
    if (!init_chip8(&chip8, rom_name)) {
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

        // If the state is being paused, skip
        if (chip8.state == PAUSE) {
            continue;
        }

        // Instruction for chip 8
        emulator_instructions(&chip8, config);

        // Print debug info
        #ifdef DEBUG
        print_debug_info(&chip8);
        #endif

        // Delay for 60fps
        SDL_Delay(60);

        // Clear screen
        clear_screen(config, sdl);

        // Update the window with changes
        update_screen(sdl, config, chip8);
    }

    // Final Cleanup
    final_cleanup(&sdl);

    exit(EXIT_SUCCESS);
}
