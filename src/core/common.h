#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>
#include <stdbool.h>

#define	GET_BYTE0(x)	((x >>  0) & 0x000000ff)
#define	GET_BYTE1(x)	((x >>  8) & 0x000000ff)
#define	GET_BYTE2(x)	((x >> 16) & 0x000000ff)
#define	GET_BYTE3(x)	((x >> 24) & 0x000000ff)

#define	CLEAR_BYTE0(x)	(x &= 0xffffff00)
#define	CLEAR_BYTE1(x)	(x &= 0xffff00ff)
#define	CLEAR_BYTE2(x)	(x &= 0xff00ffff)
#define	CLEAR_BYTE3(x)	(x &= 0x00ffffff)

#define	SWAP_BYTES(x)	    (((x & 0xff000000) >> 24) | ((x & 0x00ff0000) >> 8) | \
                            ((x & 0x0000ff00) << 8) | ((x & 0x000000ff) << 24))

#define	SET_BIT(x, bit)	    (x |= (1 << bit))
#define	GET_BIT(x, bit)	    ((x & (1 << bit)) >> bit)
#define CLEAR_BIT(x, bit)	(x &= ~(1 << bit))

#endif /* COMMON_H */