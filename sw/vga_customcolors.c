#include "config.h"
#include "util.h"
#include "vga.h"

void croc_interrupt_handler(uint32_t cause) {
    if (cause == IRQ_VGA) {
        vga_interrupt_clear();
    }
}

const uint16_t rainbow_rgb565[] = {
    0xF0C3, // 0: #F21818 (Red)
    0xF343, // 1: #F26918 (Orange-Red)
    0xF5C3, // 2: #F2BB18 (Orange-Yellow)
    0xD783, // 3: #D6F218 (Yellow-Green)
    0x8783, // 4: #85F218 (Light Green)
    0x3783, // 5: #33F218 (Bright Green)
    0x1F89, // 6: #18F24E (Mint Green)
    0x1F94, // 7: #18F2A0 (Teal Green)
    0x1F9E, // 8: #18F2F2 (Cyan)
    0x1D1E, // 9: #18A0F2 (Light Blue)
    0x1A7E, // 10: #184EF2 (Blue)
    0x30DE, // 11: #3318F2 (Indigo)
    0x80DE, // 12: #8518F2 (Violet)
    0xD0DE, // 13: #D618F2 (Magenta)
    0xF0D7, // 14: #F218BB (Pink-Magenta)
    0xF0CD, // 15: #F21869 (Deep Pink)
};

int main() {
    vga_init();
    vga_clear(3, 11);

    set_interrupt_enable(1, IRQ_VGA);
    set_global_irq_enable(1);
    vga_set_interrupt_enable(true);
    
    for(int i = 0; i < 16; ++i) {
        vga_set_color_palette(i, rainbow_rgb565[i]);
    }

    vga_set_cursor_pos(2, 2);
    vga_print("Hello, Custom Palette! \x01 \x02 \0", 3, 11);

    
    for(int x = 0; x < 32; ++x) {
        vga_set_cursor_pos(2+x, 4);
        vga_print("\xDB", (x % 16), VGA_COLOR_BLACK);
    }

    wfi();
    wfi();
    wfi();
    
    return 0;
}
