#ifndef CHIP8_H
#define CHIP8_H

#include <stdint.h>

#define MEMORY_SIZE 4096
#define STACK_SIZE 16

typedef struct
{
    uint8_t V[16];
    uint16_t I;
    uint16_t PC;
    uint8_t StackPointer;
    uint16_t Stack[STACK_SIZE];
    uint8_t Memory[MEMORY_SIZE];
    // TODO: delay timer
    // TODO: sound timer
    // TODO: input
} chip8_cpu_t;

void chip8_init(chip8_cpu_t *cpu);

void chip8_cycle(chip8_cpu_t *cpu);

#endif
