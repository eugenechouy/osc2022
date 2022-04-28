#ifndef SIGNAL_H
#define SIGNAL_H

#include "list.h"

#define SIGKILL 9

struct signal_t {
    int num;
    void (*handler)();
    struct list_head list;
};

struct signal_context_t {
    void *trapframe;
    void *stk_addr;
};

struct signal_t *signal_create(int SIGNAL, void (*handler)());
void signal_back(void *trapframe);

void __signal(int SIGNAL, void (*handler)());
void __sigkill(int pid, int SIGNAL, void *trapframe);

#endif