#include <stdio.h>
#include <string.h>
#include <time.h>
#include "chip8.h"
#include <SDL2/SDL.h>

#define WINDOW_SCALE 20
#define WINDOW_RES_W (CHIP8_SCREEN_WIDTH * WINDOW_SCALE)
#define WINDOW_RES_H (CHIP8_SCREEN_HEIGHT * WINDOW_SCALE)

static uint8_t Screen[CHIP8_SCREEN_WIDTH][CHIP8_SCREEN_HEIGHT] = {{0}};

static uint8_t rom[CHIP8_ROM_MAX_SIZE] = {0};

bool keys[16] = {false};

bool running = true;

SDL_Window *window;
SDL_Renderer *renderer;

void update_screen();

// Define the callback implementations

static uint8_t random_byte()
{
    return (uint8_t)(rand() % 0xFF);
}

static void draw_sprite_line(uint8_t b, uint8_t x, uint8_t y)
{
    for (int i = 0; i < 8; i++)
    {
        if (x + i < CHIP8_SCREEN_WIDTH)
        {
            Screen[x + i][y] ^= (b & (1 << (7 - i)) ? 1 : 0);
        }
    }

    // TODO: It's slow AF to update the whole screen whenever anything changes
    update_screen();
}

static bool key_pressed(uint8_t index)
{
    return keys[index];
}

static void clear_screen()
{
    memset(Screen, 0, CHIP8_SCREEN_WIDTH * CHIP8_SCREEN_HEIGHT);
}

void poll_input()
{
    SDL_Event e;
    bool keyOn = false;

    // Check for user input
    SDL_PollEvent(&e);

    if (e.type == SDL_QUIT)
    {
        running = false;
        return;
    }
    else if (e.type == SDL_KEYDOWN)
    {
        keyOn = true;
    }
    else if (e.type == SDL_KEYUP)
    {
        keyOn = false;
    }
    else
    {
        return;
    }

    switch (e.key.keysym.sym)
    {
    case SDLK_0:
        keys[0] = keyOn;
        break;

    case SDLK_1:
        keys[1] = keyOn;
        break;

    case SDLK_2:
        keys[2] = keyOn;
        break;

    case SDLK_3:
        keys[3] = keyOn;
        break;

    case SDLK_4:
        keys[4] = keyOn;
        break;

    case SDLK_5:
        keys[5] = keyOn;
        break;

    case SDLK_6:
        keys[6] = keyOn;
        break;

    case SDLK_7:
        keys[7] = keyOn;
        break;

    case SDLK_8:
        keys[8] = keyOn;
        break;

    case SDLK_9:
        keys[9] = keyOn;
        break;

    default:
        break;
    }
}

int main(int argc, char **argv)
{
    if (argv[1] == NULL)
    {
        printf("Must specifiy a ROM file, e.g. chip8emu <rom_file>\r\n");
        return 1;
    }

    printf("Loading ROM file: %s...\r\n", argv[1]);

    // Load the ROM file from disk
    FILE *rom_file = fopen(argv[1], "rb");

    if (rom_file == NULL)
    {
        printf("Failed to open ROM file: %s\r\n", argv[1]);
        return 1;
    }

    size_t rom_size = fread(rom, 1, sizeof(rom), rom_file);

    if (rom_size == 0)
    {
        printf("Failed to load ROM file: %s\r\n", argv[1]);
        return 1;
    }

    printf("Seeding the random number generator...\r\n");
    // Seed the random number generator
    time_t t;
    srand((unsigned)time(&t));

    // Set up the SDL window and renderer
    printf("Setting up SDL...\r\n");

    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        printf("Could not initialize SDL: %s\r\n", SDL_GetError());
        return 1;
    }

    if (SDL_CreateWindowAndRenderer(WINDOW_RES_W, WINDOW_RES_H, 0, &window, &renderer) != 0)
    {
        printf("Could not create SDL window and renderer: %s\r\n", SDL_GetError());
        return 1;
    }

    // Initialize CHIP-8 emulator core
    printf("Creating the CHIP-8 emulator core...\r\n");

    chip8_init(&random_byte, &draw_sprite_line, &key_pressed, &clear_screen);

    chip8_status_e status;

    // Copy ROM file into emulator memory
    printf("Copying ROM into CHIP-8 system memory...\r\n");
    status = chip8_load_rom(rom, rom_size);

    // Execute op codes from memory
    printf("Starting CHIP-8 emulation...\r\n");

    bool keyOn = false;

    while (running)
    {
        poll_input();

        // Fetch and execute CHIP-8 instruction
        status = chip8_cycle();

        if (status != CHIP8_SUCCESS)
        {
            running = false;
        }
    }

    printf("Exiting with status: %d\r\n", status);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    return 0;
}

void update_screen()
{
    // Render
    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(renderer);
    SDL_SetRenderDrawColor(renderer, 0x00, 0xFF, 0x00, SDL_ALPHA_OPAQUE);

    for (int i = 0; i < CHIP8_SCREEN_WIDTH; i++)
    {
        for (int j = 0; j < CHIP8_SCREEN_HEIGHT; j++)
        {
            if (Screen[i][j])
            {
                SDL_Rect rect = {i * WINDOW_SCALE, j * WINDOW_SCALE, WINDOW_SCALE, WINDOW_SCALE};

                SDL_RenderFillRect(renderer, &rect);
            }
        }
    }

    SDL_RenderPresent(renderer);
}
