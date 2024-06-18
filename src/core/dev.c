#include <stdio.h>
#include <stdlib.h>

#include "dev.h"
#include "common.h"

int dev_init(struct device_ctrl_block *dcb)
{
    if (dcb == NULL) {
        return -DEVICE_ERR_PARAMETER;
    }

    dcb->count = 0;

    // TODO: Initialize main device

    return 0;
}

int dev_deinit(struct device_ctrl_block *dcb)
{
    if (dcb == NULL) {
        return -DEVICE_ERR_PARAMETER;
    }

    // TODO: Deinitialize main device

    return 0;
}

struct device_ctrl_block *dev_alloc(void)
{
    return(struct device_ctrl_block *)malloc(sizeof(struct device_ctrl_block));
}

void dev_free(struct device_ctrl_block *dcb)
{
    free(dcb);
}
