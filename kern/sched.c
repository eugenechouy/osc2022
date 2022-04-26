#include "kern/sched.h"
#include "kern/irq.h"
#include "kern/slab.h"
#include "kern/kio.h"


// ############## runqueue ##################

struct prio_array {
    DECLARE_BITMAP(bitmap, MAX_PRIO);
    struct list_head queue[MAX_PRIO];
};

struct runqueue {
    unsigned long nr_running;
    struct prio_array array;
};

struct runqueue runqueue;
struct list_head zombie_queue;
char kernel_stack[MAX_PRIV_TASK_NUM][4096];

void runqueue_init() {
    int i;
    struct prio_array *array = &runqueue.array;
    runqueue.nr_running = 0;
    for(i=0 ; i<MAX_PRIO ; i++) {
        INIT_LIST_HEAD(&array->queue[i]);
    }
    bitmap_zero(array->bitmap, MAX_PRIO);
    INIT_LIST_HEAD(&zombie_queue);
}

void runqueue_push(struct task_struct *new_task) {
    struct prio_array *array = &runqueue.array;
    
    runqueue.nr_running += 1;
    __set_bit(new_task->prio, array->bitmap);
    list_add_tail(&new_task->list, &array->queue[new_task->prio]);
}

struct task_struct* runqueue_pop() {
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

// ############## priv task ##################

struct task_struct task_pool[MAX_PRIV_TASK_NUM];
int pid; // start from 1000

inline int get_priv_tid() {
    int i;
    for(i=0 ; i<MAX_PRIV_TASK_NUM ; i++) {
        if (!task_pool[i].used)
            return i;
    }
    return -1;
}

void task_init() {
    int i;
    for(i=0 ; i<MAX_PRIV_TASK_NUM ; i++) {
        task_pool[i].tid    = i;
        task_pool[i].used   = 0;
        task_pool[i].state  = DEAD;
    }
    // idle task
    task_pool[0].used  = 1;
    task_pool[0].prio  = 127;
    task_pool[0].state = RUNNING;
    update_current(&task_pool[0]);
    pid = 1000;
}

int privilege_task_create(void (*func)(), int prio) {
    struct task_struct *new_task;
    unsigned long stk_addr;
    int tid = -1;

    if (prio > 20 || prio < 1)
        goto out;
    
    tid = get_priv_tid();
    if (tid == -1)
        goto out;
    

    new_task = &task_pool[tid];
    new_task->used      = 1;
    new_task->prio      = prio;
    new_task->state     = RUNNING;
    new_task->ctime     = TASK_CTIME;
    new_task->resched   = 0;

    stk_addr = (unsigned long)&kernel_stack[tid][4095];
    stk_addr = stk_addr & -16; // should be round to 16
    new_task->task_context.lr = (unsigned long)func;
    new_task->task_context.fp = stk_addr;
    new_task->task_context.sp = stk_addr;
    new_task->stk_addr        = (void*)stk_addr;

    runqueue_push(new_task);
out:
    return tid;
}

// ############## normal task ##################

int task_create(void (*func)(), int prio) {
    unsigned long stk_addr;
    struct task_struct *new_task;

    if (prio <= 20)
        return -1;

    new_task = kmalloc(sizeof(struct task_struct));
    if (!new_task)
        return -1;

    new_task->tid       = pid++;
    new_task->prio      = prio;
    new_task->state     = RUNNING;
    new_task->ctime     = TASK_CTIME;
    new_task->resched   = 0;

    stk_addr = (unsigned long)kmalloc(4096);
    stk_addr = stk_addr & -16; // should be round to 16
    new_task->task_context.lr = (unsigned long)func;
    new_task->task_context.fp = stk_addr;
    new_task->task_context.sp = stk_addr;
    new_task->stk_addr        = (void*)stk_addr;

    runqueue_push(new_task);
    return new_task->tid;
}

int task_fork(void (*func)(), struct task_struct *parent, void *trapframe) {
    int i;
    unsigned long stk_addr;
    unsigned long offset;
    char *pptr;
    char *cptr;
    struct task_struct *child;

    child = kmalloc(sizeof(struct task_struct));
    if (!child)
        return -1;
    
    child->tid         = pid++;
    child->prio        = parent->prio;
    child->state       = RUNNING;
    child->ctime       = TASK_CTIME;
    child->resched     = 0;

    stk_addr = (unsigned long)kmalloc(4096);
    stk_addr = stk_addr & -16; // should be round to 16
    child->task_context.lr = (unsigned long)func;
    child->task_context.fp = stk_addr;
    
    cptr = (char *)stk_addr;
    pptr = (char *)parent->stk_addr;
    for (i=0 ; i<4096 ; i++) 
        *(cptr - i) = *(pptr - i); 

    offset = pptr - (char*)trapframe;
    child->task_context.sp = stk_addr - offset;
    child->stk_addr        = (void*)stk_addr;

    struct trapframe* child_trapframe = (struct trapframe *)child->task_context.sp;
    struct trapframe* parent_trapframe = (struct trapframe *)trapframe;
    offset = pptr - parent_trapframe->sp_el0;
    child_trapframe->sp_el0 = stk_addr - offset;
    child_trapframe->x[0] = 0;

    runqueue_push(child);
    return child->tid;
}

// #############################################

void context_switch(struct task_struct *next) {
    struct task_struct *prev = get_current();
    if (prev->state == RUNNING) {
        runqueue_push(prev);
    } else if (prev->state == DEAD) {
        if (prev->tid >= 1000)
            list_add_tail(&prev->list, &zombie_queue);
        else
            prev->used = 0;
    }
    update_current(next);
    switch_to(&prev->task_context, &next->task_context);
}

void schedule() {
    struct task_struct *next = runqueue_pop();
    if (next != 0) {
        context_switch(next);
    }
}

void kill_zombies() {
    struct task_struct *to_release;
    struct list_head *itr;
    struct list_head *tmp;

    while(1) {
        list_for_each_safe(itr, tmp, &zombie_queue) {
            to_release = list_entry(itr, struct task_struct, list);
            kprintf("Kill zombie %d\n", to_release->tid);
            kfree(to_release->stk_addr);
            kfree(to_release);
            list_del(itr);
        }
    }
}

// ############## sys call ##################

extern void run_el1_to_el0(void *, void *);

int __getpid() {
    struct task_struct *current = get_current();
    return current->tid;
}

void __exec(void (*func)()) {
    struct task_struct *current = get_current();
    run_el1_to_el0(func, current->stk_addr);
}

extern void return_from_fork();

int __fork(void *trapframe) {
    struct task_struct *parent = get_current();
    return task_fork(return_from_fork, parent, trapframe);
}

void __exit() {
    struct task_struct *current = get_current();
    current->state = DEAD;
    schedule();
}