#ifndef UART_H
#define UART_H

void uart_init();
void uart_enable_int();
void uart_handler();

void uart_flush();

char uart_sync_read();
char uart_sync_read_raw();
void uart_sync_write(unsigned int c);
void uart_sync_puts(char *s);
void uart_sync_printNum(long num, int base);

char uart_async_read();
void uart_async_write(unsigned int c);
void uart_async_puts(char *s);
void uart_async_printNum(long num, int base);

#define uart_read uart_async_read
#define uart_write uart_async_write
#define uart_puts uart_async_puts
#define uart_printNum uart_async_printNum

#endif