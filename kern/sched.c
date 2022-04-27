#include "kern/sched.h"
#include "kern/irq.h"
#include "kern/slab.h"
#include "kern/kio.h"
#include "string.h"

// ############## runqueue ##################

struct prio_array {
    DECLARE_BITMAP(bitmap, MAX_PRIO);
    struct list_head queue[MAX_PRIO];
};

struct runqueue {
    unsigned long nr_running;
    struct prio_array array;
};

#define STACKSIZE 4096

struct runqueue runqueue;
struct list_head zombie_queue;
char kernel_stack[MAX_PRIV_TASK_NUM][STACKSIZE];
char user_stack[MAX_PRIV_TASK_NUM][STACKSIZE];

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

struct task_struct *privilege_task_create(void (*func)(), int prio) {
    struct task_struct *new_task;
    unsigned long stk_addr;
    int tid = -1;

    if (prio > 20 || prio < 1)
        return 0;
    
    tid = get_priv_tid();
    if (tid == -1)
        return 0;
    

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
    stk_addr = (unsigned long)&user_stack[tid][4095];
    stk_addr = stk_addr & -16; // should be round to 16
    new_task->ustk_addr       = (void*)stk_addr;
    
    // int_disable();
    runqueue_push(new_task);
    // int_enable();
    return new_task;
}

// ############## normal task ##################

struct task_struct *task_create(void (*func)(), int prio) {
    unsigned long stk_addr;
    struct task_struct *new_task;

    if (prio <= 20)
        return 0;

    new_task = kmalloc(STACKSIZE);
    if (!new_task)
        return 0;

    new_task->tid       = pid++;
    new_task->prio      = prio;
    new_task->state     = RUNNING;
    new_task->ctime     = TASK_CTIME;
    new_task->resched   = 0;

    stk_addr = (unsigned long)kmalloc(STACKSIZE);
    stk_addr = stk_addr & -16; // should be round to 16
    new_task->task_context.lr = (unsigned long)func;
    new_task->task_context.fp = stk_addr;
    new_task->task_context.sp = stk_addr;
    new_task->stk_addr        = (void*)stk_addr;
    stk_addr = (unsigned long)kmalloc(STACKSIZE);
    stk_addr = stk_addr & -16; // should be round to 16
    new_task->ustk_addr       = (void*)stk_addr;

    runqueue_push(new_task);
    return new_task;
}

int task_fork(void (*func)(), struct task_struct *parent, void *trapframe) {
    int i;
    unsigned long offset;
    char *pptr;
    char *cptr;
    struct task_struct *child = privilege_task_create(func, parent->prio);

    if (!child)
        return -1;

    // setup kernel stack
    cptr = (char *)child->stk_addr;
    pptr = (char *)parent->stk_addr;
    offset = pptr - (char*)trapframe;
    for (i=0 ; i<offset ; i++) 
        *(cptr - i) = *(pptr - i); 
    child->task_context.sp = (unsigned long)child->stk_addr - offset;

    // setup user stack
    cptr = (char *)child->ustk_addr;
    pptr = (char *)parent->ustk_addr;
    struct trapframe* child_trapframe = (struct trapframe *)child->task_context.sp;
    struct trapframe* parent_trapframe = (struct trapframe *)trapframe;
    offset = pptr - (char *)parent_trapframe->sp_el0;
    for (i=0 ; i<offset ; i++) 
        *(cptr - i) = *(pptr - i); 
    child_trapframe->sp_el0 = (unsigned long)child->ustk_addr - offset;
    child_trapframe->x[0] = 0;
    
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
            kfree(to_release->ustk_addr);
            kfree(to_release);
            list_del(itr);
        }
    }
}

extern void run_el1_to_el0(void *, void *);

void do_exec(void (*func)()) {
    struct task_struct *current = get_current();
    // should use user stack
    run_el1_to_el0(func, current->ustk_addr);
}

// ############## sys call ##################

int __getpid() {
    struct task_struct *current = get_current();
    return current->tid;
}

void __exec(const char *user_code, char *const argv[]) {
    struct task_struct *current = get_current();
    char *user_prog1 = kmalloc(250000);
    memcpy(user_prog1, user_code, 250000);
    run_el1_to_el0(user_prog1, current->ustk_addr);
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