#ifndef SCHED_H
#define SCHED_H

#include "list.h"
#include "bitmap.h"
#include "signal.h"

#define MAX_PRIV_TASK_NUM 50
#define TASK_CTIME        1

enum task_state {
    RUNNING, READY, WAITING, INT, DEAD
};

struct task_context {
    long x19;
    long x20;
    long x21;
    long x22;
    long x23;
    long x24;
    long x25;
    long x26;
    long x27;
    long x28;
    long fp;
    long lr;
    long sp;
};

struct task_struct {

    int                 tid;
    int                 used;
    
    enum task_state     state;
    
    int                 prio;

    int                 ctime;
    int                 resched;

    void               *stk_addr;
    void               *ustk_addr; 

    struct list_head signal_list;
    struct signal_context_t *signal_context;
    struct task_context task_context;

    struct list_head list;
};

#define MAX_PRIO 128

static inline int sched_find_first_bit(const unsigned long *b) {
    if (b[0])
        return __ffs(b[0]);
    if (b[1])
        return __ffs(b[1]) + 64;
    return 128;
}

void task_init();
void runqueue_init();
struct task_struct *privilege_task_create(void (*func)(), int prio);
struct task_struct *task_create(void (*func)(), int prio);

void schedule();
void kill_zombies();

void switch_to(struct task_context *prev, struct task_context *next);
void update_current(struct task_struct *task);
struct task_struct* get_current();

int __getpid();
void __exec(const char *name, char *const argv[]);
int __fork(void *trapframe);
void __exit();
void __kill(int pid);

void do_exec(void (*func)());

static inline void thread_create(void (*func)()) {
    task_create(func, 100);
}

#endif 