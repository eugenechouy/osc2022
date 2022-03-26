#ifndef UART_H
#define UART_H

void uart_init();
void uart_flush();
char uart_read();
char uart_read_raw();
void uart_write(unsigned int c);

void uart_puts(char *s);
void uart_printNum(int num, int base);

#endif