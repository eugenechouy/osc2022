#ifndef BITMAP_H
#define BITMAP_H

#define BITS_PER_BYTE           8
#define DIV_ROUND_UP(n,d) (((n) + (d) - 1) / (d))
#define BITS_TO_LONGS(nr)       DIV_ROUND_UP(nr, BITS_PER_BYTE * sizeof(long))

#define DECLARE_BITMAP(name,bits) \
    unsigned long name[BITS_TO_LONGS(bits)]

static inline void bitmap_zero(unsigned long *dst, unsigned int nbits) {
    int i;
    for(i=0 ; i<BITS_TO_LONGS(nbits) ; i++) {
        dst[i] = 0;
    }
}

#define BITS_PER_LONG       64
#define BIT_MASK(nr)       (1UL << ((nr % BITS_PER_LONG)))
#define BIT_WORD(nr)       ((nr) / BITS_PER_LONG)

static inline unsigned long __ffs(unsigned long word) {
    int num = 0;
#if BITS_PER_LONG == 64
    if ((word & 0xffffffff) == 0) {
        num += 32;
        word >>= 32;
    }
#endif
    if ((word & 0xffff) == 0) {
        num += 16;
        word >>= 16;
    }
    if ((word & 0xff) == 0) {
        num += 8;
        word >>= 8;
    }
    if ((word & 0xf) == 0) {
        num += 4;
        word >>= 4;
    }
    if ((word & 0x3) == 0) {
        num += 2;
        word >>= 2;
    }
    if ((word & 0x1) == 0) 
        num += 1;
    return num;
}

/*
bitops
*/
static inline void __set_bit(int nr, volatile unsigned long *addr) {
    unsigned long mask = BIT_MASK(nr);
    unsigned long *p = ((unsigned long *) addr) + BIT_WORD(nr);
    *p |= mask;
}

static inline void __clear_bit(int nr, volatile unsigned long *addr) {
    unsigned long mask = BIT_MASK(nr);
    unsigned long *p = ((unsigned long *) addr) + BIT_WORD(nr);
    *p &= ~mask;
}

#endif