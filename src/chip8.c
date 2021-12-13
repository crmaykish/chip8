#include "chip8.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void chip8_init(chip8_cpu_t *cpu, uint8_t (*delay_timer)(uint8_t))
{
    if (cpu == NULL)
    {
        DEBUG_PRINT("chip8_cpu_t pointer is NULL");
        return;
    }

    // Set the delay timer function pointer
    cpu->DelayTimer = delay_timer;

    // Start the delay timer at 0xFF
    cpu->DelayTimer(0xFF);

    // Initialize CPU state and memory
    cpu->I = 0;
    cpu->PC = PC_START;
    cpu->StackPointer = 0;
    memset(cpu->V, 0, REGISTER_COUNT);
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

    if (cpu == NULL)
    {
        DEBUG_PRINT("chip8_cpu_t pointer is NULL");
        return;
    }

    // Fetch opcode
    opcode = (cpu->Memory[cpu->PC] << 8) + cpu->Memory[cpu->PC + 1];

    nnn = opcode & 0x0FFF;
    n = opcode & 0x000F;
    x = (opcode & 0x0F00) >> 8;
    y = (opcode & 0x00F0) >> 4;
    kk = opcode & 0x00FF;

    switch ((opcode & 0xF000) >> 12)
    {
    case 0:
        switch (kk)
        {
        case 0xE0: // CLS
            // TODO: clear screen
            DEBUG_PRINT("CLEAR SCREEN\r\n");
            cpu->PC += 2;
            break;
        case 0xEE: // RET
            cpu->StackPointer--;
            cpu->PC = cpu->Stack[cpu->StackPointer];
            break;
        default:
            DEBUG_PRINT("NOT IMPLEMENTED: %04X\r\n", opcode);
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
            DEBUG_PRINT("NOT IMPLEMENTED: %04X\r\n", opcode);
            exit(1);
            break;
        }

        break;
    case 9:
        DEBUG_PRINT("NOT IMPLEMENTED: %04X\r\n", opcode);
        exit(1);
        break;
    case 0xA: // LD I
        cpu->PC += 2;
        cpu->I = nnn;
        break;
    case 0xB:
        DEBUG_PRINT("NOT IMPLEMENTED: %04X\r\n", opcode);
        exit(1);
        break;
    case 0xC:
        DEBUG_PRINT("NOT IMPLEMENTED: %04X\r\n", opcode);
        exit(1);
        break;
    case 0xD:
        cpu->PC += 2;

        // Read n bytes from memory at I

        // XOR the bytes as sprites onto screen at x, y

        for (i = 0; i < n; i++)
        {
            
        }

        // If pixels are erased, set VF to 1, else 0

        // If sprite is outside screen, wrap it around the otherside

        break;
    case 0xE:

        switch (kk)
        {
        case 0x9E: // SKP Vx
            cpu->PC += 2;
            DEBUG_PRINT("TODO: SKP if key Vx is pressed\r\n");
            break;
        case 0xA1:
            cpu->PC += 2; // SKNP Vx
            DEBUG_PRINT("TODO: SKP if key Vx is NOT pressed\r\n");
            break;
            break;
        default:
            DEBUG_PRINT("NOT IMPLEMENTED: %04X\r\n", opcode);
            exit(1);
            break;
        }

        break;
    case 0xF:
        switch (kk)
        {
        case 0x07: // LD Vx, DT
            cpu->PC += 2;
            cpu->V[x] = cpu->DelayTimer(0);

            break;
        case 0x15: // LD DT, Vx
            cpu->PC += 2;
            cpu->DelayTimer(cpu->V[x]);

            break;
        case 0x29: // LD F, Vx
            cpu->PC += 2;
            DEBUG_PRINT("TODO LD F, V\r\n");

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
            DEBUG_PRINT("NOT IMPLEMENTED: %04X\r\n", opcode);
            exit(1);
            break;
        }

        break;
    default:
        DEBUG_PRINT("NOT IMPLEMENTED: %04X\r\n", opcode);
        cpu->PC += 2;

        break;
    }

    DEBUG_PRINT("OP: %04X | PC: %04X | I: %04X | SP: %02X\r\n", opcode, cpu->PC, cpu->I, cpu->StackPointer);
}
