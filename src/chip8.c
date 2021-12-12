#include "chip8.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void chip8_init(chip8_cpu_t *cpu)
{
    cpu->I = 0;
    cpu->PC = 0x200;
    cpu->StackPointer = 0;
    memset(cpu->V, 0, 16);
    memset(cpu->Stack, 0, STACK_SIZE);
    memset(cpu->Memory, 0, MEMORY_SIZE);
}

void chip8_cycle(chip8_cpu_t *cpu)
{
    uint16_t opcode;
    uint16_t nnn;
    uint8_t n;
    uint8_t x, y;
    uint8_t kk;

    uint16_t temp;
    uint8_t i;

    // Fetch opcode
    opcode = (cpu->Memory[cpu->PC] << 8) + cpu->Memory[cpu->PC + 1];

    nnn = opcode & 0x0FFF;
    n = opcode & 0x000F;
    x = (opcode & 0x0F00) >> 8;
    y = (opcode & 0x00F0) >> 4;
    kk = opcode & 0x00FF;

    printf("OPCODE: %04X\r\n", opcode);

    switch ((opcode & 0xF000) >> 12)
    {
    case 0:
        switch (kk)
        {
        case 0xE0: // CLS
            // TODO: clear screen
            printf("CLEAR SCREEN\r\n");
            cpu->PC += 2;
            break;
        case 0xEE: // RET
            // TODO: return from subroutine
            cpu->StackPointer--;
            cpu->PC = cpu->Stack[cpu->StackPointer];
            break;
        default:
            printf("NOT IMPLEMENTED: %04X\r\n", opcode);
            exit(1);
            break;
        }

        break;

    case 1: // JP
        cpu->PC = nnn;
        break;

    case 2: // CALL
        cpu->Stack[cpu->StackPointer] = cpu->PC;
        cpu->StackPointer++;
        cpu->PC = nnn;
        break;
    case 3: // SE (skip if equal)
        cpu->PC += 2;

        if (cpu->V[x] == kk)
        {
            cpu->PC += 2;
        }

        break;
    case 4: // SNE (skip if not equal)
        cpu->PC += 2;

        if (cpu->V[x] != kk)
        {
            cpu->PC += 2;
        }

        break;
    case 5: // SE (skip if registers are equal)
        cpu->PC += 2;

        if (cpu->V[x] == cpu->V[y])
        {
            cpu->PC += 2;
        }

        break;
    case 6: // LD (load kk into Vx)
        cpu->PC += 2;
        cpu->V[x] = kk;
        break;
    case 7: // ADD (Vx += kk)
        cpu->PC += 2;
        cpu->V[x] += kk;
        break;
    case 8:

        switch (n)
        {
        case 0: // LD (Vx = Vy)
            cpu->PC += 2;
            cpu->V[x] = cpu->V[y];
            break;
        case 1: // OR (Vx |= Vy)
            cpu->PC += 2;
            cpu->V[x] |= cpu->V[y];
            break;
        case 2: // AND (Vx &= Vy)
            cpu->PC += 2;
            cpu->V[x] &= cpu->V[y];
            break;
        case 3: // XOR (Vx ^= Vy)
            cpu->PC += 2;
            cpu->V[x] ^= cpu->V[y];
            break;
        case 4: // ADD (Vx += Vy)
            cpu->PC += 2;
            temp = cpu->V[x] + cpu->V[y];
            cpu->V[x] = temp & 0xFF;
            cpu->V[0xF] = (temp > 255) ? 1 : 0;
            break;
            // case 5:
            //     break;
            // case 6:
            //     break;
            // case 7:
            //     break;
            // case 0xE:
            //     break;

        default:
            printf("NOT IMPLEMENTED: %04X\r\n", opcode);
            exit(1);
            break;
        }

        break;
    case 9:
        printf("NOT IMPLEMENTED: %04X\r\n", opcode);
        exit(1);
        break;
    case 0xA: // LD I
        cpu->PC += 2;
        cpu->I = nnn;
        break;
    case 0xB:
        printf("NOT IMPLEMENTED: %04X\r\n", opcode);
        exit(1);
        break;
    case 0xC:
        printf("NOT IMPLEMENTED: %04X\r\n", opcode);
        exit(1);
        break;
    case 0xD:
        cpu->PC += 2;
        printf("TODO DRW Vx, Vy, nibble\r\n");
        break;
    case 0xE:
        printf("NOT IMPLEMENTED: %04X\r\n", opcode);
        exit(1);
        break;
    case 0xF:
        switch (kk)
        {
        case 0x29: // LD F, Vx
            cpu->PC += 2;
            printf("TODO LD F, V\r\n");

            break;
        case 0x33: //LD B, Vx
            cpu->PC += 2;
            cpu->Memory[cpu->I] = cpu->V[x] / 100;
            cpu->Memory[cpu->I + 1] = (cpu->V[x] / 100) % 10;
            cpu->Memory[cpu->I + 2] = (cpu->V[x] % 100) % 10;
            break;

        case 0x65: // LD Vx, [I]
            cpu->PC += 2;

            // TODO: include x or not?
            memcpy(cpu->V, &cpu->Memory[cpu->I], x + 1);

            break;

        default:
            printf("NOT IMPLEMENTED: %04X\r\n", opcode);
            exit(1);
            break;
        }

        break;
    default:
        printf("NOT IMPLEMENTED: %04X\r\n", opcode);
        cpu->PC += 2;

        break;
    }
}
