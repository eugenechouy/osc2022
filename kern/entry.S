.section ".text.entry"

.global _start
_start:
    bl      from_el2_to_el1

el1_start:
    // set_exception_vector_table
    adr x0, exception_vector_table
    msr vbar_el1, x0

    // clear bss
    ldr     x0, =__bss_start
    ldr     x1, =__bss_size
loop:  
    cbz     x1, ready
    str     xzr, [x0], #8   // [x0] = 0, x0 = x0 + 8
    sub     x1, x1, #1      // x1 = x1 - 1
    cbnz    x1, loop

ready:
    bl      main
spin: 
    wfe
    b       spin

from_el2_to_el1:
    mov x0, (1 << 31)
    msr hcr_el2, x0
    mov x0, 0b1111000101
    msr spsr_el2, x0
    msr elr_el2, lr
    ldr x0, = __stack_top
    msr sp_el1, x0
    eret

.global from_el1_to_el0
from_el1_to_el0:
    msr elr_el1, x0
    mov x0, 0b1111000000
    msr spsr_el1, x0
    mov x0, 0x10000000
    msr sp_el0, x0
    eret