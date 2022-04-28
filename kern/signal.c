#include "kern/signal.h"
#include "kern/sched.h"
#include "kern/slab.h"
#include "kern/kio.h"
#include "kern/irq.h"
#include "string.h"

struct signal_t *signal_create(int SIGNAL, void (*handler)()) {
    struct signal_t *new_signal = kmalloc(sizeof(struct signal_t));
    new_signal->num = SIGNAL;
    new_signal->handler = handler;
    INIT_LIST_HEAD(&new_signal->list);
    return new_signal;
}

void signal_default(int pid, int SIGNAL) {
    switch(SIGNAL) {
        case SIGKILL:
            __kill(pid);
            break;
        default:
            kprintf("Undefined signal number...\n");
    }
}

void signal_return() {
    asm volatile(
        "mov x8, #10 \n\t"
        "svc #0 \n\t"
    );
}

void signal_jump(void *trapframe, void (*handler)()) {
    struct trapframe *tf = trapframe;
    struct task_struct *current = get_current();
    struct signal_context_t *signal_context = kmalloc(sizeof(struct signal_context_t));
    signal_context->stk_addr = kmalloc(4096);
    signal_context->trapframe = kmalloc(sizeof(struct trapframe));
    // kprintf("Ready to jump %x %x %x\n", signal_context, signal_context->stk_addr, signal_context->trapframe);
    memcpy(signal_context->trapframe, trapframe, sizeof(struct trapframe));

    current->signal_context = signal_context;

    tf->x[30]   = signal_return;
    tf->elr_el1 = handler;
    tf->sp_el0  = signal_context->stk_addr;
}

void signal_back(void *trapframe) {
    struct task_struct *current = get_current();
    // kprintf("Back %x %x\n", current->signal_context, current->signal_context->trapframe);
    memcpy(trapframe, current->signal_context->trapframe, sizeof(struct trapframe));
    kfree(current->signal_context->trapframe);
    kfree(current->signal_context->stk_addr);
    kfree(current->signal_context);
}


void __signal(int SIGNAL, void (*handler)()) {
    struct task_struct *current = get_current();
    struct signal_t *signal = signal_create(SIGNAL, handler);
    list_add_tail(&signal->list, &current->signal_list);
}

void __sigkill(int pid, int SIGNAL, void *trapframe) {
    struct list_head *ptr;
    struct signal_t *signal;
    struct task_struct *current = get_current();
    if (list_empty(&current->signal_list)) 
        goto out;
    list_for_each(ptr, &current->signal_list) {
        signal = list_entry(ptr, struct signal_t, list);
        if (signal->num == SIGNAL) {
            signal_jump(trapframe, signal->handler);
            return;
        }
    }
out:
    signal_default(pid, SIGNAL);
}