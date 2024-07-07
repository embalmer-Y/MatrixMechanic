#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dev.h"
#include "common.h"

int ability_set_dependency(struct ability *self, struct ability *prv, enum ability_dep_type dep_type)
{
    struct ability_dependency *dep_node;

    if (self == NULL || prv == NULL)
        return -DEVICE_ERR_PARAMETER;

    if (prv->chd->is_private) {
        return -DEVICE_ERR_NO_SUPPORT;
    }

    dep_node = (struct ability_dependency *)malloc(sizeof(struct ability_dependency));
    if (dep_node == NULL) {
        return -DEVICE_ERR_NO_MEMORY;
    }

    if (self->prv->count == 0) {
        self->prv->head = dep_node;
        self->prv->tail = dep_node;
    } else {

        self->prv->tail->next = dep_node;
        dep_node->prev = self->prv->tail;
        self->prv->tail = dep_node;
    }

    dep_node->ability = prv;
    dep_node->type = dep_type;

    self->prv->count++;

    return DEVICE_ERR_NONE;
}

int ability_get_id(struct ability_ctrl_block *acb, uint16_t *id, const char *name)
{
    struct ability *ability;
    
    if (acb == NULL || id == NULL || name == NULL)
        return -DEVICE_ERR_PARAMETER;

    ability = acb->head;
    while (ability->next) {
        if (strcmp(ability->name, name) == 0) {
            *id = ability->id;
            return DEVICE_ERR_NONE;
        }
        ability = ability->next;
    }

    return -DEVICE_ERR_NOT_FOUND;
}

struct ability *ability_alloc(void)
{
    return(struct ability *)malloc(sizeof(struct ability));
}

void ability_free(struct ability *ability)
{
    free(ability);
}

int ability_init(struct ability_ctrl_block *acb, struct ability *ability, uint16_t id, const char *name)
{
    if (ability == NULL || name == NULL)
        return -DEVICE_ERR_PARAMETER;

    ability->id = id;
    strcpy(ability->name, name);

    if (acb->count == 0) {
        acb->head = ability;
    } else {
        acb->tail->next = ability;
        ability->prev = acb->tail;
    }
    acb->tail = ability;
    acb->count++;

    return DEVICE_ERR_NONE;
}

int ability_deinit(struct ability_ctrl_block *acb, struct ability *ability)
{
    if (acb == NULL || ability == NULL)
        return -DEVICE_ERR_PARAMETER;

    if (ability->prev) {
        ability->prev->next = ability->next;
    } else {
        acb->head = ability->next;
    }
    if (ability->next) {
        ability->next->prev = ability->prev;
    } else {
        acb->tail = ability->prev;
    }
    acb->count--;

    return DEVICE_ERR_NONE;
}

int ability_add(struct ability_ctrl_block *acb, struct ability *ability)
{
    if (acb == NULL || ability == NULL)
        return -DEVICE_ERR_PARAMETER;

    if (acb->head == NULL) {
        acb->head = ability;
    } else {
        acb->tail->next = ability;
        ability->prev = acb->tail;
        acb->tail = ability;
    }
    acb->count++;

    return DEVICE_ERR_NONE;
}

int ability_remove(struct ability_ctrl_block *acb, struct ability *ability)
{
    if (acb == NULL || ability == NULL)
        return -DEVICE_ERR_PARAMETER;

    if (ability->prev) {
        ability->prev->next = ability->next;
    } else {
        acb->head = ability->next;
    }
    if (ability->next) {
        ability->next->prev = ability->prev;
    } else {
        acb->tail = ability->prev;
    }
    acb->count--;

    return DEVICE_ERR_NONE;
}

int ability_register(struct ability_ctrl_block *acb, struct ability *ability, module_init_t *func)
{
    int ret;

    if (acb == NULL || ability == NULL)
        return -DEVICE_ERR_PARAMETER;

    ret = (*func)(ability);
    if (ret != DEVICE_ERR_NONE) {
        return ret;
    }

    ret = ability_add(acb, ability);
    if (ret != DEVICE_ERR_NONE) {
        return ret;
    }

    return DEVICE_ERR_NONE;
}

int ability_unregister(struct ability_ctrl_block *acb, struct ability *ability)
{
    int ret;

    if (acb == NULL || ability == NULL)
        return -DEVICE_ERR_PARAMETER;

    ret = ability_remove(acb, ability);
    if (ret != DEVICE_ERR_NONE) {
        return ret;
    }

    return DEVICE_ERR_NONE;
}

