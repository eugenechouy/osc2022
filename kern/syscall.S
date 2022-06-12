
.global getpid
getpid:
    mov x8, #0
    svc #0
    ret

.global uart_read
uart_read:
    mov x8, #1
    svc #0
    ret

.global uart_write
uart_write:
    mov x8, #2
    svc #0
    ret

.global exec
exec:
    mov x8, #3
    svc #0
    ret

.global fork
fork:
    mov x8, #4
    svc #0
    ret

.global exit
exit:
    mov x8, #5
    svc #0

.global mbox_call
mbox_call:
    mov x8, #6
    svc #0
    ret

.global kill
kill:
    mov x8, #7
    svc #0
    ret

.global signal
signal:
    mov x8, #8
    svc #0
    ret

.global sigkill
sigkill:
    mov x8, #9
    svc #0
    ret

.global open
open:
    mov x8, #11
    svc #0
    ret

.global close
close:
    mov x8, #12
    svc #0
    ret

.global write
write:
    mov x8, #13
    svc #0
    ret

.global read
read:
    mov x8, #14
    svc #0
    ret

.global mkdir
mkdir:
    mov x8, #15
    svc #0
    ret

.global mount
mount:
    mov x8, #16
    svc #0
    ret

.global chdir
chdir:
    mov x8, #17
    svc #0
    ret

.global lseek64
lseek64:
    mov x8, #18
    svc #0
    ret

.global sync
sync:
    mov x8, #20
    svc #0
    ret