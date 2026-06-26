#include "config.h"
#include "util.h"
#include "vga.h"
#include "obi_timer.h"
#include "print.h"
#include "uart.h"

static volatile uint8_t frame_count = 0;

void croc_interrupt_handler(uint32_t cause) {
    if (cause == IRQ_VGA) {
        vga_interrupt_clear();
        frame_count++;
    }
}

int main() {
    uart_init();
    printf("socc on croc - interrupt test\n");
    uart_write_flush();

    vga_init();
    vga_clear(VGA_COLOR_WHITE, VGA_COLOR_BLUE);

    set_interrupt_enable(1, IRQ_VGA);
    set_global_irq_enable(1);
    vga_set_interrupt_enable(true);

    frame_count = 0;
    char text[8] = "Frame X ";

    while(frame_count < 5) {
        printf("Waiting for interrupt %x\n", frame_count);
        uart_write_flush();
        wfi();
        text[6] = frame_count + 0x30;
        vga_print(text, VGA_COLOR_RED, VGA_COLOR_BLUE);
    }

    wfi();
    wfi();
    wfi();
    return 0;
}