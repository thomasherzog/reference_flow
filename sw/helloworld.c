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

// uint32_t font_data[] = {
//     0, 0, 0, 0, 2172518400, 3179381157, 2122416537, 0, 4286447616, 3288334299, 2130706407, 0, 4284874752, 2122252287, 404241468, 0, 471599104, 2139045438, 471612990, 8, 1008205824, 4284880956,1008232191, 126, 404226048, 4286463036, 410976255, 32316, 0, 1010571264, 6204, 0, 4294967295, 3284386791, 4294967271, 4294967295, 1006632960, 1111638630, 15462, 0, 0, 0, 0, 0, 1886912512, 857623640, 506671923, 0, 1715208192, 1013343846, 404258328, 0, 3439067136, 202177740, 118427148, 0, 3338534912, 3334930118, 1743251142, 3, 404226048, 3890691291, 404282172, 0, 50397184, 2139037447, 16975647, 0, 1614807040, 2139061360, 1080062076, 0, 410926104, 404232216, 404232216, 406617624, 1717960704, 1717986918, 1717960806, 0, 3690856448, 3738950619, 3638089944, 0, 107167232, 1667446300, 1664097334, 62, 0, 2130706432, 2139062143, 0, 2117867520, 404232216, 2115517566, 0, 4286463000, 404232216, 404232216, 404232216, 404232216, 404232216, 404232216, 406617855,0, 4285542416, 1060976, 0, 0, 4279110664, 527374, 0, 0, 50529027, 32639, 0, 0, 2134250496, 5174, 0, 134217728, 1042029576, 8355646, 0, 2130706432, 473841279, 526364, 0, 0, 0, 0, 0, 1008205824, 404241468, 404226072, 0, 3334864896, 99, 0, 0, 2134259200, 909522559, 909541247, 0, 504105984, 503513907, 506671152, 3084, 1701249024, 404238391, 3869699084, 0, 907804672, 997071926, 1848849211, 0, 404226048, 12, 0, 0, 404238336, 202116108, 404229132, 48, 404229120, 808464432, 404238384, 12, 0, 1013343744, 1717976319, 0, 0, 2115508224, 1579134, 0, 0, 0, 404226048, 12, 0,2113929216, 126, 0, 0, 0, 404226048, 0, 808452096, 202119192, 50529798, 0, 907804672, 1870361443, 473326439, 0, 471334912, 404232222, 1008211992, 0, 1665007616, 202911840, 2137219846, 0, 1652424704, 1614551088, 1013342304, 0, 943194112, 909524028, 2016444211, 0, 54460416, 1613963011, 506683744, 0, 104595456, 1664556803, 473326435, 0, 1669267456, 405811296, 202116120, 0, 1665007616, 1665033059, 1046700899, 0, 907804672, 1986224995, 506486908, 0, 0, 1579008, 404226048, 0, 0, 1579008, 404226048, 12, 3221225472, 202911840, 3227529240, 0, 0, 8289792, 32382, 0, 50331648, 806882310, 50727960, 0, 1665007616, 405811299, 404226072, 0, 1040187392, 2071683939, 1040399227, 0, 470286336, 1667462974, 1667465059, 0, 1715404800, 1715365478, 1063675494, 0, 1715208192, 50529091, 1013334787, 0, 908001280, 1717986918, 523658854, 0, 1719599104, 371070534, 2137409030, 0, 1719599104, 371070534, 252052998, 0, 1715208192, 1929577283, 1551262563, 0, 1667432448, 1669292899, 1667457891, 0, 406585344, 404232216, 1008211992, 0, 813170688, 808464432, 506671923, 0, 1718026240, 907949622, 1734764086, 0, 101646336, 101058054, 2137409030, 0, 2002976768, 1802207103, 1667457899, 0, 1734541312, 2070900583, 1668510587, 0, 1665007616, 1667457891, 1046700899, 0, 1715404800, 1046898278, 252052998, 0, 907804672, 1667457891, 1014922083, 28720, 1715404800, 1046898278, 1868981814, 0, 1665007616, 940442467, 1046700896, 0, 2122186752, 404232282, 1008211992, 0, 1667432448, 1667457891, 1046700899, 0, 1667432448, 1667457891, 136068707, 0, 1667432448, 1802201955, 909541227, 0, 1667432448, 471610934, 1667446326, 0, 1717960704, 1014916710, 1008211992, 0, 1669267456, 202911841, 2137211654, 0, 205258752, 202116108, 1007422476, 0, 50528256, 202114566, 808458264, 0, 809238528, 808464432, 1009791024, 0, 470286336, 25398, 0, 0, 0, 0, 0, 65535
// };

static volatile int irq_fired      = 0;
static volatile uint32_t irq_cause = 0;

static volatile int count_of_frames = 0;

// Override weak default handler from crt0.S.
// The bootrom trap handler dispatches here via the SRAM function pointer
// at 0x1000_0008, proving the full bootrom→vector table→handler chain works.
void croc_interrupt_handler(uint32_t cause) {
    irq_cause = cause;
    if (cause == IRQ_OBI_TIMER) {
        obi_timer_clear_expired();
        irq_fired = 1;
    } else if (cause == IRQ_VGA) {
        *INTERRUPT_REG &= ~(1 << CONFIG_INTERRUPTED_SHIFT);
        count_of_frames++;
    }
}

