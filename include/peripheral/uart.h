#ifndef UART_H
#define UART_H

void uart_init();

void uart_enable_int();
void uart_disable_int();
void uart_handler();

char uart_sync_read();
char uart_sync_read_raw();
void uart_sync_write(unsigned int c);
void uart_sync_puts(char *s);
void uart_sync_printNum(long num, int base);

char uart_async_read();
void uart_async_write(unsigned int c);
void uart_async_puts(char *s);

void uart_flush();
void uart_write_flush();

 
#define ENABLE_IRQS_1_AUX   (*ENABLE_IRQS_1 |= AUX_INT)
#define DISABLE_IRQS_1_AUX  (*DISABLE_IRQS_1 |= AUX_INT)

#endif