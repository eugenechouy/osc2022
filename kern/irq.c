#include "peripheral/aux.h"
#include "peripheral/uart.h"
#include "peripheral/interrupt.h"
#include "kern/timer.h"
#include "kern/irq.h"
#include "kern/sched.h"

// QA7_rev3.4 p.7
#define CORE0_IRQ_SRC ((volatile unsigned int*)(0x40000060))

// QA7_rev3.4 p.16
#define CNTPNSIRQ_INT   1
#define GPU_INT         8

void int_enable() {
    asm volatile("msr DAIFClr, 0xf");
}

void int_disable() {
    asm volatile("msr DAIFSet, 0xf");
}

void irq_main(int el_from) {
    int_disable();
    if (*CORE0_IRQ_SRC & (1 << CNTPNSIRQ_INT)) { // Timer interrupt
        if (el_from == 0) 
            task_create(timer_el0_handler, 0, 0);
        else if (el_from == 1) 
            task_create(timer_el1_handler, 0, 0);
        else 
            task_create(timer_unknown_handler, 0, 0);
        core_timer_disable();
    } else if (*CORE0_IRQ_SRC & (1 << GPU_INT)) { // GPU interrupt
        if (*IRQ_PENDING_1 & AUX_INT) {
            task_create(uart_handler, 0, 1);
            uart_disable_int();
        }
    }
    int_enable();
    task_run();
    uart_write_flush();
}