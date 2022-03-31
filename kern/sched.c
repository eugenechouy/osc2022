#include "kern/sched.h"
#include "kern/irq.h"

extern void switch_to(struct task_struct *prev, struct task_struct *next);

void context_switch(struct task_struct *next) {
    struct task_struct *prev = current;
    switch_to(prev, next);
}

void schedule() {

}


inline int get_tid() {
    for(int i=0 ; i<MAX_TASK_NUM ; i++) {
        if (!task_pool[i].used)
            return i;
    }
    return -1;
}

void init_task_queue() {
    task_queue_head = 0;
    current = 0;
    for(int i=0 ; i<MAX_TASK_NUM ; i++) {
        task_pool[i].tid    = i;
        task_pool[i].used   = 0;
        task_pool[i].state  = DEAD;
    }
}

void task_run() {
    struct task_struct *itr;

    if (!task_queue_head)
        return;
    itr = task_queue_head;
    while(itr) {
        // there is a higher priority task been interrupted, return
        if (itr->state == INT)
            break;
        if (itr->state != READY)
            break;
        // change task state
        int_disable();
        current = itr;
        itr->state = RUNNING;
        int_enable();

        itr->cb(itr->cb_args);

        int_disable();
        itr->used  = 0;
        itr->state = DEAD;
        itr = itr->next;
        task_queue_head = itr;
        current = 0;
        int_enable();
    }
}

void task_state_update() {
    if (current != 0)
        current->state = INT;
}

void task_create(void (*cb)(void*), void *args, int prio) {
    struct task_struct *new_task;
    struct task_struct *itr;
    struct task_struct *prev;
    int tid = get_tid();
    
    if (tid == -1)
        return;

    new_task = &task_pool[tid];
    new_task->used      = 1;
    new_task->prio      = prio;
    new_task->cb        = cb;
    new_task->cb_args   = args;
    new_task->next      = 0;
    new_task->state     = READY;

    if (!task_queue_head) {
        task_queue_head = new_task;
    } else {
        prev = 0;
        itr = task_queue_head;
        while(itr) {
            if (itr->prio > prio)
                break;
            prev = itr;
            itr = itr->next;
        }
        if (!prev) {
            new_task->next = task_queue_head;
            task_queue_head = new_task;
        } else {
            prev->next = new_task;
            new_task->next = itr;
        }
    }
}   