#include "peripheral/uart.h"

void svc_handler(unsigned long spsr, unsigned long elr, unsigned long esr) {
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
        uart_puts("\n");
        break;
    case 4:
        uart_puts("svc 4\n");
        break;
    default:
        uart_puts("Undefined svc number\n");
        break;
    }
}