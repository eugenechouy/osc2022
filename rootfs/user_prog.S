.section ".text"

.global _start
_start:
    mov x0, 0
    svc 1
1:
    add x0, x0, 1
    svc 0
    cmp x0, 5
    blt 1b
    svc 4
1:
    b 1b