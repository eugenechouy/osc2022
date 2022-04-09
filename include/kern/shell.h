#ifndef SHELL_H
#define SHELL_H

#define BACKSPACE       8
#define ESC             27
#define DELETE          127

#define LEFT_SHIFT kputc(ESC); \
                   kputc('['); \
                   kputc('D');

#define RIGHT_SHIFT kputc(ESC); \
                    kputc('['); \
                    kputc('C');

#define MAX_INPUT_LEN 128

void shell_input(char *cmd);
void shell_parse(char *cmd);
void shell_start();

#endif