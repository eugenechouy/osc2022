.section ".text.boot"

.global _start
_start:
    // set stack pointer
    ldr     x1, =_start
    mov     sp, x1

    // set bss
    ldr     x1, =__bss_start
    ldr     x2, =__bss_size
loop:    
    cbz     x2, finish
    str     xzr, [x1], #8
    sub     x2, x2, #1
    cbnz    x2, loop
finish:
    bl      main

spin:
    wfe
    b       spin