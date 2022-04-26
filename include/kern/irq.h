#ifndef IRQ_H
#define IRQ_H

struct trapframe {
    long x[31];
    long sp_el0;
    long spsr_el1;
    long elr_el1;
};

void int_enable();
void int_disable();

void int_init();

#endif