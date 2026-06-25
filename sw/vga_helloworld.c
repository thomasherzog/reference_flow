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
    vga_clear(VGA_COLOR_WHITE, VGA_COLOR_BLUE);

    set_interrupt_enable(1, IRQ_VGA);
    set_global_irq_enable(1);
    vga_set_interrupt_enable(true);
    
    wfi();

    vga_set_cursor_pos(27, 12);
    vga_print("Hello, VGA Text Mode! \x01 \x02 \0", VGA_COLOR_YELLOW, VGA_COLOR_BLUE);
    
    for(int x = 0; x < 8; ++x) {
        vga_set_cursor_pos(27+x, 14);
        vga_print("\xDB", x, VGA_COLOR_BLACK);
    }    

    for(int x = 0; x < 8; ++x) {
        vga_set_cursor_pos(27+x, 16);
        vga_print("\xDB", (x + 8), VGA_COLOR_BLACK);
    }    

    wfi();
    wfi();
    
    return 0;
}
