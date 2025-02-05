#include "route.h"

struct data_net_ability_info *data_net_ability_info_alloc(void)
{
    struct data_net_ability_info *info = NULL;

    info = (struct data_net_ability_info *)malloc(sizeof(struct data_net_ability_info));
    if (info == NULL)
        return NULL;

    return info;
}

struct data_net_dev_info *data_net_dev_info_alloc(void)
{
    struct data_net_dev_info *info = NULL;

    info = (struct data_net_dev_info *)malloc(sizeof(struct data_net_dev_info));
    if (info == NULL)
        return NULL;

    return info;
}

struct data_net_srv *data_net_srv_alloc(void)
{
    struct data_net_srv *srv = NULL;

    srv = (struct data_net_srv *)malloc(sizeof(struct data_net_srv));
    if (srv == NULL)
        return NULL;

    return srv;
}

void data_net_ability_info_free(struct data_net_ability_info *info)
{
    if (info == NULL)
        return;

    free(info);
}

void data_net_dev_info_free(struct data_net_dev_info *info)
{
    if (info == NULL)
        return;

    free(info);
}

void data_net_srv_free(struct data_net_srv *srv)
{
    if (srv == NULL)
        return;

    free(srv);
}

struct data_net_ability_info *data_net_ability_find_last(struct data_net_ability_info *first)
{
    struct data_net_ability_info *info;

    if (first == NULL)
        return NULL;

    for (info = first; info->next != NULL; info = info->next);

    return info;
}

struct data_net_dev_info *data_net_dev_find_last(struct data_net_dev_info *first)
{
    struct data_net_dev_info *info;

    if (first == NULL)
        return NULL;

    for (info = first; info->next != NULL; info = info->next);

    return info;
}

int data_net_ability_info_init(struct data_net_ability_info **info,
                               struct data_net_ability_cfg **cfg)
{
    if (*info == NULL || *cfg == NULL)
        return -ERR_INVALID;

    (*info) = data_net_ability_info_alloc();
    if ((*info) == NULL)
        return -ERR_NOMEM;

    (*info)->cfg = *cfg;

    return ERR_OK;
}

int data_net_dev_info_init(struct data_net_dev_info **info, 
                           uint16_t id, uint8_t is_micro)
{
    if (*info == NULL)
        return -ERR_INVALID;

    (*info) = data_net_dev_info_alloc();
    if ((*info) == NULL)
        return -ERR_NOMEM;

    (*info)->device_id = id;
    (*info)->is_micro = is_micro;
    (*info)->ability_num = 1;

    return ERR_OK;
}

int data_net_srv_init(struct data_net_srv **srv,
                      struct data_net_dev_info **dev_info)
{
    if (*srv == NULL || *dev_info == NULL)
        return -ERR_INVALID;

    (*srv) = data_net_srv_alloc();
    if ((*srv) == NULL)
        return -ERR_NOMEM;

    (*srv)->first = *dev_info;
    (*srv)->count = 1;

    return ERR_OK;
}

int data_net_ability_info_deinit(struct data_net_ability_info *info)
{
    if (info == NULL)
        return -ERR_INVALID;

    if (info->cfg->link_state != DATA_LINK_STATE_DISCONNECTED ||
        info->cfg->link_state != DATA_LINK_STATE_NONE)
        return -ERR_DATA_ROUTE_LINK_BUSY;

    data_net_ability_info_free(info);

    return ERR_OK;
}

int data_net_dev_info_deinit(struct data_net_dev_info *info)
{
    struct data_net_ability_info *temp;
    int ret, i, cnt;

    if (info == NULL)
        return -ERR_INVALID;

    cnt = info->ability_num;
    for (i = 0; i < cnt; i++) {
        temp = info->first->next;
        ret = data_net_ability_info_deinit(info->first);
        if (ret != ERR_OK) {
            return ret;
        }

        info->first = temp;
        info->ability_num--;
    }

    data_net_dev_info_free(info);

    return ERR_OK;
}

