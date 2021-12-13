#include <stdio.h>
#include <string.h>
#include "chip8.h"
#include <SDL2/SDL.h>

static uint8_t ROM[1024];

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
    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window *window = SDL_CreateWindow("CHIP-8", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 320, 0);

    SDL_Surface *surface = SDL_GetWindowSurface(window);

    SDL_UpdateWindowSurface(window);

    SDL_Delay(5000);

    FILE *rom_file = fopen(argv[1], "rb");

    size_t rom_size = fread(&ROM[0x200], 1, 1024, rom_file);

    chip8_cpu_t chip8;

    printf("CHIP-8\r\n");

    chip8_init(&chip8, &delay_timer_handler);

    // Copy ROM into emulator memory
    memcpy(chip8.Memory, ROM, sizeof(ROM));

    // Execute op codes from memory

    uint64_t cycle_count = 0;

    while (1)
    {
        chip8_cycle(&chip8);
        cycle_count++;

        // TODO: decrement the delay timer at 60Hz while the timer is not equal to 0
        if ((cycle_count % 0x100) == 0)
        {
            printf("DELAY TICK\r\n");
            delay_timer--;
        }
    }

    return 0;
}
