#include "peripheral/mailbox.h"
#include "string.h"
#include "kern/mm_types.h"

unsigned int mailbox_call(unsigned char ch, unsigned int* mailbox) {
    unsigned int message;
    unsigned int data;

    // Combine the message address (upper 28 bits) with channel number (lower 4 bits)
    message = ((unsigned long)VIRT_2_PHY(mailbox) & 0xfffffff0) | (ch & 0xf);
    // Check if Mailbox 0 status register’s full flag is set
    while((*MAILBOX_STATUS & MAILBOX_FULL)) asm volatile("nop"); 
    // write to Mailbox 1 Read/Write register
    *MAILBOX_WRITE = message;
    while(1) {
        // Check if Mailbox 0 status register’s empty flag is set
        while((*MAILBOX_STATUS & MAILBOX_EMPTY)) asm volatile("nop"); 
        // read from Mailbox 0 Read/Write register
        data = (unsigned int)(*MAILBOX_READ);
        // Check if the value is the same as you wrote
        if (data == message)  {
            return mailbox[1] == REQUEST_SUCCEED;
        }
    }
    return 0;
}

/*
https://github.com/raspberrypi/firmware/wiki/Mailbox-property-interface
Get board revision
    Tag: 0x00010002
    Request:
        Length: 0
    Response:
        Length: 4
        Value:
            u32: board revision
*/
void get_board_revision(unsigned int *result){
    unsigned int mailbox[7];

    mailbox[0] = 7 * 4; // buffer size in bytes
    mailbox[1] = REQUEST_CODE;
    // tags begin
    mailbox[2] = GET_BOARD_REVISION; // tag identifier
    mailbox[3] = 4; // maximum of request and response value buffer's length.
    mailbox[4] = TAG_REQUEST_CODE;
    mailbox[5] = 0; // value buffer
    // tags end
    mailbox[6] = END_TAG;

    // message passing procedure call
    if (mailbox_call(0x8, mailbox)) {
        // it should be 0xa020d3 for rpi3 b+
        result[0] = mailbox[5];
    }
}

/*
Get ARM memory
    Tag: 0x00010005
    Request:
        Length: 0
    Response:
        Length: 8
        Value:
            u32: base address in bytes
            u32: size in bytes
*/
void get_ARM_memory(unsigned int *result) {
    unsigned int mailbox[8];

    mailbox[0] = 8 * 4; // buffer size in bytes
    mailbox[1] = REQUEST_CODE;
    // tags begin
    mailbox[2] = GET_ARM_MEMORY; // tag identifier
    mailbox[3] = 8; // maximum of request and response value buffer's length.
    mailbox[4] = TAG_REQUEST_CODE;
    mailbox[5] = 0; // value buffer
    mailbox[6] = 0; // value buffer
    // tags end
    mailbox[7] = END_TAG;

    // message passing procedure call
    if (mailbox_call(0x8, mailbox)) {
        result[0] = mailbox[5];
        result[1] = mailbox[6];
    }
}