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

int main(int argc, char **argv)
{
    // Seed the random number generator
    time_t t;
    srand((unsigned)time(&t));

    // Set up the SDL window and renderer
    SDL_Init(SDL_INIT_VIDEO);
    SDL_CreateWindowAndRenderer(WINDOW_RES_W, WINDOW_RES_H, 0, &window, &renderer);

    // Load the ROM file from disk
    FILE *rom_file = fopen(argv[1], "rb");
    size_t rom_size = fread(rom, 1, sizeof(rom), rom_file);

    // Initialize CHIP-8 emulator core
    chip8_init(&random_byte, &draw_sprite_line);

    // Copy ROM file into emulator memory
    chip8_load_rom(rom, rom_size);

    // Execute op codes from memory

    SDL_Event e;

    bool running = true;

    printf("Starting CHIP-8 emulation...\r\n");

    while (running)
    {
        // Check for user input
        SDL_PollEvent(&e);

        switch (e.type)
        {
        case SDL_QUIT:
            running = false;
            break;

        default:
            break;
        }

        // Fetch and execute CHIP-8 instruction
        chip8_cycle();
    }

    printf("Exiting\r\n");

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
