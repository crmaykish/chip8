#ifndef CHIP8_H
#define CHIP8_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

// === CHIP-8 System Definitions === //
#define CHIP8_MEM_SIZE 0x1000
#define CHIP8_REG_COUNT 16
#define CHIP_8_STACK_SIZE 16
#define CHIP8_PC_START 0x200
#define CHIP8_SCREEN_WIDTH 64
#define CHIP8_SCREEN_HEIGHT 32
#define CHIP8_ROM_MAX_SIZE (CHIP8_MEM_SIZE - CHIP8_PC_START)

// === Status codes === //
typedef uint8_t chip8_status_e;
#define CHIP8_SUCCESS 0
#define CHIP8_ERROR 1

// === Emulator state === //
typedef uint8_t chip8_run_state_e;
#define CHIP8_STATE_STOPPED 0
#define CHIP8_STATE_RUNNING 1
#define CHIP8_STATE_WAIT_FOR_INPUT 2
#define CHIP8_STATE_INVALID_OPCODE 3
#define CHIP8_STATE_UNSUPPORTED_OPCODE 4

// === Screen drawing options === //
typedef uint8_t chip8_screen_redraw_type_e;
#define CHIP8_REDRAW_SCREEN_CLEAR 0
#define CHIP8_REDRAW_SCREEN_FULL 1

// === Pixel states === //
typedef uint8_t chip8_pixel_state_e;
#define CHIP8_PIXEL_OFF 0
#define CHIP8_PIXEL_ON 1

// === Interface functions === //
typedef uint8_t (*chip8_random_byte_ft)();
typedef void (*chip8_set_pixel_ft)(uint8_t, uint8_t, chip8_pixel_state_e);
typedef void (*chip8_clear_screen_ft)();

void chip8_init();

chip8_status_e chip8_cycle();

chip8_run_state_e chip8_get_run_state();

chip8_pixel_state_e *chip8_get_screen();

void chip8_press_key(uint8_t key);

void chip8_set_random_byte_func(chip8_random_byte_ft f);

void chip8_set_set_pixel_func(chip8_set_pixel_ft f);

void chip8_set_clear_screen_func(chip8_clear_screen_ft f);

/**
 * @brief Load a CHIP-8 rom binary into the emulator's system memory starting at location 0x200
 * 
 * @param rom ROM file byte array
 * @param bytes size of the ROM file in bytes
 * @return chip8_status_e CHIP8_STATUS_SUCCESS if ROM is loaded, otherwise an error code corresponding to the problem
 */
chip8_status_e chip8_load_rom(uint8_t *rom, size_t bytes);

/**
 * @brief Decrement the CHIP-8 sound and delay timers by 1 until they reach 0
 * 
 * This should be called at a rate of 60Hz
 */
void chip8_tick_timers();

#endif
