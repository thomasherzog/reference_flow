/**
 * @file VGA.H
 * @brief VGA Display Driver Library
 * 
 * Provides a simple text-based interface for VGA display output.
 * Supports 80x30 character terminal with 16 colors and custom glyph loading.
 * 
 * Memory Layout:
 * - 0x04000000: Color Palette Buffer (32 bytes, 16 colors × 2 bytes each)
 * - 0x04000020: Configuration Register
 * - 0x04000021: Interrupt Status Register
 * - 0x04000000 + RAM_SIZE: Text Buffer (80×30 characters = 4800 bytes)
 * - 0x04000000 + RAM_SIZE×2: Glyph Buffer (256 glyphs × 16 bytes = 4096 bytes)
 * 
 * Quick Start:
 *   vga_init();
 *   vga_set_cursor_pos(0, 0);
 *   vga_print("Hello, World!", VGA_COLOR_WHITE, VGA_COLOR_BLACK);
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/* ========== VGA Configuration ========== */

/**
 * Base address of VGA display buffer in memory.
 * All VGA registers and buffers are mapped relative to this address.
 */
#define VGA_BASE_ADDRESS 0x04000000

/**
 * Total VGA RAM size in bytes (8192 bytes = 2^13).
 * Shared among text buffer, glyph buffer, and color palette.
 */
#define VGA_RAM_SIZE (1 << (11 + 2))

/**
 * Display width in characters (80 columns).
 */
#define VGA_WIDTH  80

/**
 * Display height in characters (30 rows).
 */
#define VGA_HEIGHT 30

/**
 * Configuration register bit shifts.
 * Used internally by configuration functions.
 */
#define VGA_CONFIG_CLK_DIVIDER_SHIFT      0  /**< Clock divider setting (bits 0-3) */
#define VGA_CONFIG_BLINK_DISABLE_SHIFT    4  /**< Blink disable flag (bit 4) */
#define VGA_CONFIG_RAM_ENABLE_SHIFT       5  /**< Custom glyph RAM enable (bit 5) */
#define VGA_CONFIG_INTERRUPT_ENABLE_SHIFT 6  /**< Interrupt enable flag (bit 6) */
#define VGA_CONFIG_ENABLE_SHIFT           7  /**< VGA output enable flag (bit 7) */

/**
 * Set the VGA clock divider for timing control.
 * @param divider Clock divider value (4-bit, 0-15)
 */
void vga_set_clock_divider(uint8_t divider);

/**
 * Enable or disable cursor/text blinking.
 * @param disable_blink true to disable blinking, false to enable
 */
void vga_set_blink_disable(bool disable_blink);

/**
 * Enable or disable custom glyph RAM for custom character sets.
 * Must be enabled before using vga_load_glyph().
 * @param glyph_ram_enable true to enable custom glyphs, false for default font
 */
void vga_set_glyph_ram_enable(bool glyph_ram_enable);

/**
 * Enable or disable VGA interrupts.
 * When enabled, VGA generates interrupts (must be cleared with vga_interrupt_clear()).
 * @param interrupt_enable true to enable interrupts, false to disable
 */
void vga_set_interrupt_enable(bool interrupt_enable);

/**
 * Enable or disable VGA video output.
 * @param output_enable true to enable display output, false to disable
 */
void vga_set_output_enable(bool output_enable);

/* ========== VGA Operation ========== */

/**
 * VGA Color Palette (16 colors: 8 standard + 8 bright variants).
 * 
 * Standard Colors (0-7):
 *   BLACK, BLUE, GREEN, CYAN, RED, MAGENTA, BROWN, LIGHT_GRAY
 * 
 * Bright Colors (8-15):
 *   DARK_GRAY, LIGHT_BLUE, LIGHT_GREEN, LIGHT_CYAN,
 *   LIGHT_RED, LIGHT_MAGENTA, YELLOW, WHITE
 */
typedef enum {
    VGA_COLOR_BLACK = 0,          /**< Standard black (index 0) */
    VGA_COLOR_BLUE = 1,           /**< Standard blue (index 1) */
    VGA_COLOR_GREEN = 2,          /**< Standard green (index 2) */
    VGA_COLOR_CYAN = 3,           /**< Standard cyan (index 3) */
    VGA_COLOR_RED = 4,            /**< Standard red (index 4) */
    VGA_COLOR_MAGENTA = 5,        /**< Standard magenta (index 5) */
    VGA_COLOR_BROWN = 6,          /**< Standard brown (index 6) */
    VGA_COLOR_LIGHT_GRAY = 7,     /**< Standard light gray (index 7) */
    VGA_COLOR_DARK_GRAY = 8,      /**< Bright dark gray (index 8) */
    VGA_COLOR_LIGHT_BLUE = 9,     /**< Bright blue (index 9) */
    VGA_COLOR_LIGHT_GREEN = 10,   /**< Bright green (index 10) */
    VGA_COLOR_LIGHT_CYAN = 11,    /**< Bright cyan (index 11) */
    VGA_COLOR_LIGHT_RED = 12,     /**< Bright red (index 12) */
    VGA_COLOR_LIGHT_MAGENTA = 13, /**< Bright magenta (index 13) */
    VGA_COLOR_YELLOW = 14,        /**< Bright yellow (index 14) */
    VGA_COLOR_WHITE = 15          /**< Bright white (index 15) */
} vga_color_t;

