#ifndef __DATA_H__
#define __DATA_H__

#include "../common/common.h"

enum data_errno
{
    /* proto */
    ERR_DATA_BROKEN = 100,

    /* queue */
    ERR_DATA_QUEUE_EMPTY = 120,
    ERR_DATA_QUEUE_FULL = 121,
    ERR_DATA_QUEUE_STOPPED = 122,
    ERR_DATA_QUEUE_RUNNING = 123,
    ERR_DATA_QUEUE_SIZE_INSUFF = 124,
    ERR_DATA_QUEUE_EXIST = 125,

    /* route */
    ERR_DATA_ROUTE_UNREACHABLE = 130,
    ERR_DATA_ROUTE_LINK_BUSY = 131,
    ERR_DATA_ROUTE_NODE_BROKEN = 132,
    ERR_DATA_ROUTE_ABILITY_NOT_FOUND = 133,
    ERR_DATA_ROUTE_DEV_NOT_FOUND = 134,
};

#endif // __DATA_H__