
.global getpid
getpid:
    mov x8, #1
    svc #80
    ret

.global uart_read
uart_read:
    mov x8, #2
    svc #80
    ret

.global uart_write
uart_write:
    mov x8, #3
    svc #80
    ret

.global exec
exec:
    mov x8, #4
    svc #80
    ret

.global fork
fork:
    mov x8, #5
    svc #80
    ret

.global exit
exit:
    mov x8, #6
    svc #80
    ret