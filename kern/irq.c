#include "peripheral/aux.h"
#include "peripheral/uart.h"
#include "peripheral/interrupt.h"
#include "peripheral/arm.h"
#include "kern/timer.h"
#include "kern/irq.h"
#include "kern/sched.h"
#include "kern/softirq.h"

// QA7_rev3.4 p.16
#define CNTPNSIRQ_INT   1
#define GPU_INT         8

char int_stack[4096];

void int_enable() {
    asm volatile("msr DAIFClr, 0xf");
}

void int_disable() {
    asm volatile("msr DAIFSet, 0xf");
}

void timer_int_handler() {
    struct task_struct *current = get_current();
    if (--current->ctime <= 0) {
       current->resched = 1;
    }
    timer_sched_latency();
    timer_enable_int();
}

void int_init() {
    softirq_init();
    softirq_register(timer_int_handler, SOFTIRQ_TIMER);
    softirq_register(uart_int_handler, SOFTIRQ_UART);
    int_enable();
}

void irq_router() {
    int_disable();
    if (*CORE0_IRQ_SRC & (1 << CNTPNSIRQ_INT)) {
        timer_disable_int();
        softirq_active(SOFTIRQ_TIMER);
    } else if (*CORE0_IRQ_SRC & (1 << GPU_INT)) {
        if (*IRQ_PENDING_1 & AUX_INT) {
            uart_disable_int();
            softirq_active(SOFTIRQ_UART);
        }
    }
    int_enable();
    softirq_run();
}

void irq_main() {
    register char *sp;
    asm volatile("mov %0, sp": "=r"(sp));
    if (!(sp <= &int_stack[4095] && sp >= &int_stack[0])) {
        asm volatile("mov sp, %0" : : "r"(&int_stack[4080]));
    }
    
    irq_router();
    
    if (!(sp <= &int_stack[4095] && sp >= &int_stack[0])) {
        asm volatile("mov sp, %0" : : "r"(sp));
    }
}

void irq_resched() {
    struct task_struct *current = get_current();
    if (current->resched) {
        current->ctime = 1;
        current->resched = 0;
        schedule();
    }
}