int data_net_srv_deinit(struct data_net_srv *srv)
{
    struct data_net_dev_info *temp, *last;
    int ret, i, cnt;

    if (srv == NULL)
        return -ERR_INVALID;

    last = data_net_dev_find_last(srv->first);
    if (last == NULL)
        return -ERR_DATA_ROUTE_NODE_BROKEN;

    cnt = srv->count;

    for (i = 0; i < cnt; i++) {
        temp = last->prev;
        ret = data_net_dev_info_deinit(last);
        if (ret != ERR_OK) {
            return ret;
        }

        last = temp;
        srv->count--;
    }

    data_net_srv_free(srv);

    return ERR_OK;
}

int data_net_ability_info_add(struct data_net_dev_info **dev_info, 
                              struct data_net_ability_info **a_info)
{
    struct data_net_ability_info *temp;

    if (*dev_info == NULL || *a_info == NULL)
        return -ERR_INVALID;

    temp = data_net_ability_find_last((*dev_info)->first);
    if (temp == NULL) {
        (*dev_info)->first->next = *a_info;
    } else {
        temp->next = *a_info;
    }

    (*dev_info)->ability_num++;

    return ERR_OK;
}

int data_net_dev_info_add(struct data_net_srv **srv,
                          struct data_net_dev_info **dev_info)
{
    struct data_net_dev_info *temp;

    if (*srv == NULL || *dev_info == NULL)
        return -ERR_INVALID;

    temp = data_net_dev_find_last((*srv)->first);
    if (temp == NULL)
        return -ERR_DATA_ROUTE_NODE_BROKEN;

    temp->next = *dev_info;
    (*srv)->count++;

    return ERR_OK;
}

int data_net_ability_find_by_id(struct data_net_ability_info **first,
                                struct data_net_ability_info **target, 
                                uint16_t id)
{
    struct data_net_ability_info *temp;

    if (*first == NULL)
        return -ERR_INVALID;

    for (temp = *first; temp != NULL; temp = temp->next) {
        if (temp->cfg->ability_id == id) {
            *target = temp;
            return ERR_OK;
        }
    }

    return -ERR_DATA_ROUTE_ABILITY_NOT_FOUND;
}

int data_net_dev_find_by_id(struct data_net_dev_info **first,
                            struct data_net_dev_info **target,
                            uint16_t id)
{
    struct data_net_dev_info *temp;

    if (*first == NULL)
        return -ERR_INVALID;

    for (temp = *first; temp != NULL; temp = temp->next) {
        if (temp->device_id == id) {
            *target = temp;
            return ERR_OK;
        }
    }

    return -ERR_DATA_ROUTE_DEV_NOT_FOUND;
}

int data_net_ability_info_remove(struct data_net_dev_info **dev_info,
                                 uint16_t id)
{
    struct data_net_ability_info *temp;
    int ret;

    if (*dev_info == NULL)
        return -ERR_INVALID;

    ret = data_net_ability_find_by_id(&(*dev_info)->first, &temp, id);
    if (ret != ERR_OK)
        return ret;

    temp->prev->next = temp->next;
    if (temp->next != NULL)
        temp->next->prev = temp->prev;

    data_net_ability_info_free(temp);
    (*dev_info)->ability_num--;

    return ERR_OK;
}

int data_net_dev_info_remove(struct data_net_srv **srv, uint16_t id)
{
    struct data_net_dev_info *temp;
    int ret;

    if (*srv == NULL)
        return -ERR_INVALID;

    ret = data_net_dev_find_by_id(&(*srv)->first, &temp, id);
    if (ret != ERR_OK)
        return ret;

    temp->prev->next = temp->next;
    if (temp->next != NULL)
        temp->next->prev = temp->prev;

    data_net_dev_info_deinit(temp);
    (*srv)->count--;

    return ERR_OK;
}
