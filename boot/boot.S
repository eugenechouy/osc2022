.section ".text.boot"

.global _start
_start:
    bl      main
1:  wfe
    b       1b