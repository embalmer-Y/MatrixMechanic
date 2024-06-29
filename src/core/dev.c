#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

int device_get_id(struct device_ctrl_block *dcb, uint16_t *id, const char *name)
{
    struct device *dev;
    
    if (dcb == NULL || id == NULL || name == NULL) {
        return -DEVICE_ERR_PARAMETER;
    }

    dev = dcb->head;
    while (dev->next) {
        if (strcmp(dev->name, name) == 0) {
            *id = dev->id;
            return DEVICE_ERR_NONE;
        }
        dev = dev->next;
    }

    return -DEVICE_ERR_NOT_FOUND;
}

struct device *device_alloc(void)
{
    return(struct device *)malloc(sizeof(struct device));
}

void device_free(struct device *dev)
{
    free(dev);
}


int device_init(struct device *dev, uint16_t id, const char *name)
{
    int ret;
    if (dev == NULL) {
        return -DEVICE_ERR_PARAMETER;
    }

    dev->next = NULL;
    dev->prev = NULL;
    dev->id = id;
    strcpy(dev->name, name);

    dev->acb = acb_alloc();
    if (dev->acb == NULL) {
        return -DEVICE_ERR_NO_MEMORY;
    }

    ret = ability_acb_init(dev->acb);
    if (ret != DEVICE_ERR_NONE) {
        acb_free(dev->acb);
        return ret;
    }

    return DEVICE_ERR_NONE;
}

int device_deinit(struct device *dev)
{
    int ret;
    if (dev == NULL) {
        return -DEVICE_ERR_PARAMETER;
    }

    ret = ability_acb_deinit(dev->acb);
    if (ret != DEVICE_ERR_NONE) {
        return ret;
    }

    acb_free(dev->acb);
    dev->acb = NULL;

    device_free(dev);

    return DEVICE_ERR_NONE;
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
