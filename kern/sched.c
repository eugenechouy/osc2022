#include "kern/sched.h"
#include "kern/irq.h"

struct task_struct task_pool[MAX_TASK_NUM];

inline int get_tid() {
    for(int i=0 ; i<MAX_TASK_NUM ; i++) {
        if (!task_pool[i].used)
            return i;
    }
    return -1;
}

void task_init() {
    for(int i=0 ; i<MAX_TASK_NUM ; i++) {
        task_pool[i].tid    = i;
        task_pool[i].used   = 0;
        task_pool[i].state  = DEAD;
    }
    // idle task
    task_pool[0].used  = 1;
    task_pool[0].prio  = 127;
    task_pool[0].state = RUNNING;
    update_current(&task_pool[0]);
}


struct prio_array {
    DECLARE_BITMAP(bitmap, MAX_PRIO);
    struct list_head queue[MAX_PRIO];
};

struct runqueue {
    unsigned long nr_running;
    struct prio_array array;
};

struct runqueue runqueue;
char kernel_stack[MAX_TASK_NUM][4096];

void runqueue_init() {
    int i;
    struct prio_array *array = &runqueue.array;
    runqueue.nr_running = 0;
    for(i=0 ; i<MAX_PRIO ; i++) {
        INIT_LIST_HEAD(&(array->queue[i]));
    }
    bitmap_zero(array->bitmap, MAX_PRIO);
}

void runqueue_push(struct task_struct *new_task) {
    struct prio_array *array = &runqueue.array;
    
    runqueue.nr_running += 1;
    __set_bit(new_task->prio, array->bitmap);
    list_add_tail(&new_task->list, &array->queue[new_task->prio]);
}

struct task_struct * runqueue_pop() {
    int highest_prio;
    struct task_struct *next_task;
    struct prio_array *array = &runqueue.array;

    runqueue.nr_running -= 1;
    highest_prio = sched_find_first_bit(array->bitmap);
    // no task in queue
    if (highest_prio == MAX_PRIO) 
        return 0;
    next_task = list_entry(array->queue[highest_prio].next, struct task_struct, list);
    list_del(&next_task->list);
    if (list_empty(&array->queue[highest_prio])) 
        __clear_bit(highest_prio, array->bitmap);
    return next_task;
}


int privilege_task_create(void (*func)(), int prio) {
    struct task_struct *new_task;
    unsigned long stk_addr;
    int tid = get_tid();
    
    if (tid == -1)
        goto out;

    new_task = &task_pool[tid];
    new_task->used      = 1;
    new_task->prio      = prio;
    new_task->state     = RUNNING;
    new_task->ctime     = 1;
    new_task->resched   = 0;

    stk_addr = (unsigned long)&kernel_stack[tid][4095];
    stk_addr = (stk_addr - 1) & -16; // should be round to 16
    new_task->task_context.lr = (unsigned long)func;
    new_task->task_context.fp = stk_addr;
    new_task->task_context.sp = stk_addr;

    runqueue_push(new_task);
out:
    return tid;
}

void context_switch(struct task_struct *next) {
    struct task_struct *prev = get_current();
    if (prev->state == RUNNING) {
        runqueue_push(prev);
    }
    update_current(next);
    switch_to(&prev->task_context, &next->task_context);
}

void schedule() {
    struct task_struct *next = runqueue_pop();
    if (next)
        context_switch(next);
}