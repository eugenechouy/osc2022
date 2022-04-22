#include "bitmap.h"
#include "kern/softirq.h"

struct irq_struct {
    void (*cb)();
    enum irq_state state;
};
struct irq_struct irqueue[SOFTIRQ_NUM];

DECLARE_BITMAP(irqueue_map, SOFTIRQ_NUM);

inline int find_first_bit(const unsigned long *b) {
    if (b[0])
        return __ffs(b[0]);
    return 64;
}

void softirq_init() {
    bitmap_zero(irqueue_map, SOFTIRQ_NUM);
}

void softirq_register(void (*cb)(), int num) {
    irqueue[num].cb = cb;
    irqueue[num].state = IRQ_IDLE;
}

void softirq_active(int num) {
    __set_bit(num, irqueue_map);
    irqueue[num].state = IRQ_READY;
}

void softirq_run() {
    int num = find_first_bit(irqueue_map);

    while (num != 64) {
        // higher priority interrupt is running, return
        if (irqueue[num].state == IRQ_RUNNING)
            break;
        irqueue[num].state = IRQ_RUNNING;
        irqueue[num].cb();
        __clear_bit(num, irqueue_map);
        irqueue[num].state = IRQ_IDLE;
        num = find_first_bit(irqueue_map);
    }
}