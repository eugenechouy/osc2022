#include "kern/sched.h"
#include "kern/irq.h"
#include "kern/slab.h"
#include "kern/kio.h"
#include "string.h"
#include "fs/vfs.h"

#define ROUND_UP(n,d)   (((n) + (d-1)) & (-d))
#define ROUND_DOWN(n,d) ((n) & (-d))

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
struct task_struct *utask[1000];
int pid; // start from 1000

inline int get_priv_tid() {
    int i;
    for(i=0 ; i<MAX_PRIV_TASK_NUM ; i++) {
        if (!task_pool[i].used)
            return i;
    }
    return -1;
}

extern unsigned int __kernel_pgd;

void task_init() {
    int i;
    memset(task_pool, 0, sizeof(struct task_struct) * MAX_PRIV_TASK_NUM);
    memset(utask, 0, sizeof(struct task_struct) * 1000);
    for(i=0 ; i<MAX_PRIV_TASK_NUM ; i++) {
        task_pool[i].tid    = i;
        task_pool[i].used   = 0;
        task_pool[i].state  = DEAD;
    }
    // idle task
    task_pool[0].used  = 1;
    task_pool[0].prio  = 127;
    task_pool[0].state = RUNNING;
    task_pool[0].cwd   = rootfs->root;
    task_pool[0].croot = rootfs->root;
    fd_init(&task_pool[0].files);
    task_pool[0].mm.pgd = (unsigned long *)&__kernel_pgd;
    INIT_LIST_HEAD(&task_pool[0].signal_pend_list);
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
    new_task->cwd       = rootfs->root;
    new_task->croot     = rootfs->root;
    fd_init(&new_task->files);
    create_pgd(&new_task->mm);
    INIT_LIST_HEAD(&new_task->signal_list);
    INIT_LIST_HEAD(&new_task->signal_pend_list);

    // kernel stack
    stk_addr = (unsigned long)&kernel_stack[tid][4095];
    stk_addr = ROUND_DOWN(stk_addr, 16);
    new_task->task_context.lr = (unsigned long)func;
    new_task->task_context.fp = stk_addr;
    new_task->task_context.sp = stk_addr;
    new_task->stk_addr        = (void*)stk_addr;
    
    runqueue_push(new_task);
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

    int_disable();

    utask[pid-1000]     = new_task;
    new_task->tid       = pid++;
    new_task->prio      = prio;
    new_task->state     = RUNNING;
    new_task->ctime     = TASK_CTIME;
    new_task->resched   = 0;
    new_task->cwd       = rootfs->root;
    new_task->croot     = rootfs->root;
    fd_init(&new_task->files);
    create_pgd(&new_task->mm);
    INIT_LIST_HEAD(&new_task->signal_list);
    INIT_LIST_HEAD(&new_task->signal_pend_list);

    // kernel stack
    stk_addr = (unsigned long)kmalloc(STACKSIZE);
    stk_addr = stk_addr + STACKSIZE;
    stk_addr = ROUND_DOWN(stk_addr, 16);
    new_task->task_context.lr = (unsigned long)func;
    new_task->task_context.fp = stk_addr;
    new_task->task_context.sp = stk_addr;
    new_task->stk_addr        = (void*)stk_addr;

    runqueue_push(new_task);

    int_enable();

    return new_task;
}

int task_fork(void (*func)(), struct task_struct *parent, void *trapframe) {
    int i;
    unsigned long offset;
    char *pptr;
    char *cptr;
    struct trapframe* child_trapframe;
    struct trapframe* parent_trapframe;
    struct task_struct *child = task_create(func, parent->prio);

    if (!child)
        return -1;

    int_disable();

    // setup kernel stack
    cptr = (char *)child->stk_addr;
    pptr = (char *)parent->stk_addr;
    offset = pptr - (char*)trapframe;
    for (i=0 ; i<offset ; i++) 
        *(cptr - i) = *(pptr - i); 
    child->task_context.sp = (unsigned long)child->stk_addr - offset;

    // setup user stack
    fork_pgd(&parent->mm, &child->mm);
    mappages(&child->mm, 0x3C000000, 0x3000000, 0x3C000000);
    child_trapframe = (struct trapframe *)child->task_context.sp;
    parent_trapframe = (struct trapframe *)trapframe;
    child_trapframe->sp_el0 = parent_trapframe->sp_el0;
    child_trapframe->x[0] = 0;
    
    int_enable();
    
    if (list_empty(&parent->signal_list)) 
        goto out;
    // signal copy
    struct list_head *ptr;
    struct signal_t *signal;
    struct signal_t *new_signal;
    list_for_each(ptr, &parent->signal_list) {
        signal = list_entry(ptr, struct signal_t, list);
        new_signal = signal_create(signal->num, signal->handler);
        list_add_tail(&new_signal->list, &child->signal_list);
    }
out:
    return child->tid;
}

// #############################################

void context_switch(struct task_struct *next) {
    struct task_struct *prev = get_current();
    if (prev->state == RUNNING) {
        runqueue_push(prev);
    } else if (prev->state == DEAD) {
        kprintf("Process %d dead...\n", prev->tid);
        if (prev->tid >= 1000)
            list_add_tail(&prev->list, &zombie_queue);
        else
            prev->used = 0;
    }
    // kprintf("context switch %d ~ %d\n", prev->tid, next->tid);
    update_pgd(VIRT_2_PHY(next->mm.pgd));
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
            free_pgd(&to_release->mm);
            kfree(to_release);
            list_del(itr);
        }
    }
}

struct task_struct* get_task_struct(int pid) {
    if (pid < 1000)
        return &task_pool[pid];
    else {
        return utask[pid-1000];
    }
}

// ############## sys call ##################

int __getpid() {
    struct task_struct *current = get_current();
    return current->tid;
}

void __exec(const char *name, char *const argv[]) {
    struct task_struct *current = get_current();
    struct file *file_node;
    long filesize; 
    char *user_prog;
    
    if (vfs_open(name, 0, &file_node) < 0) {
        kprintf("Failed to exec %s", name);
        return;
    }

    filesize = vfs_lseek64(file_node, 0, SEEK_END);
    vfs_lseek64(file_node, 0, SEEK_SET);
    user_prog = kmalloc(filesize);
    vfs_read(file_node, user_prog, filesize);

    mappages(&current->mm, 0x0, 250000, VIRT_2_PHY(user_prog));
    mappages(&current->mm, USER_STK_LOW, STACKSIZE, 0);

    asm volatile("msr sp_el0, %0" : : "r"(USER_STK_HIGH));
    asm volatile("msr elr_el1, %0": : "r"(0x0));
    asm volatile("msr spsr_el1, %0" : : "r"(0b0));
    asm volatile("eret");
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

void __kill(int pid) {
    if (pid <= 0)
        return;
    struct task_struct* target = get_task_struct(pid);
    target->state = DEAD;
}