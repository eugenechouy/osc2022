#include "kern/timer.h"
#include "kern/kio.h"
#include "kern/sched.h"
#include "kern/irq.h"
#include "reset.h"
#include "syscall.h"
#include "peripheral/mailbox.h"


inline void sys_getpid(struct trapframe *trapframe) {
    trapframe->x[0] = __getpid();
}

inline void sys_uart_read(struct trapframe *trapframe) {
    int i;
    int size = trapframe->x[1];
    char *buf = (char *)trapframe->x[0];
    for(i=0 ; i<size ; i++)
        buf[i] = uart_async_read();
    trapframe->x[0] = i;
}

inline void sys_uart_write(struct trapframe *trapframe) {
    int i;
    char *buf = (char *)trapframe->x[0];
    int size = trapframe->x[1];
    for(i=0 ; i<size ; i++) {
        if (buf[i] == '\n')
            uart_async_write('\r');
        uart_async_write(buf[i]);
    }
    trapframe->x[0] = i;
}

inline void sys_exec(struct trapframe *trapframe) {
    trapframe->x[0] = 0;   
}

inline void sys_fork(struct trapframe *trapframe) {
    trapframe->x[0] = __fork(trapframe);
}

inline void sys_exit(struct trapframe *trapframe) {
    __exit();
}

inline void sys_mbox_call(struct trapframe *trapframe) {
    unsigned char ch = trapframe->x[0];
    unsigned int *mailbox = (unsigned int *)trapframe->x[1];
    trapframe->x[0] = mailbox_call(ch, mailbox);
}

inline void sys_kill(struct trapframe *trapframe) {

}

void syscall_main(struct trapframe *trapframe) {
    int_enable();
    long syscall_num = trapframe->x[8];
    switch(syscall_num) {
        case SYS_GET_PID:
            sys_getpid(trapframe);
            break;
        case SYS_UART_READ:
            sys_uart_read(trapframe);
            break;
        case SYS_UART_WRITE:
            sys_uart_write(trapframe);
            break;
        case SYS_EXEC:
            sys_exec(trapframe);
            break;
        case SYS_FORK:
            sys_fork(trapframe);
            break;
        case SYS_EXIT:
            sys_exit(trapframe);
            break;
        case SYS_MBOX_CALL:
            sys_mbox_call(trapframe);
            break;
        case SYS_KILL:
            sys_kill(trapframe);
            break;
        default:
            uart_sync_puts("Undefined syscall number, about to reboot...\n");
            reset(1000);
            while(1);
    }
    int_disable();
}

void sync_main(unsigned long spsr, unsigned long elr, unsigned long esr, struct trapframe *trapframe) {
    unsigned int svc_num; 

    svc_num = esr & 0xFFFFFF;
    switch(svc_num) {
    case 0:
        syscall_main(trapframe);
        break;
    case 1:
        kputs("svc 1\n");
        core_timer_enable();
        break;
    case 2:
        /*
        bits [31:26] 0b010101 SVC instruction execution in AArch64 state.
        */
        kprintf("\nspsr_el1: \t%x\n", spsr);
        kprintf("elr_el1: \t%x\n", elr);
        kprintf("esr_el1: \t%x\n", esr);
        break;
    default:
        uart_sync_puts("Undefined svc number, about to reboot...\n");
        reset(1000);
        while(1);
    }
}