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

uint8_t poll_input()
{
    SDL_Event e;
    uint8_t key_index;
    bool keyOn = false;

    // Check for user input
    SDL_PollEvent(&e);

    if (e.type == SDL_QUIT)
    {
        running = false;
        return 0xFF;
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
        return 0xFF;
    }

    switch (e.key.keysym.sym)
    {
    case SDLK_0:
        key_index = 0;
        break;

    case SDLK_1:
        key_index = 1;
        break;

    case SDLK_2:
        key_index = 2;
        break;

    case SDLK_3:
        key_index = 3;
        break;

    case SDLK_4:
        key_index = 4;
        break;

    case SDLK_5:
        key_index = 5;
        break;

    case SDLK_6:
        key_index = 6;
        break;

    case SDLK_7:
        key_index = 7;
        break;

    case SDLK_8:
        key_index = 8;
        break;

    case SDLK_9:
        key_index = 9;
        break;

    case SDLK_a:
        key_index = 0xA;
        break;

    case SDLK_b:
        key_index = 0xB;
        break;

    case SDLK_c:
        key_index = 0xC;
        break;

    case SDLK_d:
        key_index = 0xD;
        break;

    case SDLK_e:
        key_index = 0xE;
        break;

    case SDLK_f:
        key_index = 0xF;
        break;

    default:
        return 0xFF;
        break;
    }

    keys[key_index] = keyOn;

    return keyOn ? key_index : 0xFF;
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

    status = CHIP8_STATUS_RUNNING;

    unsigned int time = 0;
    unsigned int last_tick_time = 0;

    // TODO: emulator should manage the running variable, not the front-end

    while (running)
    {
        if (status == CHIP8_STATUS_RUNNING)
        {
            status = chip8_cycle();
            poll_input();
        }
        else if (status == CHIP8_WAITING_FOR_KEYPRESS)
        {
            uint8_t key = poll_input();

            if (key != 0xFF)
            {
                printf("input: %x\r\n", key);
                chip8_key_interrupt(key);
                status = CHIP8_STATUS_RUNNING;
            }
        }
        else
        {
            running = false;
        }

        time = SDL_GetTicks();

        if (time - last_tick_time > 17)
        {
            chip8_tick_timers();
            update_screen();
            last_tick_time = time;
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
                SDL_Rect rect = {i * WINDOW_SCALE + 1, j * WINDOW_SCALE + 1, WINDOW_SCALE - 2, WINDOW_SCALE - 2};

                SDL_RenderFillRect(renderer, &rect);
            }
        }
    }

    SDL_RenderPresent(renderer);
}
