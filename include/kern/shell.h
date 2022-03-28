#ifndef SHELL_H
#define SHELL_H

#define BACKSPACE       8
#define ESC             27
#define DELETE          127

#define LEFT_SHIFT uart_write(ESC); \
                   uart_write('['); \
                   uart_write('D');

#define RIGHT_SHIFT uart_write(ESC); \
                    uart_write('['); \
                    uart_write('C');

#define MAX_INPUT_LEN 128

void shell_input(char *cmd);
void shell_parse(char *cmd);

#endif