int ability_acb_init(struct ability_ctrl_block *acb)
{
    int ret;
    if (acb == NULL)
        return -DEVICE_ERR_PARAMETER;

    acb->count = 0;

    for (module_init_t *init_func = __start_module_init_ability;
        init_func != __stop_module_init_ability;
        init_func++) {
        struct ability *ability = ability_alloc();
        if (ability == NULL)
            return -DEVICE_ERR_NO_MEMORY;

        ret = ability_register(acb, NULL, *init_func);
        if (ret != DEVICE_ERR_NONE) {
            ability_free(ability);
            return ret;
        }
    }

    return DEVICE_ERR_NONE;
}

int ability_acb_deinit(struct ability_ctrl_block *acb)
{
    if (acb == NULL)
        return -DEVICE_ERR_PARAMETER;

    while (acb->count > 0)
    {
        struct ability *ability = acb->head;
        ability_deinit(acb, ability);
        ability_free(ability);
    }

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
    
    if (dcb == NULL || id == NULL || name == NULL)
        return -DEVICE_ERR_PARAMETER;

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
    if (dev == NULL)
        return -DEVICE_ERR_PARAMETER;

    dev->next = NULL;
    dev->prev = NULL;
    dev->id = id;
    strcpy(dev->name, name);

    dev->acb = acb_alloc();
    if (dev->acb == NULL)
        return -DEVICE_ERR_NO_MEMORY;

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
    if (ret != DEVICE_ERR_NONE)
        return ret;

    acb_free(dev->acb);
    dev->acb = NULL;

    device_free(dev);

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

int device_add(struct device_ctrl_block *dcb, struct device *dev)
{
    if (dcb == NULL || dev == NULL)
        return -DEVICE_ERR_PARAMETER;

    if (dcb->head == NULL) {
        dcb->head = dev;
    } else {
        dcb->tail->next = dev;
        dev->prev = dcb->tail;
        dcb->tail = dev;
    }
    dcb->count++;

    return DEVICE_ERR_NONE;
}


int device_remove(struct device_ctrl_block *dcb, struct device *dev)
{
    if (dcb == NULL || dev == NULL)
        return -DEVICE_ERR_PARAMETER;

    if (dev->prev) {
        dev->prev->next = dev->next;
    } else {
        dcb->head = dev->next;
    }
    if (dev->next) {
        dev->next->prev = dev->prev;
    } else {
        dcb->tail = dev->prev;
    }
    dcb->count--;

    return DEVICE_ERR_NONE;
}

int device_register(struct device_ctrl_block *dcb, struct device *dev, module_init_t *func)
{
    int ret;

    if (dcb == NULL || dev == NULL)
        return -DEVICE_ERR_PARAMETER;

    ret = (*func)(dev);
    if (ret != DEVICE_ERR_NONE)
        return ret;

    ret = device_add(dcb, dev);
    if (ret != DEVICE_ERR_NONE)
        return ret;

    return DEVICE_ERR_NONE;
}

int device_unregister(struct device_ctrl_block *dcb, struct device *dev)
{
    int ret;

    if (dcb == NULL || dev == NULL)
        return -DEVICE_ERR_PARAMETER;

    while (dev->acb->count > 0)
    {
        ret = ability_deinit(dev->acb, dev->acb->head);
        if (ret != DEVICE_ERR_NONE)
            return ret;
    }
    
    ret = device_remove(dcb, dev);
    if (ret != DEVICE_ERR_NONE)
        return ret;

    return DEVICE_ERR_NONE;
}

int device_dcb_init(struct device_ctrl_block *dcb)
{
    int ret;
    module_init_t *init_func;

    if (dcb == NULL) {
        return -DEVICE_ERR_PARAMETER;
    }

    dcb->count = 0;

    for (init_func = __start_module_init_device;
        init_func != __stop_module_init_device;
        init_func++) {
        struct device *dev = device_alloc();
        if (dev == NULL)
            return -DEVICE_ERR_NO_MEMORY;

        ret = device_register(dcb, dev, *init_func);
        if (ret != DEVICE_ERR_NONE) {
            device_free(dev);
            return ret;
        }
    }

    return DEVICE_ERR_NONE;
}

int device_dcb_deinit(struct device_ctrl_block *dcb)
{
    if (dcb == NULL)
        return -DEVICE_ERR_PARAMETER;

    while (dcb->count > 0)
    {
        struct device *dev = dcb->head;
        device_unregister(dcb, dev);
        device_free(dev);
    }

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
