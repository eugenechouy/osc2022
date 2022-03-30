#ifndef SCHED_H
#define SCHED_H

#define MAX_TASK_NUM 64

enum task_state {
    RUNNING, READY, WAITING
};

struct task_struct {

    int                 tid;
    int                 used;
    
    enum task_state     state;
    
    int                 prio;

    int                 ctime;

    int                 preemptible;

    struct task_struct *next;

    void *cb_args;
    void (*cb)(void*);
};


struct task_struct *current;
struct task_struct *task_queue_head;

struct task_struct task_pool[MAX_TASK_NUM];

void init_task_queue();
void task_create(void (*cb)(void*), void *args, int prio);
void task_run();

#endif 