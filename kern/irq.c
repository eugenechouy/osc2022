#include "peripheral/aux.h"
#include "peripheral/uart.h"
#include "peripheral/interrupt.h"

// QA7_rev3.4 p.7
#define CORE0_IRQ_SRC ((volatile unsigned int*)(0x40000060))

// QA7_rev3.4 p.16
#define CNTPNSIRQ_INT   1
#define GPU_INT         8

void timer_handler() {
    unsigned long cntpct;
    unsigned long cntfrq;

    asm volatile("mrs %0, cntpct_el0" : "=r"(cntpct));
    asm volatile("mrs %0, cntfrq_el0" : "=r"(cntfrq));
    uart_puts("Seconds after booting: ");
    uart_printNum((long)cntpct/cntfrq, 10);
    uart_puts("\n");

    asm volatile(
        "mrs x0, cntfrq_el0     \n\t"
        "mov x1, 2              \n\t"
        "mul x0, x0, x1         \n\t"
        "msr cntp_tval_el0, x0  \n\t"
    );
}

void irq_main() {
    if (*CORE0_IRQ_SRC & (1 << CNTPNSIRQ_INT)) { // Timer interrupt
        timer_handler();
    } else if (*CORE0_IRQ_SRC & (1 << GPU_INT)) { // GPU interrupt
        if (*IRQ_PENDING_1 & AUX_INT)
            uart_handler();
    }
}