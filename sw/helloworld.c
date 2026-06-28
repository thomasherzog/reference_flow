// Copyright (c) 2024 ETH Zurich and University of Bologna.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0
//
// Authors:
// - Philippe Sauter <phsauter@iis.ee.ethz.ch>

#include "uart.h"
#include "print.h"
#include "config.h"
#include "util.h"
#include "obi_timer.h"

// Bootrom trap handler address (set as mtvec by bootrom during boot)
#define BOOTROM_TRAP_HANDLER 0x02000200

// SRAM vector table addresses (read by bootrom trap handler)
#define SRAM_VEC_EXCEPTION   0x10000004
#define SRAM_VEC_INTERRUPT   0x10000008

#define BASE_ADDRESS 0x04000000
#define RAM_SIZE (1 << (11 + 2))
#define CONFIG_CLK_DIVIDER_SHIFT 0
// #define CONFIG_BLINK_DISABLE_SHIFT 4
#define CONFIG_RAM_ENABLE_SHIFT 5
#define CONFIG_INTERRUPT_ENABLE_SHIFT 6
#define CONFIG_ENABLE_SHIFT 7
#define CONFIG_INTERRUPTED_SHIFT 0

#define IRQ_VGA 20

volatile uint16_t * const COLOR_PALETTE = (volatile uint16_t*const) (BASE_ADDRESS);
volatile uint8_t * const CONFIG_REG = (volatile uint8_t*const) (BASE_ADDRESS + 0x20);
volatile uint8_t * const INTERRUPT_REG = (volatile uint8_t*const) (BASE_ADDRESS + 0x21);
volatile uint8_t * const BLINK_REG = (volatile uint8_t*const) (BASE_ADDRESS + 0x22);
volatile uint16_t * const TEXT = (volatile uint16_t*const) (BASE_ADDRESS + RAM_SIZE);
volatile uint32_t * const GLYPH = (volatile uint32_t*const) (BASE_ADDRESS + RAM_SIZE * 2);

static volatile int count_of_frames = 0;

#define SNAKE_LEN 10
int8_t snake[SNAKE_LEN][4];


// XorShift128State can alternatively be defined as a pair
// of uint64_t or a uint128_t where supported
typedef struct {
    uint32_t x[4];
} XorShift128State;

XorShift128State rng = {
    {0xdeadbeef, 0x2242424,0x52f3424,0x2adfb92}
};

// The state must be initialized to non-zero
uint32_t xorshift128(XorShift128State* state) {
	// Algorithm "xor128" from p. 5 of Marsaglia, "Xorshift RNGs"
	uint32_t t = state->x[3];
    
    uint32_t s = state->x[0]; // Perform a contrived 32-bit shift.
	state->x[3] = state->x[2];
	state->x[2] = state->x[1];
	state->x[1] = s;

	t ^= t << 11;
	t ^= t >> 8;
	return state->x[0] = t ^ s ^ (s >> 19);
}

void draw_snake() {
    for(int i = 0; i < SNAKE_LEN; i++) {
        uint8_t x = snake[i][0];
        uint8_t y = snake[i][1];
        uint8_t ascii = snake[i][2];
        uint8_t color = snake[i][3];
        
        TEXT[x + y * 80] = ascii | (((uint16_t)color) << 8);
    }
}

void move_snake(uint8_t dir) {
    for(int i = SNAKE_LEN-2; i >= 0; i--) { 
        // only copy position - not color
        snake[i+1][0] = snake[i][0];
        snake[i+1][1] = snake[i][1];
    }

    if(dir == 0 || dir == 2) {
        snake[0][0] += dir == 0 ? 1 : -1;
    } else {
        snake[0][1] += dir == 1 ? 1 : -1;
    }
    if(snake[0][0] < 0) {
        snake[0][0] += 80;
    }
    if(snake[0][1] < 0) {
        snake[0][1] += 30;
    }
    snake[0][0] %= 80;
    snake[0][1] %= 30;
}

