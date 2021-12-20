#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include "chip8.h"
#include <SDL2/SDL.h>

#define WINDOW_SCALE 20
#define WINDOW_RES_W (CHIP8_SCREEN_WIDTH * WINDOW_SCALE)
#define WINDOW_RES_H (CHIP8_SCREEN_HEIGHT * WINDOW_SCALE)

static uint8_t rom[CHIP8_ROM_MAX_SIZE] = {0};

bool running = true;

SDL_Window *window;
SDL_Renderer *renderer;

#define CURSOR_HIDE "\033[?25l"
#define CURSOR_SHOW "\033[?25h"

void term_set_color(char *color)
{
    printf("\033[%dm", color);
}

void cursor_set_col(uint8_t col)
{
    printf("\033[%dG", col);
}

void cursor_set_pos(uint8_t row, uint8_t col)
{
    printf("\033[%d;%dH", row, col);
}

void set_cursor_visible(bool visible)
{
    if (visible)
    {
        printf(CURSOR_SHOW);
    }
    else
    {
        printf(CURSOR_HIDE);
    }
}

void screen_clear()
{
    printf("\033[2J\033[H");
}

void update_screen();

// Define the callback implementations

static uint8_t random_byte()
{
    return (uint8_t)(rand() % 0xFF);
}

static bool key_pressed(uint8_t index)
{
    return false;
}

void single_pix(bool b, uint8_t x, uint8_t y)
{
}

int main(int argc, char **argv)
{
    FILE *rom_file;
    size_t rom_size;
    time_t t;
    unsigned int current_time = 0;
    unsigned int last_tick_time = 0;

    if (argv[1] == NULL)
    {
        printf("Must specifiy a ROM file, e.g. chip8emu <rom_file>\r\n");
        return 1;
    }

    printf("Loading ROM file: %s...\r\n", argv[1]);

    // Load the ROM file from disk
    rom_file = fopen(argv[1], "rb");

    if (rom_file == NULL)
    {
        printf("Failed to open ROM file: %s\r\n", argv[1]);
        return 1;
    }

    rom_size = fread(rom, 1, sizeof(rom), rom_file);

    if (rom_size == 0)
    {
        printf("Failed to load ROM file: %s\r\n", argv[1]);
        return 1;
    }

    // Seed the random number generator
    srand((unsigned)time(&t));

    // Initialize CHIP-8 emulator core
    if (chip8_init(&random_byte, &single_pix, &update_screen) != CHIP8_SUCCESS)
    {
        printf("Failed to create CHIP-8 emulator core.\r\n");
    }

    // Copy ROM file into emulator memory
    if (chip8_load_rom(rom, rom_size) != CHIP8_SUCCESS)
    {
        printf("Failed to copy ROM into CHIP-8 system memory.\r\n");
        return 1;
    }

    printf("Starting CHIP-8 emulation...\r\n");

    screen_clear();
    set_cursor_visible(false);


    // TODO: handle user input

    while (running)
    {
        chip8_run_state_e state = chip8_get_run_state();

        switch (state)
        {
        case CHIP8_STATE_RUNNING:
            // poll_input();

            if (chip8_cycle() != CHIP8_SUCCESS)
            {
                printf("Failed to execute CPU cycle.\r\n");
                running = false;
            }

            break;
        case CHIP8_STATE_WAIT_FOR_INPUT:
            // poll_input();
            break;

        default:
            running = false;
            break;
        }

        // TODO: CPU usage is at 100%, add some sleep between emulator cycles

        // Update the timers and redraw the screen at 60Hz
        struct timeval ti;
        gettimeofday(&ti, NULL);

        current_time = ti.tv_sec + (ti.tv_usec / 1000);

        if (current_time - last_tick_time > 17)
        {
            chip8_tick_timers();
            last_tick_time = current_time;
        }
    }

    printf("Exiting\r\n");

    return 0;
}

void update_screen()
{
    screen_clear();

    for (int i = 0; i < 64; i++)
    {
        for (int j = 0; j < 32; j++)
        {
            cursor_set_pos(j, i * 2);

            if (chip8_get_screen()[j * 64 + i])
            {
                printf("##");
            }
            else
            {
                printf("  ");
            }
        }
    }
}
