#ifndef BYTESWAP_H
#define BYTESWAP_H

/* Swap bytes in 32-bit value.  */
#define __bswap_32(x) \
  ((((x) & 0xff000000u) >> 24) | (((x) & 0x00ff0000u) >> 8)        \
   | (((x) & 0x0000ff00u) << 8) | (((x) & 0x000000ffu) << 24))

#endif 