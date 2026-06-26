#include "config.h"
#include "util.h"
#include "vga.h"

void croc_interrupt_handler(uint32_t cause) {
    if (cause == IRQ_VGA) {
        vga_interrupt_clear();
    }
}

int main() {
    vga_init();

    set_interrupt_enable(1, IRQ_VGA);
    set_global_irq_enable(1);
    vga_set_interrupt_enable(true);
    
    vga_set_cursor_pos(5, 3);
    vga_print("0 1 2 3 4 5 6 7 8 9 A B C D E F", VGA_COLOR_YELLOW, VGA_COLOR_BLACK);

    for (int row = 0; row < 16; row++) {
        vga_set_cursor_pos(2, row + 5);
        uint8_t row_label = (row < 10) ? ('0' + row) : ('A' + row - 10);
        vga_putc(row_label, VGA_COLOR_YELLOW, VGA_COLOR_BLACK);
        for (int col = 0; col < 16; col++) {
            vga_set_cursor_pos(5 + (col * 2), row + 5);
            uint8_t char_code = (row << 4) | col;
            vga_putc(char_code, VGA_COLOR_WHITE, VGA_COLOR_BLACK);
        }
    }

    wfi();
    wfi();
    wfi();
    
    return 0;
}