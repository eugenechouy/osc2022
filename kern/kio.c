// #include "kern/kio.h"

// unsigned int sprintf(char *dst, char *fmt) {
//     char *dst_orig = dst;

//     while (*fmt) {
//         // if (*fmt == '%') {
//         //     fmt++;
//         //     // escape %
//         //     if (*fmt == '%') {
//         //         goto put;
//         //     }
//         //     // string
//         //     if (*fmt == 's') {
//         //         char *p = __builtin_va_arg(args, char *);
//         //         while (*p) {
//         //             *dst++ = *p++;
//         //         }
//         //     }
//         //     // char
//         //     if (*fmt == 'c') {
//         //         char c = __builtin_va_arg(args, int);
//         //         *dst++ = c;
//         //     }
//         //     // number
//         //     if (*fmt == 'd') {
//         //         int arg = __builtin_va_arg(args, int);
//         //         char buf[11];
//         //         char *p = itoa(arg, buf);
//         //         while (*p) {
//         //             *dst++ = *p++;
//         //         }
//         //     }
//         //     // hex
//         //     if (*fmt == 'x') {
//         //         uint64_t arg = __builtin_va_arg(args, uint64_t);
//         //         char buf[64 + 1];
//         //         char *p = itox64(arg, buf);
//         //         while (*p) {
//         //             *dst++ = *p++;
//         //         }
//         //     }
//         //     // float
//         //     if (*fmt == 'f') {
//         //         float arg = (float)__builtin_va_arg(args, double);
//         //         char buf[19];  // sign + 10 int + dot + 6 float
//         //         char *p = ftoa(arg, buf);
//         //         while (*p) {
//         //             *dst++ = *p++;
//         //         }
//         //     }
//         // }
//         // else {
//         put:
//             *dst++ = *fmt;
//         // }
//         fmt++;
//     }
//     *dst = '\0';

//     return dst - dst_orig;  // return written bytes
// }

// void kprintf(char* fmt, ...) {
    
//     __builtin_va_list args;
//     __builtin_va_start(args, fmt);

//     char s[124];
//     sprintf(s, fmt);

//     __builtin_va_end(args);

//     // uart_async_puts(s);

//     // char* s = &str[0];
//     // while (*s) {
//     //     if (*s == '\n') uart0_write('\r');
//     //     uart0_write(*s++);
//     // }
// }
