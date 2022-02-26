#include "aux.h"
#include "gpio.h"

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
    while(r--) { 
        asm volatile("nop"); 
    }
    *GPPUDCLK0 = (1<<14) | (1<<15);     // Write to GPPUDCLK0/1 to clock the control signal into the GPIO pads you wish to modify
    r = 150;                            // Wait 150 cycles
    while(r--) { 
        asm volatile("nop"); 
    }
    *GPPUDCLK0 = 0;                     // Write to GPPUDCLK0/1 to remove the clock 

    *AUX_MU_CNTL = 3;       // Enable the transmitter and receiver
}

char uart_read() {
    char r;
    // Check AUX_MU_LSR_REG’s data ready field (Bit 0)
    do {
        asm volatile("nop"); 
    } while(!(*AUX_MU_LSR & 0x01));
    r = (char)(*AUX_MU_IO);
    return r == '\r' ? '\n' : r;
}

void uart_write(unsigned int c) {
    // Check AUX_MU_LSR_REG’s Transmitter empty field (Bit 5)
    do {
        asm volatile("nop"); 
    } while(!(*AUX_MU_LSR & 0x20));
    *AUX_MU_IO = c;
}

void uart_puts(char *s) {
    while(*s) {
        if (*s == '\n') 
            uart_write('\r');
        uart_write(*s++);
    }
}