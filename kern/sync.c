#include "peripheral/uart.h"
#include "kern/timer.h"

void sync_main(unsigned long spsr, unsigned long elr, unsigned long esr) {
    unsigned int svc_num; 

    svc_num = esr & 0xFFFFFF;
    switch(svc_num) {
    case 0:
        uart_puts("\nspsr_el1: \t");
        uart_printNum(spsr, 16);
        uart_puts("\nelr_el1: \t");
        uart_printNum(elr, 16);
        uart_puts("\nesr_el1: \t");
        uart_printNum(esr, 16);
        /*
        bits [31:26] 0b010101 SVC instruction execution in AArch64 state.
        */
        uart_puts("\n");
        break;
    case 1:
        uart_puts("svc 1\n");
        core_timer_enable();
        break;
    case 4:
        uart_puts("svc 4\n");
        
        break;
    default:
        uart_puts("Undefined svc number\n");
        break;
    }
}