int wierd_interrupt_stuff() {
    
    // Check mtvec points to bootrom trap handler (set during boot)
    uint32_t mtvec;
    asm volatile("csrr %0, mtvec" : "=r"(mtvec));
    CHECK_ASSERT(1, mtvec == BOOTROM_TRAP_HANDLER);

    // Check SRAM vector table entries point to our handlers
    uint32_t exc_handler = *(volatile uint32_t *)SRAM_VEC_EXCEPTION;
    uint32_t irq_handler = *(volatile uint32_t *)SRAM_VEC_INTERRUPT;
    CHECK_ASSERT(2, irq_handler == (uint32_t)croc_interrupt_handler);
    CHECK_ASSERT(3, exc_handler != 0);

    // --- Fire interrupt through bootrom trap handler chain ---
    // Flow: OBI timer expires → CPU traps to mtvec (0x0200_0200, bootrom)
    //     → bootrom reads mcause, loads handler from 0x1000_0008
    //     → calls croc_interrupt_handler → sets irq_fired
    //     → bootrom _trap_exit restores regs, mret

    obi_timer_set(500);
    set_interrupt_enable(1, IRQ_OBI_TIMER);
    set_global_irq_enable(1);

    // Wait for interrupt (with timeout)
    for (volatile int i = 0; i < 10000 && !irq_fired; i++)
        ;
    CHECK_ASSERT(4, irq_fired == 1);
    CHECK_ASSERT(5, irq_cause == IRQ_OBI_TIMER);

    // Disable timer and interrupts
    obi_timer_set_enable(0);
    set_interrupt_enable(0, IRQ_OBI_TIMER);
    set_global_irq_enable(0);

    // Verify global IRQs are disabled
    uint32_t mstatus = get_mstatus();
    CHECK_ASSERT(6, (mstatus & 0x8) == 0);

    return 0;
}

int main() {
    uart_init();
    printf("Hello World from socc on croc!\n");
    uart_write_flush();

    volatile char* authors = (volatile char*) (USER_ROM_BASE_ADDR);
    printf("<3 from %s", authors);
    uart_write_flush();


    if(wierd_interrupt_stuff()) {
        return 1;
    }

    printf("Interrupt check done!\n");
    
    set_interrupt_enable(1, IRQ_VGA);
    set_global_irq_enable(1);

    uint8_t config = 0;

    config |= (1 << CONFIG_CLK_DIVIDER_SHIFT); // Divide clock by (1 + 1) = 2
    // config |= (1 << CONFIG_BLINK_DISABLE_SHIFT);
    config &= ~(1 << CONFIG_RAM_ENABLE_SHIFT);
    config |= (1 << CONFIG_INTERRUPT_ENABLE_SHIFT);
    config |= (1 << CONFIG_ENABLE_SHIFT);

    *CONFIG_REG = config;
    *BLINK_REG = 1;

    // int start_char = 0;
    // for(int i = 0; i < sizeof(font_data)/sizeof(uint32_t); i++) {
    //     GLYPH[i + start_char * 4] = font_data[i];
    // }
    
    int idx = 0;
    for(int line = 0; line < 30; line++) {
        for(int col = 0; col < 80; col++) {
            int row = line;
            int idx = line * 80 + col;
            // int color = (idx * 3) % 256;
            int color = (0x0F);
            if((row + col) % 2 ==  0) {
                color = color | (1 << 7);
            }
            int c = idx % 256;
            if(row == 0) {
                c = (col % 10) + '0';
            } else if (col == 0) {
                c = (row % 10) + '0';
            } else {
                int tmp_col = (col - 1) % 16;
                int tmp_row = (row - 1) % 16;

                c = tmp_col * 16 + tmp_row;
                if(row > 16 || col > 16) c = 0;
            }

            TEXT[col + line * 80] = c | (color << 8);
        }
    }

    printf("Waiting!\n");
    uart_write_flush();

    uint32_t x = 0xdeadbeef;
    int8_t snake[10][2];
    int8_t dir = 0;
    for(;;) {
        wfi();
        x ^= x << 13;
        x ^= x >> 17;
        x ^= x << 5;
        
        TEXT[snake[9][0] + snake[9][1] * 80] = 0x0;

            x ^= x << 13;
            x ^= x >> 17;
            x ^= x << 5;
            dir += (x % 3);
        
        dir %= 4;

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

        for(int i = 8; i >= 0; i--) {
            snake[i+1][0] = snake[i][0];
            snake[i+1][1] = snake[i][1];
        }
        for(int i = 0; i < 10; i++) {
            TEXT[snake[i][0] + snake[i][1] * 80] = 0xF000;
        }
    }
    volatile int prev = count_of_frames;
    // int x = 0;

    // while(count_of_frames != 10) {
    //     wfi();
    //     if(prev != count_of_frames) {

    //         int c = TEXT[count_of_frames % (80 * 30)];
    //         int color = c >> 8;
    //         c &= 0xFF;
    //         c |= ((color + x++) % 256) << 8;
    //         TEXT[count_of_frames % (80 * 30)] = c;
    //         prev = count_of_frames;
    //     }
    // };

    printf("Done!\n");
    uart_write_flush();

    return 0;
}
