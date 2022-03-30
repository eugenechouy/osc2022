#include "peripheral/uart.h"
#include "kern/timer.h"
#include "simple_alloc.h"
#include "string.h"

unsigned long get_current_time() {
    unsigned long cntpct;
    unsigned long cntfrq;
    asm volatile("mrs %0, cntpct_el0" : "=r"(cntpct));
    asm volatile("mrs %0, cntfrq_el0" : "=r"(cntfrq));
    return cntpct/cntfrq;
}

void set_expired(unsigned int seconds) {
    unsigned long cntfrq;
    asm volatile("mrs %0, cntfrq_el0" : "=r"(cntfrq));
    asm volatile("msr cntp_tval_el0, %0" : : "r"(cntfrq * seconds));
}

void timer_el0_handler() {
    uart_puts_int("Seconds after booting: ");
    uart_printNum_int(get_current_time(), 10);
    uart_puts_int("\n");
    core_timer_enable();
    set_expired(2);
}

void timer_el1_handler() {
    struct timer_queue *next;
    unsigned long timeout;

    uart_puts_int("Seconds after booting: ");
    uart_printNum_int(get_current_time(), 10);
    uart_puts_int("\n");

    timer_head->callback(timer_head->message);
    next = timer_head->next;
    if (next) {
        next->prev = 0;
        timer_head = next;
        timeout = next->register_time + next->duration - get_current_time();
        core_timer_enable();
        set_expired(timeout);      
    } else {
        timer_head = 0;
        timer_tail = 0;
    }
}

void timer_unknown_handler() {
    uart_puts_int("Timer interrupt: unknown source EL\n");
    core_timer_enable();
}


void timer_init() {
    timer_head = 0;
    timer_tail = 0;
}

void add_timer(void (*callback)(char *), char *message, unsigned int duration) {
    struct timer_queue *new_timer = (struct timer_queue *)simple_malloc(sizeof(struct timer_queue));
    struct timer_queue *itr;
    unsigned long timeout;
    int i;

    new_timer->register_time = get_current_time();
    new_timer->duration      = duration;
    new_timer->callback      = callback;
    for(i=0 ; message[i]!='\0' ; i++) 
        new_timer->message[i] = message[i];
    new_timer->message[i] = '\0';
    new_timer->prev = 0;
    new_timer->next = 0;

    if (!timer_head) {
        timer_head = new_timer;
        timer_tail = new_timer;
        core_timer_enable();
        set_expired(duration);
    } else {
        timeout = new_timer->register_time + new_timer->duration;
        for(itr=timer_head ; itr ; itr=itr->next) {
            if(itr->register_time + itr->duration > timeout)
                break;
        }

        if (!itr) { // tail
            new_timer->prev     = timer_tail;
            timer_tail->next    = new_timer;
            timer_tail          = new_timer;
        } else if (!itr->prev) { // head
            new_timer->next     = timer_head;
            timer_head->prev    = new_timer;
            timer_head          = new_timer;
            set_expired(duration);
        } else { // middle
            new_timer->prev = itr->prev;
            new_timer->next = itr;
            itr->prev->next = new_timer;
            itr->prev       = new_timer;
        }
    }
}

void timer_callback(char *msg) {
	uart_puts_int(msg);
    uart_puts_int("\n");
}

void set_timeout(char *args) {
    int i;
    int duration;
    int message_end = -1;

    for(i=0 ; args[i]!='\0' ; i++) if (args[i] == ' ') {
        message_end = i;
        break;
    }
    if (message_end == -1) {
        uart_puts("setTimeout: MESSAGE SECONDS\n");
        return;
    }
    args[message_end] = '\0';
    duration = atoi(args+message_end+1, 10, strlen(args+message_end+1));
    if (duration <= 0 || duration >= 35) {
        uart_puts("setTimeout: time error\n");
        return;
    }
    uart_puts("Timeout: ");
    uart_printNum(duration, 10);
    uart_puts("s\n");
    add_timer(timer_callback, args, duration);
}