#ifndef SOFTIRQ_H
#define SOFTIRQ_H

#define SOFTIRQ_NUM 64

#define SOFTIRQ_TIMER 1
#define SOFTIRQ_UART  2

enum irq_state {
    IRQ_RUNNING, IRQ_READY, IRQ_IDLE
};

void softirq_init();
void softirq_register(void (*cb)(), int num);
void softirq_active(int num);
void softirq_run();

#endif
