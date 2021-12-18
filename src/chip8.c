#include "chip8.h"
#include <string.h>

#include <stdio.h>

// === Callbacks === //

static uint8_t (*random_byte)();

static void (*draw_sprite_line)(uint8_t, uint8_t, uint8_t);

static bool (*key_pressed)(uint8_t index);

static void (*clear_screen)();

// TODO: keep the screen memory internal to the emulator
// provide callbacks for screen_changed, pixel_drawn, etc.

// TODO: write an assembler/disassembler and some test programs

// TODO: add some unit tests

// === CHIP-8 System State === /

// 4KB of address space
static uint8_t Memory[CHIP8_MEM_SIZE];

// Bitmap font - this goes into the beginning of memory space
static uint8_t Font[] = {
    // First 80 bytes is font sprites
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

// Address lookup table for font sprites - where in memory does the sprite for each digit start?
static uint8_t SpriteLookup[0x10] = {0, 5, 10, 15, 20, 25, 30, 35, 40, 45, 50, 55, 60, 65, 70, 75};

// 16 8-bit registers
static uint8_t V[CHIP8_REG_COUNT];

// 16-bit address/pointer register
static uint16_t I;

// 16-bit program counter
static uint16_t PC;

// Call stack (16 16-bit addresses)
static uint16_t Stack[CHIP_8_STACK_SIZE];
static uint8_t StackPointer;

// Timers
static uint8_t DelayTimer;
static uint8_t SoundTimer;

// === Internal emulator state === //

static chip8_run_state_e run_state = CHIP8_STATE_RUNNING;

static uint8_t io_reg;

// Current opcode bytes
static uint8_t op_msb, op_lsb;

// Opcodes are always two bytes long. Fetch and decode consistent pieces of them.
// Note: These macros depend on the global opcode bytes defined above.

#define FETCH()          \
    op_msb = Memory[PC]; \
    op_lsb = Memory[PC + 1]

#define TYPE ((op_msb & 0xF0) >> 4)
#define NNN (((op_msb & 0x0F) << 8) + op_lsb)
#define N (op_lsb & 0x0F)
#define X (op_msb & 0xF)
#define Y ((op_lsb & 0xF0) >> 4)
#define KK (op_lsb)

// Working variables for opcode handling
static uint16_t temp;
static uint8_t i;

// === Emulator implementation === //

chip8_status_e chip8_init(uint8_t (*random_byte_func)(),
                          void (*draw_byte_func)(uint8_t, uint8_t, uint8_t),
                          bool (*key_pressed_func)(uint8_t),
                          void (*clear_screen_func)())
{
    if (random_byte_func == NULL ||
        draw_byte_func == NULL ||
        key_pressed_func == NULL ||
        clear_screen_func == NULL)
    {
        return CHIP8_ERROR;
    }

    I = 0;
    PC = CHIP8_PC_START;
    StackPointer = 0;
    SoundTimer = 0;
    memset(V, 0, CHIP8_REG_COUNT);
    memset(Stack, 0, CHIP_8_STACK_SIZE);
    memset(Memory, 0, CHIP8_MEM_SIZE);

    // Copy font data into memory
    memcpy(Memory, Font, sizeof(Font));

    random_byte = random_byte_func;
    draw_sprite_line = draw_byte_func;
    key_pressed = key_pressed_func;
    clear_screen = clear_screen_func;

    return CHIP8_SUCCESS;
}

chip8_status_e chip8_load_rom(uint8_t *rom, size_t bytes)
{
    if (rom == NULL || bytes > CHIP8_ROM_MAX_SIZE)
    {
        return CHIP8_ERROR;
    }

    memcpy(&Memory[CHIP8_PC_START], rom, bytes);

    return CHIP8_SUCCESS;
}

void chip8_key_interrupt(uint8_t key)
{
    if (run_state == CHIP8_STATE_WAIT_FOR_INPUT)
    {
        V[io_reg] = key;
        run_state = CHIP8_STATE_RUNNING;
    }
}

void chip8_tick_timers()
{
    if (run_state == CHIP8_STATE_RUNNING)
    {
        if (DelayTimer != 0)
        {
            DelayTimer--;
        }

        if (SoundTimer != 0)
        {
            SoundTimer--;
        }
    }
}

chip8_run_state_e chip8_get_run_state()
{
    return run_state;
}

chip8_status_e chip8_cycle()
{
    // Fetch opcode from system memory at the program counter
    FETCH();

    printf("%04X: OP: %02X%02X, I: %03X, Vf: %02X | ", PC, op_msb, op_lsb, I, V[0xF]);

    // Handle the current opcode based on its type
    switch (TYPE)
    {
    case 0:
        switch (KK)
        {
        // CLS - clear the screen
        case 0xE0:
            printf("CLS");
            PC += 2;
            clear_screen();
            break;

        // RET - return from subroutine
        case 0xEE:
            printf("RET");
            PC = Stack[StackPointer];
            StackPointer--;
            break;

        // Invalid 0-type opcode
        default:
            run_state = CHIP8_STATE_INVALID_OPCODE;
            break;
        }

        break;

    // JP nnn - absolute jump
    case 1:
        printf("JP $%03X", NNN);
        PC = NNN;
        break;

    // CALL nnn - call a subroutine at nnn
    case 2:
        printf("CALL $%03X", NNN);
        PC += 2; // TODO: I think this is correct, without it, code seems to get stuck in a loop of the first subroutine
        StackPointer++;
        Stack[StackPointer] = PC;
        PC = NNN;
        break;

    // SE Vx, kk - Skip next instruction if Vx equals kk
    case 3:
        printf("SE V%x, $%X", X, KK);
        PC += 2;
        if (V[X] == KK)
            PC += 2;
        break;

    // SNE Vx, kk - Skip next instruction if Vx does not equal kk
    case 4:
        printf("SNE V%x, $%X", X, KK);
        PC += 2;
        if (V[X] != KK)
            PC += 2;
        break;

    // SE Vx, Vy - Skip next instruction if Vx == Vy
    // Note: this is technically only a valid opcode when N == 0, not checking for that
    case 5:
        printf("SE V%x, V%x", X, Y);
        PC += 2;
        if (V[X] == V[Y])
            PC += 2;
        break;

    // LD Vx, kk - Load kk into the Vx register
    case 6:
        printf("LD V%x, $%X", X, KK);
        PC += 2;
        V[X] = KK;
        break;

    // ADD Vx, kk - Add kk to the Vx register
    case 7:
        printf("ADD V%x, $%X", X, KK);
        PC += 2;
        V[X] += KK;
        break;

    case 8:
        switch (N)
        {

        // LD Vx, Vy - Load Vy into Vx
        case 0:
            printf("LD V%x, V%x", X, Y);
            PC += 2;
            V[X] = V[Y];
            break;

        // OR Vx, Vy - Load (Vx OR Vy) into Vx
        case 1:
            printf("OR V%x, V%x", X, Y);
            PC += 2;
            V[X] |= V[Y];
            break;

        // AND Vx, Vy - Load (Vx AND Vy) into Vx
        case 2:
            printf("AND V%x, V%x", X, Y);
            PC += 2;
            V[X] &= V[Y];
            break;

        // XOR Vx, Vy - Load (Vx XOR Vy) into Vx
        case 3:
            printf("XOR V%x, V%x", X, Y);
            PC += 2;
            V[X] ^= V[Y];
            break;

        // ADD Vx, Vy - Add Vy to Vx
        case 4:
            printf("ADD V%x, V%x", X, Y);
            PC += 2;
            temp = V[X] + V[Y];
            V[X] = temp & 0xFF;
            V[0xF] = (temp > 255) ? 1 : 0;
            break;

        // SUB Vx, Vy - Subtract Vy from Vx
        case 5: // SUB Vx, Vy
            printf("SUB V%x, V%x", X, Y);
            PC += 2;
            V[X] -= V[Y];
            V[0xF] = (V[X] > V[Y]);
            break;

        // SHR Vx, {Vf}
        case 6:
            printf("SHR V%x", X);
            PC += 2;
            V[0xF] = ((V[X] & 0x1) > 0);
            V[X] /= 2;
            break;

        case 7:
            // TODO
            run_state = CHIP8_STATE_UNSUPPORTED_OPCODE;
            break;

        // SHL Vx, {Vf} - Shift Vx left, carry into Vf if necessary
        case 0xE:
            printf("SHL V%x", X);
            PC += 2;
            V[0xF] = ((V[X] & 0x10000000) > 0);
            V[X] *= 2;
            break;

        // Invalid 8-type opcode
        default:
            run_state = CHIP8_STATE_INVALID_OPCODE;
        }

        break;

    // SNE Vx, Vy - Skip next instruction if Vx does not equal Vy
    // Note: this is technically only a valid opcode when N == 0, not checking for that
    case 9:
        printf("SNE V%x, V%x", X, Y);
        PC += 2;
        if (V[X] != V[Y])
            PC += 2;
        break;

    // LD I, NNN - Load address into I
    case 0xA:
        printf("LD I, $%03X", NNN);
        PC += 2;
        I = NNN;
        break;

    case 0xB:
        // TODO
        run_state = CHIP8_STATE_UNSUPPORTED_OPCODE;
        break;

    // RAND Vx, kk - Vx is set to (random byte AND kk)
    case 0xC:
        printf("RAND V%x, $%X", X, KK);
        PC += 2;
        V[X] = random_byte() & KK;
        break;

    // DRW Vx, Vy, nibble
    case 0xD:
        printf("DRW V%x, V%x, %X", X, Y, N);

        PC += 2;

        // TODO: this does not handle sprites wrapping off screen
        // TODO: this does not properly set Vf flag

        for (i = 0; i < N; ++i)
        {
            draw_sprite_line(Memory[I + i], V[X], V[Y] + i);
        }

        break;

    case 0xE:

        switch (KK)
        {
        // SKP Vx - Skip next instruction if key with value Vx is pressed
        case 0x9E:
            printf("SKP V%x", X);
            PC += 2;
            if (key_pressed(V[X]))
                PC += 2;
            break;

        // SKNP Vx - Skip next instruction if key with value Vx is not pressed
        case 0xA1:
            printf("SKNP V%x", X);
            PC += 2;
            if (!key_pressed(V[X]))
                PC += 2;
            break;

        // Invalid E-type opcode
        default:
            run_state = CHIP8_STATE_INVALID_OPCODE;
            break;
        }

        break;

    case 0xF:
        switch (KK)
        {
        // LD Vx, DT
        case 0x07:
            printf("LD V%x, DT", X);
            PC += 2;
            V[X] = DelayTimer;
            break;

        // LD Vx, K - Wait for a key press, load key value into Vx
        case 0x0A:
            printf("LD V%x, K");
            PC += 2;
            io_reg = X;
            run_state = CHIP8_STATE_WAIT_FOR_INPUT;
            break;

        // LD DT, Vx
        case 0x15:
            printf("LD DT, V%x", X);
            PC += 2;
            DelayTimer = V[X];
            break;

        // LD ST, Vx
        case 0x18:
            printf("LD ST, V%x", X);
            PC += 2;
            SoundTimer = V[X];
            break;

        // ADD I, Vx - Add Vx to I
        case 0x1E: // ADD I, Vx
            printf("ADD I, V%x", X);
            PC += 2;
            I += V[X];
            break;

        // LD F, Vx - Set I to location of sprite for digit Vx
        case 0x29:
            printf("LD F, V%x", X);
            PC += 2;
            I = SpriteLookup[V[X]];
            break;

        // LD B, Vx - Store BCD of Vx in I, I + 1, and I + 2
        case 0x33:
            PC += 2;
            Memory[I] = (V[X] / 100) % 10;
            Memory[I + 1] = (V[X] / 10) % 10;
            Memory[I + 2] = V[X] % 10;
            break;

        // LD [I], Vx - Store registers Vo through Vx in memory starting at address I
        case 0x55:
            printf("LD [I], V%x", X);
            PC += 2;
            memcpy(&Memory[I], V, X + 1);
            break;

        // LD Vx, [I] - Read registers V0 through Vx from memory starting at address I
        case 0x65:
            printf("LD V%x, [I]", X);
            PC += 2;
            memcpy(V, &Memory[I], X + 1);
            break;

        // Invalid F-type opcode
        default:
            run_state = CHIP8_STATE_INVALID_OPCODE;
            break;
        }

        break;

    // Invalid opcode
    default:
        run_state = CHIP8_STATE_INVALID_OPCODE;
        break;
    }

    printf(" | ");

    // for (int i = 0; i < 16; i++)
    // {
    //     printf("V%x = %d, ", i, V[i]);
    // }

    printf("\r\n");

    if (run_state == CHIP8_STATE_RUNNING || run_state == CHIP8_STATE_WAIT_FOR_INPUT)
    {
        return CHIP8_SUCCESS;
    }
    else
    {
        return CHIP8_ERROR;
    }
}
