#include "vga.h"

static volatile uint16_t* const VGA_COLOR_PALETTE_BUFFER = (volatile uint16_t*) (VGA_BASE_ADDRESS);
static volatile uint8_t* const VGA_CONFIG_BUFFER = (volatile uint8_t*) (VGA_BASE_ADDRESS + 0x20);
static volatile uint8_t* const VGA_INTERRUPT_BUFFER = (volatile uint8_t*) (VGA_BASE_ADDRESS + 0x21);
static volatile uint16_t* const VGA_TEXT_BUFFER = (volatile uint16_t*) (VGA_BASE_ADDRESS + VGA_RAM_SIZE);
static volatile uint8_t* const VGA_GLYPH_BUFFER = (volatile uint8_t*) (VGA_BASE_ADDRESS + VGA_RAM_SIZE * 2);

static int cursor_x = 0;
static int cursor_y = 0;

static uint16_t vga_text_entry(const uint8_t character, const uint8_t combined_color) {
    return (uint16_t) character | (uint16_t) combined_color << 8;
}

static uint8_t vga_combine_colors(const vga_color_t foreground_color, const vga_color_t background_color) {
    return foreground_color | background_color << 4;
}

// Configuration

void vga_set_clock_divider(uint8_t divider) {
    uint8_t config = *VGA_CONFIG_BUFFER;
    config &= ~(0x0F);
    config |= (divider & 0x0F);
    *VGA_CONFIG_BUFFER = config;
}

void vga_set_blink_disable(bool disable_blink) {
    uint8_t config = *VGA_CONFIG_BUFFER;
    config &= ~(0x01 << 4);
    config |= ((disable_blink & 0x01) << 4);
    *VGA_CONFIG_BUFFER = config;
}

void vga_set_glyph_ram_enable(bool glyph_ram_enable){
    uint8_t config = *VGA_CONFIG_BUFFER;
    config &= ~(0x01 << 5);
    config |= ((glyph_ram_enable & 0x01) << 5);
    *VGA_CONFIG_BUFFER = config;
}

void vga_set_interrupt_enable(bool interrupt_enable) {
    uint8_t config = *VGA_CONFIG_BUFFER;
    config &= ~(0x01 << 6);
    config |= ((interrupt_enable & 0x01) << 6);
    *VGA_CONFIG_BUFFER = config;
}

void vga_set_output_enable(bool output_enable){
    uint8_t config = *VGA_CONFIG_BUFFER;
    config &= ~(0x01 << 7);
    config |= ((output_enable & 0x01) << 7);
    *VGA_CONFIG_BUFFER = config;
}

// VGA Operation

void vga_init(void) {
    cursor_x = 0;
    cursor_y = 0;
    vga_clear(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    vga_set_output_enable(true);
    vga_set_blink_disable(true);
}

void vga_clear(const vga_color_t foreground_color, const vga_color_t background_color) {
    const uint8_t color = vga_combine_colors(foreground_color, background_color);
    const uint16_t blank_entry = vga_text_entry(0, color);
    for (int y = 0; y < VGA_HEIGHT; y++) {
        for (int x = 0; x < VGA_WIDTH; x++) {
            VGA_TEXT_BUFFER[y * VGA_WIDTH + x] = blank_entry;
        }
    }
}

void vga_putc(uint8_t character, vga_color_t foreground_color, vga_color_t background_color) {
    if (cursor_x < 0 || cursor_x >= VGA_WIDTH || cursor_y < 0 || cursor_y >= VGA_HEIGHT) {
        return;
    }

    const uint8_t color = vga_combine_colors(foreground_color, background_color);
    const uint16_t entry = vga_text_entry(character, color);
    VGA_TEXT_BUFFER[cursor_y * VGA_WIDTH + cursor_x] = entry;
    cursor_x++;
}

void vga_print(const char *text, const vga_color_t foreground_color, const vga_color_t background_color) {
    for (size_t i = 0; text[i] != '\0'; i++) {
        if (cursor_x >= VGA_WIDTH) {
            cursor_x = 0;
            cursor_y++;
        }

        if (cursor_y >= VGA_HEIGHT) {
            vga_scroll(foreground_color, background_color);
            cursor_y = VGA_HEIGHT - 1;
        }

        vga_putc(text[i], foreground_color, background_color);
    }
}

void vga_set_cursor_pos(const int x, const int y) {
    if (x >= 0 && x < VGA_WIDTH && y >= 0 && y < VGA_HEIGHT) {
        cursor_x = x;
        cursor_y = y;
    }
}

void vga_scroll(const vga_color_t foreground_color, const vga_color_t background_color) {
    for (int y = 1; y < VGA_HEIGHT; y++) {
        for (int x = 0; x < VGA_WIDTH; x++) {
            VGA_TEXT_BUFFER[(y - 1) * VGA_WIDTH + x] = VGA_TEXT_BUFFER[y * VGA_WIDTH + x];
        }
    }

    const uint8_t color = vga_combine_colors(foreground_color, background_color);
    const uint16_t blank_entry = vga_text_entry(0, color);
    for (int x = 0; x < VGA_WIDTH; x++) {
        VGA_TEXT_BUFFER[(VGA_HEIGHT - 1) * VGA_WIDTH + x] = blank_entry;
    }
}

// Glyph Management

void vga_load_glyph(uint8_t char_code, const glyph_data_t *data) {
    size_t memory_offset = char_code * 16;
    for (size_t i = 0; i < 16; i++) {
        VGA_GLYPH_BUFFER[memory_offset + i] = data->data[i];
    }
}

// Interrupt Handling

void vga_interrupt_clear() {
    *VGA_INTERRUPT_BUFFER &= ~(1);
}

// Color Palette Management

void vga_set_color_palette(uint8_t color_index, uint16_t color_data) {
    VGA_COLOR_PALETTE_BUFFER[color_index] = color_data;
}