#include "peripheral/aux.h"
#include "peripheral/uart.h"
#include "peripheral/interrupt.h"
#include "kern/timer.h"

// QA7_rev3.4 p.7
#define CORE0_IRQ_SRC ((volatile unsigned int*)(0x40000060))

// QA7_rev3.4 p.16
#define CNTPNSIRQ_INT   1
#define GPU_INT         8

void irq_main(int el_from) {
    if (*CORE0_IRQ_SRC & (1 << CNTPNSIRQ_INT)) { // Timer interrupt
        if (el_from == 0) {
            timer_el0_handler();
        } else if (el_from == 1) {
            timer_el1_handler();
        } else {
            uart_puts("Unknown source el\n");
        }
    } else if (*CORE0_IRQ_SRC & (1 << GPU_INT)) { // GPU interrupt
        if (*IRQ_PENDING_1 & AUX_INT)
            uart_handler();
    }
}