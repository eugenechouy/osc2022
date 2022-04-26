#ifndef MAILBOX_H
#define MAILBOX_H

#include "mmio.h"

#define MAILBOX_BASE    (MMIO_BASE + 0xb880)

#define MAILBOX_READ    ((volatile unsigned int*)(MAILBOX_BASE))
#define MAILBOX_STATUS  ((volatile unsigned int*)(MAILBOX_BASE + 0x18))
#define MAILBOX_WRITE   ((volatile unsigned int*)(MAILBOX_BASE + 0x20))

#define MAILBOX_EMPTY       0x40000000
#define MAILBOX_FULL        0x80000000

// Tag
#define GET_BOARD_REVISION  0x00010002
#define GET_ARM_MEMORY      0x00010005
#define REQUEST_CODE        0x00000000
#define REQUEST_SUCCEED     0x80000000
#define REQUEST_FAILED      0x80000001
#define TAG_REQUEST_CODE    0x00000000
#define END_TAG             0x00000000

/*
Mailbox 0 defines the following channels:

0: Power management
1: Framebuffer
2: Virtual UART
3: VCHIQ
4: LEDs
5: Buttons
6: Touch screen
7:
8: Property tags (ARM -> VC)
9: Property tags (VC -> ARM)
*/
#define MAILBOX_CH_PM       0
#define MAILBOX_CH_FB       1
#define MAILBOX_CH_VUART    2
#define MAILBOX_CH_VCHIQ    3
#define MAILBOX_CH_LEDS     4
#define MAILBOX_CH_BTNS     5
#define MAILBOX_CH_TS       6
// 7
#define MAILBOX_CH_ARM2VC   8
#define MAILBOX_CH_VC2ARM   9  

unsigned int mailbox_call(unsigned char ch, unsigned int* mailbox);
void get_board_revision(unsigned int *result);
void get_ARM_memory(unsigned int *result);

#endif