#ifndef KIO_H
#define KIO_H

#include "peripheral/uart.h"

static inline void kio_init() {
    uart_init();
    uart_flush();
    uart_enable_int();
}

static inline void kputc(char c) 
{
    uart_async_write(c);
}

static inline void kputs(char *s) 
{
    uart_async_puts(s);
}

static inline void kflush() {
    uart_write_flush();
}

static inline char kscanc() {
    return uart_async_read();
}

void kprintf(char* fmt, ...);

#endif