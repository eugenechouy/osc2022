#include "kern/sched.h"
#include "kern/irq.h"

struct task_struct *current;
struct task_struct *task_queue_head;

struct task_struct task_pool[MAX_TASK_NUM];

// #####################################################################

inline int get_tid() {
    for(int i=0 ; i<MAX_TASK_NUM ; i++) {
        if (!task_pool[i].used)
            return i;
    }
    return -1;
}

void task_queue_init() {
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

// #####################################################################

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
    struct prio_array *array;
};

struct runqueue runqueue;
char kstack_pool[MAX_TASK_NUM][4096];

void runqueue_init() {
    int i;
    struct prio_array *array = runqueue.array;
    
    runqueue.nr_running = 0;
    for(i=0 ; i<MAX_PRIO ; i++) {
        INIT_LIST_HEAD(&array->queue[i]);
    }
    bitmap_zero(array->bitmap, MAX_PRIO);
}

void runqueue_push(struct task_struct *new_task) {
    struct prio_array *array = runqueue.array;
    
    runqueue.nr_running += 1;
    __set_bit(new_task->prio, array->bitmap);
    list_add_tail(&new_task->list, &array->queue[new_task->prio]);
}

struct task_struct * runqueue_pop() {
    int highest_prio;
    struct task_struct *next_task;
    struct prio_array *array = runqueue.array;

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
    int tid = get_tid();
    
    if (tid == -1)
        goto out;

    new_task = &task_pool[tid];
    new_task->used      = 1;
    new_task->prio      = prio;
    new_task->state     = RUNNING;
    new_task->ctime     = 1;
    new_task->resched   = 0;

    new_task->task_context.lr = (unsigned long)func;
    new_task->task_context.fp = (unsigned long)(&kstack_pool[tid][4080]);
    new_task->task_context.sp = (unsigned long)(&kstack_pool[tid][4080]);

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