void randomize_snake_color(uint8_t mode) {
    if(mode == 0) {
        int x = xorshift128(&rng);
        int idx = (x >> 16) % SNAKE_LEN;
        snake[idx][2] = x;
        snake[idx][3] = (x >> 8) & 0x7F;
    } else if(mode == 1) {
        int x = xorshift128(&rng);
        for(int i = 0; i < SNAKE_LEN - 1; i++) {
            snake[i][2] = x;
            snake[i][3] = (x >> 8 )& 0x7F;
        }
    } else {
        for(int i = 0; i < SNAKE_LEN - 1; i++) {
            int x = xorshift128(&rng);
            snake[i][2] = x;
            snake[i][3] = (x >> 8 )& 0x7F;
        }
    }
    
    // Last snake element is always "fake black" to hide trail
    snake[SNAKE_LEN-1][2] = snake[SNAKE_LEN-1][3] = 0x0;
}

// Override weak default handler from crt0.S.
// The bootrom trap handler dispatches here via the SRAM function pointer
// at 0x1000_0008, proving the full bootrom→vector table→handler chain works.
void croc_interrupt_handler(uint32_t cause) {
    if (cause == IRQ_VGA) {
        *INTERRUPT_REG &= ~(1 << CONFIG_INTERRUPTED_SHIFT);
        count_of_frames++;
    }
}

int main() {
    volatile char* authors = (volatile char*) (USER_ROM_BASE_ADDR);
    uart_init();
    printf("Authors: %s\n", authors);
    uart_write_flush();

    for(int i = 0; i < 100; i++) {
        xorshift128(&rng);
    }
    for(int i = 0; i < 80*30; i++) {
        TEXT[i] = 0;
    }

    set_interrupt_enable(1, IRQ_VGA);
    set_global_irq_enable(1);

    uint8_t config = 0;

    config |= (1 << CONFIG_CLK_DIVIDER_SHIFT); // Divide clock by (1 + 1) = 2
    config &= ~(1 << CONFIG_RAM_ENABLE_SHIFT);
    config |= (1 << CONFIG_INTERRUPT_ENABLE_SHIFT);
    config |= (1 << CONFIG_ENABLE_SHIFT);

    *CONFIG_REG = config;
    *BLINK_REG = 60;

    char * text_string = "!PLEASE TAPE ME!";
    int jada = 40 - 8;
    while(*text_string != 0) {
        TEXT[jada + 12 * 80] = *text_string | (0x80 << 8) | (xorshift128(&rng) << 12) | ((jada & 0x0F) << 8);
        TEXT[jada + 14 * 80] = *text_string | (0x80 << 8) | (xorshift128(&rng) << 12) | (((~xorshift128(&rng)) & 0x0F) << 8);
        TEXT[jada + 16 * 80] = *text_string | (0x80 << 8) | (xorshift128(&rng) << 12) | (((~xorshift128(&rng)) & 0x0F) << 8);

        jada++;
        text_string++;
    }

    printf("Running!\n");
    uart_write_flush();
    
    randomize_snake_color(2);
    snake[0][2] = '@';

    int8_t dir = 0;
    for(int kappa = 0; kappa < 100; kappa++) {        
        if(uart_read_ready()) {
            int r = uart_read();
            if(r == 'd') {
                dir = 0;
            } else if(r == 's') {
                dir = 1;
            } else if(r == 'a') {
                dir = 2;
            } else if(r == 'w') {
                dir = 3;
            } else if(r == 'j') {
                randomize_snake_color(0);
            } else if(r == 'k') {
                randomize_snake_color(1);
            } else if(r == 'l') {
                randomize_snake_color(2);
            } else {
                printf("Got me!\n");
            }
        }
        
        for(int k = 0; k < 4; k++)
            wfi();
        draw_snake();
        move_snake(dir);
    }
    while(1) wfi();
    return 0;
}
