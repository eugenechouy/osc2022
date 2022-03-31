#include "peripheral/aux.h"
#include "peripheral/gpio.h"
#include "peripheral/uart.h"
#include "peripheral/interrupt.h"
#include "string.h"

#define MAX_BUFFER_SIZE 1024

// If this bit is set the interrupt line is asserted whenever the transmit FIFO is empty.
#define ENABLE_TX_INT   (*AUX_MU_IER |= 2)
#define DISABLE_TX_INT  (*AUX_MU_IER &= ~(2))

char read_buffer[MAX_BUFFER_SIZE];
char write_buffer[MAX_BUFFER_SIZE];

int read_start, 
    read_end;
int write_start, 
    write_end;

void uart_init() {
    
    // Mini UART initialization
    *AUX_ENABLES |= 1;      // Enable mini UART
    *AUX_MU_CNTL = 0;       // Disable transmitter and receiver during configuration
    *AUX_MU_IER  = 0;       // Disable interrupt
    *AUX_MU_LCR  = 3;       // Set the data size to 8 bit
    *AUX_MU_MCR  = 0;       // Don’t need auto flow control
    *AUX_MU_BAUD = 270;     // Set baud rate to 115200
    *AUX_MU_IIR  = 6;       // No FIFO

    // configure GPIO pin
    // 1. configure GPFSELn register to change alternate function
    // spec p.92 p.102
    register unsigned int r = *GPFSEL1;
    r &= ~((7 << 12) | (7 << 15));  // Reset GPIO 14, 15  ex: RRP19P18P17P16P15P14P13P12P11P10
                                    //                                      000000
    r |= (2 << 12) | (2 << 15);     // Set to ALT5 (010 = GPIO Pin xx takes alternate function 5)
    *GPFSEL1 = r;
    
    // 2. configure pull up/down register to disable GPIO pull up/down
    // spec p.101
    *GPPUD = 0;                         // Write to GPPUD to set the required control signal
    r = 150;                            // Wait 150 cycles
    while(r--) asm volatile("nop"); 
    *GPPUDCLK0 = (1<<14) | (1<<15);     // Write to GPPUDCLK0/1 to clock the control signal into the GPIO pads you wish to modify
    r = 150;                            // Wait 150 cycles
    while(r--) asm volatile("nop");
    *GPPUDCLK0 = 0;                     // Write to GPPUDCLK0/1 to remove the clock 

    *AUX_MU_CNTL = 3;       // Enable the transmitter and receiver

    read_start = read_end = 0;
    write_start = write_end = 0;

    uart_sync_puts("UART initialized successfully!\n");
}

void uart_enable_int() {
    // spec p.12
    *AUX_MU_IER |= 1;
    ENABLE_IRQS_1_AUX;
}

void uart_disable_int() {
    *AUX_MU_IER = 0;
    DISABLE_IRQS_1_AUX;
    DISABLE_TX_INT;
}


void uart_handler() {
    char r;
    int tx = *AUX_MU_IIR & 0b10;
    int rx = *AUX_MU_IIR & 0b100;
    if (rx) {
        r = (char)(*AUX_MU_IO);
        read_buffer[read_end] = r;
        read_end = (read_end + 1) % MAX_BUFFER_SIZE;
    } else if (tx) {
        while(*AUX_MU_LSR & 0x20) {
            if (write_start == write_end) {
                break;
            }
            *AUX_MU_IO = write_buffer[write_start];
            write_start = (write_start + 1) % MAX_BUFFER_SIZE;
        }
    }
    uart_enable_int();
}

void uart_flush() {
    while(*AUX_MU_LSR & 0x01) 
        *AUX_MU_IO;
}

/* sync */

char uart_sync_read() {
    char r;
    // Check AUX_MU_LSR_REG’s data ready field (Bit 0)
    do {
        asm volatile("nop"); 
    } while(!(*AUX_MU_LSR & 0x01));
    r = (char)(*AUX_MU_IO);
    return r == '\r' ? '\n' : r;
}

char uart_sync_read_raw() {
    do {
        asm volatile("nop"); 
    } while(!(*AUX_MU_LSR & 0x01));
    return (char)(*AUX_MU_IO);
}

void uart_sync_write(unsigned int c) {
    // Check AUX_MU_LSR_REG’s Transmitter empty field (Bit 5)
    do {
        asm volatile("nop"); 
    } while(!(*AUX_MU_LSR & 0x20));
    *AUX_MU_IO = c;
}

void uart_sync_puts(char *s) {
    while(*s) {
        if (*s == '\n') 
            uart_sync_write('\r');
        uart_sync_write(*s++);
    }
}

void uart_sync_printNum(long num, int base) {
    char buffer[64];
    itoa(num, buffer, base);
    uart_sync_puts(buffer);
}

/* async */

char uart_async_read() {
    char r;
    while (read_start == read_end) {
        asm volatile("nop"); 
    }
    r = read_buffer[read_start];
    read_start = (read_start + 1) % MAX_BUFFER_SIZE;
    return r == '\r' ? '\n' : r;
}

void uart_async_write(unsigned int c) {
    write_buffer[write_end] = c;
    write_end = (write_end + 1) % MAX_BUFFER_SIZE;
    ENABLE_TX_INT;
}

void uart_async_write_buffer(unsigned int c) {
    write_buffer[write_end] = c;
    write_end = (write_end + 1) % MAX_BUFFER_SIZE;
}

void uart_async_puts(char *s, int enable_tx) {
    while(*s) {
        if (*s == '\n')
            uart_async_write_buffer('\r');
        uart_async_write_buffer(*s++);
    }
    if (enable_tx) ENABLE_TX_INT;
}

void uart_async_printNum(long num, int base, int enable_tx) {
    char buffer[64];
    if (itoa(num, buffer, base) == -1) {
        uart_async_puts("Error", enable_tx);
        return;
    }
    uart_async_puts(buffer, enable_tx);
}

void uart_write_flush() {
    if (write_start != write_end) 
        ENABLE_TX_INT;
}