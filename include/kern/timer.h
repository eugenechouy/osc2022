#ifndef TIMER_H
#define TIMER_H

struct timer_queue {
    unsigned long   register_time;
    unsigned int    duration;
    char            message[128];
    struct timer_queue  *prev;
    struct timer_queue  *next; 
    void (*callback)(char *);
};

struct timer_queue *timer_head;
struct timer_queue *timer_tail;

void timer_el0_handler();
void timer_el1_handler();
void timer_unknown_handler();
void timer_init();
void set_timeout(char *args);

extern void core_timer_enable();
extern void core_timer_disable();


#endif