.section ".text"

#include "peripheral/arm.h"

.global core_timer_enable
core_timer_enable:
    mov x0, 1
    msr cntp_ctl_el0, x0 // enable
    mrs x0, cntfrq_el0
    msr cntp_tval_el0, x0 // set expired time
    mov x0, 2
    ldr x1, =CORE0_TIMER_IRQ_CTRL
    str w0, [x1] // unmask timer interrupt
    ret

.global core_timer_disable
core_timer_disable:
    mov x0, 0
    msr cntp_ctl_el0, x0 // disable
    mov x0, 0
    ldr x1, =CORE0_TIMER_IRQ_CTRL
    str x0, [x1] // disable timer interrupt
    ret

.global timer_enable_int
timer_enable_int:
    mov x0, 2
    ldr x1, =CORE0_TIMER_IRQ_CTRL
    str w0, [x1] // unmask timer interrupt
    ret

.global timer_disable_int
timer_disable_int:
    mov x0, 0
    ldr x1, =CORE0_TIMER_IRQ_CTRL
    str x0, [x1] // disable timer interrupt
    ret

.global timer_sched_latency
timer_sched_latency: 
    mrs x0, cntfrq_el0
    asr x0, x0, 7 // 1/2 secs
    msr cntp_tval_el0, x0
    ret