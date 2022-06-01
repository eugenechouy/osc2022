#ifndef MMIO_H
#define MMIO_H

#include "mmu.h"

#ifdef BOOT_LOADER
#define MMIO_BASE 0x3F000000
#else
#define MMIO_BASE (KERNEL_VA_BASE | 0x3F000000) // -> 0x7E00,0000
#endif

#endif