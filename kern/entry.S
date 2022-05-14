#include "mmu.h"

.section ".text.entry"

.global _start
_start:
    
    bl      from_el2_to_el1

el1_start:
    // set_exception_vector_table
    adr x0, exception_vector_table
    msr vbar_el1, x0

    mov x0, 0b0         // enable interrupt in EL0
    msr spsr_el1, x0

    // Set up TCR_EL1
    ldr x0, =TCR_CONFIG_DEFAULT
    msr tcr_el1, x0 

    // Set up mair_el1
    ldr x0, =MAIR_CONFIG_DEFAULT
    msr mair_el1, x0

    bl pgtable_create

    mrs x2, sctlr_el1
    orr x2, x2, #1
    msr sctlr_el1, x2 // enable MMU, cache remains disabled

    // clear bss
    ldr     x0, =__bss_start
    ldr     x1, =__bss_size
loop:  
    cbz     x1, ready
    str     xzr, [x0], #8   // [x0] = 0, x0 = x0 + 8
    sub     x1, x1, #1      // x1 = x1 - 1
    cbnz    x1, loop

ready:
    // change sp to virtual address
    ldr x1, =KERNEL_VA_BASE
    add sp, sp, x1
    
    // indirect branch
    ldr x0, =kern_main
    br  x0

spin: 
    wfe
    b       spin


from_el2_to_el1:
    mov x0, (1 << 31)
    msr hcr_el2, x0
    mov x0, 0b1111000101
    msr spsr_el2, x0
    msr elr_el2, lr
    ldr x0, = __stack_kernel_top
    msr sp_el1, x0
    eret

.global from_el1_to_el0
from_el1_to_el0:
    msr elr_el1, x0
    mov x0, 0b0         // enable interrupt in EL0
    msr spsr_el1, x0   
    ldr x0, = __stack_user_top
    msr sp_el0, x0
    eret

.global run_el1_to_el0
run_el1_to_el0:
    msr elr_el1, x0
    mov x0, 0b0         // enable interrupt in EL0
    msr spsr_el1, x0   
    msr sp_el0, x1
    eret

pgtable_create:
    ldr x0, =__kernel_pgd // PGD
    lsl x0, x0, #16  // omit first 16 bit (using physical address)
    lsr x0, x0, #16
    add x1, x0, #4096 // PUD
    add x2, x1, #4096 // PMD
    add x3, x2, #4096 // PTE

    // Set up PGD
    ldr x4, =PGD0_ATTR
    orr x4, x1, x4
    str x4, [x0]

    // Set up PUD
    ldr x4, =PUD0_ATTR
    orr x4, x2, x4
    str x4, [x1]

    ldr x4, =PUD1_ATTR
    mov x5, 0x40000000
    orr x4, x5, x4
    str x4, [x1, #8]

    // Set up PMD
    mov x5, x3
    mov x6, xzr // i = 0
    mov x7, #512
pmd_loop:
    ldr x4, =PMD0_ATTR
    orr x4, x5, x4
    str x4, [x2, x6, lsl #3] // i * 8
    add x6, x6, #1 // i++
    add x5, x5, #4096
    cmp x6, x7
    b.ls pmd_loop

    // Set up normal PTE
    mov x4, xzr // physical address
    mov x5, xzr // i = 0
    mov x6, #258048
pte_nloop:
    ldr x7, =PTE_NORMAL_ATTR
    orr x7, x4, x7
    str x7, [x3, x5, lsl #3] // (i * 8)
    add x5, x5, #1
    add x4, x4, #4096
    cmp x5, x6
    b.ls pte_nloop

    // Set up device PTE
    add x6, x6, #4096 // 262144
pte_dloop:
    ldr x7, =PTE_DEVICE_ATTR
    orr x7, x4, x7
    str x7, [x3, x5, lsl #3] // (i * 8)
    add x5, x5, #1
    add x4, x4, #4096
    cmp x5, x6
    b.ls pte_dloop

    msr ttbr0_el1, x0
    msr ttbr1_el1, x0

    ret