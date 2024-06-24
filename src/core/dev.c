#include <stdio.h>
#include <stdlib.h>

#include "dev.h"
#include "common.h"

int ability_acb_init(struct ability_ctrl_block *acb)
{
    if (acb == NULL) {
        return -DEVICE_ERR_PARAMETER;
    }

    acb->count = 0;

    // TODO: Initialize ability

    return DEVICE_ERR_NONE;
}

int ability_acb_deinit(struct ability_ctrl_block *acb)
{
    if (acb == NULL) {
        return -DEVICE_ERR_PARAMETER;
    }

    // TODO: Deinitialize ability

    return DEVICE_ERR_NONE;
}

struct ability_ctrl_block *acb_alloc(void)
{
    return(struct ability_ctrl_block *)malloc(sizeof(struct ability_ctrl_block));
}

void acb_free(struct ability_ctrl_block *acb)
{
    free(acb);
}

int device_init(struct device *dev)
{
    // TODO: Initialize main device

    return DEVICE_ERR_NONE;
}

int device_deinit(struct device *dev)
{
    // TODO: Deinitialize main device

    return DEVICE_ERR_NONE;
}

struct device *device_alloc(void)
{
    return(struct device *)malloc(sizeof(struct device));
}

void device_free(struct device *dev)
{
    free(dev);
}

int device_dcb_init(struct device_ctrl_block *dcb)
{
    if (dcb == NULL) {
        return -DEVICE_ERR_PARAMETER;
    }

    dcb->count = 0;

    // TODO: Initialize main device

    return DEVICE_ERR_NONE;
}

int device_dcb_deinit(struct device_ctrl_block *dcb)
{
    if (dcb == NULL) {
        return -DEVICE_ERR_PARAMETER;
    }

    // TODO: Deinitialize main device

    return DEVICE_ERR_NONE;
}

struct device_ctrl_block *dcb_alloc(void)
{
    return(struct device_ctrl_block *)malloc(sizeof(struct device_ctrl_block));
}

void dcb_free(struct device_ctrl_block *dcb)
{
    free(dcb);
}
