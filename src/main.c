#include <stdio.h>
#include <string.h>
#include "chip8.h"

static uint8_t ROM[1024];

int main(int argc, char **argv)
{
    FILE *rom_file = fopen(argv[1], "rb");

    size_t rom_size = fread(&ROM[0x200], 1, 1024, rom_file);

    chip8_cpu_t chip8;

    printf("CHIP-8\r\n");

    chip8_init(&chip8);

    // Copy ROM into memory
    memcpy(chip8.Memory, ROM, sizeof(ROM));

    // Execute op codes from memory

    uint16_t cycle_count = 0;

    while (1)
    {
        chip8_cycle(&chip8);
        cycle_count++;
    }

    return 0;
}