/**
 * Initialize the VGA display.
 * Must be called before using any other VGA functions.
 * 
 * Actions performed:
 *   - Resets cursor position to (0, 0)
 *   - Clears screen with white text on black background
 *   - Enables VGA output
 *   - Disables text blinking
 */
void vga_init(void);

/**
 * Clear the entire display with specified colors.
 * Does not reset cursor position.
 * 
 * @param foreground_color Default text color for cleared area
 * @param background_color Background fill color
 * 
 * Example:
 *   vga_clear(VGA_COLOR_WHITE, VGA_COLOR_BLACK);  // White text on black
 */
void vga_clear(vga_color_t foreground_color, vga_color_t background_color);

/**
 * Write a single character at the current cursor position.
 * Automatically advances cursor to the next column.
 * Does nothing if cursor is out of bounds.
 * 
 * @param character ASCII character code to display
 * @param foreground_color Text color (0-15)
 * @param background_color Background color (0-15)
 */
void vga_putc(uint8_t character, vga_color_t foreground_color, vga_color_t background_color);

/**
 * Write a null-terminated string at the current cursor position.
 * Handles line wrapping and automatic scrolling when text reaches bottom.
 * 
 * @param text Null-terminated string to display
 * @param foreground_color Text color (0-15)
 * @param background_color Background color (0-15)
 * 
 * Behavior:
 *   - Wraps to next line when reaching end of row
 *   - Scrolls display when reaching bottom of screen
 *   - Cursor advances one position per character
 * 
 * Example:
 *   vga_print("Status: OK", VGA_COLOR_WHITE, VGA_COLOR_BLACK);
 */
void vga_print(const char *text, vga_color_t foreground_color, vga_color_t background_color);

/**
 * Set the cursor position to specified coordinates.
 * Does nothing if coordinates are out of bounds.
 * 
 * @param x Column (0 to 79)
 * @param y Row (0 to 29)
 * 
 * Example:
 *   vga_set_cursor_pos(10, 5);  // Column 10, row 5
 */
void vga_set_cursor_pos(int x, int y);

/**
 * Scroll display up by one line, clearing the bottom row.
 * Fills the new bottom line with blank characters in specified colors.
 * Automatically called by vga_print() when text reaches screen bottom.
 * 
 * @param foreground_color Text color for new bottom line
 * @param background_color Background color for new bottom line
 */
void vga_scroll(vga_color_t foreground_color, vga_color_t background_color);


/* ========== Glyph Management ========== */

/**
 * Custom character glyph bitmap (8x16 pixels per character).
 * Each byte represents one row of the 8-pixel-wide character.
 * Most significant bit = leftmost pixel, LSB = rightmost pixel.
 */
typedef struct {
    uint8_t data[16];  /**< 16 bytes representing 8x16 bitmap */
} glyph_data_t;

/**
 * Load a custom glyph (character bitmap) into glyph RAM.
 * Allows overriding the default font with custom character designs.
 * 
 * @param char_code Character code to override (0-255)
 * @param data Pointer to glyph_data_t containing 16-byte bitmap
 * 
 * Bitmap format (for each byte):
 *   Bit 7 = leftmost pixel, Bit 0 = rightmost pixel
 *   1 = foreground color, 0 = background color
 * 
 * Example: Load a custom 'A' character
 *   glyph_data_t custom_a = {
 *       .data = {0x18, 0x24, 0x42, 0x7E, 0x81, 0x81, 0x81, 0x81,
 *                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
 *   };
 *   vga_set_glyph_ram_enable(true);
 *   vga_load_glyph(65, &custom_a);  // 65 = 'A'
 */
void vga_load_glyph(uint8_t char_code, const glyph_data_t *data);

/* ========== Interrupt Handling ========== */

/**
 * Clear the VGA interrupt flag.
 * Must be called in the VGA interrupt handler to acknowledge the interrupt.
 * Only has effect if interrupts are enabled via vga_set_interrupt_enable(true).
 * 
 * Typical interrupt handler:
 *   void vga_isr() {
 *       // Handle VGA interrupt here
 *       vga_interrupt_clear();  // Acknowledge interrupt
 *   }
 */
void vga_interrupt_clear();

/* ========== Color Palette Management ========== */

/**
 * Customize a color in the 16-color palette.
 * Allows runtime color adjustment for all on-screen elements.
 * 
 * @param color_index Palette index (0-15, corresponds to vga_color_t values)
 * @param color_data RGB565 color value (16-bit)
 * 
 * RGB565 Format:
 *   Bits 15-11: Red (5 bits, 0-31)
 *   Bits 10-5:  Green (6 bits, 0-63)
 *   Bits 4-0:   Blue (5 bits, 0-31)
 * 
 * Examples:
 *   vga_set_color_palette(0, 0x0000);  // Black
 *   vga_set_color_palette(4, 0xF800);  // Red
 *   vga_set_color_palette(2, 0x07E0);  // Green
 *   vga_set_color_palette(3, 0x07FF);  // Cyan
 *   vga_set_color_palette(15, 0xFFFF); // White
 * 
 * Note: Changes affect all text immediately, including text already on display.
 */
void vga_set_color_palette(uint8_t color_index, uint16_t color_data);