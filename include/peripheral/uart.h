#ifndef UART_H
#define UART_H

void uart_init();
void uart_enable_int();
void uart_disable_int();
void uart_handler();

void uart_flush();

char uart_sync_read();
char uart_sync_read_raw();
void uart_sync_write(unsigned int c);
void uart_sync_puts(char *s);
void uart_sync_printNum(long num, int base);

char uart_async_read();
void uart_async_write(unsigned int c);
void uart_async_puts(char *s, int enable_tx);
void uart_async_printNum(long num, int base, int enable_tx);
void uart_write_flush();


#define ENABLE_IRQS_1_AUX   (*ENABLE_IRQS_1 |= AUX_INT)
#define DISABLE_IRQS_1_AUX  (*DISABLE_IRQS_1 |= AUX_INT)

#define uart_read               uart_async_read
#define uart_write              uart_async_write
#define uart_puts(x)            uart_async_puts(x, 1)
#define uart_puts_int(x)        uart_async_puts(x, 0)
#define uart_printNum(x, y)     uart_async_printNum(x, y, 1)
#define uart_printNum_int(x, y) uart_async_printNum(x, y, 0)

#endif