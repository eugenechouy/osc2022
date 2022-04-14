#ifndef SCHED_H
#define SCHED_H

#define MAX_TASK_NUM 64

enum task_state {
    RUNNING, READY, WAITING, INT, DEAD
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

void task_queue_init();
void task_create(void (*cb)(void*), void *args, int prio);
void task_run();
void task_state_update();

#endif 