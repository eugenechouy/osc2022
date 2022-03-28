#ifndef IRQ_H
#define IRQ_H

#include "mmio.h"

// spec p.112
// start at 0x7E00,B000
#define INT_BASE       (MMIO_BASE + 0xB000)

#define IRQ_PENDING_1        ((volatile unsigned int*)(INT_BASE + 0x204)) 
#define IRQ_PENDING_2        ((volatile unsigned int*)(INT_BASE + 0x208)) 
#define ENABLE_IRQS_1        ((volatile unsigned int*)(INT_BASE + 0x210)) 
#define ENABLE_IRQS_2        ((volatile unsigned int*)(INT_BASE + 0x214)) 
#define ENABLE_BASIC_IRQS    ((volatile unsigned int*)(INT_BASE + 0x218)) 
#define DISABLE_IRQS_1       ((volatile unsigned int*)(INT_BASE + 0x21C)) 
#define DISABLE_IRQS_2       ((volatile unsigned int*)(INT_BASE + 0x220)) 
#define DISABLE_BASIC_IRQS   ((volatile unsigned int*)(INT_BASE + 0x224)) 

#endif