#ifndef CHIP8_H
#define CHIP8_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

// === CHIP-8 System Definitions === //
#define CHIP8_MEM_SIZE 4096
#define CHIP8_REG_COUNT 16
#define CHIP_8_STACK_SIZE 16
#define CHIP8_PC_START 0x200
#define CHIP8_SCREEN_WIDTH 64
#define CHIP8_SCREEN_HEIGHT 32

#define CHIP8_ROM_MAX_SIZE (CHIP8_MEM_SIZE - CHIP8_PC_START)

// === Status codes === //
typedef uint8_t chip8_status_e;
#define CHIP8_SUCCESS 0
#define CHIP8_ERROR_ROM_POINTER_NULL 1
#define CHIP8_ERROR_ROM_TOO_BIG 2
#define CHIP8_ERROR_INVALID_OPCODE 3
#define CHIP8_ERROR_UNSUPPORTED_OPCODE 4
#define CHIP8_STATUS_EXIT 5
#define CHIP8_WAITING_FOR_KEYPRESS 6
#define CHIP8_STATUS_RUNNING 7

/**
 * @brief Initialize the CHIP-8 emulator and set up the necessary callback functions for I/O
 * 
 * @param random_byte_func Function that returns a random byte
 * @param draw_byte_func Function that draws a byte on the screen at x, y
 */
void chip8_init(uint8_t (*random_byte_func)(),
                void (*draw_byte_func)(uint8_t, uint8_t, uint8_t),
                bool (*key_pressed_func)(uint8_t),
                void (*clear_screen_func)());

/**
 * @brief Fetch and execute one instruction from memory
 * 
 * @return chip8_status_e CHIP8_SUCCESS if the opcode completed, otherwise an error code
 */
chip8_status_e chip8_cycle();

/**
 * @brief Load a CHIP-8 rom binary into the emulator's system memory starting at location 0x200
 * 
 * @param rom ROM file byte array
 * @param bytes size of the ROM file in bytes
 * @return chip8_status_e CHIP8_STATUS_SUCCESS if ROM is loaded, otherwise an error code corresponding to the problem
 */
chip8_status_e chip8_load_rom(uint8_t *rom, size_t bytes);

/**
 * @brief Alert the CHIP-8 emulator that a key was pressed and that execution should resume
 * 
 * @param key Value of the input key (0-F)
 */
void chip8_key_interrupt(uint8_t key);

/**
 * @brief Decrement the CHIP-8 sound and delay timersby 1 until they reach 0
 * 
 * This should be called at a rate of 60Hz
 */
void chip8_tick_timers();

#endif
