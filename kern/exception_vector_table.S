.section ".text"

// save general registers to stack
.macro save_all
    sub sp, sp, 32 * 18
    stp x0, x1, [sp ,16 * 0]
    stp x2, x3, [sp ,16 * 1]
    stp x4, x5, [sp ,16 * 2]
    stp x6, x7, [sp ,16 * 3]
    stp x8, x9, [sp ,16 * 4]
    stp x10, x11, [sp ,16 * 5]
    stp x12, x13, [sp ,16 * 6]
    stp x14, x15, [sp ,16 * 7]
    stp x16, x17, [sp ,16 * 8]
    stp x18, x19, [sp ,16 * 9]
    stp x20, x21, [sp ,16 * 10]
    stp x22, x23, [sp ,16 * 11]
    stp x24, x25, [sp ,16 * 12]
    stp x26, x27, [sp ,16 * 13]
    stp x28, x29, [sp ,16 * 14]

    mrs x20, sp_el0
    stp x30, x20, [sp, 16 * 15]

    mrs x20, spsr_el1
    mrs x21, elr_el1
    stp x20, x21, [sp ,16 * 16]
.endm

// load general registers from stack
.macro load_all
    ldp x20, x21, [sp ,16 * 16]
    msr spsr_el1, x20
    msr elr_el1, x21

    ldp x30, x20, [sp, 16 * 15]
    msr sp_el0, x20

    ldp x0, x1, [sp ,16 * 0]
    ldp x2, x3, [sp ,16 * 1]
    ldp x4, x5, [sp ,16 * 2]
    ldp x6, x7, [sp ,16 * 3]
    ldp x8, x9, [sp ,16 * 4]
    ldp x10, x11, [sp ,16 * 5]
    ldp x12, x13, [sp ,16 * 6]
    ldp x14, x15, [sp ,16 * 7]
    ldp x16, x17, [sp ,16 * 8]
    ldp x18, x19, [sp ,16 * 9]
    ldp x20, x21, [sp ,16 * 10]
    ldp x22, x23, [sp ,16 * 11]
    ldp x24, x25, [sp ,16 * 12]
    ldp x26, x27, [sp ,16 * 13]
    ldp x28, x29, [sp ,16 * 14]
    add sp, sp, 32 * 18
.endm

.align 11 // vector table should be aligned to 0x800
.global exception_vector_table
exception_vector_table:

    // Exception from the current EL while using SP_EL0
    b exception_handler
    .align 7 // entry size is 0x80, .align will pad 0
    b exception_handler
    .align 7
    b exception_handler
    .align 7
    b exception_handler
    .align 7

    // Exception from the current EL while using SP_ELx
    b sync_handler
    .align 7
    b irq_el1_handler
    .align 7
    b exception_handler
    .align 7
    b exception_handler
    .align 7

    // Exception from a lower EL and at least one lower EL is AArch64
    b sync_handler
    .align 7
    b irq_el0_handler
    .align 7
    b exception_handler
    .align 7
    b exception_handler
    .align 7

    // Exception from a lower EL and all lower ELs are AArch32
    b exception_handler
    .align 7
    b exception_handler
    .align 7
    b exception_handler
    .align 7
    b exception_handler
    .align 7


exception_handler:
    b exception_handler

sync_handler:
    save_all
    mrs x0, spsr_el1
    mrs x1, elr_el1
    mrs x2, esr_el1
    mov x3, sp
    bl sync_main
    load_all
    eret    

irq_el1_handler:
    save_all
    bl irq_main
    bl irq_resched
    mov x0, sp
    bl signal_run
    load_all
    eret    

irq_el0_handler:
    save_all
    bl irq_main
    bl irq_resched
    mov x0, sp
    bl signal_run
    load_all
    eret    

.global return_from_fork
return_from_fork:
    load_all
    eret    
