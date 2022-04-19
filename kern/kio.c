#include "string.h"
#include "kern/kio.h"

void kprintf(char* fmt, ...) {
    char s[124];
    char buffer[64];
    char *dst = s;
    char *p;

    __builtin_va_list args;
    __builtin_va_start(args, fmt);

    while (*fmt) { 
        if (*fmt == '%') {
            fmt++;
            // escape
            if (*fmt == '%') {
                goto put;
            }
            // string
            else if (*fmt == 's') {
                p = __builtin_va_arg(args, char *);
                while (*p) 
                    *dst++ = *p++;
            }
            // char
            else if (*fmt == 'c') {
                char c = __builtin_va_arg(args, int);
                *dst++ = c;
            }
            // decimal
            else if (*fmt == 'd') {
                int arg = __builtin_va_arg(args, int);
                if (itoa(arg, buffer, 10) != -1) {
                    p = buffer;
                    while(*p) 
                        *dst++ = *p++;
                } else {
                    *dst++ = '@';
                }
                
            }
            // hex
            else if (*fmt == 'x') {
                long arg = __builtin_va_arg(args, long);
                if (itoa(arg, buffer, 16) != -1) {
                    p = buffer;
                    while(*p) 
                        *dst++ = *p++;
                } else {
                    *dst++ = '@';
                }
            }
        } 
        else {
        put:
            *dst++ = *fmt;
        }
        *fmt++;
    }
    *dst = '\0';
    __builtin_va_end(args);

    uart_async_puts(s);
}
