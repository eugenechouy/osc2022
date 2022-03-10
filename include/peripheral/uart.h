#ifndef UART_H
#define UART_H

void uart_init();
char uart_read();
void uart_write(unsigned int c);

void uart_puts(char *s);

#endif