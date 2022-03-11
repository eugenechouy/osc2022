
extern unsigned char __begin, __end, __bootloader;

__attribute__((section(".text.relocate"))) void relocate() {
    unsigned long size    = (&__end - &__begin);
    unsigned char *from   = (unsigned char *)&__begin;
    unsigned char *to     = (unsigned char *)&__bootloader;

    while (size--) {
        *to++ = *from++;
    }

    void (*start)(void) = (void *)&__bootloader;
    start();
}