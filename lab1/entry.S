.section ".text.entry"

.global _start
_start:
2:
    ldr     x0, = __stack_top
    mov     sp, x0

    // clear bss
    ldr     x0, =__bss_start
    ldr     x1, =__bss_size
3:  cbz     x1, 4f
    // [x0] = 0, x0 = x0 + 8
    str     xzr, [x0], #8
    // x1 = x1 - 1
    sub     x1, x1, #1
    cbnz    x1, 3b
4:  bl      main
1:  wfe
    b       1b