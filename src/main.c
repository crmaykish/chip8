#include <stdio.h>
#include <string.h>
#include <time.h>
#include "chip8.h"
#include <SDL2/SDL.h>

#define WINDOW_SCALE 20
#define WINDOW_RES_W (CHIP8_SCREEN_WIDTH * WINDOW_SCALE)
#define WINDOW_RES_H (CHIP8_SCREEN_HEIGHT * WINDOW_SCALE)

static uint8_t ROM[4096 - 512];

uint8_t delay_timer;

uint8_t delay_timer_handler(uint8_t t)
{
    if (t != 0)
    {
        // Set the delay timer to t
        delay_timer = t;
    }

    return delay_timer;
}

int main(int argc, char **argv)
{
    time_t t;

    srand((unsigned)time(&t));
    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window *window;
    SDL_Renderer *renderer;

    SDL_CreateWindowAndRenderer(WINDOW_RES_W, WINDOW_RES_H, 0, &window, &renderer);

    FILE *rom_file = fopen(argv[1], "rb");

    size_t rom_size = fread(ROM, 1, sizeof(ROM), rom_file);

    chip8_cpu_t chip8;

    printf("CHIP-8\r\nStarting emulation...\r\n\r\n");

    chip8_init(&chip8, &delay_timer_handler);

    // Copy ROM into emulator memory
    for (int i = 0; i < sizeof(ROM); i++)
    {
        chip8.Memory[i + PC_START] = ROM[i];
    }

    // printf("\r\n\r\nMEM DUMP\r\n\r\n");

    // for (int i = 0; i < MEMORY_SIZE; i += 16)
    // {
    //     printf("%03X:  ", i);

    //     for (int j = 0; j < 16; j++)
    //     {
    //         printf("%02X ", chip8.Memory[i + j]);
    //     }
    //     printf("\r\n");
    // }

    // Execute op codes from memory

    uint64_t cycle_count = 0;

    SDL_Event e;

    bool running = true;

    while (running)
    {
        chip8_cycle(&chip8);
        cycle_count++;

        // TODO: decrement the delay timer at 60Hz while the timer is not equal to 0
        if ((cycle_count % 0x10) == 0)
        {

            if (delay_timer != 0)
            {
                delay_timer--;
            }
        }

        SDL_PollEvent(&e);

        switch (e.type)
        {
        case SDL_QUIT:
            running = false;
            break;

        default:
            break;
        }

        // Draw screen

        SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, SDL_ALPHA_OPAQUE);
        SDL_RenderClear(renderer);
        SDL_SetRenderDrawColor(renderer, 0x00, 0xFF, 0x00, SDL_ALPHA_OPAQUE);

        for (int i = 0; i < CHIP8_SCREEN_WIDTH; i++)
        {
            for (int j = 0; j < CHIP8_SCREEN_HEIGHT; j++)
            {
                if (chip8.Screen[i][j])
                {
                    SDL_Rect rect = {i * WINDOW_SCALE, j * WINDOW_SCALE, WINDOW_SCALE, WINDOW_SCALE};

                    SDL_RenderFillRect(renderer, &rect);
                }
            }
        }

        SDL_RenderPresent(renderer);

        // SDL_Delay(1);

        // if (cycle_count == 10)
        // {
        //     running = false;
        // }
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    printf("Exiting\r\n");

    return 0;
}
