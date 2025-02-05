#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdint.h>

#include "checksum.h"

/* Max common errno 99 */
enum common_errno {
    ERR_OK = 0,
    ERR_NOTFOUND,
    ERR_NOMEM,
    ERR_INVALID,
    ERR_NOTSUPPORTED,
};


#define	GET_BIT(x, bit)	  ((x & (1 << bit)) >> bit)
#define	SET_BIT(x, bit)	  (x |= (1 << bit))
#define	CLEAR_BIT(x, bit) (x &= ~(1 << bit))

#define	CLEAR_BYTE(x, mask)	(x &= mask)

#endif // __COMMON_H__
