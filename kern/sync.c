#include "kern/timer.h"
#include "kern/kio.h"
#include "reset.h"

void sync_main(unsigned long spsr, unsigned long elr, unsigned long esr) {
    unsigned int svc_num; 

    svc_num = esr & 0xFFFFFF;
    switch(svc_num) {
    case 0:
        /*
        bits [31:26] 0b010101 SVC instruction execution in AArch64 state.
        */
        kprintf("\nspsr_el1: \t%x\n", spsr);
        kprintf("elr_el1: \t%x\n", elr);
        kprintf("esr_el1: \t%x\n", esr);
        break;
    case 1:
        kputs("svc 1\n");
        core_timer_enable();
        break;
    case 4:
        kputs("svc 4\n");
        break;
    default:
        uart_sync_puts("Undefined svc number, about to reboot...\n");
        reset(1000);
        while(1);
    }
}