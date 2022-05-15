#ifndef ARM_H
#define ARM_H

#include "mmu.h"

#define ARM_PERI_BASE        (KERNEL_VA_BASE | 0x40000000)

#define CORE0_TIMER_IRQ_CTRL (ARM_PERI_BASE + 0x40)

// QA7_rev3.4 p.7
#define CORE0_IRQ_SRC        ((volatile unsigned int*)(ARM_PERI_BASE + 0x60))

#endif