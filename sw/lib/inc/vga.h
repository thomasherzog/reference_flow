#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// VGA Configuration

#define VGA_BASE_ADDRESS 0x04000000
#define VGA_RAM_SIZE (1 << (11 + 2))

#define VGA_WIDTH  80
#define VGA_HEIGHT 30

#define VGA_CONFIG_CLK_DIVIDER_SHIFT      0
#define VGA_CONFIG_BLINK_DISABLE_SHIFT    4
#define VGA_CONFIG_RAM_ENABLE_SHIFT       5
#define VGA_CONFIG_INTERRUPT_ENABLE_SHIFT 6
#define VGA_CONFIG_ENABLE_SHIFT           7

void vga_set_clock_divider(uint8_t divider);
void vga_set_blink_disable(bool disable_blink);
void vga_set_glyph_ram_enable(bool glyph_ram_enable);
void vga_set_interrupt_enable(bool interrupt_enable);
void vga_set_output_enable(bool output_enable);

// VGA Operation

typedef enum {
    VGA_COLOR_BLACK = 0,
    VGA_COLOR_BLUE = 1,
    VGA_COLOR_GREEN = 2,
    VGA_COLOR_CYAN = 3,
    VGA_COLOR_RED = 4,
    VGA_COLOR_MAGENTA = 5,
    VGA_COLOR_BROWN = 6,
    VGA_COLOR_LIGHT_GRAY = 7,
    VGA_COLOR_DARK_GRAY = 8,
    VGA_COLOR_LIGHT_BLUE = 9,
    VGA_COLOR_LIGHT_GREEN = 10,
    VGA_COLOR_LIGHT_CYAN = 11,
    VGA_COLOR_LIGHT_RED = 12,
    VGA_COLOR_LIGHT_MAGENTA = 13,
    VGA_COLOR_YELLOW = 14,
    VGA_COLOR_WHITE = 15
} vga_color_t;

void vga_init(void);

void vga_clear(vga_color_t foreground_color, vga_color_t background_color);

void vga_putc(uint8_t character, vga_color_t foreground_color, vga_color_t background_color);

void vga_print(const char *text, vga_color_t foreground_color, vga_color_t background_color);

void vga_set_cursor_pos(int x, int y);

void vga_scroll(vga_color_t foreground_color, vga_color_t background_color);

// Glyph Management

typedef struct {
    uint8_t data[16];
} glyph_data_t;

void vga_load_glyph(uint8_t char_code, const glyph_data_t *data);

// Interrupt Handling

void vga_interrupt_clear();