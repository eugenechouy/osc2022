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
    uart_puts("Seconds after booting: ");
    uart_printNum(get_current_time(), 10);
    uart_puts("\n");
    set_expired(2);
}

void timer_el1_handler() {
    struct timer_queue *next;
    unsigned long timeout;

    uart_puts("Seconds after booting: ");
    uart_printNum(get_current_time(), 10);
    uart_puts("\n");

    head->callback(head->message);
    next = head->next;
    if (next) {
        next->prev = 0;
        head = next;
        timeout = next->register_time + next->duration - get_current_time();
        set_expired(timeout);
    } else {
        head = 0;
        tail = 0;
        core_timer_disable();
    }
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

    if (!head) {
        head = new_timer;
        tail = new_timer;
        core_timer_enable();
        set_expired(duration);
    } else {
        timeout = new_timer->register_time + new_timer->duration;
        for(itr=head ; itr ; itr=itr->next) {
            if(itr->register_time + itr->duration > timeout)
                break;
        }

        if (!itr) { // tail
            new_timer->prev = tail;
            tail->next      = new_timer;
            tail            = new_timer;
        } else if (!itr->prev) { // head
            new_timer->next = head;
            head->prev      = new_timer;
            head            = new_timer;
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
	uart_puts(msg);
    uart_puts("\n");
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
    if (duration <= 0 || duration >= 1e5) {
        uart_puts("setTimeout: time error\n");
        return;
    }
    add_timer(timer_callback, args, duration);
}